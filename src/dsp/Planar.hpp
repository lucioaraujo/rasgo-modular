#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// PLANAR — morph vetorial XY (Módulo 43)
// ============================================================================
//
// Quatro fontes de áudio nos cantos de um quadrado; um ponto `x`/`y`
// interpola entre elas (bilinear). O ponto vem dos knobs + CV, de um
// GESTO gravado que reproduz em loop, ou de uma deriva autônoma — e a
// posição efetiva SAI como CV (`x_out`/`y_out`) pra dirigir outros
// módulos. "A relação é o processo" (identidade RASGO).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/43_planar.md`.
//
// - `x`/`y` (0–1, + CV) = posição no quadrado (A=canto sup-esq, B=sup-dir,
//   C=inf-esq, D=inf-dir);
// - `curve` (0–1) = mistura linear (0) ↔ potência constante (1) — linear
//   pra morph de CV, potência constante pra áudio (não afunda no centro);
// - `smooth` (0–1) = glide no ponto (τ de ~1 ms a ~0,5 s);
// - `rate` (0–1, 0,5 = 1×) = velocidade do loop do gesto E da deriva;
// - `drift` (0–1) = passeio 2D autônomo do ponto quando não há gesto —
//   soma de senóides incomensuráveis, DETERMINÍSTICO (sem RNG).
//
// GESTO: enquanto o gate `gesture` está alto, grava o ponto (decimado
// 32×). Na descida, se gravou o bastante, o gesto passa a tocar em loop
// (knob/CV viram nudge bipolar). Toque curto = limpa (volta ao ao vivo).
//
// `prepare()` aloca ~48 KB (2 buffers de gesto); `process()` não aloca.
// Determinístico sempre.

namespace rasgo::modular {

class Planar final : public Signal {
public:
    static constexpr int kGest = 6000;   // quadros decimados (~4 s a 48 k / 32)
    static constexpr int kDecim = 32;

    Planar()
        : Signal(
              {{"a", PortKind::Audio, ""},
               {"b", PortKind::Audio, ""},
               {"c", PortKind::Audio, ""},
               {"d", PortKind::Audio, ""},
               {"x", PortKind::Control, ""},
               {"y", PortKind::Control, ""},
               {"gesture", PortKind::Control, "gate"}},
              {{"out", PortKind::Audio, ""},
               {"x_out", PortKind::Control, ""},
               {"y_out", PortKind::Control, ""}},
              {{"x", 0.0f, 1.0f, 0.5f, ""},
               {"y", 0.0f, 1.0f, 0.5f, ""},
               {"curve", 0.0f, 1.0f, 0.5f, ""},
               {"smooth", 0.0f, 1.0f, 0.0f, ""},
               {"rate", 0.0f, 1.0f, 0.5f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "PLANAR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "PLANAR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "xy", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "X", "x", 8.0f, 28.0f);
        p.add(Widget::Kind::Knob, "Y", "y", 28.0f, 28.0f);
        p.add(Widget::Kind::Knob, "CURVE", "curve", 48.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SMTH", "smooth", 8.0f, 46.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 28.0f, 46.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 48.0f, 46.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 7.0f, 66.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 19.0f, 66.0f);
        p.add(Widget::Kind::Jack, "C", "in:c", 31.0f, 66.0f);
        p.add(Widget::Kind::Jack, "D", "in:d", 43.0f, 66.0f);
        p.add(Widget::Kind::Jack, "X", "in:x", 7.0f, 86.0f);
        p.add(Widget::Kind::Jack, "Y", "in:y", 19.0f, 86.0f);
        p.add(Widget::Kind::Jack, "GST", "in:gesture", 33.0f, 86.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 110.0f);
        p.add(Widget::Kind::Jack, "X'", "out:x_out", 21.0f, 110.0f);
        p.add(Widget::Kind::Jack, "Y'", "out:y_out", 33.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;

        gx_.assign(kGest, 0.5f);
        gy_.assign(kGest, 0.5f);
        recPos_ = 0;
        recLen_ = 0;
        decimCount_ = 0;
        playPos_ = 0.0f;
        recording_ = false;
        hasGesture_ = false;
        prevGate_ = 0.0f;

        smX_ = clamp01(parameterValue("x"));
        smY_ = clamp01(parameterValue("y"));
        driftPh1_ = 0.0f;
        driftPh2_ = 0.0f;
        driftPh3_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& xOut = outputs[1];
        AudioBlock& yOut = outputs[2];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* aIn = inputs[0];
        const AudioBlock* bIn = inputs[1];
        const AudioBlock* cIn = inputs[2];
        const AudioBlock* dIn = inputs[3];
        const AudioBlock* xIn = inputs[4];
        const AudioBlock* yIn = inputs[5];
        const AudioBlock* gIn = inputs[6];

        const float xKnob = clamp01(parameterValue("x"));
        const float yKnob = clamp01(parameterValue("y"));
        const float curve = clamp01(parameterValue("curve"));
        const float smooth = clamp01(parameterValue("smooth"));
        const float rate = clamp01(parameterValue("rate"));
        const float drift = clamp01(parameterValue("drift"));

        const float rateMul = std::exp2((rate - 0.5f) * 4.0f);   // 1/16 .. 16×
        const float tau = 0.00006f + smooth * smooth * 0.5f;   // ~instantâneo a ~0,5 s
        const float smCoef = 1.0f - std::exp(-dt_ / tau);
        const float playInc = rateMul / static_cast<float>(kDecim);
        const float driftAmp = drift * 0.35f;
        const float d1 = 0.030f * rateMul * dt_;
        const float d2 = 0.022f * rateMul * dt_;
        const float d3 = 0.041f * rateMul * dt_;

        for (std::size_t f = 0; f < frames; ++f) {
            const float xCv = xIn ? xIn->at(0, f) : 0.0f;
            const float yCv = yIn ? yIn->at(0, f) : 0.0f;
            const float gate = gIn ? gIn->at(0, f) : 0.0f;

            // --- máquina de estado do gesto (borda em taxa de amostra) ---
            if (gate >= 0.5f && prevGate_ < 0.5f) {
                recording_ = true;
                recPos_ = 0;
                decimCount_ = 0;
            } else if (gate < 0.5f && prevGate_ >= 0.5f && recording_) {
                recording_ = false;
                if (recPos_ >= 3) {
                    recLen_ = recPos_;
                    hasGesture_ = true;
                    playPos_ = 0.0f;
                } else {
                    hasGesture_ = false;   // toque curto = limpa
                }
            }
            prevGate_ = gate;

            // --- deriva 2D autônoma (determinística) ---
            driftPh1_ += d1; driftPh1_ -= std::floor(driftPh1_);
            driftPh2_ += d2; driftPh2_ -= std::floor(driftPh2_);
            driftPh3_ += d3; driftPh3_ -= std::floor(driftPh3_);
            const float twoPi = 6.28318530718f;
            const float drX = driftAmp * (0.6f * std::sin(twoPi * driftPh1_)
                                          + 0.4f * std::sin(twoPi * driftPh3_ + 1.3f));
            const float drY = driftAmp * (0.6f * std::sin(twoPi * driftPh2_ + 0.7f)
                                          + 0.4f * std::sin(twoPi * driftPh1_ * 1.37f + 2.1f));

            // --- ponto alvo ---
            float tx, ty;
            if (recording_) {
                tx = clamp01(xKnob + xCv);
                ty = clamp01(yKnob + yCv);
                if (++decimCount_ >= kDecim) {
                    decimCount_ = 0;
                    if (recPos_ < kGest) {
                        gx_[static_cast<std::size_t>(recPos_)] = tx;
                        gy_[static_cast<std::size_t>(recPos_)] = ty;
                        ++recPos_;
                    }
                }
            } else if (hasGesture_) {
                playPos_ += playInc;
                const float len = static_cast<float>(recLen_);
                while (playPos_ >= len) playPos_ -= len;
                while (playPos_ < 0.0f) playPos_ += len;
                const float gxv = lerpGest(gx_, playPos_);
                const float gyv = lerpGest(gy_, playPos_);
                // com gesto tocando o knob é ignorado; a CV ainda dá nudge
                tx = clamp01(gxv + xCv);
                ty = clamp01(gyv + yCv);
            } else {
                tx = clamp01(xKnob + xCv + drX);
                ty = clamp01(yKnob + yCv + drY);
            }

            smX_ += (tx - smX_) * smCoef;
            smY_ += (ty - smY_) * smCoef;

            // --- blend bilinear ---
            const float x = smX_, y = smY_;
            const float wa = (1.0f - x) * (1.0f - y);
            const float wb = x * (1.0f - y);
            const float wc = (1.0f - x) * y;
            const float wd = x * y;
            const float ssq = wa * wa + wb * wb + wc * wc + wd * wd;
            const float g = 1.0f / std::sqrt(std::max(ssq, 1.0e-6f));
            const float scale = 1.0f + curve * (g - 1.0f);

            const float sa = aIn ? aIn->at(0, f) : 0.0f;
            const float sb = bIn ? bIn->at(0, f) : 0.0f;
            const float sc = cIn ? cIn->at(0, f) : 0.0f;
            const float sd = dIn ? dIn->at(0, f) : 0.0f;
            const float raw = scale * (wa * sa + wb * sb + wc * sc + wd * sd);
            const float y0 = softclip(raw);

            for (std::size_t ch = 0; ch < channels; ++ch) {
                out.at(ch, f) = y0;
                xOut.at(ch, f) = smX_;
                yOut.at(ch, f) = smY_;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // transparente até |v| = 1 (morph linear de fontes ≤ 1 nunca passa disso);
    // só a potência constante pode estourar — aí satura suave, assíntota ±1,5.
    static float softclip(const float v) noexcept {
        const float k = 0.5f;
        if (v > 1.0f) return 1.0f + k * std::tanh((v - 1.0f) / k);
        if (v < -1.0f) return -1.0f - k * std::tanh((-v - 1.0f) / k);
        return v;
    }

    float lerpGest(const std::vector<float>& buf, const float pos) const noexcept {
        int i0 = static_cast<int>(pos);
        if (i0 < 0) i0 = 0;
        if (i0 >= recLen_) i0 = recLen_ - 1;
        int i1 = i0 + 1;
        if (i1 >= recLen_) i1 = 0;
        const float fr = pos - std::floor(pos);
        return buf[static_cast<std::size_t>(i0)] * (1.0f - fr)
             + buf[static_cast<std::size_t>(i1)] * fr;
    }

    std::vector<float> gx_, gy_;
    int recPos_ = 0, recLen_ = 0, decimCount_ = 0;
    float playPos_ = 0.0f;
    bool recording_ = false, hasGesture_ = false;
    float prevGate_ = 0.0f;

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float smX_ = 0.5f, smY_ = 0.5f;
    float driftPh1_ = 0.0f, driftPh2_ = 0.0f, driftPh3_ = 0.0f;
};

}  // namespace rasgo::modular
