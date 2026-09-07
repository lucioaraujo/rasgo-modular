#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// PULSAR — síntese pulsar (Módulo 56)
// ============================================================================
//
// Curtis Roads: um trem de PULSARETS — um grão curto (o pulsaret) seguido
// de silêncio, com período total `p`. Duas frequências INDEPENDENTES:
// `freq` = 1/p = a altura; `formant` = 1/d = a frequência interna do
// pulsaret, INDEPENDENTE da altura. Abaixar `formant` deixa o som mais
// oco/formântico SEM desafinar. Entre a granular e a de formante.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/56_pulsar.md`.
//
// - `freq` (20–2000 Hz, +CV 1V/oct)  taxa de repetição = a altura
// - `formant` (0,1–8×, +CV)          freq interna do pulsaret / a fundamental
// - `shape` (0–1)                    1 ciclo de seno → 2–3 → pulso estreito
// - `window` (0–1)                   retangular → Hann → expodec (percussivo)
// - `jitter` (0–1, desvio Rasgo)     jitter SEMEADO no período/amplitude
// - `mask` (0–1)                     probabilidade de PULAR um pulsaret (Roads)
// - `spread` (0–1)                   pulsarets alternados L/R + desafino
// - `level` (0–1)                    saída
//
// Fonte — soa ao carregar. `jitter=mask=0` → trem periódico determinístico.
// Pool de 4 vozes de grão. `process()` não aloca.

namespace rasgo::modular {

class Pulsar final : public Signal {
public:
    Pulsar()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"formant_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""},
               {"r", PortKind::Audio, ""}},
              {{"freq", 20.0f, 2000.0f, 110.0f, "Hz"},
               {"formant", 0.1f, 8.0f, 1.0f, ""},
               {"shape", 0.0f, 1.0f, 0.0f, ""},
               {"window", 0.0f, 1.0f, 0.5f, ""},
               {"jitter", 0.0f, 1.0f, 0.0f, ""},
               {"mask", 0.0f, 1.0f, 0.0f, ""},
               {"spread", 0.0f, 1.0f, 0.0f, ""},
               {"level", 0.0f, 1.0f, 0.7f, ""}}) {}

    std::string type() const override { return "PULSAR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "PULSAR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "pulsar", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FRMT", "formant", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SHAPE", "shape", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "WIND", "window", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "JITR", "jitter", 24.0f, 54.0f);
        p.add(Widget::Kind::Knob, "MASK", "mask", 40.0f, 54.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 8.0f, 78.0f);
        p.add(Widget::Kind::Knob, "LEVEL", "level", 24.0f, 78.0f);
        p.add(Widget::Kind::Jack, "PIT", "in:pitch", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "FQM", "in:formant_mod", 22.0f, 100.0f);
        p.add(Widget::Kind::Jack, "L", "out:out", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 22.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        phase_ = 1.0;   // dispara um pulsaret no 1º sample
        nextScale_ = 1.0f;
        idx_ = 0;
        rng_ = 0x9017A5A1C0FFEEEDULL;
        for (auto& v : voices_) v = Voice{};
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& outL = outputs[0];
        AudioBlock& outR = outputs[1];
        const std::size_t frames = outL.frames();

        const AudioBlock* pitchB = inputs[0];
        const AudioBlock* fmB = inputs[1];

        const float freqP = clampf(parameterValue("freq"), 20.0f, 2000.0f);
        const float formantP = clampf(parameterValue("formant"), 0.1f, 8.0f);
        const float shape = clamp01(parameterValue("shape"));
        const float window = clamp01(parameterValue("window"));
        const float jitter = clamp01(parameterValue("jitter"));
        const float mask = clamp01(parameterValue("mask"));
        const float spread = clamp01(parameterValue("spread"));
        const float level = clamp01(parameterValue("level"));

        for (std::size_t f = 0; f < frames; ++f) {
            const float pitchCv = pitchB ? pitchB->at(0, f) : 0.0f;
            const float fmCv = fmB ? fmB->at(0, f) : 0.0f;
            const float f0 = clampf(freqP * std::pow(2.0f, pitchCv),
                                    5.0f, sr_ * 0.5f);

            phase_ += static_cast<double>(f0) / sr_ / nextScale_;
            if (phase_ >= 1.0) {
                phase_ -= std::floor(phase_);
                nextScale_ = jitter > 0.0f
                    ? 1.0f + (rnd01() - 0.5f) * jitter * 0.9f : 1.0f;
                const float ampJit = jitter > 0.0f
                    ? 1.0f - rnd01() * jitter * 0.6f : 1.0f;
                if (mask <= 0.0f || rnd01() >= mask) {
                    Voice& v = voices_[static_cast<std::size_t>(idx_) % 4];
                    const float formHz = clampf(
                        f0 * formantP * std::pow(2.0f, fmCv), 5.0f,
                        sr_ * 0.45f);
                    v.active = true;
                    v.pos = 0.0f;
                    v.dur = sr_ / formHz;
                    v.amp = ampJit;
                    v.pan = spread > 0.0f
                        ? ((idx_ & 1) ? spread : -spread) : 0.0f;
                    ++idx_;
                }
            }

            float oL = 0.0f, oR = 0.0f;
            for (Voice& v : voices_) {
                if (!v.active) continue;
                const float t = v.pos / v.dur;
                if (t >= 1.0f) { v.active = false; continue; }
                const float cyc = 1.0f + shape * 2.0f;
                float car = std::sin(6.2831853f * t * cyc)
                          + shape * 0.3f * std::sin(18.849556f * t * cyc);
                const float win = windowFn(t, window);
                const float s = car * win * v.amp;
                oL += s * 0.5f * (1.0f - v.pan);
                oR += s * 0.5f * (1.0f + v.pan);
                v.pos += 1.0f;
            }
            oL = std::tanh(oL * 1.4f) * level;
            oR = std::tanh(oR * 1.4f) * level;
            for (std::size_t c = 0; c < outL.channels(); ++c) outL.at(c, f) = oL;
            for (std::size_t c = 0; c < outR.channels(); ++c) outR.at(c, f) = oR;
        }
    }

private:
    struct Voice {
        bool active = false;
        float pos = 0.0f, dur = 1.0f, amp = 1.0f, pan = 0.0f;
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rnd01() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(rng_ >> 40) / 16777216.0f;
    }

    // w<0,4: janela de Tukey — platô central + bordas raised-cosine curtas
    //   (SEMPRE 0 nas pontas). w=0 → quase retangular (bordas ~1%, bem
    //   brilhante); w→0,4 → Hann puro. w≥0,4: Hann → expodec (ataque
    //   rápido + cauda exponencial — grão percussivo).
    static float windowFn(const float t, const float w) noexcept {
        if (w < 0.4f) {
            const float flat = 1.0f - w / 0.4f;
            const float edge = 0.5f - 0.49f * flat;
            if (t < edge)
                return 0.5f - 0.5f * std::cos(3.14159265f * t / edge);
            if (t > 1.0f - edge)
                return 0.5f - 0.5f * std::cos(3.14159265f * (1.0f - t) / edge);
            return 1.0f;
        }
        const float hann = 0.5f - 0.5f * std::cos(6.2831853f * t);
        const float expo = (1.0f - std::exp(-40.0f * t)) * std::exp(-4.0f * t)
                         * (1.0f - t);
        return hann + (expo - hann) * ((w - 0.4f) / 0.6f);
    }

    double phase_ = 1.0;
    float nextScale_ = 1.0f;
    long idx_ = 0;
    std::uint64_t rng_ = 0x9017A5A1C0FFEEEDULL;
    Voice voices_[4];
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
