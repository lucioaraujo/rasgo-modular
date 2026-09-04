#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// CONTROL — utilidades de CV (Módulo 21)
// ============================================================================
//
// A camada chata e essencial: pegar uma tensão e mexer nela. Atenuversor
// (escala ±), offset (constante), retificação, slew/lag (portamento,
// gate→rampa, seguidor de envelope) e uma saída de SOMA/média dos dois
// canais. DUPLO — como o VCA, um rack quer pelo menos dois.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/21_control.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Make Noise Maths (canais 1/4: atenuverter + offset + somador);
//   - Serge DUSG / Smooth&Stepped (slew com rise/fall, retificação);
//   - seguidor de envelope RC clássico (retifica + passa-baixa);
//   - slew de inclinação CONSTANTE (portamento MS-20/Minimoog) vs RC —
//     soam diferente; o knob `curve` faz o contínuo entre os dois
//     (padrão do `ENVELOPE` do Rasgo).
//
// Precisão é o padrão deste módulo (é a régua do sistema). `drift` é
// pequeno e opt-in — só pra "humanizar" um offset parado.

namespace rasgo::modular {

class Control final : public Signal {
public:
    Control()
        : Signal(
              {{"in1", PortKind::Audio, ""},
               {"in2", PortKind::Audio, ""}},
              {{"out1", PortKind::Audio, ""},
               {"out2", PortKind::Audio, ""},
               {"sum", PortKind::Audio, ""}},
              {{"scale1", -2.0f, 2.0f, 1.0f, ""},
               {"offset1", -1.0f, 1.0f, 0.0f, ""},
               {"rectify1", 0.0f, 1.0f, 0.0f, ""},
               {"slew1", 0.0f, 1.0f, 0.0f, ""},
               {"curve1", 0.0f, 1.0f, 0.0f, ""},
               {"scale2", -2.0f, 2.0f, 1.0f, ""},
               {"offset2", -1.0f, 1.0f, 0.0f, ""},
               {"rectify2", 0.0f, 1.0f, 0.0f, ""},
               {"slew2", 0.0f, 1.0f, 0.0f, ""},
               {"curve2", 0.0f, 1.0f, 0.0f, ""},
               {"sum_mode", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "CONTROL"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 2 colunas de canal (5 knobs) + coluna de DRIFT/SUM + I/O embaixo
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "CONTROL", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "out", "", 2.5f, 7.0f, 40.0f);
        const char* rows[5] = {"SCALE", "OFF", "RECT", "SLEW", "CRV"};
        const char* ids[5] = {"scale", "offset", "rectify", "slew", "curve"};
        for (int ch = 0; ch < 2; ++ch) {
            const std::string n = std::to_string(ch + 1);
            const float x = 8.0f + static_cast<float>(ch) * 20.0f;
            for (int r = 0; r < 5; ++r)
                p.add(Widget::Kind::Knob, rows[r],
                      ids[r] + n, x, 27.0f + static_cast<float>(r) * 17.0f);
        }
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 48.0f, 27.0f);
        p.add(Widget::Kind::Toggle, "SUM", "sum_mode", 48.0f, 47.0f);
        p.add(Widget::Kind::Jack, "IN1", "in:in1", 6.0f, 118.0f);
        p.add(Widget::Kind::Jack, "IN2", "in:in2", 18.0f, 118.0f);
        p.add(Widget::Kind::Jack, "O1", "out:out1", 30.0f, 118.0f);
        p.add(Widget::Kind::Jack, "O2", "out:out2", 42.0f, 118.0f);
        p.add(Widget::Kind::Jack, "SUM", "out:sum", 54.0f, 118.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        y_[0] = parameterValue("offset1");
        y_[1] = parameterValue("offset2");
        driftCur_[0] = driftCur_[1] = 0.0f;
        driftTgt_[0] = driftTgt_[1] = 0.0f;
        driftCounter_ = 0;
        driftInterval_ = static_cast<std::uint32_t>(std::max(1.0f, sr_ / 8.0f));
        rng_ = 0x2545F4914F6CDD1DULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out1 = outputs[0];
        AudioBlock& out2 = outputs[1];
        AudioBlock& sumOut = outputs[2];
        const std::size_t frames = out1.frames();
        const std::size_t channels = out1.channels();

        const float scale[2] = {parameterValue("scale1"),
                                parameterValue("scale2")};
        const float offset[2] = {parameterValue("offset1"),
                                 parameterValue("offset2")};
        const float rect[2] = {parameterValue("rectify1"),
                               parameterValue("rectify2")};
        const float curve[2] = {clamp01(parameterValue("curve1")),
                                clamp01(parameterValue("curve2"))};
        const bool average = parameterValue("sum_mode") >= 0.5f;
        const float drift = clamp01(parameterValue("drift"));

        // coeficientes de slew por bloco (um exp por canal)
        float expoCoef[2], linDelta[2];
        bool instant[2];
        for (int k = 0; k < 2; ++k) {
            const float s = clamp01(parameterValue(k == 0 ? "slew1" : "slew2"));
            const float slewTime = s * s * 2.0f;   // 0..2 s
            instant[k] = slewTime < dt_;
            if (instant[k]) { expoCoef[k] = 1.0f; linDelta[k] = 0.0f; }
            else {
                expoCoef[k] = 1.0f - std::exp(-dt_ / slewTime);
                linDelta[k] = (2.0f / slewTime) * dt_;  // ±1 em slewTime s
            }
        }

        const AudioBlock* in[2] = {inputs[0], inputs[1]};
        const float driftAmp = 0.015f * drift;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (drift > 0.0f && ++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftTgt_[0] = noise() * driftAmp;
                driftTgt_[1] = noise() * driftAmp;
            }
            // desliza a deriva pros alvos (suave, sem degrau audível)
            driftCur_[0] += (driftTgt_[0] - driftCur_[0]) * 0.002f;
            driftCur_[1] += (driftTgt_[1] - driftCur_[1]) * 0.002f;

            float o[2];
            for (int k = 0; k < 2; ++k) {
                const float x = in[k] != nullptr ? in[k]->at(0, frame) : 0.0f;
                // retificação contínua = lerp(x, |x|, rect): 0 passa,
                // 0,5 meia-onda (= max(x,0) exato: 0,5x + 0,5|x|),
                // 1 onda-completa
                const float xr = x + (std::fabs(x) - x) * rect[k];
                const float tgt = scale[k] * xr + offset[k] + driftCur_[k];

                if (instant[k]) {
                    y_[k] = tgt;
                } else {
                    const float d = tgt - y_[k];
                    const float lin =
                        y_[k] + clampf(d, -linDelta[k], linDelta[k]);
                    const float expo = y_[k] + d * expoCoef[k];
                    y_[k] = lin + (expo - lin) * curve[k];
                }
                o[k] = y_[k];
            }

            float s = o[0] + o[1];
            s = average ? s * 0.5f : clampf(s, -1.0f, 1.0f);

            for (std::size_t c = 0; c < channels; ++c) {
                out1.at(c, frame) = o[0];
                out2.at(c, frame) = o[1];
                sumOut.at(c, frame) = s;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float y_[2] = {0.0f, 0.0f};
    float driftCur_[2] = {0.0f, 0.0f};
    float driftTgt_[2] = {0.0f, 0.0f};
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 6000;
    std::uint64_t rng_ = 0x2545F4914F6CDD1DULL;
};

}  // namespace rasgo::modular
