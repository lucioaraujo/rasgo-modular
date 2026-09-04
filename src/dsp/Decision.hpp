#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// DECISION — decisão / distribuição de probabilidade (Módulo 4)
// ============================================================================
//
// Aleatoriedade DOMADA: decide (gate de Bernoulli), sorteia (CV com forma
// de distribuição configurável) e lembra (laço travável "déjà-vu"). É o
// coração generativo de um patch - o que muda, quando muda, quanto muda.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/04_decisao.md`.
//
// Fontes ESTUDADAS (comportamento, não código):
//   - Mutable Branches (STM32, MIT) - portão de Bernoulli enviesado;
//   - Mutable Marbles - aleatoriedade estruturada, déjà-vu (memória
//     circular relida), spread, quantização;
//   - Frap Tools SAPÈL (hardware) - distribuição contínua uniforme ->
//     gaussiana num controle, dois canais com S&H e slew;
//   - Bastl Déjà Vu / mylar RANDOM8 - máquina de estado da CV
//     (aleatório -> looping -> travado);
//   - central limit (soma de uniformes -> normal) - sino barato, sem
//     log/cos, RT-seguro.
//
// Avanço por EVENTO: `trigger` externo (borda de subida) OU, quando
// `trigger` não está conectado, um relógio interno em `rate`.
//
// Determinístico: xorshift64* semeado em prepare() (sem entrada de seed no
// marco 1). Dois renders com os mesmos parâmetros são byte-idênticos.

namespace rasgo::modular {

class Decision final : public Signal {
public:
    Decision()
        : Signal(
              {{"trigger", PortKind::Control, "trig"},
               {"bias_mod", PortKind::Control, ""},
               {"spread_mod", PortKind::Control, ""}},
              {{"x", PortKind::Audio, ""},
               {"y", PortKind::Audio, ""},
               {"gate", PortKind::Control, "gate"}},
              {{"rate", 0.01f, 50.0f, 2.0f, "Hz"},
               {"bias", 0.0f, 1.0f, 0.5f, ""},
               {"spread", 0.0f, 1.0f, 0.6f, ""},
               {"shape", 0.0f, 1.0f, 0.0f, ""},
               {"steps", 1.0f, 32.0f, 1.0f, ""},
               {"slew", 0.0f, 1.0f, 0.0f, ""},
               {"dejavu", 0.0f, 1.0f, 0.0f, ""},
               {"loop_length", 1.0f, 16.0f, 8.0f, ""}}) {}

    std::string type() const override { return "DECISION"; }

    // Layout: 14 HP.
    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "DECISION", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "history", "", 2.5f, 8.0f, 55.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "BIAS", "bias", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SPRD", "spread", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SHAPE", "shape", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "STEPS", "steps", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "SLEW", "slew", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "DEJA", "dejavu", 35.0f, 52.0f);
        p.add(Widget::Kind::Knob, "LOOP", "loop_length", 49.0f, 52.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trigger", 5.0f, 100.0f);
        p.add(Widget::Kind::Jack, "BIAS", "in:bias_mod", 17.0f, 100.0f);
        p.add(Widget::Kind::Jack, "SPRD", "in:spread_mod", 29.0f, 100.0f);
        p.add(Widget::Kind::Jack, "X", "out:x", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "Y", "out:y", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 29.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        phase_ = 0.0;
        prevTrigger_ = 0.0f;
        rngState_ = 0x853C49E6748FEA9BULL;
        head_ = 0;
        filled_ = 0;
        for (int i = 0; i < kHist; ++i)
            xHist_[i] = yHist_[i] = gHist_[i] = 0.0f;
        x_ = y_ = 0.0f;
        gate_ = 0.0f;
        xTarget_ = yTarget_ = 0.0f;
        // um sorteio inicial pra saída não ficar presa em zero até o 1º passo
        doStep(0.5f, parameterValue("spread"));
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& xOut = outputs[0];
        AudioBlock& yOut = outputs[1];
        AudioBlock& gOut = outputs[2];
        const std::size_t frames = xOut.frames();
        const std::size_t channels = xOut.channels();

        const float rate = parameterValue("rate");
        const float biasParam = parameterValue("bias");
        const float spreadParam = parameterValue("spread");
        const float slew = parameterValue("slew");

        const AudioBlock* trigIn = inputs[0];
        const AudioBlock* biasMod = inputs[1];
        const AudioBlock* spreadMod = inputs[2];
        const bool externalClock = (trigIn != nullptr);

        const float slewCoeff = slew <= 0.0f
            ? 1.0f
            : 1.0f - std::exp(-1.0f
                              / std::max(1.0f, slew * 0.5f * sampleRate_));
        const double dp = static_cast<double>(rate) / sampleRate_;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float effBias = clampf(
                biasParam + (biasMod ? biasMod->at(0, frame) : 0.0f), 0.0f, 1.0f);
            const float effSpread = clampf(
                spreadParam + (spreadMod ? spreadMod->at(0, frame) : 0.0f),
                0.0f, 1.0f);

            bool step = false;
            if (externalClock) {
                const float t = trigIn->at(0, frame);
                if (prevTrigger_ < 0.5f && t >= 0.5f)
                    step = true;
                prevTrigger_ = t;
            } else {
                phase_ += dp;
                if (phase_ >= 1.0) {
                    phase_ -= 1.0;
                    step = true;
                }
            }
            if (step)
                doStep(effBias, effSpread);

            x_ += (xTarget_ - x_) * slewCoeff;
            y_ += (yTarget_ - y_) * slewCoeff;

            for (std::size_t channel = 0; channel < channels; ++channel) {
                xOut.at(channel, frame) = x_;
                yOut.at(channel, frame) = y_;
                gOut.at(channel, frame) = gate_;
            }
        }
    }

private:
    static constexpr int kHist = 16;

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // xorshift64* -> uniforme em [0,1)
    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    // Um valor bipolar [-1,1] com a distribuição e a quantização atuais.
    float drawValue(const float effSpread) noexcept {
        const float shape = parameterValue("shape");
        float u = uniform01();
        if (shape > 0.0f) {
            // média de 4 uniformes -> sino (central limit). Mesma média 0,5.
            const float bell = (uniform01() + uniform01() + uniform01()
                                + uniform01()) * 0.25f;
            u = (1.0f - shape) * u + shape * bell;
        }
        float c = (u - 0.5f) * 2.0f * effSpread;
        const int steps =
            static_cast<int>(std::lround(parameterValue("steps")));
        if (steps >= 2) {
            const float n = static_cast<float>(steps - 1);
            c = std::round((c * 0.5f + 0.5f) * n) / n * 2.0f - 1.0f;
        }
        return clampf(c, -1.0f, 1.0f);
    }

    // Um passo: relê da memória (déjà-vu) ou sorteia novo. Sempre reescreve
    // na memória circular.
    void doStep(const float effBias, const float effSpread) noexcept {
        int loopLen = static_cast<int>(std::lround(parameterValue("loop_length")));
        if (loopLen < 1) loopLen = 1;
        if (loopLen > kHist) loopLen = kHist;
        const float dejavu = parameterValue("dejavu");

        if (filled_ >= loopLen && uniform01() < dejavu) {
            const int idx = ((head_ - loopLen) % kHist + kHist) % kHist;
            xTarget_ = xHist_[idx];
            yTarget_ = yHist_[idx];
            gate_ = gHist_[idx];
        } else {
            xTarget_ = drawValue(effSpread);
            yTarget_ = drawValue(effSpread);
            gate_ = (uniform01() < effBias) ? 1.0f : 0.0f;
        }

        xHist_[head_] = xTarget_;
        yHist_[head_] = yTarget_;
        gHist_[head_] = gate_;
        head_ = (head_ + 1) % kHist;
        if (filled_ < kHist)
            ++filled_;
    }

    double phase_ = 0.0;
    float prevTrigger_ = 0.0f;
    std::uint64_t rngState_ = 0x853C49E6748FEA9BULL;

    float xHist_[kHist] = {};
    float yHist_[kHist] = {};
    float gHist_[kHist] = {};
    int head_ = 0;
    int filled_ = 0;

    float x_ = 0.0f;
    float y_ = 0.0f;
    float gate_ = 0.0f;
    float xTarget_ = 0.0f;
    float yTarget_ = 0.0f;
};

}  // namespace rasgo::modular
