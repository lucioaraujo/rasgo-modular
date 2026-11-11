#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>

// COMPARADOR (#80) — o limiar como acontecimento (2026-11-11; dossiê
// `dossies/80_comparador.md`; estudo da família §4.5, aprovado; "pode
// avançar nos módulos restantes").
//
// Dois princípios lidos no acervo do Antitotem (referências creditadas de
// princípio; nenhum esquema, valor ou PCB copiado):
//  - o MIXER POLARIZADOR (phono, 2010): um controle cujo CENTRO é zero —
//    cada entrada pode somar, opor-se ou anular-se;
//  - o MIXER-COMPARADOR (R. Brewster, 2002): MIX, INV e COMP — "o
//    comparador converte uma relação contínua em decisão/evento; não é
//    apenas um amplificador de saída".
//
// MIX = A·PA + B·PB + OFS (PA e PB de −1 a +1; zero no centro).
// INV = −MIX. COMP = gate quando MIX passa do LIM (com histerese HIST:
// sobe acima de LIM + HIST/2, desce abaixo de LIM − HIST/2 — o limiar não
// tremula com ruído). CRUZ = pulso a cada troca de COMP (as duas bordas).
// A entrada LIM soma ao knob: o limiar pode andar.
// Determinístico, sem estado além da histerese.
namespace rasgo::modular {

class Comparador final : public Signal {
public:
    Comparador() : Signal(
        {{"a", PortKind::Audio, ""},
         {"b", PortKind::Audio, ""},
         {"threshold_cv", PortKind::Control, ""}},
        {{"mix", PortKind::Audio, ""},
         {"inv", PortKind::Audio, ""},
         {"comp", PortKind::Control, "gate"},
         {"cross", PortKind::Control, "trig"}},
        {{"pol_a", -1.0f, 1.0f, 1.0f, ""},
         {"pol_b", -1.0f, 1.0f, -1.0f, ""},
         {"offset", -1.0f, 1.0f, 0.0f, ""},
         {"threshold", -1.0f, 1.0f, 0.0f, ""},
         {"hysteresis", 0.0f, 0.5f, 0.05f, ""}}) {}

    std::string type() const override { return "COMPARADOR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "COMPARADOR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Knob, "A", "pol_a", 6.0f, 14.0f);
        p.add(Widget::Kind::Knob, "B", "pol_b", 24.0f, 14.0f);
        p.add(Widget::Kind::Knob, "OFS", "offset", 6.0f, 36.0f);
        p.add(Widget::Kind::Knob, "LIM", "threshold", 24.0f, 36.0f);
        p.add(Widget::Kind::Knob, "HIST", "hysteresis", 15.0f, 58.0f);
        p.add(Widget::Kind::Jack, "A", "in:a", 6.0f, 88.0f);
        p.add(Widget::Kind::Jack, "B", "in:b", 18.0f, 88.0f);
        p.add(Widget::Kind::Jack, "LIM", "in:threshold_cv", 30.0f, 88.0f);
        p.add(Widget::Kind::Jack, "MIX", "out:mix", 6.0f, 104.0f);
        p.add(Widget::Kind::Jack, "INV", "out:inv", 18.0f, 104.0f);
        p.add(Widget::Kind::Jack, "COMP", "out:comp", 30.0f, 104.0f);
        p.add(Widget::Kind::Jack, "CRUZ", "out:cross", 18.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        high_ = false;
        crossLeft_ = 0;
        smooth_ = 1.0f - std::exp(-1.0f / (0.003f * sr_));   // 3 ms nos knobs
        primed_ = false;
    }

    bool high() const noexcept { return high_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* a = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* b = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* lcv = inputs.size() > 2 ? inputs[2] : nullptr;
        const float paT = std::clamp(safe(parameterValue("pol_a"), 1.0f), -1.0f, 1.0f);
        const float pbT = std::clamp(safe(parameterValue("pol_b"), -1.0f), -1.0f, 1.0f);
        const float ofT = std::clamp(safe(parameterValue("offset"), 0.0f), -1.0f, 1.0f);
        const float limK = std::clamp(safe(parameterValue("threshold"), 0.0f), -1.0f, 1.0f);
        const float hyst = std::clamp(safe(parameterValue("hysteresis"), 0.05f), 0.0f, 0.5f);
        if (!primed_) { pa_ = paT; pb_ = pbT; of_ = ofT; primed_ = true; }
        const int pulse = std::max(1, static_cast<int>(0.005f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            pa_ += (paT - pa_) * smooth_;
            pb_ += (pbT - pb_) * smooth_;
            of_ += (ofT - of_) * smooth_;
            const float lim = limK + (lcv ? safe(lcv->at(0, f), 0.0f) : 0.0f);
            // a decisão usa o canal 0 (mono); o MIX sai por canal
            const float x0 = (a ? safe(a->at(0, f), 0.0f) : 0.0f) * pa_
                           + (b ? safe(b->at(0, f), 0.0f) : 0.0f) * pb_ + of_;
            const bool was = high_;
            if (!high_ && x0 > lim + 0.5f * hyst) high_ = true;
            else if (high_ && x0 < lim - 0.5f * hyst) high_ = false;
            if (high_ != was) crossLeft_ = pulse;
            const float comp = high_ ? 1.0f : 0.0f;
            const float cross = crossLeft_ > 0 ? 1.0f : 0.0f;
            if (crossLeft_ > 0) --crossLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                const float ac = a ? safe(a->at(a->channels() > c ? c : 0, f), 0.0f) : 0.0f;
                const float bc = b ? safe(b->at(b->channels() > c ? c : 0, f), 0.0f) : 0.0f;
                const float mix = std::clamp(ac * pa_ + bc * pb_ + of_, -4.0f, 4.0f);
                outputs[0].at(c, f) = mix;
                outputs[1].at(c, f) = -mix;
                outputs[2].at(c, f) = comp;
                outputs[3].at(c, f) = cross;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    float sr_ = 48000.0f;
    bool high_ = false;
    int crossLeft_ = 0;
    float smooth_ = 1.0f;
    bool primed_ = false;
    float pa_ = 1.0f, pb_ = -1.0f, of_ = 0.0f;
};

}  // namespace rasgo::modular
