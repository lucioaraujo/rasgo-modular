// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TruePeakDetector.h (saida-ref 1.2,
// 17/11/2026, sha256 e9f1544f5f36728d…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace rasgo::modular::saida
{
// Four-times polyphase FIR true-peak detector (ITU-R BS.1770-4 annex 2).
//
// saida-ref 1.2 (17/11/2026): 48 taps per phase, Kaiser (beta 9), phase 0
// exact. The 1.1 filter (16 taps, Blackman-Harris) lost 0.7 dB at 17 kHz and
// 2.7 dB at 19 kHz (44.1 kHz), so bright material read up to 1 dB under its
// real true peak: a Batruc take delivered at "-1.00 dBTP" measured -0.2 dBTP
// (ffmpeg ebur128) and 0.0 dBTP (soxr, 8x). This one is flat (< 0.01 dB) to
// 19 kHz and -0.1 dB at 20 kHz at 44.1 kHz, and reads that take at +0.06.
//
// Provenance: copied on 2026-10-21 from the author's Navalha 2
// (RASGO/PROJETOS_AUTORAIS/NAVALHA2_JUCE, commit b5343c4, files
// src/core/TruePeakDetector.{h,cpp}, sha256 725d14d8... / cd0bf4ac...,
// GPL-3.0-or-later, same author), where it is checked against the EBU
// Tech 3341 true-peak cases. Adapted: the aquorbium namespace, and prepare()
// is noexcept (an invalid rate falls back to 48 kHz instead of throwing),
// because the Aquorbium runs it on the recorder's writer thread.
class TruePeakDetector final
{
public:
    static constexpr std::size_t oversamplingFactor = 4;
    static constexpr std::size_t tapsPerPhase = 48;  // a multiple of 8 (the partial sums)
    static_assert(tapsPerPhase % 8 == 0);

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float processSample(float input) noexcept;
    [[nodiscard]] std::size_t latencySamples() const noexcept { return tapsPerPhase / 2; }

private:
    // the three in-between phases, stored oldest-first for a contiguous dot
    // product; phase 0 is the input itself, tapsPerPhase / 2 samples late
    std::array<std::array<float, tapsPerPhase>, oversamplingFactor - 1> coefficients {};
    // the history twice over, so the newest tapsPerPhase samples are always
    // contiguous (no wrap inside the dot product)
    std::array<float, 2 * tapsPerPhase> history {};
    std::size_t writeIndex = 0;
};
}
