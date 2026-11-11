#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>

// SUPPLY (#88) — a alimentação variável, como módulo com cabos (2026-11-11;
// dossiê `dossies/88_supply.md`; estudo da família §4.5c, item 2: "a decidir
// com o autor; não implementar sem conversa" — decisão do autor: "módulo com
// cabos"). Conceito da ENERGIA do Antitotem (mesmo autor): "um só controle
// que, baixo, desacelera o clock, reduz o corpo e faz os osciladores derivarem
// uns dos outros (`railWander`); alto, devolve presença" — "uma interpretação
// musical segura, não um modelo de alimentação elétrica real". Aqui, nada é
// invisível: o SUPPLY só age onde há cabo (regra do Antitotem).
//
// ENERGIA dá a tensão de alimentação. A CARGA (o som do patch, ex.: o L+R do
// MIXER) a faz CEDER, como uma bateria sob esforço: quanto mais o patch soa,
// mais o trilho afunda (na medida de QUEDA, com a demora de TEMPO).
// Saídas:
//   TRILHO — a tensão que sobra (0–1): ao nível de um VCA, ao BPM, ao corte;
//   D1–D4  — quatro derivas (V/oct, ±60 cents no máximo) que só aparecem
//            quando o trilho está baixo: ligadas na afinação de vozes
//            diferentes, elas as fazem DESENCONTRAR (o railWander); com o
//            trilho cheio, voltam juntas; DERIVA é o quanto;
//   FALHA  — um pulso quando o trilho cai abaixo de 0,25 (o apagão), outro
//            quando volta acima de 0,35.
// Determinístico (derivas por senoides incomensuráveis), sem alocação.
namespace rasgo::modular {

class Supply final : public Signal {
public:
    Supply() : Signal(
        {{"load", PortKind::Audio, ""},
         {"energy_cv", PortKind::Control, ""}},
        {{"rail", PortKind::Control, ""},
         {"d1", PortKind::Control, "v/oct"},
         {"d2", PortKind::Control, "v/oct"},
         {"d3", PortKind::Control, "v/oct"},
         {"d4", PortKind::Control, "v/oct"},
         {"fail", PortKind::Control, "trig"}},
        {{"energy", 0.0f, 1.0f, 0.7f, ""},
         {"sag", 0.0f, 1.0f, 0.5f, ""},
         {"time", 0.05f, 5.0f, 0.8f, "s"},
         {"wander", 0.0f, 1.0f, 0.5f, ""}}) {}

    std::string type() const override { return "SUPPLY"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "SUPPLY", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "supply", "", 2.5f, 6.0f, 35.6f);
        p.add(Widget::Kind::Knob, "ENERG", "energy", 6.0f, 30.0f);
        p.add(Widget::Kind::Knob, "QUEDA", "sag", 24.0f, 30.0f);
        p.add(Widget::Kind::Knob, "TEMPO", "time", 6.0f, 52.0f);
        p.add(Widget::Kind::Knob, "DERIVA", "wander", 24.0f, 52.0f);
        p.add(Widget::Kind::Jack, "CARGA", "in:load", 7.0f, 82.0f);
        p.add(Widget::Kind::Jack, "ENERG", "in:energy_cv", 21.0f, 82.0f);
        p.add(Widget::Kind::Jack, "TRILHO", "out:rail", 7.0f, 98.0f);
        p.add(Widget::Kind::Jack, "FALHA", "out:fail", 21.0f, 98.0f);
        p.add(Widget::Kind::Jack, "D1", "out:d1", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "D2", "out:d2", 14.0f, 116.0f);
        p.add(Widget::Kind::Jack, "D3", "out:d3", 23.0f, 116.0f);
        p.add(Widget::Kind::Jack, "D4", "out:d4", 32.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        load_ = 0.0f;
        rail_ = std::clamp(parameterValue("energy"), 0.0f, 1.0f);
        down_ = false;
        failLeft_ = 0;
        t_ = 0.0;
    }

    float rail() const noexcept { return rail_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* load = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* ecv = inputs.size() > 1 ? inputs[1] : nullptr;
        const float energyK = knob("energy", 0.7f);
        const float sag = knob("sag", 0.5f);
        const float time = std::clamp(safe(parameterValue("time"), 0.8f), 0.05f, 5.0f);
        const float wander = knob("wander", 0.5f);
        const float loadCoef = 1.0f - std::exp(-1.0f / (0.05f * sr_));   // seguidor 50 ms
        const float railCoef = 1.0f - std::exp(-1.0f / (time * sr_));
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));
        const double dt = 1.0 / sr_;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            t_ += dt;
            // a carga: o nível do som que puxa (|x| suavizado, ~1 = cheio)
            const float x = load ? std::fabs(safe(load->at(0, f), 0.0f)) : 0.0f;
            load_ += (std::min(x * 2.0f, 1.0f) - load_) * loadCoef;
            const float energy = std::clamp(energyK + (ecv ? safe(ecv->at(0, f), 0.0f) : 0.0f), 0.0f, 1.0f);
            const float target = energy * (1.0f - sag * load_);
            rail_ += (target - rail_) * railCoef;
            if (!down_ && rail_ < 0.25f) { down_ = true; failLeft_ = pulse; }
            else if (down_ && rail_ > 0.35f) { down_ = false; failLeft_ = pulse; }
            // as derivas só aparecem com o trilho baixo (até ±60 cents)
            const float amt = wander * (1.0f - rail_) * 0.05f;
            const float d[4] = {
                amt * static_cast<float>(std::sin(t_ * 0.31) * 0.7 + std::sin(t_ * 0.113) * 0.3),
                amt * static_cast<float>(std::sin(t_ * 0.27 + 1.3) * 0.7 + std::sin(t_ * 0.097 + 0.5) * 0.3),
                amt * static_cast<float>(std::sin(t_ * 0.23 + 2.6) * 0.7 + std::sin(t_ * 0.131 + 2.0) * 0.3),
                amt * static_cast<float>(std::sin(t_ * 0.37 + 4.1) * 0.7 + std::sin(t_ * 0.083 + 3.3) * 0.3)};
            const float fl = failLeft_ > 0 ? 1.0f : 0.0f;
            if (failLeft_ > 0) --failLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = rail_;
                for (int k = 0; k < 4; ++k) outputs[static_cast<std::size_t>(1 + k)].at(c, f) = d[k];
                outputs[5].at(c, f) = fl;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    float knob(const char* id, const float fb) const noexcept {
        return std::clamp(safe(parameterValue(id), fb), 0.0f, 1.0f);
    }

    float sr_ = 48000.0f;
    float load_ = 0.0f, rail_ = 0.7f;
    bool down_ = false;
    int failLeft_ = 0;
    double t_ = 0.0;
};

}  // namespace rasgo::modular
