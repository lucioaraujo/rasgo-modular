#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>

// ============================================================================
// FILTER — filtro das três irmãs (Módulo 2)
// ============================================================================
//
// Três saídas (grave / centro / agudo) na MESMA frequência de corte; o
// caráter vem de COMO elas se relacionam (`spread`): de "três tomadas de
// um filtro só" (LP/BP/HP) até três passa-banda afastados que formam uma
// resposta de FORMANTE. A relação entre as saídas é o processo (Warps,
// Atlas §39; hardware de referência: Mannequins Three Sisters).
//
// Ver dossiê: `RASGO_MODULAR/dossies/02_filtro.md`.
//
// Núcleo: SVF **TPT/trapezoidal** de Andrew Simper / Cytomic ("Solving the
// continuous SVF equations using trapezoidal integration and equivalent
// currents", 2013) - paper público, reescrito do zero. Zero-delay-feedback,
// estável, afina até perto de Nyquist, dá LP/BP/HP de um cálculo só.
//
// Marco 1: sem oversampling no `drive` (alias medido; 2× é 2ª camada).

namespace rasgo::modular {

class Filter final : public Signal {
public:
    Filter()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"cutoff_mod", PortKind::Control, "v/oct"},
               {"res_mod", PortKind::Control, ""},
               {"spread_mod", PortKind::Control, ""}},
              {{"low", PortKind::Audio, ""},
               {"center", PortKind::Audio, ""},
               {"high", PortKind::Audio, ""},
               {"all", PortKind::Audio, ""}},
              {{"cutoff", 20.0f, 20000.0f, 800.0f, "Hz"},
               {"resonance", 0.0f, 1.0f, 0.15f, ""},
               {"spread", 0.0f, 1.0f, 0.0f, ""},
               {"drive", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "FILTER"; }

    // Layout: 12 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "FILTER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "response", "", 2.5f, 8.0f, 45.0f);
        p.add(Widget::Kind::Knob, "CUT", "cutoff", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RESO", "resonance", 30.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 30.0f, 54.0f);
        // entradas na faixa inferior, saídas abaixo
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 92.0f);
        p.add(Widget::Kind::Jack, "FC", "in:cutoff_mod", 17.0f, 92.0f);
        p.add(Widget::Kind::Jack, "Q", "in:res_mod", 29.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SPR", "in:spread_mod", 41.0f, 92.0f);
        p.add(Widget::Kind::Jack, "LO", "out:low", 5.0f, 114.0f);
        p.add(Widget::Kind::Jack, "CTR", "out:center", 17.0f, 114.0f);
        p.add(Widget::Kind::Jack, "HI", "out:high", 29.0f, 114.0f);
        p.add(Widget::Kind::Jack, "ALL", "out:all", 41.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        for (auto& s : sisters_)
            s = {};
        smoothCutoff_ = parameterValue("cutoff");
        paramCoeff_ = std::exp(-1.0f / (0.005f * std::max(1.0f, sampleRate)));
        nyquistLimit_ = sampleRate * 0.49f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const AudioBlock* in = inputs[0];
        const AudioBlock* cutoffMod = inputs[1];
        const AudioBlock* resMod = inputs[2];
        const AudioBlock* spreadMod = inputs[3];

        AudioBlock& low = outputs[0];
        AudioBlock& center = outputs[1];
        AudioBlock& high = outputs[2];
        AudioBlock& all = outputs[3];
        const std::size_t frames = low.frames();
        const std::size_t channels = low.channels();

        const float cutoffParam = parameterValue("cutoff");
        const float resParam = parameterValue("resonance");
        const float spreadParam = parameterValue("spread");
        const float driveParam = parameterValue("drive");
        const float inGain = 1.0f + driveParam * 6.0f;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            smoothCutoff_ = cutoffParam + paramCoeff_ * (smoothCutoff_ - cutoffParam);

            float cutoff = smoothCutoff_;
            if (cutoffMod != nullptr)
                cutoff *= std::exp2(cutoffMod->at(0, frame));
            float resonance = resParam + (resMod ? resMod->at(0, frame) : 0.0f);
            float spread = spreadParam + (spreadMod ? spreadMod->at(0, frame) : 0.0f);
            resonance = clampf(resonance, 0.0f, 1.0f);
            spread = clampf(spread, 0.0f, 1.0f);

            // Amortecimento k = 1/Q, MENOS um termo que o empurra pra
            // ligeiramente negativo perto de resonance=1 -> o SVF cresce e
            // o limitador de saída o prende num ciclo-limite (auto-
            // oscilação real numa senoide na frequência de corte, como um
            // filtro analógico com realimentação de ganho > 1).
            const float q = 0.5f * std::exp2(resonance * 8.0f);
            const float r4 = resonance * resonance * resonance * resonance;
            const float k = 1.0f / q - r4 * 0.012f;
            const float spreadOct = spread * 2.0f;  // até ±2 oitavas
            const float fLow = clampf(cutoff * std::exp2(-spreadOct), 15.0f, nyquistLimit_);
            const float fCtr = clampf(cutoff, 15.0f, nyquistLimit_);
            const float fHigh = clampf(cutoff * std::exp2(spreadOct), 15.0f, nyquistLimit_);

            for (std::size_t channel = 0; channel < channels; ++channel) {
                float x = in ? in->at(channel, frame) : 0.0f;
                if (!std::isfinite(x))
                    x = 0.0f;
                if (driveParam > 0.0f) {
                    const float d = x * inGain;
                    x = std::sin(clampf(d, -1.5f, 1.5f) * 1.5707963f);
                }
                const float lo = sisters_[channel * 3 + 0].run(x, fLow, k, sampleRate_,
                                                               SvfTap::Low);
                const float ct = sisters_[channel * 3 + 1].run(x, fCtr, k, sampleRate_,
                                                               SvfTap::Band);
                const float hi = sisters_[channel * 3 + 2].run(x, fHigh, k, sampleRate_,
                                                               SvfTap::High);
                low.at(channel, frame) = softLimit(lo);
                center.at(channel, frame) = softLimit(ct);
                high.at(channel, frame) = softLimit(hi);
                all.at(channel, frame) = softLimit((lo + ct + hi) * 0.5f);
            }
        }
    }

private:
    enum class SvfTap { Low, Band, High };

    struct Svf {
        float ic1eq = 0.0f;
        float ic2eq = 0.0f;

        float run(const float x, const float fc, const float k, const float sr,
                  const SvfTap tap) noexcept {
            const float g = std::tan(3.14159265f * fc / sr);
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float v1 = a1 * (ic1eq + g * (x - ic2eq));
            const float v2 = ic2eq + g * v1;
            // Não linearidade NO LAÇO: satura o estado, não só a saída.
            // Isso é o que prende a auto-oscilação (k negativo) num
            // ciclo-limite estável em vez de estourar pra NaN - mesmo
            // princípio da realimentação não-linear de um filtro analógico.
            ic1eq = 2.0f * saturate(v1) - ic1eq;
            ic2eq = 2.0f * saturate(v2) - ic2eq;
            switch (tap) {
            case SvfTap::Low: return v2;
            case SvfTap::Band: return v1;
            case SvfTap::High: return x - k * v1 - v2;
            }
            return v2;
        }

        static float saturate(const float v) noexcept {
            // linear até ~1, tanh depois - transparente em nível normal.
            if (v > 1.0f) return 1.0f + std::tanh(v - 1.0f);
            if (v < -1.0f) return -1.0f + std::tanh(v + 1.0f);
            return v;
        }
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float softLimit(const float v) noexcept {
        // limitador suave: linear até ~0,8, tanh depois. Segura a
        // auto-oscilação sem cortar duro.
        if (v > 0.8f) return 0.8f + 0.2f * std::tanh((v - 0.8f) * 5.0f);
        if (v < -0.8f) return -0.8f + 0.2f * std::tanh((v + 0.8f) * 5.0f);
        return v;
    }

    std::array<Svf, AudioBlock::maxChannels * 3> sisters_{};
    float smoothCutoff_ = 800.0f;
    float paramCoeff_ = 0.0f;
    float nyquistLimit_ = 23520.0f;
};

}  // namespace rasgo::modular
