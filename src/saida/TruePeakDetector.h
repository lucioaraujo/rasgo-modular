// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TruePeakDetector.h (saida-ref 1.1,
// 16/11/2026, sha256 c284eb7388b378ad…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace rasgo::modular::saida
{
// Four-times polyphase FIR true-peak detector (ITU-R BS.1770-4 annex 2).
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
    static constexpr std::size_t tapsPerPhase = 16;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float processSample(float input) noexcept;
    [[nodiscard]] std::size_t latencySamples() const noexcept { return tapsPerPhase / 2; }

private:
    std::array<std::array<float, tapsPerPhase>, oversamplingFactor> coefficients {};
    std::array<float, tapsPerPhase> history {};
    std::size_t writeIndex = 0;
};
}
