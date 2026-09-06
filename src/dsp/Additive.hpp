#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// ADDITIVE — oscilador aditivo / espectral (Módulo 42)
// ============================================================================
//
// O `OSC` é subtrativo, o `WAVETABLE` varre uma forma. `ADDITIVE` constrói
// o timbre SOMANDO 64 parciais, cada um com frequência e amplitude sob
// controle direto — o oposto do subtrativo.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/42_additive.md`.
//
// Envelope espectral por quatro knobs:
//   `tilt`    (0–1)    brilho — inclinação do espectro (k^-e, e de 2,6 a 0,15)
//   `odd`     (−1..1)  balanço ímpar/par (+1 só ímpares, −1 só pares)
//   `stretch` (−1..1)  inarmonicidade — razão_k = k + stretch·c·k·(k−1)
//   `comb`    (0–1)    pente espectral (cos sobre o índice de parcial)
//
// `drift` (0–1, desvio Rasgo) = cintilância determinística: micro-desafino
// + respiração de amplitude de senóides lentas incomensuráveis. Sem RNG —
// reprodutível. `drift=0` remove o termo.
//
// Afinação padrão do `OSC`: `freq`/`fine`/`pitch` (1 V/oct)/`fm` (linear).
// Saída limitada por seguidor de ganho + `tanh` (a soma aditiva é pontuda).
//
// `prepare()` aloca ~8 KB (LUT + 3 vetores de 64); `process()` não aloca.

namespace rasgo::modular {

class Additive final : public Signal {
public:
    static constexpr int kPartials = 64;
    static constexpr int kLut = 2048;

    Additive()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"tilt", PortKind::Control, ""},
               {"stretch", PortKind::Control, ""},
               {"fm", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 8.0f, 8000.0f, 110.0f, "Hz"},
               {"fine", -100.0f, 100.0f, 0.0f, "cent"},
               {"tilt", 0.0f, 1.0f, 0.5f, ""},
               {"odd", -1.0f, 1.0f, 0.0f, ""},
               {"stretch", -1.0f, 1.0f, 0.0f, ""},
               {"comb", 0.0f, 1.0f, 0.0f, ""},
               {"fm_amount", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "ADDITIVE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "ADDITIVE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "spec", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FINE", "fine", 22.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TILT", "tilt", 37.0f, 30.0f);
        p.add(Widget::Kind::Knob, "COMB", "comb", 52.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ODD", "odd", 7.0f, 54.0f);
        p.add(Widget::Kind::Knob, "STRCH", "stretch", 22.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 37.0f, 54.0f);
        p.add(Widget::Kind::Knob, "FM", "fm_amount", 52.0f, 54.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:pitch", 8.0f, 94.0f);
        p.add(Widget::Kind::Jack, "TILT", "in:tilt", 24.0f, 94.0f);
        p.add(Widget::Kind::Jack, "STRCH", "in:stretch", 42.0f, 94.0f);
        p.add(Widget::Kind::Jack, "FM", "in:fm", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 24.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;

        lut_.assign(kLut, 0.0f);
        for (int i = 0; i < kLut; ++i)
            lut_[static_cast<std::size_t>(i)] =
                std::sin(6.28318530718f * static_cast<float>(i)
                         / static_cast<float>(kLut));

        amp_.assign(kPartials, 0.0f);
        det_.assign(kPartials, 0.0f);
        phase_.assign(kPartials, 0.0f);

        outNorm_ = 0.1f;
        gainFollow_ = 1.0f;
        driftA_ = 0.0f;
        driftB_ = 0.0f;
        tiltS_ = clamp01(parameterValue("tilt"));
        oddS_ = clampf(parameterValue("odd"), -1.0f, 1.0f);
        stretchS_ = clampf(parameterValue("stretch"), -1.0f, 1.0f);
        combS_ = clamp01(parameterValue("comb"));
        recompute();
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* pitchIn = inputs[0];
        const AudioBlock* tiltIn = inputs[1];
        const AudioBlock* stretchIn = inputs[2];
        const AudioBlock* fmIn = inputs[3];

        const float freq = parameterValue("freq");
        const float fineOct = parameterValue("fine") / 1200.0f;
        const float fmAmt = clamp01(parameterValue("fm_amount"));
        const float nyq = 0.5f * sr_;
        const float nyqCut = 0.98f * nyq;
        const float nyqFade = 0.83f * nyq;

        // --- recomputa o banco uma vez por bloco (params mudam devagar) ---
        const float tiltT = clamp01(clamp01(parameterValue("tilt"))
                                    + (tiltIn ? tiltIn->at(0, 0) : 0.0f));
        const float stretchT = clampf(clampf(parameterValue("stretch"), -1.0f, 1.0f)
                                      + (stretchIn ? stretchIn->at(0, 0) : 0.0f),
                                      -1.0f, 1.0f);
        const float oddT = clampf(parameterValue("odd"), -1.0f, 1.0f);
        const float combT = clamp01(parameterValue("comb"));
        tiltS_ += (tiltT - tiltS_) * 0.3f;
        oddS_ += (oddT - oddS_) * 0.3f;
        stretchS_ += (stretchT - stretchS_) * 0.3f;
        combS_ += (combT - combS_) * 0.3f;
        drift_ = clamp01(parameterValue("drift"));
        recompute();
        driftA_ += 0.05f * static_cast<float>(frames) * dt_;
        driftB_ += 0.035f * static_cast<float>(frames) * dt_;
        driftA_ -= std::floor(driftA_);
        driftB_ -= std::floor(driftB_);

        float blockPeak = 1.0e-6f;
        for (std::size_t f = 0; f < frames; ++f) {
            const float pOct = pitchIn ? pitchIn->at(0, f) : 0.0f;
            float f0 = freq * std::exp2(fineOct + pOct);
            if (fmIn) f0 *= (1.0f + fmIn->at(0, f) * fmAmt * 4.0f);
            f0 = clampf(f0, 0.01f, nyqCut);

            float acc = 0.0f;
            for (int k = 0; k < kPartials; ++k) {
                const float fk = f0 * ratio_[static_cast<std::size_t>(k)]
                               * (1.0f + det_[static_cast<std::size_t>(k)]);
                if (fk >= nyqCut) break;
                float ph = phase_[static_cast<std::size_t>(k)] + fk * dt_;
                ph -= std::floor(ph);
                phase_[static_cast<std::size_t>(k)] = ph;

                float g = amp_[static_cast<std::size_t>(k)];
                if (fk > nyqFade)
                    g *= clamp01((nyqCut - fk) / (nyqCut - nyqFade));
                acc += g * lut(ph);
            }

            float y = std::tanh(acc * outNorm_ * gainFollow_);
            const float ay = std::fabs(y);
            if (ay > blockPeak) blockPeak = ay;
            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }

        const float target = std::min(1.0f, 0.9f / blockPeak);
        gainFollow_ += (target - gainFollow_)
                     * (target < gainFollow_ ? 0.5f : 0.02f);
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float lut(const float ph) const noexcept {
        const float x = ph * static_cast<float>(kLut);
        int i = static_cast<int>(x);
        const float fr = x - static_cast<float>(i);
        i &= (kLut - 1);
        const int j = (i + 1) & (kLut - 1);
        return lut_[static_cast<std::size_t>(i)] * (1.0f - fr)
             + lut_[static_cast<std::size_t>(j)] * fr;
    }

    // recomputa razão_[k], amp_[k], det_[k] a partir do estado suavizado
    void recompute() noexcept {
        const float e = 2.6f - 2.45f * tiltS_;
        const float teeth = 1.0f + 11.0f * combS_;
        const float driftA01 = 6.28318530718f * driftA_;
        const float driftB01 = 6.28318530718f * driftB_;
        float sumAmp = 0.0f;
        for (int k = 1; k <= kPartials; ++k) {
            const float kf = static_cast<float>(k);
            ratio_[static_cast<std::size_t>(k - 1)] =
                kf + stretchS_ * 0.004f * kf * (kf - 1.0f);

            float a = std::pow(kf, -e);
            a *= (k & 1) ? (1.0f + oddS_) : (1.0f - oddS_);
            if (a < 0.0f) a = 0.0f;
            a *= (1.0f - combS_)
               + combS_ * (0.5f + 0.5f * std::cos(6.28318530718f * teeth
                                                  * kf / static_cast<float>(kPartials)));
            a *= 1.0f + 0.12f * drift_ * std::sin(driftA01 + kf * 2.3999f);
            amp_[static_cast<std::size_t>(k - 1)] = a;
            sumAmp += std::fabs(a);

            det_[static_cast<std::size_t>(k - 1)] =
                0.006f * drift_ * std::sin(driftB01 + kf * 1.111f);
        }
        outNorm_ = 0.9f / std::max(sumAmp, 1.0e-4f);
    }

    std::vector<float> lut_;
    std::vector<float> amp_;
    std::vector<float> det_;
    std::vector<float> phase_;
    float ratio_[kPartials] = {};

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float outNorm_ = 0.1f;
    float gainFollow_ = 1.0f;
    float driftA_ = 0.0f, driftB_ = 0.0f, drift_ = 0.0f;
    float tiltS_ = 0.5f, oddS_ = 0.0f, stretchS_ = 0.0f, combS_ = 0.0f;
};

}  // namespace rasgo::modular
