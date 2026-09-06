#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>

// ============================================================================
// GLIDE — portamento por nota (Módulo 39)
// ============================================================================
//
// O `CONTROL` tem um `slew`, mas é um lag RC SEMPRE ligado — bom pra
// seguidor de envelope, não pra condução de melodia. `GLIDE` decide POR
// NOTA se escorrega ou salta, com subida ≠ descida e três modos:
//   0 — sempre (portamento clássico)
//   1 — slide-gated: escorrega só enquanto `slide` está alto (o *slide*
//       do TB-303 — cada passo do sequenciador decide)
//   2 — legato: escorrega só se `gate` continua alto na troca de nota
//
// Ver o dossiê: `RASGO_MODULAR/dossies/39_glide.md`.
//
// Fontes ESTUDADAS (conceito, não código): TB-303 slide; portamento
// MS-20/Minimoog (slew de inclinação CONSTANTE) vs glide RC (meia-vida
// constante) — o knob `curve` faz o contínuo entre os dois, padrão do
// `ENVELOPE`; portamento legato de synth mono; Bela Gliss / EMW glide
// processor (rise/fall separados).
//
// Precisão importa (é régua de afinação) — SEM `drift`. Determinístico,
// sem alocação / lock / IO em `process()`.

namespace rasgo::modular {

class Glide final : public Signal {
public:
    Glide()
        : Signal(
              {{"pitch", PortKind::Audio, ""},        // CV a deslizar (1 V/oct ou qualquer)
               {"slide", PortKind::Control, "gate"},  // habilita o glide (modo 1)
               {"gate", PortKind::Control, "gate"}},  // pra o modo legato (modo 2)
              {{"out", PortKind::Audio, ""},          // CV deslizada
               {"moving", PortKind::Control, "gate"}, // alto enquanto desliza
               {"done", PortKind::Control, "trig"}},  // pulso ~2 ms na chegada
              {{"time", 0.0f, 2.0f, 0.08f, "s"},     // tempo de subida (mudança de 1,0)
               {"fall", -1.0f, 1.0f, 0.0f, ""},      // assimetria: descida = time·6^fall
               {"curve", 0.0f, 1.0f, 0.3f, ""},      // linear (rate const.) ↔ exp (RC)
               {"mode", 0.0f, 2.0f, 0.0f, ""}}) {}   // 0 sempre · 1 slide · 2 legato

    std::string type() const override { return "GLIDE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "GLIDE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "trace", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "TIME", "time", 11.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FALL", "fall", 31.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CURVE", "curve", 11.0f, 54.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 31.0f, 54.0f);
        p.add(Widget::Kind::Jack, "PITCH", "in:pitch", 9.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SLIDE", "in:slide", 24.0f, 92.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 39.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 9.0f, 114.0f);
        p.add(Widget::Kind::Jack, "MOV", "out:moving", 24.0f, 114.0f);
        p.add(Widget::Kind::Jack, "DONE", "out:done", 39.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        dt_ = 1.0f / std::max(1.0f, sampleRate);
        y_ = 0.0f;
        prevGate_ = 0.0f;
        wasMoving_ = false;
        doneCountdown_ = 0;
        doneSamples_ = static_cast<int>(0.002f * sampleRate) + 1;  // ~2 ms
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& mov = outputs[1];
        AudioBlock& done = outputs[2];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float riseT = clampf(parameterValue("time"), 0.0f, 2.0f);
        const float fall = clampf(parameterValue("fall"), -1.0f, 1.0f);
        const float fallT = riseT * std::pow(6.0f, fall);
        const float curve = clamp01(parameterValue("curve"));
        const int mode = static_cast<int>(std::lround(
            clampf(parameterValue("mode"), 0.0f, 2.0f)));

        const AudioBlock* pin = inputs[0];
        const AudioBlock* sin = inputs[1];
        const AudioBlock* gin = inputs[2];

        for (std::size_t f = 0; f < frames; ++f) {
            const float target = pin ? pin->at(0, f) : y_;
            const float slideV = sin ? sin->at(0, f) : 0.0f;
            const float gateV = gin ? gin->at(0, f) : 0.0f;
            const bool gateRising = prevGate_ < 0.5f && gateV >= 0.5f;
            prevGate_ = gateV;

            bool jump;
            if (mode == 1) jump = slideV < 0.5f;             // 303: só com slide
            else if (mode == 2) jump = gateRising || gateV < 0.5f;  // legato
            else jump = false;                              // sempre desliza

            if (jump) {
                y_ = target;
            } else {
                const float d = target - y_;
                const float t = (d >= 0.0f) ? riseT : fallT;
                if (t <= dt_ || std::fabs(d) < 1.0e-7f) {
                    y_ = target;
                } else {
                    const float linStep = dt_ / t;          // mudança de 1,0 por t s
                    const float lin =
                        y_ + clampf(d, -linStep, linStep);
                    const float expo = y_ + d * (1.0f - std::exp(-dt_ / t));
                    y_ = lin + (expo - lin) * curve;
                }
            }

            const bool moving = std::fabs(target - y_) > 1.0e-4f;
            if (wasMoving_ && !moving) doneCountdown_ = doneSamples_;
            wasMoving_ = moving;
            float donePulse = 0.0f;
            if (doneCountdown_ > 0) { donePulse = 1.0f; --doneCountdown_; }

            for (std::size_t c = 0; c < channels; ++c) {
                out.at(c, f) = y_;
                mov.at(c, f) = moving ? 1.0f : 0.0f;
                done.at(c, f) = donePulse;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float dt_ = 1.0f / 48000.0f;
    float y_ = 0.0f;
    float prevGate_ = 0.0f;
    bool wasMoving_ = false;
    int doneCountdown_ = 0;
    int doneSamples_ = 96;
};

}  // namespace rasgo::modular
