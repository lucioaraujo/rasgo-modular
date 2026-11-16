// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TakeAnalysis.h (saida-ref 1.1,
// 16/11/2026, sha256 e0d477c83979cde7…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "saida/LoudnessMeter.h"
#include "saida/TruePeakDetector.h"

#include <array>
#include <cstdint>

namespace rasgo::modular::saida
{
// The measurement of a recorded take (2026-10-21; the author: "qualidade de
// áudio também, excelência de áudio"), following what Navalha 2 reports for
// its masters: sample peak, true peak (4x, BS.1770), integrated and maximum
// short-term loudness (BS.1770 / EBU R128, the Aquorbium's own meter), the
// loudness range (LRA, EBU Tech 3342: the short-term values every 100 ms,
// gated at -70 LUFS and 20 LU under their mean, P95 - P10), RMS,
// DC offset, samples at or over full scale, and non-finite samples, which are
// written as digital silence. It runs on the recorder's writer thread, never
// on the audio thread; it holds no growing storage, so a take of an hour
// costs the same memory as a take of a second.
class TakeAnalysis final
{
public:
    struct Result
    {
        std::uint64_t frames = 0;
        float samplePeakDb = -240.0f;
        float truePeakDb = -240.0f;
        float integratedLufs = LoudnessMeter::silenceLufs;
        float maxShortTermLufs = LoudnessMeter::silenceLufs;
        float loudnessRange = 0.0f;  // LRA, LU (EBU Tech 3342)
        bool loudnessRangeReady = false;
        float rmsDb = -240.0f;
        float dcLeft = 0.0f;
        float dcRight = 0.0f;
        std::uint64_t overFullScale = 0;
        std::uint64_t nonFinite = 0;
    };

    void prepare(double sampleRate) noexcept;
    // returns the sample as written (non-finite becomes 0)
    void push(float& left, float& right) noexcept;
    [[nodiscard]] Result result() const noexcept;

private:
    LoudnessMeter loudness {};
    TruePeakDetector peakLeft {};
    TruePeakDetector peakRight {};
    std::uint64_t frames = 0;
    std::uint64_t over = 0;
    std::uint64_t invalid = 0;
    float samplePeak = 0.0f;
    float truePeak = 0.0f;
    float maxShortTerm = LoudnessMeter::silenceLufs;
    double sumSquares = 0.0;
    double sumLeft = 0.0;
    double sumRight = 0.0;
    // the short-term values in 0.1 LU bins from -70 to +10 LUFS, for the LRA
    static constexpr float histogramFloor = -70.0f;
    static constexpr std::size_t histogramBins = 800;
    std::array<std::uint32_t, histogramBins> shortTermHistogram {};
    double shortTermEnergy = 0.0;
    std::uint64_t shortTermCount = 0;
    std::uint32_t shortTermCountdown = 0;
    std::uint32_t shortTermInterval = 4800;
};
}
