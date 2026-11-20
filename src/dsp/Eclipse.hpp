#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// ECLIPSE (#99) — dois ciclos que só às vezes se alinham (leitura da
// família, 9/11/2026, aprovada pelo autor: "aprovo todos, continue"; dossiê
// `dossies/99_eclipse.md`).
//
// O céu do Asterismus: a Lua tem dois meses. O SINÓDICO (29,530589 dias) é o
// das fases — lua nova, cheia. O DRACÔNICO (27,212221 dias) é o da volta aos
// NÓS, os dois pontos onde a órbita da Lua cruza o plano da Terra. Só há
// eclipse quando as duas coisas coincidem: lua nova (eclipse do Sol) ou
// cheia (eclipse da Lua) perto de um nó, dentro do LIMITE (0,051 do mês
// dracônico ≈ 18°, o limite que o Asterismus validou). Como os dois meses
// quase se encaixam, os eclipses vêm em temporadas, a cada ~6 meses, e a
// sequência inteira volta depois de 223 meses sinódicos — o SAROS (≈ 242
// dracônicos), conhecido dos babilônios. Um polirritmo astronômico: raro,
// previsível, e que só se repete em escala de séculos.
//
// Cada pulso em CLK é um dia (ou SPEED dias por segundo, sem cabo). SYN e
// DRAC trocam os dois meses: outros céus, outros polirritmos.
//
// Saídas: SUN e MOON (os eclipses), NEW (cada lua nova), NODE (cada passagem
// por um nó), ALIGN (o quanto a lua desta lunação está perto do nó, 0–1) e
// PHASE (a fase da Lua, 0 = nova, 0,5 = cheia).
//
// Desvio Rasgo: TOTAL — o eclipse bem no centro do limite (a menos de um
// terço dele) é TOTAL e o gate dura o dobro; o de borda é parcial.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Eclipse final : public Signal {
public:
    static constexpr double kSynodic = 29.530589;
    static constexpr double kDraconic = 27.212221;

    Eclipse() : Signal(
        {{"clock", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"}},
        {{"sun", PortKind::Control, "gate"},
         {"moon", PortKind::Control, "gate"},
         {"new", PortKind::Control, "gate"},
         {"node", PortKind::Control, "gate"},
         {"align", PortKind::Control, ""},
         {"phase", PortKind::Control, ""}},
        {{"speed", 0.5f, 120.0f, 12.0f, "dias/s"},
         {"limit", 0.01f, 0.15f, 0.051f, ""},
         {"synodic", 10.0f, 60.0f, 29.530589f, "dias"},
         {"draconic", 10.0f, 60.0f, 27.212221f, "dias"},
         {"gate_len", 0.01f, 1.0f, 0.15f, "s"}}) {}

    std::string type() const override { return "ECLIPSE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "ECLIPSE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "eclipse", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "SPEED", "speed", 5.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LIMIT", "limit", 20.5f, 30.0f);
        p.add(Widget::Kind::Knob, "GATE", "gate_len", 36.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SYN", "synodic", 5.0f, 50.0f);
        p.add(Widget::Kind::Knob, "DRAC", "draconic", 20.5f, 50.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 6.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "ALIGN", "out:align", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "PHASE", "out:phase", 44.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SUN", "out:sun", 6.0f, 116.0f);
        p.add(Widget::Kind::Jack, "MOON", "out:moon", 18.0f, 116.0f);
        p.add(Widget::Kind::Jack, "NEW", "out:new", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "NODE", "out:node", 44.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearSky();
    }

    // Distância (fração do mês dracônico) da Lua ao nó mais próximo.
    static double nodeDistance(double draconicPhase) noexcept {
        const double d = draconicPhase - std::floor(draconicPhase);
        const double toNode = std::fmod(d, 0.5);
        return std::min(toNode, 0.5 - toNode);
    }
    double days() const noexcept { return days_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* clock = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;

        const double speed = std::clamp(safe(parameterValue("speed"), 12.0f), 0.5f, 120.0f);
        const double limit = std::clamp(safe(parameterValue("limit"), 0.051f), 0.01f, 0.15f);
        const double syn = std::clamp(safe(parameterValue("synodic"), 29.530589f), 10.0f, 60.0f);
        const double drac = std::clamp(safe(parameterValue("draconic"), 27.212221f), 10.0f, 60.0f);
        const int gateLen = std::max(1, static_cast<int>(
            std::clamp(safe(parameterValue("gate_len"), 0.15f), 0.01f, 1.0f) * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearSky();
                prevReset_ = r;
            }
            double step = 0.0;
            if (clock) {
                const float c = safe(clock->at(0, f), 0.0f);
                if (prevClock_ < 0.5f && c >= 0.5f) step = 1.0;   // um pulso, um dia
                prevClock_ = c;
            } else {
                step = speed / sr_;
            }
            if (step > 0.0) {
                const double before = days_;
                days_ += step;
                const double s0 = before / syn, s1 = days_ / syn;
                const double d1 = days_ / drac;   // a fase dracônica agora
                // lua nova: a fase sinódica passa por um inteiro. A distância ao
                // nó é medida no INSTANTE exato da sizígia (não no dia do
                // pulso): com um pulso por dia, o erro seria de até 1/27 do
                // mês dracônico, comparável ao próprio limite.
                if (std::floor(s1) > std::floor(s0)) {
                    newLeft_ = gateLen;
                    const double nd = nodeDistance(std::floor(s1) * syn / drac);
                    align_ = static_cast<float>(std::clamp(1.0 - nd / 0.25, 0.0, 1.0));
                    if (nd < limit) sunLeft_ = nd < limit / 3.0 ? 2 * gateLen : gateLen;
                }
                // lua cheia: passa por um meio
                if (std::floor(s1 + 0.5) > std::floor(s0 + 0.5)) {
                    const double nd = nodeDistance((std::floor(s1 + 0.5) - 0.5) * syn / drac);
                    if (nd < limit) moonLeft_ = nd < limit / 3.0 ? 2 * gateLen : gateLen;
                }
                // o nó: a fase dracônica passa por um múltiplo de 1/2
                if (std::floor(2.0 * d1) > std::floor(2.0 * before / drac)) nodeLeft_ = gateLen;
                phase_ = static_cast<float>(s1 - std::floor(s1));
            }
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = sunLeft_ > 0 ? 1.0f : 0.0f;
                outputs[1].at(c, f) = moonLeft_ > 0 ? 1.0f : 0.0f;
                outputs[2].at(c, f) = newLeft_ > 0 ? 1.0f : 0.0f;
                outputs[3].at(c, f) = nodeLeft_ > 0 ? 1.0f : 0.0f;
                outputs[4].at(c, f) = align_;
                outputs[5].at(c, f) = phase_;
            }
            if (sunLeft_ > 0) --sunLeft_;
            if (moonLeft_ > 0) --moonLeft_;
            if (newLeft_ > 0) --newLeft_;
            if (nodeLeft_ > 0) --nodeLeft_;
        }
    }

private:
    void clearSky() noexcept {
        // o dia 0 é uma lua nova sobre o nó (sem pulso: nada atravessa o 0);
        // as luas seguintes se afastam do nó e voltam a ele na temporada seguinte
        days_ = 0.0;
        prevClock_ = prevReset_ = 0.0f;
        sunLeft_ = moonLeft_ = newLeft_ = nodeLeft_ = 0;
        align_ = 0.0f;
        phase_ = 0.0f;
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    double days_ = 0.0;
    float prevClock_ = 0.0f, prevReset_ = 0.0f, align_ = 0.0f, phase_ = 0.0f;
    int sunLeft_ = 0, moonLeft_ = 0, newLeft_ = 0, nodeLeft_ = 0;
};

} // namespace rasgo::modular
