#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <cmath>

// ============================================================================
// QUANTIZER — quantizador de escala (Módulo 12)
// ============================================================================
//
// Transforma CV contínua em ALTURAS de uma escala musical. A saída é em
// oitavas (1 V/oct): `pitch` alimenta o `rate_mod` de uma voz e a
// transposição fica certa. Com sample-and-hold (`trigger`), a nota só
// muda no pulso - é assim que `TURING`/`DECISION` viram melodia.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/12_quantizer.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - as tabelas de graus vêm da pesquisa registrada em
//     `RASGO_SYNTH/rasgo-synth-core/src/sequencer/Scales.hpp` (39+ escalas
//     reais, cruzadas com nomenclatura de teoria/jazz, com 3 erros de uma
//     fonte anterior corrigidos). Aqui um subconjunto curado - leve, não a
//     máquina inteira ([[feedback_generative_design_light_touch]]);
//   - quantizadores de hardware (Doepfer A-156, Intellijel Scales,
//     Marbles `t`) - histerese pra não tremular entre graus vizinhos;
//   - glide exponencial (theremin, `RASGO_SYNTH/dsp/ThereminVoice.hpp`).
//
// Determinístico (sem RNG).

namespace rasgo::modular {

class Quantizer final : public Signal {
public:
    // Subconjunto curado das tabelas pesquisadas (graus em semitons desde a
    // tônica). A ordem define o índice do parâmetro `scale`.
    struct ScaleDef {
        const char* name;
        std::array<int, 12> degrees;
        int length;
    };
    static const std::array<ScaleDef, 12>& scales() {
        static const std::array<ScaleDef, 12> s{{
            {"Chromatic", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 12},
            {"Major", {0, 2, 4, 5, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Aeolian", {0, 2, 3, 5, 7, 8, 10, 10, 10, 10, 10, 10}, 7},
            {"Dorian", {0, 2, 3, 5, 7, 9, 10, 10, 10, 10, 10, 10}, 7},
            {"Phrygian Dominant", {0, 1, 4, 5, 7, 8, 10, 10, 10, 10, 10, 10}, 7},
            {"Lydian", {0, 2, 4, 6, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Melodic Minor", {0, 2, 3, 5, 7, 9, 11, 11, 11, 11, 11, 11}, 7},
            {"Minor Pentatonic", {0, 3, 5, 7, 10, 10, 10, 10, 10, 10, 10, 10}, 5},
            {"Major Pentatonic", {0, 2, 4, 7, 9, 9, 9, 9, 9, 9, 9, 9}, 5},
            {"Hirajoshi", {0, 2, 3, 7, 8, 8, 8, 8, 8, 8, 8, 8}, 5},
            {"Whole Tone", {0, 2, 4, 6, 8, 10, 10, 10, 10, 10, 10, 10}, 6},
            {"Octave", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1},
        }};
        return s;
    }

    Quantizer()
        : Signal(
              {{"cv", PortKind::Audio, ""},
               {"transpose", PortKind::Control, "v/oct"},
               {"trigger", PortKind::Control, "trig"}},
              {{"pitch", PortKind::Audio, "v/oct"},
               {"gate", PortKind::Control, "gate"},
               {"semitone", PortKind::Audio, ""}},
              {{"scale", 0.0f, 11.0f, 1.0f, ""},
               {"root", 0.0f, 11.0f, 0.0f, "st"},
               {"range", 1.0f, 6.0f, 2.0f, "oct"},
               {"glide", 0.0f, 1.0f, 0.0f, ""},
               {"hysteresis", 0.0f, 1.0f, 0.3f, ""}}) {}

    std::string type() const override { return "QUANTIZER"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "QUANTIZER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "scale", "", 2.5f, 8.0f, 45.0f);
        p.add(Widget::Kind::Knob, "SCALE", "scale", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ROOT", "root", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "GLIDE", "glide", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "HYST", "hysteresis", 23.0f, 54.0f);
        p.add(Widget::Kind::Jack, "CV", "in:cv", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "TRSP", "in:transpose", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trigger", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ST", "out:semitone", 29.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        heldSemi_ = 0.0f;
        glideSemi_ = 0.0f;
        prevTrigger_ = 0.0f;
        gateCountdown_ = 0;
        initialized_ = false;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& pitchOut = outputs[0];
        AudioBlock& gateOut = outputs[1];
        AudioBlock& semiOut = outputs[2];
        const std::size_t frames = pitchOut.frames();
        const std::size_t channels = pitchOut.channels();

        const int scaleIdx =
            clampi(static_cast<int>(std::lround(parameterValue("scale"))), 0, 11);
        const int root =
            clampi(static_cast<int>(std::lround(parameterValue("root"))), 0, 11);
        const float range = parameterValue("range");
        const float glide = parameterValue("glide");
        const float hysteresis = parameterValue("hysteresis");
        const ScaleDef& sc = scales()[static_cast<std::size_t>(scaleIdx)];

        const AudioBlock* cv = inputs[0];
        const AudioBlock* transpose = inputs[1];
        const AudioBlock* trigger = inputs[2];
        const bool sampleHold = (trigger != nullptr);

        const float glideCoeff = glide <= 0.0f
            ? 1.0f
            : 1.0f - std::exp(-1.0f
                              / std::max(1.0f, glide * 0.2f * sampleRate_));
        const int gateSamples = static_cast<int>(0.005f * sampleRate_);

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float in = cv ? cv->at(0, frame) : 0.0f;
            const float trsp = transpose ? transpose->at(0, frame) : 0.0f;
            // CV [-1,1] -> semitons; transpose em oitavas
            const float wantSemi = in * range * 12.0f + trsp * 12.0f;

            bool update = !sampleHold;
            if (sampleHold) {
                const float t = trigger->at(0, frame);
                if (prevTrigger_ < 0.5f && t >= 0.5f)
                    update = true;
                prevTrigger_ = t;
            }

            if (update) {
                const float snapped = snap(wantSemi, sc, root);
                // histerese: só troca se afastou o bastante do valor preso
                const float deadband = hysteresis * 0.5f * averageStep(sc);
                if (!initialized_
                    || std::fabs(snapped - heldSemi_) > deadband) {
                    if (!initialized_ || snapped != heldSemi_) {
                        gateCountdown_ = gateSamples;
                        heldSemi_ = snapped;
                        initialized_ = true;
                    }
                }
            }

            // glide até o alvo
            if (glideCoeff >= 1.0f)
                glideSemi_ = heldSemi_;
            else
                glideSemi_ += (heldSemi_ - glideSemi_) * glideCoeff;

            float gate = 0.0f;
            if (gateCountdown_ > 0) {
                gate = 1.0f;
                --gateCountdown_;
            }

            const float pitchOct = glideSemi_ / 12.0f;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                pitchOut.at(channel, frame) = pitchOct;
                gateOut.at(channel, frame) = gate;
                semiOut.at(channel, frame) = glideSemi_ / 24.0f;  // ~[-1,1]
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    static float averageStep(const ScaleDef& sc) noexcept {
        return 12.0f / static_cast<float>(sc.length);
    }

    // acha o pitch da escala (em qualquer oitava) mais próximo de `semi`
    static float snap(const float semi, const ScaleDef& sc,
                      const int root) noexcept {
        const int baseOct = static_cast<int>(std::floor(semi / 12.0f));
        float best = 0.0f;
        float bestDist = 1.0e9f;
        for (int oct = baseOct - 1; oct <= baseOct + 1; ++oct) {
            for (int d = 0; d < sc.length; ++d) {
                const float cand =
                    static_cast<float>(oct * 12 + sc.degrees[static_cast<std::size_t>(d)] + root);
                const float dist = std::fabs(cand - semi);
                if (dist < bestDist) {
                    bestDist = dist;
                    best = cand;
                }
            }
        }
        return best;
    }

    float heldSemi_ = 0.0f;
    float glideSemi_ = 0.0f;
    float prevTrigger_ = 0.0f;
    int gateCountdown_ = 0;
    bool initialized_ = false;
};

}  // namespace rasgo::modular
