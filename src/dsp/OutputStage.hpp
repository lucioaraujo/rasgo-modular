#pragma once

#include "dsp/TruePeak.hpp"

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
//   3. guarda ULTRASSÔNICA — passa-baixa Butterworth de 2 polos em ~21 kHz.
//      Musicalmente transparente (−0,1 dB a 8 kHz, ~−1 dB a 15 kHz), tira
//      só a energia perto de Nyquist (ruído violeta/`bit` sustentados,
//      hash de aliasing) — o "ice-pick" que dá pressão física sem ser
//      nota. Sempre ligada: é como o bloqueio de DC, nunca é escolha
//      musical. NÃO mexe no que você OUVE como timbre agressivo.
//   4. GOVERNADOR DE CORPO (`bodyGuard` 0..1, ligado no MASTER por padrão,
//      0 = bypass exato) — a proposta do autor ("gosto de barulhos, mas
//      há alguns que passam do limite, incomodam o corpo"). Detecta
//      energia ALTA + SUSTENTADA + CONCENTRADA na faixa ~2,5–8 kHz (o
//      pico de dor/fadiga do ouvido). Ataque LENTO (~250 ms): transiente,
//      ritmo, rajada de ruído — tudo passa intocado; só o que fica
//      *parado, alto e concentrado* ali (filtro auto-oscilando, quadrada
//      do PLL segurada, folding travado) dispara. E um "gate de
//      concentração" — razão banda/total: ruído de banda larga passa
//      (energia espalhada), tom perfurante não. Quando dispara: um
//      high-shelf suave (até ~−9 dB acima de ~2 kHz), poucos dB,
//      acompanhando, release lento — "tira o fio da parte que fura", não
//      "limpa o som". Limiar alto: música de ruído passa com zero
//      redução na esmagadora maioria dos casos.
//   5. limitador com LOOK-AHEAD, orientado por PICO VERDADEIRO — atraso de
//      ~3 ms; um seguidor de envelope com ataque rápido / release lento
//      reduz o ganho ANTES do pico chegar, então não distorce o
//      transiente (o `tanh` instantâneo distorcia). O detector olha o
//      MAIOR entre o pico de amostra e a estimativa de pico ENTRE
//      amostras (`TruePeakEstimator`, `dsp/TruePeak.hpp`) — um limitador
//      que só vê o pico de amostra pode deixar passar um pico
//      reconstruído acima do teto (achado real, medido na NAVALHA 2:
//      `AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md` §3.4 — +1,33 dBFS de estouro
//      com um teto de −1 dBFS, antes da correção);
//   6. teto suave (joelho exponencial) — pega o evento de 1 amostra que
//      escapa do look-ahead (ruído, S&H, troca de cabo) sem corte duro;
//   7. teto ~−1 dBFS com margem de segurança.
//
// Desvio Rasgo: `gainReductionDb()` / `bodyGuardDb()` são telemetria
// pública — um módulo pode SEGUIR a própria redução ("o instrumento
// reage ao próprio volume"), no espírito do barramento semântico.

namespace rasgo::modular {

// SVF TPT de 2 polos (Zavalishin / Cytomic, "trapezoidal") — estável até
// perto de Nyquist, ao contrário de um biquad Direct-Form ingênuo. Estado
// = 2 floats. Devolve passa-baixa e passa-banda de um cálculo só.
struct TptSvf2 {
    float g_ = 0.0f, k_ = 1.41421356f, a1_ = 0.0f, a2_ = 0.0f, a3_ = 0.0f;
    float ic1_ = 0.0f, ic2_ = 0.0f;

    void set(const float fcOverSr, const float q) noexcept {
        const float f = fcOverSr < 0.49f ? (fcOverSr > 1e-5f ? fcOverSr : 1e-5f)
                                         : 0.49f;
        g_ = std::tan(3.14159265f * f);
        k_ = 1.0f / (q > 0.05f ? q : 0.05f);
        a1_ = 1.0f / (1.0f + g_ * (g_ + k_));
        a2_ = g_ * a1_;
        a3_ = g_ * a2_;
    }
    void reset() noexcept { ic1_ = ic2_ = 0.0f; }

    struct Out { float lp, bp; };
    Out process(const float x) noexcept {
        const float v3 = x - ic2_;
        const float v1 = a1_ * ic1_ + a2_ * v3;
        const float v2 = ic2_ + a2_ * ic1_ + a3_ * v3;
        ic1_ = 2.0f * v1 - ic1_;
        ic2_ = 2.0f * v2 - ic2_;
        return {v2, k_ * v1};   // bp com ganho de pico 1 (não Q)
    }
};

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
        tpL_.prepare();
        tpR_.prepare();

        // ---- guarda ultrassônica: LP Butterworth 2 polos a ~21 kHz -------
        const float ultraHz = std::min(21000.0f, sr_ * 0.44f);
        ultraL_.set(ultraHz / sr_, 0.70710678f);
        ultraR_.set(ultraHz / sr_, 0.70710678f);

        // ---- governador de corpo ---------------------------------------
        // 3 detectores estreitos (Q alto) cobrindo ~2,5–8 kHz — a faixa
        // "perfurante" pro corpo. Q alto rejeita bem o que está fora
        // (ex.: um tom em 11 kHz mal aparece).
        bandLo_.set(std::min(2800.0f, sr_ * 0.45f) / sr_, 3.0f);
        bandMid_.set(std::min(4800.0f, sr_ * 0.45f) / sr_, 3.0f);
        bandHi_.set(std::min(7600.0f, sr_ * 0.45f) / sr_, 3.0f);
        // seguidores LENTOS — só o que fica parado ~250 ms conta
        const float slowMs = 240.0f;
        bandAtk_ = 1.0f - std::exp(-1.0f / std::max(1.0f, slowMs * 0.001f * sr_));
        bandRel_ = 1.0f - std::exp(-1.0f
                                   / std::max(1.0f, 380.0f * 0.001f * sr_));
        // shelf: divisor de 1 polo (o corte age acima de ~1,4 kHz —
        // pega a faixa perfurante de vez, deixa o grave/médio-grave).
        shelfCoef_ = 1.0f - std::exp(-6.2831853f * 1400.0f / sr_);
        // o corte em si sobe/desce devagar (nada de degrau audível)
        cutAtk_ = 1.0f - std::exp(-1.0f / std::max(1.0f, 120.0f * 0.001f * sr_));
        cutRel_ = 1.0f - std::exp(-1.0f / std::max(1.0f, 450.0f * 0.001f * sr_));

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
        truePeak_ = 0.0f;
        nonFinite_ = 0;
        tpL_.reset();
        tpR_.reset();
        ultraL_.reset();
        ultraR_.reset();
        bandLo_.reset();
        bandMid_.reset();
        bandHi_.reset();
        bandEnv_ = totalEnv_ = 0.0f;
        bodyCut_ = 0.0f;
        shelfLpL_ = shelfLpR_ = 0.0f;
        bodyGrDb_ = 0.0f;
    }

    // processa um par estéreo IN-PLACE (sem alocação, sem lock).
    // `doDc`/`doLimit` gatilham as etapas 2 e 5; `bodyGuard` (0..1) a 4.
    // A guarda de finitude (1), a ultrassônica (3) e o teto suave (6)
    // rodam sempre — são baratas e são segurança.
    void process(float& l, float& r, const bool doDc = true,
                 const bool doLimit = true, const float bodyGuard = 0.0f) noexcept {
        l = finite(l);
        r = finite(r);

        // 2. bloqueio de DC (passa-alta de 1 polo)
        if (doDc) {
            const float yl = l - dcXl_ + dcR_ * dcYl_;
            dcXl_ = l; dcYl_ = yl; l = yl;
            const float yr = r - dcXr_ + dcR_ * dcYr_;
            dcXr_ = r; dcYr_ = yr; r = yr;
        }

        // 3. guarda ultrassônica — sempre (segurança, não timbre)
        l = ultraL_.process(l).lp;
        r = ultraR_.process(r).lp;

        // 4. governador de corpo — detecção sempre (mantém o estado
        //    quente e determinístico); o corte só age com `bodyGuard > 0`.
        {
            const float mono = 0.5f * (l + r);
            const float b1 = std::fabs(bandLo_.process(mono).bp);
            const float b2 = std::fabs(bandMid_.process(mono).bp);
            const float b3 = std::fabs(bandHi_.process(mono).bp);
            const float band = std::max(b1, std::max(b2, b3));
            const float amono = std::fabs(mono);
            bandEnv_ += (band - bandEnv_)
                * (band > bandEnv_ ? bandAtk_ : bandRel_);
            totalEnv_ += (amono - totalEnv_)
                * (amono > totalEnv_ ? bandAtk_ : bandRel_);
            // gate de concentração: tom perfurante (energia concentrada na
            // faixa) dispara; ruído de banda larga (energia espalhada) não.
            const float conc = bandEnv_ / (totalEnv_ + 1e-6f);
            const float gate = smoothstep(0.42f, 0.72f, conc);
            const float harsh = bandEnv_ * gate;
            // limiar alto (~−15 dBFS de banda) + ganho suave acima dele,
            // teto de corte ~0,66 (≈ −9 dB no shelf)
            const float over = harsh - 0.18f;
            float target = over > 0.0f ? over * 2.1f : 0.0f;
            if (target > 0.72f) target = 0.72f;
            target *= (bodyGuard < 0.0f ? 0.0f : (bodyGuard > 1.0f ? 1.0f
                                                                   : bodyGuard));
            bodyCut_ += (target - bodyCut_)
                * (target > bodyCut_ ? cutAtk_ : cutRel_);

            if (bodyCut_ > 1e-4f) {
                shelfLpL_ += shelfCoef_ * (l - shelfLpL_);
                shelfLpR_ += shelfCoef_ * (r - shelfLpR_);
                // high-shelf: mantém o grave (lp), atenua o resto por `cut`
                l = shelfLpL_ + (l - shelfLpL_) * (1.0f - bodyCut_);
                r = shelfLpR_ + (r - shelfLpR_) * (1.0f - bodyCut_);
            } else {
                // segue o sinal pra não dar salto quando voltar a agir
                shelfLpL_ += shelfCoef_ * (l - shelfLpL_);
                shelfLpR_ += shelfCoef_ * (r - shelfLpR_);
            }
            const float bgr = bodyCut_ < 0.999f
                ? -20.0f * std::log10(1.0f - bodyCut_) : 60.0f;
            if (bgr > bodyGrDb_) bodyGrDb_ = bgr;
        }

        // pico verdadeiro (entre amostras) da saída dos guardas — o que
        // de fato entra no limitador; roda sempre (barato).
        const float tpL = tpL_.processSample(l);
        const float tpR = tpR_.processSample(r);
        const float tp = tpL > tpR ? tpL : tpR;
        if (tp > truePeak_) truePeak_ = tp;

        float ol, orr;
        if (doLimit) {
            // 5. limitador com look-ahead, orientado por pico verdadeiro
            const float samplePeak = std::fabs(l) > std::fabs(r) ? std::fabs(l)
                                                                 : std::fabs(r);
            const float peak = samplePeak > tp ? samplePeak : tp;
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

        // 6. teto suave (último recurso de 1 amostra)
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
    // quanto o governador de corpo está atenuando o agudo (dB de shelf)
    float bodyGuardDb() const noexcept { return bodyGrDb_; }
    float outputPeak() const noexcept { return outPeak_; }
    // maior pico VERDADEIRO (entre amostras) na saída dos guardas, entrando
    // no limitador — se ficar bem acima de `outputPeak()`, o pico de
    // amostra sozinho estava escondendo estouro
    float inputTruePeak() const noexcept { return truePeak_; }
    std::uint64_t nonFiniteCount() const noexcept { return nonFinite_; }
    void clearTelemetry() noexcept {
        grDb_ = 0.0f; outPeak_ = 0.0f; truePeak_ = 0.0f; bodyGrDb_ = 0.0f;
    }
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
    static float smoothstep(const float a, const float b, const float x) noexcept {
        if (b <= a) return x >= b ? 1.0f : 0.0f;
        float t = (x - a) / (b - a);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return t * t * (3.0f - 2.0f * t);
    }

    float sr_ = 48000.0f;
    float ceiling_ = 0.891f, detect_ = 0.861f, knee_ = 0.864f;
    float dcR_ = 0.999f, atkCoef_ = 0.1f, relCoef_ = 0.001f;
    float dcXl_ = 0.0f, dcYl_ = 0.0f, dcXr_ = 0.0f, dcYr_ = 0.0f;
    float env_ = 0.0f, gain_ = 1.0f;
    float grDb_ = 0.0f, outPeak_ = 0.0f, truePeak_ = 0.0f;
    std::uint64_t nonFinite_ = 0;
    std::vector<float> dl_, dr_;
    std::size_t len_ = 2, w_ = 0;
    TruePeakEstimator tpL_, tpR_;

    // guarda ultrassônica
    TptSvf2 ultraL_, ultraR_;
    // governador de corpo
    TptSvf2 bandLo_, bandMid_, bandHi_;
    float bandAtk_ = 0.0f, bandRel_ = 0.0f, shelfCoef_ = 0.0f;
    float cutAtk_ = 0.0f, cutRel_ = 0.0f;
    float bandEnv_ = 0.0f, totalEnv_ = 0.0f, bodyCut_ = 0.0f;
    float shelfLpL_ = 0.0f, shelfLpR_ = 0.0f, bodyGrDb_ = 0.0f;
};

}  // namespace rasgo::modular
