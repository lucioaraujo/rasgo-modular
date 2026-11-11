#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// MAGNET (#89) — uma grade de ímãs (rascunho de 2026-11-09, ideia do autor:
// "imãs"; entrou em 11/11/2026, decisão do autor: "magnet e orbit entram";
// dossiê `dossies/89_magnet.md`).
//
// O modelo de Ising (E. Ising, 1925; L. Onsager, 1944): 8×8 pequenos ímãs
// (spins), cada um para cima ou para baixo, numa grade circular. Cada um quer
// se alinhar com os vizinhos (COUPL > 0: ferromagneto) ou se opor a eles
// (COUPL < 0: antiferromagneto, um tabuleiro de xadrez); a temperatura (TEMP)
// os agita. Abaixo da temperatura crítica (Tc ≈ 2,27) formam-se domínios
// grandes; acima, o ruído vence; perto dela, domínios de todos os tamanhos
// aparecem e somem — a borda do caos. Dinâmica de Metropolis (1953), semeada.
// Conceito lido no CVfunk Magnets (MIT); nenhum código copiado.
//
// A cada pulso (CLK ou o relógio interno em RATE) acontecem SWEEP×64
// tentativas de virar um ímã; uma CABEÇA anda pela grade, uma casa por pulso.
//
// Saídas: GT (gate quando o ímã sob a cabeça aponta para cima), MAG
// (magnetização, −1..1), ENER (energia por ímã, 0 = mínima … 1 = máxima),
// FLIP (fração da grade que virou neste pulso, 0–1).
//
// Desvio Rasgo:
//  (a) QUENCH — um gatilho que esquenta tudo de uma vez: a grade vira ruído e
//      volta a se organizar na temperatura atual;
//  (b) FIELD por CV — um ímã externo empurra a grade inteira.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Magnet final : public Signal {
public:
    static constexpr int kSide = 8;
    static constexpr int kN = kSide * kSide;

    Magnet() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"quench", PortKind::Control, "trig"},
         {"temp_cv", PortKind::Control, ""},
         {"field_cv", PortKind::Control, ""}},
        {{"gate", PortKind::Control, "gate"},
         {"mag", PortKind::Control, ""},
         {"energy", PortKind::Control, ""},
         {"flips", PortKind::Control, ""}},
        {{"temp", 0.1f, 6.0f, 2.3f, ""},
         {"coupling", -1.0f, 1.0f, 1.0f, ""},
         {"field", -1.0f, 1.0f, 0.0f, ""},
         {"sweep", 0.05f, 1.0f, 0.5f, ""},
         {"rate", 0.1f, 30.0f, 6.0f, "Hz"},
         {"gate_len", 0.05f, 0.95f, 0.5f, ""}}) {}

    std::string type() const override { return "MAGNET"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "MAGNET", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "magnet", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "TEMP", "temp", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "COUPL", "coupling", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "FIELD", "field", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SWEEP", "sweep", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 36.0f, 50.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "QNCH", "in:quench", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "TEMP", "in:temp_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FLD", "in:field_cv", 44.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "MAG", "out:mag", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ENER", "out:energy", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "FLIP", "out:flips", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x3A6E7B0072ULL;
        phase_ = 1.0;
        prevClock_ = prevQuench_ = 0.0f;
        extPeriod_ = 0.2f * sr_;
        sinceClock_ = 0.0f;
        gateSamples_ = 0;
        head_ = 0;
        flips_ = 0.0f;
        randomize();
    }

    int spin(int i) const noexcept { return spins_[static_cast<std::size_t>(((i % kN) + kN) % kN)]; }
    float magnetization() const noexcept {
        int s = 0;
        for (auto v : spins_) s += v;
        return static_cast<float>(s) / static_cast<float>(kN);
    }
    // Energia por ímã normalizada a 0–1 (só o acoplamento; 0,5 se COUPL = 0).
    float energy01(float coupling) const noexcept {
        if (std::fabs(coupling) < 1e-6f) return 0.5f;
        int bonds = 0;
        for (int y = 0; y < kSide; ++y)
            for (int x = 0; x < kSide; ++x)
                bonds += at(x, y) * (at(x + 1, y) + at(x, y + 1));
        const float e = -coupling * static_cast<float>(bonds) / static_cast<float>(kN);   // −2|J| … 2|J|
        return std::clamp(0.5f + e / (4.0f * std::fabs(coupling)), 0.0f, 1.0f);
    }
    // Os 64 ímãs em 3 palavras de 24 bits (para o mostrador).
    float word(int w) const noexcept {
        std::uint32_t v = 0;
        for (int b = 0; b < 24; ++b) {
            const int i = w * 24 + b;
            if (i < kN && spins_[static_cast<std::size_t>(i)] > 0) v |= 1U << b;
        }
        return static_cast<float>(v);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* clock = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* quench = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* tcv = inputs.size() > 2 ? inputs[2] : nullptr;
        const AudioBlock* fcv = inputs.size() > 3 ? inputs[3] : nullptr;

        const float tempK = std::clamp(safe(parameterValue("temp"), 2.3f), 0.1f, 6.0f);
        const float coupling = std::clamp(safe(parameterValue("coupling"), 1.0f), -1.0f, 1.0f);
        const float fieldK = std::clamp(safe(parameterValue("field"), 0.0f), -1.0f, 1.0f);
        const float sweep = std::clamp(safe(parameterValue("sweep"), 0.5f), 0.05f, 1.0f);
        const float rate = std::clamp(safe(parameterValue("rate"), 6.0f), 0.1f, 30.0f);
        const float gateLen = std::clamp(safe(parameterValue("gate_len"), 0.5f), 0.05f, 0.95f);
        const double dp = static_cast<double>(rate) / sr_;
        const int tries = std::max(1, static_cast<int>(sweep * static_cast<float>(kN)));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (quench) {
                const float q = safe(quench->at(0, f), 0.0f);
                if (prevQuench_ < 0.5f && q >= 0.5f) randomize();
                prevQuench_ = q;
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
                const float temp = std::clamp(tempK + (tcv ? 2.0f * safe(tcv->at(0, f), 0.0f) : 0.0f), 0.1f, 8.0f);
                const float field = std::clamp(fieldK + (fcv ? safe(fcv->at(0, f), 0.0f) : 0.0f), -2.0f, 2.0f);
                int flipped = 0;
                for (int t = 0; t < tries; ++t) {
                    const int i = static_cast<int>(unit() * static_cast<float>(kN)) % kN;
                    const int x = i % kSide, y = i / kSide;
                    const int s = at(x, y);
                    const int nb = at(x + 1, y) + at(x - 1, y) + at(x, y + 1) + at(x, y - 1);
                    const float dE = 2.0f * static_cast<float>(s) * (coupling * static_cast<float>(nb) + field);
                    if (dE <= 0.0f || unit() < std::exp(-dE / temp)) {
                        spins_[static_cast<std::size_t>(i)] = static_cast<std::int8_t>(-s);
                        ++flipped;
                    }
                }
                flips_ = static_cast<float>(flipped) / static_cast<float>(kN);
                head_ = (head_ + 1) % kN;
                mag_ = magnetization();
                energy_ = energy01(coupling);
                const float period = clock ? extPeriod_ : static_cast<float>(sr_ / rate);
                gateSamples_ = spins_[static_cast<std::size_t>(head_)] > 0 ? std::max(1, static_cast<int>(gateLen * period)) : 0;
            }
            const float gate = gateSamples_ > 0 ? 1.0f : 0.0f;
            if (gateSamples_ > 0) --gateSamples_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = gate;
                outputs[1].at(c, f) = mag_;
                outputs[2].at(c, f) = energy_;
                outputs[3].at(c, f) = flips_;
            }
        }
    }

private:
    int at(int x, int y) const noexcept {
        const int xx = ((x % kSide) + kSide) % kSide, yy = ((y % kSide) + kSide) % kSide;
        return spins_[static_cast<std::size_t>(yy * kSide + xx)];
    }
    void randomize() noexcept {
        for (auto& s : spins_) s = static_cast<std::int8_t>(unit() < 0.5f ? -1 : 1);
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x3A6E7B0072ULL;
    std::array<std::int8_t, kN> spins_{};
    double phase_ = 1.0;
    float prevClock_ = 0.0f, prevQuench_ = 0.0f, extPeriod_ = 9600.0f, sinceClock_ = 0.0f;
    int gateSamples_ = 0, head_ = 0;
    float flips_ = 0.0f, mag_ = 0.0f, energy_ = 0.5f;
};

} // namespace rasgo::modular
