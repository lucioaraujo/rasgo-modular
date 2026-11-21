#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>

// ============================================================================
// SUM — soma de sinais com atenuversores (Módulo 104)
// ============================================================================
//
// Auditoria 2 (§3.1, 20/11/2026): no Modular uma SAÍDA já alimenta vários
// cabos, mas uma ENTRADA aceita um cabo só (`SignalGraph`: "a soma é feita
// por um módulo Sum explícito") — e não havia esse módulo: somar duas CVs
// pedia um MIXER, um VCA4, a MATRIX ou o ABACUS. O SUM é o fan-in explícito
// que o núcleo sempre pressupôs. Feito em 21/11/2026 ("faça as
// implementações que faltam"); o MULT fica como está (patches e seeds).
//
// Quatro entradas, cada uma com um atenuversor (−2…+2, padrão +1), mais
// OFFSET (−1…+1). Saídas: SUM, INV (−SUM) e AVG (a média das entradas
// ligadas, × os atenuversores). SAT (0/1) troca o teto duro de ±8 por uma
// saturação suave ±1 — útil para somar áudio sem estourar o resto do patch.
// Serve a CV e a áudio (taxa de amostra, sem atraso, sem alocação).

namespace rasgo::modular {

class Summer final : public Signal {
public:
    Summer()
        : Signal(
              {{"in1", PortKind::Control, ""},
               {"in2", PortKind::Control, ""},
               {"in3", PortKind::Control, ""},
               {"in4", PortKind::Control, ""}},
              {{"sum", PortKind::Control, ""},
               {"inv", PortKind::Control, ""},
               {"avg", PortKind::Control, ""}},
              {{"amt1", -2.0f, 2.0f, 1.0f, ""},
               {"amt2", -2.0f, 2.0f, 1.0f, ""},
               {"amt3", -2.0f, 2.0f, 1.0f, ""},
               {"amt4", -2.0f, 2.0f, 1.0f, ""},
               {"offset", -1.0f, 1.0f, 0.0f, ""},
               {"soft", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "SUM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "SUM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "sum", "", 2.5f, 6.0f, 35.0f);
        for (int k = 0; k < 4; ++k) {
            const float y = 30.0f + static_cast<float>(k) * 15.0f;
            const std::string n = std::to_string(k + 1);
            p.add(Widget::Kind::Jack, "IN" + n, "in:in" + n, 7.0f, y);
            p.add(Widget::Kind::Knob, "A" + n, "amt" + n, 24.0f, y);
        }
        p.add(Widget::Kind::Knob, "OFS", "offset", 7.0f, 92.0f);
        p.add(Widget::Kind::Toggle, "SAT", "soft", 24.0f, 89.0f);
        p.add(Widget::Kind::Jack, "SUM", "out:sum", 5.0f, 114.0f);
        p.add(Widget::Kind::Jack, "INV", "out:inv", 18.0f, 114.0f);
        p.add(Widget::Kind::Jack, "AVG", "out:avg", 31.0f, 114.0f);
        return p;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        float amt[4];
        int connected = 0;
        for (int k = 0; k < 4; ++k) {
            amt[k] = clampf(parameterValue(kAmt[k]), -2.0f, 2.0f);
            if (inputs.size() > static_cast<std::size_t>(k) && inputs[static_cast<std::size_t>(k)] != nullptr)
                ++connected;
        }
        const float offset = clampf(parameterValue("offset"), -1.0f, 1.0f);
        const bool soft = parameterValue("soft") >= 0.5f;

        for (std::size_t f = 0; f < frames; ++f) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k) {
                const AudioBlock* in = inputs.size() > static_cast<std::size_t>(k)
                    ? inputs[static_cast<std::size_t>(k)] : nullptr;
                if (in == nullptr) continue;
                float x = in->at(0, f);
                if (!std::isfinite(x)) x = 0.0f;
                s += amt[k] * x;
            }
            const float avgRaw = connected > 0 ? s / static_cast<float>(connected) : 0.0f;
            s += offset;
            const float avg = avgRaw + offset;
            const float sum = soft ? std::tanh(s) : clampf(s, -8.0f, 8.0f);
            const float av = soft ? std::tanh(avg) : clampf(avg, -8.0f, 8.0f);
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = sum;
                outputs[1].at(c, f) = -sum;
                outputs[2].at(c, f) = av;
            }
        }
    }

private:
    static constexpr const char* kAmt[4] = {"amt1", "amt2", "amt3", "amt4"};
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
};

}  // namespace rasgo::modular
