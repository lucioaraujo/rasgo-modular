#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// ARP (#64) — arpejador (2026-11-08, aprovado pelo autor a partir do estudo do
// Cardinal, dossies/ESTUDO_cardinal_catalogo_e_licencas.md §5.2).
//
// Percorre as notas de um acorde no clock: a tabela de 10 formatos é a do
// CHORD (#26), com inversão e extensão em oitavas. Modos clássicos dos
// arpejadores de sintetizador (Roland Jupiter-4, ARP Odyssey — ideia
// pública): sobe, desce, ida e volta, ida e volta repetindo as pontas,
// aleatório e embaralhado. Conceito lido no Bogaudio ARP e no Amalgamated
// Harmonics Arp 3.x; nenhum código copiado.
//
// Desvio Rasgo:
//  (a) DIVERGIR e CONVERGIR — do centro do acorde para fora, e de fora para o
//      centro (a ideia dos padrões que divergem da Amalgamated Harmonics);
//  (b) SKIP — chance SEMEADA de pausa em cada passo: o arpejo respira;
//  (c) ROOT em 1 V/oct aceita direto a saída `root` do HARMONY (semitom/12):
//      o arpejo anda com a progressão.
//
// Modo autônomo: sem CLOCK, o relógio interno em RATE; sem ROOT, a nota de
// base (0 V). Soa ao carregar. Aleatório, embaralhado e SKIP vêm de um
// xorshift semeado no prepare() — determinístico. process() não aloca.
namespace rasgo::modular {

class Arp final : public Signal {
public:
    static constexpr int kModes = 8;

    Arp() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"},
         {"root", PortKind::Control, "v/oct"},
         {"chord_cv", PortKind::Control, ""}},
        {{"pitch", PortKind::Audio, ""},
         {"gate", PortKind::Control, "gate"},
         {"eos", PortKind::Control, "gate"}},
        {{"rate", 0.01f, 40.0f, 6.0f, "Hz"},
         {"gate_len", 0.05f, 0.95f, 0.5f, ""},
         {"chord", 0.0f, 9.0f, 3.0f, ""},
         {"inversion", 0.0f, 3.0f, 0.0f, ""},
         {"octaves", 1.0f, 4.0f, 2.0f, ""},
         {"mode", 0.0f, 7.0f, 0.0f, ""},
         {"skip", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "ARP"; }

    Panel panel() const override {
        Panel p;
        // 10 HP (era 16 — pedido do autor, 8/11: largo demais). O Display
        // "arp" mostra as notas rolando (a altura, com os gates em destaque).
        p.hp = 10;
        p.add(Widget::Kind::Label, "ARP", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "arp", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "CHORD", "chord", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "INV", "inversion", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "OCT", "octaves", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "SKIP", "skip", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 36.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 5.0f, 70.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "ROOT", "in:root", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CHD", "in:chord_cv", 43.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "EOS", "out:eos", 31.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0xA4B9C0FFEE640001ULL;
        phase_ = 1.0;          // o primeiro passo cai na primeira amostra
        prevClock_ = prevReset_ = 0.0f;
        extPeriod_ = 0.2f * sr_;
        sinceClock_ = 0.0f;
        gateSamples_ = eosSamples_ = 0;
        step_ = -1;
        pitch_ = 0.0f;
        gateOn_ = false;
        count_ = 0;
        order_ = {};
    }

    // As notas (semitons acima da tônica, em ordem crescente) para um formato,
    // inversão e extensão — exposto para os testes.
    static int notes(int chord, int inversion, int octaves, std::array<int, 16>& out) noexcept {
        static constexpr int kChords[10][4] = {
            {0, 0, 0, 0}, {0, 12, 0, 12}, {0, 7, 12, 19}, {0, 4, 7, 12}, {0, 3, 7, 12},
            {0, 5, 7, 12}, {0, 4, 7, 11}, {0, 3, 7, 10}, {0, 3, 6, 9}, {0, 4, 7, 14}};
        const int c = std::clamp(chord, 0, 9);
        std::array<int, 4> base{};
        int n = 0;
        for (int k = 0; k < 4; ++k) {
            const int v = kChords[c][k];
            bool dup = false;
            for (int j = 0; j < n; ++j) dup = dup || base[static_cast<std::size_t>(j)] == v;
            if (!dup) base[static_cast<std::size_t>(n++)] = v;
        }
        sortFirst(base, n);
        // Inversão: as `inversion` notas mais graves sobem uma oitava.
        for (int k = 0; k < std::clamp(inversion, 0, 3) && k < n; ++k) base[static_cast<std::size_t>(k)] += 12;
        sortFirst(base, n);
        int total = 0;
        for (int o = 0; o < std::clamp(octaves, 1, 4); ++o)
            for (int k = 0; k < n && total < 16; ++k)
                out[static_cast<std::size_t>(total++)] = base[static_cast<std::size_t>(k)] + 12 * o;
        return total;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 3) return;
        const AudioBlock* clock = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* root = inputs.size() > 2 ? inputs[2] : nullptr;
        const AudioBlock* chordCv = inputs.size() > 3 ? inputs[3] : nullptr;

        const float rate = std::clamp(safe(parameterValue("rate"), 6.0f), 0.01f, 40.0f);
        const float gateLen = std::clamp(safe(parameterValue("gate_len"), 0.5f), 0.05f, 0.95f);
        const int inversion = static_cast<int>(std::lround(safe(parameterValue("inversion"), 0.0f)));
        const int octaves = static_cast<int>(std::lround(safe(parameterValue("octaves"), 2.0f)));
        const int mode = std::clamp(static_cast<int>(std::lround(safe(parameterValue("mode"), 0.0f))), 0, kModes - 1);
        const float skip = std::clamp(safe(parameterValue("skip"), 0.0f), 0.0f, 1.0f);
        const float chordKnob = safe(parameterValue("chord"), 3.0f);
        const double dp = static_cast<double>(rate) / sr_;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) { step_ = -1; count_ = 0; phase_ = 1.0; }
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
                int chord = static_cast<int>(std::lround(chordKnob + (chordCv ? 9.0f * safe(chordCv->at(0, f), 0.0f) : 0.0f)));
                std::array<int, 16> list{};
                const int n = notes(chord, inversion, octaves, list);
                const int length = sequenceLength(mode, n);
                count_ = (step_ < 0) ? 0 : (count_ + 1) % length;
                if (count_ == 0) {
                    if (mode == 5) shuffle(n);
                    eosSamples_ = std::max(1, static_cast<int>(0.005f * sr_));
                }
                step_ = indexFor(mode, count_, n);
                const float base = root ? safe(root->at(0, f), 0.0f) : 0.0f;
                pitch_ = base + static_cast<float>(list[static_cast<std::size_t>(std::clamp(step_, 0, n - 1))]) / 12.0f;
                gateOn_ = !(skip > 0.0f && unit() < skip);
                const float period = clock ? extPeriod_ : static_cast<float>(sr_ / rate);
                gateSamples_ = gateOn_ ? std::max(1, static_cast<int>(gateLen * period)) : 0;
            }

            const float gate = gateSamples_ > 0 ? 1.0f : 0.0f;
            if (gateSamples_ > 0) --gateSamples_;
            const float eos = eosSamples_ > 0 ? 1.0f : 0.0f;
            if (eosSamples_ > 0) --eosSamples_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = pitch_;
                outputs[1].at(c, f) = gate;
                outputs[2].at(c, f) = eos;
            }
        }
    }

    // O comprimento de um ciclo e a nota de cada posição — expostos para os testes.
    static int sequenceLength(int mode, int n) noexcept {
        if (n <= 1) return 1;
        if (mode == 2) return 2 * n - 2;
        if (mode == 3) return 2 * n;
        return n;
    }
    int indexFor(int mode, int k, int n) noexcept {
        if (n <= 1) return 0;
        switch (mode) {
        case 0: return k;                                   // sobe
        case 1: return n - 1 - k;                           // desce
        case 2: return k < n ? k : 2 * n - 2 - k;           // ida e volta
        case 3: return k < n ? k : 2 * n - 1 - k;           // ida e volta repetindo as pontas
        case 4: return static_cast<int>(unit() * static_cast<float>(n)) % n;  // aleatório
        case 5: return order_[static_cast<std::size_t>(k % n)];              // embaralhado
        case 6: return divergent(k, n);                     // divergir: do centro para fora
        default: return divergent(n - 1 - k, n);            // convergir: de fora para o centro
        }
    }
    static int divergent(int k, int n) noexcept {
        const int center = (n - 1) / 2;
        const int step = (k + 1) / 2;
        const int v = (k % 2 == 1) ? center + step : center - step;
        return std::clamp(v, 0, n - 1);
    }

private:
    // Inserção nas primeiras n (≤ 4) posições — std::sort num pedaço do array
    // dispara um falso -Warray-bounds no GCC 13.
    static void sortFirst(std::array<int, 4>& a, int n) noexcept {
        for (int i = 1; i < std::min(n, 4); ++i)
            for (int j = i; j > 0 && a[static_cast<std::size_t>(j - 1)] > a[static_cast<std::size_t>(j)]; --j)
                std::swap(a[static_cast<std::size_t>(j - 1)], a[static_cast<std::size_t>(j)]);
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    void shuffle(int n) noexcept {
        for (int i = 0; i < n; ++i) order_[static_cast<std::size_t>(i)] = i;
        for (int i = n - 1; i > 0; --i) {
            const int j = static_cast<int>(unit() * static_cast<float>(i + 1)) % (i + 1);
            std::swap(order_[static_cast<std::size_t>(i)], order_[static_cast<std::size_t>(j)]);
        }
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0xA4B9C0FFEE640001ULL;
    double phase_ = 1.0;
    float prevClock_ = 0.0f, prevReset_ = 0.0f, extPeriod_ = 9600.0f, sinceClock_ = 0.0f;
    int gateSamples_ = 0, eosSamples_ = 0, step_ = -1, count_ = 0;
    float pitch_ = 0.0f;
    bool gateOn_ = false;
    std::array<int, 16> order_{};
};

} // namespace rasgo::modular
