#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// MARKOV (#66) — cadeia de Markov que aprende e improvisa (2026-11-09,
// aprovado pelo autor a partir do estudo do Cardinal,
// dossies/ESTUDO_cardinal_catalogo_e_licencas.md §4b).
//
// Cadeia de primeira ordem sobre as 12 classes de altura: uma tabela de
// transições 12×12 (de qual nota para qual). Aprende do que ouve — a cada
// gate em LRN.G, a altura em LRN.P (1 V/oct) vira uma transição contada — e
// improvisa no próprio clock, sorteando a próxima nota pelas transições da
// atual. A ideia é antiga e pública: L. Hiller e L. Isaacson, Illiac Suite
// (1957); I. Xenakis, Analogique A (1958–59). Conceito lido no ZetaCarinae
// Rosenchance e GuildensTurn (GPL-3.0+); nenhum código copiado.
//
// Desvio Rasgo:
//  (a) TEMP — temperatura do sorteio: 0 = sempre a transição mais provável
//      (repete o que aprendeu), 1 = as probabilidades aprendidas, 2 = quase
//      uniforme (inventa);
//  (b) SURP — saída de surpresa: quão improvável foi a nota tocada (0 = a
//      esperada, 1 = a mais rara) — um sinal para mover o resto do patch;
//  (c) MEM — esquecimento: as transições antigas desbotam, a cadeia segue o
//      que está tocando agora.
//
// Modo autônomo: sem nada ligado, começa de uma caminhada por uma
// pentatônica (0, 2, 4, 7, 9) e toca no relógio interno — soa ao carregar.
// O sorteio vem de um xorshift semeado no prepare(): determinístico.
// process() não aloca.
namespace rasgo::modular {

class Markov final : public Signal {
public:
    static constexpr int kStates = 12;

    Markov() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"learn_pitch", PortKind::Control, "v/oct"},
         {"learn_gate", PortKind::Control, "gate"},
         {"reset", PortKind::Control, "trig"}},
        {{"pitch", PortKind::Audio, ""},
         {"gate", PortKind::Control, "gate"},
         {"surprise", PortKind::Control, ""}},
        {{"rate", 0.05f, 30.0f, 5.0f, "Hz"},
         {"gate_len", 0.05f, 0.95f, 0.5f, ""},
         {"temp", 0.0f, 2.0f, 1.0f, ""},
         {"memory", 0.0f, 1.0f, 0.8f, ""},
         {"learn", 0.0f, 1.0f, 1.0f, ""},
         {"octave", -2.0f, 2.0f, 0.0f, "oct"}}) {}

    std::string type() const override { return "MARKOV"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "MARKOV", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "markov", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "TEMP", "temp", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MEM", "memory", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "OCT", "octave", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 20.5f, 50.0f);
        p.add(Widget::Kind::Toggle, "LEARN", "learn", 38.3f, 54.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "LRN.P", "in:learn_pitch", 18.5f, 96.0f);
        p.add(Widget::Kind::Jack, "LRN.G", "in:learn_gate", 33.5f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 45.5f, 96.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SURP", "out:surprise", 31.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x3A4C0FF1E5E66ULL;
        phase_ = 1.0;
        prevClock_ = prevReset_ = prevLearn_ = 0.0f;
        extPeriod_ = 0.2f * sr_;
        sinceClock_ = 0.0f;
        gateSamples_ = 0;
        surprise_ = 0.0f;
        resetTable();
        state_ = 0;
        octaveBase_ = 0.0f;
        lastLearned_ = -1;
    }

    // Probabilidade da transição a → b (para os testes e o mostrador).
    float probability(int a, int b) const noexcept {
        const auto& r = table_[static_cast<std::size_t>(std::clamp(a, 0, kStates - 1))];
        float sum = 0.0f;
        for (float v : r) sum += v;
        return sum > 0.0f ? r[static_cast<std::size_t>(std::clamp(b, 0, kStates - 1))] / sum : 0.0f;
    }
    int state() const noexcept { return state_; }

    // A classe de altura (0–11) de uma tensão 1 V/oct.
    static int pitchClass(float v) noexcept {
        const int semis = static_cast<int>(std::lround(v * 12.0f));
        return ((semis % 12) + 12) % 12;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 3) return;
        const AudioBlock* clock = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* lp = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* lg = inputs.size() > 2 ? inputs[2] : nullptr;
        const AudioBlock* reset = inputs.size() > 3 ? inputs[3] : nullptr;

        const float rate = std::clamp(safe(parameterValue("rate"), 5.0f), 0.05f, 30.0f);
        const float gateLen = std::clamp(safe(parameterValue("gate_len"), 0.5f), 0.05f, 0.95f);
        const float temp = std::clamp(safe(parameterValue("temp"), 1.0f), 0.0f, 2.0f);
        const float memory = std::clamp(safe(parameterValue("memory"), 0.8f), 0.0f, 1.0f);
        const bool learn = safe(parameterValue("learn"), 1.0f) >= 0.5f;
        const float octave = std::round(std::clamp(safe(parameterValue("octave"), 0.0f), -2.0f, 2.0f));
        const double dp = static_cast<double>(rate) / sr_;
        // MEM 1 = não esquece; 0 = cada transição nova pesa quase tudo.
        const float keep = 0.5f + 0.5f * memory;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) { resetTable(); state_ = 0; lastLearned_ = -1; }
                prevReset_ = r;
            }
            if (lg && lp) {
                const float g = safe(lg->at(0, f), 0.0f);
                if (prevLearn_ < 0.5f && g >= 0.5f && learn) {
                    const float v = safe(lp->at(0, f), 0.0f);
                    const int pc = pitchClass(v);
                    octaveBase_ = std::floor(v + 0.5f / 12.0f);
                    if (lastLearned_ >= 0) learnTransition(lastLearned_, pc, keep);
                    lastLearned_ = pc;
                }
                prevLearn_ = g;
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
                state_ = choose(state_, temp);
                const float period = clock ? extPeriod_ : static_cast<float>(sr_ / rate);
                gateSamples_ = std::max(1, static_cast<int>(gateLen * period));
            }
            const float pitch = octaveBase_ + octave + static_cast<float>(state_) / 12.0f;
            const float gate = gateSamples_ > 0 ? 1.0f : 0.0f;
            if (gateSamples_ > 0) --gateSamples_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = pitch;
                outputs[1].at(c, f) = gate;
                outputs[2].at(c, f) = surprise_;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    // A tabela inicial: uma caminhada pela pentatônica (0, 2, 4, 7, 9) — de
    // cada grau para os vizinhos, com um pouco de chance de saltar.
    void resetTable() noexcept {
        static constexpr int kPenta[5] = {0, 2, 4, 7, 9};
        for (auto& r : table_) r.fill(0.0f);
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 5; ++j) {
                const int d = std::abs(i - j);
                const float w = d == 1 ? 1.0f : (d == 2 ? 0.4f : (d == 0 ? 0.2f : 0.1f));
                table_[static_cast<std::size_t>(kPenta[i])][static_cast<std::size_t>(kPenta[j])] = w;
            }
        // classes fora da escala voltam para a escala
        for (int s = 0; s < kStates; ++s) {
            float sum = 0.0f;
            for (float v : table_[static_cast<std::size_t>(s)]) sum += v;
            if (sum <= 0.0f)
                for (int k : kPenta) table_[static_cast<std::size_t>(s)][static_cast<std::size_t>(k)] = 1.0f;
        }
    }
    void learnTransition(int a, int b, float keep) noexcept {
        auto& r = table_[static_cast<std::size_t>(a)];
        float sum = 0.0f;
        for (float& v : r) { v *= keep; sum += v; }
        r[static_cast<std::size_t>(b)] += 1.0f;
        // teto: a linha nunca cresce sem limite (sessões longas)
        if (sum > 64.0f) for (float& v : r) v *= 64.0f / sum;
    }
    // Sorteia a próxima nota. TEMP 0 → a mais provável; 1 → as aprendidas;
    // 2 → quase uniforme. A surpresa é 1 − p/pmax da nota escolhida.
    int choose(int from, float temp) noexcept {
        const auto& r = table_[static_cast<std::size_t>(std::clamp(from, 0, kStates - 1))];
        std::array<float, kStates> w{};
        float pmax = 0.0f;
        for (float v : r) pmax = std::max(pmax, v);
        if (pmax <= 0.0f) { surprise_ = 0.0f; return from; }
        int best = 0;
        for (int k = 0; k < kStates; ++k)
            if (r[static_cast<std::size_t>(k)] > r[static_cast<std::size_t>(best)]) best = k;
        if (temp < 0.02f) { surprise_ = 0.0f; return best; }
        float sum = 0.0f;
        const float expo = 1.0f / temp;
        for (int k = 0; k < kStates; ++k) {
            const float p = r[static_cast<std::size_t>(k)] / pmax;
            // acima de 1, mistura com o uniforme sobre as notas já conhecidas
            float v = p > 0.0f ? std::pow(p, expo) : 0.0f;
            if (temp > 1.0f && p > 0.0f) v = v + (temp - 1.0f) * (1.0f - v);
            w[static_cast<std::size_t>(k)] = v;
            sum += v;
        }
        float x = unit() * sum;
        int pick = best;
        for (int k = 0; k < kStates; ++k) {
            x -= w[static_cast<std::size_t>(k)];
            if (x <= 0.0f && w[static_cast<std::size_t>(k)] > 0.0f) { pick = k; break; }
        }
        surprise_ = 1.0f - r[static_cast<std::size_t>(pick)] / pmax;
        return pick;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x3A4C0FF1E5E66ULL;
    double phase_ = 1.0;
    float prevClock_ = 0.0f, prevReset_ = 0.0f, prevLearn_ = 0.0f;
    float extPeriod_ = 9600.0f, sinceClock_ = 0.0f;
    int gateSamples_ = 0;
    float surprise_ = 0.0f;
    std::array<std::array<float, kStates>, kStates> table_{};
    int state_ = 0;
    float octaveBase_ = 0.0f;
    int lastLearned_ = -1;
};

} // namespace rasgo::modular
