#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>

// ============================================================================
// TEMPO — o andamento que respira (Módulo 106)
// ============================================================================
//
// Proposta `dossies/PROPOSTA_forma_e_andamento.md`; "pode implementar"
// (autor, 21/11/2026). Dá ao CLOCK um andamento que se move: a saída RATE
// vai ao jack RATE do CLOCK (1 V = ×2). Sozinho já soa (arco de frase e
// rubato); com o FORM ligado (TENS, SECT, END→CODA), segue a mesma forma.
//
// A razão de andamento é o produto de camadas, alisado por uma mola de 2ª
// ordem (Rasgo Synth Asterismus: ω 1,1·MOLA, ζ 0,62 — sem degrau):
//   - salto-e-espera: a cada SECT, um passo de andamento sorteado (±6 %)
//     que espera até a próxima troca (rasgo-synth);
//   - accelerando: 1 + ACCEL·tensão (rumo ao clímax);
//   - arco de frase (N. Todd, 1992): mais rápido no meio da frase, de média
//     zero (a frase dura o mesmo); ARCH também sai como CV, para a dinâmica;
//   - rubato em série de Fourier (4 harmônicos, 1/k²; Asterismus), de média
//     zero na frase: o tempo emprestado é devolvido;
//   - ritardando final, quando CODA sobe: o modelo do corredor que para
//     (A. Friberg & J. Sundberg, JASA 1999): v(x) = [1 + (wᵠ − 1)·x]^(1/q),
//     w = FINAL, q = SHAPE, x de 0 a 1 em RIT segundos; no fim, END sobe;
//   - fermata: FERM segura o relógio (1/16 do andamento) por FERMT tempos.
// MODE: 0 ESTRITO (sem arco nem rubato) · 1 ARCO · 2 ELÁSTICO (tudo).
// A frase conta pulsos do CLK (PHR tempos); sem CLK, conta 2 tempos/s.

namespace rasgo::modular {

class Tempo final : public Signal {
public:
    Tempo()
        : Signal(
              {{"tension", PortKind::Control, ""},
               {"section", PortKind::Control, "trig"},
               {"coda", PortKind::Control, "gate"},
               {"fermata", PortKind::Control, "trig"},
               {"clock", PortKind::Control, "trig"}},
              {{"rate", PortKind::Control, "v/oct"},
               {"arch", PortKind::Control, ""},
               {"beat", PortKind::Control, "trig"},
               {"end", PortKind::Control, "gate"}},
              {{"accel", 0.0f, 0.3f, 0.1f, ""},
               {"final", 0.3f, 1.0f, 0.6f, ""},
               {"shape", 1.0f, 4.0f, 2.5f, ""},
               {"ritard", 1.0f, 60.0f, 8.0f, "s"},
               {"phrase", 2.0f, 32.0f, 8.0f, ""},
               {"arch_depth", 0.0f, 0.15f, 0.04f, ""},
               {"rubato", 0.0f, 1.0f, 0.3f, ""},
               {"spring", 0.1f, 2.0f, 1.0f, ""},
               {"fermata_len", 0.5f, 4.0f, 1.0f, ""},
               {"mode", 0.0f, 2.0f, 2.0f, ""},
               {"seed", 1.0f, 65535.0f, 1.0f, ""}}) {}

    std::string type() const override { return "TEMPO"; }

    Panel panel() const override {
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "TEMPO", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "tempo", "", 2.5f, 6.0f, 76.0f);
        p.add(Widget::Kind::Knob, "ACCEL", "accel", 7.0f, 30.0f);
        p.add(Widget::Kind::Knob, "FINAL", "final", 21.0f, 30.0f);
        p.add(Widget::Kind::Knob, "SHAPE", "shape", 35.0f, 30.0f);
        p.add(Widget::Kind::Knob, "RIT", "ritard", 49.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 63.0f, 30.0f);
        p.add(Widget::Kind::Knob, "PHR", "phrase", 7.0f, 52.0f);
        p.add(Widget::Kind::Knob, "ARCH", "arch_depth", 21.0f, 52.0f);
        p.add(Widget::Kind::Knob, "RUBATO", "rubato", 35.0f, 52.0f);
        p.add(Widget::Kind::Knob, "MOLA", "spring", 49.0f, 52.0f);
        p.add(Widget::Kind::Knob, "FERMT", "fermata_len", 63.0f, 52.0f);
        p.add(Widget::Kind::Knob, "SEED", "seed", 7.0f, 74.0f);
        p.add(Widget::Kind::Jack, "TENS", "in:tension", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "SECT", "in:section", 17.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CODA", "in:coda", 29.0f, 96.0f);
        p.add(Widget::Kind::Jack, "FERM", "in:fermata", 41.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 53.0f, 96.0f);
        p.add(Widget::Kind::Jack, "RATE", "out:rate", 5.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ARCH", "out:arch", 17.0f, 116.0f);
        p.add(Widget::Kind::Jack, "BEAT", "out:beat", 29.0f, 116.0f);
        p.add(Widget::Kind::Jack, "END", "out:end", 41.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        seededWith_ = -1;
        ratio_ = 1.0; vel_ = 0.0; step_ = 1.0;
        phrasePos_ = 0.0; beatPhase_ = 0.0; clockPeriod_ = 0.5 * sr_; sinceClock_ = 0.0;
        codaX_ = -1.0; fermLeft_ = 0.0; beatTrig_ = 0;
        prevSect_ = prevCoda_ = prevFerm_ = prevClock_ = 0.0f;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const int seed = static_cast<int>(parameterValue("seed") + 0.5f);
        if (seed != seededWith_) {
            seededWith_ = seed;
            rng_ = 0xA0761D6478BD642FULL ^ (static_cast<std::uint64_t>(seed) * 0x9E3779B97F4A7C15ULL);
            for (int k = 0; k < 4; ++k) rubPhase_[k] = 6.283185307 * uni();
        }
        const int mode = clampi(static_cast<int>(std::lround(parameterValue("mode"))), 0, 2);
        const double accel = parameterValue("accel");
        const double fin = clampd(parameterValue("final"), 0.3, 1.0);
        const double q = clampd(parameterValue("shape"), 1.0, 4.0);
        const double ritSec = clampd(parameterValue("ritard"), 1.0, 60.0);
        const double phraseBeats = clampd(parameterValue("phrase"), 2.0, 32.0);
        const double archD = mode >= 1 ? clampd(parameterValue("arch_depth"), 0.0, 0.15) : 0.0;
        const double rub = mode == 2 ? clampd(parameterValue("rubato"), 0.0, 1.0) * 0.06 : 0.0;
        const double w = 1.1 * clampd(parameterValue("spring"), 0.1, 2.0);   // rad/s
        const double zeta = 0.62;
        const double fermSec = clampd(parameterValue("fermata_len"), 0.5, 4.0);
        const AudioBlock* tens = inputs[0];
        const AudioBlock* sect = inputs[1];
        const AudioBlock* coda = inputs[2];
        const AudioBlock* ferm = inputs[3];
        const AudioBlock* clk = inputs[4];
        const double dt = 1.0 / sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            if (sect) {
                const float v = sect->at(0, f);
                if (prevSect_ < 0.5f && v >= 0.5f) {
                    step_ = 0.94 + 0.12 * uni();     // salto-e-espera
                    phrasePos_ = 0.0;                // a seção nova abre uma frase
                }
                prevSect_ = v;
            }
            if (coda) {
                const float v = coda->at(0, f);
                if (prevCoda_ < 0.5f && v >= 0.5f) { codaX_ = 0.0; codaFrom_ = ratio_; }
                if (prevCoda_ >= 0.5f && v < 0.5f) codaX_ = -1.0;
                prevCoda_ = v;
            }
            if (ferm) {
                const float v = ferm->at(0, f);
                if (prevFerm_ < 0.5f && v >= 0.5f) fermLeft_ = fermSec * clockPeriod_ / sr_;
                prevFerm_ = v;
            }
            bool tick = false;
            if (clk) {
                const float c = clk->at(0, f);
                if (prevClock_ < 0.5f && c >= 0.5f) {
                    tick = true;
                    if (sinceClock_ > 1.0) clockPeriod_ = sinceClock_;
                    sinceClock_ = 0.0;
                }
                prevClock_ = c;
                sinceClock_ += 1.0;
            }

            // a frase: pulsos do CLK, ou 2 tempos/s pela razão atual
            if (clk) { if (tick) phrasePos_ += 1.0 / phraseBeats; }
            else phrasePos_ += 2.0 * ratio_ * dt / phraseBeats;
            if (phrasePos_ >= 1.0) phrasePos_ -= std::floor(phrasePos_);
            const double phi = phrasePos_;
            const double archShape = std::sin(3.141592653589793 * phi);   // 0 → 1 → 0
            // Todd: mais rápido no meio, média zero (∫ sin(πφ) = 2/π)
            const double arch = 1.0 + archD * (archShape - 2.0 / 3.141592653589793);
            double rubSum = 0.0;
            for (int k = 1; k <= 4; ++k)
                rubSum += std::sin(6.283185307 * k * phi + rubPhase_[k - 1]) / (k * k);
            const double rubato = 1.0 + rub * rubSum / 1.42;   // Σ1/k² (k ≤ 4) ≈ 1,42

            const double t = tens ? clampd(tens->at(0, f), 0.0, 1.0) : 0.0;
            double target = step_ * (1.0 + accel * t) * arch * rubato;

            // ritardando final (corredor que para): sai do andamento em que
            // estava e desce sem voltar até FINAL — sem arco nem rubato,
            // como num final; a curva já é suave, a mola fica de fora
            bool ended = false;
            if (codaX_ >= 0.0) {
                codaX_ = std::min(1.0, codaX_ + dt / ritSec);
                const double v = std::pow(1.0 + (std::pow(fin, q) - 1.0) * codaX_, 1.0 / q);
                const double from = std::max(codaFrom_, fin);
                ratio_ = fin + (from - fin) * (fin < 1.0 ? (v - fin) / (1.0 - fin) : 1.0);
                vel_ = 0.0;
                ended = codaX_ >= 1.0;
            } else {
                // mola de 2ª ordem: a razão anda até o alvo sem degrau
                const double acc = w * w * (target - ratio_) - 2.0 * zeta * w * vel_;
                vel_ += acc * dt;
                ratio_ += vel_ * dt;
            }
            double out = ratio_;
            if (fermLeft_ > 0.0) { fermLeft_ -= dt; out = 1.0 / 16.0; }
            out = clampd(out, 1.0 / 16.0, 4.0);
            ratio_out_ = out;

            // BEAT: o pulso já deformado (para quem não passa pelo CLOCK)
            const double baseHz = sr_ / std::max(1.0, clockPeriod_);
            beatPhase_ += baseHz * out * dt;
            if (beatPhase_ >= 1.0) { beatPhase_ -= 1.0; beatTrig_ = static_cast<int>(0.005f * sr_); }

            const float rateV = static_cast<float>(std::log2(out));
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = rateV;
                outputs[1].at(c, f) = static_cast<float>(archShape);
                outputs[2].at(c, f) = beatTrig_ > 0 ? 1.0f : 0.0f;
                outputs[3].at(c, f) = ended ? 1.0f : 0.0f;
            }
            if (beatTrig_ > 0) --beatTrig_;
        }
    }

    // para os testes e o visor: a razão de andamento atual
    double ratio() const noexcept { return ratio_out_; }

private:
    double uni() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<double>((x >> 11) & ((1ULL << 53) - 1)) / static_cast<double>(1ULL << 53);
    }
    static int clampi(const int v, const int lo, const int hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
    static double clampd(const double v, const double lo, const double hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 1;
    int seededWith_ = -1, beatTrig_ = 0;
    double rubPhase_[4] = {0.0, 0.0, 0.0, 0.0};
    double ratio_ = 1.0, vel_ = 0.0, step_ = 1.0, ratio_out_ = 1.0;
    double phrasePos_ = 0.0, beatPhase_ = 0.0, clockPeriod_ = 24000.0, sinceClock_ = 0.0;
    double codaX_ = -1.0, codaFrom_ = 1.0, fermLeft_ = 0.0;
    float prevSect_ = 0.0f, prevCoda_ = 0.0f, prevFerm_ = 0.0f, prevClock_ = 0.0f;
};

}  // namespace rasgo::modular
