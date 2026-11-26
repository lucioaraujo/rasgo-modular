#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// RUNGLER (#102) — o coração do Benjolin de Rob Hordijk (leitura da família,
// 9/11/2026, aprovada pelo autor: "aprovo todos, continue"; dossiê
// `dossies/102_rungler.md`).
//
// Dois osciladores de triângulo, A e B, e um registrador de 8 bits. A cada
// subida do quadrado de A, o registrador anda uma casa e recebe um bit novo:
// o quadrado de B naquele instante. Os três últimos bits viram uma tensão de
// 8 degraus — o RUNGLER — que volta e modula a frequência dos dois
// osciladores (MODA, MODB). O laço fecha: com pouca modulação, um padrão
// que se repete; com muita, o circuito tropeça entre travar num ritmo e
// virar ruído — a "fratura matemática" do Benjolin e do Blippoo Box. LOOP é
// a chave de Hordijk: no máximo, o bit que sai do fim volta para o começo
// (em vez do bit de B) e o padrão de 8 passos se repete para sempre; no
// meio, de vez em quando o bit novo entra.
//
// O Rasgo Synth Studio tem um Rungler (`Rungler.hpp`, mesmo autor); aqui ele
// é refeito no desenho de Hordijk (três bits, o comparador PWM, a chave de
// laço). Conceito público dos textos de Hordijk; nenhum código de clone.
//
// Saídas: TRIA e TRIB (os triângulos, áudio), PWM (o comparador A > B, o
// som rouco do Benjolin) e RUNG (os 8 degraus, 0–1: um sequenciador que
// pensa sozinho).
//
// Desvio Rasgo: LOOP por CV e um laço semeado — o padrão que o LOOP prende
// nasce de uma semente fixa, então o mesmo patch prende o mesmo padrão.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Rungler final : public Signal {
public:
    Rungler() : Signal(
        {{"pitch_a", PortKind::Control, "v/oct"},
         {"pitch_b", PortKind::Control, "v/oct"},
         {"loop_cv", PortKind::Control, ""}},
        {{"tri_a", PortKind::Audio, ""},
         {"tri_b", PortKind::Audio, ""},
         {"pwm", PortKind::Audio, ""},
         {"rung", PortKind::Control, ""}},
        {{"freq_a", 0.1f, 4000.0f, 7.0f, "Hz"},
         {"freq_b", 0.1f, 4000.0f, 230.0f, "Hz"},
         {"mod_a", 0.0f, 1.0f, 0.4f, ""},
         {"mod_b", 0.0f, 1.0f, 0.5f, ""},
         {"loop", 0.0f, 1.0f, 0.0f, ""},
         {"level", 0.0f, 1.0f, 0.5f, ""}}) {}

    std::string type() const override { return "RUNGLER"; }

    Panel panel() const override {
        Panel p;
        // Arquétipo das FONTES (autor, 26/11/2026): o knob herói no centro;
        // dois heróis (12 mm) lado a lado — os dois
        // osciladores que se modulam — e o LOOP (o registrador) entre eles.
        p.hp = 10;
        p.add(Widget::Kind::Label, "RUNGLER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "rungler", "", 2.5f, 6.0f, 45.80f);
        p.add(Widget::Kind::Knob, "FRQA", "freq_a", 8.40f, 27.0f, 12.0f);   // herói
        p.add(Widget::Kind::Knob, "FRQB", "freq_b", 30.40f, 27.0f, 12.0f);   // herói
        p.add(Widget::Kind::Knob, "MODA", "mod_a", 10.40f, 48.0f);
        p.add(Widget::Kind::Knob, "LOOP", "loop", 21.40f, 48.0f);
        p.add(Widget::Kind::Knob, "MODB", "mod_b", 32.40f, 48.0f);
        p.add(Widget::Kind::Knob, "LEVEL", "level", 21.40f, 64.0f);
        p.add(Widget::Kind::Jack, "PTA", "in:pitch_a", 14.40f, 88.0f);
        p.add(Widget::Kind::Jack, "LOOP", "in:loop_cv", 25.40f, 88.0f);
        p.add(Widget::Kind::Jack, "PTB", "in:pitch_b", 36.40f, 88.0f);
        p.add(Widget::Kind::Jack, "TRIA", "out:tri_a", 8.60f, 110.0f);
        p.add(Widget::Kind::Jack, "PWM", "out:pwm", 19.80f, 110.0f);
        p.add(Widget::Kind::Jack, "RUNG", "out:rung", 31.00f, 110.0f);
        p.add(Widget::Kind::Jack, "TRIB", "out:tri_b", 42.20f, 110.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        reg_ = 0xB2u;   // a semente do laço
        rng_ = 0x8E1D1F0ULL;
        phA_ = 0.0; phB_ = 0.25;
        rung_ = dac(reg_);
        steps_ = 0;
    }

    std::uint8_t reg() const noexcept { return reg_; }
    long steps() const noexcept { return steps_; }
    // os três bits mais novos do registrador → 0, 1/7 … 1
    static float dac(std::uint8_t r) noexcept { return static_cast<float>((r >> 5) & 7u) / 7.0f; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* pa = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* pb = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* lcv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float fA = std::clamp(safe(parameterValue("freq_a"), 7.0f), 0.1f, 4000.0f);
        const float fB = std::clamp(safe(parameterValue("freq_b"), 230.0f), 0.1f, 4000.0f);
        const float modA = std::clamp(safe(parameterValue("mod_a"), 0.4f), 0.0f, 1.0f);
        const float modB = std::clamp(safe(parameterValue("mod_b"), 0.5f), 0.0f, 1.0f);
        const float loopK = std::clamp(safe(parameterValue("loop"), 0.0f), 0.0f, 1.0f);
        const float level = std::clamp(safe(parameterValue("level"), 0.5f), 0.0f, 1.0f);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            // o rungler volta para as duas frequências: até 4 oitavas de modulação
            const float r = rung_ - 0.5f;
            float hzA = fA * std::exp2(4.0f * modA * r);
            float hzB = fB * std::exp2(-4.0f * modB * r);
            if (pa) hzA *= std::exp2(std::clamp(safe(pa->at(0, f), 0.0f), -8.0f, 8.0f));
            if (pb) hzB *= std::exp2(std::clamp(safe(pb->at(0, f), 0.0f), -8.0f, 8.0f));
            hzA = std::clamp(hzA, 0.02f, 0.4f * sr_);
            hzB = std::clamp(hzB, 0.02f, 0.4f * sr_);
            float loop = loopK;
            if (lcv) loop = std::clamp(loop + safe(lcv->at(0, f), 0.0f), 0.0f, 1.0f);

            phA_ += static_cast<double>(hzA) / sr_;
            phB_ += static_cast<double>(hzB) / sr_;
            if (phB_ >= 1.0) phB_ -= std::floor(phB_);
            // a subida do quadrado de A (fase passa por 0): o registrador anda
            if (phA_ >= 1.0) {
                phA_ -= std::floor(phA_);
                const bool bitB = phB_ < 0.5;   // o quadrado de B agora
                const bool last = (reg_ & 0x80u) != 0;
                // LOOP: o bit que sai volta, com probabilidade crescente
                const bool keep = loop >= 1.0f || (loop > 0.0f && unit() < loop);
                const bool in = keep ? last : bitB;
                reg_ = static_cast<std::uint8_t>((reg_ << 1) | (in ? 1u : 0u));
                rung_ = dac(reg_);
                ++steps_;
            }
            const float triA = static_cast<float>(4.0 * std::fabs(phA_ - 0.5) - 1.0);
            const float triB = static_cast<float>(4.0 * std::fabs(phB_ - 0.5) - 1.0);
            const float pwm = triA > triB ? 1.0f : -1.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = level * triA;
                outputs[1].at(c, f) = level * triB;
                outputs[2].at(c, f) = level * 0.6f * pwm;
                outputs[3].at(c, f) = rung_;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint8_t reg_ = 0xB2u;
    std::uint64_t rng_ = 0x8E1D1F0ULL;
    double phA_ = 0.0, phB_ = 0.25;
    float rung_ = 0.0f;
    long steps_ = 0;
};

} // namespace rasgo::modular
