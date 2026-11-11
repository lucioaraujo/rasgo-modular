#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// POLYGON (#82) — polígonos inscritos num ciclo (2026-11-11; dossiê
// `dossies/82_polygon.md`; estudo da família §4.3: "camadas N:M:… com fases,
// gates por camada, gate de coincidência e EOS no realinhamento — a geometria
// rítmica do Batruc como módulo"; "avance em todos eles").
//
// Um CICLO comum e até 4 camadas. A camada k é um polígono de Nₖ vértices
// inscrito no ciclo (0 = desligada): Nₖ pulsos igualmente espaçados por
// ciclo, girados pela fase φₖ (até um lado do polígono). Três contra dois,
// cinco contra quatro… — o polímetro como figura. COINC pulsa quando dois ou
// mais vértices caem juntos (±1,5 ms); EOS no fim de cada ciclo, quando todos
// realinham; FASE é a posição no ciclo (0..1, CV).
//
// O ciclo dura CICLO segundos; com cabo em CLK, dura PASSOS pulsos do relógio
// (a fase anda entre os pulsos pela estimativa do período, como no CLOCK). RST
// volta ao começo. Determinístico, sem alocação.
namespace rasgo::modular {

class Polygon final : public Signal {
public:
    static constexpr int kLayers = 4;

    Polygon() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"}},
        {{"g1", PortKind::Control, "trig"},
         {"g2", PortKind::Control, "trig"},
         {"g3", PortKind::Control, "trig"},
         {"g4", PortKind::Control, "trig"},
         {"coinc", PortKind::Control, "trig"},
         {"eos", PortKind::Control, "trig"},
         {"phase", PortKind::Control, ""}},
        {{"n1", 0.0f, 16.0f, 3.0f, ""},
         {"n2", 0.0f, 16.0f, 4.0f, ""},
         {"n3", 0.0f, 16.0f, 0.0f, ""},
         {"n4", 0.0f, 16.0f, 0.0f, ""},
         {"phi1", 0.0f, 1.0f, 0.0f, ""},
         {"phi2", 0.0f, 1.0f, 0.0f, ""},
         {"phi3", 0.0f, 1.0f, 0.0f, ""},
         {"phi4", 0.0f, 1.0f, 0.0f, ""},
         {"cycle", 0.25f, 32.0f, 2.0f, "s"},
         {"steps", 1.0f, 64.0f, 16.0f, ""}}) {}

    std::string type() const override { return "POLYGON"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "POLYGON", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "polygon", "", 2.5f, 6.0f, 66.1f);
        p.add(Widget::Kind::Knob, "N1", "n1", 9.0f, 28.0f);
        p.add(Widget::Kind::Knob, "N2", "n2", 25.0f, 28.0f);
        p.add(Widget::Kind::Knob, "N3", "n3", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "N4", "n4", 57.0f, 28.0f);
        p.add(Widget::Kind::Knob, "F1", "phi1", 9.0f, 46.0f);
        p.add(Widget::Kind::Knob, "F2", "phi2", 25.0f, 46.0f);
        p.add(Widget::Kind::Knob, "F3", "phi3", 41.0f, 46.0f);
        p.add(Widget::Kind::Knob, "F4", "phi4", 57.0f, 46.0f);
        p.add(Widget::Kind::Knob, "CICLO", "cycle", 9.0f, 64.0f);
        p.add(Widget::Kind::Knob, "PASSOS", "steps", 25.0f, 64.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 88.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 88.0f);
        p.add(Widget::Kind::Jack, "G1", "out:g1", 7.0f, 104.0f);
        p.add(Widget::Kind::Jack, "G2", "out:g2", 19.0f, 104.0f);
        p.add(Widget::Kind::Jack, "G3", "out:g3", 31.0f, 104.0f);
        p.add(Widget::Kind::Jack, "G4", "out:g4", 43.0f, 104.0f);
        p.add(Widget::Kind::Jack, "COINC", "out:coinc", 7.0f, 120.0f);
        p.add(Widget::Kind::Jack, "EOS", "out:eos", 21.0f, 120.0f);
        p.add(Widget::Kind::Jack, "FASE", "out:phase", 35.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        phase_ = 0.0;
        lastIdx_.fill(-1);
        left_.fill(0);
        lastFire_.fill(-1000000);
        coincLeft_ = eosLeft_ = 0;
        now_ = 0;
        prevClk_ = prevReset_ = 0.0f;
        clkSeen_ = false;
        clkSamples_ = 0.0;
        clkPeriod_ = 0.25 * sr_;
        clkCount_ = 0;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 7) return;
        const AudioBlock* clk = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        std::array<int, kLayers> nn{};
        std::array<double, kLayers> phi{};
        static const char* kN[kLayers] = {"n1", "n2", "n3", "n4"};
        static const char* kP[kLayers] = {"phi1", "phi2", "phi3", "phi4"};
        for (int k = 0; k < kLayers; ++k) {
            nn[static_cast<std::size_t>(k)] = static_cast<int>(std::lround(
                std::clamp(safe(parameterValue(kN[k]), 0.0f), 0.0f, 16.0f)));
            phi[static_cast<std::size_t>(k)] = std::clamp(safe(parameterValue(kP[k]), 0.0f), 0.0f, 1.0f);
        }
        const double cycle = std::clamp(safe(parameterValue("cycle"), 2.0f), 0.25f, 32.0f);
        const int steps = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("steps"), 16.0f), 1.0f, 64.0f)));
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));
        const long window = std::max(1L, static_cast<long>(0.0015f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            ++now_;
            bool wrapped = false;
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) {
                    phase_ = 0.0;
                    clkCount_ = 0;
                    lastIdx_.fill(-1);
                }
                prevReset_ = r;
            }
            double next = phase_;
            if (clk) {
                const float c = safe(clk->at(0, f), 0.0f);
                clkSamples_ += 1.0;
                if (prevClk_ < 0.5f && c >= 0.5f) {
                    if (clkSeen_ && clkSamples_ > 1.0) clkPeriod_ = clkSamples_;
                    clkSamples_ = 0.0;
                    clkSeen_ = true;
                    clkCount_ = (clkCount_ + 1) % steps;
                    if (clkCount_ == 0) wrapped = true;
                }
                prevClk_ = c;
                const double within = std::min(0.999, clkSamples_ / std::max(1.0, clkPeriod_));
                next = (static_cast<double>(clkCount_) + within) / static_cast<double>(steps);
            } else {
                next = phase_ + 1.0 / (cycle * sr_);
                if (next >= 1.0) { next -= 1.0; wrapped = true; }
            }
            phase_ = next;
            if (wrapped) eosLeft_ = pulse;

            int fired = 0;
            for (int k = 0; k < kLayers; ++k) {
                const std::size_t ks = static_cast<std::size_t>(k);
                const int n = nn[ks];
                if (n <= 0) { lastIdx_[ks] = -1; continue; }
                // o vértice atual da camada (girada por φ, até um lado)
                double u = phase_ * n - phi[ks];
                u -= std::floor(u / n) * n;
                const int idx = static_cast<int>(std::floor(u));
                if (idx != lastIdx_[ks]) {
                    // na largada (ou depois de um RST) só dispara se a
                    // camada estiver mesmo em cima de um vértice
                    if (lastIdx_[ks] >= 0 || (u - std::floor(u)) < 1.0e-3) {
                        left_[ks] = pulse;
                        lastFire_[ks] = now_;
                        ++fired;
                    }
                    lastIdx_[ks] = idx;
                }
            }
            if (fired > 0) {
                int near = 0;
                for (int k = 0; k < kLayers; ++k)
                    if (nn[static_cast<std::size_t>(k)] > 0
                        && now_ - lastFire_[static_cast<std::size_t>(k)] <= window) ++near;
                if (near >= 2) coincLeft_ = pulse;
            }

            float g[kLayers];
            for (int k = 0; k < kLayers; ++k) {
                g[k] = left_[static_cast<std::size_t>(k)] > 0 ? 1.0f : 0.0f;
                if (left_[static_cast<std::size_t>(k)] > 0) --left_[static_cast<std::size_t>(k)];
            }
            const float co = coincLeft_ > 0 ? 1.0f : 0.0f;
            const float eo = eosLeft_ > 0 ? 1.0f : 0.0f;
            if (coincLeft_ > 0) --coincLeft_;
            if (eosLeft_ > 0) --eosLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                for (int k = 0; k < kLayers; ++k) outputs[static_cast<std::size_t>(k)].at(c, f) = g[k];
                outputs[4].at(c, f) = co;
                outputs[5].at(c, f) = eo;
                outputs[6].at(c, f) = static_cast<float>(phase_);
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    float sr_ = 48000.0f;
    double phase_ = 0.0;
    std::array<int, kLayers> lastIdx_{};
    std::array<int, kLayers> left_{};
    std::array<long, kLayers> lastFire_{};
    int coincLeft_ = 0, eosLeft_ = 0;
    long now_ = 0;
    float prevClk_ = 0.0f, prevReset_ = 0.0f;
    bool clkSeen_ = false;
    double clkSamples_ = 0.0, clkPeriod_ = 12000.0;
    int clkCount_ = 0;
};

}  // namespace rasgo::modular
