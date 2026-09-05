#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>

// ============================================================================
// ENVELOPE — envelope AD/ASR + VCA embutido (Módulo 6)
// ============================================================================
//
// O contorno: um gate entra, uma forma de amplitude sai — e o módulo já
// aplica essa forma a um sinal de áudio (VCA embutido). É o que "dá corpo"
// aos disparos do CLOCK e às decisões do DECISION.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/06_envelope.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Make Noise Maths - envelope como função (rise/fall, curva contínua
//     côncava<->convexa), e o "canal" que é gerador E processador;
//   - Mannequins Just Friends - "transient" (one-shot) vs "sustain"
//     (segue o gate) no mesmo controle;
//   - Joranalogue Contour 1 - AD/ASR com fim-de-ciclo e curva;
//   - VCA linic​ear vs exponencial - a lei do VCA como caráter.
//
// Curva contínua: `curve` = 0 convexa (ataque "mole"), 0,5 linear, 1
// côncava (ataque "estalado"/exponencial). Determinístico (sem RNG).

namespace rasgo::modular {

class Envelope final : public Signal {
public:
    Envelope()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"gate", PortKind::Control, "gate"},
               {"time_mod", PortKind::Control, "v/oct"}},
              {{"out", PortKind::Audio, ""},
               {"env", PortKind::Control, ""}},
              {{"attack", 0.001f, 10.0f, 0.01f, "s"},
               {"decay", 0.001f, 10.0f, 0.3f, "s"},
               {"sustain", 0.0f, 1.0f, 0.0f, ""},
               {"release", 0.001f, 10.0f, 0.4f, "s"},
               {"curve", 0.0f, 1.0f, 0.6f, ""},
               {"mode", 0.0f, 1.0f, 0.0f, ""},  // 0 = gated (ASR), 1 = trigger (AD)
               {"vca_depth", 0.0f, 1.0f, 1.0f, ""},
               {"level", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "ENVELOPE"; }

    // Layout: 12 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "ENVELOPE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "contour", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "ATK", "attack", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DEC", "decay", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SUS", "sustain", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "REL", "release", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CURVE", "curve", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "VCA", "vca_depth", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "LVL", "level", 35.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "TRIG", "mode", 49.0f, 54.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "TIME", "in:time_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ENV", "out:env", 17.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        stage_ = Stage::Idle;
        segPhase_ = 0.0f;
        env_ = 0.0f;
        segStart_ = 0.0f;
        prevGate_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& audioOut = outputs[0];
        AudioBlock& envOut = outputs[1];
        const std::size_t frames = audioOut.frames();
        const std::size_t channels = audioOut.channels();

        const float attack = parameterValue("attack");
        const float decay = parameterValue("decay");
        const float sustain = parameterValue("sustain");
        const float release = parameterValue("release");
        const float curve = parameterValue("curve");
        const bool triggerMode = parameterValue("mode") >= 0.5f;
        const float depth = parameterValue("vca_depth");
        const float level = parameterValue("level");

        const AudioBlock* in = inputs[0];
        const AudioBlock* gate = inputs[1];
        const AudioBlock* timeMod = inputs[2];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float timeScale = timeMod
                ? clampf(std::exp2(timeMod->at(0, frame)), 0.03125f, 32.0f)
                : 1.0f;
            const float g = gate ? gate->at(0, frame) : 0.0f;
            const bool rising = prevGate_ < 0.5f && g >= 0.5f;
            const bool falling = prevGate_ >= 0.5f && g < 0.5f;
            prevGate_ = g;

            if (rising) {
                stage_ = Stage::Attack;
                segPhase_ = 0.0f;
                segStart_ = env_;
            } else if (falling && !triggerMode
                       && (stage_ == Stage::Attack || stage_ == Stage::Decay
                           || stage_ == Stage::Sustain)) {
                stage_ = Stage::Release;
                segPhase_ = 0.0f;
                segStart_ = env_;
            }

            switch (stage_) {
            case Stage::Idle:
                env_ = 0.0f;
                break;
            case Stage::Attack: {
                segPhase_ += 1.0f
                    / std::max(1.0f, attack * timeScale * sampleRate_);
                if (segPhase_ >= 1.0f) {
                    env_ = 1.0f;
                    stage_ = Stage::Decay;
                    segPhase_ = 0.0f;
                    segStart_ = 1.0f;
                } else {
                    env_ = segStart_ + (1.0f - segStart_) * shape(segPhase_, curve);
                }
                break;
            }
            case Stage::Decay: {
                segPhase_ += 1.0f
                    / std::max(1.0f, decay * timeScale * sampleRate_);
                const float target = sustain;
                if (segPhase_ >= 1.0f) {
                    env_ = target;
                    if (!triggerMode && prevGate_ >= 0.5f && sustain > 0.0001f) {
                        stage_ = Stage::Sustain;
                    } else {
                        stage_ = Stage::Release;
                        segPhase_ = 0.0f;
                        segStart_ = env_;
                    }
                } else {
                    env_ = target
                        + (segStart_ - target) * (1.0f - shape(segPhase_, curve));
                }
                break;
            }
            case Stage::Sustain:
                env_ = sustain;
                if (prevGate_ < 0.5f) {
                    stage_ = Stage::Release;
                    segPhase_ = 0.0f;
                    segStart_ = env_;
                }
                break;
            case Stage::Release: {
                segPhase_ += 1.0f
                    / std::max(1.0f, release * timeScale * sampleRate_);
                if (segPhase_ >= 1.0f) {
                    env_ = 0.0f;
                    stage_ = Stage::Idle;
                } else {
                    env_ = segStart_ * (1.0f - shape(segPhase_, curve));
                }
                break;
            }
            }

            const float envUnit = clampf(env_, 0.0f, 1.0f);
            const float gain = (1.0f - depth) + depth * envUnit;
            const float x = in ? in->at(0, frame) : 0.0f;
            const float audio = x * gain;
            const float envValue = envUnit * level;

            for (std::size_t channel = 0; channel < channels; ++channel) {
                audioOut.at(channel, frame) = audio;
                envOut.at(channel, frame) = envValue;
            }
        }
    }

private:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // x in [0,1] -> [0,1] crescente. curve<0.5 convexa (expoente>1,
    // começa devagar); 0,5 linear; >0,5 côncava (começa rápido).
    static float shape(const float x, const float curve) noexcept {
        const float cx = clampf(x, 0.0f, 1.0f);
        float exponent;
        if (curve < 0.5f)
            exponent = 1.0f + (0.5f - curve) * 6.0f;      // 1 .. 4
        else
            exponent = 1.0f / (1.0f + (curve - 0.5f) * 6.0f);  // 1 .. 0,25
        return std::pow(cx, exponent);
    }

    Stage stage_ = Stage::Idle;
    float segPhase_ = 0.0f;
    float env_ = 0.0f;
    float segStart_ = 0.0f;
    float prevGate_ = 0.0f;
};

}  // namespace rasgo::modular
