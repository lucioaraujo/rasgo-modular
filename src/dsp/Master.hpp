#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/OutputStage.hpp"

#include <cmath>

// ============================================================================
// MASTER — barramento de saída estéreo (Módulo 17)
// ============================================================================
//
// O último nó antes das caixas. Largura estéreo (mid/side), soma mono,
// bloqueio de DC, limitador suave de segurança e ganho de master. Saída
// estéreo + uma saída de controle `level` (pico com decaimento) pra um
// VU.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/17_master.md`.
//
// Fontes ESTUDADAS (prática de estúdio / DSP público):
//   - matriz mid/side: mid = (L+R)/2, side = (L−R)/2; `width` escala o
//     side (0 = mono, 1 = normal, 2 = largo);
//   - bloqueio de DC: passa-alta de 1 polo em ~5 Hz (`y = x − x₁ + R·y₁`);
//   - limitador suave: `tanh` com makeup, transparente abaixo do teto.
//
// Determinístico (sem RNG). Sem alocação.

namespace rasgo::modular {

class Master final : public Signal {
public:
    Master()
        : Signal(
              {{"in", PortKind::Audio, ""}},
              {{"out", PortKind::Audio, ""},
               {"level", PortKind::Control, ""}},
              {{"gain", -60.0f, 12.0f, 0.0f, "dB"},
               {"width", 0.0f, 2.0f, 1.0f, ""},
               {"mono", 0.0f, 1.0f, 0.0f, ""},
               {"dc_block", 0.0f, 1.0f, 1.0f, ""},
               {"limit", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "MASTER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "MASTER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "vu", "", 2.5f, 8.0f, 35.0f);
        p.add(Widget::Kind::Slider, "GAIN", "gain", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "WIDTH", "width", 26.0f, 28.0f);
        p.add(Widget::Kind::Toggle, "MONO", "mono", 24.0f, 50.0f);
        p.add(Widget::Kind::Toggle, "DC", "dc_block", 24.0f, 64.0f);
        p.add(Widget::Kind::Toggle, "LIMIT", "limit", 24.0f, 78.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 104.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 17.0f, 104.0f);
        p.add(Widget::Kind::Jack, "VU", "out:level", 29.0f, 104.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        peak_ = 0.0f;
        // proteção de saída de excelência (look-ahead + teto suave) —
        // padrão RASGO, ver `src/dsp/OutputStage.hpp`
        out_.prepare(sampleRate, -1.0f /*teto dBFS*/, 3.0f /*look-ahead ms*/,
                     120.0f /*release ms*/);
        // pico decai ~300 ms
        peakDecay_ = std::exp(-1.0f / (0.3f * std::max(1.0f, sampleRate)));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& level = outputs[1];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* in = inputs[0];
        const float gain = dbToGain(parameterValue("gain"));
        const float width = parameterValue("width");
        const bool mono = parameterValue("mono") >= 0.5f;
        const bool dcBlock = parameterValue("dc_block") >= 0.5f;
        const bool limit = parameterValue("limit") >= 0.5f;
        const std::size_t inCh = in ? in->channels() : 0;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float l = in ? in->at(0, frame) : 0.0f;
            float r = (in && inCh >= 2) ? in->at(1, frame) : l;

            // largura mid/side
            const float mid = 0.5f * (l + r);
            float side = 0.5f * (l - r) * width;
            l = mid + side;
            r = mid - side;
            if (mono) { l = mid; r = mid; }

            l *= gain;
            r *= gain;

            // proteção de saída: guarda de finitude + (DC) + limitador com
            // look-ahead + teto suave — não distorce o transiente
            out_.process(l, r, dcBlock, limit);

            const float mag = std::fabs(l) > std::fabs(r)
                ? std::fabs(l) : std::fabs(r);
            peak_ = mag > peak_ ? mag : peak_ * peakDecay_;

            if (channels >= 2) {
                out.at(0, frame) = l;
                out.at(1, frame) = r;
                for (std::size_t ch = 2; ch < channels; ++ch)
                    out.at(ch, frame) = 0.5f * (l + r);
            } else {
                out.at(0, frame) = 0.5f * (l + r);
            }
            for (std::size_t ch = 0; ch < level.channels(); ++ch)
                level.at(ch, frame) = peak_ > 1.0f ? 1.0f : peak_;
        }
    }

    // telemetria da proteção (leitura não RT-crítica; p/ um medidor de GR
    // no painel ou pra um módulo SEGUIR a própria redução de ganho)
    float gainReductionDb() const noexcept { return out_.gainReductionDb(); }

private:
    static float dbToGain(const float db) noexcept {
        return db <= -60.0f ? 0.0f : std::pow(10.0f, db / 20.0f);
    }

    OutputStage out_;
    float peak_ = 0.0f;
    float peakDecay_ = 0.9999f;
};

}  // namespace rasgo::modular
