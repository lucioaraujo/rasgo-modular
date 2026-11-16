// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/LoudnessMeter.cpp (saida-ref 1.1,
// 16/11/2026, sha256 ac55ad7f9e6a5064…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/LoudnessMeter.h"

#include "saida/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rasgo::modular::saida
{
namespace
{
constexpr double pi = 3.14159265358979323846;
}

void LoudnessMeter::prepare(double sampleRate) noexcept
{
    preparedRate = numeric::sampleRate(sampleRate);
    sliceLength = static_cast<std::size_t>(std::max(
        1.0, std::round(preparedRate * 0.01)));
    designKWeighting();
    reset();
}

void LoudnessMeter::reset() noexcept
{
    filterStates = {};
    sliceEnergy = 0.0;
    sliceFill = 0;
    recentSlices.fill(0.0);
    recentWrite = 0;
    recentCount = 0;
    slicesSinceBlock = 0;
    histogramEnergy.fill(0.0);
    histogramCount.fill(0);
    absoluteGateEnergy = 0.0;
    absoluteGateBlocks = 0;
    integratedBlockCount = 0;
    integratedLufs = silenceLufs;
}

void LoudnessMeter::push(float left, float right) noexcept
{
    if (!std::isfinite(left)) left = 0.0f;
    if (!std::isfinite(right)) right = 0.0f;

    const auto weightedLeft = filter(0, static_cast<double>(left));
    const auto weightedRight = filter(1, static_cast<double>(right));
    sliceEnergy += weightedLeft * weightedLeft
        + weightedRight * weightedRight;
    if (++sliceFill < sliceLength) return;

    const auto meanSquare = sliceEnergy / static_cast<double>(sliceLength);
    sliceEnergy = 0.0;
    sliceFill = 0;
    recentSlices[recentWrite] = meanSquare;
    recentWrite = (recentWrite + 1u) % shortTermSlices;
    if (recentCount < shortTermSlices) ++recentCount;

    // the gating blocks: 400 ms, every 100 ms, as BS.1770 defines them
    if (++slicesSinceBlock >= blockHopSlices)
    {
        slicesSinceBlock = 0;
        if (recentCount >= momentarySlices)
            registerIntegratedBlock(windowMeanSquare(momentarySlices));
    }
}

LoudnessMeter::Readings LoudnessMeter::readings() const noexcept
{
    Readings result;
    result.momentaryReady = recentCount >= momentarySlices;
    result.shortTermReady = recentCount >= shortTermSlices;
    if (result.momentaryReady)
        result.momentaryLufs = windowLufs(momentarySlices);
    if (result.shortTermReady)
        result.shortTermLufs = windowLufs(shortTermSlices);
    result.integratedBlocks = integratedBlockCount;
    result.integratedLufs = integratedLufs;
    return result;
}

void LoudnessMeter::registerIntegratedBlock(double energy) noexcept
{
    if (integratedBlockCount != std::numeric_limits<std::uint64_t>::max())
        ++integratedBlockCount;
    const auto loudness = energyToLufs(energy);
    if (loudness <= silenceLufs) return;

    const auto binValue = (loudness - histogramMinimumLufs)
        / histogramStepLufs;
    const auto bin = std::min(histogramBins - 1u,
        static_cast<std::size_t>(std::max(0.0f, binValue)));
    histogramEnergy[bin] += energy;
    if (histogramCount[bin] != std::numeric_limits<std::uint64_t>::max())
        ++histogramCount[bin];
    absoluteGateEnergy += energy;
    if (absoluteGateBlocks != std::numeric_limits<std::uint64_t>::max())
        ++absoluteGateBlocks;
    updateIntegratedLoudness();
}

void LoudnessMeter::updateIntegratedLoudness() noexcept
{
    if (absoluteGateBlocks == 0) return;
    const auto relativeGate = energyToLufs(absoluteGateEnergy
        / static_cast<double>(absoluteGateBlocks)) - 10.0f;
    double gatedEnergy = 0.0;
    std::uint64_t gatedBlocks = 0;
    for (std::size_t bin = 0; bin < histogramBins; ++bin)
    {
        const auto binLoudness = histogramMinimumLufs
            + (static_cast<float>(bin) + 0.5f) * histogramStepLufs;
        if (binLoudness <= relativeGate) continue;
        gatedEnergy += histogramEnergy[bin];
        gatedBlocks += histogramCount[bin];
    }
    if (gatedBlocks != 0)
        integratedLufs = energyToLufs(gatedEnergy
            / static_cast<double>(gatedBlocks));
}

double LoudnessMeter::windowMeanSquare(std::size_t slices) const noexcept
{
    const auto available = std::min(recentCount, slices);
    if (available == 0) return 0.0;
    double sum = 0.0;
    for (std::size_t offset = 0; offset < available; ++offset)
    {
        const auto index = (recentWrite + shortTermSlices - 1u - offset)
            % shortTermSlices;
        sum += recentSlices[index];
    }
    return sum / static_cast<double>(available);
}

float LoudnessMeter::windowLufs(std::size_t slices) const noexcept
{
    return energyToLufs(windowMeanSquare(slices));
}

float LoudnessMeter::energyToLufs(double energy) noexcept
{
    if (!(energy > 0.0) || !std::isfinite(energy)) return silenceLufs;
    return std::max(silenceLufs,
        static_cast<float>(-0.691 + 10.0 * std::log10(energy)));
}

void LoudnessMeter::designKWeighting() noexcept
{
    // ITU-R BS.1770 K-weighting prototypes, bilinear-transformed at the
    // active sample rate. This avoids silently reusing 48 kHz coefficients.
    {
        constexpr double frequency = 1681.974450955533;
        constexpr double gainDb = 3.999843853973347;
        constexpr double q = 0.7071752369554196;
        const auto k = std::tan(pi * frequency / preparedRate);
        const auto highGain = std::pow(10.0, gainDb / 20.0);
        const auto bandGain = std::pow(highGain, 0.4996667741545416);
        const auto denominator = 1.0 + k / q + k * k;
        shelf.b0 = (highGain + bandGain * k / q + k * k) / denominator;
        shelf.b1 = 2.0 * (k * k - highGain) / denominator;
        shelf.b2 = (highGain - bandGain * k / q + k * k) / denominator;
        shelf.a1 = 2.0 * (k * k - 1.0) / denominator;
        shelf.a2 = (1.0 - k / q + k * k) / denominator;
    }
    {
        constexpr double frequency = 38.13547087602444;
        constexpr double q = 0.5003270373238773;
        const auto k = std::tan(pi * frequency / preparedRate);
        const auto denominator = 1.0 + k / q + k * k;
        highPass.b0 = 1.0;
        highPass.b1 = -2.0;
        highPass.b2 = 1.0;
        highPass.a1 = 2.0 * (k * k - 1.0) / denominator;
        highPass.a2 = (1.0 - k / q + k * k) / denominator;
    }
}

double LoudnessMeter::filter(std::size_t channel, double input) noexcept
{
    auto& state = filterStates[channel];
    const auto shelfOutput = shelf.b0 * input
        + shelf.b1 * state.x1 + shelf.b2 * state.x2
        - shelf.a1 * state.shelfY1 - shelf.a2 * state.shelfY2;
    state.x2 = state.x1;
    state.x1 = input;
    state.shelfY2 = state.shelfY1;
    state.shelfY1 = shelfOutput;

    const auto output = highPass.b0 * shelfOutput
        + highPass.b1 * state.shelfX1 + highPass.b2 * state.shelfX2
        - highPass.a1 * state.highPassY1
        - highPass.a2 * state.highPassY2;
    state.shelfX2 = state.shelfX1;
    state.shelfX1 = shelfOutput;
    state.highPassY2 = state.highPassY1;
    state.highPassY1 = output;
    return output;
}
}
