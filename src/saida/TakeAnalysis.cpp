// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/TakeAnalysis.cpp (saida-ref 1.1,
// 16/11/2026, sha256 392a3a46b4e3067b…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/TakeAnalysis.h"

#include <algorithm>
#include <cmath>

namespace rasgo::modular::saida
{
namespace
{
float toDb(double linear) noexcept
{
    return linear > 1.0e-12 ? static_cast<float>(20.0 * std::log10(linear)) : -240.0f;
}
}

void TakeAnalysis::prepare(double sampleRate) noexcept
{
    const auto rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? sampleRate : 48000.0;
    loudness.prepare(rate);
    loudness.reset();
    peakLeft.prepare(rate);
    peakRight.prepare(rate);
    frames = over = invalid = 0;
    samplePeak = truePeak = 0.0f;
    maxShortTerm = LoudnessMeter::silenceLufs;
    sumSquares = sumLeft = sumRight = 0.0;
    shortTermHistogram.fill(0);
    shortTermEnergy = 0.0;
    shortTermCount = 0;
    shortTermInterval = static_cast<std::uint32_t>(rate / 10.0);  // every 100 ms
    shortTermCountdown = shortTermInterval;
}

void TakeAnalysis::push(float& left, float& right) noexcept
{
    if (!std::isfinite(left)) { left = 0.0f; ++invalid; }
    if (!std::isfinite(right)) { right = 0.0f; ++invalid; }
    ++frames;
    const auto l = std::abs(left), r = std::abs(right);
    samplePeak = std::max({ samplePeak, l, r });
    if (l >= 1.0f) ++over;
    if (r >= 1.0f) ++over;
    truePeak = std::max({ truePeak, peakLeft.processSample(left), peakRight.processSample(right) });
    sumSquares += static_cast<double>(left) * left + static_cast<double>(right) * right;
    sumLeft += left;
    sumRight += right;
    loudness.push(left, right);
    if (--shortTermCountdown == 0)
    {
        shortTermCountdown = shortTermInterval;
        const auto readings = loudness.readings();
        if (readings.shortTermReady)
        {
            maxShortTerm = std::max(maxShortTerm, readings.shortTermLufs);
            // the LRA's absolute gate: -70 LUFS
            if (readings.shortTermLufs > histogramFloor)
            {
                const auto bin = std::min<std::size_t>(
                    histogramBins - 1,
                    static_cast<std::size_t>((readings.shortTermLufs - histogramFloor) * 10.0f));
                ++shortTermHistogram[bin];
                shortTermEnergy += std::pow(10.0, readings.shortTermLufs / 10.0);
                ++shortTermCount;
            }
        }
    }
}

TakeAnalysis::Result TakeAnalysis::result() const noexcept
{
    Result out;
    out.frames = frames;
    out.samplePeakDb = toDb(samplePeak);
    out.truePeakDb = toDb(std::max(truePeak, samplePeak));
    out.integratedLufs = loudness.readings().integratedLufs;
    out.maxShortTermLufs = maxShortTerm;
    if (frames > 0)
    {
        const auto n = static_cast<double>(frames);
        out.rmsDb = toDb(std::sqrt(sumSquares / (2.0 * n)));
        out.dcLeft = static_cast<float>(sumLeft / n);
        out.dcRight = static_cast<float>(sumRight / n);
    }
    // LRA (EBU Tech 3342): relative gate 20 LU under the energy mean of the
    // gated short-term values, then the 95th minus the 10th percentile
    if (shortTermCount > 0)
    {
        const auto relative = 10.0 * std::log10(shortTermEnergy / static_cast<double>(shortTermCount)) - 20.0;
        const auto first = static_cast<std::size_t>(
            std::clamp((relative - histogramFloor) * 10.0, 0.0, static_cast<double>(histogramBins - 1)));
        std::uint64_t kept = 0;
        for (auto b = first; b < histogramBins; ++b)
            kept += shortTermHistogram[b];
        if (kept > 0)
        {
            const auto percentile = [&](double p) {
                const auto target = static_cast<std::uint64_t>(std::ceil(p * static_cast<double>(kept)));
                std::uint64_t seen = 0;
                for (auto b = first; b < histogramBins; ++b)
                {
                    seen += shortTermHistogram[b];
                    if (seen >= std::max<std::uint64_t>(1, target))
                        return histogramFloor + (static_cast<float>(b) + 0.5f) * 0.1f;
                }
                return histogramFloor + static_cast<float>(histogramBins) * 0.1f;
            };
            out.loudnessRange = percentile(0.95) - percentile(0.10);
            out.loudnessRangeReady = true;
        }
    }
    out.overFullScale = over;
    out.nonFinite = invalid;
    return out;
}
}
