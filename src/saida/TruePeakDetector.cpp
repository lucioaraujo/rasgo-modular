// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TruePeakDetector.cpp (saida-ref 1.2,
// 17/11/2026, sha256 3c4fb43aa0ffd7d7…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/TruePeakDetector.h"

#include <algorithm>
#include <cmath>

namespace rasgo::modular::saida
{
namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double kaiserBeta = 9.0;

double sinc(double value) noexcept
{
    if (std::abs(value) < 1.0e-12) return 1.0;
    return std::sin(pi * value) / (pi * value);
}

// modified Bessel function of the first kind, order 0 (power series)
double besselI0(double x) noexcept
{
    double sum = 1.0, term = 1.0;
    const auto quarter = x * x * 0.25;
    for (int k = 1; k < 64 && term > sum * 1.0e-17; ++k)
    {
        term *= quarter / (static_cast<double>(k) * static_cast<double>(k));
        sum += term;
    }
    return sum;
}
}

void TruePeakDetector::prepare(double) noexcept
{
    // the 4x interpolation filter does not depend on the rate; the rate is
    // kept in the signature for parity with the original
    constexpr auto totalTaps = oversamplingFactor * tapsPerPhase;
    constexpr double half = static_cast<double>(totalTaps) * 0.5;
    std::array<double, totalTaps> prototype {};
    for (std::size_t tap = 0; tap < totalTaps; ++tap)
    {
        // centred on a whole input sample, so phase 0 is a pure delay and the
        // other phases fall at 1/4, 2/4 and 3/4 of a sample
        const auto position = static_cast<double>(tap) - half;
        const auto ratio = position / half;
        const auto window = besselI0(kaiserBeta * std::sqrt(std::max(0.0, 1.0 - ratio * ratio)))
            / besselI0(kaiserBeta);
        prototype[tap] = sinc(position / oversamplingFactor) * window;
    }
    // Normalising each polyphase branch independently guarantees unity DC
    // gain at every inserted phase.
    for (std::size_t phase = 1; phase < oversamplingFactor; ++phase)
    {
        double branchSum = 0.0;
        for (std::size_t tap = 0; tap < tapsPerPhase; ++tap)
            branchSum += prototype[phase + tap * oversamplingFactor];
        // tap k multiplies the input k samples old; stored oldest-first
        for (std::size_t tap = 0; tap < tapsPerPhase; ++tap)
            coefficients[phase - 1][tapsPerPhase - 1 - tap]
                = static_cast<float>(prototype[phase + tap * oversamplingFactor] / branchSum);
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
    history[writeIndex + tapsPerPhase] = input;
    writeIndex = (writeIndex + 1) % tapsPerPhase;
    // the newest tapsPerPhase samples, oldest first
    const float* window = history.data() + writeIndex;
    // phase 0: the input sample itself, at the filters' delay
    float peak = std::abs(window[tapsPerPhase - 1 - tapsPerPhase / 2]);
    // eight partial sums in float: the compiler vectorises them without
    // reordering a single accumulator (no fast-math needed); the error is
    // ~1e-7, far below the 0.01 dB this meter resolves
    for (const auto& branch : coefficients)
    {
        std::array<float, 8> partial {};
        for (std::size_t tap = 0; tap < tapsPerPhase; tap += 8)
            for (std::size_t lane = 0; lane < 8; ++lane)
                partial[lane] += branch[tap + lane] * window[tap + lane];
        const auto interpolated = ((partial[0] + partial[1]) + (partial[2] + partial[3]))
            + ((partial[4] + partial[5]) + (partial[6] + partial[7]));
        peak = std::max(peak, std::abs(interpolated));
    }
    return peak;
}
}
