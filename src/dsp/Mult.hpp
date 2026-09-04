#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>

// ============================================================================
// MULT — Múltiplo processado (Módulo 34)
// ============================================================================
//
// No grafo digital o fan-out já é livre — um múltiplo que só COPIA não faz
// nada que um cabo não faça. Este PROCESSA cada saída: 1 entrada → 4 saídas,
// cada uma com atenuversor + offset próprios (um mini-CONTROL por tomada).
// É o distribuidor de CV. Ocioso (sem `in`), vira 4 fontes de tensão manual.
// Modo `dual` (Doepfer A-180-2): a 1ª entrada alimenta out1/out2, a 2ª
// (`in2`) alimenta out3/out4 — dois múltiplos de 1→2 num painel só.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/34_mult.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - múltiplo bufferizado (Doepfer A-180, Intellijel Buff Mult);
//   - atenuversor + offset (Maths, Serge, `CONTROL` do Rasgo);
//   - "voltage spreader" (Frap Tools, Doepfer A-138s).
//
// Determinístico, sem RNG.

namespace rasgo::modular {

class Mult final : public Signal {
public:
    Mult()
        : Signal(
              {{"in", PortKind::Control, ""},
               {"in2", PortKind::Control, ""}},
              {{"out1", PortKind::Control, ""},
               {"out2", PortKind::Control, ""},
               {"out3", PortKind::Control, ""},
               {"out4", PortKind::Control, ""}},
              {{"dual", 0.0f, 1.0f, 0.0f, ""},
               {"scale1", -2.0f, 2.0f, 1.0f, ""},
               {"offset1", -1.0f, 1.0f, 0.0f, ""},
               {"scale2", -2.0f, 2.0f, 1.0f, ""},
               {"offset2", -1.0f, 1.0f, 0.0f, ""},
               {"scale3", -2.0f, 2.0f, 1.0f, ""},
               {"offset3", -1.0f, 1.0f, 0.0f, ""},
               {"scale4", -2.0f, 2.0f, 1.0f, ""},
               {"offset4", -1.0f, 1.0f, 0.0f, ""},
               {"slew", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "MULT"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "MULT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "taps", "", 2.5f, 7.0f, 44.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 28.0f);
        p.add(Widget::Kind::Jack, "IN2", "in:in2", 20.0f, 28.0f);
        p.add(Widget::Kind::Toggle, "DUAL", "dual", 30.0f, 25.0f);
        p.add(Widget::Kind::Knob, "SLEW", "slew", 40.0f, 30.0f);
        for (int k = 0; k < 4; ++k) {
            const float y = 44.0f + static_cast<float>(k) * 16.0f;
            const std::string n = std::to_string(k + 1);
            p.add(Widget::Kind::Knob, "SCL" + n, "scale" + n, 8.0f, y);
            p.add(Widget::Kind::Knob, "OFF" + n, "offset" + n, 24.0f, y);
        }
        for (int k = 0; k < 4; ++k)
            p.add(Widget::Kind::Jack, "O" + std::to_string(k + 1),
                  "out:out" + std::to_string(k + 1),
                  7.0f + static_cast<float>(k) * 11.0f, 112.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        for (auto& v : y_) v = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        float scale[4], offset[4];
        for (int k = 0; k < 4; ++k) {
            const std::string n = std::to_string(k + 1);
            scale[k] = clampf(parameterValue("scale" + n), -2.0f, 2.0f);
            offset[k] = clampf(parameterValue("offset" + n), -1.0f, 1.0f);
        }
        const float slew = clamp01(parameterValue("slew"));
        const float slewTime = slew * slew * 0.5f;
        const float slewCoef =
            slewTime < dt_ ? 1.0f : (1.0f - std::exp(-dt_ / slewTime));
        const bool dual = parameterValue("dual") >= 0.5f;

        const AudioBlock* in = inputs[0];
        const AudioBlock* in2 = inputs[1];

        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in != nullptr ? in->at(0, f) : 0.0f;
            // modo dual: out3/out4 seguem a 2ª entrada (Doepfer A-180-2)
            const float x2 = dual
                ? (in2 != nullptr ? in2->at(0, f) : 0.0f)
                : x;
            for (int k = 0; k < 4; ++k) {
                const float src = k < 2 ? x : x2;
                const float tgt = clampf(scale[k] * src + offset[k], -8.0f, 8.0f);
                y_[k] += (tgt - y_[k]) * slewCoef;
                for (std::size_t c = 0; c < channels; ++c)
                    outputs[static_cast<std::size_t>(k)].at(c, f) = y_[k];
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    float y_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

}  // namespace rasgo::modular
