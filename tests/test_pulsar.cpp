// Teste isolado do Módulo 56 (PULSAR — síntese pulsar).
// Critérios do dossiê `dossies/56_pulsar.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Pulsar.hpp"
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
struct Ins { Gen pitch, formantMod; };

std::vector<float> run(Pulsar& p, int blocks, const Ins& g, int outIdx = 0) {
    p.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB), bf(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bp.at(0, k) = g.pitch ? g.pitch(n + k) : 0.0f;
            bf.at(0, k) = g.formantMod ? g.formantMod(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.pitch ? &bp : nullptr, g.formantMod ? &bf : nullptr};
        p.process(ins, out);
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
double centroid(const std::vector<float>& x, std::size_t a, std::size_t b) {
    double num = 0.0, den = 0.0;
    for (double hz = 80.0; hz <= 8000.0; hz += 80.0) {
        const double m = magAt(x, hz, a, b);
        num += hz * m; den += m;
    }
    return den > 1e-12 ? num / den : 0.0;
}
double band(const std::vector<float>& x, double f0, double f1,
            std::size_t a, std::size_t b) {
    double s = 0.0;
    for (double hz = f0; hz <= f1; hz += (f1 - f0) / 12.0)
        s += magAt(x, hz, a, b);
    return s;
}

void testPeriodic() {
    Pulsar p;
    p.setParameter("freq", 110.0f);
    p.setParameter("formant", 1.0f);
    p.setParameter("mask", 0.0f);
    p.setParameter("jitter", 0.0f);
    Ins g;
    const auto y = run(p, 200, g);
    const double f1 = magAt(y, 110.0, 5000, 40000);
    const double floor = magAt(y, 155.0, 5000, 40000);
    EXPECT(f1 > 5.0 * floor);
    EXPECT(rms(y, 5000, 45000) > 0.05);
}

void testFormantIndependent() {
    auto get = [](float formant) {
        Pulsar p;
        p.setParameter("freq", 110.0f);
        p.setParameter("formant", formant);
        p.setParameter("window", 0.4f);
        Ins g;
        return run(p, 200, g);
    };
    const auto lo = get(1.0f);
    const auto hi = get(5.0f);
    // formante alto = pulsaret mais curto = mais brilho: o centróide sobe
    EXPECT(centroid(hi, 5000, 45000) > 1.5 * centroid(lo, 5000, 45000));
    // a ALTURA não muda: 110 Hz continua sendo pico local nos dois casos
    // (o pente harmônico carrega a altura; a fundamental enfraquece mas fica)
    for (const auto& y : {lo, hi}) {
        const double f110 = magAt(y, 110.0, 5000, 45000);
        EXPECT(f110 > 2.0 * magAt(y, 165.0, 5000, 45000));
        EXPECT(f110 > magAt(y, 90.0, 5000, 45000));
    }
}

void testFormantMonotonic() {
    double prev = 0.0;
    bool mono = true;
    // a partir de formante 2 (em formante≈1 o pulsaret preenche o período
    // inteiro — caso degenerado sem silêncio, fora da faixa útil)
    for (float fm : {2.0f, 3.5f, 5.0f, 8.0f}) {
        Pulsar p;
        p.setParameter("freq", 100.0f);
        p.setParameter("formant", fm);
        Ins g;
        const auto y = run(p, 150, g);
        const double c = centroid(y, 5000, 35000);
        if (c <= prev) mono = false;
        prev = c;
    }
    EXPECT(mono);
}

void testWindow() {
    auto hiRatio = [](float w) {
        Pulsar p;
        p.setParameter("freq", 120.0f);
        p.setParameter("formant", 2.0f);
        p.setParameter("window", w);
        Ins g;
        const auto y = run(p, 200, g);
        return band(y, 6000.0, 12000.0, 5000, 45000)
             / std::max(1e-9, band(y, 200.0, 2000.0, 5000, 45000));
    };
    EXPECT(hiRatio(0.0f) > 3.0 * hiRatio(0.5f) + 1e-6);
}

void testMask() {
    auto y0 = [] {
        Pulsar p; p.setParameter("mask", 0.0f);
        Ins g; return run(p, 200, g);
    }();
    Pulsar pm; pm.setParameter("mask", 0.85f);
    Ins g; const auto ym = run(pm, 200, g);
    EXPECT(rms(ym, 5000, 45000) < 0.55 * rms(y0, 5000, 45000));
    std::size_t sil = 0;
    for (std::size_t i = 5000; i < ym.size(); ++i)
        if (std::fabs(ym[i]) < 1e-4f) ++sil;
    EXPECT((double)sil / (double)(ym.size() - 5000) > 0.2);
}

void testJitterSmears() {
    auto width = [](float jit) {
        Pulsar p;
        p.setParameter("freq", 110.0f);
        p.setParameter("jitter", jit);
        Ins g;
        const auto y = run(p, 400, g);
        const double at110 = magAt(y, 110.0, 10000, 90000);
        const double around = magAt(y, 95.0, 10000, 90000)
                            + magAt(y, 128.0, 10000, 90000);
        return around / std::max(1e-9, at110);
    };
    EXPECT(width(0.8f) > 3.0 * width(0.0f) + 1e-4);
}

void testSpreadAndLevel() {
    Pulsar p;
    p.setParameter("spread", 0.8f);
    Ins g;
    const auto l = run(p, 150, g, 0);
    const auto rr = run(p, 150, g, 1);
    double d = 0.0;
    for (std::size_t i = 0; i < l.size() && i < rr.size(); ++i)
        d += std::fabs(l[i] - rr[i]);
    EXPECT(d > 1.0);

    auto lvl = [](float x) {
        Pulsar q; q.setParameter("level", x); q.setParameter("spread", 0.0f);
        Ins gg; return rms(run(q, 150, gg), 5000, 35000);
    };
    const double r1 = lvl(0.3f), r2 = lvl(0.6f);
    EXPECT(r2 > 1.6 * r1 && r2 < 2.6 * r1);
}

void testDeterminism() {
    auto mk = [](Pulsar& p) {
        p.setParameter("mask", 0.4f);
        p.setParameter("jitter", 0.5f);
        p.setParameter("formant", 3.0f);
        p.setParameter("spread", 0.5f);
    };
    Ins g;
    Pulsar a; mk(a);
    Pulsar b; mk(b);
    const auto ra = run(a, 200, g), rb = run(b, 200, g);
    const auto la = run(a, 200, g, 1), lb = run(b, 200, g, 1);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = (ra[i] == rb[i]) && (la[i] == lb[i]);
    EXPECT(same);
}

void testBounded() {
    Pulsar p;
    p.setParameter("freq", 1000.0f);
    p.setParameter("formant", 6.0f);
    p.setParameter("shape", 1.0f);
    p.setParameter("window", 0.0f);
    p.setParameter("jitter", 1.0f);
    Ins g;
    const auto y = run(p, 150, g);
    for (float v : y) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
}

void testPanel() {
    Pulsar p;
    check(validatePanel(p).empty(), "painel PULSAR fecha");
    std::cout << renderAscii(p);
}

}  // namespace

int main() {
    testPeriodic();
    testFormantIndependent();
    testFormantMonotonic();
    testWindow();
    testMask();
    testJitterSmears();
    testSpreadAndLevel();
    testDeterminism();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular PULSAR tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
