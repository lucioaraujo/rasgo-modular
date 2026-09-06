#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// BOXCAR — averager de porta com reconstrução (Módulo 51)
// ============================================================================
//
// Reinterpretação musical do *boxcar averager* (integrador de porta —
// equipamento de teste nuclear, ref. Stanford Research SR200 NIM / SR-235):
// um gatilho travado no evento repetitivo, um DELAY posiciona uma janela
// (APERTURE) num ponto do período, o conteúdo da janela é integrado
// (média) e EMPILHADO com as capturas anteriores (média de N). Varrendo o
// delay ao longo do período (SCAN) reconstrói-se a forma de onda inteira
// — o ruído descorrelacionado tende a zero, o sinal coerente "emerge".
//
// Ver o dossiê: `RASGO_MODULAR/dossies/51_boxcar.md`.
//
// - `delay`    (0–1)  onde a janela abre, fração do período medido
// - `aperture` (0–1)  largura da janela, fração do período (→0 = amostra pontual)
// - `average`  (1–64) profundidade N da média (EMA com piso 1/min(N,hits))
// - `scan`     (−1..1) velocidade/direção da varredura do delay (0 = estático)
// - `mode`     (0 follower · 1 reconstruct · 2 oscillator) — 1/2 querem `scan`>0
//              pra o buffer encher em toda a fase
// - `rate`     (Hz)   relógio interno (se `trig` livre) + releitura no modo 2
// - `thresh`   (−1..1) limiar do auto-trigger (edge trigger, se `trig` livre)
// - `geiger`   (0–1)  densidade de um trem de gates de Poisson LIVRE (saída)
// - `blend`    (0–1)  `in` ↔ resultado
//
// Origem do conceito: boxcar de bancada (domínio público). Desvio Rasgo:
// (a) o `scan` que vira OSCILADOR relendo o buffer próprio (`mode 2`);
// (b) o período MEDIDO normaliza os bins — reconstrói mesmo em rubato;
// (c) `geiger` = Poisson livre integrado ao grafo. `geiger` também é um
// modo novo do `NOISE`. Determinístico (a média é aritmética; o `geiger`
// é xorshift semeado). `process()` não aloca.
//
// Modo autônomo: com `in` DESCONECTADO o módulo alimenta a análise com um
// piso de ruído interno de −34 dB (o "ruído de instrumento" que um boxcar
// de verdade enxerga através) — assim `out` já toca uma textura fraca que
// evolui e `geiger` já pulsa ao carregar. Semeado (reprodutível).

namespace rasgo::modular {

class Boxcar final : public Signal {
public:
    Boxcar()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"trig", PortKind::Control, "trig"},
               {"sweep", PortKind::Control, ""},
               {"thr", PortKind::Control, ""}},
              {{"out", PortKind::Audio, ""},
               {"geiger", PortKind::Control, "gate"}},
              {{"delay", 0.0f, 1.0f, 0.0f, ""},
               {"aperture", 0.0f, 1.0f, 0.1f, ""},
               {"average", 1.0f, 64.0f, 8.0f, ""},
               {"scan", -1.0f, 1.0f, 0.0f, ""},
               {"mode", 0.0f, 2.0f, 0.0f, ""},
               {"rate", 0.05f, 40.0f, 2.0f, "Hz"},
               {"thresh", -1.0f, 1.0f, 0.0f, ""},
               {"geiger", 0.0f, 1.0f, 0.0f, ""},
               {"blend", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "BOXCAR"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "BOXCAR", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "boxcar", "", 2.5f, 6.0f, 66.0f);
        p.add(Widget::Kind::Knob, "DLY", "delay", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "APER", "aperture", 24.0f, 28.0f);
        p.add(Widget::Kind::Knob, "AVG", "average", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SCAN", "scan", 58.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 7.0f, 50.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 24.0f, 50.0f);
        p.add(Widget::Kind::Knob, "THRSH", "thresh", 41.0f, 50.0f);
        p.add(Widget::Kind::Knob, "GEI", "geiger", 58.0f, 50.0f);
        p.add(Widget::Kind::Knob, "BLEND", "blend", 7.0f, 72.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 92.0f);
        p.add(Widget::Kind::Jack, "TRIG", "in:trig", 21.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SWP", "in:sweep", 34.0f, 92.0f);
        p.add(Widget::Kind::Jack, "THR", "in:thr", 47.0f, 92.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 8.0f, 114.0f);
        p.add(Widget::Kind::Jack, "GEIG", "out:geiger", 24.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        recon_.assign(kBins, 0.0f);
        hits_.assign(kBins, 0);
        periodEma_ = 0.5 * static_cast<double>(sr_);
        phase_ = 0.0;
        intPhase_ = 0.0;
        prevTrig_ = 0.0f;
        prevIn_ = 0.0f;
        winStartPhase_ = 0.0;
        apLen_ = 1.0;
        winSum_ = 0.0;
        winCount_ = 0;
        winOpen_ = false;
        curBinLo_ = 0;
        curBinSpan_ = 1;
        scanPos_ = 0.0;
        heldOut_ = 0.0f;
        freePhase_ = 0.0;
        poisCd_ = 0.0;
        geigerRemain_ = 0.0;
        rng_ = 0x0B0C0A2E5A1C0FFEULL;
        rngN_ = 0xC0FFEE1234ABCDEFULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        AudioBlock& geig = outputs[1];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* trigB = inputs[1];
        const AudioBlock* sweepB = inputs[2];
        const AudioBlock* thrB = inputs[3];

        const float delayP = clamp01(parameterValue("delay"));
        const float apertureP = clamp01(parameterValue("aperture"));
        const int avg = static_cast<int>(std::lround(
            clampf(parameterValue("average"), 1.0f, 64.0f)));
        const float scanP = clampf(parameterValue("scan"), -1.0f, 1.0f);
        const int mode = static_cast<int>(std::lround(
            clampf(parameterValue("mode"), 0.0f, 2.0f)));
        const float rate = clampf(parameterValue("rate"), 0.05f, 40.0f);
        const float threshP = clampf(parameterValue("thresh"), -1.0f, 1.0f);
        const float geigerP = clamp01(parameterValue("geiger"));
        const float blend = clamp01(parameterValue("blend"));

        const double dp = static_cast<double>(rate) / static_cast<double>(sr_);
        const double pulseLen = 0.005 * static_cast<double>(sr_);
        const double maxPeriod = 4.0 * static_cast<double>(sr_);

        for (std::size_t f = 0; f < frames; ++f) {
            // `in` desconectado → piso de ruído interno (modo autônomo)
            const float inV = inB
                ? inB->at(0, f)
                : (static_cast<float>(static_cast<std::int32_t>(xnN() >> 32))
                   / 2147483648.0f) * 0.02f;
            const float sweepV = sweepB ? sweepB->at(0, f) : 0.0f;
            const float thrCV = thrB ? thrB->at(0, f) : 0.0f;
            const float threshEff = clampf(threshP + thrCV, -1.0f, 1.0f);

            // ---- referência de repetição ----
            bool edge = false;
            if (trigB) {
                const float tg = trigB->at(0, f);
                edge = (prevTrig_ < 0.5f && tg >= 0.5f);
                prevTrig_ = tg;
            } else if (inB) {
                edge = (prevIn_ < threshEff && inV >= threshEff);
                prevIn_ = inV;
            } else {
                intPhase_ += dp;
                if (intPhase_ >= 1.0) { intPhase_ -= 1.0; edge = true; }
            }

            if (edge) {
                closeWindow(avg);   // fecha janela pendente antes de reabrir
                if (phase_ > 4.0) {
                    const double measured =
                        clampd(phase_, 8.0, maxPeriod);
                    periodEma_ += (measured - periodEma_) * 0.25;
                }
                phase_ = 0.0;
                const double delayPos = wrap01(
                    static_cast<double>(delayP) + static_cast<double>(sweepV)
                    + scanPos_);
                winStartPhase_ = delayPos * periodEma_;
                apLen_ = std::max(1.0,
                                  static_cast<double>(apertureP) * periodEma_);
                // a janela cobre as fases [delayPos, delayPos+aperture] →
                // a média entra nos bins desse arco (a abertura É uma
                // suavização em fase); aperture→0 = 1 bin (boxcar pontual)
                curBinLo_ = static_cast<int>(
                    std::lround(delayPos * static_cast<double>(kBins - 1)));
                curBinLo_ %= static_cast<int>(kBins);
                if (curBinLo_ < 0) curBinLo_ += static_cast<int>(kBins);
                curBinSpan_ = std::max(1, static_cast<int>(std::lround(
                    static_cast<double>(apertureP)
                    * static_cast<double>(kBins - 1))));
                if (curBinSpan_ > static_cast<int>(kBins))
                    curBinSpan_ = static_cast<int>(kBins);
                scanPos_ = wrap01(scanPos_
                                  + static_cast<double>(scanP) * (1.0 / 24.0));
            }

            // ---- janela de abertura ----
            if (phase_ >= winStartPhase_
                && phase_ < winStartPhase_ + apLen_) {
                winSum_ += static_cast<double>(inV);
                winCount_ += 1;
                winOpen_ = true;
            } else if (winOpen_) {
                closeWindow(avg);
            }
            phase_ += 1.0;

            // ---- saída ----
            float o = 0.0f;
            if (mode == 0) {
                o = heldOut_;
            } else if (mode == 1) {
                const double ph = periodEma_ > 1.0
                    ? phase_ / periodEma_ : 0.0;
                o = readRecon(wrap01(ph));
            } else {
                freePhase_ += dp;
                freePhase_ -= std::floor(freePhase_);
                o = readRecon(static_cast<float>(freePhase_));
            }
            float y = inV + (o - inV) * blend;
            y = clampf(y, -8.0f, 8.0f);

            // ---- geiger: Poisson LIVRE, semeado ----
            float g = 0.0f;
            if (geigerP > 1.0e-4f) {
                poisCd_ -= 1.0;
                if (poisCd_ <= 0.0) {
                    geigerRemain_ = pulseLen;
                    double u = static_cast<double>(xn() >> 11)
                             * (1.0 / 9007199254740992.0);
                    if (u < 1.0e-12) u = 1.0e-12;
                    const double lambda = 0.5
                        + static_cast<double>(geigerP) * geigerP * 40.0;
                    double interval = -std::log(u)
                        * (static_cast<double>(sr_) / lambda);
                    poisCd_ = interval < 1.0 ? 1.0 : interval;
                }
            }
            if (geigerRemain_ > 0.0) { g = 1.0f; geigerRemain_ -= 1.0; }

            for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = y;
            for (std::size_t c = 0; c < geig.channels(); ++c) geig.at(c, f) = g;
        }
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }
    static double clampd(const double v, const double lo,
                         const double hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float wrap01(const double x) noexcept {
        return static_cast<float>(x - std::floor(x));
    }

    std::uint64_t xn() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return rng_;
    }
    std::uint64_t xnN() noexcept {
        rngN_ ^= rngN_ << 13; rngN_ ^= rngN_ >> 7; rngN_ ^= rngN_ << 17;
        return rngN_;
    }

    // fecha a janela corrente e empilha a média nos bins do arco de abertura
    void closeWindow(const int avg) noexcept {
        if (!winOpen_) return;
        const float m = winCount_ > 0
            ? static_cast<float>(winSum_ / static_cast<double>(winCount_))
            : 0.0f;
        for (int k = 0; k < curBinSpan_; ++k) {
            const std::size_t b = static_cast<std::size_t>(
                (curBinLo_ + k) % static_cast<int>(kBins));
            int& h = hits_[b];
            if (h < avg) ++h;
            float& r = recon_[b];
            r += (m - r) / static_cast<float>(h);
        }
        // `mode 0` segura a ESTIMATIVA promediada nesse ponto de fase
        // (com `average` = 1 é a última janela crua; com N alto, o S&H
        // "sem tremor" — o ruído descorrelato já saiu)
        heldOut_ = recon_[static_cast<std::size_t>(curBinLo_)];
        winSum_ = 0.0;
        winCount_ = 0;
        winOpen_ = false;
    }

    // índice fracionário u·(K−1), interpolação linear entre bins (wrap)
    float readRecon(const float u) const noexcept {
        const float fp = u * static_cast<float>(kBins - 1);
        int i0 = static_cast<int>(fp);
        if (i0 < 0) i0 = 0;
        if (i0 >= static_cast<int>(kBins)) i0 = static_cast<int>(kBins) - 1;
        const int i1 = (i0 + 1) % static_cast<int>(kBins);
        const float fr = fp - static_cast<float>(i0);
        return recon_[static_cast<std::size_t>(i0)] * (1.0f - fr)
             + recon_[static_cast<std::size_t>(i1)] * fr;
    }

    static constexpr std::size_t kBins = 2048;

    std::vector<float> recon_;
    std::vector<int> hits_;
    double periodEma_ = 24000.0;
    double phase_ = 0.0;
    double intPhase_ = 0.0;
    float prevTrig_ = 0.0f, prevIn_ = 0.0f;
    double winStartPhase_ = 0.0;
    double apLen_ = 1.0;
    double winSum_ = 0.0;
    long winCount_ = 0;
    bool winOpen_ = false;
    int curBinLo_ = 0;
    int curBinSpan_ = 1;
    double scanPos_ = 0.0;
    float heldOut_ = 0.0f;
    double freePhase_ = 0.0;
    double poisCd_ = 0.0;
    double geigerRemain_ = 0.0;
    std::uint64_t rng_ = 0x0B0C0A2E5A1C0FFEULL;
    std::uint64_t rngN_ = 0xC0FFEE1234ABCDEFULL;
    float sr_ = 48000.0f;
};

}  // namespace rasgo::modular
