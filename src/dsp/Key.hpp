#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// KEY (#73) — o tom de quem toca (2026-11-11; dossiê `dossies/73_key.md`).
//
// Escuta uma linha de alturas e estima a TONALIDADE pelos perfis de
// Krumhansl e Kessler (1982): um histograma das 12 classes de altura,
// PONDERADO PELA DURAÇÃO de cada nota (a nota longa pesa mais que a de
// passagem — a escolha do Maravi, irmão RASGO), correlacionado com os 24
// perfis maior/menor. A margem para o segundo colocado sai como CERTEZA:
// ambiguidade real (relativa, cromatismo) é dita, não escondida.
//
// Sai no formato do QUANTIZER: RAIZ = tônica/12 V (entra em ROOT) e ESCALA
// = índice/11 (Maior = 1, menor natural = 2; entra em SCL). O patch passa a
// responder no tom de quem canta ou toca (SIGNAL-IN → SCOPE → KEY → QUANTIZER).
//
// Histerese: o tom publicado só troca quando o mesmo candidato vence por
// ESTAB notas seguidas. O histograma desbota no tempo (MEM) para seguir
// modulações. Causal, determinístico, sem alocação. Perfis publicados;
// código próprio.
namespace rasgo::modular {

class Key final : public Signal {
public:
    Key() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"gate", PortKind::Control, "gate"},
         {"reset", PortKind::Control, "trig"}},
        {{"root", PortKind::Control, ""},
         {"scale", PortKind::Control, ""},
         {"certainty", PortKind::Control, ""},
         {"changed", PortKind::Control, "trig"}},
        {{"memory", 2.0f, 60.0f, 15.0f, "s"},
         {"stability", 1.0f, 16.0f, 4.0f, ""}}) {}

    std::string type() const override { return "KEY"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "KEY", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "key", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "MEM", "memory", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "ESTAB", "stability", 30.0f, 30.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 31.0f, 96.0f);
        p.add(Widget::Kind::Jack, "ROOT", "out:root", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "SCL", "out:scale", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "CERT", "out:certainty", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "MUD", "out:changed", 43.0f, 116.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearAll();
    }

    // Para os testes e o mostrador.
    int tonic() const noexcept { return tonic_; }          // −1 = ainda sem tom
    bool minor() const noexcept { return minor_; }
    float certainty() const noexcept { return certainty_; }
    std::array<float, 12> histogram() const noexcept { return hist_; }

    static int pitchClass(float v) noexcept {
        const int semis = static_cast<int>(std::lround(v * 12.0f));
        return ((semis % 12) + 12) % 12;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* pitch = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* gate = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* reset = inputs.size() > 2 ? inputs[2] : nullptr;
        const float memory = std::clamp(safe(parameterValue("memory"), 15.0f), 2.0f, 60.0f);
        const int stability = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("stability"), 4.0f), 1.0f, 16.0f)));
        const float fade = std::exp(-1.0f / (memory * sr_));   // por amostra
        const int pulseLen = std::max(1, static_cast<int>(0.010f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearAll();
                prevReset_ = r;
            }
            int pc = curPc_;
            if (pitch) pc = pitchClass(safe(pitch->at(0, f), 0.0f));
            bool noteNow = false;
            if (gate) {
                const float g = safe(gate->at(0, f), 0.0f);
                if (prevGate_ < 0.5f && g >= 0.5f) noteNow = pitch != nullptr;
                prevGate_ = g;
            } else if (pitch && pc != curPc_) {
                noteNow = true;
            }
            // a nota que soa acumula a sua duração (amostra a amostra)
            if (curPc_ >= 0) curDur_ += 1.0f;
            if (noteNow) {
                if (curPc_ >= 0) {
                    hist_[static_cast<std::size_t>(curPc_)] += curDur_ / sr_;
                    estimate(stability, pulseLen);
                }
                curDur_ = 0.0f;
            }
            curPc_ = pitch ? pc : curPc_;
            // desbota (um multiplicador por amostra; barato)
            if ((f & 63u) == 0u) {
                const float k = std::pow(fade, 64.0f);
                for (auto& h : hist_) h *= k;
            }

            const float col = pulseLeft_ > 0 ? 1.0f : 0.0f;
            if (pulseLeft_ > 0) --pulseLeft_;
            const float rootV = tonic_ >= 0 ? static_cast<float>(tonic_) / 12.0f : 0.0f;
            const float scaleV = (minor_ ? 2.0f : 1.0f) / 11.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = rootV;
                outputs[1].at(c, f) = scaleV;
                outputs[2].at(c, f) = certainty_;
                outputs[3].at(c, f) = col;
            }
        }
    }

private:
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    void clearAll() noexcept {
        hist_.fill(0.0f);
        curPc_ = -1;
        curDur_ = 0.0f;
        tonic_ = -1;
        minor_ = false;
        certainty_ = 0.0f;
        candTonic_ = -1;
        candMinor_ = false;
        candRun_ = 0;
        pulseLeft_ = 0;
        prevGate_ = prevReset_ = 0.0f;
    }

    // Pearson entre o histograma girado para a tônica t e um perfil
    static double correlation(const std::array<float, 12>& h, const int t,
                              const std::array<double, 12>& prof) noexcept {
        double mx = 0.0, my = 0.0;
        for (int i = 0; i < 12; ++i) { mx += h[static_cast<std::size_t>((i + t) % 12)]; my += prof[static_cast<std::size_t>(i)]; }
        mx /= 12.0; my /= 12.0;
        double num = 0.0, dx = 0.0, dy = 0.0;
        for (int i = 0; i < 12; ++i) {
            const double x = h[static_cast<std::size_t>((i + t) % 12)] - mx;
            const double y = prof[static_cast<std::size_t>(i)] - my;
            num += x * y; dx += x * x; dy += y * y;
        }
        return (dx <= 0.0 || dy <= 0.0) ? 0.0 : num / std::sqrt(dx * dy);
    }

    void estimate(const int stability, const int pulseLen) noexcept {
        // Krumhansl & Kessler (1982), índice 0 = tônica
        static constexpr std::array<double, 12> kMajor{6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
        static constexpr std::array<double, 12> kMinor{6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17};
        int used = 0;
        for (const float v : hist_) if (v > 1.0e-6f) ++used;
        if (used < 2) return;   // uma classe só: qualquer tom explica igual
        double best = -2.0, second = -2.0;
        int bt = 0; bool bm = false;
        for (int t = 0; t < 12; ++t)
            for (int m = 0; m < 2; ++m) {
                const double r = correlation(hist_, t, m ? kMinor : kMajor);
                if (r > best) { second = best; best = r; bt = t; bm = m == 1; }
                else if (r > second) second = r;
            }
        // CERTEZA: a margem para o segundo (0,15 de margem já é tom claro)
        certainty_ = static_cast<float>(std::clamp((best - second) / 0.15, 0.0, 1.0));
        // histerese: o mesmo candidato precisa vencer `stability` notas seguidas
        if (bt == candTonic_ && bm == candMinor_) ++candRun_;
        else { candTonic_ = bt; candMinor_ = bm; candRun_ = 1; }
        const bool first = tonic_ < 0;
        if ((first || bt != tonic_ || bm != minor_) && candRun_ >= stability) {
            tonic_ = bt;
            minor_ = bm;
            if (!first) pulseLeft_ = pulseLen;
        }
    }

    float sr_ = 48000.0f;
    std::array<float, 12> hist_{};
    int curPc_ = -1;
    float curDur_ = 0.0f;
    int tonic_ = -1;
    bool minor_ = false;
    float certainty_ = 0.0f;
    int candTonic_ = -1;
    bool candMinor_ = false;
    int candRun_ = 0;
    int pulseLeft_ = 0;
    float prevGate_ = 0.0f, prevReset_ = 0.0f;
};

}  // namespace rasgo::modular
