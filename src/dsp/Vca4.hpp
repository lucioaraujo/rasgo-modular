#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <string>

// ============================================================================
// VCA4 — banco de 4 VCAs + mixer (Módulo 59)
// ============================================================================
//
// O `VCA` (#20) é DUPLO — voz + modulação. Um patch grande precisa de
// VCA em quantidade (Mutable Veils, Intellijel Quad VCA): quatro canais
// com CV, um `mix` somado na saída, curva linear/exponencial
// compartilhada. É a utilidade nº 1 do formato modular.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/59_vca4.md`.
//
// - `levelN`  (0–1)      ganho base do canal N (o knob que a CV soma).
// - `cvN_amt` (−1..1)    atenuversor da CV do canal N.
// - `curve`   (0–1)      0 = linear (bom pra somar CV), 1 = exponencial
//                        (dB-linear, bom pra volume percebido). Comum.
// - `mix_gain`(0–2)      ganho da saída `mix` (soma dos 4 canais).
// - `drift`   (0–1, desvio Rasgo)  wobble lento e SEMEADO nos 4 ganhos.
//
// Modulação por PORTA: `cvN` atenuvertida SOMA ao knob (o knob fica
// vivo). `process()` não aloca. `drift=0` → determinístico.

namespace rasgo::modular {

class Vca4 final : public Signal {
public:
    Vca4()
        : Signal(makeInputs(), makeOutputs(), makeParams()) {}

    std::string type() const override { return "VCA4"; }

    Panel panel() const override {
        Panel p;
        p.hp = 18;
        p.add(Widget::Kind::Label, "VCA4", "", 2.5f, 2.0f);
        for (int ch = 0; ch < 4; ++ch) {
            const std::string n = std::to_string(ch + 1);
            const float x = 8.0f + static_cast<float>(ch) * 18.0f;
            p.add(Widget::Kind::Knob, ("LVL" + n), "level" + n, x, 16.0f);
            p.add(Widget::Kind::Knob, ("CV" + n), "cv" + n + "_amt", x, 34.0f);
            p.add(Widget::Kind::Jack, ("IN" + n), "in:in" + n, x, 78.0f);
            p.add(Widget::Kind::Jack, ("C" + n), "in:cv" + n, x, 98.0f);
            p.add(Widget::Kind::Jack, ("O" + n), "out:out" + n, x, 118.0f);
        }
        p.add(Widget::Kind::Knob, "CURVE", "curve", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "MIXG", "mix_gain", 26.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 44.0f, 54.0f);
        p.add(Widget::Kind::Jack, "MIX", "out:mix", 80.0f, 118.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        for (int k = 0; k < 4; ++k)
            gSmooth_[k] = parameterValue("level" + std::to_string(k + 1));
        driftState_ = 0.0f;
        driftCounter_ = 0;
        rng_ = 0xC4A5EEDBAD5EED04ULL;
        smoothCoef_ =
            1.0f - std::exp(-1.0f / std::max(1.0f, 0.0015f * sampleRate));
        driftInterval_ =
            static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 40.0f));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        float lvl[4], cvAmt[4];
        for (int k = 0; k < 4; ++k) {
            const std::string n = std::to_string(k + 1);
            lvl[k] = parameterValue("level" + n);
            cvAmt[k] = parameterValue("cv" + n + "_amt");
        }
        const float curve = clampf(parameterValue("curve"), 0.0f, 1.0f);
        const float mixGain = clampf(parameterValue("mix_gain"), 0.0f, 2.0f);
        const float drift = clamp01(parameterValue("drift"));
        const float driftStep = 0.0004f * drift * drift;

        const AudioBlock* in[4] = {inputs[0], inputs[2], inputs[4], inputs[6]};
        const AudioBlock* cv[4] = {inputs[1], inputs[3], inputs[5], inputs[7]};

        for (std::size_t f = 0; f < frames; ++f) {
            if (++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftState_ += noise() * driftStep;
                driftState_ = clampf(driftState_, -0.03f, 0.03f);
            }
            const float driftMul = 1.0f + driftState_;

            float y[4];
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                float g = lvl[k];
                if (cv[k] != nullptr) g += cvAmt[k] * cv[k]->at(0, f);
                g = clampf(g, 0.0f, 1.0f);
                const float gShaped =
                    curve <= 0.0f ? g : std::pow(g, 1.0f + 3.0f * curve);
                gSmooth_[k] += (gShaped * driftMul - gSmooth_[k]) * smoothCoef_;
                const float x = in[k] != nullptr ? in[k]->at(0, f) : 0.0f;
                y[k] = softSat(x * gSmooth_[k]);
                sum += y[k];
            }
            const float m = softSat(sum * mixGain);

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = y[0];
                outputs[1].at(c, f) = y[1];
                outputs[2].at(c, f) = y[2];
                outputs[3].at(c, f) = y[3];
                outputs[4].at(c, f) = m;
            }
        }
    }

private:
    static std::vector<PortDescriptor> makeInputs() {
        std::vector<PortDescriptor> v;
        for (int k = 1; k <= 4; ++k) {
            v.push_back({"in" + std::to_string(k), PortKind::Audio, ""});
            v.push_back({"cv" + std::to_string(k), PortKind::Control, ""});
        }
        return v;
    }
    static std::vector<PortDescriptor> makeOutputs() {
        std::vector<PortDescriptor> v;
        for (int k = 1; k <= 4; ++k)
            v.push_back({"out" + std::to_string(k), PortKind::Audio, ""});
        v.push_back({"mix", PortKind::Audio, ""});
        return v;
    }
    static std::vector<ParameterDescriptor> makeParams() {
        std::vector<ParameterDescriptor> v;
        for (int k = 1; k <= 4; ++k) {
            v.push_back({"level" + std::to_string(k), 0.0f, 1.0f, 0.0f, ""});
            v.push_back({"cv" + std::to_string(k) + "_amt", -1.0f, 1.0f, 1.0f, ""});
        }
        v.push_back({"curve", 0.0f, 1.0f, 0.0f, ""});
        v.push_back({"mix_gain", 0.0f, 2.0f, 1.0f, ""});
        v.push_back({"drift", 0.0f, 1.0f, 0.0f, ""});
        return v;
    }

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    static float softSat(const float x) noexcept {
        if (x > 0.9f) return 0.9f + 0.1f * std::tanh((x - 0.9f) * 8.0f);
        if (x < -0.9f) return -0.9f + 0.1f * std::tanh((x + 0.9f) * 8.0f);
        return x;
    }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float gSmooth_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float driftState_ = 0.0f;
    float smoothCoef_ = 0.05f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 1200;
    std::uint64_t rng_ = 0xC4A5EEDBAD5EED04ULL;
};

}  // namespace rasgo::modular
