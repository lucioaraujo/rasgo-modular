#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

// ============================================================================
// LOGIC — lógica e utilidades de clock (Módulo 22)
// ============================================================================
//
// A peça que RECOMBINA o tempo: divide/multiplica um clock, faz AND/OR/XOR
// de dois gates, um flip-flop tipo T e um atraso de gate. Fecha o rack de
// partida (`PESQUISA_MODULOS.md §2.1`).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/22_logic.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - ALM Pamela's Workout — dividir/multiplicar medindo o intervalo;
//   - Mutable Kinks / Doepfer A-166 — AND/OR/XOR simultâneos;
//   - flip-flop T (lógica digital) — alterna a cada borda de subida;
//   - Doepfer A-160 — contador módulo-N sobre as bordas.
//
// Sem RNG (timing de precisão). Anel de atraso alocado em prepare().

namespace rasgo::modular {

class Logic final : public Signal {
public:
    Logic()
        : Signal(
              {{"clock", PortKind::Control, ""},
               {"a", PortKind::Control, ""},
               {"b", PortKind::Control, ""},
               {"reset", PortKind::Control, ""}},
              {{"div", PortKind::Control, ""},
               {"and", PortKind::Control, ""},
               {"or", PortKind::Control, ""},
               {"xor", PortKind::Control, ""},
               {"flip", PortKind::Control, ""}},
              {{"rate", 0.1f, 40.0f, 2.0f, "Hz"},
               {"divide", 1.0f, 32.0f, 2.0f, ""},
               {"multiply", 1.0f, 8.0f, 1.0f, ""},
               {"gate_len", 0.02f, 0.98f, 0.5f, ""},
               {"delay", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "LOGIC"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "LOGIC", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "state", "", 2.5f, 7.0f, 40.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 8.0f, 26.0f);
        p.add(Widget::Kind::Knob, "DIV", "divide", 28.0f, 26.0f);
        p.add(Widget::Kind::Knob, "MULT", "multiply", 8.0f, 46.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 28.0f, 46.0f);
        p.add(Widget::Kind::Knob, "DELAY", "delay", 8.0f, 66.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 6.0f, 92.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 18.0f, 92.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 30.0f, 92.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 42.0f, 92.0f);
        p.add(Widget::Kind::Jack, "DIV", "out:div", 6.0f, 110.0f);
        p.add(Widget::Kind::Jack, "AND", "out:and", 15.0f, 110.0f);
        p.add(Widget::Kind::Jack, "OR", "out:or", 24.0f, 110.0f);
        p.add(Widget::Kind::Jack, "XOR", "out:xor", 33.0f, 110.0f);
        p.add(Widget::Kind::Jack, "FLIP", "out:flip", 42.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        ringSize_ = std::max<std::size_t>(
            1, static_cast<std::size_t>(0.2f * sr_) + 2);
        ring_.assign(ringSize_, 0.0f);
        ringWrite_ = 0;
        prevClock_ = prevA_ = prevReset_ = false;
        intPhase_ = 0.0f;
        sinceEdge_ = sinceDivPulse_ = 0;
        clockPeriod_ = divPeriod_ =
            std::max(1.0f, sr_ / std::max(0.1f, parameterValue("rate")));
        subInterval_ = clockPeriod_;
        subCount_ = 1;
        divCount_ = 0;
        flip_ = false;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const int divide = clampi(
            static_cast<int>(std::lround(parameterValue("divide"))), 1, 32);
        const int multiply = clampi(
            static_cast<int>(std::lround(parameterValue("multiply"))), 1, 8);
        const float gateLen = parameterValue("gate_len");
        const float rate = std::max(0.1f, parameterValue("rate"));
        const float intInc = rate / sr_;
        std::size_t delaySamples = static_cast<std::size_t>(
            parameterValue("delay") * 0.2f * sr_);
        if (delaySamples >= ringSize_) delaySamples = ringSize_ - 1;

        const AudioBlock* clk = inputs[0];
        const AudioBlock* inA = inputs[1];
        const AudioBlock* inB = inputs[2];
        const AudioBlock* inR = inputs[3];

        for (std::size_t f = 0; f < frames; ++f) {
            const bool rv = inR != nullptr && inR->at(0, f) >= 0.5f;
            const bool resetEdge = rv && !prevReset_;
            prevReset_ = rv;
            if (resetEdge) {
                divCount_ = 0;
                flip_ = false;
                subCount_ = multiply;   // sem sub-ticks pendentes
            }

            // ---- fonte de bordas: externa se `clock` conectado -----------
            bool edge = false;
            if (clk != nullptr) {
                const bool hi = clk->at(0, f) >= 0.5f;
                edge = hi && !prevClock_;
                prevClock_ = hi;
            } else {
                intPhase_ += intInc;
                if (intPhase_ >= 1.0f) { intPhase_ -= 1.0f; edge = true; }
            }

            ++sinceEdge_;
            ++sinceDivPulse_;

            bool tick = false;
            if (edge) {
                if (sinceEdge_ > 1)
                    clockPeriod_ = static_cast<float>(sinceEdge_);
                sinceEdge_ = 0;
                subInterval_ = clockPeriod_ / static_cast<float>(multiply);
                subCount_ = 1;          // a própria borda é o 1º tick
                tick = !rv;
            } else if (!rv && subCount_ < multiply
                       && static_cast<float>(sinceEdge_)
                              >= subInterval_ * static_cast<float>(subCount_)) {
                ++subCount_;
                tick = true;
            }

            bool divPulse = false;
            if (tick) {
                if (++divCount_ >= divide) {
                    divCount_ = 0;
                    if (sinceDivPulse_ > 1)
                        divPeriod_ = static_cast<float>(sinceDivPulse_);
                    sinceDivPulse_ = 0;
                    divPulse = true;
                }
            }
            (void)divPulse;

            const float divRaw =
                static_cast<float>(sinceDivPulse_) < gateLen * divPeriod_
                    ? 1.0f : 0.0f;

            // ---- atraso de gate (anel de 0/1) ---------------------------
            ring_[ringWrite_] = divRaw;
            const std::size_t readIdx =
                (ringWrite_ + ringSize_ - delaySamples) % ringSize_;
            const float divOut = ring_[readIdx];
            ringWrite_ = (ringWrite_ + 1) % ringSize_;

            // ---- lógica booleana --------------------------------------
            const bool av = inA != nullptr && inA->at(0, f) >= 0.5f;
            const bool bv = inB != nullptr && inB->at(0, f) >= 0.5f;
            if (av && !prevA_) flip_ = !flip_;
            prevA_ = av;
            if (rv) flip_ = false;

            const float aAndB = (av && bv) ? 1.0f : 0.0f;
            const float aOrB = (av || bv) ? 1.0f : 0.0f;
            const float aXorB = (av != bv) ? 1.0f : 0.0f;
            const float flipV = flip_ ? 1.0f : 0.0f;

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = divOut;
                outputs[1].at(c, f) = aAndB;
                outputs[2].at(c, f) = aOrB;
                outputs[3].at(c, f) = aXorB;
                outputs[4].at(c, f) = flipV;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    float sr_ = 48000.0f;
    std::vector<float> ring_;
    std::size_t ringSize_ = 1;
    std::size_t ringWrite_ = 0;

    bool prevClock_ = false;
    bool prevA_ = false;
    bool prevReset_ = false;
    float intPhase_ = 0.0f;
    std::uint64_t sinceEdge_ = 0;
    std::uint64_t sinceDivPulse_ = 0;
    float clockPeriod_ = 24000.0f;
    float divPeriod_ = 24000.0f;
    float subInterval_ = 24000.0f;
    int subCount_ = 1;
    int divCount_ = 0;
    bool flip_ = false;
};

}  // namespace rasgo::modular
