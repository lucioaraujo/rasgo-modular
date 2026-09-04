#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// SEQUENCE — sequenciador de passos com família de comportamentos (Módulo 15)
// ============================================================================
//
// A intenção, não só o acaso. Um padrão de 8 passos EDITÁVEL (altura e
// gate por passo, como parâmetros — o "edit surface" é o próprio patch de
// texto) tocado por uma FAMÍLIA DE COMPORTAMENTOS de leitura: pra frente,
// pra trás, ping-pong, aleatório, browniano. O `TURING` (Módulo 8) faz
// sequência que emerge do acaso; o `SEQUENCE` faz a sequência que você
// escreve e depois deixa o comportamento reler.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/15_sequence.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Hexen §119 - "o sequenciador não é um objeto, é uma família de
//     comportamentos de leitura sobre um padrão";
//   - Make Noise René / Metropolix / Intellijel Metropolix - direção de
//     leitura como parâmetro; passo com gate/altura/repetição próprios;
//   - Grids / Marbles - modo browniano (passo a passo ±1) entre "travado"
//     e "aleatório".
//
// Determinístico: xorshift64* semeado em prepare() (modos random/brown).
// Avança por `clock` externo OU relógio interno em `rate`.

namespace rasgo::modular {

class StepSequencer final : public Signal {
public:
    // 0 forward · 1 backward · 2 pingpong · 3 random · 4 brownian
    StepSequencer()
        : Signal(
              {{"clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"}},
              {{"pitch", PortKind::Audio, ""},
               {"gate", PortKind::Control, "gate"},
               {"eos", PortKind::Control, "gate"}},
              {{"length", 1.0f, 8.0f, 8.0f, ""},
               {"mode", 0.0f, 4.0f, 0.0f, ""},
               {"rate", 0.01f, 40.0f, 4.0f, "Hz"},
               {"gate_len", 0.05f, 0.95f, 0.5f, ""},
               {"glide", 0.0f, 1.0f, 0.0f, ""},
               {"range", 0.0f, 2.0f, 1.0f, "oct"},
               {"p1", -1.0f, 1.0f, 0.0f, ""}, {"g1", 0.0f, 1.0f, 1.0f, ""},
               {"p2", -1.0f, 1.0f, 0.25f, ""}, {"g2", 0.0f, 1.0f, 1.0f, ""},
               {"p3", -1.0f, 1.0f, 0.5f, ""}, {"g3", 0.0f, 1.0f, 0.0f, ""},
               {"p4", -1.0f, 1.0f, 0.25f, ""}, {"g4", 0.0f, 1.0f, 1.0f, ""},
               {"p5", -1.0f, 1.0f, -0.25f, ""}, {"g5", 0.0f, 1.0f, 1.0f, ""},
               {"p6", -1.0f, 1.0f, 0.0f, ""}, {"g6", 0.0f, 1.0f, 0.0f, ""},
               {"p7", -1.0f, 1.0f, 0.5f, ""}, {"g7", 0.0f, 1.0f, 1.0f, ""},
               {"p8", -1.0f, 1.0f, 0.75f, ""}, {"g8", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "SEQUENCE"; }

    // Layout: 20 HP - uma fileira de altura, uma de gate.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm.
        // 8 colunas de passo (altura + gate), macros e I/O embaixo
        Panel p;
        p.hp = 20;
        p.add(Widget::Kind::Label, "SEQUENCE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "steps", "", 2.5f, 8.0f, 95.0f);
        for (int i = 0; i < 8; ++i) {
            const std::string n = std::to_string(i + 1);
            const float x = 7.0f + static_cast<float>(i) * 12.0f;
            p.add(Widget::Kind::Slider, ("P" + n), "p" + n, x, 25.0f);
            p.add(Widget::Kind::Toggle, ("G" + n), "g" + n, x, 64.0f);
        }
        p.add(Widget::Kind::Knob, "LEN", "length", 8.0f, 80.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 24.0f, 80.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 40.0f, 80.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 56.0f, 80.0f);
        p.add(Widget::Kind::Knob, "GLIDE", "glide", 72.0f, 80.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 88.0f, 80.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 5.0f, 104.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 17.0f, 104.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 5.0f, 117.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 17.0f, 117.0f);
        p.add(Widget::Kind::Jack, "EOS", "out:eos", 29.0f, 117.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        idx_ = 0;
        dir_ = 1;
        prevClock_ = 0.0f;
        prevReset_ = 0.0f;
        rngState_ = 0xC2B2AE3D27D4EB4FULL;
        eosCountdown_ = 0;
        extGateSamples_ = 0;
        extPeriod_ = std::max(64.0f, sampleRate / 4.0f);
        gateSamples_ = static_cast<int>(0.003f * sampleRate);
        pitch_ = stepPitch(0) * parameterValue("range");
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& pitchOut = outputs[0];
        AudioBlock& gateOut = outputs[1];
        AudioBlock& eosOut = outputs[2];
        const std::size_t frames = pitchOut.frames();
        const std::size_t channels = pitchOut.channels();

        const int length = clampi(
            static_cast<int>(std::lround(parameterValue("length"))), 1, 8);
        const int mode = clampi(
            static_cast<int>(std::lround(parameterValue("mode"))), 0, 4);
        const float rate = parameterValue("rate");
        const float gateLen = parameterValue("gate_len");
        const float glide = parameterValue("glide");
        const float range = parameterValue("range");

        const AudioBlock* clock = inputs[0];
        const AudioBlock* reset = inputs[1];
        const bool externalClock = (clock != nullptr);
        const double dp = static_cast<double>(rate) / sampleRate_;
        const float glideCoeff = glide <= 0.0f
            ? 1.0f
            : 1.0f - std::exp(-1.0f
                              / std::max(1.0f, glide * 0.15f * sampleRate_));

        for (std::size_t frame = 0; frame < frames; ++frame) {
            if (reset) {
                const float r = reset->at(0, frame);
                if (prevReset_ < 0.5f && r >= 0.5f) {
                    idx_ = 0;
                    dir_ = 1;
                    phase_ = 0.0;
                }
                prevReset_ = r;
            }

            bool stepped = false;
            if (externalClock) {
                const float c = clock->at(0, frame);
                if (prevClock_ < 0.5f && c >= 0.5f) {
                    advance(length, mode);
                    stepped = true;
                    // janela de gate: fração do intervalo estimado (usa o
                    // último período entre clocks, ~200 ms default)
                    extGateSamples_ = static_cast<int>(
                        gateLen * std::max(64.0f, extPeriod_));
                }
                prevClock_ = c;
                extPeriod_ += 1.0f;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) {
                    phase_ -= 1.0;
                    advance(length, mode);
                    stepped = true;
                }
            }
            if (stepped) {
                if (externalClock && extPeriod_ > 1.0f)
                    extPeriod_ = 1.0f;  // reinicia a contagem do período
                if (idx_ == 0)
                    eosCountdown_ = gateSamples_;
            }

            const float target = stepPitch(idx_) * range;
            if (glideCoeff >= 1.0f)
                pitch_ = target;
            else
                pitch_ += (target - pitch_) * glideCoeff;

            const bool stepGateOn = parameterValue(gateParam(idx_)) >= 0.5f;
            float gate = 0.0f;
            if (stepGateOn) {
                if (externalClock) {
                    if (extGateSamples_ > 0) {
                        gate = 1.0f;
                        --extGateSamples_;
                    }
                } else {
                    gate = phase_ < gateLen ? 1.0f : 0.0f;
                }
            }

            float eos = 0.0f;
            if (eosCountdown_ > 0) {
                eos = 1.0f;
                --eosCountdown_;
            }

            for (std::size_t channel = 0; channel < channels; ++channel) {
                pitchOut.at(channel, frame) = pitch_;
                gateOut.at(channel, frame) = gate;
                eosOut.at(channel, frame) = eos;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    static const char* pitchParam(const int i) noexcept {
        static const char* names[8] = {"p1", "p2", "p3", "p4",
                                       "p5", "p6", "p7", "p8"};
        return names[i & 7];
    }
    static const char* gateParam(const int i) noexcept {
        static const char* names[8] = {"g1", "g2", "g3", "g4",
                                       "g5", "g6", "g7", "g8"};
        return names[i & 7];
    }
    float stepPitch(const int i) const noexcept {
        return parameterValue(pitchParam(i));
    }

    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    void advance(const int length, const int mode) noexcept {
        switch (mode) {
        case 0:  // forward
            idx_ = (idx_ + 1) % length;
            break;
        case 1:  // backward
            idx_ = (idx_ - 1 + length) % length;
            break;
        case 2:  // pingpong
            if (length == 1) { idx_ = 0; break; }
            idx_ += dir_;
            if (idx_ >= length - 1) { idx_ = length - 1; dir_ = -1; }
            else if (idx_ <= 0) { idx_ = 0; dir_ = 1; }
            break;
        case 3:  // random
            idx_ = static_cast<int>(uniform01() * static_cast<float>(length))
                % length;
            break;
        case 4:  // brownian
        default: {
            const float r = uniform01();
            const int d = r < 0.34f ? -1 : (r < 0.67f ? 0 : 1);
            idx_ = (idx_ + d + length) % length;
            break;
        }
        }
    }

    double phase_ = 0.0;
    int idx_ = 0;
    int dir_ = 1;
    float prevClock_ = 0.0f;
    float prevReset_ = 0.0f;
    std::uint64_t rngState_ = 0xC2B2AE3D27D4EB4FULL;
    float pitch_ = 0.0f;
    int eosCountdown_ = 0;
    int extGateSamples_ = 0;
    float extPeriod_ = 12000.0f;
    int gateSamples_ = 144;
};

}  // namespace rasgo::modular
