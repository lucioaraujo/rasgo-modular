// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/OutputStage.cpp (saida-ref 1.1,
// 16/11/2026, sha256 95ca6ed6ad5b0ac6…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/OutputStage.h"

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

float OutputStage::dbToGain(float value) noexcept
{
    return std::pow(10.0f, value / 20.0f);
}

void OutputStage::saturatingIncrement(std::uint64_t& value) noexcept
{
    if (value != std::numeric_limits<std::uint64_t>::max()) ++value;
}

float OutputStage::catmull(float a, float b, float c, float d,
                           float phase) noexcept
{
    const auto phase2 = phase * phase;
    const auto phase3 = phase2 * phase;
    return 0.5f * ((2.0f * b)
        + (-a + c) * phase
        + (2.0f * a - 5.0f * b + 4.0f * c - d) * phase2
        + (-a + 3.0f * b - 3.0f * c + d) * phase3);
}

float OutputStage::estimatedPeak(const std::array<float, 4>& history) noexcept
{
    auto peak = std::max(std::abs(history[1]), std::abs(history[2]));
    for (const auto phase : { 0.25f, 0.50f, 0.75f })
        peak = std::max(peak, std::abs(catmull(
            history[0], history[1], history[2], history[3], phase)));
    return peak;
}

void OutputStage::prepare(double sampleRate) noexcept
{
    preparedRate = numeric::sampleRate(sampleRate);
    lookaheadSamples = static_cast<std::uint32_t>(std::clamp(
        std::lround(preparedRate * 0.005), 1l,
        static_cast<long>(delayCapacity - 4u)));
    dcCoefficient = static_cast<float>(std::exp(-2.0 * pi * 10.0 / preparedRate));
    smoothingCoefficient = static_cast<float>(
        -std::expm1(-1.0 / (preparedRate * 0.015)));
    rmsCoefficient = static_cast<float>(
        -std::expm1(-1.0 / (preparedRate * 0.400)));
    loudnessMeter.prepare(preparedRate);
    for (auto* detector : { &limiterDetectorLeft, &limiterDetectorRight, &meterDetectorLeft, &meterDetectorRight })
        detector->prepare(preparedRate);
    targetTrimGain = dbToGain(controls.trimDb);
    targetMuteGain = controls.muted ? 0.0f : 1.0f;
    targetCeilingGain = dbToGain(controls.ceilingDb);
    releaseCoefficient = static_cast<float>(-std::expm1(
        -1.0 / (preparedRate * static_cast<double>(controls.releaseMs) * 0.001)));
    reset();
}

void OutputStage::reset() noexcept
{
    delayLeft.fill(0.0f);
    delayRight.fill(0.0f);
    detectorLeft.fill(0.0f);
    detectorRight.fill(0.0f);
    outputHistoryLeft.fill(0.0f);
    outputHistoryRight.fill(0.0f);
    for (auto* detector : { &limiterDetectorLeft, &limiterDetectorRight, &meterDetectorLeft, &meterDetectorRight })
        detector->reset();
    writeIndex = 0;
    queueHead = queueSize = 0;
    sampleCounter = 0;
    previousDcInputLeft = previousDcInputRight = 0.0f;
    previousDcOutputLeft = previousDcOutputRight = 0.0f;
    currentTrimGain = targetTrimGain;
    currentMuteGain = targetMuteGain;
    currentCeilingGain = targetCeilingGain;
    limiterGain = 1.0f;
    invalidSampleCount = 0;
    clearMeters();
}

void OutputStage::setParameters(Parameters value) noexcept
{
    controls.trimDb = numeric::clampFinite(value.trimDb, -18.0f, 6.0f,
                                            controls.trimDb);
    controls.ceilingDb = numeric::clampFinite(value.ceilingDb, -6.0f, -0.1f,
                                               controls.ceilingDb);
    controls.releaseMs = numeric::clampFinite(value.releaseMs, 50.0f, 500.0f,
                                               controls.releaseMs);
    controls.muted = value.muted;
    targetTrimGain = dbToGain(controls.trimDb);
    targetMuteGain = controls.muted ? 0.0f : 1.0f;
    targetCeilingGain = dbToGain(controls.ceilingDb);
    // A stricter ceiling is a safety action and must not wait for smoothing.
    currentCeilingGain = std::min(currentCeilingGain, targetCeilingGain);
    releaseCoefficient = static_cast<float>(-std::expm1(
        -1.0 / (preparedRate * static_cast<double>(controls.releaseMs) * 0.001)));
}

OutputStage::Sample OutputStage::process(float left, float right) noexcept
{
    if (!std::isfinite(left))
    {
        left = 0.0f;
        saturatingIncrement(invalidSampleCount);
    }
    if (!std::isfinite(right))
    {
        right = 0.0f;
        saturatingIncrement(invalidSampleCount);
    }
    inputSamplePeak = std::max(inputSamplePeak,
                               std::max(std::abs(left), std::abs(right)));

    const auto dcLeft = left - previousDcInputLeft
        + dcCoefficient * previousDcOutputLeft;
    const auto dcRight = right - previousDcInputRight
        + dcCoefficient * previousDcOutputRight;
    previousDcInputLeft = left;
    previousDcInputRight = right;
    previousDcOutputLeft = dcLeft;
    previousDcOutputRight = dcRight;

    currentTrimGain += (targetTrimGain - currentTrimGain) * smoothingCoefficient;
    currentMuteGain += (targetMuteGain - currentMuteGain) * smoothingCoefficient;
    currentCeilingGain += (targetCeilingGain - currentCeilingGain)
        * smoothingCoefficient;
    const auto gainBeforeLimiter = currentTrimGain * currentMuteGain;
    const auto preparedLeft = dcLeft * gainBeforeLimiter;
    const auto preparedRight = dcRight * gainBeforeLimiter;
    delayLeft[writeIndex] = preparedLeft;
    delayRight[writeIndex] = preparedRight;

    detectorLeft = { detectorLeft[1], detectorLeft[2], detectorLeft[3],
                     preparedLeft };
    detectorRight = { detectorRight[1], detectorRight[2], detectorRight[3],
                      preparedRight };
    // the 4x true peak (it answers ~8 samples late: well inside the 5 ms
    // lookahead), with the old estimate kept as a floor
    const auto detectedPeak = std::max({ estimatedPeak(detectorLeft), estimatedPeak(detectorRight),
                                         limiterDetectorLeft.processSample(preparedLeft),
                                         limiterDetectorRight.processSample(preparedRight) });
    // the gain this sample requires, into the sliding minimum of the window
    // (the lookahead, the detector's latency and a small margin)
    const auto required = detectedPeak > currentCeilingGain && detectedPeak > 0.0f
        ? currentCeilingGain / detectedPeak : 1.0f;
    const auto window = static_cast<std::uint64_t>(lookaheadSamples)
        + limiterDetectorLeft.latencySamples() + 4u;
    const auto at = [this](std::size_t i) { return (queueHead + i) % delayCapacity; };
    while (queueSize > 0 && requiredGain[at(queueSize - 1)] >= required)
        --queueSize;
    requiredGain[at(queueSize)] = required;
    requiredAt[at(queueSize)] = sampleCounter;
    ++queueSize;
    while (queueSize > 0 && requiredAt[queueHead] + window <= sampleCounter)
    {
        queueHead = (queueHead + 1) % delayCapacity;
        --queueSize;
    }
    ++sampleCounter;
    const auto windowGain = queueSize > 0 ? requiredGain[queueHead] : 1.0f;
    if (windowGain < limiterGain)
        limiterGain = windowGain;
    else
        limiterGain += (windowGain - limiterGain) * releaseCoefficient;

    const auto readIndex = (writeIndex + delayCapacity - lookaheadSamples)
        % delayCapacity;
    Sample result { delayLeft[readIndex] * limiterGain,
                    delayRight[readIndex] * limiterGain };
    writeIndex = (writeIndex + 1u) % delayCapacity;

    outputSamplePeak = std::max(outputSamplePeak,
        std::max(std::abs(result.left), std::abs(result.right)));
    outputHistoryLeft = { outputHistoryLeft[1], outputHistoryLeft[2],
                          outputHistoryLeft[3], result.left };
    outputHistoryRight = { outputHistoryRight[1], outputHistoryRight[2],
                           outputHistoryRight[3], result.right };
    outputTruePeak = std::max({ outputTruePeak,
        estimatedPeak(outputHistoryLeft), estimatedPeak(outputHistoryRight),
        meterDetectorLeft.processSample(result.left), meterDetectorRight.processSample(result.right) });
    const auto squared = (result.left * result.left
        + result.right * result.right) * 0.5f;
    rmsState += (squared - rmsState) * rmsCoefficient;
    correlationLL += (result.left * result.left - correlationLL) * rmsCoefficient;
    correlationRR += (result.right * result.right - correlationRR) * rmsCoefficient;
    correlationLR += (result.left * result.right - correlationLR) * rmsCoefficient;
    loudnessMeter.push(result.left, result.right);
    clipped = clipped || outputTruePeak >= 1.0f;
    return result;
}

OutputStage::Telemetry OutputStage::telemetry() const noexcept
{
    Telemetry result;
    result.inputSamplePeak = inputSamplePeak;
    result.outputSamplePeak = outputSamplePeak;
    result.estimatedTruePeak = outputTruePeak;
    result.rms = std::sqrt(std::max(0.0f, rmsState));
    const auto power = correlationLL * correlationRR;
    result.correlation = power > 1.0e-14f
        ? std::clamp(correlationLR / std::sqrt(power), -1.0f, 1.0f) : 0.0f;
    const auto loudness = loudnessMeter.readings();
    result.momentaryLufs = loudness.momentaryLufs;
    result.shortTermLufs = loudness.shortTermLufs;
    result.integratedLufs = loudness.integratedLufs;
    result.loudnessBlocks = loudness.integratedBlocks;
    result.momentaryLoudnessReady = loudness.momentaryReady;
    result.shortTermLoudnessReady = loudness.shortTermReady;
    result.gainReductionDb = limiterGain < 1.0f
        ? -20.0f * std::log10(std::max(limiterGain, 1.0e-9f)) : 0.0f;
    result.invalidSamples = invalidSampleCount;
    result.latencySamples = lookaheadSamples;
    result.clipLatched = clipped;
    return result;
}

void OutputStage::clearMeters() noexcept
{
    inputSamplePeak = 0.0f;
    outputSamplePeak = 0.0f;
    outputTruePeak = 0.0f;
    rmsState = 0.0f;
    correlationLL = correlationRR = correlationLR = 0.0f;
    loudnessMeter.reset();
    invalidSampleCount = 0;
    clipped = false;
}
}
