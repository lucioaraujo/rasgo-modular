#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

// ============================================================================
// ABACUS — Aritmética binária de CV (Módulo 31)
// ============================================================================
//
// O `LOGIC` recombina o TEMPO; o `CONTROL` faz utilidades CONTÍNUAS de CV.
// Este trata a CV como NÚMERO INTEIRO: resto, quantização a degraus, e um
// CONTADOR BINÁRIO cujos bits viram ritmo (ideia do Numeric Repetitor —
// padrão que emerge de contar). É também o RETIFICADOR dedicado (meia-onda
// +/−, onda completa, sinal). Sem `a` conectado, a fonte é a própria rampa
// do contador — o ABACUS sozinho toca melodia (`quant`) + ritmo (`p1/p2/carry`).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/31_abacus.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Noise Engineering Numeric Repetitor / Bin Seq (contador + máscara → ritmo);
//   - retificador clássico (meia-onda / onda-completa como saídas separadas);
//   - aritmética modular (a mod n dobra a reta numa janela);
//   - divisor binário / Gray code (bit k = ÷2^(k+1); XOR de bits adjacentes).
//
// Determinístico: aritmética pura, sem RNG. Contador exato.

namespace rasgo::modular {

class Abacus final : public Signal {
public:
    Abacus()
        : Signal(
              {{"a", PortKind::Control, ""},
               {"b", PortKind::Control, ""},
               {"clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"}},
              {{"math", PortKind::Control, ""},
               {"quant", PortKind::Control, ""},
               {"rect", PortKind::Control, ""},
               {"p1", PortKind::Control, "gate"},
               {"p2", PortKind::Control, "gate"},
               {"carry", PortKind::Control, "gate"}},
              {{"op", 0.0f, 7.0f, 0.0f, ""},
               {"modulus", 2.0f, 32.0f, 8.0f, ""},
               {"steps", 2.0f, 16.0f, 8.0f, ""},
               {"range", 0.1f, 4.0f, 1.0f, ""},
               {"rect_mode", 0.0f, 3.0f, 2.0f, ""},
               {"count_step", -4.0f, 4.0f, 1.0f, ""},
               {"pattern", 0.0f, 1.0f, 0.3f, ""},
               {"slew", 0.0f, 1.0f, 0.0f, ""},
               {"rate", 0.1f, 30.0f, 4.0f, "Hz"}}) {}

    std::string type() const override { return "ABACUS"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "ABACUS", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "count", "", 2.5f, 6.0f, 66.1f);
        p.add(Widget::Kind::Knob, "OP", "op", 9.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MOD", "modulus", 25.0f, 28.0f);
        p.add(Widget::Kind::Knob, "STEP", "steps", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RNG", "range", 57.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RECT", "rect_mode", 9.0f, 46.0f);
        p.add(Widget::Kind::Knob, "CNT", "count_step", 25.0f, 46.0f);
        p.add(Widget::Kind::Knob, "PAT", "pattern", 41.0f, 46.0f);
        p.add(Widget::Kind::Knob, "SLEW", "slew", 57.0f, 46.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 9.0f, 64.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 7.0f, 88.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 19.0f, 88.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 31.0f, 88.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 43.0f, 88.0f);
        p.add(Widget::Kind::Jack, "MTH", "out:math", 7.0f, 112.0f);
        p.add(Widget::Kind::Jack, "QNT", "out:quant", 18.0f, 112.0f);
        p.add(Widget::Kind::Jack, "RCT", "out:rect", 29.0f, 112.0f);
        p.add(Widget::Kind::Jack, "P1", "out:p1", 40.0f, 112.0f);
        p.add(Widget::Kind::Jack, "P2", "out:p2", 51.0f, 112.0f);
        p.add(Widget::Kind::Jack, "CRY", "out:carry", 62.0f, 112.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        dt_ = 1.0f / sr_;
        count_ = 0;
        prevBucket_ = 0;
        prevClock_ = false;
        prevReset_ = false;
        started_ = false;
        phase_ = 0.0f;
        p1_ = 0.0f;
        p2_ = 0.0f;
        aSrc_ = 0.0f;
        carryCd_ = 0;
        mathOut_ = 0.0f;
        quantOut_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const int op = clampi(
            static_cast<int>(std::lround(parameterValue("op"))), 0, 7);
        const long modulus = static_cast<long>(clampi(
            static_cast<int>(std::lround(parameterValue("modulus"))), 2, 32));
        const int steps = clampi(
            static_cast<int>(std::lround(parameterValue("steps"))), 2, 16);
        const float range = clampf(parameterValue("range"), 0.1f, 4.0f);
        const int rectMode = clampi(
            static_cast<int>(std::lround(parameterValue("rect_mode"))), 0, 3);
        const long countStep = static_cast<long>(
            std::lround(clampf(parameterValue("count_step"), -4.0f, 4.0f)));
        const float pattern = clamp01(parameterValue("pattern"));
        const float slew = clamp01(parameterValue("slew"));
        const float rate = std::max(0.01f, parameterValue("rate"));

        const float slewTime = slew * slew * 0.5f;
        const float slewCoef =
            slewTime < dt_ ? 1.0f : (1.0f - std::exp(-dt_ / slewTime));
        const float qStep = range / static_cast<float>(steps);
        const int bitA = clampi(static_cast<int>(pattern * 3.999f), 0, 3);
        const int carryLen = static_cast<int>(0.005f * sr_) + 1;

        const AudioBlock* inA = inputs[0];
        const AudioBlock* inB = inputs[1];
        const AudioBlock* clock = inputs[2];
        const AudioBlock* reset = inputs[3];
        const bool externalClock = (clock != nullptr);
        const bool haveA = (inA != nullptr);
        const float intInc = rate / sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            if (reset != nullptr) {
                const bool hi = reset->at(0, f) >= 0.5f;
                if (hi && !prevReset_) {
                    count_ = 0;
                    prevBucket_ = 0;
                    started_ = false;
                    phase_ = 0.0f;
                }
                prevReset_ = hi;
            }

            // ---- tique do contador ----
            bool tick = false;
            if (externalClock) {
                const bool hi = clock->at(0, f) >= 0.5f;
                if (hi && !prevClock_) tick = true;
                prevClock_ = hi;
            } else {
                phase_ += intInc;
                if (phase_ >= 1.0f) { phase_ -= 1.0f; tick = true; }
            }
            if (tick) {
                if (started_) count_ += countStep;
                started_ = true;
                const long bucket = floorDiv(count_, modulus);
                if (bucket != prevBucket_) {
                    carryCd_ = carryLen;
                    prevBucket_ = bucket;
                }
                const long c = posMod(count_, modulus);
                const long b0 = (c >> bitA) & 1;
                const long b1 = (c >> (bitA + 1)) & 1;
                p1_ = static_cast<float>(b0);
                p2_ = static_cast<float>(b0 ^ b1);
                aSrc_ = (static_cast<float>(c) / static_cast<float>(modulus))
                        * 2.0f * range - range;
            }

            const float a = haveA ? inA->at(0, f) : aSrc_;
            const float b = inB != nullptr ? inB->at(0, f) : 0.0f;

            // ---- math ----
            // op 0-3: aritmética contínua. op 4-7: BIT A BIT (Lunetta) —
            // `a`/`b` viram inteiros de 5 bits (janela ±range → 0..31), a
            // operação lógica roda nos bits, o resultado volta a ±range.
            float m;
            if (op >= 4) {
                const long ia = clampl(std::lround(
                    (a + range) / (2.0f * range) * 31.0f), 0, 31);
                const long ib = clampl(std::lround(
                    (b + range) / (2.0f * range) * 31.0f), 0, 31);
                long ir;
                switch (op) {
                    case 5:  ir = ia | ib; break;
                    case 6:  ir = ia ^ ib; break;
                    case 7:  ir = (~(ia & ib)) & 31; break;
                    default: ir = ia & ib; break;   // 4: AND
                }
                m = static_cast<float>(ir) / 31.0f * 2.0f * range - range;
            } else {
                switch (op) {
                    case 1: m = a - b; break;
                    case 2: m = a * b; break;
                    case 3: m = a - std::floor(a / range) * range; break;
                    default: m = a + b; break;
                }
            }
            m = clampf(m, -8.0f, 8.0f);
            mathOut_ += (m - mathOut_) * slewCoef;

            // ---- quant ----
            const float qv =
                clampf(std::round(a / qStep) * qStep, -8.0f, 8.0f);
            quantOut_ += (qv - quantOut_) * slewCoef;

            // ---- rect ----
            float r;
            switch (rectMode) {
                case 0: r = a > 0.0f ? a : 0.0f; break;
                case 1: r = a < 0.0f ? a : 0.0f; break;
                case 3:
                    r = a > 1e-4f ? range : (a < -1e-4f ? -range : 0.0f);
                    break;
                default: r = std::fabs(a); break;
            }

            const float carry = carryCd_ > 0 ? 1.0f : 0.0f;
            if (carryCd_ > 0) --carryCd_;

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = mathOut_;
                outputs[1].at(c, f) = quantOut_;
                outputs[2].at(c, f) = r;
                outputs[3].at(c, f) = p1_;
                outputs[4].at(c, f) = p2_;
                outputs[5].at(c, f) = carry;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static long clampl(const long v, const long lo, const long hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    static long floorDiv(const long a, const long n) noexcept {
        const long q = a / n;
        return (a % n != 0 && ((a < 0) != (n < 0))) ? q - 1 : q;
    }
    static long posMod(const long a, const long n) noexcept {
        const long r = a % n;
        return r < 0 ? r + n : r;
    }

    float sr_ = 48000.0f;
    float dt_ = 1.0f / 48000.0f;
    long count_ = 0;
    long prevBucket_ = 0;
    bool prevClock_ = false;
    bool prevReset_ = false;
    bool started_ = false;
    float phase_ = 0.0f;
    float p1_ = 0.0f;
    float p2_ = 0.0f;
    float aSrc_ = 0.0f;
    int carryCd_ = 0;
    float mathOut_ = 0.0f;
    float quantOut_ = 0.0f;
};

}  // namespace rasgo::modular
