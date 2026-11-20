#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// SWARM (#98) — o cardume (leitura da família, 9/11/2026, aprovada pelo
// autor: "aprovo todos, continue"; dossiê `dossies/98_swarm.md`).
//
// Seis relógios, cada um com o seu andamento natural (RATE ± SPREAD), que se
// ouvem: o modelo de Kuramoto (1975), dθᵢ/dt = ωᵢ + (K/N)·Σ sen(θⱼ − θᵢ). Um
// só controle, COHES, vai do cardume coeso — todos no mesmo pulso, a
// HOMORRITMIA — às vozes soltas — cada uma no seu tempo, a HETEROFONIA — e,
// no meio, o cardume que se forma e se desfaz. É o coro de fases do
// Aquorbium (`ChorusPhase`, mesmo autor: rãs que se sincronizam) e o cardume
// coeso/disperso de Blackwell e Bentley (2002, swarm music).
//
// Saídas: G1–G4 (o pulso de quatro dos peixes), ALL (o pulso do cardume
// inteiro: a fase média) e SYNC (o quanto estão juntos, 0–1: o parâmetro de
// ordem r de Kuramoto).
//
// Desvio Rasgo:
//  (a) SCAR — o susto: um gatilho espalha as fases ao acaso (o predador
//      passou), e o cardume se reagrupa no tempo que COHES permite;
//  (b) LEAD — um relógio externo é o líder: com cabo, o cardume inteiro
//      ouve também esse pulso e é puxado por ele.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Swarm final : public Signal {
public:
    static constexpr int kN = 6;

    Swarm() : Signal(
        {{"scare", PortKind::Control, "trig"},
         {"lead", PortKind::Control, "trig"},
         {"cohesion_cv", PortKind::Control, ""}},
        {{"g1", PortKind::Control, "gate"},
         {"g2", PortKind::Control, "gate"},
         {"g3", PortKind::Control, "gate"},
         {"g4", PortKind::Control, "gate"},
         {"all", PortKind::Control, "gate"},
         {"sync", PortKind::Control, ""}},
        {{"rate", 0.2f, 16.0f, 2.0f, "Hz"},
         {"spread", 0.0f, 0.5f, 0.12f, ""},
         {"cohesion", 0.0f, 1.0f, 0.5f, ""},
         {"gate_len", 0.05f, 0.9f, 0.3f, ""}}) {}

    std::string type() const override { return "SWARM"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "SWARM", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "swarm", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "COHES", "cohesion", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 5.0f, 50.0f);
        p.add(Widget::Kind::Jack, "SCAR", "in:scare", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "LEAD", "in:lead", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "COH", "in:cohesion_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SYNC", "out:sync", 44.0f, 96.0f);
        p.add(Widget::Kind::Jack, "G1", "out:g1", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "G2", "out:g2", 15.5f, 116.0f);
        p.add(Widget::Kind::Jack, "G3", "out:g3", 25.0f, 116.0f);
        p.add(Widget::Kind::Jack, "G4", "out:g4", 34.5f, 116.0f);
        p.add(Widget::Kind::Jack, "ALL", "out:all", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x5CA77E12ULL;
        // os andamentos naturais: desvios fixos e espalhados (−1..1)
        static constexpr std::array<float, kN> kDev{-1.0f, 0.62f, -0.31f, 0.95f, 0.08f, -0.7f};
        dev_ = kDev;
        scatter();
        meanPhase_ = 0.0f;
        prevScare_ = prevLead_ = 0.0f;
        leadPhase_ = 0.0f; leadPeriod_ = 0.0f; sinceLead_ = 0.0f; leadActive_ = false;
        gates_.fill(0);
        allLeft_ = 0;
        r_ = 0.0f;
    }

    float phase(int i) const noexcept { return th_[static_cast<std::size_t>(std::clamp(i, 0, kN - 1))]; }
    float order() const noexcept { return r_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* scare = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* lead = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* ccv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float rate = std::clamp(safe(parameterValue("rate"), 2.0f), 0.2f, 16.0f);
        const float spread = std::clamp(safe(parameterValue("spread"), 0.12f), 0.0f, 0.5f);
        const float cohK = std::clamp(safe(parameterValue("cohesion"), 0.5f), 0.0f, 1.0f);
        const float gl = std::clamp(safe(parameterValue("gate_len"), 0.3f), 0.05f, 0.9f);
        const float dt = 1.0f / sr_;
        const float tw = 6.2831853f;
        const int gateLen = std::max(1, static_cast<int>(gl * sr_ / rate));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (scare) {
                const float s = safe(scare->at(0, f), 0.0f);
                if (prevScare_ < 0.5f && s >= 0.5f) scatter();
                prevScare_ = s;
            }
            // o líder: um relógio externo vira mais uma voz que todos ouvem
            sinceLead_ += 1.0f;
            if (lead) {
                const float l = safe(lead->at(0, f), 0.0f);
                if (prevLead_ < 0.5f && l >= 0.5f) {
                    if (sinceLead_ > 16.0f) leadPeriod_ = sinceLead_;
                    sinceLead_ = 0.0f;
                    leadPhase_ = 0.0f;
                    leadActive_ = true;
                }
                prevLead_ = l;
            } else {
                leadActive_ = false;
            }
            if (leadActive_ && leadPeriod_ > 0.0f) {
                leadPhase_ += 1.0f / leadPeriod_;
                if (leadPhase_ >= 1.0f) leadPhase_ -= 1.0f;
            }
            float coh = cohK;
            if (ccv) coh = std::clamp(coh + safe(ccv->at(0, f), 0.0f), 0.0f, 1.0f);
            // K acima do crítico do espalhamento (Kc ≈ largura da distribuição)
            const float spreadW = tw * rate * std::max(spread, 0.02f);
            const float K = coh * coh * 4.0f * spreadW;
            // o campo médio (Kuramoto): r·e^{iψ} = (1/N) Σ e^{iθ}
            float cx = 0.0f, cy = 0.0f;
            for (int i = 0; i < kN; ++i) {
                cx += std::cos(tw * th_[static_cast<std::size_t>(i)]);
                cy += std::sin(tw * th_[static_cast<std::size_t>(i)]);
            }
            cx /= static_cast<float>(kN); cy /= static_cast<float>(kN);
            r_ = std::sqrt(cx * cx + cy * cy);
            const float psi = std::atan2(cy, cx);
            for (int i = 0; i < kN; ++i) {
                const std::size_t ii = static_cast<std::size_t>(i);
                const float w = tw * rate * (1.0f + spread * dev_[ii]);
                const float th = tw * th_[ii];
                float dth = w + K * r_ * std::sin(psi - th);
                if (leadActive_) dth += K * std::sin(tw * leadPhase_ - th);
                th_[ii] += dth * dt / tw;
                if (th_[ii] >= 1.0f) {
                    th_[ii] -= std::floor(th_[ii]);
                    if (i < 4) gates_[ii] = gateLen;
                } else if (th_[ii] < 0.0f) {
                    th_[ii] -= std::floor(th_[ii]);
                }
            }
            // o pulso do cardume: a volta da fase média
            const float mp = (psi < 0.0f ? psi + tw : psi) / tw;
            if (mp < meanPhase_ - 0.5f && r_ > 0.2f) allLeft_ = gateLen;
            meanPhase_ = mp;

            for (std::size_t c = 0; c < channels; ++c) {
                for (std::size_t g = 0; g < 4; ++g) outputs[g].at(c, f) = gates_[g] > 0 ? 1.0f : 0.0f;
                outputs[4].at(c, f) = allLeft_ > 0 ? 1.0f : 0.0f;
                outputs[5].at(c, f) = r_;
            }
            for (auto& g : gates_) if (g > 0) --g;
            if (allLeft_ > 0) --allLeft_;
        }
    }

private:
    void scatter() noexcept {
        for (auto& t : th_) t = unit();
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x5CA77E12ULL;
    std::array<float, kN> th_{}, dev_{};
    std::array<int, 4> gates_{};
    float meanPhase_ = 0.0f, prevScare_ = 0.0f, prevLead_ = 0.0f;
    float leadPhase_ = 0.0f, leadPeriod_ = 0.0f, sinceLead_ = 0.0f;
    bool leadActive_ = false;
    int allLeft_ = 0;
    float r_ = 0.0f;
};

} // namespace rasgo::modular
