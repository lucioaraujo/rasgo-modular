// Teste isolado do Módulo 45 (FORMANT — ressoador espectral multibanda).
// Critérios do dossiê `dossies/45_formant.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Formant.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 256;

using Gen = std::function<float(std::size_t)>;

Gen whiteNoise(float amp) {
    auto s = std::make_shared<std::uint32_t>(0x1234567u);
    return [s, amp](std::size_t) {
        *s = *s * 1664525u + 1013904223u;
        return amp * (static_cast<float>(static_cast<std::int32_t>(*s)) / 2147483648.0f);
    };
}

std::vector<float> render(Formant& fm, int blocks, Gen in,
                          float vowel, float shift, float res, float mix,
                          float drift = 0.0f) {
    fm.setParameter("vowel", vowel);
    fm.setParameter("shift", shift);
    fm.setParameter("res", res);
    fm.setParameter("mix", mix);
    fm.setParameter("drift", drift);
    fm.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bv(kSr, 1, kB), bs(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k)
            bi.at(0, k) = in ? in(n + k) : 0.0f;
        std::vector<const AudioBlock*> ins{in ? &bi : nullptr, nullptr, nullptr};
        fm.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
        n += kB;
    }
    return r;
}

double magAt(const std::vector<float>& x, double hz) {
    const std::size_t start = x.size() / 2;
    const std::size_t n = x.size() - start;
    double re = 0.0, im = 0.0, wsum = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    for (std::size_t i = start; i < x.size(); ++i) {
        const double tt = (double)(i - start);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * tt / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        wsum += w;
    }
    return std::sqrt(re * re + im * im) / wsum;
}

// frequência do pico de magnitude numa faixa (varredura grosseira)
double peakFreq(const std::vector<float>& x, double lo, double hi) {
    double best = lo, bestM = -1.0;
    for (double f = lo; f <= hi; f += 20.0) {
        const double m = magAt(x, f);
        if (m > bestM) { bestM = m; best = f; }
    }
    return best;
}

double rms(const std::vector<float>& x) {
    double s = 0.0;
    const std::size_t a = x.size() / 2;
    for (std::size_t i = a; i < x.size(); ++i) s += x[i] * (double)x[i];
    return std::sqrt(s / (double)(x.size() - a));
}

void testVowelAPeaks() {
    Formant fm;
    const auto r = render(fm, 60, whiteNoise(0.3f), 0.0f, 0.0f, 0.6f, 1.0f);
    // vogal A: F1≈600, F2≈1040. Vale em ~1500.
    EXPECT(magAt(r, 600) > 1.5 * magAt(r, 1500));
    EXPECT(magAt(r, 1040) > 1.5 * magAt(r, 1500));
}

void testVowelF2Moves() {
    Formant a;  const auto rA = render(a, 60, whiteNoise(0.3f), 0.0f, 0.0f, 0.7f, 1.0f);
    Formant i;  const auto rI = render(i, 60, whiteNoise(0.3f), 0.5f, 0.0f, 0.7f, 1.0f);
    Formant e;  const auto rE = render(e, 60, whiteNoise(0.3f), 0.25f, 0.0f, 0.7f, 1.0f);
    const double f2A = peakFreq(rA, 800, 1400);    // A: F2 ≈ 1040
    const double f2E = peakFreq(rE, 1200, 1900);   // E: F2 ≈ 1620
    const double f2I = peakFreq(rI, 1400, 2100);   // I: F2 ≈ 1750
    std::cerr << "  [F2] A=" << f2A << " E=" << f2E << " I=" << f2I << '\n';
    EXPECT(f2A < f2E);
    EXPECT(f2E <= f2I + 60.0);
    EXPECT(f2A < 1300.0 && f2I > 1500.0);
}

void testShiftRaisesSpectrum() {
    Formant lo;  const auto rl = render(lo, 60, whiteNoise(0.3f), 0.0f, 0.0f, 0.5f, 1.0f);
    Formant hi;  const auto rh = render(hi, 60, whiteNoise(0.3f), 0.0f, 0.8f, 0.5f, 1.0f);
    // shift up empurra o F1 de A (600) pra ~600·2,3 ≈ 1380
    EXPECT(peakFreq(rh, 900, 1800) > peakFreq(rl, 400, 900) + 300.0);
}

void testResNarrows() {
    Formant broad;  const auto rb = render(broad, 60, whiteNoise(0.3f), 0.0f, 0.0f, 0.05f, 1.0f);
    Formant sharp;  const auto rs = render(sharp, 60, whiteNoise(0.3f), 0.0f, 0.0f, 0.95f, 1.0f);
    // razão vale/pico cai com res alto (bandas mais estreitas)
    const double ratioB = magAt(rb, 1500) / (magAt(rb, 600) + 1e-9);
    const double ratioS = magAt(rs, 1500) / (magAt(rs, 600) + 1e-9);
    EXPECT(ratioS < ratioB * 0.7);
}

void testMixDry() {
    Formant fm;
    auto sig = whiteNoise(0.4f);
    const auto r = render(fm, 10, sig, 0.3f, 0.2f, 0.8f, 0.0f);
    auto ref = whiteNoise(0.4f);
    for (std::size_t i = 0; i < r.size(); ++i)
        EXPECT(std::fabs(r[i] - ref(i)) < 1e-6f);
}

void testSilentInput() {
    Formant fm;
    const auto r = render(fm, 20, nullptr, 0.5f, 0.0f, 0.9f, 1.0f);
    for (float v : r) EXPECT(v == 0.0f);
}

void testDeterminism() {
    Formant a, b;
    const auto ra = render(a, 30, whiteNoise(0.3f), 0.4f, 0.3f, 0.6f, 1.0f, 0.5f);
    const auto rb = render(b, 30, whiteNoise(0.3f), 0.4f, 0.3f, 0.6f, 1.0f, 0.5f);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testBounded() {
    for (float vw = 0.0f; vw <= 1.0f; vw += 0.2f) {
        Formant fm;
        const auto r = render(fm, 30, whiteNoise(1.0f), vw, 1.0f, 1.0f, 1.0f, 1.0f);
        for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
        EXPECT(rms(r) > 0.001);
    }
}

// ---- vocoder (modo, +2026-09-07) ----

// renderiza com carrier em `in` (índice 0) e modulador em `mod` (índice 3)
std::vector<float> renderVoc(Formant& fm, int blocks, Gen carrier, Gen mod,
                             float vocoder, float vowel = 0.3f) {
    fm.setParameter("vowel", vowel);
    fm.setParameter("shift", 0.0f);
    fm.setParameter("res", 0.4f);
    fm.setParameter("mix", 1.0f);
    fm.setParameter("drift", 0.0f);
    fm.setParameter("vocoder", vocoder);
    fm.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bv(kSr, 1, kB), bs(kSr, 1, kB), bm(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = carrier ? carrier(n + k) : 0.0f;
            bm.at(0, k) = mod ? mod(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{&bi, nullptr, nullptr,
                                           mod ? &bm : nullptr};
        fm.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
        n += kB;
    }
    return r;
}

Gen saw(double hz, float amp) {
    return [hz, amp](std::size_t i) {
        const double ph = std::fmod((double)i * hz / kSr, 1.0);
        return amp * static_cast<float>(2.0 * ph - 1.0);
    };
}

// modulador de "sílabas": ruído de banda larga com envelope de 4 Hz
Gen syllables(float amp) {
    auto s = std::make_shared<std::uint32_t>(0xBEEF01u);
    return [s, amp](std::size_t i) {
        *s = *s * 1664525u + 1013904223u;
        const float wn = static_cast<float>(static_cast<std::int32_t>(*s))
                       / 2147483648.0f;
        const double t = (double)i / kSr;
        const float env = (std::fmod(t, 0.25) < 0.12) ? 1.0f : 0.03f;
        return amp * wn * env;
    };
}

void testVocoderZeroIsClassicFormant() {
    // vocoder=0 → idêntico ao FORMANT sem modulador
    Formant a;
    const auto ra = renderVoc(a, 60, saw(110.0, 0.5f), syllables(0.4f), 0.0f);
    Formant b;
    const auto rb = render(b, 60, saw(110.0, 0.5f), 0.3f, 0.0f, 0.4f, 1.0f);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testVocoderFollowsModulatorEnvelope() {
    Formant fm;
    const auto y = renderVoc(fm, 300, saw(110.0, 0.5f), syllables(0.4f), 1.0f);
    // RMS durante as "sílabas" muito maior que nos vãos
    double loud = 0.0, quiet = 0.0; std::size_t nl = 0, nq = 0;
    for (std::size_t i = y.size() / 3; i < y.size(); ++i) {
        const double t = (double)i / kSr;
        if (std::fmod(t, 0.25) < 0.12) { loud += y[i] * (double)y[i]; ++nl; }
        else { quiet += y[i] * (double)y[i]; ++nq; }
    }
    const double lr = std::sqrt(loud / std::max<std::size_t>(1, nl));
    const double qr = std::sqrt(quiet / std::max<std::size_t>(1, nq));
    EXPECT(lr > 0.01);
    EXPECT(lr > 2.0 * qr);
}

void testVocoderNeedsModulator() {
    // vocoder=1 mas sem `mod` cabeado → cai no FORMANT clássico
    Formant a;
    const auto ra = renderVoc(a, 40, saw(110.0, 0.5f), nullptr, 1.0f);
    Formant b;
    const auto rb = render(b, 40, saw(110.0, 0.5f), 0.3f, 0.0f, 0.4f, 1.0f);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testVocoderBounded() {
    Formant fm;
    const auto y = renderVoc(fm, 200, saw(90.0, 1.0f), syllables(1.0f), 1.0f,
                             0.8f);
    for (float v : y) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testPanel() {
    Formant fm;
    check(validatePanel(fm).empty(), "painel FORMANT fecha");
    std::cout << renderAscii(fm);
}

}  // namespace

int main() {
    testVowelAPeaks();
    testVowelF2Moves();
    testShiftRaisesSpectrum();
    testResNarrows();
    testMixDry();
    testSilentInput();
    testDeterminism();
    testBounded();
    testVocoderZeroIsClassicFormant();
    testVocoderFollowsModulatorEnvelope();
    testVocoderNeedsModulator();
    testVocoderBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular FORMANT tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
