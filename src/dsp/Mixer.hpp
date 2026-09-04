#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>

// ============================================================================
// MIXER — soma estéreo de fontes (Módulo 16)
// ============================================================================
//
// Onde as vozes viram uma imagem. 4 canais mono, cada um com ganho (dB),
// posição no campo estéreo (`pan`, lei de potência constante) e mute.
// Saída estéreo (esquerda no canal 0, direita no canal 1). Se o grafo
// estiver em mono, a saída é a soma L+R.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/16_mixer.md`.
//
// Fontes ESTUDADAS (prática de estúdio, não código):
//   - lei de pan de potência constante (sin/cos) - energia constante ao
//     varrer o pan, sem o buraco no centro da lei linear;
//   - mixer de barramento clássico - ganho por canal + soma + ganho de
//     saída; fan-in explícito (aqui um nó, não N cabos numa porta).
//
// Determinístico (sem RNG). Sem alocação.

namespace rasgo::modular {

class Mixer final : public Signal {
public:
    Mixer()
        : Signal(
              {{"ch1", PortKind::Audio, ""},
               {"ch2", PortKind::Audio, ""},
               {"ch3", PortKind::Audio, ""},
               {"ch4", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"gain1", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan1", -1.0f, 1.0f, 0.0f, ""},
               {"mute1", 0.0f, 1.0f, 0.0f, ""},
               {"gain2", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan2", -1.0f, 1.0f, 0.0f, ""},
               {"mute2", 0.0f, 1.0f, 0.0f, ""},
               {"gain3", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan3", -1.0f, 1.0f, 0.0f, ""},
               {"mute3", 0.0f, 1.0f, 0.0f, ""},
               {"gain4", -60.0f, 12.0f, 0.0f, "dB"},
               {"pan4", -1.0f, 1.0f, 0.0f, ""},
               {"mute4", 0.0f, 1.0f, 0.0f, ""},
               {"out_gain", -24.0f, 12.0f, 0.0f, "dB"}}) {}

    std::string type() const override { return "MIXER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 4 tiras de canal verticais + saída embaixo
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "MIXER", "", 2.5f, 2.0f);
        for (int c = 0; c < 4; ++c) {
            const std::string n = std::to_string(c + 1);
            const float x = 8.0f + static_cast<float>(c) * 16.0f;
            p.add(Widget::Kind::Slider, ("CH" + n), "gain" + n, x, 12.0f);
            p.add(Widget::Kind::Knob, "PAN", "pan" + n, x, 62.0f);
            p.add(Widget::Kind::Toggle, "M", "mute" + n, x, 82.0f);
            p.add(Widget::Kind::Jack, n, "in:ch" + n, x, 98.0f);
        }
        p.add(Widget::Kind::Knob, "OUT", "out_gain", 8.0f, 112.0f);
        p.add(Widget::Kind::Jack, "L+R", "out:out", 56.0f, 116.0f);
        return p;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        float gainLin[4], panL[4], panR[4];
        bool on[4];
        for (int c = 0; c < 4; ++c) {
            const std::string n = std::to_string(c + 1);
            on[c] = parameterValue("mute" + n) < 0.5f && inputs[c] != nullptr;
            gainLin[c] = dbToGain(parameterValue("gain" + n));
            const float pan =
                clampf(parameterValue("pan" + n), -1.0f, 1.0f);
            const float t = (pan + 1.0f) * 0.7853982f;  // 0..π/2
            panL[c] = std::cos(t);
            panR[c] = std::sin(t);
        }
        const float outGain = dbToGain(parameterValue("out_gain"));

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float l = 0.0f, r = 0.0f;
            for (int c = 0; c < 4; ++c) {
                if (!on[c])
                    continue;
                const float s = inputs[c]->at(0, frame) * gainLin[c];
                l += s * panL[c];
                r += s * panR[c];
            }
            l *= outGain;
            r *= outGain;
            if (channels >= 2) {
                out.at(0, frame) = l;
                out.at(1, frame) = r;
                for (std::size_t ch = 2; ch < channels; ++ch)
                    out.at(ch, frame) = 0.5f * (l + r);
            } else {
                out.at(0, frame) = 0.5f * (l + r);
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float dbToGain(const float db) noexcept {
        return db <= -60.0f ? 0.0f : std::pow(10.0f, db / 20.0f);
    }
};

}  // namespace rasgo::modular
