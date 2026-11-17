// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/DeliveryMaster.h (saida-ref 1.2,
// 17/11/2026, sha256 13f78b43fc779c77…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <vector>

namespace rasgo::modular::saida
{
// Delivery master (saida-ref 1.1, 2026-11-16). Author: "nivele todos os
// instrumentos para que fiquem preparados para os spotifys da vida"; decision:
// the REC keeps the raw take and also writes a streaming version, made
// OFFLINE from the whole take; the live sound is never normalised
// (SAIDA_AUDIO_COMUM: a live performance is not normalised automatically).
//
// What it does, on a finished take:
//  1. measures the integrated loudness (TakeAnalysis, BS.1770);
//  2. applies the gain to the target (-14 LUFS by default);
//  3. runs the reference OutputStage limiter (true peak 4x, lookahead,
//     ceiling -1 dBTP), latency compensated;
//  4. measures again and corrects the gain for what the limiter took away
//     (a few passes), never past `maxLimiterReductionDb` of peak reduction:
//     a dynamic take that cannot reach the target without crushing stops
//     earlier and says so (`reachedTarget = false`). Streaming services turn
//     quieter tracks up on their own in their normal mode.
//
// STREAMING: the take is read in blocks, several times (one pass to measure,
// a few to find the gain, one to write), so memory stays constant whatever
// the length ("saber trabalhar sem estressar a máquina"). Each instrument
// adapts Source/Sink to its own file reader/writer (JUCE or its own WAV
// code); the writer is where the TPDF dither belongs. Never on the audio
// thread.
class DeliveryMaster final
{
public:
    struct Settings
    {
        float targetLufs = -14.0f;
        float ceilingDbtp = -1.0f;
        float maxLimiterReductionDb = 6.0f;  // the dynamics guard
        float maxGainDb = 30.0f;             // never lift noise floors without bound
        int passes = 4;
        std::size_t blockFrames = 8192;
    };

    struct Report
    {
        float inputLufs = -70.0f;
        float inputTruePeakDb = -240.0f;
        float gainDb = 0.0f;
        float outputLufs = -70.0f;
        float outputTruePeakDb = -240.0f;  // 1.2: checked at 16x (offline), not the live 4x
        float limiterReductionDb = 0.0f;  // the most the limiter took from a peak
        bool reachedTarget = false;
        bool silent = false;              // nothing to master: written as it came
        bool failed = false;              // the source or the sink failed
    };

    // the take, read from the start each time rewind() is called
    struct Source
    {
        virtual ~Source() = default;
        virtual bool rewind() = 0;
        // fills up to `frames` frames; returns how many (0 at the end)
        virtual std::size_t read(float* left, float* right, std::size_t frames) = 0;
    };
    struct Sink
    {
        virtual ~Sink() = default;
        virtual bool write(const float* left, const float* right, std::size_t frames) = 0;
    };

    static Report process(Source& source, double sampleRate, Sink& sink, const Settings& settings);
    static Report process(Source& source, double sampleRate, Sink& sink);

    // convenience for short material and tests: whole buffers in memory
    static Report process(const std::vector<float>& left, const std::vector<float>& right, double sampleRate,
                          std::vector<float>& outLeft, std::vector<float>& outRight, const Settings& settings);
    static Report process(const std::vector<float>& left, const std::vector<float>& right, double sampleRate,
                          std::vector<float>& outLeft, std::vector<float>& outRight);
};
}
