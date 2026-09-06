#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// DRUM — voz de percussão (Módulo 47)
// ============================================================================
//
// O `MATTER`+`NOISE`+`ENVELOPE` já montam um bumbo à mão. `DRUM` empacota
// isso num gesto: um gate → um golpe.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/47_drum.md`.
//
// Três camadas: CORPO (senóide com envelope de altura — o "pow" do 808;
// `map` mistura com tanh → clique do 909), ESTALO (ruído por passa-alta
// com envelope curto — `snap`), ENVELOPE de amplitude (`decay`).
//
// - `tone` (20–1000 Hz, +CV v/oct)  altura do corpo
// - `bend` (0–1)   profundidade do envelope de altura (varredura)
// - `decay` (0–1)  decaimento geral (~20 ms a ~2 s)
// - `snap` (0–1)   dose do ruído de ataque
// - `map` (0–1)    caráter 808 → 909 → acústico
// - `drive` (0–1)  saturação de saída (tanh + makeup)
// - `roll` (0–1)   auto-disparo interno (0 = só gate; senão ~2–40 Hz)
// - `drift` (0–1)  humanização por golpe (xorshift semeado no disparo —
//                  determinístico dada a sequência de gates)
//
// `prepare()` não aloca; `process()` não aloca. Determinístico.

namespace rasgo::modular {

class Drum final : public Signal {
public:
    Drum()
        : Signal(
              {{"gate", PortKind::Control, "trig"},
               {"accent", PortKind::Control, ""},
               {"tone", PortKind::Control, "v/oct"}},
              {{"out", PortKind::Audio, ""}},
              {{"tone", 20.0f, 1000.0f, 55.0f, "Hz"},
               {"bend", 0.0f, 1.0f, 0.6f, ""},
               {"decay", 0.0f, 1.0f, 0.4f, ""},
               {"snap", 0.0f, 1.0f, 0.3f, ""},
               {"map", 0.0f, 1.0f, 0.0f, ""},
               {"drive", 0.0f, 1.0f, 0.0f, ""},
               {"roll", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "DRUM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "DRUM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "hit", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "BEND", "bend", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DECAY", "decay", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SNAP", "snap", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MAP", "map", 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 24.0f, 50.0f);
        p.add(Widget::Kind::Knob, "ROLL", "roll", 41.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 58.0f, 50.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "ACC", "in:accent", 24.0f, 92.0f);
        p.add(Widget::Kind::Jack, "PIT", "in:tone", 40.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        ampEnv_ = 0.0f;
        pitchEnv_ = 0.0f;
        noiseEnv_ = 0.0f;
        bodyPh_ = 0.0f;
        hp_ = 0.0f;
        wPrev_ = 0.0f;
        prevGate_ = 0.0f;
        rollCount_ = 0;
        rng_ = 0x9E3779B97F4A7C15ULL;
        hitPitch_ = 1.0f;
        hitDecay_ = 1.0f;
        hitLevel_ = 1.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* gateIn = inputs[0];
        const AudioBlock* accIn = inputs[1];
        const AudioBlock* toneIn = inputs[2];

        const float tone = clampf(parameterValue("tone"), 20.0f, 1000.0f);
        const float bend = clamp01(parameterValue("bend"));
        const float decay = clamp01(parameterValue("decay"));
        const float snap = clamp01(parameterValue("snap"));
        const float map = clamp01(parameterValue("map"));
        const float drive = clamp01(parameterValue("drive"));
        const float roll = clamp01(parameterValue("roll"));

        const int rollPeriod = roll > 0.0f
            ? static_cast<int>(sr_ / (2.0f + roll * 38.0f)) : 0;
        const float hpFc = 800.0f + map * 6000.0f;
        const float hpCoef = std::exp(-6.28318530718f * hpFc / sr_);
        const float driveG = 1.0f + drive * 4.0f;
        const float driveMk = 1.0f + drive * 1.5f;

        for (std::size_t f = 0; f < frames; ++f) {
            const float g = gateIn ? gateIn->at(0, f) : 0.0f;
            bool trig = (prevGate_ < 0.5f && g >= 0.5f);
            prevGate_ = g;
            if (rollPeriod > 0) {
                if (++rollCount_ >= rollPeriod) { rollCount_ = 0; trig = true; }
            } else {
                rollCount_ = 0;
            }

            if (trig) {
                ampEnv_ = 1.0f;
                pitchEnv_ = 1.0f;
                noiseEnv_ = 1.0f;
                bodyPh_ = 0.0f;
                const float drift = clamp01(parameterValue("drift"));
                hitPitch_ = 1.0f + drift * 0.06f * (rnd11());
                hitDecay_ = 1.0f + drift * 0.25f * (rnd11());
                const float acc = accIn ? clampf(accIn->at(0, f), 0.0f, 2.0f) : 1.0f;
                hitLevel_ = acc * (1.0f + drift * 0.20f * (rnd11()));
            }

            // envelopes
            const float decayT = 0.02f * std::pow(100.0f, decay * hitDecay_);
            ampEnv_ *= std::exp(-dt_ / decayT);
            pitchEnv_ *= std::exp(-dt_ / 0.035f);
            noiseEnv_ *= std::exp(-dt_ / (0.006f + decay * 0.04f));

            // corpo
            const float toneCv = toneIn ? toneIn->at(0, f) : 0.0f;
            const float f0 = tone * std::exp2(toneCv) * hitPitch_;
            const float fbody = clampf(f0 * (1.0f + bend * 6.0f * pitchEnv_),
                                       1.0f, 0.48f * sr_);
            bodyPh_ += fbody * dt_;
            bodyPh_ -= std::floor(bodyPh_);
            float raw = std::sin(6.28318530718f * bodyPh_);
            raw = (1.0f - map) * raw + map * std::tanh(raw * 3.0f);

            // estalo
            const float w = noise();
            hp_ = hpCoef * hp_ + w - wPrev_;   // passa-alta de 1 polo
            wPrev_ = w;
            const float snapS = hp_ * snap * noiseEnv_ * (0.6f + map * 0.5f);

            const float body = raw * ampEnv_ * (0.9f - map * 0.2f);
            float y = body + snapS;
            y = std::tanh(y * hitLevel_ * driveG) / driveMk;

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    std::uint64_t xn() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return rng_;
    }
    float noise() noexcept {
        return static_cast<float>(static_cast<std::int32_t>(xn() >> 32))
             / 2147483648.0f;
    }
    // ±1 uniforme, avança o mesmo stream (humanização — 1 saque por chamada)
    float rnd11() noexcept {
        return static_cast<float>(static_cast<std::int32_t>(xn() >> 33))
             / 1073741824.0f;
    }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float ampEnv_ = 0.0f, pitchEnv_ = 0.0f, noiseEnv_ = 0.0f;
    float bodyPh_ = 0.0f;
    float hp_ = 0.0f, wPrev_ = 0.0f;
    float prevGate_ = 0.0f;
    int rollCount_ = 0;
    std::uint64_t rng_ = 0x9E3779B97F4A7C15ULL;
    float hitPitch_ = 1.0f, hitDecay_ = 1.0f, hitLevel_ = 1.0f;
};

}  // namespace rasgo::modular
