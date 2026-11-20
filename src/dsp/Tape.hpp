#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// TAPE (#96) — a fita magnética (lote 4 da tape music, 9/11/2026, aprovado
// pelo autor: "aprovo todos"; dossiê `dossies/96_tape.md`).
//
// A máquina de fita como cor e como desgaste. O som passa por:
//  1. a SATURAÇÃO magnética (DRIVE) — arredonda os picos em vez de cortá-los,
//     com a pequena assimetria do bias (o saturador de fita do Rasgo Synth
//     Studio, `TapeSaturator.hpp`, mesmo autor, adaptado);
//  2. a CABEÇA — a perda de agudo e a "corcova" de grave (head bump), que
//     dependem da velocidade da fita (SPEED: 7,5, 15 ou 30 ips);
//  3. o TRANSPORTE — WOW (a oscilação lenta do motor, 0,3–1,2 Hz, com uma
//     deriva ao acaso) e FLUTTER (o tremor rápido do cabrestante, 6–11 Hz),
//     por uma linha de atraso modulada;
//  4. o CHIADO (HISS), o ruído do óxido.
// Na tape music (Schaeffer, Henry, Bebe e Louis Barron, Daphne Oram), a fita
// era o instrumento; aqui ela é o meio por onde o patch passa.
//
// Conceito também lido no ChowTape (GPL; histerese de Jiles–Atherton);
// nenhum código copiado — aqui a saturação é a do Studio, sem modelo de
// histerese.
//
// Desvio Rasgo: WEAR, a fita gasta e "mastigada" — DROPOUTS (o óxido que
// falta: o nível cai de repente e volta, mais abafado) e o AMASSADO (rajadas
// de flutter quando a fita enruga). Com WEAR 0, a fita é nova.
//
// prepare() aloca a linha; process() não aloca; determinístico.
namespace rasgo::modular {

class Tape final : public Signal {
public:
    Tape() : Signal(
        {{"in", PortKind::Audio, ""},
         {"wear_cv", PortKind::Control, ""},
         {"drive_cv", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""},
         {"dropout", PortKind::Control, "gate"}},
        {{"drive", 0.0f, 1.0f, 0.4f, ""},
         {"speed", 0.0f, 2.0f, 1.0f, ""},
         {"wow", 0.0f, 1.0f, 0.25f, ""},
         {"flutter", 0.0f, 1.0f, 0.2f, ""},
         {"hiss", 0.0f, 1.0f, 0.15f, ""},
         {"wear", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "TAPE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "TAPE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "tape", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "DRIVE", "drive", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "WOW", "wow", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FLUT", "flutter", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "HISS", "hiss", 20.5f, 50.0f);
        p.add(Widget::Kind::Knob, "WEAR", "wear", 36.0f, 50.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "WEAR", "in:wear_cv", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "DRV", "in:drive_cv", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "DROP", "out:dropout", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        line_.assign(static_cast<std::size_t>(0.012f * sr_) + 4, 0.0f);
        pos_ = 0;
        rng_ = 0x7A9E0FF1CEULL;
        wowPh_ = 0.0; flPh1_ = 0.0; flPh2_ = 0.37;
        wowDrift_ = 0.0f;
        lp_ = hissLp_ = hissHp_ = 0.0f;
        bumpS1_ = bumpS2_ = 0.0f;
        dropEnv_ = 1.0f; dropTarget_ = 1.0f; dropLeft_ = 0;
        crinkle_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* wcv = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* dcv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float driveK = std::clamp(safe(parameterValue("drive"), 0.4f), 0.0f, 1.0f);
        // SPEED 0 / 1 / 2 = 7,5 / 15 / 30 polegadas por segundo
        const float speed = std::clamp(safe(parameterValue("speed"), 1.0f), 0.0f, 2.0f);
        const float wow = std::clamp(safe(parameterValue("wow"), 0.25f), 0.0f, 1.0f);
        const float flutter = std::clamp(safe(parameterValue("flutter"), 0.2f), 0.0f, 1.0f);
        const float hiss = std::clamp(safe(parameterValue("hiss"), 0.15f), 0.0f, 1.0f);
        const float wearK = std::clamp(safe(parameterValue("wear"), 0.0f), 0.0f, 1.0f);
        const float ips = 7.5f * std::exp2(speed);
        // a cabeça: o agudo cai mais na fita lenta (~8 kHz a 7,5 ips … 20 kHz a 30)
        const float cutHz = std::min(0.45f * sr_, 8000.0f * std::pow(ips / 7.5f, 0.66f));
        const float lpK = 1.0f - std::exp(-6.2831853f * cutHz / sr_);
        // a corcova de grave: ~40 Hz a 7,5 ips, sobe com a velocidade
        const float bumpHz = 40.0f * std::sqrt(ips / 7.5f) * 1.6f;
        const float bumpG = std::tan(3.14159265f * bumpHz / sr_);
        // o transporte (amplitudes da linha de atraso, em amostras)
        const float wowDepth = wow * 0.0025f * sr_ * (7.5f / ips + 0.5f);
        const float flDepth = flutter * 0.00018f * sr_;
        const double wowHz = 0.35 + 0.5 * static_cast<double>(wow);
        const double fl1 = 6.3, fl2 = 10.7;
        const float hissAmp = hiss * hiss * 0.02f;
        const std::size_t L = line_.size();
        const float center = 0.5f * static_cast<float>(L - 4);
        const int gateLen = std::max(1, static_cast<int>(0.01f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in ? safe(in->at(0, f), 0.0f) : 0.0f;
            float drive = driveK;
            if (dcv) drive = std::clamp(drive + safe(dcv->at(0, f), 0.0f), 0.0f, 1.0f);
            float wear = wearK;
            if (wcv) wear = std::clamp(wear + safe(wcv->at(0, f), 0.0f), 0.0f, 1.0f);

            // 1. saturação (o saturador de fita do Studio)
            const float d = x * (1.0f + 3.0f * drive);
            const float sat = std::tanh(d * 0.8f + d * d * 0.05f) / std::tanh(0.85f) / (1.0f + 1.2f * drive);

            // 2. cabeça: corcova de grave (SVF passa-faixa somado) e perda de agudo
            const float hp = (sat - 1.4f * bumpS1_ - bumpS2_) / (1.0f + 1.4f * bumpG + bumpG * bumpG);
            const float bp = bumpG * hp + bumpS1_;
            bumpS1_ = bumpG * hp + bp;
            const float lpb = bumpG * bp + bumpS2_;
            bumpS2_ = bumpG * bp + lpb;
            const float bumped = sat + 0.35f * bp;
            // no dropout, a cabeça perde ainda mais agudo (o óxido falta)
            const float k = lpK * (0.25f + 0.75f * dropEnv_);
            lp_ += (bumped - lp_) * k;

            // 3. transporte: escreve e lê com atraso modulado (interpolação linear)
            line_[pos_] = lp_;
            wowPh_ += wowHz / sr_; if (wowPh_ >= 1.0) wowPh_ -= 1.0;
            flPh1_ += fl1 / sr_; if (flPh1_ >= 1.0) flPh1_ -= 1.0;
            flPh2_ += fl2 / sr_; if (flPh2_ >= 1.0) flPh2_ -= 1.0;
            wowDrift_ = 0.9995f * wowDrift_ + 0.0005f * (2.0f * unit() - 1.0f) * 8.0f;
            // o amassado: rajadas de flutter que nascem do desgaste e somem
            if (wear > 0.0f && unit() < wear * 0.4f / sr_) crinkle_ = 1.0f;
            crinkle_ *= 0.99985f;
            const float mod = wowDepth * (static_cast<float>(std::sin(6.283185307179586 * wowPh_)) + 0.6f * wowDrift_)
                            + flDepth * (static_cast<float>(std::sin(6.283185307179586 * flPh1_))
                                         + 0.5f * static_cast<float>(std::sin(6.283185307179586 * flPh2_)))
                            + crinkle_ * 0.0006f * sr_ * (2.0f * unit() - 1.0f) * 0.15f;
            float delay = std::clamp(center + mod, 1.0f, static_cast<float>(L - 3));
            float rp = static_cast<float>(pos_) - delay;
            while (rp < 0.0f) rp += static_cast<float>(L);
            const std::size_t i0 = static_cast<std::size_t>(rp) % L, i1 = (i0 + 1) % L;
            const float fr = rp - std::floor(rp);
            float y = line_[i0] * (1.0f - fr) + line_[i1] * fr;
            if (++pos_ >= L) pos_ = 0;

            // desgaste: dropouts (Poisson, ~WEAR·0,8 por segundo)
            if (dropLeft_ <= 0 && wear > 0.0f && unit() < wear * 0.8f / sr_) {
                dropLeft_ = static_cast<int>((0.03f + 0.17f * unit()) * sr_);
                dropTarget_ = 0.05f + 0.35f * unit();
                gateLeft_ = gateLen;
            }
            if (dropLeft_ > 0) --dropLeft_;
            else dropTarget_ = 1.0f;
            dropEnv_ += (dropTarget_ - dropEnv_) * (dropTarget_ < dropEnv_ ? 0.01f : 0.002f) * (48000.0f / sr_);
            y *= dropEnv_;

            // 4. chiado: ruído claro (passa-alta suave), sempre por baixo
            const float n = 2.0f * unit() - 1.0f;
            hissLp_ += (n - hissLp_) * 0.35f;
            hissHp_ += (hissLp_ - hissHp_) * 0.02f;
            y += hissAmp * (hissLp_ - hissHp_);

            if (!std::isfinite(y)) { y = 0.0f; prepareLine(); }
            const float gate = gateLeft_ > 0 ? 1.0f : 0.0f;
            if (gateLeft_ > 0) --gateLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = y;
                outputs[1].at(c, f) = gate;
            }
        }
    }

private:
    void prepareLine() noexcept {
        std::fill(line_.begin(), line_.end(), 0.0f);
        lp_ = bumpS1_ = bumpS2_ = 0.0f;
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::vector<float> line_;
    std::size_t pos_ = 0;
    std::uint64_t rng_ = 0x7A9E0FF1CEULL;
    double wowPh_ = 0.0, flPh1_ = 0.0, flPh2_ = 0.37;
    float wowDrift_ = 0.0f, lp_ = 0.0f, hissLp_ = 0.0f, hissHp_ = 0.0f;
    float bumpS1_ = 0.0f, bumpS2_ = 0.0f;
    float dropEnv_ = 1.0f, dropTarget_ = 1.0f, crinkle_ = 0.0f;
    int dropLeft_ = 0, gateLeft_ = 0;
};

} // namespace rasgo::modular
