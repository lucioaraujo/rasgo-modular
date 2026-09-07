// Teste isolado do Módulo 52 (SWIRL — chorus/flanger/ensemble/phaser).
// Critérios do dossiê `dossies/52_swirl.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Swirl.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
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
struct Ins { Gen in, rateMod, mixMod; };

// devolve o canal `outIdx` (0 = L, 1 = R)
std::vector<float> run(Swirl& s, int blocks, const Ins& g, int outIdx = 0) {
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), br(kSr, 1, kB), bm(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = g.in ? g.in(n + k) : 0.0f;
            br.at(0, k) = g.rateMod ? g.rateMod(n + k) : 0.0f;
            bm.at(0, k) = g.mixMod ? g.mixMod(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.in ? &bi : nullptr, g.rateMod ? &br : nullptr,
            g.mixMod ? &bm : nullptr};
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c) s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
}
double magAt(const std::vector<float>& x, double hz, std::size_t a, std::size_t b) {
    double re = 0.0, im = 0.0, ws = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    const std::size_t n = b - a;
    for (std::size_t i = a; i < b && i < x.size(); ++i) {
        const double t = (double)(i - a);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * t / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        ws += w;
    }
    return std::sqrt(re * re + im * im) / ws;
}
double bandEnergy(const std::vector<float>& x, double f0, double f1,
                  std::size_t a, std::size_t b) {
    double s = 0.0;
    for (double hz = f0; hz <= f1; hz += (f1 - f0) / 12.0)
        s += magAt(x, hz, a, b);
    return s;
}

Gen sine(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(2.0 * M_PI * hz * (double)n / kSr);
    };
}
float hnoise(std::uint32_t n) {
    n = (n << 13) ^ n;
    const std::uint32_t m = n * (n * n * 15731u + 789221u) + 1376312589u;
    return 1.0f - (float)(m & 0x7fffffffu) / 1073741824.0f;
}
Gen noise(float amp) {
    return [amp](std::size_t n) {
        return amp * hnoise(static_cast<std::uint32_t>(n) + 3u);
    };
}

void finite(const std::vector<float>& r, float lim = 1.6f) {
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < lim);
}

void testBypass() {
    Swirl s;
    s.setParameter("mix", 0.0f);
    s.setParameter("type", 1.0f);
    s.setParameter("feedback", 0.8f);
    Ins g; g.in = sine(300.0, 0.5f);
    const auto r = run(s, 40, g);
    bool eq = true;
    for (std::size_t i = 0; i < r.size(); ++i) {
        const float want = 0.5f * std::sin(2.0 * M_PI * 300.0 * (double)i / kSr);
        if (std::fabs(r[i] - want) > 1e-6f) eq = false;
    }
    EXPECT(eq);
}

void testChorusWidensAndStereo() {
    Swirl s;
    s.setParameter("type", 0.0f);       // chorus
    s.setParameter("mix", 0.5f);
    s.setParameter("depth", 0.7f);
    s.setParameter("spread", 0.8f);
    s.setParameter("rate", 1.5f);
    s.setParameter("age", 0.0f);
    Ins g; g.in = sine(1000.0, 0.5f);
    const auto l = run(s, 200, g, 0);
    const auto rr = run(s, 200, g, 1);

    Swirl dry; dry.setParameter("mix", 0.0f);
    const auto d = run(dry, 200, g, 0);

    // bandas laterais: energia FORA de ±40 Hz do pico sobe
    const double sideWet = bandEnergy(l, 1060.0, 1300.0, 10000, 45000);
    const double sideDry = bandEnergy(d, 1060.0, 1300.0, 10000, 45000);
    EXPECT(sideWet > 3.0 * sideDry + 1e-4);
    // L ≠ R com spread
    double diff = 0.0;
    for (std::size_t i = 10000; i < l.size() && i < rr.size(); ++i)
        diff += std::fabs(l[i] - rr[i]);
    EXPECT(diff > 1.0);
    finite(l);
}

void testFlangerFeedbackSharpens() {
    auto peak = [](float fb) {
        Swirl s;
        s.setParameter("type", 1.0f);
        s.setParameter("mix", 0.5f);
        s.setParameter("depth", 0.0f);       // atraso PARADO → pente estático
        s.setParameter("feedback", fb);
        s.setParameter("age", 0.0f);
        Ins g; g.in = noise(0.5f);
        const auto r = run(s, 200, g);
        // energia num pico do pente (base ~1,2 ms → f ~ 830 Hz e harmônicos)
        return magAt(r, 833.0, 20000, 45000);
    };
    EXPECT(peak(0.9f) > 1.4 * peak(0.0f));
}

void testFlangerMoves() {
    Swirl s;
    s.setParameter("type", 1.0f);
    s.setParameter("mix", 0.5f);
    s.setParameter("depth", 1.0f);
    s.setParameter("feedback", 0.6f);
    s.setParameter("rate", 0.7f);
    s.setParameter("age", 0.0f);
    Ins g; g.in = noise(0.5f);
    const auto r = run(s, 500, g);
    // o notch do pente varre → a energia numa FREQUÊNCIA fixa (200 Hz,
    // dentro da faixa de varredura) sobe e desce no tempo
    double lo = 1e9, hi = -1e9;
    for (std::size_t w = 20000; w + 8000 < r.size(); w += 8000) {
        const double m = magAt(r, 200.0, w, w + 8000);
        lo = std::min(lo, m); hi = std::max(hi, m);
    }
    EXPECT(hi > 2.0 * lo + 1e-4);
    finite(r);
}

void testPhaser() {
    Swirl ph;
    ph.setParameter("type", 3.0f);
    ph.setParameter("mix", 0.6f);
    ph.setParameter("depth", 0.8f);
    ph.setParameter("feedback", 0.5f);
    ph.setParameter("rate", 0.5f);
    Ins g; g.in = noise(0.5f);
    const auto rp = run(ph, 300, g);

    Swirl fl;
    fl.setParameter("type", 1.0f);
    fl.setParameter("mix", 0.6f);
    fl.setParameter("depth", 0.8f);
    fl.setParameter("feedback", 0.5f);
    fl.setParameter("rate", 0.5f);
    const auto rf = run(fl, 300, g);

    // phaser ≠ flanger no mesmo gesto
    double diff = 0.0;
    for (std::size_t i = 10000; i < rp.size(); ++i)
        diff += std::fabs(rp[i] - rf[i]);
    EXPECT(diff > 5.0);
    EXPECT(rms(rp, 10000, 60000) > 0.05);
    finite(rp);
}

void testTone() {
    auto hi = [](float tone) {
        Swirl s;
        s.setParameter("type", 0.0f);
        s.setParameter("mix", 1.0f);
        s.setParameter("tone", tone);
        s.setParameter("age", 0.0f);
        Ins g; g.in = noise(0.5f);
        const auto r = run(s, 200, g);
        return bandEnergy(r, 4500.0, 9000.0, 20000, 45000);
    };
    EXPECT(hi(-1.0f) < 0.5 * hi(0.0f));
}

void testAutoOscillate() {
    Swirl s;
    s.setParameter("type", 1.0f);      // flanger
    s.setParameter("feedback", 1.0f);
    s.setParameter("age", 0.3f);
    s.setParameter("mix", 1.0f);
    s.setParameter("rate", 0.3f);
    Ins g;   // NADA na entrada
    const auto r = run(s, 300, g);
    EXPECT(rms(r, 40000, 70000) > 0.01);   // canta do piso de ruído do age
    finite(r, 2.0f);
}

void testDeterminism() {
    auto mk = [](Swirl& s) {
        s.setParameter("type", 2.0f);
        s.setParameter("age", 0.4f);
        s.setParameter("feedback", 0.3f);
        s.setParameter("depth", 0.6f);
        s.setParameter("mix", 0.5f);
    };
    Ins g; g.in = sine(220.0, 0.4f);
    Swirl a; mk(a);
    Swirl b; mk(b);
    const auto ra = run(a, 200, g), rb = run(b, 200, g);
    const auto la = run(a, 200, g, 1), lb = run(b, 200, g, 1);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = (ra[i] == rb[i]) && (la[i] == lb[i]);
    EXPECT(same);
}

void testCleanIsPureDeterministic() {
    Swirl s;
    s.setParameter("age", 0.0f);
    s.setParameter("type", 0.0f);
    Ins g; g.in = sine(440.0, 0.5f);
    const auto a = run(s, 120, g), b = run(s, 120, g);
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i) same = a[i] == b[i];
    EXPECT(same);
}

void testPanel() {
    Swirl s;
    check(validatePanel(s).empty(), "painel SWIRL fecha");
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testBypass();
    testChorusWidensAndStereo();
    testFlangerFeedbackSharpens();
    testFlangerMoves();
    testPhaser();
    testTone();
    testAutoOscillate();
    testDeterminism();
    testCleanIsPureDeterministic();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SWIRL tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
