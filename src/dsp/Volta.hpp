#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// VOLTA — o laço que volta (Módulo 107), irmão reversível do TURING
// ============================================================================
//
// Decisão do autor (21/11/2026): "turing reversível, módulo irmão, mas com um
// nome mais curto". Do estudo da família (Navalha 2, "transformação
// reversível"): o TURING reescreve o próprio laço — o que muda se perde. Na
// VOLTA a sequência-base NUNCA é reescrita:
//
//   - MUT transforma passos numa ORDEM FIXA por passo (cada passo tem o seu
//     limiar, sorteado uma vez): subir o MUT troca mais passos pelo seu valor
//     alternativo; descer devolve exatamente o laço de antes;
//   - ERODE cala passos, também em ordem fixa (o pulso some, a CV segura);
//   - MEM protege os passos de menor limiar de memória: não mudam nem calam;
//   - NEW (gatilho) é o único jeito de trocar a base — um laço novo.
//
// Saídas como as do TURING (CV, CV2, PULSE) para se trocarem no patch, e POS
// (o passo atual, 0–1). Determinístico: tudo nasce do SEED.

namespace rasgo::modular {

class Volta final : public Signal {
public:
    Volta()
        : Signal(
              {{"clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"},
               {"new", PortKind::Control, "trig"},
               {"mut_cv", PortKind::Control, ""},
               {"ero_cv", PortKind::Control, ""}},
              {{"cv", PortKind::Audio, ""},
               {"cv2", PortKind::Audio, ""},
               {"pulse", PortKind::Control, "gate"},
               {"pos", PortKind::Control, ""}},
              {{"rate", 0.01f, 50.0f, 4.0f, "Hz"},
               {"length", 2.0f, 16.0f, 8.0f, ""},
               {"mutate", 0.0f, 1.0f, 0.0f, ""},
               {"erode", 0.0f, 1.0f, 0.0f, ""},
               {"protect", 0.0f, 1.0f, 0.0f, ""},
               {"range", 0.0f, 1.0f, 0.6f, ""},
               {"steps", 1.0f, 32.0f, 1.0f, ""},
               {"offset", -1.0f, 1.0f, 0.0f, ""},
               {"seed", 1.0f, 65535.0f, 1.0f, ""}}) {}

    std::string type() const override { return "VOLTA"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "VOLTA", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "register", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LEN", "length", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MUT", "mutate", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ERODE", "erode", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MEM", "protect", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "RANGE", "range", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "STEPS", "steps", 35.0f, 52.0f);
        p.add(Widget::Kind::Knob, "OFST", "offset", 49.0f, 52.0f);
        p.add(Widget::Kind::Knob, "SEED", "seed", 7.0f, 74.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 17.0f, 96.0f);
        p.add(Widget::Kind::Jack, "NEW", "in:new", 29.0f, 96.0f);
        p.add(Widget::Kind::Jack, "MUT", "in:mut_cv", 41.0f, 96.0f);
        p.add(Widget::Kind::Jack, "ERO", "in:ero_cv", 53.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CV", "out:cv", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "CV2", "out:cv2", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PULSE", "out:pulse", 29.0f, 116.0f);
        p.add(Widget::Kind::Jack, "POS", "out:pos", 41.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        generation_ = 0;
        seededWith_ = -1;
        cursor_ = -1;
        phase_ = 0.0;
        prevClock_ = prevReset_ = prevNew_ = 0.0f;
        cv_ = cv2_ = pulse_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const int seed = static_cast<int>(parameterValue("seed") + 0.5f);
        if (seed != seededWith_) { seededWith_ = seed; generation_ = 0; build(); }
        const float rate = parameterValue("rate");
        const int length = clampi(static_cast<int>(std::lround(parameterValue("length"))), 2, kMax);
        const float mutP = parameterValue("mutate"), eroP = parameterValue("erode");
        const float prot = clampf(parameterValue("protect"), 0.0f, 1.0f);
        const float range = parameterValue("range"), offset = parameterValue("offset");
        const int steps = clampi(static_cast<int>(std::lround(parameterValue("steps"))), 1, 32);
        const AudioBlock* clock = inputs[0];
        const AudioBlock* reset = inputs[1];
        const AudioBlock* fresh = inputs[2];
        const AudioBlock* mutCv = inputs[3];
        const AudioBlock* eroCv = inputs[4];
        const double dp = static_cast<double>(rate) / sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            if (fresh) {
                const float v = fresh->at(0, f);
                if (prevNew_ < 0.5f && v >= 0.5f) { ++generation_; build(); }
                prevNew_ = v;
            }
            if (reset) {
                const float v = reset->at(0, f);
                if (prevReset_ < 0.5f && v >= 0.5f) { cursor_ = -1; phase_ = 0.0; }
                prevReset_ = v;
            }
            bool step = false;
            if (clock) {
                const float c = clock->at(0, f);
                if (prevClock_ < 0.5f && c >= 0.5f) step = true;
                prevClock_ = c;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) { phase_ -= 1.0; step = true; }
            }
            if (step) {
                cursor_ = (cursor_ + 1) % length;
                const float mut = clampf(mutP + (mutCv ? mutCv->at(0, f) : 0.0f), 0.0f, 1.0f);
                const float ero = clampf(eroP + (eroCv ? eroCv->at(0, f) : 0.0f), 0.0f, 1.0f);
                emit(cursor_, length, mut, ero, prot, range, steps, offset);
            }
            const float pos = cursor_ < 0 ? 0.0f
                : static_cast<float>(cursor_) / static_cast<float>(length - 1);
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = cv_;
                outputs[1].at(c, f) = cv2_;
                outputs[2].at(c, f) = pulse_;
                outputs[3].at(c, f) = pos;
            }
        }
    }

    // para o visor e os testes: o valor que o passo i tocaria agora
    float stepValue(const int i, const float mut, const float prot) const noexcept {
        return value(i, mut, prot);
    }

private:
    static constexpr int kMax = 16;

    void build() noexcept {
        rng_ = 0x9E3779B97F4A7C15ULL ^ (static_cast<std::uint64_t>(seededWith_) * 0x2545F4914F6CDD1DULL)
             ^ (static_cast<std::uint64_t>(generation_) * 0xBF58476D1CE4E5B9ULL);
        for (int i = 0; i < kMax; ++i) {
            base_[i] = uni();
            alt_[i] = uni();
            mutRank_[i] = uni();
            eroRank_[i] = uni();
            memRank_[i] = uni();
        }
    }
    bool isProtected(const int i, const float prot) const noexcept { return memRank_[i] < prot; }
    float value(const int i, const float mut, const float prot) const noexcept {
        if (!isProtected(i, prot) && mutRank_[i] < mut) return alt_[i];
        return base_[i];
    }

    void emit(const int i, const int length, const float mut, const float ero,
              const float prot, const float range, const int steps, const float offset) noexcept {
        const bool eroded = !isProtected(i, prot) && eroRank_[i] < ero;
        const float v = value(i, mut, prot);
        if (!eroded) {
            float x = (v - 0.5f) * 2.0f * range + offset;
            if (steps >= 2) {
                const float n = static_cast<float>(steps - 1);
                x = std::round((x * 0.5f + 0.5f) * n) / n * 2.0f - 1.0f;
            }
            cv_ = clampf(x, -1.0f, 1.0f);
            // CV2: o passo e os dois seguintes, como o "Volts" do TURING
            const float w = v * 0.6f + value((i + 1) % length, mut, prot) * 0.25f
                          + value((i + 2) % length, mut, prot) * 0.15f;
            cv2_ = clampf((w - 0.5f) * 2.0f * range + offset, -1.0f, 1.0f);
        }
        pulse_ = (!eroded && v >= 0.5f) ? 1.0f : 0.0f;
    }

    float uni() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF) / static_cast<float>(0x1000000);
    }
    static int clampi(const int v, const int lo, const int hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
    static float clampf(const float v, const float lo, const float hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    float base_[kMax] = {}, alt_[kMax] = {}, mutRank_[kMax] = {}, eroRank_[kMax] = {}, memRank_[kMax] = {};
    std::uint64_t rng_ = 1;
    int seededWith_ = -1, generation_ = 0, cursor_ = -1;
    double phase_ = 0.0;
    float sr_ = 48000.0f, prevClock_ = 0.0f, prevReset_ = 0.0f, prevNew_ = 0.0f;
    float cv_ = 0.0f, cv2_ = 0.0f, pulse_ = 0.0f;
};

}  // namespace rasgo::modular
