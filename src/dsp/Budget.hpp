#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// BUDGET (#86) — o orçamento de eventos (2026-11-11; dossiê
// `dossies/86_budget.md`; estudo da família §4.5: "4 gates entram, 4 saem;
// por ciclo há um teto de eventos que as vozes disputam (competição) ou
// cedem umas às outras (cooperação) — a ecologia do Aquorbium vista pelo
// lado do tempo"; "avance em todos eles"). Conceito do Aquorbium e do
// Antitotem (mesmo autor), não código.
//
// Quatro gates entram (G1–G4) e saem (O1–O4). A cada CICLO (um pulso em CLK,
// ou CICLO segundos sem cabo) há TETO eventos para todos:
//   0 COMPETE — quem chega primeiro leva; acabou o orçamento, ninguém mais
//               toca até o próximo ciclo;
//   1 COOPERA — cada voz ativa tem a sua parte (TETO / vozes que tocaram no
//               ciclo anterior); passar da parte só vale se as outras ainda
//               não gastaram a delas — quem já falou muito cede a vez.
// Um gate barrado não sai (o O fica calado nesse evento). Saídas: O1–O4,
// SALDO (o que resta do orçamento, 0–1) e FIM (um pulso quando acaba).
// Determinístico.
namespace rasgo::modular {

class Budget final : public Signal {
public:
    static constexpr int kVoices = 4;

    Budget() : Signal(
        {{"g1", PortKind::Control, "gate"},
         {"g2", PortKind::Control, "gate"},
         {"g3", PortKind::Control, "gate"},
         {"g4", PortKind::Control, "gate"},
         {"clock", PortKind::Control, "trig"}},
        {{"o1", PortKind::Control, "gate"},
         {"o2", PortKind::Control, "gate"},
         {"o3", PortKind::Control, "gate"},
         {"o4", PortKind::Control, "gate"},
         {"balance", PortKind::Control, ""},
         {"spent", PortKind::Control, "trig"}},
        {{"ceiling", 1.0f, 32.0f, 8.0f, ""},
         {"mode", 0.0f, 1.0f, 0.0f, ""},
         {"cycle", 0.25f, 16.0f, 2.0f, "s"}}) {}

    std::string type() const override { return "BUDGET"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "BUDGET", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "budget", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "TETO", "ceiling", 8.0f, 30.0f);
        p.add(Widget::Kind::Toggle, "COOP", "mode", 23.0f, 32.0f, 10.0f);
        p.add(Widget::Kind::Knob, "CICLO", "cycle", 38.0f, 30.0f);
        p.add(Widget::Kind::Jack, "G1", "in:g1", 7.0f, 72.0f);
        p.add(Widget::Kind::Jack, "G2", "in:g2", 19.0f, 72.0f);
        p.add(Widget::Kind::Jack, "G3", "in:g3", 31.0f, 72.0f);
        p.add(Widget::Kind::Jack, "G4", "in:g4", 43.0f, 72.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 88.0f);
        p.add(Widget::Kind::Jack, "O1", "out:o1", 7.0f, 104.0f);
        p.add(Widget::Kind::Jack, "O2", "out:o2", 19.0f, 104.0f);
        p.add(Widget::Kind::Jack, "O3", "out:o3", 31.0f, 104.0f);
        p.add(Widget::Kind::Jack, "O4", "out:o4", 43.0f, 104.0f);
        p.add(Widget::Kind::Jack, "SALDO", "out:balance", 7.0f, 120.0f);
        p.add(Widget::Kind::Jack, "FIM", "out:spent", 21.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        prevIn_.fill(0.0f);
        passing_.fill(false);
        used_.fill(0);
        activePrev_ = kVoices;
        spentTotal_ = 0;
        t_ = 0.0;
        prevClk_ = 0.0f;
        endLeft_ = 0;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* clk = inputs.size() > 4 ? inputs[4] : nullptr;
        const int ceiling = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("ceiling"), 8.0f), 1.0f, 32.0f)));
        const bool coop = parameterValue("mode") >= 0.5f;
        const double cycle = std::clamp(safe(parameterValue("cycle"), 2.0f), 0.25f, 16.0f);
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            bool newCycle = false;
            if (clk) {
                const float c = safe(clk->at(0, f), 0.0f);
                if (prevClk_ < 0.5f && c >= 0.5f) newCycle = true;
                prevClk_ = c;
            } else {
                t_ += 1.0 / sr_;
                if (t_ >= cycle) { t_ -= cycle; newCycle = true; }
            }
            if (newCycle) {
                int act = 0;
                for (const int u : used_) act += u > 0 ? 1 : 0;
                activePrev_ = std::max(1, act);
                used_.fill(0);
                spentTotal_ = 0;
            }
            float o[kVoices];
            for (int k = 0; k < kVoices; ++k) {
                const std::size_t ks = static_cast<std::size_t>(k);
                const AudioBlock* g = inputs.size() > ks ? inputs[ks] : nullptr;
                const float v = g ? safe(g->at(0, f), 0.0f) : 0.0f;
                if (prevIn_[ks] < 0.5f && v >= 0.5f) passing_[ks] = admit(k, ceiling, coop);
                if (v < 0.5f) passing_[ks] = false;
                prevIn_[ks] = v;
                o[k] = passing_[ks] ? v : 0.0f;
            }
            const float bal = static_cast<float>(ceiling - spentTotal_) / static_cast<float>(ceiling);
            const float fin = endLeft_ > 0 ? 1.0f : 0.0f;
            if (endLeft_ > 0) --endLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                for (int k = 0; k < kVoices; ++k) outputs[static_cast<std::size_t>(k)].at(c, f) = o[k];
                outputs[4].at(c, f) = std::max(0.0f, bal);
                outputs[5].at(c, f) = fin;
            }
            (void)pulse;
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    bool admit(const int k, const int ceiling, const bool coop) noexcept {
        if (spentTotal_ >= ceiling) return false;
        bool ok = true;
        if (coop) {
            const int share = std::max(1, ceiling / activePrev_);
            if (used_[static_cast<std::size_t>(k)] >= share) {
                // passar da parte: só se o que sobra cobre o que as outras
                // ainda não gastaram da delas
                int owed = 0;
                for (int j = 0; j < kVoices; ++j)
                    if (j != k) owed += std::max(0, share - used_[static_cast<std::size_t>(j)]);
                ok = ceiling - spentTotal_ > owed;
            }
        }
        if (!ok) return false;
        ++used_[static_cast<std::size_t>(k)];
        ++spentTotal_;
        if (spentTotal_ >= ceiling) endLeft_ = std::max(1, static_cast<int>(0.005f * sr_));
        return true;
    }

    float sr_ = 48000.0f;
    std::array<float, kVoices> prevIn_{};
    std::array<bool, kVoices> passing_{};
    std::array<int, kVoices> used_{};
    int activePrev_ = kVoices, spentTotal_ = 0;
    double t_ = 0.0;
    float prevClk_ = 0.0f;
    int endLeft_ = 0;
};

}  // namespace rasgo::modular
