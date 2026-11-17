// Rasgo Modular — cópia de RASGO/REFERENCIAS/SAIDA_TECNICA/src/DeliveryMaster.cpp (saida-ref 1.2,
// 17/11/2026, sha256 ad96886d905d4c05…), namespace rasgo::modular::saida. Proveniência na referência.
// Copyright (C) 2026 Lúcio de Araújo

#include "saida/DeliveryMaster.h"

#include "saida/OutputStage.h"
#include "saida/TakeAnalysis.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace rasgo::modular::saida
{
namespace
{
// saida-ref 1.2 (17/11/2026): the offline check of the delivered file's true
// peak. The live 4x detector (48 taps per phase) is flat to 20 kHz, but a 4x
// grid misses up to 0.7 dB between its points for content near Nyquist, and
// takes do carry it (a Batruc take: -24 dB of its energy between 20.5 and
// 22.05 kHz; delivered at "-1.00 dBTP", it read -0.51 at 32x). Offline, the
// cost does not matter: 16x, 128 taps per phase, Kaiser (beta 12), flat to
// 21 kHz at 44.1 kHz; it agreed with a 32x reference to 0.01 dB on that take.
// Only the points next to a sample within 6 dB of the ceiling are
// interpolated (an intersample peak is not 6 dB over both its neighbours in
// any real take); the rest of the take costs nothing.
class PreciseTruePeak final
{
public:
    static constexpr std::size_t factor = 16;
    static constexpr std::size_t taps = 128;

    PreciseTruePeak()
    {
        constexpr double pi = 3.14159265358979323846;
        constexpr double beta = 12.0;
        auto i0 = [](double x) {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 80 && term > sum * 1.0e-17; ++k)
            {
                term *= x * x * 0.25 / (static_cast<double>(k) * static_cast<double>(k));
                sum += term;
            }
            return sum;
        };
        constexpr auto total = factor * taps;
        constexpr auto half = static_cast<double>(total) * 0.5;
        std::vector<double> prototype((factor - 1) * taps, 0.0);
        for (std::size_t phase = 1; phase < factor; ++phase)
        {
            double sum = 0.0;
            for (std::size_t t = 0; t < taps; ++t)
            {
                const auto position = static_cast<double>(t * factor + phase) - half;
                const auto ratio = position / half;
                const auto x = position / static_cast<double>(factor);
                const auto s = std::sin(pi * x) / (pi * x);  // never 0: phase > 0
                const auto value = s * i0(beta * std::sqrt(std::max(0.0, 1.0 - ratio * ratio))) / i0(beta);
                prototype[(phase - 1) * taps + t] = value;
                sum += value;
            }
            for (std::size_t t = 0; t < taps; ++t)
                prototype[(phase - 1) * taps + t] /= sum;
        }
        coefficients.assign(prototype.begin(), prototype.end());
    }

    void reset(float gateLinear) noexcept
    {
        gate = gateLinear;
        peak = 0.0;
        for (auto& h : history) h.fill(0.0f);
        at = 0;
    }

    void push(float left, float right) noexcept
    {
        history[0][at] = history[0][at + taps] = left;
        history[1][at] = history[1][at + taps] = right;
        at = (at + 1) % taps;
        for (std::size_t ch = 0; ch < 2; ++ch)
        {
            const float* w = history[ch].data() + at;  // oldest first
            // the two samples around the interpolated points
            const auto a = std::abs(w[taps / 2 - 1]), b = std::abs(w[taps / 2]);
            peak = std::max(peak, static_cast<double>(std::max(a, b)));
            if (std::max(a, b) < gate) continue;
            for (std::size_t phase = 0; phase + 1 < factor; ++phase)
            {
                // eight partial sums in float (vectorised; error ~1e-7)
                const float* h = coefficients.data() + phase * taps;
                std::array<float, 8> partial {};
                for (std::size_t t = 0; t < taps; t += 8)
                    for (std::size_t lane = 0; lane < 8; ++lane)
                        partial[lane] += h[t + lane] * w[t + lane];
                const auto y = ((partial[0] + partial[1]) + (partial[2] + partial[3]))
                    + ((partial[4] + partial[5]) + (partial[6] + partial[7]));
                peak = std::max(peak, static_cast<double>(std::abs(y)));
            }
        }
    }

    [[nodiscard]] float db() const noexcept
    {
        return peak > 1.0e-12 ? static_cast<float>(20.0 * std::log10(peak)) : -240.0f;
    }

private:
    std::vector<float> coefficients;
    std::array<std::array<float, 2 * taps>, 2> history {};
    std::size_t at = 0;
    float gate = 0.0f;
    double peak = 0.0;
};

struct Pass
{
    float lufs = -70.0f;
    float truePeakDb = -240.0f;  // the precise (16x) true peak of the output
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
            float checkCeilingDbtp, PreciseTruePeak& precise, DeliveryMaster::Sink* sink)
{
    Pass result;
    if (!source.rewind()) { result.ok = false; return result; }
    OutputStage stage;
    stage.prepare(rate);
    stage.setParameters({ 0.0f, ceilingDbtp, 150.0f, false });
    stage.reset();
    TakeAnalysis analysis;
    analysis.prepare(rate);
    precise.reset(std::pow(10.0f, (checkCeilingDbtp - 6.0f) / 20.0f));
    const auto gain = std::pow(10.0f, gainDb / 20.0f);
    const auto latency = static_cast<std::size_t>(stage.latencySamples());
    std::vector<float> inL(block), inR(block), outL(block), outR(block);
    std::size_t skipped = 0, pending = 0;
    bool ended = false;
    auto flush = [&]() {
        for (std::size_t i = 0; i < pending; ++i)
        {
            analysis.push(outL[i], outR[i]);
            precise.push(outL[i], outR[i]);
        }
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
    for (std::size_t i = 0; i < PreciseTruePeak::taps; ++i)  // the last samples through the check
        precise.push(0.0f, 0.0f);
    const auto measured = analysis.result();
    result.lufs = measured.integratedLufs;
    result.truePeakDb = std::max(measured.truePeakDb, precise.db());
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

    // find the gain by measuring only (nothing written). The limiter's own
    // ceiling starts at the target and is lowered by what the precise check
    // finds over it (1.2), at most 3 dB.
    PreciseTruePeak precise;
    auto limiterCeiling = ceiling;
    // a gain once found over the guard is never tried again (no oscillation
    // between a gain over the guard and one under it)
    auto gainOverGuard = settings.maxGainDb + 1.0f;
    Pass best;
    float bestGain = gain, bestCeiling = ceiling;
    bool haveBest = false;
    // ranking: within the guard and under the ceiling first, then the
    // loudest; a pass that fails either only wins over another that fails
    auto valid = [&](const Pass& p) {
        return p.reductionDb <= maxReduction + 0.25f && p.truePeakDb <= ceiling + 0.02f;
    };
    for (int pass = 0; pass < std::max(1, settings.passes) + 2; ++pass)
    {
        const auto result = render(source, rate, block, gain, limiterCeiling, ceiling, precise, nullptr);
        if (!result.ok) { report.failed = true; return report; }
        const bool withinGuard = result.reductionDb <= maxReduction + 0.25f;
        const bool better = !haveBest
            || (valid(result) && (!valid(best) || result.lufs > best.lufs))
            || (!valid(result) && !valid(best) && result.reductionDb < best.reductionDb);
        if (better)
        {
            best = result;
            bestGain = gain;
            bestCeiling = limiterCeiling;
            haveBest = true;
        }
        const auto over = result.truePeakDb - ceiling;
        if (over > 0.02f)
            limiterCeiling = std::max(ceiling - 3.0f, limiterCeiling - over - 0.02f);
        if (!withinGuard)
            gainOverGuard = std::min(gainOverGuard, gain);
        const auto missing = settings.targetLufs - result.lufs;
        if (std::abs(missing) <= 0.2f && over <= 0.02f && withinGuard)
            break;
        auto next = std::clamp(gain + missing, -settings.maxGainDb, settings.maxGainDb);
        next = std::min(next, limiterCeiling + maxReduction + 1.0f - input.truePeakDb);
        if (!withinGuard)
            next = gain - (result.reductionDb - maxReduction);
        if (withinGuard)  // between a gain inside the guard and one over it: halve the gap
            next = std::min(next, 0.5f * (gain + gainOverGuard));
        if (withinGuard && over <= 0.02f && gainOverGuard - gain < 0.1f)
            break;  // as loud as the guard allows
        if (std::abs(next - gain) < 0.05f && over <= 0.02f)
            break;
        gain = next;
    }
    // the one pass that writes, with the chosen gain and limiter ceiling
    const auto written = render(source, rate, block, bestGain, bestCeiling, ceiling, precise, &sink);
    report.failed = !written.ok;
    report.gainDb = bestGain;
    report.outputLufs = written.lufs;
    report.outputTruePeakDb = written.truePeakDb;
    report.limiterReductionDb = written.reductionDb;
    report.reachedTarget = std::abs(settings.targetLufs - written.lufs) <= 0.5f
        && written.truePeakDb <= ceiling + 0.05f;
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
