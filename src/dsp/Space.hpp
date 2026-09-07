#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

// ============================================================================
// SPACE — atraso multitap com difusão (Módulo 10)
// ============================================================================
//
// Onde o som acontece. Uma linha de atraso com várias tomadas em tempos
// diferentes; realimentação com filtro de tom; uma cadeia de all-pass que
// espalha os ecos até virarem cauda. De eco rítmico a reverberação, pelo
// mesmo objeto - o espaço é um contínuo, não dois efeitos.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/10_space.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Schroeder / Moorer - reverberação por comb + all-pass; a difusão
//     como all-pass em série;
//   - Dattorro (1997), "Effect Design" - a topologia de reverb por
//     figura-8 de all-pass com damping no laço;
//   - Mutable Rainmaker / multitap clássico - N tomadas com tempo e ganho
//     próprios como material rítmico;
//   - chorus/ensemble - LFO no tempo de leitura (`mod`).
//
// Determinístico: LFO interno (sem RNG); fases das tomadas fixas.
// Buffer alocado em prepare(); process() não aloca.

namespace rasgo::modular {

class Space final : public Signal {
public:
    Space()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"time_mod", PortKind::Control, ""},
               {"feedback_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""},
               {"wet", PortKind::Audio, ""}},
              {{"time", 0.002f, 2.0f, 0.28f, "s"},
               {"taps", 1.0f, 8.0f, 4.0f, ""},
               {"spread", 0.0f, 1.0f, 0.6f, ""},
               {"feedback", 0.0f, 0.95f, 0.35f, ""},
               {"diffusion", 0.0f, 1.0f, 0.4f, ""},
               {"tone", 0.0f, 1.0f, 0.5f, ""},
               {"mod", 0.0f, 1.0f, 0.15f, ""},
               {"mix", 0.0f, 1.0f, 0.35f, ""}}) {}

    std::string type() const override { return "SPACE"; }

    // Layout: 14 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "SPACE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "taps", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "TIME", "time", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TAPS", "taps", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DIFF", "diffusion", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "MOD", "mod", 35.0f, 52.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 49.0f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "TIME", "in:time_mod", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "FBK", "in:feedback_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "WET", "out:wet", 17.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        bufLen_ = static_cast<std::size_t>(std::max(1.0f, sampleRate * 2.2f));
        buffer_.assign(bufLen_, 0.0f);
        writePos_ = 0;
        lpState_ = 0.0f;
        lfoPhase_ = 0.0f;
        // glide de ~10 ms nos parâmetros contínuos: sem isto, quando o VARIA
        // (ou um arrasto de knob) varre `time`, a tomada de atraso salta de
        // posição a cada bloco = um clique. `ctlPrimed_` faz o 1º bloco
        // assentar no valor exato → patch estático fica byte-idêntico.
        ctlCoef_ = 1.0f - std::exp(-1.0f / (0.010f * sampleRate));
        ctlPrimed_ = false;
        for (int i = 0; i < kAllpass; ++i) {
            apBuf_[i].assign(kApLen[i], 0.0f);
            apPos_[i] = 0;
        }
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& wetOut = outputs[1];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float timeParam = parameterValue("time");
        const int taps = clampi(
            static_cast<int>(std::lround(parameterValue("taps"))), 1, 8);
        const float spread = parameterValue("spread");
        const float feedbackParam = parameterValue("feedback");
        const float diffusion = parameterValue("diffusion");
        const float tone = parameterValue("tone");
        const float modAmt = parameterValue("mod");
        const float mix = parameterValue("mix");

        const AudioBlock* in = inputs[0];
        const AudioBlock* timeMod = inputs[1];
        const AudioBlock* fbMod = inputs[2];

        const float lfoInc = 0.13f / sampleRate_;    // ~0,13 Hz
        const float modSamples = modAmt * 0.004f * sampleRate_;

        if (!ctlPrimed_) {
            sTime_ = timeParam; sSpread_ = spread; sFb_ = feedbackParam;
            sTone_ = tone; sMix_ = mix;
            ctlPrimed_ = true;
        }

        for (std::size_t frame = 0; frame < frames; ++frame) {
            sTime_   += (timeParam     - sTime_)   * ctlCoef_;
            sSpread_ += (spread        - sSpread_) * ctlCoef_;
            sFb_     += (feedbackParam - sFb_)     * ctlCoef_;
            sTone_   += (tone          - sTone_)   * ctlCoef_;
            sMix_    += (mix           - sMix_)    * ctlCoef_;
            const float lpCoeff = 0.08f + sTone_ * 0.9f;  // 1 = brilhante

            const float dry = in ? in->at(0, frame) : 0.0f;
            float t = sTime_;
            if (timeMod)
                t = clampf(t + timeMod->at(0, frame), 0.001f, 2.1f);
            float fb = sFb_;
            if (fbMod)
                fb = clampf(fb + fbMod->at(0, frame), 0.0f, 0.97f);

            lfoPhase_ += lfoInc;
            if (lfoPhase_ >= 1.0f)
                lfoPhase_ -= 1.0f;
            const float lfo = std::sin(6.2831853f * lfoPhase_) * modSamples;

            const float baseDelay = t * sampleRate_;

            // soma das tomadas
            float wet = 0.0f;
            for (int k = 0; k < taps; ++k) {
                const float frac = static_cast<float>(k + 1)
                    / static_cast<float>(taps);
                const float d =
                    baseDelay * (1.0f - sSpread_ + sSpread_ * frac)
                    + lfo * (k % 2 == 0 ? 1.0f : -1.0f);
                wet += readInterp(d) * (1.0f - 0.12f * static_cast<float>(k));
            }
            wet /= static_cast<float>(taps);

            // realimentação: filtro de tom no laço, sem all-pass (mantém o
            // andamento dos ecos limpo)
            lpState_ += lpCoeff * (wet - lpState_);
            buffer_[writePos_] = softLimit(dry + fb * lpState_);
            writePos_ = (writePos_ + 1) % bufLen_;

            // difusão: cadeia de all-pass só na SAÍDA molhada (de eco
            // discreto a cauda espalhada), misturada por `diffusion`
            float diffused = wet;
            for (int i = 0; i < kAllpass; ++i)
                diffused = allpass(i, diffused, 0.6f);
            const float wetMixed =
                (1.0f - diffusion) * wet + diffusion * diffused;
            const float outValue = sMix_ * wetMixed + (1.0f - sMix_) * dry;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                out.at(channel, frame) = outValue;
                wetOut.at(channel, frame) = wetMixed;
            }
        }
    }

private:
    static constexpr int kAllpass = 4;
    // comprimentos primos curtos (ms @ 48k ≈ 4,7 / 7,3 / 10,9 / 15,4)
    static constexpr std::size_t kApLen[kAllpass] = {223, 353, 523, 739};

    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float softLimit(const float x) noexcept { return std::tanh(x); }

    float readInterp(const float delaySamples) const noexcept {
        float d = delaySamples;
        if (d < 1.0f) d = 1.0f;
        if (d > static_cast<float>(bufLen_ - 2))
            d = static_cast<float>(bufLen_ - 2);
        const float rp = static_cast<float>(writePos_) - d;
        float idx = rp;
        while (idx < 0.0f)
            idx += static_cast<float>(bufLen_);
        const std::size_t i0 = static_cast<std::size_t>(idx) % bufLen_;
        const std::size_t i1 = (i0 + 1) % bufLen_;
        const float frac = idx - std::floor(idx);
        return buffer_[i0] * (1.0f - frac) + buffer_[i1] * frac;
    }

    float allpass(const int i, const float x, const float g) noexcept {
        const std::size_t n = kApLen[i];
        const float delayed = apBuf_[i][apPos_[i]];
        const float v = x - g * delayed;
        apBuf_[i][apPos_[i]] = v;
        apPos_[i] = (apPos_[i] + 1) % n;
        return delayed + g * v;
    }

    std::vector<float> buffer_;
    std::size_t bufLen_ = 1;
    std::size_t writePos_ = 0;
    float lpState_ = 0.0f;
    float lfoPhase_ = 0.0f;
    // parâmetros contínuos suavizados por amostra (anti-zíper / anti-clique)
    float sTime_ = 0.0f, sSpread_ = 0.0f, sFb_ = 0.0f, sTone_ = 0.0f, sMix_ = 0.0f;
    float ctlCoef_ = 0.0f;
    bool ctlPrimed_ = false;

    std::vector<float> apBuf_[kAllpass];
    std::size_t apPos_[kAllpass] = {0, 0, 0, 0};
};

}  // namespace rasgo::modular
