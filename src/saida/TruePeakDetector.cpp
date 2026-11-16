// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TruePeakDetector.cpp (saida-ref 1.1,
// 16/11/2026, sha256 46303b915ead999b…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/TruePeakDetector.h"

#include <algorithm>
#include <cmath>

namespace rasgo::modular::saida
{
namespace
{
constexpr double pi = 3.14159265358979323846;

double sinc(double value) noexcept
{
    if (std::abs(value) < 1.0e-12) return 1.0;
    return std::sin(pi * value) / (pi * value);
}
}

void TruePeakDetector::prepare(double) noexcept
{
    // the 4x interpolation filter does not depend on the rate; the rate is
    // kept in the signature for parity with the original
    constexpr auto totalTaps = oversamplingFactor * tapsPerPhase;
    constexpr double centre = (static_cast<double>(totalTaps) - 1.0) * 0.5;
    std::array<double, totalTaps> prototype {};
    for (std::size_t tap = 0; tap < totalTaps; ++tap)
    {
        const auto position = static_cast<double>(tap) - centre;
        // Four-term Blackman-Harris window: strong stop-band rejection keeps
        // near-Nyquist content from becoming phase-dependent false peaks.
        const auto phase = 2.0 * pi * static_cast<double>(tap) / static_cast<double>(totalTaps - 1);
        const auto window = 0.35875 - 0.48829 * std::cos(phase) + 0.14128 * std::cos(2.0 * phase)
            - 0.01168 * std::cos(3.0 * phase);
        prototype[tap] = sinc(position / oversamplingFactor) * window;
    }
    // Normalising each polyphase branch independently guarantees unity DC
    // gain at every inserted phase, including the half-sample-centred FIR.
    for (std::size_t phase = 0; phase < oversamplingFactor; ++phase)
    {
        double branchSum = 0.0;
        for (std::size_t tap = 0; tap < tapsPerPhase; ++tap)
            branchSum += prototype[phase + tap * oversamplingFactor];
        for (std::size_t tap = 0; tap < tapsPerPhase; ++tap)
            coefficients[phase][tap] = static_cast<float>(prototype[phase + tap * oversamplingFactor] / branchSum);
    }
    reset();
}

void TruePeakDetector::reset() noexcept
{
    history.fill(0.0f);
    writeIndex = 0;
}

float TruePeakDetector::processSample(float input) noexcept
{
    if (!std::isfinite(input)) input = 0.0f;
    history[writeIndex] = input;
    writeIndex = (writeIndex + 1) % history.size();
    float peak = 0.0f;
    for (const auto& branch : coefficients)
    {
        double interpolated = 0.0;
        auto readIndex = writeIndex;
        for (std::size_t tap = 0; tap < tapsPerPhase; ++tap)
        {
            readIndex = readIndex == 0 ? history.size() - 1 : readIndex - 1;
            interpolated += static_cast<double>(branch[tap]) * history[readIndex];
        }
        peak = std::max(peak, static_cast<float>(std::abs(interpolated)));
    }
    return peak;
}
}
