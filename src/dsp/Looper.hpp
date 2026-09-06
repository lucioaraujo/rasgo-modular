#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// LOOPER — delay com hold / reverse / fita (Módulo 41)
// ============================================================================
//
// O `SPACE` é reverb, o `MEMORY` é granular. `LOOPER` é o delay de LINHA
// com os três gestos que o `PESQUISA §6` pediu: HOLD (congela e repete
// infinito), REVERSE (lê pra trás, sem clique) e caráter de FITA/BBD num
// knob (`age` = perda de agudo no laço + wow&flutter + saturação + ruído).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/41_looper.md`.
//
// `age = 0`, sem `hold`/`reverse` → delay digital limpo. Determinístico
// exceto pelo ruído de `age` (semeado — reprodutível). Buffer de ~2,2 s
// pré-alocado no `prepare()`; `process()` não aloca.

namespace rasgo::modular {

class Looper final : public Signal {
public:
    Looper()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"time", PortKind::Control, ""},
               {"freeze", PortKind::Control, "gate"},
               {"rev", PortKind::Control, "gate"}},
              {{"out", PortKind::Audio, ""},
               {"wet", PortKind::Audio, ""}},
              {{"time", 0.001f, 2.0f, 0.3f, "s"},
               {"feedback", 0.0f, 1.1f, 0.4f, ""},
               {"age", 0.0f, 1.0f, 0.2f, ""},
               {"mix", 0.0f, 1.0f, 0.5f, ""},
               {"hold", 0.0f, 1.0f, 0.0f, ""},
               {"reverse", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "LOOPER"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "LOOPER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "echo", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "TIME", "time", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "AGE", "age", 40.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 8.0f, 54.0f);
        p.add(Widget::Kind::Toggle, "HOLD", "hold", 26.0f, 56.0f);
        p.add(Widget::Kind::Toggle, "REV", "reverse", 40.0f, 56.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 94.0f);
        p.add(Widget::Kind::Jack, "TIME", "in:time", 21.0f, 94.0f);
        p.add(Widget::Kind::Jack, "FRZ", "in:freeze", 34.0f, 94.0f);
        p.add(Widget::Kind::Jack, "REV", "in:rev", 47.0f, 94.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "WET", "out:wet", 21.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        const std::size_t n =
            static_cast<std::size_t>(2.2f * sr_) + 4;
        buf_.assign(n, 0.0f);
        len_ = n;
        w_ = 0;
        dSmooth_ = clampf(parameterValue("time") * sr_, 4.0f,
                          static_cast<float>(n - 4));
        revPh_ = 0.0f;
        lpZ_ = 0.0f;
        wowPh_ = 0.0f;
        flutPh_ = 0.0f;
        prevHold_ = false;
        holdAnchor_ = 0;
        holdCount_ = 0;
        rng_ = 0x2E1B9A57C3D40FULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& wetOut = outputs[1];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float timeP = clampf(parameterValue("time"), 0.001f, 2.0f);
        const float fbk = clampf(parameterValue("feedback"), 0.0f, 1.1f);
        const float age = clamp01(parameterValue("age"));
        const float mix = clamp01(parameterValue("mix"));
        const bool holdP = parameterValue("hold") >= 0.5f;
        const bool revP = parameterValue("reverse") >= 0.5f;

        const AudioBlock* in = inputs[0];
        const AudioBlock* timeIn = inputs[1];
        const AudioBlock* frzIn = inputs[2];
        const AudioBlock* revIn = inputs[3];

        const float lpCoef = 1.0f - std::exp(-6.28318f * (12000.0f - age * 10500.0f)
                                             / sr_);
        const float wowInc = 0.9f / sr_;    // ~0,9 Hz
        const float flutInc = 6.5f / sr_;   // ~6,5 Hz
        const float maxD = static_cast<float>(len_ - 4);

        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in ? in->at(0, f) : 0.0f;
            const bool holdA =
                holdP || (frzIn && frzIn->at(0, f) >= 0.5f);
            const bool revA =
                revP || (revIn && revIn->at(0, f) >= 0.5f);

            // tempo suavizado
            float dTarget = timeP * sr_;
            if (timeIn) dTarget += timeIn->at(0, f) * sr_;
            dTarget = clampf(dTarget, 4.0f, maxD);
            dSmooth_ += (dTarget - dSmooth_) * 0.0005f;

            // borda de entrada em HOLD: ancora a janela
            if (holdA && !prevHold_) {
                holdAnchor_ = w_;
                holdCount_ = 0;
            }
            prevHold_ = holdA;

            // wow & flutter
            wowPh_ += wowInc; wowPh_ -= std::floor(wowPh_);
            flutPh_ += flutInc; flutPh_ -= std::floor(flutPh_);
            const float wow = age * (0.003f * std::sin(6.28318f * wowPh_)
                                     + 0.0015f * std::sin(6.28318f * flutPh_));

            // ---- leitura ----
            float wet;
            const double wRef =
                holdA ? static_cast<double>(holdAnchor_)
                      : static_cast<double>(w_);
            if (revA) {
                const float d = dSmooth_;
                const float gA = 0.5f - 0.5f * std::cos(6.28318f * revPh_);
                const float gB = 1.0f - gA;
                const float pB = revPh_ + 0.5f - std::floor(revPh_ + 0.5f);
                const float a = readBuf(wRef - 1.0 - static_cast<double>(revPh_ * d));
                const float b = readBuf(wRef - 1.0 - static_cast<double>(pB * d));
                wet = a * gA + b * gB;
                revPh_ += 1.0f / d;
                revPh_ -= std::floor(revPh_);
            } else if (holdA) {
                const float d = dSmooth_;
                const float ph =
                    std::fmod(static_cast<float>(holdCount_), std::max(1.0f, d));
                wet = readBuf(static_cast<double>(holdAnchor_) - d
                              + static_cast<double>(ph));
            } else {
                wet = readBuf(wRef - static_cast<double>(dSmooth_ * (1.0f + wow)));
            }

            // ---- caráter de fita no laço ----
            float fb = wet;
            lpZ_ += (fb - lpZ_) * lpCoef;
            fb = lpZ_;
            fb = std::tanh(fb * (1.0f + age * 0.6f)) / (1.0f + age * 0.3f);
            fb += noise() * age * 0.0006f;

            // ---- escrita ----
            if (!holdA) {
                buf_[w_] = x + fb * fbk;
                w_ = (w_ + 1) % len_;
            }
            ++holdCount_;

            const float y = x * (1.0f - mix) + wet * mix;
            for (std::size_t c = 0; c < channels; ++c) {
                out.at(c, f) = y;
                wetOut.at(c, f) = wet;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float noise() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32)) / 2147483648.0f;
    }

    // leitura interpolada, índice absoluto (pode ser negativo/além — wrap)
    float readBuf(double idx) const noexcept {
        const double L = static_cast<double>(len_);
        idx = std::fmod(idx, L);
        if (idx < 0.0) idx += L;
        const std::size_t i0 = static_cast<std::size_t>(idx);
        const std::size_t i1 = (i0 + 1) % len_;
        const float fr = static_cast<float>(idx - static_cast<double>(i0));
        return buf_[i0] * (1.0f - fr) + buf_[i1] * fr;
    }

    std::vector<float> buf_;
    std::size_t len_ = 1;
    std::size_t w_ = 0;
    float sr_ = 48000.0f;
    float dSmooth_ = 14400.0f;
    float revPh_ = 0.0f;
    float lpZ_ = 0.0f;
    float wowPh_ = 0.0f, flutPh_ = 0.0f;
    bool prevHold_ = false;
    std::size_t holdAnchor_ = 0;
    std::uint64_t holdCount_ = 0;
    std::uint64_t rng_ = 0x2E1B9A57C3D40FULL;
};

}  // namespace rasgo::modular
