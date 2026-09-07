#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SHIFTER — deslocador de frequência (Módulo 58)
// ============================================================================
//
// Move o espectro INTEIRO por um Δf FIXO EM HERTZ (não em razão). Como
// os intervalos entre parciais deixam de ser harmônicos, o som fica
// metálico / sineiro / inarmônico — e o `feedback` clássico é um drone
// que sobe (ou desce) para sempre (o "barber pole" / shimmer).
//
// O `SHAPE` faz ring-modulation (as bandas SOMA **e** DIFERENÇA,
// simétricas em torno da portadora). Um deslocador de frequência de
// verdade entrega **só uma** banda lateral (SSB — single sideband) — e
// aqui as DUAS saídas ao mesmo tempo: `up` (espectro + Δf) e `down`
// (espectro − Δf). A relação entre elas é o processo (idioma Three
// Sisters).
//
// Ver o dossiê: `RASGO_MODULAR/dossies/58_shifter.md`.
//
// - `shift`      (−2000..2000 Hz, +CV)  o deslocamento. 0 = passa-direto.
// - `feedback`   (−0,95..0,95)          parte da saída volta pra entrada
//                                       → o espectro sobe/desce sem parar
//                                       (`tanh` no laço, nunca explode).
// - `tone`       (−1..1)                inclina o molhado (1 polo).
// - `drift`      (0–1, desvio Rasgo)    wobble lento e SEMEADO no Δf.
// - `mix`        (0–1)                  seco ↔ deslocado (nas duas saídas).
//
// SSB por transformada de Hilbert (FIR de 511 taps, janela de Blackman +
// linha de atraso casada de 255 ≈ 5,3 ms) → modulação em quadratura.
// Rejeição de imagem > 80 dB acima de ~400 Hz, ~20 dB perto de 100 Hz;
// no grave profundo degrada para ring-mod (limite estrutural do FIR —
// como os deslocadores de hardware). `process()` não aloca (o buffer
// vem no `prepare()`). `drift=0` → determinístico.

namespace rasgo::modular {

class Shifter final : public Signal {
public:
    Shifter()
        : Signal(
              {{"in", PortKind::Audio, ""},
               {"shift_mod", PortKind::Control, ""}},
              {{"up", PortKind::Audio, ""},
               {"down", PortKind::Audio, ""}},
              {{"shift", -2000.0f, 2000.0f, 0.0f, "Hz"},
               {"feedback", -0.95f, 0.95f, 0.0f, ""},
               {"tone", -1.0f, 1.0f, 0.0f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""},
               {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "SHIFTER"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "SHIFTER", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "shifter", "", 2.5f, 6.0f, 46.0f);
        p.add(Widget::Kind::Knob, "SHIFT", "shift", 9.0f, 32.0f);
        p.add(Widget::Kind::Knob, "FBK", "feedback", 27.0f, 32.0f);
        p.add(Widget::Kind::Knob, "TONE", "tone", 9.0f, 56.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 27.0f, 56.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 9.0f, 80.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 8.0f, 100.0f);
        p.add(Widget::Kind::Jack, "SFT", "in:shift_mod", 22.0f, 100.0f);
        p.add(Widget::Kind::Jack, "UP", "out:up", 8.0f, 120.0f);
        p.add(Widget::Kind::Jack, "DN", "out:down", 22.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);

        // FIR de Hilbert: h[k] = 2/(π k) para k ímpar, 0 par; janela de
        // Blackman. A saída imaginária = x ∗ h ; a real = x atrasada de M.
        // Só metade dos taps é ≠ 0 — guardamos os não-nulos compactados
        // (offset + coef) pra o laço de `process` não pagar os zeros.
        nTaps_ = 0;
        for (int i = 0; i < kN; ++i) {
            const int k = i - kM;
            if (k % 2 == 0) continue;
            const double bl = 0.42
                - 0.5 * std::cos(2.0 * 3.14159265358979 * i / (kN - 1))
                + 0.08 * std::cos(4.0 * 3.14159265358979 * i / (kN - 1));
            tapOff_[nTaps_] = i;
            tapCoef_[nTaps_] =
                static_cast<float>(2.0 / (3.14159265358979 * k) * bl);
            ++nTaps_;
        }
        buf_.assign(kN, 0.0f);
        wr_ = 0;

        oscPh_ = 0.0;
        fbUp_ = fbDown_ = 0.0f;
        toneZ_ = 0.0f;
        toneCoef_ = 1.0f - std::exp(-2.0f * 3.14159265f * 1400.0f / sr_);
        driftLfo_ = 0.0f;
        rng_ = 0x5F1F7E12C0FFEE01ULL;
        sShift_ = 0.0f;
        primed_ = false;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& outUp = outputs[0];
        AudioBlock& outDn = outputs[1];
        const std::size_t frames = outUp.frames();

        const AudioBlock* inB = inputs[0];
        const AudioBlock* modB = inputs[1];

        const float shiftP = clampf(parameterValue("shift"), -2000.0f, 2000.0f);
        const float fb = clampf(parameterValue("feedback"), -0.95f, 0.95f);
        const float tone = clampf(parameterValue("tone"), -1.0f, 1.0f);
        const float drift = clamp01(parameterValue("drift"));
        const float mix = clamp01(parameterValue("mix"));

        if (!primed_) { sShift_ = shiftP; primed_ = true; }
        const float shiftSmooth = 1.0f - std::exp(-1.0f / (0.004f * sr_));

        for (std::size_t f = 0; f < frames; ++f) {
            const float dry = inB ? inB->at(0, f) : 0.0f;

            // --- Δf efetivo: knob (deslizado) + CV (Hz) + drift semeado ---
            driftLfo_ += (rnd11() * 0.5f - driftLfo_) * 0.00003f;
            const float modHz = modB ? modB->at(0, f) * 1000.0f : 0.0f;
            const float shTarget = clampf(shiftP + modHz, -4000.0f, 4000.0f);
            sShift_ += (shTarget - sShift_) * shiftSmooth;
            const float df = sShift_ * (1.0f + drift * driftLfo_ * 0.5f);

            // --- entrada + realimentação (o "barber pole"): a saída `up`
            //     volta pra entrada → o espectro sobe de novo e de novo,
            //     glissando infinito. `fb` negativo puxa de `down`. ---
            const float fbTap = fb >= 0.0f ? fbUp_ : fbDown_;
            const float x = std::tanh(dry + fbTap * std::fabs(fb));

            // --- Hilbert FIR + atraso casado ---
            buf_[static_cast<std::size_t>(wr_)] = x;
            const int rd = wr_;
            wr_ = wr_ + 1 == kN ? 0 : wr_ + 1;
            int ri = rd - kM;
            if (ri < 0) ri += kN;
            const float re = buf_[static_cast<std::size_t>(ri)];
            float im = 0.0f;
            for (int t = 0; t < nTaps_; ++t) {
                int idx = rd - tapOff_[t];
                if (idx < 0) idx += kN;
                im += tapCoef_[t] * buf_[static_cast<std::size_t>(idx)];
            }

            // --- modulação em quadratura → SSB ---
            oscPh_ += static_cast<double>(df) / sr_;
            oscPh_ -= std::floor(oscPh_);
            const float c = std::cos(6.2831853f * static_cast<float>(oscPh_));
            const float s = std::sin(6.2831853f * static_cast<float>(oscPh_));
            float up = re * c - im * s;      // espectro + df
            float dn = re * c + im * s;      // espectro − df

            // --- tom (1 polo) só no molhado: <0 abafa, >0 realça agudo ---
            toneZ_ += (0.5f * (up + dn) - toneZ_) * toneCoef_;
            if (tone < 0.0f) {
                up += (toneZ_ - up) * (-tone);
                dn += (toneZ_ - dn) * (-tone);
            } else if (tone > 0.0f) {
                up += (up - toneZ_) * tone;
                dn += (dn - toneZ_) * tone;
            }

            // softclip de segurança (feedback alto + tone alto podem
            // passar da unidade); transparente abaixo de ~0,8
            up = softClip(up);
            dn = softClip(dn);
            fbUp_ = up;
            fbDown_ = dn;

            const float oUp = dry + (up - dry) * mix;
            const float oDn = dry + (dn - dry) * mix;
            for (std::size_t ch = 0; ch < outUp.channels(); ++ch)
                outUp.at(ch, f) = oUp;
            for (std::size_t ch = 0; ch < outDn.channels(); ++ch)
                outDn.at(ch, f) = oDn;
        }
    }

private:
    static constexpr int kN = 511;
    static constexpr int kM = 255;

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    static float softClip(const float x) noexcept {
        if (x > 0.8f) return 0.8f + std::tanh(x - 0.8f) * 0.2f;
        if (x < -0.8f) return -0.8f + std::tanh(x + 0.8f) * 0.2f;
        return x;
    }

    float rnd11() noexcept {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 7; rng_ ^= rng_ << 17;
        return static_cast<float>(static_cast<std::int32_t>(rng_ >> 32))
             / 2147483648.0f;
    }

    float sr_ = 48000.0f;
    std::array<float, kN / 2 + 1> tapCoef_{};   // só os taps ≠ 0
    std::array<int, kN / 2 + 1> tapOff_{};
    int nTaps_ = 0;
    std::vector<float> buf_;
    int wr_ = 0;
    double oscPh_ = 0.0;
    float fbUp_ = 0.0f, fbDown_ = 0.0f;
    float toneZ_ = 0.0f, toneCoef_ = 0.2f;
    float driftLfo_ = 0.0f;
    std::uint64_t rng_ = 0x5F1F7E12C0FFEE01ULL;
    float sShift_ = 0.0f;
    bool primed_ = false;
};

}  // namespace rasgo::modular
