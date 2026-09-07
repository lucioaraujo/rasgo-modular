// Teste isolado do Módulo 53 (CRUSH — destruidor lo-fi / decimador).
// Critérios do dossiê `dossies/53_crush.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Crush.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <set>
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

std::vector<float> run(Crush& cr, int blocks, const Ins& g) {
    cr.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
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
        cr.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
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
Gen sine(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(2.0 * M_PI * hz * (double)n / kSr);
    };
}

void testBypass() {
    Crush cr;
    cr.setParameter("mix", 0.0f);
    cr.setParameter("bits", 3.0f);
    cr.setParameter("glitch", 0.8f);
    Ins g; g.in = sine(300.0, 0.6f);
    const auto r = run(cr, 40, g);
    bool eq = true;
    for (std::size_t i = 0; i < r.size(); ++i) {
        const float want = 0.6f * std::sin(2.0 * M_PI * 300.0 * (double)i / kSr);
        if (std::fabs(r[i] - want) > 1e-6f) eq = false;
    }
    EXPECT(eq);
}

void testStaircaseAliases() {
    Crush cr;
    cr.setParameter("rate", 1000.0f);   // S&H a 1 kHz → Nyquist 500 Hz
    cr.setParameter("bits", 16.0f);
    cr.setParameter("mix", 1.0f);
    Ins g; g.in = sine(700.0, 0.6f);     // 700 > 500 → dobra pra 300 Hz
    const auto r = run(cr, 200, g);
    const double alias = magAt(r, 300.0, 10000, 45000);
    const double orig = magAt(r, 700.0, 10000, 45000);
    EXPECT(alias > 0.1);          // a energia aparece em 300, que não existia
    EXPECT(alias > orig);         // e domina o 700 original
}

void testBitLevels() {
    Crush cr;
    cr.setParameter("bits", 2.0f);
    cr.setParameter("rate", 24000.0f);
    cr.setParameter("mix", 1.0f);
    cr.setParameter("drive", 1.0f);
    Ins g; g.in = sine(220.0, 0.9f);
    const auto r = run(cr, 60, g);
    std::set<int> lv;
    for (std::size_t i = 2000; i < r.size(); ++i)
        lv.insert(static_cast<int>(std::lround(r[i] * 1000.0f)));
    EXPECT(lv.size() <= 6);   // bits=2 → ~4 níveis (± borda)
}

void testWrapDiffersFromClip() {
    auto render = [](float wrap) {
        Crush cr;
        cr.setParameter("wrap", wrap);
        cr.setParameter("drive", 3.0f);
        cr.setParameter("bits", 16.0f);
        cr.setParameter("rate", 24000.0f);
        cr.setParameter("mix", 1.0f);
        Ins g; g.in = sine(200.0, 0.7f);
        return run(cr, 120, g);
    };
    const auto clip = render(0.0f), wrp = render(1.0f);
    double d = 0.0, e = 0.0;
    for (std::size_t i = 5000; i < clip.size(); ++i) {
        d += std::fabs(clip[i] - wrp[i]); e += std::fabs(clip[i]);
    }
    EXPECT(d > 0.3 * e);   // enrolar ≠ clipar, e bastante
    // as descontinuidades do wrap (saltos de ±2) são mais abruptas que o
    // topo achatado do clip → mais energia de agudo
    auto hi = [](const std::vector<float>& r) {
        double s = 0.0;
        for (double hz = 2000.0; hz <= 8000.0; hz += 500.0)
            s += magAt(r, hz, 5000, 50000);
        return s;
    };
    EXPECT(hi(wrp) > 1.3 * hi(clip) + 1e-4);
}

void testGlitchDropouts() {
    Crush cr;
    cr.setParameter("glitch", 0.85f);
    cr.setParameter("rate", 3000.0f);
    cr.setParameter("bits", 16.0f);
    cr.setParameter("mix", 1.0f);
    Ins g; g.in = sine(220.0, 0.7f);
    const auto r = run(cr, 200, g);
    std::size_t nearZero = 0, plateau = 0;
    for (std::size_t i = 5000; i < r.size(); ++i) {
        if (std::fabs(r[i]) < 1e-4f) ++nearZero;
        if (i > 5000 && r[i] == r[i - 1]) ++plateau;
    }
    const double frac = (double)nearZero / (double)(r.size() - 5000);
    const double pfrac = (double)plateau / (double)(r.size() - 5000);
    EXPECT(frac > 0.10);          // muitos dropouts
    EXPECT(pfrac > 0.3);          // e amostras travadas / hold grosso
}

void testJitterSmears() {
    // jitter irregulariza o clock do S&H → sidebands / smear em torno do
    // tom: energia FORA do pico (250 Hz, longe de 440) sobe
    auto off = [](float jit) {
        Crush cr;
        cr.setParameter("jitter", jit);
        cr.setParameter("rate", 3500.0f);
        cr.setParameter("bits", 16.0f);
        cr.setParameter("mix", 1.0f);
        Ins g; g.in = sine(440.0, 0.7f);
        const auto r = run(cr, 300, g);
        return magAt(r, 250.0, 10000, 70000)
             + magAt(r, 620.0, 10000, 70000);
    };
    EXPECT(off(0.8f) > 2.0 * off(0.0f) + 1e-4);
}

void testTone() {
    auto hf = [](float tone) {
        Crush cr;
        cr.setParameter("tone", tone);
        cr.setParameter("bits", 6.0f);
        cr.setParameter("rate", 24000.0f);
        cr.setParameter("mix", 1.0f);
        Ins g; g.in = sine(300.0, 0.6f);
        const auto r = run(cr, 120, g);
        double s = 0.0;
        for (double hz = 5000.0; hz <= 12000.0; hz += 700.0)
            s += magAt(r, hz, 5000, 50000);
        return s;
    };
    EXPECT(hf(-1.0f) < 0.4 * hf(0.0f) + 1e-5);
}

void testAutonomous() {
    Crush cr;
    cr.setParameter("mix", 1.0f);
    cr.setParameter("rate", 2000.0f);
    cr.setParameter("bits", 8.0f);
    Ins g;   // NADA na entrada
    const auto r = run(cr, 120, g);
    EXPECT(rms(r, 2000, r.size()) > 0.05);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.6f);
}

void testDeterminism() {
    auto mk = [](Crush& cr) {
        cr.setParameter("glitch", 0.5f);
        cr.setParameter("jitter", 0.6f);
        cr.setParameter("bits", 5.0f);
        cr.setParameter("rate", 4000.0f);
        cr.setParameter("mix", 0.8f);
    };
    Ins g; g.in = sine(180.0, 0.5f);
    Crush a; mk(a);
    Crush b; mk(b);
    const auto ra = run(a, 200, g), rb = run(b, 200, g);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testPanel() {
    Crush cr;
    check(validatePanel(cr).empty(), "painel CRUSH fecha");
    std::cout << renderAscii(cr);
}

}  // namespace

int main() {
    testBypass();
    testStaircaseAliases();
    testBitLevels();
    testWrapDiffersFromClip();
    testGlitchDropouts();
    testJitterSmears();
    testTone();
    testAutonomous();
    testDeterminism();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular CRUSH tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
