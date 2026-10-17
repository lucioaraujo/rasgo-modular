#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <cmath>

// LISTEN (#61) — three causal analysis bands, levels and attack gates.
// Conceptual reference: Trioio's band listening (see dossies/61_listen.md).
// Original real-time implementation: no offline filter, buffer or dependency
// copied from Trioio. Bands overlap; they do not separate instruments.
namespace rasgo::modular {

class Listen final : public Signal {
public:
    Listen() : Signal(
        {{"in", PortKind::Audio, ""}},
        {{"thru", PortKind::Audio, ""},
         {"low", PortKind::Control, ""}, {"mid", PortKind::Control, ""},
         {"high", PortKind::Control, ""},
         {"low_hit", PortKind::Control, "gate"},
         {"mid_hit", PortKind::Control, "gate"},
         {"high_hit", PortKind::Control, "gate"}},
        {{"low_cut", 60.0f, 800.0f, 200.0f, "Hz"},
         {"high_cut", 1000.0f, 8000.0f, 2000.0f, "Hz"},
         {"response", 0.0f, 1.0f, 0.25f, ""},
         {"sens", 0.0f, 1.0f, 0.5f, ""},
         {"floor", -72.0f, -18.0f, -54.0f, "dBFS"},
         {"rearm", 20.0f, 250.0f, 70.0f, "ms"}}) {}

    std::string type() const override { return "LISTEN"; }

    Panel panel() const override {
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "LISTEN", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "bands", "", 2.5f, 7.0f, 75.0f);
        p.add(Widget::Kind::Knob, "LOW", "low_cut", 9.0f, 32.0f);
        p.add(Widget::Kind::Knob, "HIGH", "high_cut", 33.0f, 32.0f);
        p.add(Widget::Kind::Knob, "RESP", "response", 57.0f, 32.0f);
        p.add(Widget::Kind::Knob, "SENS", "sens", 9.0f, 55.0f);
        p.add(Widget::Kind::Knob, "FLOOR", "floor", 33.0f, 55.0f);
        p.add(Widget::Kind::Knob, "REARM", "rearm", 57.0f, 55.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 83.0f);
        p.add(Widget::Kind::Jack, "THRU", "out:thru", 30.0f, 83.0f);
        p.add(Widget::Kind::Jack, "LOW", "out:low", 8.0f, 104.0f);
        p.add(Widget::Kind::Jack, "MID", "out:mid", 30.0f, 104.0f);
        p.add(Widget::Kind::Jack, "HIGH", "out:high", 52.0f, 104.0f);
        p.add(Widget::Kind::Jack, "L-HIT", "out:low_hit", 8.0f, 121.0f);
        p.add(Widget::Kind::Jack, "M-HIT", "out:mid_hit", 30.0f, 121.0f);
        p.add(Widget::Kind::Jack, "H-HIT", "out:high_hit", 52.0f, 121.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        state_ = {};
        level_ = fast_ = slow_ = {};
        pulse_ = cooldown_ = {};
        warmup_ = 0;
        updateCuts();
        fastAttack_ = coefficient(0.0008f);
        fastRelease_ = coefficient(0.030f);
        slowAttack_ = coefficient(0.025f);
        slowRelease_ = coefficient(0.100f);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 7) return;
        const auto& p = parameters();
        if (safe(p[0].value, 200.0f) != lowCut_
            || safe(p[1].value, 2000.0f) != highCut_) {
            updateCuts();
            warmup_ = static_cast<int>(0.05f * sr_);
        }
        const float response = safe(p[2].value, 0.25f);
        const float attack = coefficient(0.001f + response * response * 0.08f);
        const float release = coefficient(0.04f + response * response * 1.2f);
        const float ratio = 1.45f + (1.0f - safe(p[3].value, 0.5f)) * 1.55f;
        const float floor = std::pow(10.0f, safe(p[4].value, -54.0f) / 20.0f);
        const int rearm = static_cast<int>(safe(p[5].value, 70.0f) * sr_ * 0.001f);
        const int pulseLength = std::max(1, static_cast<int>(sr_ * 0.005f));
        const auto* in = inputs.empty() ? nullptr : inputs[0];
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < outputs[0].frames(); ++f) {
            std::array<float, 3> magnitude{};
            for (std::size_t c = 0; c < channels; ++c) {
                const float original = in ? in->at(c, f) : 0.0f;
                const float finite = safe(original, 0.0f);
                outputs[0].at(c, f) = finite;
                const double x = std::clamp(static_cast<double>(finite), -16.0, 16.0);
                auto& s = state_[c];
                magnitude[0] += static_cast<float>(std::fabs(s[0].tick(coeff_[0], x)));
                const double mid = s[1].tick(coeff_[1], x);
                magnitude[1] += static_cast<float>(std::fabs(s[2].tick(coeff_[2], mid)));
                magnitude[2] += static_cast<float>(std::fabs(s[3].tick(coeff_[3], x)));
            }
            const bool detecting = warmup_ == 0;
            if (warmup_ > 0) --warmup_;
            for (std::size_t b = 0; b < 3; ++b) {
                const float x = magnitude[b] / static_cast<float>(channels);
                follow(level_[b], x, attack, release);
                follow(fast_[b], x, fastAttack_, fastRelease_);
                follow(slow_[b], x, slowAttack_, slowRelease_);
                if (cooldown_[b] > 0) --cooldown_[b];
                if (detecting && cooldown_[b] == 0 && fast_[b] > floor
                    && fast_[b] > slow_[b] * ratio + floor) {
                    pulse_[b] = pulseLength;
                    cooldown_[b] = rearm;
                }
                const float hit = pulse_[b] > 0 ? 1.0f : 0.0f;
                if (pulse_[b] > 0) --pulse_[b];
                for (std::size_t c = 0; c < channels; ++c) {
                    outputs[1 + b].at(c, f) = std::clamp(level_[b], 0.0f, 1.0f);
                    outputs[4 + b].at(c, f) = hit;
                }
            }
        }
    }

private:
    struct Coeff { double b0{}, b1{}, b2{}, a1{}, a2{}; };
    struct State {
        double z1{}, z2{};
        double tick(const Coeff& q, double x) noexcept {
            const double y = q.b0 * x + z1;
            z1 = q.b1 * x - q.a1 * y + z2;
            z2 = q.b2 * x - q.a2 * y;
            if (std::fabs(z1) < 1.0e-24) z1 = 0;
            if (std::fabs(z2) < 1.0e-24) z2 = 0;
            return y;
        }
    };
    static float safe(float v, float fallback) noexcept {
        return std::isfinite(v) ? v : fallback;
    }
    float coefficient(float seconds) const noexcept {
        return 1.0f - std::exp(-1.0f / (sr_ * seconds));
    }
    static void follow(float& value, float target, float attack, float release) noexcept {
        value += (target - value) * (target > value ? attack : release);
        if (value < 1.0e-20f) value = 0;
    }
    Coeff filter(float hz, bool highpass) const noexcept {
        constexpr double pi = 3.14159265358979323846;
        const double omega = 2.0 * pi * std::min(hz, 0.4f * sr_) / sr_;
        const double cs = std::cos(omega), alpha = std::sin(omega) / std::sqrt(2.0);
        const double a0 = 1.0 + alpha;
        const double b = (highpass ? 1.0 + cs : 1.0 - cs) * 0.5 / a0;
        return {b, (highpass ? -2.0 : 2.0) * b, b,
                -2.0 * cs / a0, (1.0 - alpha) / a0};
    }
    void updateCuts() noexcept {
        lowCut_ = safe(parameters()[0].value, 200.0f);
        highCut_ = safe(parameters()[1].value, 2000.0f);
        coeff_ = {filter(lowCut_, false), filter(lowCut_, true),
                  filter(highCut_, false), filter(highCut_, true)};
    }
    float sr_ = 48000, lowCut_ = 200, highCut_ = 2000;
    float fastAttack_{}, fastRelease_{}, slowAttack_{}, slowRelease_{};
    std::array<Coeff, 4> coeff_{};
    std::array<std::array<State, 4>, AudioBlock::maxChannels> state_{};
    std::array<float, 3> level_{}, fast_{}, slow_{};
    std::array<int, 3> pulse_{}, cooldown_{};
    int warmup_ = 0;
};

} // namespace rasgo::modular
