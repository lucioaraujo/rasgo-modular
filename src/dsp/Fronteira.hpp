#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>

// FRONTEIRA (#79) — onde a célula melódica de quem toca começa (2026-11-11;
// dossiê `dossies/79_fronteira.md`; estudo da família §4.9, "FRASE",
// aprovado; o nome FRONTEIRA porque o PHRASE do Modular já é outro módulo).
//
// LBDM — Local Boundary Detection Model (E. Cambouropoulos, 2001), a forma
// computável das regras de agrupamento de Lerdahl & Jackendoff (GTTM,
// 1983): percebemos fronteira onde há MUDANÇA. Três dimensões por par de
// notas — o SALTO de altura, o intervalo entre ataques (IOI) e a PAUSA — e
// em cada uma a força de fronteira da nota i é vᵢ·(r(vᵢ₋₁,vᵢ) + r(vᵢ,vᵢ₊₁)),
// com r(a,b) = |a−b|/(a+b). Pesos 0,25 / 0,50 / 0,25 (o ritmo agrupa mais
// que a altura; ponto de partida da literatura, como no `phrasing.py` do
// Maravi — mesmo autor, conceito, não código).
//
// Ao vivo: a força da nota i precisa da nota i+1, então a fronteira sai no
// ATAQUE da nota seguinte (uma nota de atraso, dito). Normalização causal:
// cada dimensão divide pelo seu máximo recente, que desbota (MEM notas).
// Fronteira = força ≥ LIM, maior que a da nota anterior, e a célula tem pelo
// menos MIN notas; as 5 primeiras notas só formam a referência. SILÊNCIO longo (sem ataque por SIL s) fecha a frase na
// hora: um pulso em FIM.
//
// Saídas: FRONT (pulso: começou uma célula), FORÇA (a força da última nota,
// 0–1), NOTAS (o tamanho da última célula fechada, /16) e FIM (o silêncio).
namespace rasgo::modular {

class Fronteira final : public Signal {
public:
    Fronteira() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"gate", PortKind::Control, "gate"}},
        {{"boundary", PortKind::Control, "trig"},
         {"strength", PortKind::Control, ""},
         {"notes", PortKind::Control, ""},
         {"end", PortKind::Control, "trig"}},
        {{"threshold", 0.05f, 0.9f, 0.3f, ""},
         {"memory", 4.0f, 64.0f, 16.0f, ""},
         {"min_notes", 2.0f, 8.0f, 2.0f, ""},
         {"silence", 0.3f, 4.0f, 1.5f, "s"}}) {}

    std::string type() const override { return "FRONTEIRA"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "FRONTEIRA", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "fronteira", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "LIM", "threshold", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MEM", "memory", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MIN", "min_notes", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SIL", "silence", 8.0f, 54.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FRONT", "out:boundary", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "FORCA", "out:strength", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "NOTAS", "out:notes", 31.0f, 116.0f);
        p.add(Widget::Kind::Jack, "FIM", "out:end", 43.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearAll();
    }

    long boundaries() const noexcept { return bounds_; }
    float lastStrength() const noexcept { return strength_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 4) return;
        const AudioBlock* pitch = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* gate = inputs.size() > 1 ? inputs[1] : nullptr;
        const float lim = std::clamp(safe(parameterValue("threshold"), 0.3f), 0.05f, 0.9f);
        const float mem = std::clamp(safe(parameterValue("memory"), 16.0f), 4.0f, 64.0f);
        const int minNotes = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("min_notes"), 2.0f), 2.0f, 8.0f)));
        const double silence = std::clamp(safe(parameterValue("silence"), 1.5f), 0.3f, 4.0f) * sr_;
        const int pulse = std::max(1, static_cast<int>(0.010f * sr_));
        const float fade = std::exp(-1.0f / mem);   // por nota

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            now_ += 1.0;
            const float g = gate ? safe(gate->at(0, f), 0.0f) : 0.0f;
            const float pv = pitch ? safe(pitch->at(0, f), 0.0f) : 0.0f;
            if (prevGate_ < 0.5f && g >= 0.5f) onNote(pv * 12.0f, lim, minNotes, fade, pulse);
            if (prevGate_ >= 0.5f && g < 0.5f) lastOff_ = now_;
            prevGate_ = g;
            // o silêncio longo fecha a frase na hora
            if (n_ > 0 && !closedBySilence_ && g < 0.5f && now_ - onset_[0] > silence
                && now_ - lastOff_ > 0.25 * silence) {
                closedBySilence_ = true;
                endLeft_ = pulse;
                notesOut_ = std::min(1.0f, static_cast<float>(cellLen_) / 16.0f);
            }
            const float b = bLeft_ > 0 ? 1.0f : 0.0f;
            const float e = endLeft_ > 0 ? 1.0f : 0.0f;
            if (bLeft_ > 0) --bLeft_;
            if (endLeft_ > 0) --endLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = b;
                outputs[1].at(c, f) = strength_;
                outputs[2].at(c, f) = notesOut_;
                outputs[3].at(c, f) = e;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    static float degree(const float a, const float b) noexcept {
        const float s = a + b;
        return s > 0.0f ? std::fabs(a - b) / s : 0.0f;
    }

    void clearAll() noexcept {
        n_ = 0;
        now_ = 0.0;
        lastOff_ = 0.0;
        prevGate_ = 0.0f;
        strength_ = prevStrength_ = 0.0f;
        maxS_.fill(1.0e-6f);
        cellLen_ = 0;
        bLeft_ = endLeft_ = 0;
        notesOut_ = 0.0f;
        bounds_ = 0;
        closedBySilence_ = false;
    }

    // notas guardadas: [0] = a mais recente
    void onNote(const float semi, const float lim, const int minNotes, const float fade,
                const int pulse) noexcept {
        const double offPrev = lastOff_;
        // desloca o histórico
        for (int i = 3; i > 0; --i) {
            onset_[static_cast<std::size_t>(i)] = onset_[static_cast<std::size_t>(i - 1)];
            semi_[static_cast<std::size_t>(i)] = semi_[static_cast<std::size_t>(i - 1)];
            for (int d = 0; d < 3; ++d) val_[static_cast<std::size_t>(i)][static_cast<std::size_t>(d)] = val_[static_cast<std::size_t>(i - 1)][static_cast<std::size_t>(d)];
        }
        onset_[0] = now_;
        semi_[0] = semi;
        if (n_ >= 1) {
            val_[0][0] = std::fabs(semi_[0] - semi_[1]);
            val_[0][1] = static_cast<float>((onset_[0] - onset_[1]) / sr_);
            val_[0][2] = static_cast<float>(std::max(0.0, (onset_[0] - offPrev) / sr_));
        } else {
            val_[0] = {0.0f, 0.0f, 0.0f};
        }
        n_ = std::min(n_ + 1, 1 << 20);
        ++cellLen_;
        // a silêncio fechou a frase: esta nota começa uma célula nova
        if (closedBySilence_) {
            closedBySilence_ = false;
            cellLen_ = 1;
            bLeft_ = pulse;
            ++bounds_;
            strength_ = 1.0f;
            prevStrength_ = 0.0f;
            return;
        }
        // a força da nota ANTERIOR (índice 1) já se pode medir: precisa dos
        // valores 2 (antes), 1 (ela) e 0 (depois)
        if (n_ < 3) return;
        static constexpr float kW[3] = {0.25f, 0.50f, 0.25f};
        float s = 0.0f;
        for (int d = 0; d < 3; ++d) {
            const float v = val_[1][static_cast<std::size_t>(d)];
            const float raw = v * (degree(val_[2][static_cast<std::size_t>(d)], v)
                                 + degree(v, val_[0][static_cast<std::size_t>(d)]));
            float& mx = maxS_[static_cast<std::size_t>(d)];
            mx = std::max(raw, mx * fade);
            s += kW[d] * (mx > 1.0e-6f ? raw / mx : 0.0f);
        }
        prevStrength_ = strength_;
        strength_ = std::clamp(s, 0.0f, 1.0f);
        // a nota anterior começou uma célula? (a célula anterior tinha
        // cellLen_ − 1 notas antes dela)
        // as primeiras notas só formam a referência (os máximos recentes)
        if (n_ < 6) return;
        if (strength_ >= lim && strength_ >= prevStrength_ && cellLen_ - 1 >= minNotes) {
            notesOut_ = std::min(1.0f, static_cast<float>(cellLen_ - 1) / 16.0f);
            cellLen_ = 1;   // a célula nova já tem a nota anterior… e esta
            ++cellLen_;
            bLeft_ = pulse;
            ++bounds_;
        }
    }

    float sr_ = 48000.0f;
    double now_ = 0.0, lastOff_ = 0.0;
    std::array<double, 4> onset_{};
    std::array<float, 4> semi_{};
    std::array<std::array<float, 3>, 4> val_{};
    std::array<float, 3> maxS_{};
    int n_ = 0, cellLen_ = 0;
    float prevGate_ = 0.0f, strength_ = 0.0f, prevStrength_ = 0.0f, notesOut_ = 0.0f;
    int bLeft_ = 0, endLeft_ = 0;
    long bounds_ = 0;
    bool closedBySilence_ = false;
};

}  // namespace rasgo::modular
