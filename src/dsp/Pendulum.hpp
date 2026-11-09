#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// PENDULUM (#70) — pêndulo duplo (2026-11-09, ideia do autor: "pêndulos").
//
// Um pêndulo pendurado na ponta de outro: com pouca energia, balança quase
// regular; com muita, o de baixo dá voltas inteiras e o movimento vira caos
// (sensível ao menor detalhe). Equações de Lagrange do pêndulo duplo, de
// livro-texto (massas pontuais, hastes rígidas sem massa), integradas por
// Runge-Kutta de 4ª ordem. No Cardinal não há pêndulo (os caóticos de lá são
// atratores: Sapphire Frolic/Glee/Lark, HetrickCV); ideia e implementação
// próprias.
//
// Saídas: X1/Y1 e X2/Y2 (as posições das duas pontas, −1..1, o centro no
// pivô), CROSS (um pulso quando a ponta de baixo passa pelo meio) e FLIP (um
// pulso quando o braço de baixo dá a volta por cima — o sinal do caos).
//
// Desvio Rasgo:
//  (a) AUTO — o atrito (DAMP) gasta a energia; quando ela cai abaixo de um
//      quinto da inicial, um empurrão SEMEADO devolve o movimento: o pêndulo
//      nunca para;
//  (b) KICK — um gatilho empurra o braço de baixo (mais energia, mais caos).
//
// Parâmetros: ENERGY (o ângulo inicial: de quase parado a de cabeça para
// baixo), RATIO (comprimento do braço de baixo / de cima), MASS (massa de
// baixo / de cima), SPEED (a escala de tempo), DAMP. process() não aloca;
// determinístico.
namespace rasgo::modular {

class Pendulum final : public Signal {
public:
    Pendulum() : Signal(
        {{"kick", PortKind::Control, "trig"},
         {"speed_cv", PortKind::Control, ""}},
        {{"x1", PortKind::Control, ""},
         {"y1", PortKind::Control, ""},
         {"x2", PortKind::Control, ""},
         {"y2", PortKind::Control, ""},
         {"cross", PortKind::Control, "gate"},
         {"flip", PortKind::Control, "gate"}},
        {{"energy", 0.0f, 1.0f, 0.6f, ""},
         {"ratio", 0.3f, 2.0f, 1.0f, ""},
         {"mass", 0.2f, 3.0f, 1.0f, ""},
         {"speed", 0.05f, 4.0f, 1.0f, ""},
         {"damp", 0.0f, 1.0f, 0.1f, ""},
         {"auto", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "PENDULUM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "PENDULUM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "pendulum", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "ENRGY", "energy", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATIO", "ratio", 25.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MASS", "mass", 43.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "DAMP", "damp", 25.0f, 52.0f);
        p.add(Widget::Kind::Toggle, "AUTO", "auto", 45.3f, 56.0f);
        p.add(Widget::Kind::Jack, "KICK", "in:kick", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SPD", "in:speed_cv", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CROSS", "out:cross", 35.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FLIP", "out:flip", 51.0f, 96.0f);
        p.add(Widget::Kind::Jack, "X1", "out:x1", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "Y1", "out:y1", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "X2", "out:x2", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "Y2", "out:y2", 43.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x9E17D0170070ULL;
        prevKick_ = 0.0f;
        crossSamples_ = flipSamples_ = 0;
        const float e = std::clamp(parameterValue("energy"), 0.0f, 1.0f);
        launch(e);
        prevX2_ = 0.0;
        turns_ = 0.0;
    }

    double energy() const noexcept { return energyOf(s_); }
    long flips() const noexcept { return flipCount_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* kick = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* scv = inputs.size() > 1 ? inputs[1] : nullptr;

        const double ratio = std::clamp(safe(parameterValue("ratio"), 1.0f), 0.3f, 2.0f);
        const double mass = std::clamp(safe(parameterValue("mass"), 1.0f), 0.2f, 3.0f);
        const float speedK = std::clamp(safe(parameterValue("speed"), 1.0f), 0.05f, 4.0f);
        const double damp = std::clamp(safe(parameterValue("damp"), 0.1f), 0.0f, 1.0f) * 0.6;
        const bool autoKick = safe(parameterValue("auto"), 1.0f) >= 0.5f;
        const float energyK = std::clamp(safe(parameterValue("energy"), 0.6f), 0.0f, 1.0f);
        l2_ = ratio;
        m2_ = mass;
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));
        const double norm = 1.0 / (1.0 + ratio);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (kick) {
                const float k = safe(kick->at(0, f), 0.0f);
                if (prevKick_ < 0.5f && k >= 0.5f) s_[3] += 4.0 * (unit() < 0.5f ? -1.0 : 1.0);
                prevKick_ = k;
            }
            const float cv = scv ? std::clamp(safe(scv->at(0, f), 0.0f), -2.0f, 2.0f) : 0.0f;
            // SPEED 1 → o pêndulo de 1 m sob g = 9,8 (período ~2 s); a CV dobra a cada +1
            const double dt = static_cast<double>(speedK) * std::exp2(static_cast<double>(cv)) / sr_;
            rk4(dt, damp);

            const double x1 = std::sin(s_[0]), y1 = -std::cos(s_[0]);
            const double x2 = x1 + l2_ * std::sin(s_[1]), y2 = y1 - l2_ * std::cos(s_[1]);
            if ((prevX2_ < 0.0) != (x2 < 0.0) && y2 < 0.0) crossSamples_ = pulse;
            prevX2_ = x2;
            // FLIP: o braço de baixo completou meia volta a mais por cima
            const double t = std::floor((s_[1] + kPi) / (2.0 * kPi));
            if (t != turns_) { flipSamples_ = pulse; turns_ = t; ++flipCount_; if (flipCount_ > 1000000000L) flipCount_ = 0; }

            if (autoKick && energyOf(s_) < 0.2 * targetEnergy(energyK)) launch(std::max(0.2f, energyK));

            const float cross = crossSamples_ > 0 ? 1.0f : 0.0f;
            const float flip = flipSamples_ > 0 ? 1.0f : 0.0f;
            if (crossSamples_ > 0) --crossSamples_;
            if (flipSamples_ > 0) --flipSamples_;
            const float o[6] = {static_cast<float>(x1 * norm), static_cast<float>(y1 * norm),
                                static_cast<float>(x2 * norm), static_cast<float>(y2 * norm), cross, flip};
            for (std::size_t c = 0; c < channels; ++c)
                for (int k = 0; k < 6; ++k)
                    outputs[static_cast<std::size_t>(k)].at(c, f) = std::isfinite(o[k]) ? o[k] : 0.0f;
        }
        // estado inválido (só com parâmetros absurdos) → relança
        for (double v : s_) if (!std::isfinite(v)) { launch(energyK); break; }
    }

private:
    using State = std::array<double, 4>;   // θ1, θ2, ω1, ω2
    static constexpr double kG = 9.8;
    static constexpr double kPi = 3.14159265358979323846;

    State deriv(const State& s, double damp) const noexcept {
        const double m1 = 1.0, l1 = 1.0, m2 = m2_, l2 = l2_;
        const double d = s[0] - s[1];
        const double den = 2.0 * m1 + m2 - m2 * std::cos(2.0 * d);
        const double a1 = (-kG * (2.0 * m1 + m2) * std::sin(s[0]) - m2 * kG * std::sin(s[0] - 2.0 * s[1])
                           - 2.0 * std::sin(d) * m2 * (s[3] * s[3] * l2 + s[2] * s[2] * l1 * std::cos(d)))
                          / (l1 * den);
        const double a2 = (2.0 * std::sin(d) * (s[2] * s[2] * l1 * (m1 + m2) + kG * (m1 + m2) * std::cos(s[0])
                           + s[3] * s[3] * l2 * m2 * std::cos(d)))
                          / (l2 * den);
        return {s[2], s[3], a1 - damp * s[2], a2 - damp * s[3]};
    }
    void rk4(double dt, double damp) noexcept {
        const State k1 = deriv(s_, damp);
        State t{};
        for (int i = 0; i < 4; ++i) t[static_cast<std::size_t>(i)] = s_[static_cast<std::size_t>(i)] + 0.5 * dt * k1[static_cast<std::size_t>(i)];
        const State k2 = deriv(t, damp);
        for (int i = 0; i < 4; ++i) t[static_cast<std::size_t>(i)] = s_[static_cast<std::size_t>(i)] + 0.5 * dt * k2[static_cast<std::size_t>(i)];
        const State k3 = deriv(t, damp);
        for (int i = 0; i < 4; ++i) t[static_cast<std::size_t>(i)] = s_[static_cast<std::size_t>(i)] + dt * k3[static_cast<std::size_t>(i)];
        const State k4 = deriv(t, damp);
        for (int i = 0; i < 4; ++i) {
            const auto u = static_cast<std::size_t>(i);
            s_[u] += dt / 6.0 * (k1[u] + 2.0 * k2[u] + 2.0 * k3[u] + k4[u]);
        }
    }
    // Energia acima do repouso (os dois pendurados parados = 0).
    double energyOf(const State& s) const noexcept {
        const double m1 = 1.0, l1 = 1.0, m2 = m2_, l2 = l2_;
        const double v1sq = l1 * l1 * s[2] * s[2];
        const double v2sq = v1sq + l2 * l2 * s[3] * s[3] + 2.0 * l1 * l2 * s[2] * s[3] * std::cos(s[0] - s[1]);
        const double kin = 0.5 * m1 * v1sq + 0.5 * m2 * v2sq;
        const double y1 = -l1 * std::cos(s[0]), y2 = y1 - l2 * std::cos(s[1]);
        const double pot = m1 * kG * (y1 + l1) + m2 * kG * (y2 + l1 + l2);
        return kin + pot;
    }
    // A energia de um lançamento parado com os dois braços no ângulo de ENERGY.
    double targetEnergy(float e) const noexcept {
        const double a = angleFor(e);
        return energyOf({a, a, 0.0, 0.0});
    }
    static double angleFor(float e) noexcept { return 0.15 + static_cast<double>(e) * 2.9; }   // ~9° … ~175°
    void launch(float e) noexcept {
        const double a = angleFor(e);
        s_ = {a * (0.9 + 0.2 * unit()), a * (0.9 + 0.2 * unit()) * (unit() < 0.5f ? 1.0 : -1.0), 0.0, 0.0};
        turns_ = std::floor((s_[1] + kPi) / (2.0 * kPi));
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x9E17D0170070ULL;
    State s_{};
    double l2_ = 1.0, m2_ = 1.0;
    float prevKick_ = 0.0f;
    int crossSamples_ = 0, flipSamples_ = 0;
    double prevX2_ = 0.0, turns_ = 0.0;
    long flipCount_ = 0;
};

} // namespace rasgo::modular
