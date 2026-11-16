// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/LoudnessMeter.h (saida-ref 1.1,
// 16/11/2026, sha256 ca1a18ed080327af…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace rasgo::modular::saida
{
// Diagnostic ITU-R BS.1770 / EBU R128 loudness meter. It never changes the
// signal. The bounded storage keeps push() allocation-free on the audio thread.
//
// This implementation is an Aquorbium adaptation of the author's own RASGO
// Modular loudness work. Both derive the K-weighting filters from the active
// sample rate. Energy is kept in 10 ms slices (2026-10-22; 100 ms before):
// M and S move every 10 ms, so a short tone is read whole wherever it falls
// (EBU Tech 3341 #13-14 failed by 0.45 LU at 100 ms); the integrated
// loudness still takes the normative 400 ms blocks every 100 ms.
class LoudnessMeter final
{
public:
    static constexpr float silenceLufs = -70.0f;
    struct Readings
    {
        float momentaryLufs = silenceLufs;
        float shortTermLufs = silenceLufs;
        float integratedLufs = silenceLufs;
        std::uint64_t integratedBlocks = 0;
        bool momentaryReady = false;
        bool shortTermReady = false;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void push(float left, float right) noexcept;
    [[nodiscard]] Readings readings() const noexcept;

private:
    static constexpr std::size_t momentarySlices = 40;   // 400 ms
    static constexpr std::size_t shortTermSlices = 300;  // 3 s
    static constexpr std::size_t blockHopSlices = 10;    // the integrated blocks' 100 ms hop
    static constexpr float histogramMinimumLufs = -70.0f;
    static constexpr float histogramMaximumLufs = 70.0f;
    // A resolução de 0,1 LU limita o erro de decisão junto à porta relativa
    // sem criar um pico de CPU no callback quando o integrado é atualizado.
    static constexpr float histogramStepLufs = 0.1f;
    static constexpr std::size_t histogramBins = 1400;

    struct Biquad
    {
        double b0 = 1.0;
        double b1 = 0.0;
        double b2 = 0.0;
        double a1 = 0.0;
        double a2 = 0.0;
    };

    struct FilterState
    {
        double x1 = 0.0;
        double x2 = 0.0;
        double shelfY1 = 0.0;
        double shelfY2 = 0.0;
        double shelfX1 = 0.0;
        double shelfX2 = 0.0;
        double highPassY1 = 0.0;
        double highPassY2 = 0.0;
    };

    void designKWeighting() noexcept;
    [[nodiscard]] double filter(std::size_t channel, double input) noexcept;
    [[nodiscard]] double windowMeanSquare(std::size_t slices) const noexcept;
    [[nodiscard]] float windowLufs(std::size_t slices) const noexcept;
    [[nodiscard]] static float energyToLufs(double energy) noexcept;
    void registerIntegratedBlock(double energy) noexcept;
    void updateIntegratedLoudness() noexcept;

    double preparedRate = 48000.0;
    Biquad shelf {};
    Biquad highPass {};
    std::array<FilterState, 2> filterStates {};
    std::size_t sliceLength = 480;
    std::size_t slicesSinceBlock = 0;
    double sliceEnergy = 0.0;
    std::size_t sliceFill = 0;
    std::array<double, shortTermSlices> recentSlices {};
    std::size_t recentWrite = 0;
    std::size_t recentCount = 0;
    std::array<double, histogramBins> histogramEnergy {};
    std::array<std::uint64_t, histogramBins> histogramCount {};
    double absoluteGateEnergy = 0.0;
    std::uint64_t absoluteGateBlocks = 0;
    std::uint64_t integratedBlockCount = 0;
    float integratedLufs = silenceLufs;
};
}
