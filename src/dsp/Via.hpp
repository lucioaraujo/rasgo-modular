#pragma once

#include "core/SignalGraph.hpp"
#include "dsp/Oversampler.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// VIA (#83) — o cabo com matéria (2026-11-11; dossiê `dossies/83_via.md`;
// estudo da família §4.5 e §4.5c, aprovado; "avance em todos eles").
//
// Prior art do autor: as PORTAS do Antitotem (`CmosVoice::feedbackSample`,
// AGPL, mesmo autor) — passagens de retorno combináveis cuja média é o
// "retorno composto". Desvio obrigatório (o próximo passo que o próprio
// autor deixou escrito): aqui as portas servem a QUALQUER cabo do patch, não
// só ao retorno de uma voz. E uma porta a mais, do estudo: o ALTO-FALANTE.
//
// Sete portas (chaves); as abertas fazem a MÉDIA:
//   RES    — o resistor: o sinal, atenuado;
//   DIODO  — retificado: |x|·2 − 1 (só o "lado de cima", com direção);
//   CAP    — o estado do capacitor: retém e solta (o caminho LEMBRA; MEM é o
//            tempo de carga, 1 ms – 2 s);
//   PULSO  — só a borda: o sinal virado limiar (tanh(20x), sem ser um degrau
//            seco);
//   TRANS  — tanh assimétrico com o capacitor (o transistor);
//   REFLUX — a diferença entre o sinal e a memória do capacitor (×0,82), "uma
//            corrente que retorna alterada";
//   FALANTE — um corpo mecânico ressonante (2ª ordem em CORPO, 40–400 Hz) que
//            satura: o cone que bate no limite.
// As portas rodam a 2× (`Oversampler2x`): diodo e pulso geram harmônicos.
// Nenhuma porta aberta = silêncio. Saídas: OUT (a média, × NÍVEL) e CAP (o
// estado do capacitor, como CV). Determinístico, sem alocação.
namespace rasgo::modular {

class Via final : public Signal {
public:
    Via() : Signal(
        {{"in", PortKind::Audio, ""},
         {"memory_cv", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""},
         {"cap", PortKind::Control, ""}},
        {{"res", 0.0f, 1.0f, 1.0f, ""},
         {"diode", 0.0f, 1.0f, 0.0f, ""},
         {"capacitor", 0.0f, 1.0f, 0.0f, ""},
         {"pulse", 0.0f, 1.0f, 0.0f, ""},
         {"transistor", 0.0f, 1.0f, 0.0f, ""},
         {"reflux", 0.0f, 1.0f, 0.0f, ""},
         {"speaker", 0.0f, 1.0f, 0.0f, ""},
         {"memory", 0.001f, 2.0f, 0.05f, "s"},
         {"body", 40.0f, 400.0f, 110.0f, "Hz"},
         {"level", 0.0f, 1.0f, 0.8f, ""}}) {}

    std::string type() const override { return "VIA"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "VIA", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Toggle, "RES", "res", 5.0f, 12.0f);
        p.add(Widget::Kind::Toggle, "DIODO", "diode", 26.0f, 12.0f);
        p.add(Widget::Kind::Toggle, "CAP", "capacitor", 5.0f, 21.0f);
        p.add(Widget::Kind::Toggle, "PULSO", "pulse", 26.0f, 21.0f);
        p.add(Widget::Kind::Toggle, "TRANS", "transistor", 5.0f, 30.0f);
        p.add(Widget::Kind::Toggle, "REFLX", "reflux", 26.0f, 30.0f);
        p.add(Widget::Kind::Toggle, "FALANTE", "speaker", 5.0f, 39.0f);
        p.add(Widget::Kind::Knob, "MEM", "memory", 8.0f, 52.0f);
        p.add(Widget::Kind::Knob, "CORPO", "body", 23.0f, 52.0f);
        p.add(Widget::Kind::Knob, "NIVEL", "level", 38.0f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "MEM", "in:memory_cv", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "CAP", "out:cap", 19.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        for (auto& c : ch_) c = Chan{};
        os_.reset();
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* memCv = inputs.size() > 1 ? inputs[1] : nullptr;
        const bool open[7] = {on("res"), on("diode"), on("capacitor"), on("pulse"),
                              on("transistor"), on("reflux"), on("speaker")};
        int nOpen = 0;
        for (const bool b : open) nOpen += b ? 1 : 0;
        const float memP = std::clamp(safe(parameterValue("memory"), 0.05f), 0.001f, 2.0f);
        const float body = std::clamp(safe(parameterValue("body"), 110.0f), 40.0f, 400.0f);
        const float level = std::clamp(safe(parameterValue("level"), 0.8f), 0.0f, 1.0f);
        const float osRate = 2.0f * sr_;
        // FALANTE: ressonador de 2ª ordem (biquad passa-banda, Q ~4) a 2×
        const float w0 = 6.2831853f * body / osRate;
        const float alpha = std::sin(w0) / (2.0f * 4.0f);
        const float a0 = 1.0f + alpha;
        const float b0 = alpha / a0, a1 = -2.0f * std::cos(w0) / a0, a2 = (1.0f - alpha) / a0;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = std::min(outputs[0].channels(), AudioBlock::maxChannels);
        for (std::size_t f = 0; f < frames; ++f) {
            float mem = memP;
            if (memCv) mem = std::clamp(mem * std::exp2(4.0f * safe(memCv->at(0, f), 0.0f)), 0.001f, 4.0f);
            const float k = 1.0f - std::exp(-1.0f / (mem * osRate));   // carga do capacitor a 2×
            for (std::size_t c = 0; c < channels; ++c) {
                float x = in ? safe(in->at(in->channels() > c ? c : 0, f), 0.0f) : 0.0f;
                x = std::clamp(x, -4.0f, 4.0f);
                Chan& s = ch_[c];
                const float y = os_.process(c, x, [&](const float v) noexcept {
                    s.cap += (v - s.cap) * k;
                    float sum = 0.0f;
                    if (open[0]) sum += v;
                    if (open[1]) sum += std::fabs(v) * 2.0f - 1.0f;
                    if (open[2]) sum += s.cap;
                    if (open[3]) sum += std::tanh(v * 20.0f);
                    if (open[4]) sum += std::tanh(v * 2.2f + s.cap * 0.42f);
                    if (open[5]) sum += v - s.cap * 0.82f;
                    if (open[6]) {
                        const float r = b0 * (v - s.x2) - a1 * s.y1 - a2 * s.y2;
                        s.x2 = s.x1; s.x1 = v; s.y2 = s.y1; s.y1 = r;
                        sum += std::tanh(r * 3.0f);   // o cone bate no limite
                    }
                    return nOpen > 0 ? sum / static_cast<float>(nOpen) : 0.0f;
                });
                // DIODO com nada na entrada daria −1 constante: tira o DC
                // pela metade lenta (o acoplamento de saída)
                s.dc += (y - s.dc) * 0.0005f;
                const float yo = (open[1] || open[3] ? y - s.dc : y) * level;
                outputs[0].at(c, f) = std::isfinite(yo) ? std::clamp(yo, -2.0f, 2.0f) : 0.0f;
                outputs[1].at(c, f) = s.cap;
            }
        }
    }

private:
    struct Chan { float cap = 0.0f, x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f, dc = 0.0f; };
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    bool on(const char* id) const { return parameterValue(id) >= 0.5f; }

    float sr_ = 48000.0f;
    std::array<Chan, AudioBlock::maxChannels> ch_{};
    Oversampler2x os_;
};

}  // namespace rasgo::modular
