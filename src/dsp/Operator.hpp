#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// OPERATOR — voz FM multi-operador (Módulo 44)
// ============================================================================
//
// O `OSC` faz TZFM linear de UM par. `OPERATOR` é FM de verdade: 4
// operadores (senóides), 8 algoritmos de roteamento, razões de frequência
// QUANTIZADAS a um conjunto musical, e realimentação no operador A.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/44_operator.md`.
//
// - `algo` (0–7)        qual operador modula qual (ordem fixa A→B→C→D)
// - `ratio_b/c/d` (0–9) razão de B/C/D vs A, quantizada à tabela musical
// - `index` (0–1, +CV)  profundidade de modulação global (0 = 4 senóides)
// - `feedback` (0–1)    A modula a própria fase (média de 2 amostras, DX7)
// - `drift` (0–1)       micro-desafino lento por operador, DETERMINÍSTICO
//
// Afinação padrão do `OSC` (`freq`/`fine`/`pitch`). Sem envelope embutido
// (é um oscilador — patch `ENVELOPE → index` e `ENVELOPE → VCA`).
//
// `prepare()` aloca a LUT de 2048; `process()` não aloca. Determinístico.

namespace rasgo::modular {

class Operator final : public Signal {
public:
    static constexpr int kLut = 2048;

    Operator()
        : Signal(
              {{"pitch", PortKind::Control, "v/oct"},
               {"index", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"freq", 8.0f, 8000.0f, 110.0f, "Hz"},
               {"fine", -100.0f, 100.0f, 0.0f, "cent"},
               {"algo", 0.0f, 7.0f, 0.0f, ""},
               {"ratio_b", 0.0f, 9.0f, 1.0f, ""},
               {"ratio_c", 0.0f, 9.0f, 3.0f, ""},
               {"ratio_d", 0.0f, 9.0f, 5.0f, ""},
               {"index", 0.0f, 1.0f, 0.3f, ""},
               {"feedback", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "OPERATOR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "OPERATOR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "fm", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 6.0f, 28.0f);
        p.add(Widget::Kind::Knob, "FINE", "fine", 19.0f, 28.0f);
        p.add(Widget::Kind::Knob, "ALGO", "algo", 32.0f, 28.0f);
        p.add(Widget::Kind::Knob, "INDEX", "index", 45.0f, 28.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RB", "ratio_b", 6.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RC", "ratio_c", 19.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RD", "ratio_d", 32.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 45.0f, 50.0f);
        p.add(Widget::Kind::Jack, "1V/O", "in:pitch", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "IDX", "in:index", 24.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        lut_.assign(kLut, 0.0f);
        for (int i = 0; i < kLut; ++i)
            lut_[static_cast<std::size_t>(i)] =
                std::sin(6.28318530718f * static_cast<float>(i)
                         / static_cast<float>(kLut));
        for (int i = 0; i < 4; ++i) { ph_[i] = 0.0f; dph_[i] = 0.0f; }
        a1_ = 0.0f;
        a2_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* pitchIn = inputs[0];
        const AudioBlock* idxIn = inputs[1];

        const float freq = parameterValue("freq");
        const float fineOct = parameterValue("fine") / 1200.0f;
        const float idxKnob = clamp01(parameterValue("index"));
        const float fbP = clamp01(parameterValue("feedback"));
        const float drift = clamp01(parameterValue("drift"));
        const float nyq = 0.5f * sr_;

        const Algo& alg = kAlgos[algoIndex(parameterValue("algo"))];
        const float rB = kRatio[ratioIndex(parameterValue("ratio_b"))];
        const float rC = kRatio[ratioIndex(parameterValue("ratio_c"))];
        const float rD = kRatio[ratioIndex(parameterValue("ratio_d"))];
        const float ratio[4] = {1.0f, rB, rC, rD};

        const float fb = fbP * fbP;   // até ~1 ciclo de auto-desvio (≈ DX7 máx)
        const int nCar = alg.carrier[0] + alg.carrier[1]
                       + alg.carrier[2] + alg.carrier[3];
        const float carScale = 1.0f / static_cast<float>(std::max(1, nCar));

        // deriva: avança 4 fases lentas por bloco
        static const float dInc[4] = {0.021f, 0.029f, 0.037f, 0.045f};
        for (int i = 0; i < 4; ++i) {
            dph_[i] += dInc[i] * static_cast<float>(frames) * dt_;
            dph_[i] -= std::floor(dph_[i]);
        }
        float det[4];
        for (int i = 0; i < 4; ++i)
            det[i] = 0.004f * drift
                   * std::sin(6.28318530718f * dph_[i]
                              + static_cast<float>(i) * 1.7f);

        for (std::size_t f = 0; f < frames; ++f) {
            const float pOct = pitchIn ? pitchIn->at(0, f) : 0.0f;
            float f0 = freq * std::exp2(fineOct + pOct);
            f0 = clampf(f0, 0.01f, 0.45f * (2.0f * nyq));

            const float idx = clamp01(idxKnob + (idxIn ? idxIn->at(0, f) : 0.0f));
            const float depth = idx * idx * 6.0f;

            for (int i = 0; i < 4; ++i) {
                ph_[i] += f0 * ratio[i] * (1.0f + det[i]) * dt_;
                ph_[i] -= std::floor(ph_[i]);
            }

            const float A = lut(ph_[0] + fb * 0.5f * (a1_ + a2_));
            const float B = lut(ph_[1] + depth * (alg.aToB ? A : 0.0f));
            const float C = lut(ph_[2] + depth * ((alg.aToC ? A : 0.0f)
                                                  + (alg.bToC ? B : 0.0f)));
            const float D = lut(ph_[3] + depth * ((alg.aToD ? A : 0.0f)
                                                  + (alg.bToD ? B : 0.0f)
                                                  + (alg.cToD ? C : 0.0f)));
            a2_ = a1_;
            a1_ = A;

            float sum = 0.0f;
            if (alg.carrier[0]) sum += A;
            if (alg.carrier[1]) sum += B;
            if (alg.carrier[2]) sum += C;
            if (alg.carrier[3]) sum += D;
            // soma ÷ nº de portadoras → |·| ≤ 1 (senóides); softclip é só a
            // rede de segurança pro feedback e pra portadoras em fase.
            const float y = softclip(sum * carScale);

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
        }
    }

private:
    struct Algo {
        bool aToB, aToC, aToD, bToC, bToD, cToD;
        bool carrier[4];
    };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    static int algoIndex(const float v) noexcept {
        int i = static_cast<int>(v + 0.5f);
        return i < 0 ? 0 : (i > 7 ? 7 : i);
    }
    static int ratioIndex(const float v) noexcept {
        int i = static_cast<int>(v + 0.5f);
        return i < 0 ? 0 : (i > 9 ? 9 : i);
    }

    // transparente até |v|=1 (soma ÷ nº portadoras já é ≤ 1); assíntota ±1,5
    static float softclip(const float v) noexcept {
        const float k = 0.5f;
        if (v > 1.0f) return 1.0f + k * std::tanh((v - 1.0f) / k);
        if (v < -1.0f) return -1.0f - k * std::tanh((-v - 1.0f) / k);
        return v;
    }

    float lut(float x) const noexcept {
        x -= std::floor(x);
        const float p = x * static_cast<float>(kLut);
        int i = static_cast<int>(p);
        const float fr = p - static_cast<float>(i);
        i &= (kLut - 1);
        const int j = (i + 1) & (kLut - 1);
        return lut_[static_cast<std::size_t>(i)] * (1.0f - fr)
             + lut_[static_cast<std::size_t>(j)] * fr;
    }

    static constexpr float kRatio[10] = {
        0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f};

    // 8 algoritmos — ver dossiê §3. {aToB,aToC,aToD,bToC,bToD,cToD},{carA..D}
    static constexpr Algo kAlgos[8] = {
        {1,0,0,1,0,1, {0,0,0,1}},   // 0  A→B→C→D
        {1,0,0,1,0,0, {0,0,1,1}},   // 1  A→B→C , D
        {1,0,0,0,0,1, {0,1,0,1}},   // 2  A→B , C→D
        {0,1,0,1,0,1, {0,0,0,1}},   // 3  A→C , B→C→D
        {0,0,1,0,1,1, {0,0,0,1}},   // 4  A→D , B→D , C→D
        {1,0,0,0,0,0, {0,1,1,1}},   // 5  A→B , C , D
        {1,1,1,0,0,0, {0,1,1,1}},   // 6  A→B , A→C , A→D
        {0,0,0,0,0,0, {1,1,1,1}},   // 7  A , B , C , D  (aditivo)
    };

    std::vector<float> lut_;
    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float ph_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float dph_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float a1_ = 0.0f, a2_ = 0.0f;
};

}  // namespace rasgo::modular
