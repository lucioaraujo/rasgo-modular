#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// FORMANT — ressoador espectral multibanda (Módulo 45)
// ============================================================================
//
// O `PARAMETRIC` é EQ estático (biquads em série). `FORMANT` é o oposto:
// 5 passa-faixas em PARALELO cujas frequências/bandas/ganhos seguem uma
// tabela de VOGAIS e são varridos por um knob — o espectro FALA.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/45_formant.md`.
//
// - `vowel` (0–1, +CV)  posição na sequência A → E → I → O → U
// - `shift` (−1..1)     escala todas as frequências (2^(shift·1,5)) — trato
// - `res`   (0–1)       estreita as bandas (bw / (1 + res·8)) — canta/apita
// - `vocoder` (0–1)     mistura: os GANHOS das 5 bandas passam a seguir a
//                       energia do MODULADOR (entrada `mod`) em cada
//                       frequência de formante em vez da tabela de vogal.
//                       0 = FORMANT clássico; 1 = vocoder de 5 bandas
//                       (Dudley). Sem `mod` cabeado → tratado como 0.
// - `mix`   (0–1)       seco ↔ ressoado (0 = passa-direto bit-exato)
// - `drift` (0–1)       wobble lento por formante, DETERMINÍSTICO (sem RNG)
//
// Núcleo: 5× SVF TPT (Simper/Cytomic), não-linearidade no laço — mesmo do
// `FILTER`/`WASP`. No modo vocoder, +5 SVF de análise no `mod` + 5
// seguidores de envelope. Sem entrada → silêncio (é TRANSFORM).
// `process()` não aloca; os dados de vogal são `constexpr`. Determinístico
// sempre (`vocoder=0` → saída byte-idêntica à versão sem esta feature).

namespace rasgo::modular {

class Formant final : public Signal {
public:
    static constexpr int kBands = 5;
    static constexpr int kVowels = 5;   // A E I O U

    Formant()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"vowel", PortKind::Control, ""},
               {"shift", PortKind::Control, ""},
               {"mod", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"vowel", 0.0f, 1.0f, 0.0f, ""},
               {"shift", -1.0f, 1.0f, 0.0f, ""},
               {"res", 0.0f, 1.0f, 0.4f, ""},
               {"vocoder", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "FORMANT"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "FORMANT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "spec", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "VOWEL", "vowel", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SHIFT", "shift", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RES", "res", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "VOCOD", "vocoder", 24.0f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "VOW", "in:vowel", 22.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SHF", "in:shift", 36.0f, 92.0f);
        p.add(Widget::Kind::Jack, "MOD", "in:mod", 50.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        for (int k = 0; k < kBands; ++k) {
            svf_[k].ic1eq = 0.0f;
            svf_[k].ic2eq = 0.0f;
            anaSvf_[k].ic1eq = 0.0f;
            anaSvf_[k].ic2eq = 0.0f;
            env_[k] = 0.0f;
            dph_[k] = 0.0f;
        }
        envCoef_ = 1.0f - std::exp(-1.0f / (0.012f * sr_));   // ~12 ms
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* in = inputs[0];
        const AudioBlock* vowIn = inputs[1];
        const AudioBlock* shfIn = inputs[2];
        const AudioBlock* modIn = inputs.size() > 3 ? inputs[3] : nullptr;

        const float vowKnob = clamp01(parameterValue("vowel"));
        const float shfKnob = clampf(parameterValue("shift"), -1.0f, 1.0f);
        const float res = clamp01(parameterValue("res"));
        const float mix = clamp01(parameterValue("mix"));
        const float drift = clamp01(parameterValue("drift"));
        // vocoder só quando há modulador cabeado
        const float voc = modIn ? clamp01(parameterValue("vocoder")) : 0.0f;
        const float nyqCut = 0.45f * sr_;
        const float resDiv = 1.0f + res * 8.0f;

        // deriva: avança 5 fases lentas por bloco
        static const float dInc[kBands] =
            {0.017f, 0.023f, 0.031f, 0.037f, 0.043f};
        for (int k = 0; k < kBands; ++k) {
            dph_[k] += dInc[k] * static_cast<float>(frames) * dt_;
            dph_[k] -= std::floor(dph_[k]);
        }
        float det[kBands];
        for (int k = 0; k < kBands; ++k)
            det[k] = 0.03f * drift
                   * std::sin(6.28318530718f * dph_[k]
                              + static_cast<float>(k) * 1.3f);

        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in ? in->at(0, f) : 0.0f;
            const float m = (voc > 0.0f && modIn) ? modIn->at(0, f) : 0.0f;

            const float vw = clamp01(vowKnob + (vowIn ? vowIn->at(0, f) : 0.0f))
                           * static_cast<float>(kVowels - 1);
            int seg = static_cast<int>(vw);
            if (seg > kVowels - 2) seg = kVowels - 2;
            const float t = vw - static_cast<float>(seg);
            const float shMul = std::exp2(
                clampf(shfKnob + (shfIn ? shfIn->at(0, f) : 0.0f), -1.0f, 1.0f)
                * 1.5f);

            float wet = 0.0f;
            for (int k = 0; k < kBands; ++k) {
                const float fa = kFreq[seg][k], fb = kFreq[seg + 1][k];
                float fk = std::exp(std::log(fa) * (1.0f - t) + std::log(fb) * t)
                         * shMul * (1.0f + det[k]);
                fk = clampf(fk, 20.0f, nyqCut);
                const float bw = (kBw[seg][k] * (1.0f - t) + kBw[seg + 1][k] * t)
                               / resDiv;
                const float gDb = kGain[seg][k] * (1.0f - t) + kGain[seg + 1][k] * t;
                float gLin = std::pow(10.0f, gDb / 20.0f);
                const float kk = clampf(bw / fk, 0.02f, 2.0f);

                // vocoder: o ganho da banda passa a seguir a energia do
                // MODULADOR nessa frequência (banco de análise + seguidor)
                if (voc > 0.0f) {
                    const float ab = anaSvf_[k].runBand(m, fk, kk, sr_);
                    const float amag = std::fabs(ab);
                    env_[k] += (amag - env_[k])
                             * (amag > env_[k] ? envCoef_ * 5.0f : envCoef_);
                    const float gVoc = clampf(env_[k] * 6.0f, 0.0f, 4.0f);
                    gLin = gLin * (1.0f - voc) + gVoc * voc;
                }

                const float bnd = svf_[k].runBand(x, fk, kk, sr_);
                wet += bnd * kk * gLin;
            }

            const float y = softLimit(x * (1.0f - mix) + wet * mix * 1.6f);
            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    struct Svf {
        float ic1eq = 0.0f, ic2eq = 0.0f;
        float runBand(const float x, const float fc, const float k,
                      const float sr) noexcept {
            const float g = std::tan(3.14159265f * fc / sr);
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float v1 = a1 * (ic1eq + g * (x - ic2eq));
            const float v2 = ic2eq + g * v1;
            ic1eq = 2.0f * sat(v1) - ic1eq;
            ic2eq = 2.0f * sat(v2) - ic2eq;
            return v1;
        }
        static float sat(const float v) noexcept {
            if (v > 1.0f) return 1.0f + std::tanh(v - 1.0f);
            if (v < -1.0f) return -1.0f + std::tanh(v + 1.0f);
            return v;
        }
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    static float softLimit(const float v) noexcept {
        if (v > 0.8f) return 0.8f + 0.2f * std::tanh((v - 0.8f) * 5.0f);
        if (v < -0.8f) return -0.8f + 0.2f * std::tanh((v + 0.8f) * 5.0f);
        return v;
    }

    // Formantes de vogais cantadas — voz de baixo. Frequência (Hz), ganho
    // (dB rel. F1), largura de banda (Hz). Dados fonéticos publicados
    // (tabelas Csound `fof` / Fant 1960) — fato, não código.
    //                              A       E       I       O       U
    static constexpr float kFreq[kVowels][kBands] = {
        { 600.0f, 1040.0f, 2250.0f, 2450.0f, 2750.0f},   // A
        { 400.0f, 1620.0f, 2400.0f, 2800.0f, 3100.0f},   // E
        { 250.0f, 1750.0f, 2600.0f, 3050.0f, 3340.0f},   // I
        { 400.0f,  750.0f, 2400.0f, 2600.0f, 2900.0f},   // O
        { 350.0f,  600.0f, 2400.0f, 2675.0f, 2950.0f},   // U
    };
    static constexpr float kGain[kVowels][kBands] = {
        { 0.0f,  -7.0f,  -9.0f,  -9.0f, -20.0f},   // A
        { 0.0f, -12.0f,  -9.0f, -12.0f, -18.0f},   // E
        { 0.0f, -30.0f, -16.0f, -22.0f, -28.0f},   // I
        { 0.0f, -11.0f, -21.0f, -20.0f, -40.0f},   // O
        { 0.0f, -20.0f, -32.0f, -28.0f, -36.0f},   // U
    };
    static constexpr float kBw[kVowels][kBands] = {
        { 60.0f,  70.0f, 110.0f, 120.0f, 130.0f},   // A
        { 40.0f,  80.0f, 100.0f, 120.0f, 120.0f},   // E
        { 60.0f,  90.0f, 100.0f, 120.0f, 120.0f},   // I
        { 40.0f,  80.0f, 100.0f, 120.0f, 120.0f},   // O
        { 40.0f,  80.0f, 100.0f, 120.0f, 120.0f},   // U
    };

    Svf svf_[kBands];
    Svf anaSvf_[kBands];              // banco de análise do modulador (vocoder)
    float env_[kBands] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float envCoef_ = 0.02f;
    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float dph_[kBands] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
};

}  // namespace rasgo::modular
