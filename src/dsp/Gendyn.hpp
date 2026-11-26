#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// GENDYN (#101) — a síntese estocástica dinâmica de Xenakis (leitura da
// família, 9/11/2026, aprovada pelo autor: "aprovo todos, continue"; dossiê
// `dossies/101_gendyn.md`).
//
// Iannis Xenakis (Formalized Music; GENDY3, 1991): a onda é um polígono de N
// pontos (POINTS), percorrido em ordem. Cada vez que passa por um ponto, a
// AMPLITUDE dele e a DURAÇÃO até o próximo andam um passo ao acaso (AMP e
// DUR) — um passeio aleatório, não um sorteio novo — refletido nas bordas
// (as "barreiras elásticas" de Xenakis), para nunca grudar nelas. A onda não
// se repete e não foge: um som que vive. O passo segue uma de quatro
// distribuições (DIST: uniforme, gaussiana, Cauchy — os saltos raros e
// grandes que Xenakis preferia — e logística).
//
// Copiado e adaptado do Rasgo Synth Studio (`GendynOscillator.hpp`, mesmo
// autor): aqui sem `std::random` (gerador próprio, sem alocação) e com uma
// altura central.
//
// Desvio Rasgo:
//  (a) FREQ e PTCH (V/oct) — a duração total do polígono é puxada para o
//      período da altura pedida (como no GENDY3 com alvo), e BAND diz o quanto
//      ela pode fugir: BAND 0, uma nota afinada que respira; BAND 1, ruído;
//  (b) FRZ — com o gate alto, o passeio para e a onda se repete: o retrato
//      congelado de um instante.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Gendyn final : public Signal {
public:
    static constexpr int kMax = 16;

    Gendyn() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"freeze", PortKind::Control, "gate"},
         {"amp_cv", PortKind::Control, ""}},
        {{"out", PortKind::Audio, ""}},
        {{"points", 3.0f, 16.0f, 8.0f, ""},
         {"freq", 20.0f, 2000.0f, 110.0f, "Hz"},
         {"band", 0.0f, 1.0f, 0.3f, ""},
         {"amp_step", 0.0f, 1.0f, 0.12f, ""},
         {"dur_step", 0.0f, 1.0f, 0.1f, ""},
         {"dist", 0.0f, 3.0f, 2.0f, ""},
         {"level", 0.0f, 1.0f, 0.5f, ""}}) {}

    std::string type() const override { return "GENDYN"; }

    Panel panel() const override {
        Panel p;
        // Arquétipo das FONTES (autor, 26/11/2026): o knob herói no centro;
        // os pontos e a banda nos lados.
        p.hp = 10;
        p.add(Widget::Kind::Label, "GENDYN", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "gendyn", "", 2.5f, 6.0f, 45.80f);
        p.add(Widget::Kind::Knob, "PTS", "points", 6.40f, 31.0f);
        p.add(Widget::Kind::Knob, "FREQ", "freq", 18.40f, 27.0f, 14.0f);   // herói
        p.add(Widget::Kind::Knob, "BAND", "band", 36.40f, 31.0f);
        p.add(Widget::Kind::Knob, "AMP", "amp_step", 7.40f, 50.0f);
        p.add(Widget::Kind::Knob, "DUR", "dur_step", 21.40f, 50.0f);
        p.add(Widget::Kind::Knob, "DIST", "dist", 35.40f, 50.0f);
        p.add(Widget::Kind::Knob, "LEVEL", "level", 21.40f, 66.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 11.40f, 92.0f);
        p.add(Widget::Kind::Jack, "FRZ", "in:freeze", 25.40f, 92.0f);
        p.add(Widget::Kind::Jack, "AMP", "in:amp_cv", 39.40f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 25.40f, 110.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        rng_ = 0x6E4D7A11ULL;
        for (int i = 0; i < kMax; ++i) {
            amps_[static_cast<std::size_t>(i)] = 2.0f * unit() - 1.0f;
            durs_[static_cast<std::size_t>(i)] = 1.0f;   // relativo: começa igual em todos
        }
        index_ = 0;
        into_ = 0.0f;
        dc_ = 0.0f;
    }

    float amplitude(int i) const noexcept { return amps_[static_cast<std::size_t>(std::clamp(i, 0, kMax - 1))]; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.empty()) return;
        const AudioBlock* pitch = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* freeze = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* acv = inputs.size() > 2 ? inputs[2] : nullptr;

        const int n = static_cast<int>(std::lround(std::clamp(safe(parameterValue("points"), 8.0f), 3.0f, 16.0f)));
        const float freqK = std::clamp(safe(parameterValue("freq"), 110.0f), 20.0f, 2000.0f);
        const float band = std::clamp(safe(parameterValue("band"), 0.3f), 0.0f, 1.0f);
        const float ampStepK = std::clamp(safe(parameterValue("amp_step"), 0.12f), 0.0f, 1.0f);
        const float durStep = std::clamp(safe(parameterValue("dur_step"), 0.1f), 0.0f, 1.0f);
        const int dist = static_cast<int>(std::lround(std::clamp(safe(parameterValue("dist"), 2.0f), 0.0f, 3.0f)));
        const float level = std::clamp(safe(parameterValue("level"), 0.5f), 0.0f, 1.0f);
        // a barreira das durações (relativas): BAND 0 → de 0,5× a 2× (o desenho
        // ainda respira), BAND 1 → de 0,1× a 4×
        const float dLo = 0.5f - 0.4f * band, dHi = 2.0f + 2.0f * band;
        const float dcK = 1.0f - std::exp(-6.2831853f * 8.0f / sr_);

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            float hz = freqK;
            if (pitch) hz *= std::exp2(std::clamp(safe(pitch->at(0, f), 0.0f), -6.0f, 6.0f));
            hz = std::clamp(hz, 5.0f, 0.25f * sr_);
            const bool frozen = freeze && safe(freeze->at(0, f), 0.0f) >= 0.5f;
            float ampStep = ampStepK;
            if (acv) ampStep = std::clamp(ampStep + safe(acv->at(0, f), 0.0f), 0.0f, 1.0f);
            if (index_ >= n) index_ = 0;
            // a duração do segmento em amostras: o período dividido entre os
            // pontos. A soma das durações é puxada para o período da altura
            // pedida com força (1 − BAND): BAND 0, a altura fica presa (só o
            // desenho da onda passeia); BAND 1, o período passeia livre.
            float sum = 0.0f;
            for (int i = 0; i < n; ++i) sum += durs_[static_cast<std::size_t>(i)];
            const float pull = std::pow(static_cast<float>(n) / std::max(sum, 1.0e-3f), 1.0f - band);
            const float segBase = sr_ / (hz * static_cast<float>(n));
            const float seg = std::max(1.0f, segBase * durs_[static_cast<std::size_t>(index_)] * pull);
            const int next = (index_ + 1) % n;
            const float frac = std::clamp(into_ / seg, 0.0f, 1.0f);
            const float y = amps_[static_cast<std::size_t>(index_)] * (1.0f - frac)
                          + amps_[static_cast<std::size_t>(next)] * frac;
            into_ += 1.0f;
            if (into_ >= seg) {
                into_ -= seg;
                if (!frozen) {
                    auto& a = amps_[static_cast<std::size_t>(index_)];
                    a = reflect(a + draw(dist) * ampStep * 0.5f, -1.0f, 1.0f);
                    auto& d = durs_[static_cast<std::size_t>(index_)];
                    d = reflect(d + draw(dist) * durStep * 0.5f * (dHi - dLo), dLo, dHi);
                }
                index_ = next;
            }
            // sem DC (a média do polígono passeia)
            dc_ += (y - dc_) * dcK;
            const float out = level * (y - dc_);
            for (std::size_t c = 0; c < channels; ++c) outputs[0].at(c, f) = out;
        }
    }

private:
    static float reflect(float v, float lo, float hi) noexcept {
        const float range = hi - lo;
        if (!(range > 0.0f) || !std::isfinite(v)) return lo;
        float m = std::fmod(v - lo, 2.0f * range);
        if (m < 0.0f) m += 2.0f * range;
        return lo + (m <= range ? m : 2.0f * range - m);
    }
    // um passo em [−1, 1] na distribuição escolhida (as do Studio)
    float draw(int dist) noexcept {
        const float u = unit();
        switch (dist) {
            case 0: return 2.0f * u - 1.0f;
            case 1: {   // gaussiana (Box–Muller), σ 0,35
                const float u2 = std::max(unit(), 1.0e-7f);
                const float g = std::sqrt(-2.0f * std::log(u2)) * std::cos(6.2831853f * u);
                return std::clamp(0.35f * g, -1.0f, 1.0f);
            }
            case 2: return std::clamp(std::tan(3.14159265f * (u - 0.5f) * 0.9f) * 0.15f, -1.0f, 1.0f);   // Cauchy
            default: {
                const float c = std::clamp(u, 1.0e-6f, 1.0f - 1.0e-6f);
                return std::clamp(std::log(c / (1.0f - c)) * 0.2f, -1.0f, 1.0f);   // logística
            }
        }
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }
    float unit() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 0x6E4D7A11ULL;
    std::array<float, kMax> amps_{}, durs_{};
    int index_ = 0;
    float into_ = 0.0f, dc_ = 0.0f;
};

} // namespace rasgo::modular
