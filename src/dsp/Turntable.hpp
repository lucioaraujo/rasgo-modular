#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// TURNTABLE — toca-discos com prato de inércia (Módulo 50)
// ============================================================================
//
// O `SAMPLER` (#48) dispara fatias; o `TURNTABLE` lê o mesmo buffer por um
// PRATO COM MASSA: a posição de leitura é a integral de uma velocidade
// angular com inércia — o motor puxa devagar (`torque`), a mão empurra
// forte (`scratch`), o freio faz descer coasting (`brake`, `friction`).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/50_turntable.md`.
//
// Desvio Rasgo: o Navalha 2 rejeita a metáfora de DJ de propósito; aqui
// o RASGO diverge, mas com o MODELO FÍSICO do prato (EDO de 1ª ordem),
// não um "botão de scratch". SEM quantização de BPM — beatmatch é gesto.
//
// - `speed` (−1..1)  velocidade alvo: sinal·2^|speed| → ±0,5×–±2×
// - `torque` (0–1)   força do motor (tempo de subida ~1,1 s a ~10 ms)
// - `friction` (0–1) atrito: parada no `brake` + retorno pós-scratch
// - `grab` (0–1)     quanto a CV `scratch` joga o prato (firmeza da mão)
// - `start` (0–1)    onde a agulha cai (`trig`)
// - `wear` (0–1)     estalos de vinil + instabilidade — DETERMINÍSTICO
// - `loop` (0/1)     groove travado
//
// Gravação: gate `rec` grava `in` (~8 s). Ou `setBuffer()` do painel.
// Sem buffer → silêncio. `process()` não aloca. Determinístico.

namespace rasgo::modular {

class Turntable final : public Signal {
public:
    Turntable()
        : Signal(
              {{"trig", PortKind::Control, "trig"},
               {"in", PortKind::Audio, ""},
               {"rec", PortKind::Control, "gate"},
               {"scratch", PortKind::Control, ""},
               {"brake", PortKind::Control, "gate"}},
              {{"out", PortKind::Audio, ""}},
              {{"speed", -1.0f, 1.0f, 0.0f, ""},
               {"torque", 0.0f, 1.0f, 0.6f, ""},
               {"friction", 0.0f, 1.0f, 0.4f, ""},
               {"grab", 0.0f, 1.0f, 0.7f, ""},
               {"start", 0.0f, 1.0f, 0.0f, ""},
               {"wear", 0.0f, 1.0f, 0.0f, ""},
               {"loop", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "TURNTABLE"; }

    void setBuffer(std::vector<float> mono, const float srcRate) {
        loaded_ = std::move(mono);
        loadedRate_ = srcRate > 0.0f ? srcRate : 48000.0f;
        recLen_ = 0;
    }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "TURNTABLE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "vinyl", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "TORQ", "torque", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "FRIC", "friction", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "GRAB", "grab", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "START", "start", 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "WEAR", "wear", 24.0f, 50.0f);
        p.add(Widget::Kind::Toggle, "LOOP", "loop", 42.0f, 52.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trig", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 22.0f, 92.0f);
        p.add(Widget::Kind::Jack, "REC", "in:rec", 34.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SCR", "in:scratch", 46.0f, 92.0f);
        p.add(Widget::Kind::Jack, "BRK", "in:brake", 58.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        bufLen_ = static_cast<std::size_t>(8.0f * sr_) + 4;
        rec_.assign(bufLen_, 0.0f);
        recWrite_ = 0;
        recLen_ = 0;
        recording_ = false;
        prevRec_ = 0.0f;
        prevTrig_ = 0.0f;
        readPos_ = 0.0;
        platterVel_ = 0.0;
        motorRunning_ = false;
        dropEnv_ = 1.0f;
        dcX1_ = 0.0f;
        dcY1_ = 0.0f;
        rng_ = 0x51ED270B5A1C0FFEULL;
        wobCount_ = 0;
        wob_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* trigIn = inputs[0];
        const AudioBlock* audIn = inputs[1];
        const AudioBlock* recIn = inputs[2];
        const AudioBlock* scrIn = inputs[3];
        const AudioBlock* brkIn = inputs[4];

        const float speedP = clampf(parameterValue("speed"), -1.0f, 1.0f);
        const float torque = clamp01(parameterValue("torque"));
        const float friction = clamp01(parameterValue("friction"));
        const float grab = clamp01(parameterValue("grab"));
        const float startP = clamp01(parameterValue("start"));
        const float wear = clamp01(parameterValue("wear"));
        const bool loop = parameterValue("loop") >= 0.5f;

        const bool useLoaded = !loaded_.empty();
        const std::vector<float>& buf = useLoaded ? loaded_ : rec_;
        const double effLen = static_cast<double>(useLoaded ? loaded_.size()
                                                            : recLen_);
        const double rateRatio = useLoaded
            ? (static_cast<double>(loadedRate_) / static_cast<double>(sr_))
            : 1.0;

        const float speedRatio = (speedP < 0.0f ? -1.0f : 1.0f)
                               * std::pow(2.0f, std::fabs(speedP));
        const float motorCoef = 0.00002f + torque * torque * 0.0022f;
        // atrito ocioso mínimo (o motor servo segura a velocidade); o
        // atrito real só conta no `brake`.
        const float dragIdle = friction * 0.00003f;
        const float dropRamp = 1.0f - std::exp(-1.0f / (0.005f * sr_));
        const float wearGain = 1.0f - 0.04f * wear;
        const float rampCoef = dropRamp;

        for (std::size_t f = 0; f < frames; ++f) {
            // ---- gravação ----
            const float recG = recIn ? recIn->at(0, f) : 0.0f;
            if (!useLoaded) {
                if (recG >= 0.5f && prevRec_ < 0.5f) {
                    recording_ = true;
                    recWrite_ = 0;
                }
                if (recording_) {
                    rec_[recWrite_] = audIn ? audIn->at(0, f) : 0.0f;
                    if (++recWrite_ >= bufLen_) { recording_ = false; recLen_ = bufLen_; }
                }
                if (recG < 0.5f && prevRec_ >= 0.5f && recording_) {
                    recording_ = false;
                    recLen_ = recWrite_;
                }
            }
            prevRec_ = recG;

            // ---- agulha (trig) ----
            const float tg = trigIn ? trigIn->at(0, f) : 0.0f;
            if (tg >= 0.5f && prevTrig_ < 0.5f && effLen >= 8.0) {
                readPos_ = startP * effLen;
                dropEnv_ = 0.0f;
                motorRunning_ = true;   // pôr a agulha liga o motor
            }
            prevTrig_ = tg;

            const float scr = scrIn ? scrIn->at(0, f) : 0.0f;
            const bool brakeOn = brkIn && brkIn->at(0, f) >= 0.5f;

            // ---- física do prato ----
            // motor só puxa depois do 1º `trig` (a agulha caiu); a mão
            // (`scratch`) move o prato mesmo com o motor parado.
            const double motorTgt = (motorRunning_ && !brakeOn)
                ? static_cast<double>(speedRatio) : 0.0;
            platterVel_ += (motorTgt - platterVel_) * static_cast<double>(motorCoef);
            platterVel_ -= platterVel_
                * static_cast<double>(brakeOn ? dragIdle * 22.0f + 0.0004f
                                              : dragIdle);
            // a mão
            const double handTgt = static_cast<double>(scr) * 4.0;
            const float grip = clamp01(std::fabs(scr) * 5.0f) * grab;
            platterVel_ += (handTgt - platterVel_)
                * static_cast<double>(grip) * 0.3;

            // instabilidade de rotação (wear)
            if (wear > 0.0f && ++wobCount_ >= 256) {
                wobCount_ = 0;
                wob_ = (rnd01() - 0.5f) * wear * 0.0006f;
            }
            platterVel_ += static_cast<double>(wob_);

            // ---- avança a posição ----
            readPos_ += platterVel_ * rateRatio;
            if (effLen >= 8.0) {
                if (loop) {
                    while (readPos_ >= effLen) readPos_ -= effLen;
                    while (readPos_ < 0.0) readPos_ += effLen;
                } else {
                    if (readPos_ < 0.0) { readPos_ = 0.0; platterVel_ = 0.0; }
                    if (readPos_ > effLen - 2.0) {
                        readPos_ = effLen - 2.0;
                        platterVel_ = 0.0;   // "o disco acabou"
                    }
                }
            }

            // ---- leitura + envelope + wear ----
            dropEnv_ += (1.0f - dropEnv_) * rampCoef;
            float y = 0.0f;
            if (effLen >= 8.0) {
                y = readBuf(buf, readPos_) * dropEnv_ * wearGain;
                if (wear > 0.0f) y += crackle(wear);
            }

            // acoplamento AC (o vinil não tem DC): quando o prato para, a
            // amostra congelada some em ~40 ms em vez de virar um degrau.
            const float yHp = y - dcX1_ + 0.99950f * dcY1_;
            dcX1_ = y;
            dcY1_ = yHp;

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = yHp;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    std::uint64_t xn() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return rng_;
    }
    float rnd01() noexcept {
        return static_cast<float>(xn() >> 40) / 16777216.0f;
    }
    // estalo de vinil: impulsos esparsos, densidade e amplitude ∝ wear²
    float crackle(const float wear) noexcept {
        const float w2 = wear * wear;
        if (rnd01() < 0.0006f + w2 * 0.004f)
            return (rnd01() * 2.0f - 1.0f) * (0.05f + w2 * 0.35f);
        return 0.0f;
    }

    static float readBuf(const std::vector<float>& b, const double pos) noexcept {
        double p = pos;
        if (p < 0.0) p = 0.0;
        if (p > static_cast<double>(b.size()) - 2.0)
            p = static_cast<double>(b.size()) - 2.0;
        if (p < 0.0) return 0.0f;
        const std::size_t i0 = static_cast<std::size_t>(p);
        const std::size_t i1 = i0 + 1 < b.size() ? i0 + 1 : i0;
        const float fr = static_cast<float>(p - static_cast<double>(i0));
        return b[i0] * (1.0f - fr) + b[i1] * fr;
    }

    std::vector<float> rec_;
    std::vector<float> loaded_;
    float loadedRate_ = 48000.0f;
    std::size_t bufLen_ = 1;
    std::size_t recWrite_ = 0;
    std::size_t recLen_ = 0;
    bool recording_ = false;
    float prevRec_ = 0.0f, prevTrig_ = 0.0f;

    double readPos_ = 0.0;
    double platterVel_ = 0.0;
    bool motorRunning_ = false;
    float dropEnv_ = 1.0f;
    float dcX1_ = 0.0f, dcY1_ = 0.0f;
    std::uint64_t rng_ = 0x51ED270B5A1C0FFEULL;
    int wobCount_ = 0;
    float wob_ = 0.0f;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
