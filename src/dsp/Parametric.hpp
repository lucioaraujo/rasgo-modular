#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>

// ============================================================================
// PARAMETRIC — equalizador paramétrico multi-estágio (Módulo 13)
// ============================================================================
//
// A ferramenta de precisão do espectro, irmã do FILTER (que é as "três
// irmãs" expressivas). Quatro estágios de biquad em SÉRIE, cada um com
// tipo, frequência, ganho e Q próprios - de corte a realce a shelf.
// Modelado a partir da ficha de recursos do VCV Parametra (8 filtros CV,
// 17 tipos, VCA de saída, soft-clip), mas escrito do zero a partir da
// teoria pública (RBJ Audio EQ Cookbook) - o código do Parametra é
// fechado e não foi consultado.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/13_parametric.md`.
//
// Fontes ESTUDADAS (fórmulas públicas, não código):
//   - Robert Bristow-Johnson, "Cookbook formulae for audio EQ biquad
//     filter coefficients" (a referência canônica de todo EQ digital):
//     LP/HP/lowshelf/highshelf/peak/notch a partir de w0, Q e ganho;
//   - VCV Parametra (ficha pública de recursos, sem código) - a ideia
//     de N biquads em série CV-controláveis com VCA e soft-clip;
//   - cascata de biquads para inclinações de 24/48 dB/oct.
//
// Desvio Rasgo: Q contínuo (não os degraus 1/2/4/8/16 do Parametra);
// `sweep` desloca todos os estágios como um grupo (a relação entre as
// bandas como processo, Warps/Atlas §39); `amount` escala todos os
// ganhos. Determinístico (sem RNG). Coeficientes recalculados por bloco.

namespace rasgo::modular {

class Parametric final : public Signal {
public:
    // 0 Off · 1 LowCut · 2 LowShelf · 3 Peak · 4 HighShelf · 5 HighCut
    Parametric()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"sweep", PortKind::Control, "v/oct"},
               {"amount", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"type1", 0.0f, 5.0f, 1.0f, ""},
               {"freq1", 20.0f, 20000.0f, 80.0f, "Hz"},
               {"gain1", -24.0f, 24.0f, 0.0f, "dB"},
               {"q1", 0.1f, 10.0f, 0.7f, ""},
               {"slope1", 1.0f, 3.0f, 1.0f, ""},  // 1=12 2=24 3=48 dB/oct (só cortes)
               {"type2", 0.0f, 5.0f, 3.0f, ""},
               {"freq2", 20.0f, 20000.0f, 300.0f, "Hz"},
               {"gain2", -24.0f, 24.0f, 0.0f, "dB"},
               {"q2", 0.1f, 10.0f, 1.0f, ""},
               {"slope2", 1.0f, 3.0f, 1.0f, ""},
               {"type3", 0.0f, 5.0f, 3.0f, ""},
               {"freq3", 20.0f, 20000.0f, 1500.0f, "Hz"},
               {"gain3", -24.0f, 24.0f, 0.0f, "dB"},
               {"q3", 0.1f, 10.0f, 1.0f, ""},
               {"slope3", 1.0f, 3.0f, 1.0f, ""},
               {"type4", 0.0f, 5.0f, 4.0f, ""},
               {"freq4", 20.0f, 20000.0f, 6000.0f, "Hz"},
               {"gain4", -24.0f, 24.0f, 0.0f, "dB"},
               {"q4", 0.1f, 10.0f, 0.7f, ""},
               {"slope4", 1.0f, 3.0f, 1.0f, ""},
               {"output", -24.0f, 24.0f, 0.0f, "dB"},
               {"drive", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "PARAMETRIC"; }

    // Layout: 22 HP - uma fileira por estágio.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 4 estágios (linha = tipo/freq/gain/q/slope), macros e I/O embaixo
        Panel p;
        p.hp = 22;
        p.add(Widget::Kind::Label, "PARAMETRIC", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "spectrum", "", 2.5f, 6.0f, 106.8f);
        const char* rows[4] = {"1", "2", "3", "4"};
        for (int s = 0; s < 4; ++s) {
            const float y = 27.0f + static_cast<float>(s) * 16.0f;
            p.add(Widget::Kind::Knob, rows[s], "type" + std::string(rows[s]), 8.0f, y);
            p.add(Widget::Kind::Knob, "FREQ", "freq" + std::string(rows[s]), 28.0f, y);
            p.add(Widget::Kind::Knob, "GAIN", "gain" + std::string(rows[s]), 48.0f, y);
            p.add(Widget::Kind::Knob, "Q", "q" + std::string(rows[s]), 68.0f, y);
            p.add(Widget::Kind::Knob, "SLP", "slope" + std::string(rows[s]), 88.0f, y);
        }
        p.add(Widget::Kind::Knob, "OUT", "output", 8.0f, 94.0f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 28.0f, 94.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 48.0f, 94.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 118.0f);
        p.add(Widget::Kind::Jack, "SWP", "in:sweep", 17.0f, 118.0f);
        p.add(Widget::Kind::Jack, "AMT", "in:amount", 29.0f, 118.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 41.0f, 118.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        for (auto& st : stages_)
            st = Stage{};
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* in = inputs[0];
        const AudioBlock* sweepIn = inputs[1];
        const AudioBlock* amountIn = inputs[2];

        const float sweepOct = sweepIn ? sweepIn->at(0, 0) : 0.0f;
        const float amount =
            1.0f + (amountIn ? amountIn->at(0, 0) : 0.0f);  // escala os ganhos
        const float freqScale = std::exp2(clampf(sweepOct, -4.0f, 4.0f));

        // recalcula os coeficientes uma vez por bloco
        for (int s = 0; s < kStages; ++s) {
            const std::string n = std::to_string(s + 1);
            const int t = clampi(
                static_cast<int>(std::lround(parameterValue("type" + n))), 0, 5);
            const float f = clampf(parameterValue("freq" + n) * freqScale,
                                   15.0f, sampleRate_ * 0.45f);
            const float g = parameterValue("gain" + n) * amount;
            const float q = clampf(parameterValue("q" + n), 0.05f, 20.0f);
            const int slp = clampi(
                static_cast<int>(std::lround(parameterValue("slope" + n))), 1, 3);
            // cortes: 12/24/48 dB/oct = 1/2/4 biquads em cascata
            const int sections = (t == 1 || t == 5)
                ? (slp == 1 ? 1 : (slp == 2 ? 2 : 4))
                : 1;
            updateStage(stages_[s], t, f, g, q);
            if (stages_[s].sections != sections) {
                for (int k = 0; k < kMaxSections; ++k)
                    stages_[s].z1[k] = stages_[s].z2[k] = 0.0f;
                stages_[s].sections = sections;
            }
        }

        const float outGain = dbToGain(parameterValue("output"));
        const float drive = parameterValue("drive");
        const float driveGain = 1.0f + drive * 5.0f;
        const float mix = parameterValue("mix");

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float dry = in ? in->at(0, frame) : 0.0f;
            float x = dry;
            for (auto& st : stages_) {
                if (!st.active)
                    continue;
                for (int k = 0; k < st.sections; ++k) {
                    const float y = st.b0 * x + st.z1[k];
                    st.z1[k] = st.b1 * x - st.a1 * y + st.z2[k];
                    st.z2[k] = st.b2 * x - st.a2 * y;
                    x = y;
                }
            }
            x *= outGain;
            if (drive > 0.0f)
                x = std::tanh(x * driveGain) / std::tanh(driveGain);
            // soft-clip de segurança (±1, como o ±5V do Parametra em escala)
            x = softClip(x);

            const float outValue = mix * x + (1.0f - mix) * dry;
            for (std::size_t channel = 0; channel < channels; ++channel)
                out.at(channel, frame) = outValue;
        }
    }

private:
    static constexpr int kStages = 4;
    static constexpr int kMaxSections = 4;  // 48 dB/oct = 4 biquads em cascata
    struct Stage {
        bool active = false;
        int sections = 1;  // 1/2/4 pra 12/24/48 dB/oct (só cortes)
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1[kMaxSections] = {};  // Direct Form II transposta, por seção
        float z2[kMaxSections] = {};
    };

    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float dbToGain(const float db) noexcept {
        return std::pow(10.0f, db / 20.0f);
    }
    // Soft-clip de SEGURANÇA: transparente até ±1 (não colore o EQ),
    // satura suave acima (o ±5V do Parametra, em escala normalizada).
    static float softClip(const float x) noexcept {
        if (x > 1.0f) return 1.0f + std::tanh(x - 1.0f) * 0.4f;
        if (x < -1.0f) return -1.0f + std::tanh(x + 1.0f) * 0.4f;
        return x;
    }

    // Fórmulas RBJ (Cookbook). `type`: 1 LowCut, 2 LowShelf, 3 Peak,
    // 4 HighShelf, 5 HighCut. Coeficientes já normalizados por a0.
    void updateStage(Stage& st, const int type, const float f0,
                     const float gainDb, const float q) noexcept {
        if (type == 0) {
            st.active = false;
            return;
        }
        st.active = true;
        const float w = 6.2831853f * f0 / sampleRate_;
        const float cw = std::cos(w);
        const float sw = std::sin(w);
        const float A = std::pow(10.0f, gainDb / 40.0f);
        // alpha: forma "Q" para peak/cortes, forma "slope S" para shelves
        // (RBJ Cookbook) - `q` reinterpretado como slope nas shelves.
        float alpha;
        if (type == 2 || type == 4) {
            const float S = clampf(q > 1.0f ? 1.0f : q, 0.05f, 1.0f);
            alpha = 0.5f * sw
                * std::sqrt((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
        } else {
            alpha = sw / (2.0f * q);
        }
        float b0, b1, b2, a0, a1, a2;
        switch (type) {
        case 1:  // LowCut (HPF 12 dB)
            b0 = (1.0f + cw) * 0.5f;
            b1 = -(1.0f + cw);
            b2 = (1.0f + cw) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cw;
            a2 = 1.0f - alpha;
            break;
        case 5:  // HighCut (LPF 12 dB)
            b0 = (1.0f - cw) * 0.5f;
            b1 = 1.0f - cw;
            b2 = (1.0f - cw) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cw;
            a2 = 1.0f - alpha;
            break;
        case 2: {  // LowShelf
            const float ta = 2.0f * std::sqrt(A) * alpha;
            b0 = A * ((A + 1.0f) - (A - 1.0f) * cw + ta);
            b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw);
            b2 = A * ((A + 1.0f) - (A - 1.0f) * cw - ta);
            a0 = (A + 1.0f) + (A - 1.0f) * cw + ta;
            a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cw);
            a2 = (A + 1.0f) + (A - 1.0f) * cw - ta;
            break;
        }
        case 4: {  // HighShelf
            const float ta = 2.0f * std::sqrt(A) * alpha;
            b0 = A * ((A + 1.0f) + (A - 1.0f) * cw + ta);
            b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw);
            b2 = A * ((A + 1.0f) + (A - 1.0f) * cw - ta);
            a0 = (A + 1.0f) - (A - 1.0f) * cw + ta;
            a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cw);
            a2 = (A + 1.0f) - (A - 1.0f) * cw - ta;
            break;
        }
        case 3:
        default: {  // Peak
            b0 = 1.0f + alpha * A;
            b1 = -2.0f * cw;
            b2 = 1.0f - alpha * A;
            a0 = 1.0f + alpha / A;
            a1 = -2.0f * cw;
            a2 = 1.0f - alpha / A;
            break;
        }
        }
        const float inv = 1.0f / a0;
        st.b0 = b0 * inv;
        st.b1 = b1 * inv;
        st.b2 = b2 * inv;
        st.a1 = a1 * inv;
        st.a2 = a2 * inv;
    }

    Stage stages_[kStages];
};

}  // namespace rasgo::modular
