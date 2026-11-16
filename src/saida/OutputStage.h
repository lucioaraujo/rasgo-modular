// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/OutputStage.h (saida-ref 1.1,
// 16/11/2026, sha256 4d8d89ef407a6151…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "saida/LoudnessMeter.h"
#include "saida/TruePeakDetector.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace rasgo::modular::saida
{
// Final technical safety stage. It is deliberately independent from the
// creative Master and cannot receive ecological modulation.
class OutputStage final
{
public:
    static constexpr std::size_t delayCapacity = 4096;

    struct Parameters
    {
        float trimDb = 0.0f;
        float ceilingDb = -1.0f;
        float releaseMs = 150.0f;
        bool muted = false;
    };

    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        float inputSamplePeak = 0.0f;
        float outputSamplePeak = 0.0f;
        float estimatedTruePeak = 0.0f;
        float rms = 0.0f;
        float momentaryLufs = LoudnessMeter::silenceLufs;
        float shortTermLufs = LoudnessMeter::silenceLufs;
        float integratedLufs = LoudnessMeter::silenceLufs;
        float gainReductionDb = 0.0f;
        // stereo correlation of the output, -1 (opposite) .. 0 (unrelated) ..
        // +1 (mono), averaged like the RMS (~400 ms); 0 in silence (2026-10-22)
        float correlation = 0.0f;
        std::uint64_t loudnessBlocks = 0;
        std::uint64_t invalidSamples = 0;
        std::uint32_t latencySamples = 0;
        bool momentaryLoudnessReady = false;
        bool shortTermLoudnessReady = false;
        bool clipLatched = false;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Sample process(float left, float right) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;
    // the safety limiter's gain now, 0..1 (1 = no reduction): read per
    // sample for the feedback taps without building the whole telemetry
    [[nodiscard]] float limiterGainNow() const noexcept { return limiterGain; }
    [[nodiscard]] std::uint32_t latencySamples() const noexcept { return lookaheadSamples; }
    void clearMeters() noexcept;

private:
    static float dbToGain(float value) noexcept;
    static float catmull(float a, float b, float c, float d, float phase) noexcept;
    static float estimatedPeak(const std::array<float, 4>& history) noexcept;
    static void saturatingIncrement(std::uint64_t& value) noexcept;

    std::array<float, delayCapacity> delayLeft {};
    std::array<float, delayCapacity> delayRight {};
    std::array<float, 4> detectorLeft {};
    std::array<float, 4> detectorRight {};
    std::array<float, 4> outputHistoryLeft {};
    std::array<float, 4> outputHistoryRight {};
    // true peak, 4x oversampled (2026-10-22): the 4-sample estimate alone let
    // intersample peaks through, up to +0.9 dBTP over a -0.1 dB ceiling
    // (measured with this detector); the limiter now detects with it and
    // the meter reads it
    TruePeakDetector limiterDetectorLeft {}, limiterDetectorRight {};
    TruePeakDetector meterDetectorLeft {}, meterDetectorRight {};
    // the gain each detected sample requires, as a sliding minimum over the
    // lookahead window (a monotonic queue, fixed capacity): the limiter
    // follows it - down at once, back up by the release once the peak has
    // left the window. Before 2026-10-22 the gain only ever went down while
    // an overload lasted: one +18 dB spike held a steady signal 12 dB under
    // the ceiling for as long as the overload went on.
    std::array<float, delayCapacity> requiredGain {};
    std::array<std::uint64_t, delayCapacity> requiredAt {};
    std::size_t queueHead = 0, queueSize = 0;
    std::uint64_t sampleCounter = 0;
    Parameters controls {};
    double preparedRate = 48000.0;
    std::size_t writeIndex = 0;
    std::uint32_t lookaheadSamples = 240;
    float previousDcInputLeft = 0.0f;
    float previousDcInputRight = 0.0f;
    float previousDcOutputLeft = 0.0f;
    float previousDcOutputRight = 0.0f;
    float dcCoefficient = 0.9987f;
    float smoothingCoefficient = 0.001f;
    float releaseCoefficient = 0.0001f;
    float targetTrimGain = 1.0f;
    float currentTrimGain = 1.0f;
    float targetMuteGain = 1.0f;
    float currentMuteGain = 1.0f;
    float targetCeilingGain = 0.8912509f;
    float currentCeilingGain = 0.8912509f;
    float limiterGain = 1.0f;
    float inputSamplePeak = 0.0f;
    float outputSamplePeak = 0.0f;
    float outputTruePeak = 0.0f;
    float rmsState = 0.0f;
    float correlationLL = 0.0f, correlationRR = 0.0f, correlationLR = 0.0f;
    float rmsCoefficient = 0.00005f;
    LoudnessMeter loudnessMeter {};
    std::uint64_t invalidSampleCount = 0;
    bool clipped = false;
};
}
