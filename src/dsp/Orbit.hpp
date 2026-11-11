#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// ORBIT (#90) — planetas em gravidade (rascunho de 2026-11-09; entrou em
// 11/11/2026, decisão do autor: "magnet e orbit entram"; dossiê
// `dossies/90_orbit.md`; a
// física é a mesma do céu do Asterismus: gravitação de Newton integrada por
// leapfrog simplético, `PerformanceSkyDirector.hpp` — conceito, não código).
//
// Um sol no centro e 2 ou 3 planetas. Cada planeta é puxado pelo sol e, na
// proporção de MASS, pelos outros planetas (perturbação: com MASS alta as
// órbitas se desfazem em caos). RATIO afasta as órbitas (pela 3ª lei de
// Kepler, a órbita RATIO vezes maior demora RATIO^1,5 vezes mais); ECC
// achata as elipses. Leapfrog (kick-drift-kick): a energia não deriva.
// Conceito lido no AlgoritmArte Planetz (GPL-3.0+); nenhum código copiado.
//
// Saídas: X1/Y1, X2/Y2 (posições, −1..1), DIST (distância entre os planetas 1
// e 2, 0–1), CONJ (pulso na conjunção: os dois alinhados com o sol), PERI
// (pulso quando o planeta 1 passa no ponto mais perto do sol).
//
// Desvio Rasgo:
//  (a) LIGHT — o "tempo-luz" do Asterismus: o pulso de conjunção chega
//      atrasado pela distância entre os planetas (0 = na hora; 1 = até ~1 s);
//  (b) NUDGE — um gatilho empurra o planeta 1 (a órbita muda de forma);
//  (c) um planeta que escapa (ou cai no sol) volta à sua órbita: o sistema
//      nunca se perde.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Orbit final : public Signal {
public:
    static constexpr int kMax = 3;

    Orbit() : Signal(
        {{"nudge", PortKind::Control, "trig"},
         {"speed_cv", PortKind::Control, ""}},
        {{"x1", PortKind::Control, ""},
         {"y1", PortKind::Control, ""},
         {"x2", PortKind::Control, ""},
         {"y2", PortKind::Control, ""},
         {"dist", PortKind::Control, ""},
         {"conj", PortKind::Control, "gate"},
         {"peri", PortKind::Control, "gate"}},
        {{"planets", 2.0f, 3.0f, 3.0f, ""},
         {"ratio", 1.2f, 2.5f, 1.6f, ""},
         {"ecc", 0.0f, 0.8f, 0.2f, ""},
         {"mass", 0.0f, 1.0f, 0.15f, ""},
         {"speed", 0.05f, 4.0f, 1.0f, ""},
         {"light", 0.0f, 1.0f, 0.3f, ""}}) {}

    std::string type() const override { return "ORBIT"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "ORBIT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "orbit", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "PLNTS", "planets", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RATIO", "ratio", 25.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ECC", "ecc", 43.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MASS", "mass", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 25.0f, 52.0f);
        p.add(Widget::Kind::Knob, "LIGHT", "light", 43.0f, 52.0f);
        p.add(Widget::Kind::Jack, "NUDGE", "in:nudge", 8.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SPD", "in:speed_cv", 22.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CONJ", "out:conj", 35.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PERI", "out:peri", 48.0f, 96.0f);
        p.add(Widget::Kind::Jack, "X1", "out:x1", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "Y1", "out:y1", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "X2", "out:x2", 29.0f, 116.0f);
        p.add(Widget::Kind::Jack, "Y2", "out:y2", 40.0f, 116.0f);
        p.add(Widget::Kind::Jack, "DIST", "out:dist", 52.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        prevNudge_ = 0.0f;
        conjDelay_ = -1;
        conjSamples_ = periSamples_ = 0;
        ratio_ = std::clamp(parameterValue("ratio"), 1.2f, 2.5f);
        ecc_ = std::clamp(parameterValue("ecc"), 0.0f, 0.8f);
        for (int i = 0; i < kMax; ++i) place(i);
        prevRel_ = relAngle();
        prevRadial_ = 0.0;
    }

    // Energia total (cinética + potencial do sol e mútua) — para os testes.
    double energy(double massK) const noexcept {
        double e = 0.0;
        for (int i = 0; i < kMax; ++i) {
            const auto& b = b_[static_cast<std::size_t>(i)];
            e += 0.5 * kM * (b.vx * b.vx + b.vy * b.vy) - kM / std::sqrt(b.x * b.x + b.y * b.y + kEps);
            for (int j = i + 1; j < kMax; ++j) {
                const auto& c = b_[static_cast<std::size_t>(j)];
                const double dx = b.x - c.x, dy = b.y - c.y;
                e -= massK * kM * kM / std::sqrt(dx * dx + dy * dy + kEps);
            }
        }
        return e;
    }
    double radius(int i) const noexcept {
        const auto& b = b_[static_cast<std::size_t>(std::clamp(i, 0, kMax - 1))];
        return std::sqrt(b.x * b.x + b.y * b.y);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 7) return;
        const AudioBlock* nudge = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* scv = inputs.size() > 1 ? inputs[1] : nullptr;

        const int planets = std::clamp(static_cast<int>(std::lround(safe(parameterValue("planets"), 3.0f))), 2, kMax);
        const float ratio = std::clamp(safe(parameterValue("ratio"), 1.6f), 1.2f, 2.5f);
        const float ecc = std::clamp(safe(parameterValue("ecc"), 0.2f), 0.0f, 0.8f);
        const double massK = static_cast<double>(std::clamp(safe(parameterValue("mass"), 0.15f), 0.0f, 1.0f)) * 0.05;
        const float speedK = std::clamp(safe(parameterValue("speed"), 1.0f), 0.05f, 4.0f);
        const float light = std::clamp(safe(parameterValue("light"), 0.3f), 0.0f, 1.0f);
        // Mudou a forma pedida (RATIO/ECC)? Os planetas vão para as órbitas novas.
        if (std::fabs(ratio - ratio_) > 1e-4f || std::fabs(ecc - ecc_) > 1e-4f) {
            ratio_ = ratio; ecc_ = ecc;
            for (int i = 0; i < kMax; ++i) place(i);
        }
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (nudge) {
                const float n = safe(nudge->at(0, f), 0.0f);
                if (prevNudge_ < 0.5f && n >= 0.5f) { b_[0].vx *= 1.15; b_[0].vy *= 1.15; }
                prevNudge_ = n;
            }
            const float cv = scv ? std::clamp(safe(scv->at(0, f), 0.0f), -2.0f, 2.0f) : 0.0f;
            // SPEED 1 → o planeta 1 (raio 0,3) dá uma volta em ~2 s na órbita
            // circular (ECC 0); com ECC o planeta parte do afélio, a órbita
            // encolhe e o período cai pela 3ª lei de Kepler (ECC 0,3: ~1,35 s)
            const double dt = kTimeScale * static_cast<double>(speedK) * std::exp2(static_cast<double>(cv)) / sr_;
            leapfrog(dt, planets, massK);
            for (int i = 0; i < planets; ++i) {
                const auto& b = b_[static_cast<std::size_t>(i)];
                const double r2 = b.x * b.x + b.y * b.y;
                if (!std::isfinite(r2) || r2 > 2.25 || r2 < 0.0009) place(i);   // escapou ou caiu
            }
            // conjunção 1–2: o ângulo relativo passa por zero
            const double rel = relAngle();
            if (prevRel_ < 0.0 && rel >= 0.0 && std::fabs(rel - prevRel_) < 1.0) {
                const double dx = b_[0].x - b_[1].x, dy = b_[0].y - b_[1].y;
                conjDelay_ = static_cast<int>(static_cast<double>(light) * std::sqrt(dx * dx + dy * dy) * sr_);
            }
            prevRel_ = rel;
            if (conjDelay_ >= 0 && conjDelay_-- == 0) conjSamples_ = pulse;
            // periélio do planeta 1: a velocidade radial passa de − para +
            const double radial = b_[0].x * b_[0].vx + b_[0].y * b_[0].vy;
            if (prevRadial_ < 0.0 && radial >= 0.0) periSamples_ = pulse;
            prevRadial_ = radial;

            const double dx = b_[0].x - b_[1].x, dy = b_[0].y - b_[1].y;
            const float o[7] = {
                static_cast<float>(std::clamp(b_[0].x, -1.0, 1.0)), static_cast<float>(std::clamp(b_[0].y, -1.0, 1.0)),
                static_cast<float>(std::clamp(b_[1].x, -1.0, 1.0)), static_cast<float>(std::clamp(b_[1].y, -1.0, 1.0)),
                static_cast<float>(std::clamp(std::sqrt(dx * dx + dy * dy) * 0.5, 0.0, 1.0)),
                conjSamples_ > 0 ? 1.0f : 0.0f, periSamples_ > 0 ? 1.0f : 0.0f};
            if (conjSamples_ > 0) --conjSamples_;
            if (periSamples_ > 0) --periSamples_;
            for (std::size_t c = 0; c < channels; ++c)
                for (int k = 0; k < 7; ++k)
                    outputs[static_cast<std::size_t>(k)].at(c, f) = std::isfinite(o[k]) ? o[k] : 0.0f;
        }
    }

private:
    struct Body { double x = 0, y = 0, vx = 0, vy = 0; };
    static constexpr double kM = 1.0;          // massa do planeta (unidade)
    static constexpr double kGM = 1.0;         // G·M do sol
    static constexpr double kEps = 1e-4;       // amaciamento (sem singularidade)
    static constexpr double kR1 = 0.3;         // raio da órbita 1
    // período da órbita 1 = 2π√(r³/GM) ≈ 1,0326 → ~2 s com SPEED 1
    static constexpr double kTimeScale = 1.0326 / 2.0;

    // Começa no afélio: raio r, velocidade tangencial √(GM/r)·√(1 − e).
    void place(int i) noexcept {
        const double r = std::min(1.2, kR1 * std::pow(static_cast<double>(ratio_), static_cast<double>(i)));
        const double a0 = 2.0943951 * static_cast<double>(i);   // 120° entre eles
        const double v = std::sqrt(kGM / r) * std::sqrt(1.0 - static_cast<double>(ecc_));
        auto& b = b_[static_cast<std::size_t>(i)];
        b.x = r * std::cos(a0); b.y = r * std::sin(a0);
        b.vx = -v * std::sin(a0); b.vy = v * std::cos(a0);
    }
    void accel(int planets, double massK, std::array<double, kMax>& ax, std::array<double, kMax>& ay) const noexcept {
        for (int i = 0; i < planets; ++i) {
            const auto& b = b_[static_cast<std::size_t>(i)];
            const double r2 = b.x * b.x + b.y * b.y + kEps;
            const double inv = kGM / (r2 * std::sqrt(r2));
            double gx = -b.x * inv, gy = -b.y * inv;
            for (int j = 0; j < planets; ++j) {
                if (j == i) continue;
                const auto& c = b_[static_cast<std::size_t>(j)];
                const double dx = c.x - b.x, dy = c.y - b.y;
                const double d2 = dx * dx + dy * dy + kEps;
                const double k = massK * kM / (d2 * std::sqrt(d2));
                gx += dx * k; gy += dy * k;
            }
            ax[static_cast<std::size_t>(i)] = gx; ay[static_cast<std::size_t>(i)] = gy;
        }
    }
    void leapfrog(double dt, int planets, double massK) noexcept {
        std::array<double, kMax> ax{}, ay{};
        accel(planets, massK, ax, ay);
        for (int i = 0; i < planets; ++i) {
            auto& b = b_[static_cast<std::size_t>(i)];
            b.vx += 0.5 * dt * ax[static_cast<std::size_t>(i)]; b.vy += 0.5 * dt * ay[static_cast<std::size_t>(i)];
            b.x += dt * b.vx; b.y += dt * b.vy;
        }
        accel(planets, massK, ax, ay);
        for (int i = 0; i < planets; ++i) {
            auto& b = b_[static_cast<std::size_t>(i)];
            b.vx += 0.5 * dt * ax[static_cast<std::size_t>(i)]; b.vy += 0.5 * dt * ay[static_cast<std::size_t>(i)];
        }
    }
    double relAngle() const noexcept {
        const double a = std::atan2(b_[0].y, b_[0].x) - std::atan2(b_[1].y, b_[1].x);
        return std::remainder(a, 6.283185307179586);
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    std::array<Body, kMax> b_{};
    float ratio_ = 1.6f, ecc_ = 0.2f;
    float prevNudge_ = 0.0f;
    int conjDelay_ = -1, conjSamples_ = 0, periSamples_ = 0;
    double prevRel_ = 0.0, prevRadial_ = 0.0;
};

} // namespace rasgo::modular
