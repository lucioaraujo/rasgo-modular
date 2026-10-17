#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <cmath>
#include <cstdint>

// PHRASE (#62): a bounded motif, played by external gestures/clock.
// Original implementation; no scale archive or cultural rule is embedded.
// Quantisation belongs to the downstream QUANTIZER, chosen by the musician.
namespace rasgo::modular {

class Phrase final : public Signal {
public:
    Phrase() : Signal(
        {{"pitch", PortKind::Audio, "v/oct"},
         {"clock", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"},
         {"stim", PortKind::Control, ""},
         {"return", PortKind::Control, "trig"}},
        {{"pitch", PortKind::Audio, "v/oct"},
         {"gate", PortKind::Control, "gate"},
         {"accent", PortKind::Control, ""},
         {"end", PortKind::Control, "trig"},
         {"phase", PortKind::Control, ""}},
        {{"length", 2, 16, 8, "steps"},
         {"span", 1, 36, 12, "st"},
         {"direction", -1, 1, 0, ""},
         {"variation", 0, 1, 0.25f, ""},
         {"space", 0, 1, 0.15f, ""},
         {"cadence", 0, 1, 0.8f, ""},
         {"gate_ms", 5, 400, 80, "ms"},
         {"seed", 1, 65535, 1, ""}}) {}

    std::string type() const override { return "PHRASE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 20;
        p.add(Widget::Kind::Label, "PHRASE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "phrase", "", 2.5f, 7.0f, 94.0f);
        const char* labels[] = {"LEN", "SPAN", "DIR", "VAR", "SPACE", "CAD", "GATE", "SEED"};
        const char* binds[] = {"length", "span", "direction", "variation", "space", "cadence", "gate_ms", "seed"};
        for (int i = 0; i < 8; ++i)
            p.add(Widget::Kind::Knob, labels[i], binds[i],
                  9.0f + static_cast<float>(i % 4) * 24.0f,
                  i < 4 ? 32.0f : 56.0f);
        const char* ins[] = {"pitch", "clock", "reset", "stim", "return"};
        const char* il[] = {"PITCH", "CLK", "RST", "STIM", "RET"};
        const char* outs[] = {"pitch", "gate", "accent", "end", "phase"};
        const char* ol[] = {"PITCH", "GATE", "ACC", "END", "PHASE"};
        for (int i = 0; i < 5; ++i) {
            const float x = 10.0f + static_cast<float>(i) * 20.0f;
            p.add(Widget::Kind::Jack, il[i], std::string("in:") + ins[i], x, 88.0f);
            p.add(Widget::Kind::Jack, ol[i], std::string("out:") + outs[i], x, 115.0f);
        }
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        reseed();
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const auto seed = static_cast<std::uint32_t>(value(7, 1));
        if (seed != seed_) reseed();
        const int length = std::clamp(static_cast<int>(std::lround(value(0, 8))), 2, 16);
        const float width = value(1, 12) / 12.0f;
        const float direction = value(2, 0);
        const float variation = value(3, 0.25f);
        const float space = value(4, 0.15f);
        const float cadence = value(5, 0.8f);
        const int gateLength = std::max(1, static_cast<int>(value(6, 80) * sr_ * 0.001f));
        const auto read = [&](std::size_t i, std::size_t f, float fallback) {
            if (i >= inputs.size() || !inputs[i]) return fallback;
            const float v = inputs[i]->at(0, f);
            return std::isfinite(v) ? v : fallback;
        };
        for (std::size_t f = 0; f < outputs[0].frames(); ++f) {
            const bool clock = read(1, f, 0) >= 0.5f;
            const bool reset = read(2, f, 0) >= 0.5f;
            const bool returning = read(4, f, 0) >= 0.5f;
            if (reset && !prevReset_) {
                next_ = 0; gateLeft_ = endLeft_ = 0; phase_ = accent_ = 0;
                held_ = anchor_ = 0; pendingReturn_ = false;
                rng_ = seed_;
            }
            if (returning && !prevReturn_) pendingReturn_ = true;
            if (clock && !prevClock_ && !reset) {
                if (next_ >= length) next_ = 0;
                const int step = next_;
                if (step == 0) anchor_ = std::clamp(read(0, f, 0), -6.0f, 6.0f);
                const float stimulus = std::clamp(read(3, f, 1), 0.0f, 1.0f);
                const bool ending = step == length - 1 || pendingReturn_;
                phase_ = static_cast<float>(step) / static_cast<float>(length - 1);
                // Motif remains recognisable: bounded contour + remembered
                // seeded degrees. Variation changes interpretation, not memory.
                float offset = motif_[static_cast<std::size_t>(step)] * (1 - std::fabs(direction))
                             + direction * phase_;
                if (step == 0) offset = 0;
                const float perturb = (random() - 0.5f) * variation * 0.5f
                                    * (0.5f + stimulus * 0.5f);
                offset = std::clamp(offset + (step == 0 ? 0 : perturb), -1.0f, 1.0f);
                const bool resolve = pendingReturn_ || (ending && random() < cadence);
                if (resolve) offset = 0;
                const bool rest = step > 0 && !ending && random() < space * (1 - stimulus * 0.5f);
                if (!rest) {
                    held_ = std::clamp(anchor_ + offset * width, -8.0f, 8.0f);
                    accent_ = std::clamp((step == 0 ? 0.8f : ending ? 0.65f : 0.45f)
                                        + 0.2f * stimulus, 0.0f, 1.0f);
                    gateLeft_ = gateLength;
                } else { gateLeft_ = 0; accent_ = 0; }
                if (ending) {
                    endLeft_ = std::max(1, static_cast<int>(sr_ * 0.005f));
                    next_ = 0; pendingReturn_ = false;
                } else ++next_;
            }
            prevClock_ = clock; prevReset_ = reset; prevReturn_ = returning;
            // Separate consecutive gates, including faster clocks than GATE.
            // One low sample on a new event gives downstream envelopes an edge.
            const bool eventGap = clock && !previousClockSample_;
            previousClockSample_ = clock;
            const float gate = gateLeft_ > 0 && !eventGap ? 1.0f : 0.0f;
            const float end = endLeft_ > 0 ? 1.0f : 0.0f;
            if (gateLeft_ > 0) --gateLeft_;
            if (endLeft_ > 0) --endLeft_;
            for (std::size_t c = 0; c < outputs[0].channels(); ++c) {
                outputs[0].at(c, f) = held_;
                outputs[1].at(c, f) = gate;
                outputs[2].at(c, f) = accent_;
                outputs[3].at(c, f) = end;
                outputs[4].at(c, f) = phase_;
            }
        }
    }

private:
    float value(std::size_t i, float fallback) const noexcept {
        const float v = parameters()[i].value;
        return std::isfinite(v) ? v : fallback;
    }
    float random() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
        return static_cast<float>(rng_ >> 8) / 16777216.0f;
    }
    void reseed() noexcept {
        seed_ = static_cast<std::uint32_t>(value(7, 1));
        rng_ = std::max<std::uint32_t>(seed_, 1);
        float position = 0;
        for (auto& m : motif_) {
            position = std::clamp(position + (random() - 0.5f) * 0.8f, -0.8f, 0.8f);
            m = std::round(position * 12.0f) / 12.0f;
        }
        rng_ = seed_;
        next_ = gateLeft_ = endLeft_ = 0;
        held_ = anchor_ = accent_ = phase_ = 0;
        prevClock_ = prevReset_ = prevReturn_ = previousClockSample_ = pendingReturn_ = false;
    }
    std::array<float, 16> motif_{};
    float sr_ = 48000, held_{}, anchor_{}, accent_{}, phase_{};
    std::uint32_t seed_ = 1, rng_ = 1;
    int next_{}, gateLeft_{}, endLeft_{};
    bool prevClock_{}, prevReset_{}, prevReturn_{}, previousClockSample_{}, pendingReturn_{};
};

} // namespace rasgo::modular
