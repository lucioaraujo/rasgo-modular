#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// VCA — amplificador controlado por tensão (Módulo 20)
// ============================================================================
//
// O verbo "multiplicar" da gramática. DUPLO (2 canais) — um rack precisa
// de pelo menos dois quase sempre (voz + modulação). Faz AM, tremolo,
// gate, ducking, e o `sum` o transforma num mixer de 2 canais.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/20_vca.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - VCA linear (Doepfer A-132) vs exponencial (A-131) — linear pra
//     somar CV, exp ("dB-linear") pra volume percebido;
//   - Quad VCA como mixer (Intellijel/4ms) — saídas somadas;
//   - atenuverter na entrada de CV (Maths) — o knob é a base, a CV soma;
//   - saturação de VCA analógico — ganho alto não é linear perfeito.
//
// Modulação por PORTA de verdade: `cv` atenuvertida SOMA ao knob `level`
// -> o knob fica vivo (ao contrário de `connectToParameter`). Desvio
// Rasgo: `drift` — os dois ganhos ganham uma oscilação lenta seeded.

namespace rasgo::modular {

class Vca final : public Signal {
public:
    Vca()
        : Signal(
              {{"in1", PortKind::Audio, ""},
               {"cv1", PortKind::Control, ""},
               {"in2", PortKind::Audio, ""},
               {"cv2", PortKind::Control, ""}},
              {{"out1", PortKind::Audio, ""},
               {"out2", PortKind::Audio, ""},
               {"sum", PortKind::Audio, ""}},
              {{"level1", 0.0f, 1.0f, 0.0f, ""},
               {"cv1_amount", -1.0f, 1.0f, 1.0f, ""},
               {"response1", 0.0f, 1.0f, 0.0f, ""},
               {"level2", 0.0f, 1.0f, 0.0f, ""},
               {"cv2_amount", -1.0f, 1.0f, 1.0f, ""},
               {"response2", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "VCA"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 2 tiras de canal + drift + I/O
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "VCA", "", 2.5f, 2.0f);
        for (int ch = 0; ch < 2; ++ch) {
            const std::string n = std::to_string(ch + 1);
            const float x = 8.0f + static_cast<float>(ch) * 24.0f;
            p.add(Widget::Kind::Knob, ("LVL" + n), "level" + n, x, 16.0f);
            p.add(Widget::Kind::Knob, ("CV" + n), "cv" + n + "_amount", x, 36.0f);
            p.add(Widget::Kind::Knob, ("RSP" + n), "response" + n, x, 56.0f);
            p.add(Widget::Kind::Jack, ("IN" + n), "in:in" + n, x, 100.0f);
            p.add(Widget::Kind::Jack, ("CV" + n), "in:cv" + n, x + 10.0f, 100.0f);
            p.add(Widget::Kind::Jack, ("O" + n), "out:out" + n, x, 116.0f);
        }
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 8.0f, 74.0f);
        p.add(Widget::Kind::Jack, "SUM", "out:sum", 20.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        gSmooth_[0] = parameterValue("level1");
        gSmooth_[1] = parameterValue("level2");
        driftState_ = 0.0f;
        driftCounter_ = 0;
        rng_ = 0x853C49E6748FEA9BULL;
        smoothCoef_ =
            1.0f - std::exp(-1.0f / std::max(1.0f, 0.0015f * sampleRate));
        driftInterval_ =
            static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 40.0f));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out1 = outputs[0];
        AudioBlock& out2 = outputs[1];
        AudioBlock& sumOut = outputs[2];
        const std::size_t frames = out1.frames();
        const std::size_t channels = out1.channels();

        const float lvl[2] = {parameterValue("level1"), parameterValue("level2")};
        const float cvAmt[2] = {parameterValue("cv1_amount"),
                                parameterValue("cv2_amount")};
        const float resp[2] = {parameterValue("response1"),
                               parameterValue("response2")};
        const float drift = parameterValue("drift");
        const float driftStep = 0.0004f * drift * drift;

        const AudioBlock* in[2] = {inputs[0], inputs[2]};
        const AudioBlock* cv[2] = {inputs[1], inputs[3]};

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftState_ += noise() * driftStep;
                driftState_ = clampf(driftState_, -0.03f, 0.03f);
            }
            const float driftMul = 1.0f + driftState_;

            float y[2];
            for (int k = 0; k < 2; ++k) {
                float g = lvl[k];
                if (cv[k] != nullptr)
                    g += cvAmt[k] * cv[k]->at(0, frame);
                g = clampf(g, 0.0f, 1.0f);
                const float gShaped =
                    resp[k] <= 0.0f ? g : std::pow(g, 1.0f + 3.0f * resp[k]);
                const float gFinal = gShaped * driftMul;
                gSmooth_[k] += (gFinal - gSmooth_[k]) * smoothCoef_;
                const float x = in[k] != nullptr ? in[k]->at(0, frame) : 0.0f;
                y[k] = softSat(x * gSmooth_[k]);
            }
            const float s = softSat(y[0] + y[1]);

            for (std::size_t c = 0; c < channels; ++c) {
                out1.at(c, frame) = y[0];
                out2.at(c, frame) = y[1];
                sumOut.at(c, frame) = s;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    // saturação suave: transparente até ±0,9, tanh acima (teto ~1,0)
    static float softSat(const float x) noexcept {
        if (x > 0.9f) return 0.9f + 0.1f * std::tanh((x - 0.9f) * 8.0f);
        if (x < -0.9f) return -0.9f + 0.1f * std::tanh((x + 0.9f) * 8.0f);
        return x;
    }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float gSmooth_[2] = {0.0f, 0.0f};
    float driftState_ = 0.0f;
    float smoothCoef_ = 0.05f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 1200;
    std::uint64_t rng_ = 0x853C49E6748FEA9BULL;
};

}  // namespace rasgo::modular
