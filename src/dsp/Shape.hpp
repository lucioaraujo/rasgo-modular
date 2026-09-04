#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/Oversampler.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// SHAPE — modelador de timbre (Módulo 24)
// ============================================================================
//
// Ring-mod + wavefolder + wrap + saturação numa cadeia, com VCA de saída.
// A síntese por distorção controlada da costa oeste (Buchla/Serge), como
// módulo visível — a relação `Fold`/`RingMod` do `Cable` faz isso mas
// escondido e sem os controles de caráter.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/24_shape.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Buchla 259/258 "Timbre" — folder + `symmetry` (bias = harmônicos pares);
//   - Serge Wave Multipliers — dobras em série;
//   - ring modulator clássico — `x·y` de 4 quadrantes;
//   - dobra triangular fechada `4·|u/4 − round(u/4)| − 1`.
//
// Antialias (híbrido, "no talo"): a dobra + wrap + sat rodam a 2× pelo
// `Oversampler2x` E com ADAA de 1ª ordem dentro do laço 2× — a
// antiderivada FECHADA de `m(u)` (dobra triangular ⊕ wrap-around, ambas
// com integral contínua; Parker/Esqueda/Välimäki 2016) mata o alias
// entre fs/2 e fs antes da decimação, o oversampling cuida do resto.
// Atraso de grupo ~2,5 amostras. Ver dossiê §6.

namespace rasgo::modular {

class Shape final : public Signal {
public:
    Shape()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"mod", PortKind::Audio, ""},
               {"fold_mod", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"ring", 0.0f, 1.0f, 0.0f, ""},
               {"fold", 0.0f, 1.0f, 0.0f, ""},
               {"symmetry", -1.0f, 1.0f, 0.0f, ""},
               {"wrap", 0.0f, 1.0f, 0.0f, ""},
               {"sat", 0.0f, 1.0f, 0.15f, ""},
               {"level", 0.0f, 1.0f, 0.8f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "SHAPE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "SHAPE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "wave", "", 2.5f, 7.0f, 40.0f);
        p.add(Widget::Kind::Knob, "RING", "ring", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "FOLD", "fold", 28.0f, 26.0f);
        p.add(Widget::Kind::Knob, "SYM", "symmetry", 8.0f, 46.0f);
        p.add(Widget::Kind::Knob, "WRAP", "wrap", 28.0f, 46.0f);
        p.add(Widget::Kind::Knob, "SAT", "sat", 8.0f, 66.0f);
        p.add(Widget::Kind::Knob, "LVL", "level", 28.0f, 66.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 8.0f, 82.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 6.0f, 110.0f);
        p.add(Widget::Kind::Jack, "MOD", "in:mod", 18.0f, 110.0f);
        p.add(Widget::Kind::Jack, "FCV", "in:fold_mod", 30.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 42.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        driftCur_ = 0.0f;
        driftTgt_ = 0.0f;
        uPrev_ = 0.0f;
        os_.reset();
        driftCounter_ = 0;
        driftInterval_ = static_cast<std::uint32_t>(
            std::max(1.0f, std::max(1.0f, sampleRate) / 8.0f));
        rng_ = 0x100000001B3ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float ring = clamp01(parameterValue("ring"));
        const float foldP = clamp01(parameterValue("fold"));
        const float sym = clampf(parameterValue("symmetry"), -1.0f, 1.0f);
        const float wrap = clamp01(parameterValue("wrap"));
        const float sat = clamp01(parameterValue("sat"));
        const float level = clamp01(parameterValue("level"));
        const float drift = clamp01(parameterValue("drift"));

        const AudioBlock* in = inputs[0];
        const AudioBlock* mod = inputs[1];
        const AudioBlock* fcv = inputs[2];
        const float driftAmp = 0.04f * drift;

        for (std::size_t f = 0; f < frames; ++f) {
            if (drift > 0.0f && ++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftTgt_ = noise() * driftAmp;
            }
            driftCur_ += (driftTgt_ - driftCur_) * 0.002f;

            float x = in != nullptr ? in->at(0, f) : 0.0f;
            if (mod != nullptr) {
                const float rm = x * mod->at(0, f);
                x += (rm - x) * ring;
            }
            x += sym * 0.5f;

            const float foldCv = fcv != nullptr ? fcv->at(0, f) : 0.0f;
            const float foldAmt = clamp01(foldP + foldCv);
            const float drive = 1.0f + foldAmt * 6.0f * (1.0f + driftCur_);

            // núcleo sem memória (dobra ⊕ wrap com ADAA de 1ª ordem, depois
            // sat/tanh), rodado a 2× pelo oversampler. `drive` é constante
            // nas 2 sub-amostras; o estado do ADAA (`uPrev_`) acompanha o
            // rate dobrado.
            const auto core = [&](const float xs) noexcept -> float {
                const float u = xs * drive;
                const float du = u - uPrev_;
                const float m = std::fabs(du) > 1.0e-5f
                    ? (shaperIntegral(u, wrap) - shaperIntegral(uPrev_, wrap))
                        / du
                    : shaperM(0.5f * (u + uPrev_), wrap);
                uPrev_ = u;
                return m + (std::tanh(m * 1.5f) - m) * sat;
            };
            float y = os_.process(0, x, core) * level;

            for (std::size_t c = 0; c < channels; ++c)
                out.at(c, f) = y;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // shaper: dobra triangular fechada (`ph` desloca pra passar pela
    // origem; ~linear perto de 0, reflete a cada ±2) misturada por `wrap`
    // com wrap-around seco (dente-de-serra que reentra em ±1).
    static float shaperM(const float u, const float wrap) noexcept {
        const float ph = u * 0.25f + 0.25f;
        const float folded = 4.0f * std::fabs(ph - std::round(ph)) - 1.0f;
        const float wrapped = u - 2.0f * std::round(u * 0.5f);
        return folded + (wrapped - folded) * wrap;
    }
    // ∫ m du — CONTÍNUA em todo u (a antiderivada de uma função limitada
    // sempre é). ∫folded = 8·w·|w| − 4·w (w = ph − round ph, zera nos
    // limites do período); ∫wrapped = 2·s² (s = u/2 − round(u/2)).
    static float shaperIntegral(const float u, const float wrap) noexcept {
        const float ph = u * 0.25f + 0.25f;
        const float w = ph - std::round(ph);
        const float integralFold = 8.0f * w * std::fabs(w) - 4.0f * w;
        const float s = u * 0.5f - std::round(u * 0.5f);
        const float integralWrap = 2.0f * s * s;
        return integralFold + (integralWrap - integralFold) * wrap;
    }
    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float driftCur_ = 0.0f;
    float driftTgt_ = 0.0f;
    float uPrev_ = 0.0f;   // ADAA: entrada driveada da sub-amostra anterior
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 6000;
    std::uint64_t rng_ = 0x100000001B3ULL;
    Oversampler2x os_{};
};

}  // namespace rasgo::modular
