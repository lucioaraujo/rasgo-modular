#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// CRUSH — destruidor lo-fi / decimador (Módulo 53)
// ============================================================================
//
// O `wear` do RASGO está espalhado (SAMPLER/TURNTABLE/LOOPER); DAMAGE é o
// único verbo da árvore de 18 sem módulo. O CRUSH é o destruidor DIGITAL:
// redução de taxa (sem anti-alias — o aliasing É o som), redução de bits,
// transbordo (clip ↔ overflow que enrola), glitch (dropout / travada /
// repique) e jitter de clock. Tudo semeado — dano REPRODUTÍVEL.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/53_crush.md`.
//
// - `rate` (100–24 k Hz, +CV)  taxa do S&H interno
// - `bits` (1–16)              quantização a 2^bits níveis
// - `drive` (0–4×)             ganho antes da quantização (empurra pro wrap)
// - `wrap` (0–1)               0 clipa · 1 enrola (dente de serra brutal)
// - `glitch` (0–1)             probabilidade de falha por hold
// - `jitter` (0–1)             instabilidade da taxa (wow digital)
// - `tone` (−1..1)             filtro de 1 polo na saída (<0 LP, >0 HP)
// - `mix` (0–1, +CV)           seco ↔ destruído
//
// `in` livre + `mix=1` → S&H do próprio ruído = fonte lo-fi/glitch.
// `mix=0` → passa-direto. `process()` não aloca.

namespace rasgo::modular {

class Crush final : public Signal {
public:
    Crush()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"rate_mod", PortKind::Control, ""},
               {"mix_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"rate", 100.0f, 24000.0f, 6000.0f, "Hz"},
               {"bits", 1.0f, 16.0f, 12.0f, ""},
               {"drive", 0.0f, 4.0f, 1.0f, ""},
               {"wrap", 0.0f, 1.0f, 0.0f, ""},
               {"glitch", 0.0f, 1.0f, 0.0f, ""},
               {"jitter", 0.0f, 1.0f, 0.0f, ""},
               {"tone", -1.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "CRUSH"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "CRUSH", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "crush", "", 2.5f, 6.0f, 45.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "BITS", "bits", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "WRAP", "wrap", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "GLTCH", "glitch", 24.0f, 54.0f);
        p.add(Widget::Kind::Knob, "JITR", "jitter", 40.0f, 54.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 8.0f, 78.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 24.0f, 78.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RTM", "in:rate_mod", 20.0f, 100.0f);
        p.add(Widget::Kind::Jack, "MXM", "in:mix_mod", 32.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        holdPh_ = 1.0f;      // força um hold no 1º sample
        held_ = 0.0f;
        toneZ_ = 0.0f;
        jitterLfo_ = 0.0f;
        rng_ = 0xC205ADEC1A1C0FFEULL;
        toneCoef_ = 1.0f - std::exp(-2.0f * 3.14159265f * 1600.0f / sr_);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* rmB = inputs[1];
        const AudioBlock* mmB = inputs[2];

        const int bits = static_cast<int>(std::lround(
            clampf(parameterValue("bits"), 1.0f, 16.0f)));
        const float drive = clampf(parameterValue("drive"), 0.0f, 4.0f);
        const float wrap = clamp01(parameterValue("wrap"));
        const float glitch = clamp01(parameterValue("glitch"));
        const float jitter = clamp01(parameterValue("jitter"));
        const float tone = clampf(parameterValue("tone"), -1.0f, 1.0f);
        const float mixP = clamp01(parameterValue("mix"));
        const float qL = std::pow(2.0f,
                                  static_cast<float>(std::max(1, bits - 1)));

        for (std::size_t f = 0; f < frames; ++f) {
            const float in = inB ? inB->at(0, f) : 0.0f;
            const float rate = clampf(
                parameterValue("rate") + (rmB ? rmB->at(0, f) : 0.0f),
                20.0f, sr_ * 0.5f);
            const float mix = clamp01(mixP + (mmB ? mmB->at(0, f) : 0.0f));

            // ---- clock do S&H (com jitter — passeio lento = "wow" digital) ----
            jitterLfo_ += (rnd01() - 0.5f) * 0.04f;
            jitterLfo_ = clampf(jitterLfo_, -1.0f, 1.0f);
            const float step = (rate / sr_)
                * (1.0f + jitter * jitterLfo_ * 0.9f);
            holdPh_ += step;
            if (holdPh_ >= 1.0f) {
                holdPh_ -= std::floor(holdPh_);
                const float src =
                    (inB ? in : (rnd01() * 2.0f - 1.0f)) * drive;
                if (glitch > 0.0f && rnd01() < glitch) {
                    const float sub = rnd01();
                    held_ = sub < 0.4f ? held_
                          : sub < 0.7f ? 0.0f
                          : clampf(held_ * 1.8f, -1.0f, 1.0f);
                } else {
                    held_ = src;
                }
            }

            // ---- quantização ----
            float q = std::round(held_ * qL) / qL;

            // ---- transbordo: clip ↔ enrola ----
            const float qc = clampf(q, -1.0f, 1.0f);
            const float qw = q - 2.0f * std::round(q * 0.5f);   // saw wrap
            float o = qc + (qw - qc) * wrap;

            // ---- filtro de tom ----
            toneZ_ += (o - toneZ_) * toneCoef_;
            if (tone < 0.0f) o = o + (toneZ_ - o) * (-tone);
            else             o = o + ((o - toneZ_) - o) * tone;

            float y = in + (o - in) * mix;
            y = clampf(y, -4.0f, 4.0f);
            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rnd01() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(rng_ >> 40) / 16777216.0f;
    }

    float holdPh_ = 1.0f;
    float held_ = 0.0f;
    float toneZ_ = 0.0f;
    float jitterLfo_ = 0.0f;
    float toneCoef_ = 0.2f;
    std::uint64_t rng_ = 0xC205ADEC1A1C0FFEULL;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
