// Teste isolado do Módulo 51 (BOXCAR — averager de porta com reconstrução).
// Critérios do dossiê `dossies/51_boxcar.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Boxcar.hpp"
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
struct Ins { Gen in, trig, sweep, thr; };

std::vector<float> run(Boxcar& b, int blocks, const Ins& g, int outIdx = 0) {
    b.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bt(kSr, 1, kB), bs(kSr, 1, kB), br(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = g.in ? g.in(n + k) : 0.0f;
            bt.at(0, k) = g.trig ? g.trig(n + k) : 0.0f;
            bs.at(0, k) = g.sweep ? g.sweep(n + k) : 0.0f;
            br.at(0, k) = g.thr ? g.thr(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.in ? &bi : nullptr, g.trig ? &bt : nullptr,
            g.sweep ? &bs : nullptr, g.thr ? &br : nullptr};
        b.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c)
        s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
}
double stdev(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double m = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c) m += v[i];
    m /= std::max<std::size_t>(1, c);
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i)
        s += (v[i] - m) * (v[i] - m);
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
int risingEdges(const std::vector<float>& v) {
    int e = 0;
    for (std::size_t i = 1; i < v.size(); ++i)
        if (v[i - 1] < 0.5f && v[i] >= 0.5f) ++e;
    return e;
}

Gen sine(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(2.0 * M_PI * hz * (double)n / kSr);
    };
}
// ruído hash determinístico em [-1,1]
float hnoise(std::uint32_t n) {
    n = (n << 13) ^ n;
    const std::uint32_t m = n * (n * n * 15731u + 789221u) + 1376312589u;
    return 1.0f - (float)(m & 0x7fffffffu) / 1073741824.0f;
}
Gen noisy(double hz, float amp, float noiseAmp) {
    return [hz, amp, noiseAmp](std::size_t n) {
        return amp * std::sin(2.0 * M_PI * hz * (double)n / kSr)
             + noiseAmp * hnoise(static_cast<std::uint32_t>(n) + 7u);
    };
}
Gen pulses(double hz) {
    return [hz](std::size_t n) {
        const double p = hz / kSr;
        const double ph = std::fmod((double)n * p, 1.0);
        return ph < p * 20.0 ? 1.0f : 0.0f;   // pulso curto
    };
}

void finiteBounded(const std::vector<float>& r) {
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 2.0f);
}

void testFixedPointSAndH() {
    Boxcar b;
    b.setParameter("aperture", 0.0f);   // amostra pontual
    b.setParameter("scan", 0.0f);
    b.setParameter("mode", 0.0f);       // follower
    b.setParameter("average", 1.0f);    // sem média → S&H clássico
    Ins g;
    g.in = sine(100.0, 0.6f);
    g.trig = pulses(400.0);
    const auto r = run(b, 100, g);      // ~0,53 s
    // patamares: a saída só muda perto de cada trig (400 Hz → ~213 em 100
    // blocos) e segura entre eles
    int changes = 0;
    for (std::size_t i = 1; i < r.size(); ++i)
        if (std::fabs(r[i] - r[i - 1]) > 1e-6f) ++changes;
    EXPECT(changes > 120 && changes < 320);
    EXPECT(stdev(r, 2000, r.size()) > 0.05);   // e varia (não trava em 0)
    finiteBounded(r);
}

void testWindowMeanReducesNoise() {
    Boxcar b;
    b.setParameter("delay", 0.25f);
    b.setParameter("aperture", 0.1f);
    b.setParameter("average", 32.0f);
    b.setParameter("scan", 0.0f);
    b.setParameter("mode", 0.0f);
    Ins g;
    g.in = noisy(200.0, 0.4f, 0.8f);
    g.trig = pulses(200.0);            // travado no período do sinal
    const auto r = run(b, 400, g);     // ~2,1 s
    // fase fixa + média de N → a saída converge pra uma CONSTANTE:
    // desvio-padrão tardio bem menor que o inicial
    const double early = stdev(r, 3000, 15000);
    const double late = stdev(r, 60000, 90000);
    EXPECT(late < 0.4 * early);
    finiteBounded(r);
}

void testReconstruct() {
    Boxcar b;
    b.setParameter("aperture", 0.15f);
    b.setParameter("average", 16.0f);
    b.setParameter("scan", 1.0f);      // varre o período todo
    b.setParameter("mode", 1.0f);      // reconstruct
    Ins g;
    g.in = sine(150.0, 0.5f);
    g.trig = pulses(150.0);
    const auto r = run(b, 480, g);     // ~2,6 s (~10 varreduras)
    const double f0 = magAt(r, 150.0, 90000, 120000);
    EXPECT(f0 > 0.03);
    EXPECT(f0 > 4.0 * magAt(r, 300.0, 90000, 120000));
    EXPECT(f0 > 4.0 * magAt(r, 450.0, 90000, 120000));
    finiteBounded(r);
}

void testPureNoiseAveragesDown() {
    Boxcar b;
    b.setParameter("aperture", 0.15f);
    b.setParameter("average", 24.0f);
    b.setParameter("scan", 1.0f);
    b.setParameter("mode", 1.0f);
    Ins g;
    g.in = [](std::size_t n) {
        return 0.6f * hnoise(static_cast<std::uint32_t>(n) + 11u);
    };
    g.trig = pulses(200.0);
    const auto r = run(b, 500, g);
    // ruído descorrelacionado do trigger → a reconstrução tende a 0
    EXPECT(rms(r, 90000, 120000) < 0.45 * rms(r, 0, 4000) + 0.02);
    finiteBounded(r);
}

void testSelfTrigger() {
    Boxcar b;
    b.setParameter("aperture", 0.0f);
    b.setParameter("scan", 0.0f);
    b.setParameter("mode", 0.0f);
    b.setParameter("average", 1.0f);
    b.setParameter("delay", 0.3f);     // amostra longe do cruzamento (perto do pico)
    b.setParameter("thresh", 0.0f);
    Ins g;
    g.in = sine(83.0, 0.7f);           // sem cabo de trig → cruza o limiar ~83/s
    const auto r = run(b, 200, g);     // ~1,07 s
    int changes = 0;
    for (std::size_t i = 1; i < r.size(); ++i)
        if (std::fabs(r[i] - r[i - 1]) > 1e-6f) ++changes;
    EXPECT(changes > 20 && changes < 200);      // captura a cada cruzamento
    EXPECT(rms(r, 4000, r.size()) > 0.2);       // pegou o corpo do sinal, não o zero
    finiteBounded(r);
}

void testGeiger() {
    Boxcar on;
    on.setParameter("geiger", 1.0f);
    Ins g;
    const auto r = run(on, 400, g, 1);          // ~2,13 s, saída `geiger`
    const int ev = risingEdges(r);
    // λ(1) = 0,5 + 40 ≈ 40,5 ev/s → ~86 em 2,13 s; folga ±50%
    EXPECT(ev > 40 && ev < 170);
    for (float v : r) EXPECT(v == 0.0f || v == 1.0f);

    Boxcar off;
    off.setParameter("geiger", 0.0f);
    const auto r0 = run(off, 40, g, 1);
    for (float v : r0) EXPECT(v == 0.0f);
}

void testDeterminism() {
    auto mk = [](Boxcar& b) {
        b.setParameter("geiger", 0.5f);
        b.setParameter("scan", 0.3f);
        b.setParameter("average", 12.0f);
        b.setParameter("mode", 1.0f);
    };
    Ins g;
    g.in = noisy(180.0, 0.4f, 0.3f);
    g.trig = pulses(180.0);
    Boxcar a; mk(a);
    Boxcar b; mk(b);
    const auto ra = run(a, 200, g);
    const auto rb = run(b, 200, g);
    const auto ga = run(a, 200, g, 1);
    const auto gb = run(b, 200, g, 1);
    bool same = ra.size() == rb.size() && ga.size() == gb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = (ra[i] == rb[i]) && (ga[i] == gb[i]);
    EXPECT(same);
}

void testBlendBypass() {
    Boxcar b;
    b.setParameter("blend", 0.0f);
    Ins g;
    g.in = sine(220.0, 0.5f);
    g.trig = pulses(220.0);
    const auto r = run(b, 60, g);
    // blend=0 → out == in amostra a amostra
    bool eq = true;
    for (std::size_t i = 0; i < r.size(); ++i) {
        const float want = 0.5f * std::sin(2.0 * M_PI * 220.0 * (double)i / kSr);
        if (std::fabs(r[i] - want) > 1e-6f) eq = false;
    }
    EXPECT(eq);

    Boxcar b2;
    b2.setParameter("blend", 1.0f);
    b2.setParameter("aperture", 0.1f);
    const auto r2 = run(b2, 60, g);
    double diff = 0.0;
    for (std::size_t i = 5000; i < r2.size(); ++i) {
        const float want = 0.5f * std::sin(2.0 * M_PI * 220.0 * (double)i / kSr);
        diff += std::fabs(r2[i] - want);
    }
    EXPECT(diff > 1.0);   // blend=1 processa (difere do seco)
}

void testPeriodChange() {
    Boxcar b;
    b.setParameter("scan", 1.0f);
    b.setParameter("mode", 1.0f);
    b.setParameter("aperture", 0.15f);
    Ins g;
    g.in = sine(160.0, 0.5f);
    g.trig = [](std::size_t n) {
        const double hz = n < 120000 ? 200.0 : 100.0;   // período dobra na metade
        const double p = hz / kSr;
        const double ph = std::fmod((double)n * p, 1.0);
        return ph < p * 20.0 ? 1.0f : 0.0f;
    };
    const auto r = run(b, 600, g);
    finiteBounded(r);
    EXPECT(rms(r, 130000, 150000) > 0.01);   // continua reconstruindo
}

void testAutonomous() {
    Boxcar b;
    b.setParameter("rate", 4.0f);
    Ins g;   // NADA conectado
    const auto r = run(b, 200, g);
    EXPECT(rms(r, 0, r.size()) > 0.0);        // piso de ruído interno → não é zero puro
    finiteBounded(r);
}

void testPanel() {
    Boxcar b;
    check(validatePanel(b).empty(), "painel BOXCAR fecha");
    std::cout << renderAscii(b);
}

}  // namespace

int main() {
    testFixedPointSAndH();
    testWindowMeanReducesNoise();
    testReconstruct();
    testPureNoiseAveragesDown();
    testSelfTrigger();
    testGeiger();
    testDeterminism();
    testBlendBypass();
    testPeriodChange();
    testAutonomous();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular BOXCAR tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
