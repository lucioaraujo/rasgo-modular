#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// PREDATOR (#97) — predador e presa (leitura da família, 9/11/2026, aprovada
// pelo autor: "aprovo todos, continue"; dossiê `dossies/97_predator.md`).
//
// As equações de Lotka (1925) e Volterra (1926): a presa cresce sozinha e é
// comida; o predador cresce comendo e morre de fome. As duas populações
// giram numa órbita fechada, o predador um quarto de ciclo atrás da presa.
// A integração é a do Aquorbium (`PopulationCycles`, mesmo autor, copiado e
// adaptado): em logaritmos, por Euler simplético — as órbitas não derivam e
// as populações nunca ficam negativas.
//
// Saídas: PREY e PRED (as populações, 0–1, em escala logarítmica), CATCH
// (um pulso a cada captura, quando o predador está em alta), FLEE (um pulso
// no pico da presa: a fuga) e CANON — a linha da presa repetida com um
// atraso que ENCOLHE quando o predador cresce: o cânone que se aperta até o
// stretto, a caça do Aquorbium em música (o diretor ecológico de lá).
//
// Desvio Rasgo:
//  (a) CATCH também ENTRA: um gatilho externo é uma captura — o patch caça;
//  (b) WILD — o ruído do ambiente (semeado) empurra as populações, e a órbita
//      passeia entre ciclos largos e estreitos, como um ecossistema real.
//
// Controle a cada 64 amostras; process() não aloca; determinístico.
namespace rasgo::modular {

class Predator final : public Signal {
public:
    static constexpr int kCanon = 4096;   // passos de controle (~5,5 s a 48 kHz)

    Predator() : Signal(
        {{"catch", PortKind::Control, "trig"},
         {"rate_cv", PortKind::Control, ""}},
        {{"prey", PortKind::Control, ""},
         {"pred", PortKind::Control, ""},
         {"catch", PortKind::Control, "gate"},
         {"flee", PortKind::Control, "gate"},
         {"canon", PortKind::Control, ""}},
        {{"period", 4.0f, 300.0f, 30.0f, "s"},
         {"size", 0.1f, 0.95f, 0.6f, ""},
         {"hunt", 0.0f, 1.0f, 0.4f, ""},
         {"wild", 0.0f, 1.0f, 0.15f, ""},
         {"delay", 0.2f, 5.0f, 2.0f, "s"}}) {}

    std::string type() const override { return "PREDATOR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "PREDATOR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "predator", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "PERIOD", "period", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SIZE", "size", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "HUNT", "hunt", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "WILD", "wild", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DELAY", "delay", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "CTCH", "in:catch", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RATE", "in:rate_cv", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CANON", "out:canon", 44.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PREY", "out:prey", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "PRED", "out:pred", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "CTCH", "out:catch", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "FLEE", "out:flee", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0xB0A7FEEDULL;
        const float size = std::clamp(safe(parameterValue("size"), 0.6f), 0.1f, 0.95f);
        // começa fora do equilíbrio (N = P = 1): o tamanho da órbita
        logPrey_ = std::log(1.0f + 3.0f * size);
        logPred_ = 0.0f;
        counter_ = 0;
        canon_.fill(0.0f);
        head_ = 0;
        prevIn_ = 0.0f;
        prevPrey_ = prevPrevPrey_ = 0.0f;
        catchLeft_ = fleeLeft_ = 0;
        cooldown_ = 0;
    }

    float prey() const noexcept { return std::exp(logPrey_); }
    float predators() const noexcept { return std::exp(logPred_); }
    // a "energia" da órbita de Lotka–Volterra (conservada sem perturbação)
    float invariant() const noexcept {
        return (std::exp(logPrey_) - logPrey_) + (std::exp(logPred_) - logPred_);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const AudioBlock* cin = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* rcv = inputs.size() > 1 ? inputs[1] : nullptr;

        const float periodK = std::clamp(safe(parameterValue("period"), 30.0f), 4.0f, 300.0f);
        const float hunt = std::clamp(safe(parameterValue("hunt"), 0.4f), 0.0f, 1.0f);
        const float wild = std::clamp(safe(parameterValue("wild"), 0.15f), 0.0f, 1.0f);
        const float delayMax = std::clamp(safe(parameterValue("delay"), 2.0f), 0.2f, 5.0f);
        const float size = std::clamp(safe(parameterValue("size"), 0.6f), 0.1f, 0.95f);
        // o tamanho da órbita como alvo do invariante (mínimo 2 no equilíbrio)
        const float s0 = 1.0f + 3.0f * size;
        const float hTarget = (s0 - std::log(s0)) + 1.0f;
        constexpr int kStep = 64;
        const float dt = static_cast<float>(kStep) / sr_;
        const int gateLen = std::max(1, static_cast<int>(0.02f * sr_));
        const int maxSteps = std::min(kCanon - 1, static_cast<int>(delayMax / dt));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            bool captureNow = false;
            if (cin) {
                const float c = safe(cin->at(0, f), 0.0f);
                if (prevIn_ < 0.5f && c >= 0.5f) captureNow = true;
                prevIn_ = c;
            }
            if (++counter_ >= kStep) {
                counter_ = 0;
                float period = periodK;
                if (rcv) period /= std::exp2(std::clamp(safe(rcv->at(0, f), 0.0f), -4.0f, 4.0f));
                const float rate = 6.2831853f / std::max(1.0f, period);
                // Euler simplético em logaritmos (Aquorbium): x com o y antigo, y com o x novo
                logPrey_ += dt * rate * (1.0f - std::exp(logPred_));
                logPred_ += dt * rate * (std::exp(logPrey_) - 1.0f);
                if (wild > 0.0f) {
                    const float k = wild * 0.15f * std::sqrt(dt * rate);
                    logPrey_ += k * (2.0f * unit() - 1.0f);
                    logPred_ += k * (2.0f * unit() - 1.0f);
                }
                // a caça automática: com o predador em alta, capturas de vez em quando
                if (cooldown_ > 0) --cooldown_;
                if (hunt > 0.0f && logPred_ > 0.2f && cooldown_ == 0
                    && unit() < hunt * dt * rate * 6.0f * std::exp(logPred_))
                    captureNow = true;
                if (captureNow) {
                    logPrey_ += std::log(1.0f - 0.06f);
                    logPred_ += std::log(1.0f + 0.04f);
                    catchLeft_ = gateLen;
                    cooldown_ = static_cast<int>(0.15f / dt);
                }
                // SIZE ao vivo: a órbita cresce ou encolhe devagar até o tamanho
                // pedido (afasta ou aproxima as duas populações do equilíbrio)
                {
                    const float h = invariant();
                    const float g = 1.0f + std::clamp(0.4f * dt * rate * (hTarget - h), -0.01f, 0.01f);
                    logPrey_ *= g;
                    logPred_ *= g;
                }
                logPrey_ = std::clamp(logPrey_, -6.0f, 3.0f);
                logPred_ = std::clamp(logPred_, -6.0f, 3.0f);
                // a fuga: o pico da presa (o máximo local)
                if (prevPrey_ > prevPrevPrey_ && prevPrey_ >= logPrey_) fleeLeft_ = gateLen;
                prevPrevPrey_ = prevPrey_;
                prevPrey_ = logPrey_;
                preyOut_ = std::clamp((logPrey_ + 3.0f) / 6.0f, 0.0f, 1.0f);
                predOut_ = std::clamp((logPred_ + 3.0f) / 6.0f, 0.0f, 1.0f);
                // o cânone: a presa repetida com atraso que encolhe com o predador
                canon_[static_cast<std::size_t>(head_)] = preyOut_;
                const float closeness = std::clamp(predOut_ * 1.4f - 0.2f, 0.0f, 1.0f);
                const int lag = std::max(1, static_cast<int>(static_cast<float>(maxSteps) * (1.0f - 0.9f * closeness)));
                canonOut_ = canon_[static_cast<std::size_t>((head_ - lag + kCanon) % kCanon)];
                head_ = (head_ + 1) % kCanon;
            }
            const float cg = catchLeft_ > 0 ? 1.0f : 0.0f;
            const float fg = fleeLeft_ > 0 ? 1.0f : 0.0f;
            if (catchLeft_ > 0) --catchLeft_;
            if (fleeLeft_ > 0) --fleeLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = preyOut_;
                outputs[1].at(c, f) = predOut_;
                outputs[2].at(c, f) = cg;
                outputs[3].at(c, f) = fg;
                outputs[4].at(c, f) = canonOut_;
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
    std::uint64_t rng_ = 0xB0A7FEEDULL;
    float logPrey_ = 0.5f, logPred_ = 0.0f;
    int counter_ = 0, head_ = 0, catchLeft_ = 0, fleeLeft_ = 0, cooldown_ = 0;
    std::array<float, kCanon> canon_{};
    float prevIn_ = 0.0f, prevPrey_ = 0.0f, prevPrevPrey_ = 0.0f;
    float preyOut_ = 0.5f, predOut_ = 0.5f, canonOut_ = 0.5f;
};

} // namespace rasgo::modular
