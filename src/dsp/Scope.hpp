#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>

// ============================================================================
// SCOPE — Osciloscópio + análise (Módulo 29)
// ============================================================================
//
// Ferramenta de medição — e, desvio Rasgo, as medições SAEM COMO CV: num
// osciloscópio de hardware a tela é um beco sem saída; aqui o nível, o brilho
// (centroide espectral) e a altura detectada VOLTAM pro patch. `in` passa
// LIMPO em `thru` (saída 0 — o que o Display do painel desenha). `trig` é um
// comparador com histerese (trigger do scope + disparador utilitário).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/29_scope.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - osciloscópio de bancada (trigger de nível/borda/histerese, timebase);
//   - Mordax DATA / ALM MUM M8 / Intellijel µScope (scope de rack utilitário);
//   - centroide espectral pelo diferenciador (Parseval: energia de x' = 2º
//     momento do espectro) — resultado matemático público;
//   - detecção de pitch por período entre cruzamentos de zero (ZCR).

namespace rasgo::modular {

class Scope final : public Signal {
public:
    Scope()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"ext", PortKind::Control, ""}},
              {{"thru", PortKind::Audio, ""},
               {"trig", PortKind::Control, "gate"},
               {"level", PortKind::Control, ""},
               {"bright", PortKind::Control, ""},
               {"pitch", PortKind::Control, "v/oct"}},
              {{"trigger", -1.0f, 1.0f, 0.0f, ""},
               {"edge", 0.0f, 1.0f, 0.0f, ""},
               {"reject", 0.0f, 1.0f, 0.1f, ""},
               {"response", 0.0f, 1.0f, 0.3f, ""},
               {"hold", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "SCOPE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "SCOPE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "scope", "", 2.5f, 6.0f, 66.1f);
        p.add(Widget::Kind::Knob, "TRIG", "trigger", 9.0f, 34.0f);
        p.add(Widget::Kind::Knob, "EDGE", "edge", 27.0f, 34.0f);
        p.add(Widget::Kind::Knob, "REJ", "reject", 45.0f, 34.0f);
        p.add(Widget::Kind::Knob, "RESP", "response", 9.0f, 56.0f);
        p.add(Widget::Kind::Knob, "HOLD", "hold", 27.0f, 56.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 82.0f);
        p.add(Widget::Kind::Jack, "EXT", "in:ext", 19.0f, 82.0f);
        p.add(Widget::Kind::Jack, "THRU", "out:thru", 7.0f, 106.0f);
        p.add(Widget::Kind::Jack, "TRIG", "out:trig", 19.0f, 106.0f);
        p.add(Widget::Kind::Jack, "LVL", "out:level", 31.0f, 106.0f);
        p.add(Widget::Kind::Jack, "BRT", "out:bright", 43.0f, 106.0f);
        p.add(Widget::Kind::Jack, "PIT", "out:pitch", 55.0f, 106.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        lvl_ = 0.0f;
        bright_ = 0.0f;
        bTarget_ = 0.0f;
        pitchOct_ = 0.0f;
        pitchTarget_ = 0.0f;
        xPrev_ = 0.0f;
        sumX2_ = 0.0f;
        sumD2_ = 0.0f;
        win_ = 0;
        sinceZC_ = 0;
        periodEst_ = sr_ / 220.0f;
        lastP_ = sr_ / 220.0f;
        lockCount_ = 0;
        zcArmed_ = false;
        armed_ = false;
        trigCd_ = 0;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const float trigger = clampf(parameterValue("trigger"), -1.0f, 1.0f);
        const bool falling = parameterValue("edge") >= 0.5f;
        const float rejBand = clamp01(parameterValue("reject")) * 0.5f;
        const float response = clamp01(parameterValue("response"));
        const bool hold = parameterValue("hold") >= 0.5f;

        // seguidores: `response` alonga tudo; ataque sempre << release
        const float atkT = 0.001f + response * response * 0.15f;
        const float relT = 0.04f + response * response * 2.0f;
        const float lvlAtk = 1.0f - std::exp(-1.0f / (atkT * sr_));
        const float lvlRel = 1.0f - std::exp(-1.0f / (relT * sr_));
        // brilho/pitch: uma constante só, média por `response`
        const float smoothT = 0.02f + response * response * 0.8f;
        const float smCoef = 1.0f - std::exp(-1.0f / (smoothT * sr_));
        const int trigLen = static_cast<int>(0.001f * sr_) + 1;

        const AudioBlock* in = inputs[0];
        const AudioBlock* ext = inputs[1];

        const float hiThr = trigger + rejBand;
        const float loThr = trigger - rejBand;

        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in != nullptr ? in->at(0, f) : 0.0f;
            const float s = ext != nullptr ? ext->at(0, f) : x;

            // ---- nível: pico com ataque rápido, release lento ----
            const float pk = std::fabs(x);
            if (!hold) {
                lvl_ += (pk - lvl_) * (pk > lvl_ ? lvlAtk : lvlRel);
            }

            // ---- centroide espectral pelo diferenciador (janela 512) ----
            const float d = x - xPrev_;
            xPrev_ = x;
            sumX2_ += x * x;
            sumD2_ += d * d;
            if (++win_ >= 512) {
                const float ratio = sumD2_ / (sumX2_ > 1e-12f ? sumX2_ : 1e-12f);
                const float fc = (sr_ * 0.15915494f) * std::sqrt(ratio);
                bTarget_ = clamp01(std::sqrt(fc / 8000.0f));
                win_ = 0;
                sumX2_ = 0.0f;
                sumD2_ = 0.0f;
            }
            if (!hold) bright_ += (bTarget_ - bright_) * smCoef;

            // ---- pitch: período entre cruzamentos de zero de subida ----
            // só "trava" depois de 3 períodos consistentes (±25%) — ruído,
            // que dá períodos aleatórios, nunca trava → pitch fica em 0.
            ++sinceZC_;
            if (zcArmed_ && x > 0.02f) {
                const float p = static_cast<float>(sinceZC_);
                const float hz = sr_ / std::max(1.0f, p);
                if (lvl_ > 0.02f && hz >= 20.0f && hz <= 5000.0f) {
                    if (std::fabs(p - lastP_) < 0.25f * lastP_) {
                        if (lockCount_ < 8) ++lockCount_;
                        periodEst_ += (p - periodEst_) * 0.30f;
                    } else {
                        lockCount_ = 0;
                        periodEst_ = p;
                    }
                    lastP_ = p;
                } else {
                    lockCount_ = 0;
                }
                sinceZC_ = 0;
                zcArmed_ = false;
            }
            if (x < -0.02f) zcArmed_ = true;
            if (sinceZC_ > static_cast<long>(sr_ / 15.0f)) lockCount_ = 0; // sem sinal
            if (lockCount_ >= 3 && lvl_ > 0.02f) {
                const float phz = sr_ / std::max(1.0f, periodEst_);
                pitchTarget_ = std::log2(phz / 110.0f);
            } else {
                pitchTarget_ = 0.0f;
            }
            if (!hold) pitchOct_ += (pitchTarget_ - pitchOct_) * smCoef;

            // ---- trigger: comparador com histerese + borda ----
            if (falling) {
                if (s > hiThr) armed_ = true;
                else if (armed_ && s < loThr) { trigCd_ = trigLen; armed_ = false; }
            } else {
                if (s < loThr) armed_ = true;
                else if (armed_ && s > hiThr) { trigCd_ = trigLen; armed_ = false; }
            }
            const float trg = trigCd_ > 0 ? 1.0f : 0.0f;
            if (trigCd_ > 0) --trigCd_;

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = x;         // thru — limpo
                outputs[1].at(c, f) = trg;
                outputs[2].at(c, f) = lvl_;
                outputs[3].at(c, f) = bright_;
                outputs[4].at(c, f) = pitchOct_;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    float sr_ = 48000.0f;
    float lvl_ = 0.0f;
    float bright_ = 0.0f;
    float bTarget_ = 0.0f;
    float pitchOct_ = 0.0f;
    float pitchTarget_ = 0.0f;
    float xPrev_ = 0.0f;
    float sumX2_ = 0.0f;
    float sumD2_ = 0.0f;
    int win_ = 0;
    long sinceZC_ = 0;
    float periodEst_ = 218.0f;
    float lastP_ = 218.0f;
    int lockCount_ = 0;
    bool zcArmed_ = false;
    bool armed_ = false;
    int trigCd_ = 0;
};

}  // namespace rasgo::modular
