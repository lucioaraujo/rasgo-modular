#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// CELL (#65) — autômato celular como sequenciador (2026-11-09, aprovado pelo
// autor a partir do estudo do Cardinal, dossies/ESTUDO_cardinal_catalogo_e_licencas.md §4b).
//
// Autômato celular elementar (S. Wolfram, "Statistical mechanics of cellular
// automata", Rev. Mod. Phys. 55, 1983 — regras 0–255): uma linha de células
// vivas/mortas; a cada pulso do clock nasce a geração seguinte, e cada célula
// olha para si e as duas vizinhas (a linha é circular). Conceito lido no
// voxglitch Glitch Sequencer (GPL-3.0+); nenhum código copiado.
//
// Leitura musical: a célula sob a CABEÇA (HEAD) dá o gate; as três células
// seguintes, lidas como número binário (0–7), escolhem o grau de uma
// pentatônica maior em 1 V/oct; RANGE (0–2) acrescenta oitavas, cada uma
// quando a célula seguinte está viva; DENS é a fração de células vivas.
//
// Desvio Rasgo:
//  (a) MUT — a cada geração, chance SEMEADA de inverter uma célula: a regra
//      nunca congela de todo, o padrão respira;
//  (b) a linha que MORRE (nenhuma viva) ou PARA (igual à anterior) renasce
//      sozinha com outro sorteio semeado — e o pulso EOS avisa;
//  (c) RULE por CV (`rule_cv`, 0–1 percorre as 256 regras).
//
// Modo autônomo: sem CLK, relógio interno em RATE; soa ao carregar. A linha
// inicial vem de um xorshift semeado no prepare() — determinístico.
// process() não aloca.
namespace rasgo::modular {

class Cell final : public Signal {
public:
    static constexpr int kMaxCells = 24;   // cabe exato num float (o mostrador)

    Cell() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"},
         {"rule_cv", PortKind::Control, ""}},
        {{"pitch", PortKind::Audio, ""},
         {"gate", PortKind::Control, "gate"},
         {"density", PortKind::Control, ""},
         {"eos", PortKind::Control, "gate"}},
        {{"rate", 0.05f, 30.0f, 4.0f, "Hz"},
         {"rule", 0.0f, 255.0f, 30.0f, ""},
         {"cells", 8.0f, 24.0f, 16.0f, ""},
         {"head", 0.0f, 1.0f, 0.0f, ""},
         {"range", 0.0f, 2.0f, 1.0f, "oct"},
         {"mutate", 0.0f, 1.0f, 0.05f, ""},
         {"gate_len", 0.05f, 0.95f, 0.5f, ""}}) {}

    std::string type() const override { return "CELL"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "CELL", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "cell", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "RULE", "rule", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CELLS", "cells", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "HEAD", "head", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "MUT", "mutate", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 36.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 5.0f, 70.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RULE", "in:rule_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "DENS", "out:density", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "EOS", "out:eos", 43.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0xCE11A0707A5E0065ULL;
        phase_ = 1.0;
        prevClock_ = prevReset_ = 0.0f;
        extPeriod_ = 0.25f * sr_;
        sinceClock_ = 0.0f;
        gateSamples_ = eosSamples_ = 0;
        pitch_ = 0.0f;
        started_ = false;
        row_ = 0;
        reseed(16);
    }

    // A geração seguinte de uma linha circular de `n` células — exposta para
    // os testes. Bit i = célula i; vizinhança (esq, eu, dir) → bit da regra.
    static std::uint32_t step(std::uint32_t row, int rule, int n) noexcept {
        const int cells = std::clamp(n, 1, kMaxCells);
        const auto r = static_cast<unsigned>(std::clamp(rule, 0, 255));
        std::uint32_t next = 0;
        for (int i = 0; i < cells; ++i) {
            const int l = (i + cells - 1) % cells, c = i, rr = (i + 1) % cells;
            const unsigned idx = ((row >> l) & 1U) << 2 | ((row >> c) & 1U) << 1 | ((row >> rr) & 1U);
            if ((r >> idx) & 1U) next |= (1U << i);
        }
        return next;
    }

    std::uint32_t row() const noexcept { return row_; }
    // Para o mostrador: a linha atual como float (exato até 24 bits).
    float rowAsFloat() const noexcept { return static_cast<float>(row_); }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* clock = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* ruleCv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float rate = std::clamp(safe(parameterValue("rate"), 4.0f), 0.05f, 30.0f);
        const float ruleKnob = safe(parameterValue("rule"), 30.0f);
        const int cells = std::clamp(static_cast<int>(std::lround(safe(parameterValue("cells"), 16.0f))), 8, kMaxCells);
        const float headK = std::clamp(safe(parameterValue("head"), 0.0f), 0.0f, 1.0f);
        const float range = std::clamp(safe(parameterValue("range"), 1.0f), 0.0f, 2.0f);
        const float mutate = std::clamp(safe(parameterValue("mutate"), 0.05f), 0.0f, 1.0f);
        const float gateLen = std::clamp(safe(parameterValue("gate_len"), 0.5f), 0.05f, 0.95f);
        const double dp = static_cast<double>(rate) / sr_;
        const int head = std::min(cells - 1, static_cast<int>(headK * static_cast<float>(cells)));
        const std::uint32_t mask = cells >= 32 ? 0xFFFFFFFFU : ((1U << cells) - 1U);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) { reseed(cells); started_ = false; phase_ = 1.0; }
                prevReset_ = r;
            }
            bool tick = false;
            if (clock) {
                const float c = safe(clock->at(0, f), 0.0f);
                sinceClock_ += 1.0f;
                if (prevClock_ < 0.5f && c >= 0.5f) {
                    tick = true;
                    if (sinceClock_ > 16.0f) extPeriod_ = sinceClock_;
                    sinceClock_ = 0.0f;
                }
                prevClock_ = c;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) { phase_ -= std::floor(phase_); tick = true; }
            }

            if (tick) {
                row_ &= mask;
                if (started_) {
                    const int rule = static_cast<int>(std::lround(ruleKnob
                        + (ruleCv ? 255.0f * safe(ruleCv->at(0, f), 0.0f) : 0.0f)));
                    const std::uint32_t next = step(row_, rule, cells) & mask;
                    const bool stuck = next == row_ || next == 0;
                    row_ = next;
                    if (mutate > 0.0f && unit() < mutate)
                        row_ ^= 1U << static_cast<unsigned>(static_cast<int>(unit() * static_cast<float>(cells)) % cells);
                    if (stuck && (row_ == 0 || row_ == next)) {
                        reseed(cells);
                        eosSamples_ = std::max(1, static_cast<int>(0.005f * sr_));
                    }
                    if (row_ == 0) reseed(cells);
                }
                started_ = true;
                const bool alive = (row_ >> head) & 1U;
                unsigned word = 0;
                for (int k = 1; k <= 3; ++k)
                    word = (word << 1) | ((row_ >> ((head + k) % cells)) & 1U);
                static constexpr int kPenta[8] = {0, 2, 4, 7, 9, 12, 14, 16};
                // RANGE (0–2, inteiro): cada oitava extra entra quando a
                // célula seguinte (cabeça+4, cabeça+5) está viva.
                int octs = 0;
                const int extra = static_cast<int>(std::lround(range));
                for (int k = 0; k < extra; ++k)
                    octs += static_cast<int>((row_ >> ((head + 4 + k) % cells)) & 1U);
                pitch_ = static_cast<float>(kPenta[word]) / 12.0f + static_cast<float>(octs);
                const float period = clock ? extPeriod_ : static_cast<float>(sr_ / rate);
                gateSamples_ = alive ? std::max(1, static_cast<int>(gateLen * period)) : 0;
                density_ = static_cast<float>(popcount(row_ & mask)) / static_cast<float>(cells);
            }

            const float gate = gateSamples_ > 0 ? 1.0f : 0.0f;
            if (gateSamples_ > 0) --gateSamples_;
            const float eos = eosSamples_ > 0 ? 1.0f : 0.0f;
            if (eosSamples_ > 0) --eosSamples_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = pitch_;
                outputs[1].at(c, f) = gate;
                outputs[2].at(c, f) = density_;
                outputs[3].at(c, f) = eos;
            }
        }
    }

private:
    static int popcount(std::uint32_t v) noexcept {
        int n = 0;
        while (v) { v &= v - 1; ++n; }
        return n;
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    // Linha nova: cada célula viva com chance 0,4; nunca toda morta.
    void reseed(int cells) noexcept {
        row_ = 0;
        for (int i = 0; i < std::clamp(cells, 1, kMaxCells); ++i)
            if (unit() < 0.4f) row_ |= 1U << i;
        if (row_ == 0) row_ = 1U << (std::clamp(cells, 1, kMaxCells) / 2);
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0xCE11A0707A5E0065ULL;
    double phase_ = 1.0;
    float prevClock_ = 0.0f, prevReset_ = 0.0f, extPeriod_ = 12000.0f, sinceClock_ = 0.0f;
    int gateSamples_ = 0, eosSamples_ = 0;
    float pitch_ = 0.0f, density_ = 0.0f;
    bool started_ = false;
    std::uint32_t row_ = 0;
};

} // namespace rasgo::modular
