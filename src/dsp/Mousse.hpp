#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// MOUSSE (#69) — deformação de um material mole (2026-11-09, ideia do autor:
// "deformações (tipo mousse)").
//
// Uma espuma viscoelástica: quando se aperta, ela afunda — uma parte na hora
// (a mola), outra devagar (o fluido que escorre) — e, solta, volta sem pressa.
// Modelo de livro-texto da viscoelasticidade, o "sólido linear padrão"
// (Zener): uma mola em paralelo com um ramo mola+amortecedor. Aqui:
//   alvo  = força / rigidez
//   rápido: segue o alvo em ~5 ms           (a parte elástica)
//   lento : segue o alvo em τ ao afundar e em τ·RET ao voltar (o fluido)
//   fundo = (1 − SLOW)·rápido + SLOW·lento
// Nenhuma coleção do Cardinal faz isto; ideia e implementação próprias.
//
// A força vem de golpes (HIT: cada gatilho é um empurrão que decai em ~80 ms,
// FORCE dá o tamanho) e de pressão contínua (PRESS, CV 0–1).
//
// Desvio Rasgo:
//  (a) FATIGUE — cada golpe amolece um pouco a espuma, que endurece de novo
//      devagar: golpes seguidos afundam cada vez mais;
//  (b) modo autônomo — sem HIT ligado, a espuma é tocada sozinha em RATE, com
//      a força variando por acaso SEMEADO: o MOUSSE se move ao carregar.
//
// Saídas: DEPTH (0–1, o quanto está afundada), SPEED (0–1, a velocidade da
// deformação), REST (gate: a espuma voltou ao repouso). process() não aloca.
namespace rasgo::modular {

class Mousse final : public Signal {
public:
    Mousse() : Signal(
        {{"hit", PortKind::Control, "trig"},
         {"press", PortKind::Control, ""}},
        {{"depth", PortKind::Control, ""},
         {"speed", PortKind::Control, ""},
         {"rest", PortKind::Control, "gate"}},
        {{"stiff", 0.0f, 1.0f, 0.5f, ""},
         {"visc", 0.05f, 8.0f, 1.5f, "s"},
         {"slow", 0.0f, 1.0f, 0.7f, ""},
         {"recover", 1.0f, 8.0f, 3.0f, ""},
         {"force", 0.0f, 1.0f, 0.6f, ""},
         {"fatigue", 0.0f, 1.0f, 0.2f, ""},
         {"rate", 0.05f, 4.0f, 0.5f, "Hz"}}) {}

    std::string type() const override { return "MOUSSE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "MOUSSE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "mousse", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "STIFF", "stiff", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "VISC", "visc", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "SLOW", "slow", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RET", "recover", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "FORCE", "force", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "FATG", "fatigue", 36.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 5.0f, 70.0f);
        p.add(Widget::Kind::Jack, "HIT", "in:hit", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PRESS", "in:press", 21.0f, 96.0f);
        p.add(Widget::Kind::Jack, "DEPTH", "out:depth", 8.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SPEED", "out:speed", 24.0f, 116.0f);
        p.add(Widget::Kind::Jack, "REST", "out:rest", 39.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x30055EF0A40069ULL;
        fast_ = slow_ = depth_ = prevDepth_ = 0.0f;
        hitForce_ = 0.0f;
        soft_ = 0.0f;
        prevHit_ = 0.0f;
        phase_ = 0.75;   // o primeiro toque autônomo logo depois de abrir
    }

    // A rigidez efetiva (com a fadiga) — exposta para os testes.
    float softness() const noexcept { return soft_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 3) return;
        const AudioBlock* hit = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* press = inputs.size() > 1 ? inputs[1] : nullptr;

        const float stiff = std::clamp(safe(parameterValue("stiff"), 0.5f), 0.0f, 1.0f);
        const float visc = std::clamp(safe(parameterValue("visc"), 1.5f), 0.05f, 8.0f);
        const float slowShare = std::clamp(safe(parameterValue("slow"), 0.7f), 0.0f, 1.0f);
        const float recover = std::clamp(safe(parameterValue("recover"), 3.0f), 1.0f, 8.0f);
        const float force = std::clamp(safe(parameterValue("force"), 0.6f), 0.0f, 1.0f);
        const float fatigue = std::clamp(safe(parameterValue("fatigue"), 0.2f), 0.0f, 1.0f);
        const float rate = std::clamp(safe(parameterValue("rate"), 0.5f), 0.05f, 4.0f);

        const float kFast = coef(0.005f);
        const float kIn = coef(visc * 0.25f);
        const float kOut = coef(visc * recover);
        const float kHit = std::exp(-1.0f / (0.08f * sr_));
        const float kHeal = coef(6.0f);          // a fadiga passa em ~6 s
        const double dp = static_cast<double>(rate) / sr_;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            bool strike = false;
            float strikeSize = force;
            if (hit) {
                const float h = safe(hit->at(0, f), 0.0f);
                if (prevHit_ < 0.5f && h >= 0.5f) strike = true;
                prevHit_ = h;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) {
                    phase_ -= std::floor(phase_);
                    strike = true;
                    strikeSize = force * (0.4f + 0.6f * unit());
                }
            }
            if (strike) {
                hitForce_ = std::max(hitForce_, strikeSize);
                soft_ = std::min(0.8f, soft_ + 0.25f * fatigue);
            }
            hitForce_ *= kHit;
            soft_ += (0.0f - soft_) * kHeal;

            const float p = press ? std::clamp(safe(press->at(0, f), 0.0f), 0.0f, 1.0f) : 0.0f;
            const float k = (0.25f + 1.75f * stiff) * (1.0f - soft_);   // rigidez
            const float target = std::clamp((hitForce_ * 4.0f + p) / std::max(0.05f, k), 0.0f, 1.0f);

            fast_ += (target - fast_) * kFast;
            slow_ += (target - slow_) * (target > slow_ ? kIn : kOut);
            depth_ = std::clamp((1.0f - slowShare) * fast_ + slowShare * slow_, 0.0f, 1.0f);
            const float speed = std::clamp(std::fabs(depth_ - prevDepth_) * sr_ * 0.05f, 0.0f, 1.0f);
            prevDepth_ = depth_;
            const float rest = depth_ < 0.02f ? 1.0f : 0.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = depth_;
                outputs[1].at(c, f) = speed;
                outputs[2].at(c, f) = rest;
            }
        }
    }

private:
    float coef(float seconds) const noexcept { return 1.0f - std::exp(-1.0f / (std::max(1e-4f, seconds) * sr_)); }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x30055EF0A40069ULL;
    float fast_ = 0.0f, slow_ = 0.0f, depth_ = 0.0f, prevDepth_ = 0.0f;
    float hitForce_ = 0.0f, soft_ = 0.0f, prevHit_ = 0.0f;
    double phase_ = 0.75;
};

} // namespace rasgo::modular
