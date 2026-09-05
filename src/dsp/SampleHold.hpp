#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// SH — Sample & Hold duplo (Módulo 23)
// ============================================================================
//
// O `NOISE` tem UMA saída `sh`. Este é o S&H DUPLO com canais
// CORRELACIONÁVEIS — a dupla que gera uma altura e um timbre relacionados
// (Marbles `X`). Cada canal segura `inN` (se conectado) ou o próprio acaso
// interno, no pulso de `trigN` ou do relógio interno em `rate`.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/23_sample_hold.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - S&H clássico (Buchla 265/266, Doepfer A-148);
//   - Buchla 266 smooth random (glide até o alvo — `slew`);
//   - Mutable Marbles `X`/spread (dois canais com correlação ajustável);
//   - track & hold; `shape` do `DECISION` (uniforme→sino).

namespace rasgo::modular {

class SampleHold final : public Signal {
public:
    SampleHold()
        : Signal(
              {{"in1", PortKind::Control, ""},
               {"trig1", PortKind::Control, "trig"},
               {"in2", PortKind::Control, ""},
               {"trig2", PortKind::Control, "trig"}},
              {{"out1", PortKind::Control, ""},
               {"out2", PortKind::Control, ""}},
              {{"rate", 0.02f, 40.0f, 4.0f, "Hz"},
               {"slew1", 0.0f, 1.0f, 0.0f, ""},
               {"slew2", 0.0f, 1.0f, 0.0f, ""},
               {"slope", -1.0f, 1.0f, 0.0f, ""},
               {"track1", 0.0f, 1.0f, 0.0f, ""},
               {"track2", 0.0f, 1.0f, 0.0f, ""},
               {"spread", 0.0f, 1.0f, 0.0f, ""},
               {"correlation", -1.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "SH"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "SH", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "hold", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 8.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 26.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SLOPE", "slope", 44.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SLW1", "slew1", 8.0f, 48.0f);
        p.add(Widget::Kind::Knob, "SLW2", "slew2", 26.0f, 48.0f);
        p.add(Widget::Kind::Knob, "CORR", "correlation", 44.0f, 48.0f);
        p.add(Widget::Kind::Toggle, "TRK1", "track1", 8.0f, 68.0f);
        p.add(Widget::Kind::Toggle, "TRK2", "track2", 26.0f, 68.0f);
        p.add(Widget::Kind::Jack, "IN1", "in:in1", 7.0f, 92.0f);
        p.add(Widget::Kind::Jack, "T1", "in:trig1", 19.0f, 92.0f);
        p.add(Widget::Kind::Jack, "IN2", "in:in2", 34.0f, 92.0f);
        p.add(Widget::Kind::Jack, "T2", "in:trig2", 46.0f, 92.0f);
        p.add(Widget::Kind::Jack, "O1", "out:out1", 16.0f, 112.0f);
        p.add(Widget::Kind::Jack, "O2", "out:out2", 40.0f, 112.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        held_[0] = held_[1] = 0.0f;
        y_[0] = y_[1] = 0.0f;
        intPhase_[0] = intPhase_[1] = 0.0f;
        prevTrig_[0] = prevTrig_[1] = false;
        rng_[0] = 0x9E3779B97F4A7C15ULL;
        rng_[1] = 0xD1B54A32D192ED03ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const float rate = std::max(0.001f, parameterValue("rate"));
        const float intInc = rate / sr_;
        const float spread = clamp01(parameterValue("spread"));
        const float corr = clampf(parameterValue("correlation"), -1.0f, 1.0f);
        const float slew[2] = {clamp01(parameterValue("slew1")),
                               clamp01(parameterValue("slew2"))};
        // `slope` (−1..1) troca o tempo de SUBIDA pelo de DESCIDA: >0 =
        // desliza pra baixo devagar, sobe rápido (portamento de pluck);
        // <0 = o oposto. 0 → simétrico (idêntico ao comportamento antigo).
        const float slope = clampf(parameterValue("slope"), -1.0f, 1.0f);
        const bool track[2] = {parameterValue("track1") >= 0.5f,
                               parameterValue("track2") >= 0.5f};

        float coefUp[2], coefDn[2];
        for (int k = 0; k < 2; ++k) {
            const float base = slew[k] * slew[k] * 2.0f;
            const float up = base * (1.0f + slope);
            const float dn = base * (1.0f - slope);
            coefUp[k] = up < dt_ ? 1.0f : (1.0f - std::exp(-dt_ / up));
            coefDn[k] = dn < dt_ ? 1.0f : (1.0f - std::exp(-dt_ / dn));
        }

        const AudioBlock* in[2] = {inputs[0], inputs[2]};
        const AudioBlock* trg[2] = {inputs[1], inputs[3]};

        for (std::size_t f = 0; f < frames; ++f) {
            // acaso interno dos dois canais (só usado se `in` livre)
            const float u1 = shapedDraw(rng_[0], spread);
            const float u2i = shapedDraw(rng_[1], spread);
            const float u2 = corr >= 0.0f
                ? u2i + (u1 - u2i) * corr
                : u2i + (-u1 - u2i) * (-corr);
            const float uk[2] = {u1, u2};

            for (int k = 0; k < 2; ++k) {
                bool edge = false;
                const bool haveTrg = trg[k] != nullptr;
                if (haveTrg) {
                    const bool hi = trg[k]->at(0, f) >= 0.5f;
                    edge = hi && !prevTrig_[k];
                    prevTrig_[k] = hi;
                    if (track[k] && hi) {
                        held_[k] = in[k] != nullptr ? in[k]->at(0, f) : uk[k];
                    }
                } else {
                    intPhase_[k] += intInc;
                    if (intPhase_[k] >= 1.0f) { intPhase_[k] -= 1.0f; edge = true; }
                }
                if (edge)
                    held_[k] = in[k] != nullptr ? in[k]->at(0, f) : uk[k];

                const float diff = held_[k] - y_[k];
                y_[k] += diff * (diff >= 0.0f ? coefUp[k] : coefDn[k]);
            }

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = y_[0];
                outputs[1].at(c, f) = y_[1];
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    static float uniform(std::uint64_t& s) noexcept {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }
    // uniforme (spread 0) -> sino (spread 1, média de 4) — padrão `shape`
    static float shapedDraw(std::uint64_t& s, const float spread) noexcept {
        const float u = uniform(s);
        if (spread <= 0.0f) return u;
        const float m = 0.25f * (u + uniform(s) + uniform(s) + uniform(s));
        return u + (m - u) * spread;
    }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float held_[2] = {0.0f, 0.0f};
    float y_[2] = {0.0f, 0.0f};
    float intPhase_[2] = {0.0f, 0.0f};
    bool prevTrig_[2] = {false, false};
    std::uint64_t rng_[2] = {0x9E3779B97F4A7C15ULL, 0xD1B54A32D192ED03ULL};
};

}  // namespace rasgo::modular
