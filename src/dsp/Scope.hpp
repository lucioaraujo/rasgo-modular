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
               {"pitch", PortKind::Control, "v/oct"},
               {"onset", PortKind::Control, "gate"}},
              {{"trigger", -1.0f, 1.0f, 0.0f, ""},
               {"edge", 0.0f, 1.0f, 0.0f, ""},
               {"reject", 0.0f, 1.0f, 0.1f, ""},
               {"response", 0.0f, 1.0f, 0.3f, ""},
               {"hold", 0.0f, 1.0f, 0.0f, ""},
               {"sens", 0.0f, 1.0f, 0.4f, ""}}) {}

    std::string type() const override { return "SCOPE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 15;
        p.add(Widget::Kind::Label, "SCOPE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "scope", "", 2.5f, 6.0f, 68.0f);
        p.add(Widget::Kind::Knob, "TRIG", "trigger", 9.0f, 34.0f);
        p.add(Widget::Kind::Knob, "EDGE", "edge", 27.0f, 34.0f);
        p.add(Widget::Kind::Knob, "REJ", "reject", 45.0f, 34.0f);
        p.add(Widget::Kind::Knob, "RESP", "response", 9.0f, 56.0f);
        p.add(Widget::Kind::Knob, "HOLD", "hold", 27.0f, 56.0f);
        p.add(Widget::Kind::Knob, "SENS", "sens", 45.0f, 56.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 82.0f);
        p.add(Widget::Kind::Jack, "EXT", "in:ext", 19.0f, 82.0f);
        p.add(Widget::Kind::Jack, "THRU", "out:thru", 7.0f, 106.0f);
        p.add(Widget::Kind::Jack, "TRIG", "out:trig", 19.0f, 106.0f);
        p.add(Widget::Kind::Jack, "LVL", "out:level", 31.0f, 106.0f);
        p.add(Widget::Kind::Jack, "BRT", "out:bright", 43.0f, 106.0f);
        p.add(Widget::Kind::Jack, "PIT", "out:pitch", 55.0f, 106.0f);
        p.add(Widget::Kind::Jack, "ONS", "out:onset", 67.0f, 106.0f);
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
        // ---- pitch por autocorrelação (YIN) num sinal decimado ----
        decSr_ = sr_ / static_cast<float>(kDec);
        decAcc_ = 0.0f;
        decCnt_ = 0;
        hopCnt_ = 0;
        dw_ = 0;
        for (auto& v : dhist_) v = 0.0f;
        periodEst_ = decSr_ / 220.0f;   // em amostras DECIMADAS
        lastP_ = periodEst_;
        lockCount_ = 0;
        armed_ = false;
        trigCd_ = 0;
        // detector de onset/transiente (envelope rápido vs lento)
        envFast_ = envSlow_ = 0.0f;
        onFastAtk_ = 1.0f - std::exp(-1.0f / (0.0008f * sr_));
        onFastRel_ = 1.0f - std::exp(-1.0f / (0.030f * sr_));
        onSlowAtk_ = 1.0f - std::exp(-1.0f / (0.015f * sr_));
        onSlowRel_ = 1.0f - std::exp(-1.0f / (0.070f * sr_));
        onsetCd_ = 0;
        onsetLock_ = 0;
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

            // ---- pitch por autocorrelação (YIN) num sinal decimado 3× ----
            // robusto a harmônicos (o ZCR reportava 2×/3× a altura); ruído
            // → sem mínimo abaixo do limiar → `pitch` fica em 0.
            decAcc_ += x;
            if (++decCnt_ >= kDec) {
                decCnt_ = 0;
                dhist_[static_cast<std::size_t>(dw_)] =
                    decAcc_ / static_cast<float>(kDec);
                dw_ = (dw_ + 1) % kHist;
                decAcc_ = 0.0f;
                if (++hopCnt_ >= kHop) {
                    hopCnt_ = 0;
                    if (lvl_ > 0.02f) analyzePitch();
                    else lockCount_ = 0;
                }
            }
            if (lockCount_ >= 2 && lvl_ > 0.02f) {
                const float phz = decSr_ / std::max(1.0f, periodEst_);
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

            // ---- onset: envelope rápido dispara acima do lento ----
            const float ax = std::fabs(x);
            envFast_ += (ax - envFast_)
                      * (ax > envFast_ ? onFastAtk_ : onFastRel_);
            envSlow_ += (ax - envSlow_)
                      * (ax > envSlow_ ? onSlowAtk_ : onSlowRel_);
            const float sens = clamp01(parameterValue("sens"));
            const float ratioThr = 1.2f + (1.0f - sens) * 1.8f;
            if (onsetLock_ > 0) --onsetLock_;
            if (onsetLock_ == 0 && lvl_ > 0.01f
                && envFast_ > envSlow_ * ratioThr + 0.004f) {
                onsetCd_ = static_cast<int>(0.002f * sr_) + 1;
                onsetLock_ = static_cast<int>(0.030f * sr_);
            }
            const float ons = onsetCd_ > 0 ? 1.0f : 0.0f;
            if (onsetCd_ > 0) --onsetCd_;

            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = x;         // thru — limpo
                outputs[1].at(c, f) = trg;
                outputs[2].at(c, f) = lvl_;
                outputs[3].at(c, f) = bright_;
                outputs[4].at(c, f) = pitchOct_;
                outputs[5].at(c, f) = ons;
            }
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // YIN (de Cheveigné & Kawahara, 2002) sobre o buffer decimado.
    // Diferença acumulada normalizada → 1º mínimo local abaixo do limiar
    // → interpolação parabólica. `periodEst_` em amostras decimadas.
    void analyzePitch() noexcept {
        // copia a janela (mais antigo → mais novo) contígua
        float w[kHist];
        for (int j = 0; j < kHist; ++j)
            w[j] = dhist_[static_cast<std::size_t>((dw_ + j) % kHist)];

        float dp[kMaxLag + 2];
        dp[0] = 1.0f;
        double running = 0.0;
        int bestTau = -1;
        for (int tau = 1; tau <= kMaxLag; ++tau) {
            double d = 0.0;
            for (int i = 0; i < kWin; ++i) {
                const float diff = w[i] - w[i + tau];
                d += static_cast<double>(diff) * diff;
            }
            running += d;
            dp[tau] = running > 1e-12
                ? static_cast<float>(d * tau / running) : 1.0f;
            if (tau >= kMinLag && bestTau < 0
                && dp[tau] < 0.15f && dp[tau] < dp[tau - 1]) {
                // confirma que é mínimo local (olha 1 à frente)
                double dn = 0.0;
                for (int i = 0; i < kWin; ++i) {
                    const float diff = w[i] - w[i + tau + 1];
                    dn += static_cast<double>(diff) * diff;
                }
                const float dpn = running + dn > 1e-12
                    ? static_cast<float>(dn * (tau + 1) / (running + dn)) : 1.0f;
                if (dp[tau] < dpn) bestTau = tau;
            }
        }
        if (bestTau < 0) {
            // sem período claro (ruído / silêncio) — solta a trava aos poucos
            if (--lockCount_ < 0) lockCount_ = 0;
            return;
        }
        // interpolação parabólica em dp[bestTau-1..+1]
        const float a = dp[bestTau - 1], b = dp[bestTau], c = dp[bestTau + 1];
        const float denom = a - 2.0f * b + c;
        const float delta = std::fabs(denom) > 1e-6f
            ? 0.5f * (a - c) / denom : 0.0f;
        const float p = static_cast<float>(bestTau)
            + clampf(delta, -1.0f, 1.0f);

        if (std::fabs(p - lastP_) < 0.25f * lastP_) {
            if (lockCount_ < 6) ++lockCount_;
            periodEst_ += (p - periodEst_) * 0.35f;
        } else {
            lockCount_ = 0;
            periodEst_ = p;
        }
        lastP_ = p;
    }

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

    // pitch por autocorrelação (YIN) — sinal decimado 3×
    static constexpr int kDec = 3;
    static constexpr int kWin = 320;     // janela de comparação (dec)
    static constexpr int kMaxLag = 300;  // ~53 Hz reais a decSr/3
    static constexpr int kMinLag = 16;   // ~1000 Hz reais
    static constexpr int kHist = kWin + kMaxLag + 2;
    static constexpr int kHop = 64;      // analisa a cada ~12 ms
    float decSr_ = 16000.0f;
    float dhist_[kHist] = {};
    float decAcc_ = 0.0f;
    int decCnt_ = 0;
    int hopCnt_ = 0;
    int dw_ = 0;
    float periodEst_ = 72.0f;
    float lastP_ = 72.0f;
    int lockCount_ = 0;

    bool armed_ = false;
    int trigCd_ = 0;

    // onset / transiente
    float envFast_ = 0.0f, envSlow_ = 0.0f;
    float onFastAtk_ = 0.02f, onFastRel_ = 0.001f;
    float onSlowAtk_ = 0.001f, onSlowRel_ = 0.0002f;
    int onsetCd_ = 0, onsetLock_ = 0;
};

}  // namespace rasgo::modular
