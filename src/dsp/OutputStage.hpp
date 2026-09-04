#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

// ============================================================================
// OutputStage — proteção de saída de nível de excelência (usada pelo MASTER)
// ============================================================================
//
// Padrão de EXCELÊNCIA SONORA da família RASGO. Estudado de
// `NAVALHA2_JUCE/src/core/OutputStage.{h,cpp}` + `LookaheadLimiter` e
// `ANTITOTEM/src/core/OutputStage.h` (código do próprio autor, GPLv3/
// AGPLv3 — compatível). Reescrito aqui no idioma header-only zero-dep do
// Rasgo Modular.
//
// Princípio (dos dois): **"saturação criativa é do patch; remoção de DC e
// contenção de pico NÃO são"**. Este estágio é proteção técnica, não tom:
//
//   1. guarda de finitude  — NaN/Inf viram 0 (contados);
//   2. bloqueio de DC      — passa-alta de 1 polo (~5 Hz);
//   3. limitador com LOOK-AHEAD — atraso de ~3 ms; um seguidor de
//      envelope com ataque rápido / release lento reduz o ganho ANTES do
//      pico chegar, então não distorce o transiente (o `tanh`
//      instantâneo distorcia);
//   4. teto suave (joelho exponencial) — pega o evento de 1 amostra que
//      escapa do look-ahead (ruído, S&H, troca de cabo) sem corte duro;
//   5. teto ~−1 dBFS com margem de segurança.
//
// Desvio Rasgo: `gainReductionDb()` é telemetria pública — um módulo pode
// SEGUIR a própria redução de ganho ("o instrumento reage ao próprio
// volume"), no espírito do barramento semântico.

namespace rasgo::modular {

class OutputStage {
public:
    void prepare(const float sampleRate, const float ceilingDb = -1.0f,
                 const float lookaheadMs = 3.0f,
                 const float releaseMs = 120.0f) {
        sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
        ceiling_ = std::pow(10.0f, ceilingDb / 20.0f);
        detect_ = ceiling_ * std::pow(10.0f, -0.3f / 20.0f);  // margem 0,3 dB
        // o joelho só pega o evento de 1 amostra bem perto do teto — abaixo
        // dele o estágio é transparente (o limitador com look-ahead faz o
        // trabalho de verdade)
        knee_ = ceiling_ * 0.97f;
        const float dcHz = 5.0f;
        dcR_ = std::exp(-6.2831853f * dcHz / sr_);
        // ataque rápido o bastante pra assentar dentro do look-ahead
        atkCoef_ = 1.0f - std::exp(-1.0f / std::max(1.0f, 0.0005f * sr_));
        relCoef_ = 1.0f - std::exp(-1.0f
                                   / std::max(1.0f, releaseMs * 0.001f * sr_));
        const std::size_t look =
            std::max<std::size_t>(1, static_cast<std::size_t>(
                std::lround(lookaheadMs * 0.001f * sr_)));
        len_ = look + 1;
        dl_.assign(len_, 0.0f);
        dr_.assign(len_, 0.0f);
        reset();
    }

    void reset() noexcept {
        for (auto& v : dl_) v = 0.0f;
        for (auto& v : dr_) v = 0.0f;
        w_ = 0;
        dcXl_ = dcYl_ = dcXr_ = dcYr_ = 0.0f;
        env_ = 0.0f;
        gain_ = 1.0f;
        grDb_ = 0.0f;
        outPeak_ = 0.0f;
        nonFinite_ = 0;
    }

    // processa um par estéreo IN-PLACE (sem alocação, sem lock).
    // `doDc`/`doLimit` gatilham as etapas 2 e 3; a guarda de finitude (1) e
    // o teto suave (4) rodam sempre — são baratos e são segurança.
    void process(float& l, float& r, const bool doDc = true,
                 const bool doLimit = true) noexcept {
        l = finite(l);
        r = finite(r);

        // 2. bloqueio de DC (passa-alta de 1 polo)
        if (doDc) {
            const float yl = l - dcXl_ + dcR_ * dcYl_;
            dcXl_ = l; dcYl_ = yl; l = yl;
            const float yr = r - dcXr_ + dcR_ * dcYr_;
            dcXr_ = r; dcYr_ = yr; r = yr;
        }

        float ol, orr;
        if (doLimit) {
            // 3. limitador com look-ahead
            const float peak = std::fabs(l) > std::fabs(r) ? std::fabs(l)
                                                           : std::fabs(r);
            env_ += (peak - env_) * (peak > env_ ? atkCoef_ : relCoef_);
            const float target = env_ > detect_ && env_ > 0.0f ? detect_ / env_
                                                               : 1.0f;
            gain_ += (target - gain_) * (target < gain_ ? atkCoef_ : relCoef_);
            if (gain_ < 0.0f) gain_ = 0.0f;
            if (gain_ > 1.0f) gain_ = 1.0f;

            dl_[w_] = l;
            dr_[w_] = r;
            w_ = (w_ + 1) % len_;
            ol = dl_[w_] * gain_;
            orr = dr_[w_] * gain_;
        } else {
            gain_ += (1.0f - gain_) * relCoef_;   // solta o limitador
            ol = l;
            orr = r;
        }

        // 4. teto suave (último recurso de 1 amostra)
        ol = softCeiling(ol);
        orr = softCeiling(orr);

        // telemetria
        const float gr = gain_ > 0.0f ? -20.0f * std::log10(gain_) : 120.0f;
        if (gr > grDb_) grDb_ = gr;
        const float op = std::fabs(ol) > std::fabs(orr) ? std::fabs(ol)
                                                        : std::fabs(orr);
        if (op > outPeak_) outPeak_ = op;

        l = ol;
        r = orr;
    }

    // telemetria — leia e zere no intervalo do bloco/UI (não RT-crítico)
    float gainReductionDb() const noexcept { return grDb_; }
    float outputPeak() const noexcept { return outPeak_; }
    std::uint64_t nonFiniteCount() const noexcept { return nonFinite_; }
    void clearTelemetry() noexcept { grDb_ = 0.0f; outPeak_ = 0.0f; }
    float ceilingLinear() const noexcept { return ceiling_; }

private:
    float finite(const float x) noexcept {
        if (std::isfinite(x)) return x;
        ++nonFinite_;
        return 0.0f;
    }
    float softCeiling(const float x) const noexcept {
        const float m = std::fabs(x);
        if (m <= knee_) return x;
        const float curved = knee_ + (ceiling_ - knee_)
            * (1.0f - std::exp(-(m - knee_) / (ceiling_ - knee_)));
        return std::copysign(curved, x);
    }

    float sr_ = 48000.0f;
    float ceiling_ = 0.891f, detect_ = 0.861f, knee_ = 0.864f;
    float dcR_ = 0.999f, atkCoef_ = 0.1f, relCoef_ = 0.001f;
    float dcXl_ = 0.0f, dcYl_ = 0.0f, dcXr_ = 0.0f, dcYr_ = 0.0f;
    float env_ = 0.0f, gain_ = 1.0f;
    float grDb_ = 0.0f, outPeak_ = 0.0f;
    std::uint64_t nonFinite_ = 0;
    std::vector<float> dl_, dr_;
    std::size_t len_ = 2, w_ = 0;
};

}  // namespace rasgo::modular
