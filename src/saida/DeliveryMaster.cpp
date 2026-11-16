// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/DeliveryMaster.cpp (saida-ref 1.1,
// 16/11/2026, sha256 3f2230c4ae1cde7f…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/DeliveryMaster.h"

#include "saida/OutputStage.h"
#include "saida/TakeAnalysis.h"

#include <algorithm>
#include <cmath>

namespace rasgo::modular::saida
{
namespace
{
struct Pass
{
    float lufs = -70.0f;
    float truePeakDb = -240.0f;
    float reductionDb = 0.0f;
    bool ok = true;
};

// one read of the take: measured as it comes
bool measure(DeliveryMaster::Source& source, double rate, std::size_t block, TakeAnalysis::Result& out)
{
    if (!source.rewind()) return false;
    std::vector<float> l(block), r(block);
    TakeAnalysis analysis;
    analysis.prepare(rate);
    for (;;)
    {
        const auto n = source.read(l.data(), r.data(), block);
        if (n == 0) break;
        for (std::size_t i = 0; i < n; ++i)
            analysis.push(l[i], r[i]);
    }
    out = analysis.result();
    return true;
}

// gain, then the limiter, latency compensated (the output lines up with the
// input); measured, and written when there is a sink
Pass render(DeliveryMaster::Source& source, double rate, std::size_t block, float gainDb, float ceilingDbtp,
            DeliveryMaster::Sink* sink)
{
    Pass result;
    if (!source.rewind()) { result.ok = false; return result; }
    OutputStage stage;
    stage.prepare(rate);
    stage.setParameters({ 0.0f, ceilingDbtp, 150.0f, false });
    stage.reset();
    TakeAnalysis analysis;
    analysis.prepare(rate);
    const auto gain = std::pow(10.0f, gainDb / 20.0f);
    const auto latency = static_cast<std::size_t>(stage.latencySamples());
    std::vector<float> inL(block), inR(block), outL(block), outR(block);
    std::size_t skipped = 0, pending = 0;
    bool ended = false;
    auto flush = [&]() {
        for (std::size_t i = 0; i < pending; ++i)
            analysis.push(outL[i], outR[i]);
        if (sink != nullptr && pending > 0 && !sink->write(outL.data(), outR.data(), pending))
            result.ok = false;
        pending = 0;
    };
    std::size_t tail = latency;  // zeros fed after the end, to get the last samples out
    while (result.ok && (!ended || tail > 0))
    {
        std::size_t n = 0;
        if (!ended)
        {
            n = source.read(inL.data(), inR.data(), block);
            if (n == 0) ended = true;
        }
        if (ended)
        {
            n = std::min(tail, block);
            std::fill_n(inL.begin(), n, 0.0f);
            std::fill_n(inR.begin(), n, 0.0f);
            tail -= n;
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            const auto out = stage.process(inL[i] * gain, inR[i] * gain);
            result.reductionDb = std::max(result.reductionDb,
                                          -20.0f * std::log10(std::max(stage.limiterGainNow(), 1.0e-9f)));
            if (skipped < latency) { ++skipped; continue; }
            outL[pending] = out.left;
            outR[pending] = out.right;
            if (++pending == block) flush();
        }
    }
    flush();
    const auto measured = analysis.result();
    result.lufs = measured.integratedLufs;
    result.truePeakDb = measured.truePeakDb;
    return result;
}

struct CopyAll
{
    static bool run(DeliveryMaster::Source& source, DeliveryMaster::Sink& sink, std::size_t block)
    {
        if (!source.rewind()) return false;
        std::vector<float> l(block), r(block);
        for (;;)
        {
            const auto n = source.read(l.data(), r.data(), block);
            if (n == 0) return true;
            if (!sink.write(l.data(), r.data(), n)) return false;
        }
    }
};
}

DeliveryMaster::Report DeliveryMaster::process(Source& source, double sampleRate, Sink& sink, const Settings& settings)
{
    Report report;
    const auto rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? sampleRate : 48000.0;
    const auto block = std::clamp<std::size_t>(settings.blockFrames, 256, 1u << 20);
    TakeAnalysis::Result input;
    if (!measure(source, rate, block, input)) { report.failed = true; return report; }
    report.inputLufs = input.integratedLufs;
    report.inputTruePeakDb = input.truePeakDb;
    if (input.frames == 0 || input.integratedLufs <= -69.9f)
    {
        report.silent = true;
        report.outputLufs = input.integratedLufs;
        report.outputTruePeakDb = input.truePeakDb;
        report.failed = !CopyAll::run(source, sink, block);
        return report;
    }

    const auto ceiling = std::clamp(settings.ceilingDbtp, -6.0f, -0.1f);
    const auto maxReduction = std::max(0.0f, settings.maxLimiterReductionDb);
    auto gain = std::clamp(settings.targetLufs - input.integratedLufs, -settings.maxGainDb, settings.maxGainDb);
    // the dynamics guard, before anything is rendered: the gain may lift the
    // peaks at most `maxReduction` over the ceiling
    gain = std::min(gain, ceiling + maxReduction - input.truePeakDb);

    // find the gain by measuring only (nothing written)
    Pass best;
    float bestGain = gain;
    bool haveBest = false;
    for (int pass = 0; pass < std::max(1, settings.passes); ++pass)
    {
        const auto result = render(source, rate, block, gain, ceiling, nullptr);
        if (!result.ok) { report.failed = true; return report; }
        const bool withinGuard = result.reductionDb <= maxReduction + 0.25f;
        if (!haveBest || (withinGuard && result.lufs > best.lufs))
        {
            best = result;
            bestGain = gain;
            haveBest = true;
        }
        const auto missing = settings.targetLufs - result.lufs;
        if (std::abs(missing) <= 0.2f)
            break;
        auto next = std::clamp(gain + missing, -settings.maxGainDb, settings.maxGainDb);
        next = std::min(next, ceiling + maxReduction + 1.0f - input.truePeakDb);
        if (!withinGuard)
            next = gain - (result.reductionDb - maxReduction);
        if (std::abs(next - gain) < 0.05f)
            break;
        gain = next;
    }
    // the one pass that writes, with the chosen gain
    const auto written = render(source, rate, block, bestGain, ceiling, &sink);
    report.failed = !written.ok;
    report.gainDb = bestGain;
    report.outputLufs = written.lufs;
    report.outputTruePeakDb = written.truePeakDb;
    report.limiterReductionDb = written.reductionDb;
    report.reachedTarget = std::abs(settings.targetLufs - written.lufs) <= 0.5f;
    (void) best;
    return report;
}

DeliveryMaster::Report DeliveryMaster::process(const std::vector<float>& left, const std::vector<float>& right,
                                               double sampleRate, std::vector<float>& outLeft,
                                               std::vector<float>& outRight, const Settings& settings)
{
    struct VectorSource final : Source
    {
        const std::vector<float>& l;
        const std::vector<float>& r;
        std::size_t n, at = 0;
        VectorSource(const std::vector<float>& a, const std::vector<float>& b)
            : l(a), r(b), n(std::min(a.size(), b.size())) {}
        bool rewind() override { at = 0; return true; }
        std::size_t read(float* left, float* right, std::size_t frames) override
        {
            const auto k = std::min(frames, n - at);
            std::copy_n(l.begin() + static_cast<std::ptrdiff_t>(at), k, left);
            std::copy_n(r.begin() + static_cast<std::ptrdiff_t>(at), k, right);
            at += k;
            return k;
        }
    } source(left, right);
    struct VectorSink final : Sink
    {
        std::vector<float>& l;
        std::vector<float>& r;
        VectorSink(std::vector<float>& a, std::vector<float>& b) : l(a), r(b) { l.clear(); r.clear(); }
        bool write(const float* left, const float* right, std::size_t frames) override
        {
            l.insert(l.end(), left, left + frames);
            r.insert(r.end(), right, right + frames);
            return true;
        }
    } sink(outLeft, outRight);
    return process(source, sampleRate, sink, settings);
}

DeliveryMaster::Report DeliveryMaster::process(Source& source, double sampleRate, Sink& sink)
{
    return process(source, sampleRate, sink, Settings {});
}

DeliveryMaster::Report DeliveryMaster::process(const std::vector<float>& left, const std::vector<float>& right,
                                               double sampleRate, std::vector<float>& outLeft,
                                               std::vector<float>& outRight)
{
    return process(left, right, sampleRate, outLeft, outRight, Settings {});
}
}
