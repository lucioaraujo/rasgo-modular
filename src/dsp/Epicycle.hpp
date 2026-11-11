#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// EPICYCLE (#81) — círculos dentro de círculos (2026-11-11; dossiê
// `dossies/81_epicycle.md`; estudo da família §1 e §4.3, aprovado; "avance
// em todos eles"). Conceito do Batruc (epiciclos de Fourier desenhados:
// `GeometryCurves`), mesmo autor — conceito, não código.
//
// Até 8 círculos: o centro de cada um gira na borda do anterior. A soma é
// uma série de Fourier desenhada: z(t) = Σ rₖ·e^{i(ωₖ t + φₖ)}. Com poucos
// círculos e RATE lento, é ritmo e gesto (X/Y como CV); com muitos e RATE
// no áudio (PTCH em 1 V/oct), é timbre — a forma desenhada vira a onda.
//
// MODO — escolhas DIFERENTES, nomeadas (o crivo do Batruc: não chamar de
// "Fibonacci" o que não é):
//   0 QUADRADA  — velocidades ímpares (1, 3, 5…), raios 1/k: a série da onda
//                 quadrada;
//   1 SERRA     — velocidades 1, 2, 3…, raios 1/k: a série da dente de serra;
//   2 FIB-RAIO  — velocidades 1, 2, 3…, raios 1/Fₙ (Fibonacci nos PESOS);
//   3 FIB-VELOC — velocidades Fₙ (1, 2, 3, 5, 8…), raios 1/n (Fibonacci
//                 nas VELOCIDADES).
// DESV desafina as velocidades (rₖ fica): com DESV > 0 a figura nunca fecha.
// Saídas: X, Y (normalizadas: −1..1), R (o raio |z|, 0..1), ANG (o ângulo
// de z, 0..1) e GT (um pulso cada vez que Y cruza o zero subindo — o ritmo
// do desenho). Determinístico, sem alocação.
namespace rasgo::modular {

class Epicycle final : public Signal {
public:
    static constexpr int kMax = 8;

    Epicycle() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"reset", PortKind::Control, "trig"}},
        {{"x", PortKind::Audio, ""},
         {"y", PortKind::Audio, ""},
         {"radius", PortKind::Control, ""},
         {"angle", PortKind::Control, ""},
         {"gate", PortKind::Control, "trig"}},
        {{"rate", 0.01f, 2000.0f, 0.25f, "Hz"},
         {"circles", 1.0f, 8.0f, 3.0f, ""},
         {"mode", 0.0f, 3.0f, 0.0f, ""},
         {"detune", 0.0f, 1.0f, 0.0f, ""},
         {"phase", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "EPICYCLE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "EPICYCLE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "epicycle", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CIRC", "circles", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODO", "mode", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DESV", "detune", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "FASE", "phase", 23.0f, 54.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 7.0f, 92.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 92.0f);
        p.add(Widget::Kind::Jack, "X", "out:x", 7.0f, 106.0f);
        p.add(Widget::Kind::Jack, "Y", "out:y", 19.0f, 106.0f);
        p.add(Widget::Kind::Jack, "R", "out:radius", 31.0f, 106.0f);
        p.add(Widget::Kind::Jack, "ANG", "out:angle", 7.0f, 120.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 19.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        t_ = 0.0;
        prevY_ = 0.0f;
        prevReset_ = 0.0f;
        gateLeft_ = 0;
    }

    // velocidade e raio do círculo k (0..7) no modo dado
    static void circle(const int mode, const int k, double& speed, double& radius) noexcept {
        static constexpr double kFib[kMax + 2] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
        const double n = static_cast<double>(k + 1);
        switch (mode) {
        case 0: speed = 2.0 * n - 1.0; radius = 1.0 / (2.0 * n - 1.0); break;
        case 1: speed = n; radius = 1.0 / n; break;
        case 2: speed = n; radius = 1.0 / kFib[k]; break;
        default: speed = kFib[k]; radius = 1.0 / n; break;
        }
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const AudioBlock* pitch = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        const double rate = std::clamp(safe(parameterValue("rate"), 0.25f), 0.01f, 2000.0f);
        const int n = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("circles"), 3.0f), 1.0f, 8.0f)));
        const int mode = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("mode"), 0.0f), 0.0f, 3.0f)));
        const double detune = std::clamp(safe(parameterValue("detune"), 0.0f), 0.0f, 1.0f);
        const double phase0 = std::clamp(safe(parameterValue("phase"), 0.0f), 0.0f, 1.0f);
        std::array<double, kMax> sp{}, rd{};
        double sumR = 0.0;
        for (int k = 0; k < n; ++k) {
            circle(mode, k, sp[static_cast<std::size_t>(k)], rd[static_cast<std::size_t>(k)]);
            // DESV: um desvio irracional crescente por círculo (√2, √3…)
            sp[static_cast<std::size_t>(k)] *= 1.0 + detune * 0.08 * std::sqrt(static_cast<double>(k + 2)) * (k > 0 ? 1.0 : 0.0);
            sumR += rd[static_cast<std::size_t>(k)];
        }
        const double norm = sumR > 0.0 ? 1.0 / sumR : 1.0;
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));
        constexpr double kTau = 6.283185307179586;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) t_ = 0.0;
                prevReset_ = r;
            }
            const double hz = std::min(rate * (pitch ? std::exp2(std::clamp(
                static_cast<double>(safe(pitch->at(0, f), 0.0f)), -10.0, 10.0)) : 1.0),
                0.45 * sr_ / static_cast<double>(std::max(1.0, sp[static_cast<std::size_t>(n - 1)])));
            t_ += hz / sr_;
            if (t_ > 1.0e6) t_ = std::fmod(t_, 1.0e3);   // fase longa sem perder precisão
            double x = 0.0, y = 0.0;
            for (int k = 0; k < n; ++k) {
                const double a = kTau * (sp[static_cast<std::size_t>(k)] * t_ + phase0);
                x += rd[static_cast<std::size_t>(k)] * std::cos(a);
                y += rd[static_cast<std::size_t>(k)] * std::sin(a);
            }
            x *= norm;
            y *= norm;
            const float yf = static_cast<float>(y);
            if (prevY_ < 0.0f && yf >= 0.0f) gateLeft_ = pulse;
            prevY_ = yf;
            const float rad = static_cast<float>(std::sqrt(x * x + y * y));
            const float ang = static_cast<float>(std::atan2(y, x) / kTau + 0.5);
            const float gt = gateLeft_ > 0 ? 1.0f : 0.0f;
            if (gateLeft_ > 0) --gateLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = static_cast<float>(x);
                outputs[1].at(c, f) = yf;
                outputs[2].at(c, f) = rad;
                outputs[3].at(c, f) = ang;
                outputs[4].at(c, f) = gt;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    float sr_ = 48000.0f;
    double t_ = 0.0;
    float prevY_ = 0.0f, prevReset_ = 0.0f;
    int gateLeft_ = 0;
};

}  // namespace rasgo::modular
