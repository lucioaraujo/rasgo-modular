#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SWIRL — efeitos de modulação: chorus / flanger / ensemble / phaser (Módulo 52)
// ============================================================================
//
// O RASGO tem reverb (SPACE, HALL), delay de linha (LOOPER), granular
// (MEMORY), fold (SHAPE, WASP) — mas NENHUM efeito de modulação. O SWIRL
// é a família inteira num módulo: atrasos CURTOS modulados (chorus,
// flanger, ensemble) + a cascata all-pass (phaser), com caráter BBD.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/52_swirl.md`.
//
// - `type` (0 chorus · 1 flanger · 2 ensemble · 3 phaser)
// - `rate` (0,02–8 Hz, +CV)  velocidade do LFO triangular
// - `depth` (0–1)            profundidade da modulação
// - `feedback` (−1..1)       ressonância (flanger/phaser); `tanh` no laço
// - `spread` (0–1)           largura estéreo (LFO de R defasado) + desafino
// - `tone` (−1..1)           filtro de 1 polo no molhado (<0 LP, >0 HP)
// - `age` (0–1, desvio Rasgo) caráter BBD — companding + ruído semeado + wobble
// - `mix` (0–1, +CV)         seco ↔ molhado
//
// `mix=0` → passa-direto. `age=0` → determinístico puro. Buffer ~50 ms/
// canal pré-alocado; `process()` não aloca.

namespace rasgo::modular {

class Swirl final : public Signal {
public:
    Swirl()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"rate_mod", PortKind::Control, ""},
               {"mix_mod", PortKind::Control, ""}},
              {{"l", PortKind::Audio, ""},
               {"r", PortKind::Audio, ""}},
              {{"type", 0.0f, 3.0f, 0.0f, ""},
               {"rate", 0.02f, 8.0f, 0.4f, "Hz"},
               {"depth", 0.0f, 1.0f, 0.5f, ""},
               {"feedback", -1.0f, 1.0f, 0.0f, ""},
               {"spread", 0.0f, 1.0f, 0.5f, ""},
               {"tone", -1.0f, 1.0f, 0.0f, ""},
               {"age", 0.0f, 1.0f, 0.15f, ""},
               {"mix", 0.0f, 1.0f, 0.4f, ""}}) {}

    std::string type() const override { return "SWIRL"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "SWIRL", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "swirl", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "TYPE", "type", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DEPTH", "depth", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 24.0f, 54.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 40.0f, 54.0f);
        p.add(Widget::Kind::Knob, "AGE", "age", 8.0f, 78.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 24.0f, 78.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "RTM", "in:rate_mod", 21.0f, 100.0f);
        p.add(Widget::Kind::Jack, "MXM", "in:mix_mod", 34.0f, 100.0f);
        p.add(Widget::Kind::Jack, "L", "out:l", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 21.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        len_ = static_cast<std::size_t>(0.055f * sr_) + 4;
        for (int c = 0; c < 2; ++c) {
            buf_[c].assign(len_, 0.0f);
            w_[c] = 0;
            fbState_[c] = 0.0f;
            toneZ_[c] = 0.0f;
            for (int k = 0; k < kStages; ++k) apS_[c][k] = 0.0f;
        }
        phL_ = 0.0f;
        wobCnt_ = 0;
        wob_ = 0.0f;
        rng_ = 0x5117E7E1D5A1C0FFULL;
        toneCoef_ = 1.0f - std::exp(-2.0f * 3.14159265f * 1500.0f / sr_);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& outL = outputs[0];
        AudioBlock& outR = outputs[1];
        const std::size_t frames = outL.frames();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* rmB = inputs[1];
        const AudioBlock* mmB = inputs[2];

        const int type = static_cast<int>(std::lround(
            clampf(parameterValue("type"), 0.0f, 3.0f)));
        const float depth = clamp01(parameterValue("depth"));
        const float feedback = clampf(parameterValue("feedback"), -1.0f, 1.0f);
        const float spread = clamp01(parameterValue("spread"));
        const float tone = clampf(parameterValue("tone"), -1.0f, 1.0f);
        const float age = clamp01(parameterValue("age"));
        const float mixP = clamp01(parameterValue("mix"));

        // parâmetros por tipo: atraso base (ms), faixa de modulação (ms),
        // nº de vozes, escala da realimentação
        static constexpr float kBaseMs[4] = {12.0f, 1.2f, 11.0f, 0.0f};
        static constexpr float kModMs[4]  = {4.0f, 5.5f, 3.0f, 0.0f};
        static constexpr int   kVoices[4] = {2, 1, 3, 1};
        // flanger: escala > 1 no talo → auto-oscila (regeneração passando
        // da unidade, como num flanger de verdade); o tanh no laço limita.
        static constexpr float kFbAmt[4]  = {0.30f, 1.08f, 0.20f, 0.70f};
        const int voices = kVoices[type];
        const float fbAmt = feedback * kFbAmt[type];

        for (std::size_t f = 0; f < frames; ++f) {
            const float in = inB ? inB->at(0, f) : 0.0f;
            const float rate = clampf(
                parameterValue("rate") + (rmB ? rmB->at(0, f) : 0.0f),
                0.01f, 20.0f);
            const float mix = clamp01(mixP + (mmB ? mmB->at(0, f) : 0.0f));

            // fase do LFO (o triângulo é derivado dela por canal/voz)
            phL_ += rate / sr_;
            phL_ -= std::floor(phL_);
            const float phR = phL_ + spread * 0.25f;

            // wobble de BBD (raro, ∝ age)
            if (age > 0.0f && ++wobCnt_ >= 512) {
                wobCnt_ = 0;
                wob_ = (rnd01() - 0.5f) * age * 0.4f;
            }

            float wetL, wetR;
            if (type == 3) {
                wetL = phaser(0, in, feedback, depth, tri(phL_), tone);
                wetR = phaser(1, in, feedback, depth, tri(phR), tone);
            } else {
                wetL = modLine(0, in, fbAmt, age, depth, phL_,
                               kBaseMs[type], kModMs[type], voices, tone);
                wetR = modLine(1, in, fbAmt, age, depth, phR,
                               kBaseMs[type], kModMs[type], voices, tone);
            }

            float oL = in + (wetL - in) * mix;
            float oR = in + (wetR - in) * mix;
            oL = clampf(oL, -4.0f, 4.0f);
            oR = clampf(oR, -4.0f, 4.0f);
            for (std::size_t c = 0; c < outL.channels(); ++c) outL.at(c, f) = oL;
            for (std::size_t c = 0; c < outR.channels(); ++c) outR.at(c, f) = oR;
        }
    }

private:
    static constexpr int kStages = 6;

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    static float tri(float ph) noexcept {          // triângulo em [-1,1]
        ph -= std::floor(ph);
        return 2.0f * std::fabs(2.0f * ph - 1.0f) - 1.0f;
    }

    std::uint64_t xn() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return rng_;
    }
    float rnd01() noexcept {
        return static_cast<float>(xn() >> 40) / 16777216.0f;
    }

    // companding BBD: comprime a dinâmica antes da linha, expande depois
    static float compress(const float x, const float age) noexcept {
        if (age <= 1.0e-4f) return x;
        const float e = 1.0f - 0.25f * age;
        return x < 0.0f ? -std::pow(-x, e) : std::pow(x, e);
    }
    static float expand(const float x, const float age) noexcept {
        if (age <= 1.0e-4f) return x;
        const float e = 1.0f / (1.0f - 0.25f * age);
        return x < 0.0f ? -std::pow(-x, e) : std::pow(x, e);
    }

    float readBuf(const int c, double idx) const noexcept {
        const double L = static_cast<double>(len_);
        idx = std::fmod(idx, L);
        if (idx < 0.0) idx += L;
        const std::size_t i0 = static_cast<std::size_t>(idx);
        const std::size_t i1 = (i0 + 1) % len_;
        const float fr = static_cast<float>(idx - static_cast<double>(i0));
        return buf_[c][i0] * (1.0f - fr) + buf_[c][i1] * fr;
    }

    float toneFilter(const int c, const float x, const float tone) noexcept {
        toneZ_[c] += (x - toneZ_[c]) * toneCoef_;
        if (tone < 0.0f) return x + (toneZ_[c] - x) * (-tone);   // → passa-baixa
        return x + ((x - toneZ_[c]) - x) * tone;                 // → passa-alta
    }

    // chorus / flanger / ensemble
    float modLine(const int c, const float in, const float fbAmt,
                  const float age, const float depth, const float ph,
                  const float baseMs, const float modMs, const int voices,
                  const float tone) noexcept {
        float inC = in + fbState_[c] * fbAmt;
        if (age > 0.0f) inC += (rnd01() * 2.0f - 1.0f) * age * 0.002f;
        buf_[c][w_[c]] = compress(inC, age);

        const float sr = sr_;
        float acc = 0.0f;
        for (int v = 0; v < voices; ++v) {
            // triângulo é contínuo (só tem quina) → sem zíper; a leitura
            // interpolada segura o resto
            const float lv = tri(ph + static_cast<float>(v) * 0.37f);
            const double d = static_cast<double>(
                (baseMs + depth * modMs * (0.5f + 0.5f * lv) + wob_)
                * sr * 0.001f);
            acc += readBuf(c, static_cast<double>(w_[c]) - d);
        }
        float wet = expand(acc / static_cast<float>(voices), age);
        wet = toneFilter(c, wet, tone);
        fbState_[c] = std::tanh(wet * 1.1f) * 0.9f;
        w_[c] = (w_[c] + 1) % len_;
        return wet;
    }

    // phaser: 6 all-pass de 1ª ordem TPT varridos pelo LFO
    float phaser(const int c, const float in, const float feedback,
                 const float depth, const float lfo,
                 const float tone) noexcept {
        const float sweep = 0.5f + 0.5f * depth * lfo;    // 0..1
        const float fc = 180.0f * std::pow(2200.0f / 180.0f, sweep);
        const float tw = std::tan(3.14159265f * fc / sr_);
        const float g = tw / (1.0f + tw);

        float x = in + fbState_[c] * feedback * 0.7f;
        for (int k = 0; k < kStages; ++k) {
            const float vpar = (x - apS_[c][k]) * g;
            const float lp = vpar + apS_[c][k];
            apS_[c][k] = lp + vpar;
            x = 2.0f * lp - x;      // all-pass = 2·LP − entrada
        }
        float wet = toneFilter(c, x, tone);
        fbState_[c] = std::tanh(wet * 1.1f) * 0.9f;
        return wet;
    }

    std::vector<float> buf_[2];
    std::size_t len_ = 1;
    std::size_t w_[2] = {0, 0};
    float fbState_[2] = {0.0f, 0.0f};
    float apS_[2][kStages] = {};
    float toneZ_[2] = {0.0f, 0.0f};
    float toneCoef_ = 0.15f;
    float phL_ = 0.0f;
    int wobCnt_ = 0;
    float wob_ = 0.0f;
    std::uint64_t rng_ = 0x5117E7E1D5A1C0FFULL;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
