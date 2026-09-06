#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// DelayPitchShifter — deslocamento de altura por linha de atraso janelada
// ============================================================================
//
// Duas leituras de uma linha de atraso de 10 ms, janeladas por `sin` e
// defasadas meia fase; o ponto de leitura anda a uma taxa proporcional a
// `(1 − razão)` — clássico "delay-line pitch shifter". Interpolação
// polinomial de 4 pontos idêntica ao `vd~` do Pure Data. Passa-alta de
// 5 Hz na saída (tira o DC do janelamento).
//
// ORIGEM DO ALGORITMO: exemplo `3.audio.examples/G09.pitchshift.pd` do
// Pure Data (Miller Puckette) — descrito em "Theory and Technique of
// Electronic Music". Domínio público.
//
// PORTE: a implementação C++ vem de `RASGO/NAVALHA2_JUCE/src/core/
// HeritagePitch.cpp` (`LegacyPitchChannel`) — Navalha original de
// **Glerm Soares**, reescrita JUCE/C++ **Navalha 2 de Lúcio Araújo**,
// GPL-3.0-or-later (compatível com o AGPLv3-or-later do RASGO Modular).
// Adaptado aqui: `setSemitones(int)` → `setRatio(float)` contínuo (o
// RASGO quer oitavas/cents, não os 24 passos históricos); mesmo núcleo.
// Ver `dossies/ESTUDO_audio_sampling.md §2`.
//
// Header-only, sem dependência. `prepare()` aloca a linha (~480 amostras
// a 48 k); `process()` não aloca.

namespace rasgo::modular {

class DelayPitchShifter {
public:
    void prepare(const float sampleRate) {
        rate_ = std::max(1.0f, sampleRate);
        const std::size_t n =
            static_cast<std::size_t>(std::ceil(rate_ * 0.010f)) + 4;
        delay_.assign(n, 0.0f);
        hpCoef_ = std::exp(-2.0f * 3.14159265f * 5.0f / rate_);
        reset();
    }

    void reset() noexcept {
        std::fill(delay_.begin(), delay_.end(), 0.0f);
        writeIndex_ = 0;
        phase_ = 0.0f;
        hpPrevIn_ = 0.0f;
        hpPrevOut_ = 0.0f;
    }

    // razão de frequência: 2^oitavas. 1 = sem deslocamento. Clampada a
    // [0,25; 4] (±2 oitavas) — além disso o artefato domina.
    void setRatio(const double ratio) noexcept {
        const double r = ratio < 0.25 ? 0.25 : (ratio > 4.0 ? 4.0 : ratio);
        phaseInc_ = (1.0 - r) / (0.010 * static_cast<double>(rate_));
    }

    // O núcleo roda em `double` (como o original do Navalha 2 — o passeio
    // de fase acumula por horas; `float` degrada a janela e vira ruído).
    [[nodiscard]] float process(const float input) noexcept {
        if (delay_.empty()) return input;

        delay_[writeIndex_] = input;
        double secondPhase = phase_ + 0.5;
        if (secondPhase >= 1.0) secondPhase -= 1.0;

        const double wA = std::sin(3.141592653589793 * phase_);
        const double wB = std::sin(3.141592653589793 * secondPhase);
        const double rate10 = static_cast<double>(rate_) * 0.010;
        const double shifted =
            readDelay(phase_ * rate10) * wA
            + readDelay(secondPhase * rate10) * wB;

        writeIndex_ = (writeIndex_ + 1) % delay_.size();
        phase_ += phaseInc_;
        phase_ -= std::floor(phase_);

        const double filtered =
            shifted - hpPrevIn_ + static_cast<double>(hpCoef_) * hpPrevOut_;
        hpPrevIn_ = shifted;
        hpPrevOut_ = filtered;
        return static_cast<float>(filtered);
    }

private:
    // interpolação de 4 pontos do `vd~` do Pure Data
    [[nodiscard]] double readDelay(const double delaySamples) const noexcept {
        const double size = static_cast<double>(delay_.size());
        double position = static_cast<double>(writeIndex_)
            - std::min(std::max(delaySamples, 0.0), size - 2.0);
        while (position < 0.0) position += size;

        const std::size_t bi =
            static_cast<std::size_t>(position) % delay_.size();
        const double a = delay_[(bi + delay_.size() - 1) % delay_.size()];
        const double b = delay_[bi];
        const double c = delay_[(bi + 1) % delay_.size()];
        const double d = delay_[(bi + 2) % delay_.size()];
        const double fr = position - std::floor(position);

        const double cMinusB = c - b;
        return b + fr * (cMinusB
            - (1.0 / 6.0) * (1.0 - fr)
                * ((d - a - 3.0 * cMinusB) * fr + d + 2.0 * a - 3.0 * b));
    }

    std::vector<float> delay_;
    std::size_t writeIndex_ = 0;
    float rate_ = 48000.0f;
    double phase_ = 0.0;
    double phaseInc_ = 0.0;
    double hpPrevIn_ = 0.0;
    double hpPrevOut_ = 0.0;
    float hpCoef_ = 0.999f;
};

}  // namespace rasgo::modular
