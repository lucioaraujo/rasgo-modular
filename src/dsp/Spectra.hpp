#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SPECTRA — resíntese espectral (Módulo 57)
// ============================================================================
//
// Ouve um sinal, encontra os parciais mais fortes e RE-OSCILA como um
// banco de senóides que SEGUE o som — a ponte análise → síntese que
// faltava (o `ADDITIVE` constrói do zero; o `MEMORY` grão no tempo; aqui
// o material é o espectro de curto prazo de uma entrada). Panharmonium /
// Rainmaker spectral no idioma RASGO.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/57_spectra.md`.
//
// ANÁLISE: banco de 64 passa-faixas ressonantes log-espaçados (35 Hz–14
// kHz, bandas que se sobrepõem) + seguidor de pico por banda. A cada
// ~6 ms (hop) pega os `voices` picos locais mais fortes e — com
// interpolação parabólica em log-freq entre as 3 bandas vizinhas —
// refina a frequência de cada um.
//
// SÍNTESE: `voices` (2–24) senóides de fase contínua; cada uma DESLIZA
// (sem zíper) para a freq/amp do pico que herdou. `blur` = quão devagar
// (0 = trava rápido no som, 1 = borra/arrasta). `freeze` congela os
// alvos. `shift` (+CV 1 V/oct) e `stretch` (inarmônico) transpõem a
// re-síntese; `tone` inclina o espectro; `jitter` = wobble SEMEADO por
// voz.
//
// Fonte — soa ao carregar: `in` livre → um ruído interno de −30 dB com
// dois "parciais fantasma" que derivam devagar (semeados) alimenta a
// análise → drone tonal que evolui sozinho. `mix` seco↔ressoado.
// `process()` não aloca. `jitter=0` + `in` cabeado → determinístico.

namespace rasgo::modular {

class Spectra final : public Signal {
public:
    Spectra()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"pitch", PortKind::Control, "v/oct"},
               {"freeze", PortKind::Control, "gate"}},
              {{"out", PortKind::Audio, ""},
               {"r", PortKind::Audio, ""}},
              {{"voices", 2.0f, 24.0f, 12.0f, ""},
               {"blur", 0.0f, 1.0f, 0.3f, ""},
               {"shift", -2.0f, 2.0f, 0.0f, "oct"},
               {"stretch", -1.0f, 1.0f, 0.0f, ""},
               {"tone", -1.0f, 1.0f, 0.0f, ""},
               {"jitter", 0.0f, 1.0f, 0.0f, ""},
               {"freeze", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "SPECTRA"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "SPECTRA", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "spectra", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "VOICE", "voices", 9.0f, 32.0f);
        p.add(Widget::Kind::Knob, "BLUR", "blur", 27.0f, 32.0f);
        p.add(Widget::Kind::Knob, "SHIFT", "shift", 45.0f, 32.0f);
        p.add(Widget::Kind::Knob, "STRCH", "stretch", 9.0f, 56.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 27.0f, 56.0f);
        p.add(Widget::Kind::Knob, "JITR", "jitter", 45.0f, 56.0f);
        p.add(Widget::Kind::Toggle, "FRZ", "freeze", 9.0f, 80.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 27.0f, 80.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "PIT", "in:pitch", 22.0f, 100.0f);
        p.add(Widget::Kind::Jack, "FRZ", "in:freeze", 36.0f, 100.0f);
        p.add(Widget::Kind::Jack, "L", "out:out", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 22.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        rng_ = 0x51EC7A0DEADBEEF1ULL;

        // banco de análise: 64 passa-faixas ressonantes log-espaçados,
        // 35 Hz–14 kHz, largura de banda ~= 1/9 de oitava (as bandas se
        // sobrepõem, então um parcial entre bandas ainda é detectado)
        const float f0 = 35.0f, f1 = std::min(14000.0f, sr_ * 0.45f);
        for (int b = 0; b < kBands; ++b) {
            const float t = static_cast<float>(b) / (kBands - 1);
            bandHz_[b] = f0 * std::pow(f1 / f0, t);
            const float w = 6.2831853f * bandHz_[b] / sr_;
            const float bw = bandHz_[b] * 0.11f;           // Hz
            const float r = std::exp(-3.14159265f * bw / sr_);
            bpA1_[b] = 2.0f * r * std::cos(w);
            bpA2_[b] = -r * r;
            bpB0_[b] = 1.0f - r;             // BP normalizado (num. x−x[-2])
            x1_[b] = x2_[b] = y1_[b] = y2_[b] = 0.0f;
            env_[b] = 0.0f;
        }
        envAtk_ = 1.0f - std::exp(-1.0f / (0.003f * sr_));
        envRel_ = 1.0f - std::exp(-1.0f / (0.060f * sr_));
        hop_ = std::max<std::size_t>(1, static_cast<std::size_t>(sr_ * 0.006f));
        hopCnt_ = 0;

        for (auto& v : voices_) v = Voice{};
        ghostPh_[0] = 0.0f; ghostPh_[1] = 0.37f;
        ghostF_[0] = 180.0f; ghostF_[1] = 430.0f;
        ghostDrift_[0] = ghostDrift_[1] = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& outL = outputs[0];
        AudioBlock& outR = outputs[1];
        const std::size_t frames = outL.frames();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* pitchB = inputs[1];
        const AudioBlock* frzB = inputs[2];

        const int voicesP = static_cast<int>(std::lround(
            clampf(parameterValue("voices"), 2.0f, static_cast<float>(kMaxV))));
        const float blur = clamp01(parameterValue("blur"));
        const float shiftP = clampf(parameterValue("shift"), -2.0f, 2.0f);
        const float stretch = clampf(parameterValue("stretch"), -1.0f, 1.0f);
        const float tone = clampf(parameterValue("tone"), -1.0f, 1.0f);
        const float jitter = clamp01(parameterValue("jitter"));
        const bool freezeP = parameterValue("freeze") >= 0.5f;
        const float mix = clamp01(parameterValue("mix"));

        // deslize por amostra dos alvos: blur 0 → ~5 ms, blur 1 → ~600 ms
        const float glideT = 0.005f + blur * blur * 0.6f;
        const float glide = 1.0f - std::exp(-1.0f / (glideT * sr_));

        for (std::size_t f = 0; f < frames; ++f) {
            // ---- entrada da análise (ou ruído interno + parciais fantasma) --
            float x;
            if (inB) {
                x = inB->at(0, f);
            } else {
                for (int g = 0; g < 2; ++g) {
                    ghostDrift_[g] += (rnd11() * 0.5f - ghostDrift_[g]) * 0.00002f;
                    const float gf = ghostF_[g] * (1.0f + ghostDrift_[g] * 0.4f);
                    ghostPh_[g] += gf / sr_;
                    ghostPh_[g] -= std::floor(ghostPh_[g]);
                }
                x = rnd11() * 0.10f
                  + 0.16f * std::sin(6.2831853f * ghostPh_[0])
                  + 0.12f * std::sin(6.2831853f * ghostPh_[1]);
            }

            // ---- banco de análise (por amostra) — BP normalizado ----
            for (int b = 0; b < kBands; ++b) {
                const float y = bpB0_[b] * (x - x2_[b])
                              + bpA1_[b] * y1_[b] + bpA2_[b] * y2_[b];
                x2_[b] = x1_[b]; x1_[b] = x;
                y2_[b] = y1_[b]; y1_[b] = y;
                const float mag = std::fabs(y);
                const float k = mag > env_[b] ? envAtk_ : envRel_;
                env_[b] += (mag - env_[b]) * k;
            }

            // ---- hop: reatribui as vozes aos picos (a menos que congelado) --
            const bool frozen = freezeP
                || (frzB && frzB->at(0, f) >= 0.5f);
            if (++hopCnt_ >= hop_) {
                hopCnt_ = 0;
                if (!frozen) {
                    pickPeaks(voicesP);
                    for (int i = 0; i < voicesP; ++i) {
                        voices_[i].fTarget = peakHz_[i];
                        voices_[i].aTarget = peakAmp_[i];
                    }
                    for (int i = voicesP; i < kMaxV; ++i)
                        voices_[i].aTarget = 0.0f;
                }
            }

            // ---- re-síntese ----
            const float pitchCv = pitchB ? pitchB->at(0, f) : 0.0f;
            const float xpose = std::pow(2.0f, shiftP + pitchCv);
            float oL = 0.0f, oR = 0.0f;
            for (int i = 0; i < voicesP; ++i) {
                Voice& v = voices_[i];
                v.freq += (v.fTarget - v.freq) * glide;
                v.amp  += (v.aTarget - v.amp)  * glide;
                if (v.amp < 1e-5f && v.aTarget < 1e-5f) continue;

                // stretch inarmônico + jitter semeado
                const float rel = static_cast<float>(i)
                                / std::max(1, voicesP - 1) - 0.5f;
                v.jit += (rnd11() - v.jit) * 0.0006f;
                float fk = v.freq * xpose
                         * (1.0f + stretch * 0.35f * rel)
                         * (1.0f + jitter * v.jit * 0.03f);
                fk = clampf(fk, 8.0f, sr_ * 0.47f);

                v.ph += fk / sr_;
                v.ph -= std::floor(v.ph);
                // inclinação espectral (tone)
                float g = v.amp;
                if (tone != 0.0f)
                    g *= std::pow(clampf(fk / 800.0f, 0.05f, 20.0f), tone * 1.3f);
                const float s = g * std::sin(6.2831853f * v.ph);
                const float pan = (i & 1) ? 0.28f : -0.28f;
                oL += s * (0.5f - pan * 0.5f);
                oR += s * (0.5f + pan * 0.5f);
            }
            oL = std::tanh(oL * 1.2f);
            oR = std::tanh(oR * 1.2f);

            const float dry = inB ? inB->at(0, f) : 0.0f;
            const float mL = dry + (oL - dry) * mix;
            const float mR = dry + (oR - dry) * mix;
            for (std::size_t c = 0; c < outL.channels(); ++c) outL.at(c, f) = mL;
            for (std::size_t c = 0; c < outR.channels(); ++c) outR.at(c, f) = mR;
        }
    }

private:
    static constexpr int kBands = 64;
    static constexpr int kMaxV = 24;

    struct Voice {
        double ph = 0.0;
        float freq = 110.0f, amp = 0.0f;
        float fTarget = 110.0f, aTarget = 0.0f;
        float jit = 0.0f;
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float rnd11() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(static_cast<std::int32_t>(rng_ >> 32))
             / 2147483648.0f;
    }

    // escolhe os `n` picos locais mais fortes do banco → peakHz_/peakAmp_
    void pickPeaks(const int n) noexcept {
        // pico global pra o corte de ruído
        float mx = 1e-9f;
        for (int b = 0; b < kBands; ++b) mx = std::max(mx, env_[b]);
        // candidatos: máximos locais acima de −40 dB do maior
        std::array<std::pair<float, int>, kBands> cand{};
        int nc = 0;
        for (int b = 1; b < kBands - 1; ++b)
            if (env_[b] >= env_[b - 1] && env_[b] > env_[b + 1]
                && env_[b] > mx * 0.01f)
                cand[nc++] = {env_[b], b};
        // ordena decrescente
        std::sort(cand.begin(), cand.begin() + nc,
                  [](const auto& a, const auto& c) { return a.first > c.first; });
        for (int i = 0; i < n; ++i) {
            if (i < nc) {
                const int b = cand[static_cast<std::size_t>(i)].second;
                // interpolação parabólica em log-freq entre b-1,b,b+1
                const float lm = std::log(std::max(1e-6f, env_[b - 1]));
                const float lc = std::log(std::max(1e-6f, env_[b]));
                const float lr = std::log(std::max(1e-6f, env_[b + 1]));
                const float denom = lm - 2.0f * lc + lr;
                const float delta = std::fabs(denom) > 1e-6f
                    ? 0.5f * (lm - lr) / denom : 0.0f;
                const float lf0 = std::log(bandHz_[b - 1]);
                const float lf2 = std::log(bandHz_[b + 1]);
                const float step = 0.5f * (lf2 - lf0);
                peakHz_[i] = std::exp(std::log(bandHz_[b])
                                      + clampf(delta, -1.0f, 1.0f) * step);
                peakAmp_[i] = env_[b] * 1.5f;
            } else {
                peakAmp_[i] = 0.0f;
                peakHz_[i] = peakHz_[i > 0 ? i - 1 : 0];
            }
        }
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x51EC7A0DEADBEEF1ULL;

    float bandHz_[kBands] = {};
    float bpA1_[kBands] = {}, bpA2_[kBands] = {}, bpB0_[kBands] = {};
    float x1_[kBands] = {}, x2_[kBands] = {};
    float y1_[kBands] = {}, y2_[kBands] = {};
    float env_[kBands] = {};
    float envAtk_ = 0.1f, envRel_ = 0.01f;

    float peakHz_[kMaxV] = {};
    float peakAmp_[kMaxV] = {};

    std::size_t hop_ = 288;
    std::size_t hopCnt_ = 0;

    Voice voices_[kMaxV];
    float ghostPh_[2] = {0.0f, 0.0f};
    float ghostF_[2] = {180.0f, 430.0f};
    float ghostDrift_[2] = {0.0f, 0.0f};
};

}  // namespace rasgo::modular
