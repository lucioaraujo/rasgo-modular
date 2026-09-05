#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// DRIFT — campo de deriva (Módulo 27)
// ============================================================================
//
// "Modulação transforma o presente; deriva transforma o que o instrumento
// considera seu estado normal." Fonte de CV que se move em escala de
// MINUTOS, com memória (momentum) e correlação (as 4 saídas contam a mesma
// história de ângulos diferentes) — pra o patch se DESENVOLVER sozinho.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/27_drift.md`.
//
// Fontes ESTUDADAS (código do autor, conceito):
//   - ANTITOTEM deriveFromMemory / brief CRI-DRF-001: momentum acumula e
//     retroalimenta; cadência por loop; memória;
//   - AQUORBIUM BiomaBrain::correlatedValues: um LFSR compartilhado, cada
//     saída = soma ponderada DIFERENTE dos mesmos bits + LFO próprio.

namespace rasgo::modular {

class Drift final : public Signal {
public:
    Drift()
        : Signal(
              {{"advance", PortKind::Control, "trig"},
               {"rate_mod", PortKind::Control, ""}},
              {{"a", PortKind::Control, ""},
               {"b", PortKind::Control, ""},
               {"c", PortKind::Control, ""},
               {"d", PortKind::Control, ""},
               {"field", PortKind::Control, ""},
               {"event", PortKind::Control, ""}},
              {{"rate", 0.002f, 1.0f, 0.05f, "Hz"},
               {"depth", 0.0f, 1.0f, 0.5f, ""},
               {"momentum", 0.0f, 1.0f, 0.5f, ""},
               {"stride", 0.0f, 1.0f, 0.45f, ""},
               {"anchor", 0.0f, 1.0f, 0.0f, ""},
               {"bias", -1.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "DRIFT"; }

    // Layout revisto 2026-09-06 (passe de ergonomia): 10 HP (W 50,8 mm),
    // display cheio, 3 fileiras de jacks agrupadas — entradas · 4 saídas
    // de campo correlacionadas (a-d) · campo bruto + gatilho de virada.
    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "DRIFT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "field", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 9.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DEPTH", "depth", 30.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MOMT", "momentum", 9.0f, 48.0f);
        p.add(Widget::Kind::Knob, "STRD", "stride", 30.0f, 48.0f);
        p.add(Widget::Kind::Knob, "BIAS", "bias", 9.0f, 70.0f);
        p.add(Widget::Kind::Knob, "ANCHR", "anchor", 30.0f, 70.0f);
        p.add(Widget::Kind::Jack, "ADV", "in:advance", 8.0f, 90.0f);
        p.add(Widget::Kind::Jack, "RATE", "in:rate_mod", 20.0f, 90.0f);
        p.add(Widget::Kind::Jack, "A", "out:a", 8.0f, 106.0f);
        p.add(Widget::Kind::Jack, "B", "out:b", 17.0f, 106.0f);
        p.add(Widget::Kind::Jack, "C", "out:c", 26.0f, 106.0f);
        p.add(Widget::Kind::Jack, "D", "out:d", 35.0f, 106.0f);
        p.add(Widget::Kind::Jack, "FLD", "out:field", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "EVT", "out:event", 21.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        const float bias = parameterValue("bias");
        reg_ = 0xACE1u;
        field_ = bias;
        vel_ = 0.0f;
        clockPhase_ = 0.0f;
        prevAdv_ = false;
        eventTimer_ = 0;
        rng_ = 0x2545F4914F6CDD1DULL;
        anchorReg_ = 0xACE1u;
        anchorField_ = bias;
        anchorSet_ = false;
        tickCount_ = 0;
        for (int k = 0; k < 4; ++k) {
            phase_[k] = 0.11f * static_cast<float>(k);
            held_[k] = 0.0f;
            out_[k] = bias;
        }
        fieldOut_ = bias;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const float rate = parameterValue("rate");
        const float depth = clamp01(parameterValue("depth"));
        const float momentum = clamp01(parameterValue("momentum"));
        const float stride = clamp01(parameterValue("stride"));
        const float anchor = clamp01(parameterValue("anchor"));
        const float bias = clampf(parameterValue("bias"), -1.0f, 1.0f);

        const AudioBlock* adv = inputs[0];
        const AudioBlock* rmod = inputs[1];

        static constexpr float ratio[4] = {1.0f, 1.37f, 1.83f, 2.41f};
        // pesos distintos por saída (somam ~1) — leituras diferentes do
        // mesmo registrador
        static constexpr float w[4][8] = {
            {0.42f, 0.24f, 0.14f, 0.08f, 0.05f, 0.03f, 0.02f, 0.02f},
            {0.02f, 0.02f, 0.03f, 0.05f, 0.08f, 0.14f, 0.24f, 0.42f},
            {0.12f, 0.40f, 0.06f, 0.22f, 0.03f, 0.10f, 0.02f, 0.05f},
            {0.05f, 0.02f, 0.22f, 0.03f, 0.40f, 0.06f, 0.12f, 0.10f}};

        for (std::size_t f = 0; f < frames; ++f) {
            const float rateHz = clampf(
                rate + (rmod ? rmod->at(0, f) * 0.5f : 0.0f), 0.001f, 4.0f);

            bool tick = false;
            if (adv != nullptr) {
                const bool hi = adv->at(0, f) >= 0.5f;
                tick = hi && !prevAdv_;
                prevAdv_ = hi;
            } else {
                clockPhase_ += rateHz / sr_;
                if (clockPhase_ >= 1.0f) { clockPhase_ -= 1.0f; tick = true; }
            }

            if (tick) {
                ++tickCount_;
                // MEMÓRIA DE TOPOLOGIA: a cada 8 tiques grava o "marco"
                // (LFSR + campo); em cada tique, com prob ∝ anchor², volta
                // pro marco em vez de dar um passo novo — a deriva ORBITA
                // paisagens em vez de vagar pra sempre. anchor = 0 → nunca
                // grava nem volta (sequência de RNG idêntica à antiga).
                if (anchor > 0.0f && (tickCount_ & 7u) == 0u) {
                    anchorReg_ = reg_;
                    anchorField_ = field_;
                    anchorSet_ = true;
                }
                const bool recall = anchorSet_
                    && (0.5f + 0.5f * white()) < anchor * anchor * 0.6f;
                if (recall) {
                    reg_ = anchorReg_;
                    vel_ *= 0.25f;
                    field_ += (anchorField_ - field_) * 0.6f;
                    field_ = clampf(field_, -1.0f, 1.0f);
                } else {
                    // LFSR de 8 bits: x^8 + x^6 + x^5 + x^4 + 1 (taps 0xB8)
                    const unsigned fb =
                        parity(static_cast<unsigned>(reg_) & 0xB8u);
                    reg_ = static_cast<std::uint16_t>(
                        ((reg_ << 1) | fb) & 0xFFu);
                    if (reg_ == 0) reg_ = 1;

                    const float impulse = white() * depth * 0.08f;
                    // momentum 0 -> velocidade zera todo tique (passeio branco)
                    // momentum 1 -> velocidade quase não decai (tendência longa)
                    vel_ = vel_ * (0.2f + 0.78f * momentum)
                         + impulse * (1.1f - 0.75f * momentum);
                    field_ += vel_;
                    // retorno ao repouso: forte com momentum baixo, ~nulo alto
                    field_ +=
                        (bias - field_) * (0.008f + 0.16f * (1.0f - momentum));
                    field_ = clampf(field_, -1.0f, 1.0f);
                }

                for (int k = 0; k < 4; ++k) {
                    float acc = 0.0f;
                    for (int i = 0; i < 8; ++i)
                        acc += ((reg_ >> i) & 1u) ? w[k][i] : 0.0f;
                    held_[k] = acc * 2.0f - 1.0f;
                }
                eventTimer_ = static_cast<std::uint32_t>(0.02f * sr_) + 1;
            }

            // glide relativo à cadência: ~0,4 tique de constante de tempo
            const float glide = clampf(rateHz / (0.4f * sr_), 2.0e-6f, 6.0e-4f);
            for (int k = 0; k < 4; ++k) {
                phase_[k] += (rateHz * 0.35f * ratio[k]) / sr_;
                phase_[k] -= std::floor(phase_[k]);
                const float own = std::sin(phase_[k] * 6.2831853f);
                // stride 0 -> só o campo compartilhado (as 4 saídas idênticas)
                // stride 1 -> sobretudo o LFO próprio + a leitura ponderada
                const float raw =
                    field_ * (1.0f - 0.85f * stride)
                    + own * (0.6f * stride)
                    + held_[k] * (0.06f + 0.3f * stride);
                const float target = bias + depth * clampf(raw, -1.0f, 1.0f);
                out_[k] += (target - out_[k]) * glide;
            }
            fieldOut_ += (bias + depth * field_ - fieldOut_) * glide;
            const float ev = eventTimer_ > 0 ? 1.0f : 0.0f;
            if (eventTimer_ > 0) --eventTimer_;

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = out_[0];
                outputs[1].at(c, f) = out_[1];
                outputs[2].at(c, f) = out_[2];
                outputs[3].at(c, f) = out_[3];
                outputs[4].at(c, f) = fieldOut_;
                outputs[5].at(c, f) = ev;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    static unsigned parity(unsigned x) noexcept {
        x ^= x >> 4; x ^= x >> 2; x ^= x >> 1; return x & 1u;
    }
    float white() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::int32_t>(x >> 32))
            / 2147483648.0f;
    }

    float sr_ = 48000.0f;
    std::uint16_t reg_ = 0xACE1u;
    std::uint16_t anchorReg_ = 0xACE1u;
    float field_ = 0.0f, vel_ = 0.0f, clockPhase_ = 0.0f;
    float anchorField_ = 0.0f;
    bool anchorSet_ = false;
    std::uint32_t tickCount_ = 0;
    bool prevAdv_ = false;
    std::uint32_t eventTimer_ = 0;
    std::uint64_t rng_ = 0x2545F4914F6CDD1DULL;
    float phase_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float held_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float out_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float fieldOut_ = 0.0f;
};

}  // namespace rasgo::modular
