#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// TURING — laço de registrador de deslocamento (Módulo 8)
// ============================================================================
//
// Uma sequência que não é programada: nasce do acaso e depois pode ser
// TRAVADA. Um registrador de deslocamento de `length` estágios circula
// valores; a cada clock, o valor que reentra é o que saiu (laço preservado)
// ou um valor novo/mutado - `lock` controla essa probabilidade. CCW = puro
// acaso; meio = 50% de mudar; CW = laço travado.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/08_turing.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Music Thing Turing Machine Mk II (Tom Whitwell) - shift-register de
//     16 bits, knob "Lock" = probabilidade de mutação por ciclo;
//     expansores Volts (soma ponderada de bits) e Pulses (bits como gates);
//   - Mutable Marbles `déjà-vu` - a mesma ideia de memória circular relida;
//   - a quantização a alturas (2ª camada, cruza com harmonia/RASGO_SYNTH).
//
// Determinístico: xorshift64* semeado em prepare(); registrador inicial
// pseudo-aleatório reprodutível. Avança por `clock` externo OU, sem ele,
// por relógio interno em `rate`.

namespace rasgo::modular {

class TuringLoop final : public Signal {
public:
    TuringLoop()
        : Signal(
              {{"clock", PortKind::Control, "trig"},
               {"lock_mod", PortKind::Control, ""}},
              {{"cv", PortKind::Audio, ""},
               {"cv2", PortKind::Audio, ""},
               {"pulse", PortKind::Control, "gate"}},
              {{"rate", 0.01f, 50.0f, 4.0f, "Hz"},
               {"length", 2.0f, 16.0f, 8.0f, ""},
               {"lock", 0.0f, 1.0f, 0.5f, ""},
               {"mutate", 0.0f, 1.0f, 1.0f, ""},
               {"range", 0.0f, 1.0f, 0.6f, ""},
               {"steps", 1.0f, 32.0f, 1.0f, ""},
               {"offset", -1.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "TURING"; }

    // Layout: 12 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "TURING", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "register", "", 2.5f, 8.0f, 55.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LEN", "length", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LOCK", "lock", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MUT", "mutate", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "STEPS", "steps", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "OFST", "offset", 35.0f, 52.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "LOCK", "in:lock_mod", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "CV", "out:cv", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "CV2", "out:cv2", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PLS", "out:pulse", 29.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        prevClock_ = 0.0f;
        rngState_ = 0x94D049BB133111EBULL;
        for (int i = 0; i < kMax; ++i)
            reg_[i] = uniform01();
        computeOutputs(parameterValue("range"),
                       static_cast<int>(std::lround(parameterValue("steps"))),
                       parameterValue("offset"));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& cvOut = outputs[0];
        AudioBlock& cv2Out = outputs[1];
        AudioBlock& pulseOut = outputs[2];
        const std::size_t frames = cvOut.frames();
        const std::size_t channels = cvOut.channels();

        const float rate = parameterValue("rate");
        const int length = clampi(
            static_cast<int>(std::lround(parameterValue("length"))), 2, kMax);
        const float lockParam = parameterValue("lock");
        const float mutate = parameterValue("mutate");
        const float range = parameterValue("range");
        const int steps =
            static_cast<int>(std::lround(parameterValue("steps")));
        const float offset = parameterValue("offset");

        const AudioBlock* clock = inputs[0];
        const AudioBlock* lockMod = inputs[1];
        const bool externalClock = (clock != nullptr);
        const double dp = static_cast<double>(rate) / sampleRate_;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float lock = clampf(
                lockParam + (lockMod ? lockMod->at(0, frame) : 0.0f), 0.0f, 1.0f);

            bool step = false;
            if (externalClock) {
                const float c = clock->at(0, frame);
                if (prevClock_ < 0.5f && c >= 0.5f)
                    step = true;
                prevClock_ = c;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) {
                    phase_ -= 1.0;
                    step = true;
                }
            }
            if (step)
                advance(length, lock, mutate, range, steps, offset);

            for (std::size_t channel = 0; channel < channels; ++channel) {
                cvOut.at(channel, frame) = cv_;
                cv2Out.at(channel, frame) = cv2_;
                pulseOut.at(channel, frame) = pulse_;
            }
        }
    }

private:
    static constexpr int kMax = 16;

    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    void advance(const int length, const float lock, const float mutate,
                 const float range, const int steps,
                 const float offset) noexcept {
        // valor que sai da frente
        const float front = reg_[0];
        // desloca
        for (int i = 0; i < length - 1; ++i)
            reg_[i] = reg_[i + 1];

        // o que reentra na cauda
        float tail = front;  // laço preservado
        if (uniform01() >= lock) {
            // muda: parcial (ruído) ou total (novo sorteio) por `mutate`
            if (mutate >= 0.999f)
                tail = uniform01();
            else
                tail = clampf(front + (uniform01() * 2.0f - 1.0f) * mutate,
                              0.0f, 1.0f);
        }
        reg_[length - 1] = tail;

        computeOutputs(range, steps, offset);
    }

    void computeOutputs(const float range, const int steps,
                        const float offset) noexcept {
        // cv = frente do registrador, bipolar, escalado e quantizado
        float x = (reg_[0] - 0.5f) * 2.0f * range + offset;
        if (steps >= 2) {
            const float n = static_cast<float>(steps - 1);
            x = std::round((x * 0.5f + 0.5f) * n) / n * 2.0f - 1.0f;
        }
        cv_ = clampf(x, -1.0f, 1.0f);

        // cv2 = soma ponderada de alguns estágios (expansor "Volts")
        const float w = reg_[0] * 0.5f + reg_[2] * 0.25f + reg_[5] * 0.15f
            + reg_[7] * 0.1f;
        cv2_ = clampf((w - 0.5f) * 2.0f * range + offset, -1.0f, 1.0f);

        // pulse = estágio da frente como bit (expansor "Pulses")
        pulse_ = reg_[0] >= 0.5f ? 1.0f : 0.0f;
    }

    double phase_ = 0.0;
    float prevClock_ = 0.0f;
    std::uint64_t rngState_ = 0x94D049BB133111EBULL;

    float reg_[kMax] = {};
    float cv_ = 0.0f;
    float cv2_ = 0.0f;
    float pulse_ = 0.0f;
};

}  // namespace rasgo::modular
