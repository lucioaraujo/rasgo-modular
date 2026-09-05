#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// LPG — low-pass gate a vactrol (Módulo 25)
// ============================================================================
//
// `FILTER`+`VCA`+`ENVELOPE` aproximam, mas o LAG DE VACTROL (resposta
// assimétrica com cauda longa da fotocélula) é o que faz o timbre plucky
// da costa oeste — um golpe abre o filtro E a amplitude juntos, e a queda
// tem a curva natural de um ressoador percutido.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/25_lpg.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Buchla 292 / série 200 LPG (um elemento controla filtro + VCA);
//   - Make Noise Optomix (crossfade contínuo filtro↔VCA);
//   - modelo de fotocélula (LDR): sobe rápido, desce devagar e não-linear;
//   - SVF / 1-polo em cascata (filtro de 2 polos barato).
//
// Antialias: o filtro do LPG é LINEAR (sem saturação no laço) — não gera
// alias como o `WASP`/`SHAPE`. O risco é zipper na modulação rápida do
// corte (golpe/`cv` bruscos); um 1-polo de ~0,5 ms suaviza o `fc` alvo.
// Não precisa de oversampling.

namespace rasgo::modular {

class Lpg final : public Signal {
public:
    Lpg()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"strike", PortKind::Control, "trig"},
               {"cv", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""}},
              {{"mode", 0.0f, 1.0f, 0.5f, ""},
               {"response", 0.0f, 1.0f, 0.4f, ""},
               {"offset", 0.0f, 1.0f, 0.0f, ""},
               {"resonance", 0.0f, 1.0f, 0.2f, ""},
               {"bounce", 0.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "LPG"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "LPG", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "env", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 8.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RESP", "response", 28.0f, 28.0f);
        p.add(Widget::Kind::Knob, "OFST", "offset", 8.0f, 48.0f);
        p.add(Widget::Kind::Knob, "RESO", "resonance", 28.0f, 48.0f);
        p.add(Widget::Kind::Knob, "BNCE", "bounce", 8.0f, 68.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 28.0f, 68.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 6.0f, 100.0f);
        p.add(Widget::Kind::Jack, "STRK", "in:strike", 18.0f, 100.0f);
        p.add(Widget::Kind::Jack, "CV", "in:cv", 30.0f, 100.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 42.0f, 100.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        env_ = 0.0f;
        prevStrike_ = false;
        bouncePhase_ = 2.0f;   // >1 = inativo
        bounceWin_ = std::max(1.0f, 0.032f * sr_);   // janela ~32 ms
        lp1_ = lp2_ = 0.0f;
        smoothFc_ = 20.0f;
        fcCoef_ = 1.0f - std::exp(-1.0f / (0.0005f * sr_));  // ~0,5 ms
        driftCur_ = driftTgt_ = 0.0f;
        driftCounter_ = 0;
        driftInterval_ = static_cast<std::uint32_t>(std::max(1.0f, sr_ / 8.0f));
        rng_ = 0xA0761D6478BD642FULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const float mode = clamp01(parameterValue("mode"));
        const float response = clamp01(parameterValue("response"));
        const float offset = clamp01(parameterValue("offset"));
        const float reso = clamp01(parameterValue("resonance"));
        const float bounce = clamp01(parameterValue("bounce"));
        const float drift = clamp01(parameterValue("drift"));

        // coefs do vactrol — por bloco (um exp cada)
        const float atkCoef =
            1.0f - std::exp(-1.0f / (0.002f * sr_));
        const float relTime = response * response * 2.5f * sr_ + 0.03f * sr_;
        const float relBase = 1.0f - std::exp(-1.0f / relTime);

        // `mode`: 0 = só filtro, 1 = só VCA, 0.5 = OS DOIS totalmente
        // ativos (o LPG clássico — fecha de vez quando o env cai a 0).
        const float fResp = 1.0f - std::max(0.0f, mode - 0.5f) * 2.0f;
        const float aResp = std::min(1.0f, mode * 2.0f);
        const float driftAmp = 0.03f * drift;

        const AudioBlock* in = inputs[0];
        const AudioBlock* strike = inputs[1];
        const AudioBlock* cv = inputs[2];

        for (std::size_t f = 0; f < frames; ++f) {
            if (drift > 0.0f && ++driftCounter_ >= driftInterval_) {
                driftCounter_ = 0;
                driftTgt_ = noise() * driftAmp;
            }
            driftCur_ += (driftTgt_ - driftCur_) * 0.002f;

            const bool strikeHi = strike != nullptr && strike->at(0, f) >= 0.5f;
            const float g0 = strikeHi ? 1.0f : 0.0f;
            const float cvv = cv != nullptr ? cv->at(0, f) : 0.0f;
            const float target = clamp01(std::max(g0, offset) + cvv);
            // BOUNCE: na borda ↑ de `strike`, dispara uma janela de
            // overshoot (senoide amortecida). `bounce = 0` → `bump = 0`.
            if (bounce > 0.0f && strikeHi && !prevStrike_) bouncePhase_ = 0.0f;
            prevStrike_ = strikeHi;

            // cauda: freia perto de 0 (a "memória" do LDR)
            const float relCoef = relBase * (0.15f + 0.85f * env_)
                * (1.0f + driftCur_);
            const float coef = target > env_ ? atkCoef : relCoef;
            env_ += (target - env_) * coef;
            env_ = clamp01(env_);

            // BOUNCE: senoide amortecida sobre a janela pós-golpe, somada
            // ao env — o vactrol conduz ACIMA do regime por um instante.
            float envEff = env_;
            if (bouncePhase_ < 1.0f) {
                const float ph = bouncePhase_;
                const float bump = bounce * 0.4f * env_
                    * std::sin(3.14159265f * ph * 2.6f) * (1.0f - ph);
                envEff = clampf(env_ + bump, 0.0f, 1.35f);
                bouncePhase_ += 1.0f / bounceWin_;
            }

            const float fEnv = 1.0f - fResp * (1.0f - envEff);
            const float aEnv = 1.0f - aResp * (1.0f - envEff);
            // suaviza o corte alvo (1 polo ~0,5 ms) — mata o zipper de
            // modulação rápida sem borrar o "pluck"
            const float fcTarget = 20.0f + fEnv * fEnv * 12000.0f;
            smoothFc_ += (fcTarget - smoothFc_) * fcCoef_;
            const float fc = smoothFc_;

            const float gc = fc / (fc + sr_ * 0.31831f);
            const float fb = reso * 2.4f * (1.0f - gc * 0.4f);
            const float x = (in != nullptr ? in->at(0, f) : 0.0f)
                - fb * (lp1_ - lp2_);
            lp1_ += gc * (x - lp1_);
            lp2_ += gc * (lp1_ - lp2_);

            const float y = lp2_ * aEnv;
            for (std::size_t c = 0; c < channels; ++c)
                out.at(c, f) = y;
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
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float sr_ = 48000.0f;
    float env_ = 0.0f;
    bool prevStrike_ = false;
    float bouncePhase_ = 2.0f, bounceWin_ = 1536.0f;
    float lp1_ = 0.0f, lp2_ = 0.0f;
    float smoothFc_ = 20.0f;
    float fcCoef_ = 0.0f;
    float driftCur_ = 0.0f, driftTgt_ = 0.0f;
    std::uint32_t driftCounter_ = 0;
    std::uint32_t driftInterval_ = 6000;
    std::uint64_t rng_ = 0xA0761D6478BD642FULL;
};

}  // namespace rasgo::modular
