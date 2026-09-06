#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// HALL — reverberação FDN (Módulo 46)
// ============================================================================
//
// O `SPACE` (#10) é multitap + all-pass em série. `HALL` é a rede de
// atraso realimentada (FDN, Jot & Chaigne 1991) que o `PESQUISA §2`
// anotou como pendência: 8 linhas de atraso realimentadas por uma matriz
// de Householder (reflexão ortogonal — sem perda), decaimento dependente
// da frequência, e modulação das linhas pra a cauda não apitar.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/46_hall.md`.
//
// - `size`  (0–1, +CV)  escala os 8 comprimentos (0,3×–1,7×, ~8–88 ms)
// - `decay` (0–1, +CV)  RT60 = 0,2·75^decay (0,2 s a 15 s)
// - `damp`  (0–1)       passa-baixa de 1 polo NO laço (agudo decai antes)
// - `mod`   (0–1)       modulação do ponto de leitura — chorus, quebra o
//                       ringing. DETERMINÍSTICO (senóides, sem RNG).
// - `pre`   (0–1)       pré-atraso (0 a ~120 ms)
// - `mix`   (0–1)       seco ↔ molhado (0 = passa-direto bit-exato)
// - `freeze` (gate)     g_i → 1 (cauda infinita) + entrada → 0
//
// Householder é ortogonal → a rede é estável pra g_i ≤ 1. `softLimit` na
// saída + flush de denormais. `prepare()` aloca ~0,4 MB; `process()` não
// aloca. Estéreo (`l`/`r` descorrelacionadas). Determinístico.

namespace rasgo::modular {

class Hall final : public Signal {
public:
    static constexpr int kLines = 8;

    Hall()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"size", PortKind::Control, ""},
               {"decay", PortKind::Control, ""},
               {"freeze", PortKind::Control, "gate"}},
              {{"l", PortKind::Audio, ""},
               {"r", PortKind::Audio, ""}},
              {{"size", 0.0f, 1.0f, 0.5f, ""},
               {"decay", 0.0f, 1.0f, 0.5f, ""},
               {"damp", 0.0f, 1.0f, 0.4f, ""},
               {"mod", 0.0f, 1.0f, 0.2f, ""},
               {"pre", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 0.3f, ""}}) {}

    std::string type() const override { return "HALL"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "HALL", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "tail", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "SIZE", "size", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DECAY", "decay", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damp", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MOD", "mod", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "PRE", "pre", 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 24.0f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SIZE", "in:size", 22.0f, 92.0f);
        p.add(Widget::Kind::Jack, "DEC", "in:decay", 38.0f, 92.0f);
        p.add(Widget::Kind::Jack, "FRZ", "in:freeze", 52.0f, 92.0f);
        p.add(Widget::Kind::Jack, "L", "out:l", 8.0f, 114.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 20.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        const float scale = sr_ / 48000.0f;

        std::size_t maxLen = 0;
        for (int i = 0; i < kLines; ++i) {
            len_[i] = static_cast<float>(kBase[i]) * scale;
            const std::size_t need =
                static_cast<std::size_t>(len_[i] * 1.7f) + 64;
            maxLen = std::max(maxLen, need);
        }
        for (int i = 0; i < kLines; ++i) {
            line_[i].assign(maxLen, 0.0f);
            wr_[i] = 0;
            lp_[i] = 0.0f;
            modPh_[i] = static_cast<float>(i) * 0.1234f;
            lineOut_[i] = 0.0f;
        }
        preBuf_.assign(static_cast<std::size_t>(0.13f * sr_) + 4, 0.0f);
        preWr_ = 0;
        inRamp_ = 1.0f;
        prevFreeze_ = false;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& outL = outputs[0];
        AudioBlock& outR = outputs[1];
        const std::size_t frames = outL.frames();
        const std::size_t channels = outL.channels();

        const AudioBlock* in = inputs[0];
        const AudioBlock* sizeIn = inputs[1];
        const AudioBlock* decIn = inputs[2];
        const AudioBlock* frzIn = inputs[3];

        const float sizeKnob = clamp01(parameterValue("size"));
        const float decKnob = clamp01(parameterValue("decay"));
        const float damp = clamp01(parameterValue("damp"));
        const float mod = clamp01(parameterValue("mod"));
        const float pre = clamp01(parameterValue("pre"));
        const float mix = clamp01(parameterValue("mix"));

        const float fcHz = 18000.0f - damp * 16000.0f;
        const float dampCoef = 1.0f - std::exp(-6.28318530718f * fcHz / sr_);
        const float modDepth = mod * 18.0f;
        const float preSamp = pre * 0.12f * sr_;
        const float rampCoef = 1.0f - std::exp(-1.0f / (0.01f * sr_));

        // taxas de LFO por linha (~0,5–1,4 Hz), fases já distintas
        static const float modRate[kLines] =
            {0.53f, 0.61f, 0.74f, 0.83f, 0.97f, 1.09f, 1.21f, 1.37f};

        for (std::size_t f = 0; f < frames; ++f) {
            const float dry = in ? in->at(0, f) : 0.0f;
            const bool frz = frzIn && frzIn->at(0, f) >= 0.5f;

            const float sizeEff = clamp01(sizeKnob
                + (sizeIn ? sizeIn->at(0, f) : 0.0f));
            const float decEff = clamp01(decKnob
                + (decIn ? decIn->at(0, f) : 0.0f));
            const float rt60 = 0.2f * std::pow(75.0f, decEff);
            const float dmul = 0.3f + sizeEff * 1.4f;

            // rampa de entrada (freeze)
            const float rampTarget = frz ? 0.0f : 1.0f;
            inRamp_ += (rampTarget - inRamp_) * rampCoef;
            prevFreeze_ = frz;

            // pré-atraso
            preBuf_[static_cast<std::size_t>(preWr_)] = dry;
            const float preOut = readRing(preBuf_, preWr_, preSamp);
            preWr_ = (preWr_ + 1) % static_cast<int>(preBuf_.size());
            const float xin = preOut * inRamp_;

            // --- ler as linhas + damping + decaimento ---
            float s[kLines];
            for (int i = 0; i < kLines; ++i) {
                modPh_[i] += modRate[i] * dt_;
                modPh_[i] -= std::floor(modPh_[i]);
                const float d = len_[i] * dmul
                    + modDepth * std::sin(6.28318530718f * modPh_[i]);
                float v = readLine(i, d);
                lp_[i] += (v - lp_[i]) * dampCoef;
                v = lp_[i];
                const float g = frz ? 1.0f
                    : std::pow(10.0f, -3.0f * (len_[i] * dmul * dt_) / rt60);
                s[i] = v * g;
                lineOut_[i] = v;   // pré-decaimento, pra as saídas
            }

            // --- matriz de Householder + escrita ---
            float h = 0.0f;
            for (int i = 0; i < kLines; ++i) h += s[i];
            h *= 2.0f / static_cast<float>(kLines);
            static const float inVec[kLines] =
                {0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f};
            for (int i = 0; i < kLines; ++i) {
                float w = xin * inVec[i] + (s[i] - h);
                w = flush(w);
                line_[i][static_cast<std::size_t>(wr_[i])] = w;
                wr_[i] = (wr_[i] + 1) % static_cast<int>(line_[i].size());
            }

            // --- saídas estéreo descorrelacionadas ---
            const float wetL = lineOut_[0] - lineOut_[2] + lineOut_[4] - lineOut_[6];
            const float wetR = lineOut_[1] - lineOut_[3] + lineOut_[5] - lineOut_[7];
            const float yL = softLimit(dry * (1.0f - mix) + wetL * mix * 0.6f);
            const float yR = softLimit(dry * (1.0f - mix) + wetR * mix * 0.6f);

            for (std::size_t c = 0; c < channels; ++c) {
                outL.at(c, f) = yL;
                outR.at(c, f) = yR;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    static float flush(const float v) noexcept {
        return (v < 1.0e-18f && v > -1.0e-18f) ? 0.0f : v;
    }
    static float softLimit(const float v) noexcept {
        if (v > 0.8f) return 0.8f + 0.2f * std::tanh((v - 0.8f) * 5.0f);
        if (v < -0.8f) return -0.8f + 0.2f * std::tanh((v + 0.8f) * 5.0f);
        return v;
    }

    // leitura interpolada `delay` amostras atrás da escrita da linha i
    float readLine(const int i, const float delay) const noexcept {
        const auto& b = line_[static_cast<std::size_t>(i)];
        const float n = static_cast<float>(b.size());
        float p = static_cast<float>(wr_[i]) - clampf(delay, 1.0f, n - 2.0f);
        while (p < 0.0f) p += n;
        const int i0 = static_cast<int>(p);
        const float fr = p - static_cast<float>(i0);
        const int i1 = (i0 + 1) % static_cast<int>(b.size());
        return b[static_cast<std::size_t>(i0)] * (1.0f - fr)
             + b[static_cast<std::size_t>(i1)] * fr;
    }

    static float readRing(const std::vector<float>& b, const int wr,
                          const float delay) noexcept {
        const float n = static_cast<float>(b.size());
        float p = static_cast<float>(wr) - clampf(delay, 0.0f, n - 2.0f);
        while (p < 0.0f) p += n;
        const int i0 = static_cast<int>(p);
        const float fr = p - static_cast<float>(i0);
        const int i1 = (i0 + 1) % static_cast<int>(b.size());
        return b[static_cast<std::size_t>(i0)] * (1.0f - fr)
             + b[static_cast<std::size_t>(i1)] * fr;
    }

    static constexpr int kBase[kLines] =
        {1237, 1381, 1607, 1777, 1949, 2137, 2273, 2477};

    std::vector<float> line_[kLines];
    std::vector<float> preBuf_;
    float len_[kLines] = {0};
    float lp_[kLines] = {0};
    float modPh_[kLines] = {0};
    float lineOut_[kLines] = {0};
    int wr_[kLines] = {0};
    int preWr_ = 0;
    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float inRamp_ = 1.0f;
    bool prevFreeze_ = false;
};

}  // namespace rasgo::modular
