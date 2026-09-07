// Teste isolado do Módulo 55 (RESONATOR — ressoador modal externo).
// Critérios do dossiê `dossies/55_resonator.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Resonator.hpp"
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
struct Ins { Gen in, strike, freqMod; };

std::vector<float> run(Resonator& r, int blocks, const Ins& g, int outIdx = 0) {
    r.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bs(kSr, 1, kB), bf(kSr, 1, kB);
    std::vector<float> res;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = g.in ? g.in(n + k) : 0.0f;
            bs.at(0, k) = g.strike ? g.strike(n + k) : 0.0f;
            bf.at(0, k) = g.freqMod ? g.freqMod(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.in ? &bi : nullptr, g.strike ? &bs : nullptr,
            g.freqMod ? &bf : nullptr};
        r.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            res.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return res;
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
Gen impulseAt(std::size_t at) {
    return [at](std::size_t n) { return n == at ? 1.0f : 0.0f; };
}
Gen noise(float amp) {
    return [amp](std::size_t n) {
        std::uint32_t m = (std::uint32_t)n * 2654435761u + 12345u;
        m ^= m >> 15; m *= 2246822519u; m ^= m >> 13;
        return amp * ((float)(m & 0xffff) / 32768.0f - 1.0f);
    };
}

void testHarmonics() {
    Resonator r;
    r.setParameter("freq", 220.0f);
    r.setParameter("structure", 0.0f);
    r.setParameter("partials", 8.0f);
    r.setParameter("decay", 0.4f);
    r.setParameter("mix", 1.0f);
    // rajada de ruído por ~0,3 s, depois silêncio → excita e decai
    Ins g; g.in = [](std::size_t n){
        if (n >= 4000 && n < 4200) return 0.9f;
        return 0.0f;
    };
    const auto y = run(r, 260, g);
    const double f1 = magAt(y, 220.0, 5000, 40000);
    const double f2 = magAt(y, 440.0, 5000, 40000);
    const double f3 = magAt(y, 660.0, 5000, 40000);
    const double floor = magAt(y, 330.0, 5000, 40000);
    EXPECT(f1 > 4.0 * floor && f2 > 2.5 * floor && f3 > 1.5 * floor);
    EXPECT(rms(y, 5000, 12000) > 3.0 * rms(y, 45000, 60000));
}

void testDecayTime() {
    auto tail = [](float d) {
        Resonator r;
        r.setParameter("decay", d);
        r.setParameter("freq", 200.0f);
        r.setParameter("mix", 1.0f);
        Ins g; g.in = impulseAt(200);
        const auto y = run(r, 300, g);
        return rms(y, 40000, 70000);
    };
    EXPECT(tail(0.9f) > 5.0 * tail(0.1f));
}

void testDamp() {
    auto ratio = [](float damp) {
        Resonator r;
        r.setParameter("damp", damp);
        r.setParameter("freq", 150.0f);
        r.setParameter("partials", 10.0f);
        r.setParameter("decay", 0.7f);
        r.setParameter("mix", 1.0f);
        Ins g; g.in = impulseAt(200);
        const auto y = run(r, 300, g);
        // energia do 6º harmônico na cauda / do fundamental na cauda
        const double hi = magAt(y, 900.0, 40000, 70000);
        const double lo = magAt(y, 150.0, 40000, 70000);
        return hi / std::max(1e-9, lo);
    };
    EXPECT(ratio(0.9f) < 0.5 * ratio(0.0f) + 1e-6);
}

void testInharmonic() {
    Resonator r;
    r.setParameter("structure", 1.0f);
    r.setParameter("freq", 200.0f);
    r.setParameter("partials", 8.0f);
    r.setParameter("decay", 0.5f);
    r.setParameter("mix", 1.0f);
    Ins g; g.in = impulseAt(200);
    const auto y = run(r, 200, g);
    // o 2º parcial esticado NÃO cai em 400 Hz exato
    const double at400 = magAt(y, 400.0, 2000, 40000);
    double best = 0.0;
    for (double hz = 405.0; hz <= 470.0; hz += 5.0)
        best = std::max(best, magAt(y, hz, 2000, 40000));
    EXPECT(best > 1.5 * at400 + 1e-4);
}

void testTiltCrosses() {
    auto lohi = [](float tilt) {
        Resonator r;
        r.setParameter("tilt", tilt);
        r.setParameter("freq", 120.0f);
        r.setParameter("partials", 12.0f);
        r.setParameter("decay", 0.6f);
        r.setParameter("mix", 1.0f);
        Ins g; g.in = noise(0.4f);
        const auto lo = run(r, 200, g, 0);
        const auto hi = run(r, 200, g, 2);
        return std::pair<double, double>(rms(lo, 10000, 45000),
                                        rms(hi, 10000, 45000));
    };
    const auto neg = lohi(-1.0f);
    const auto pos = lohi(1.0f);
    EXPECT(neg.first > 2.0 * neg.second);    // tilt<0 → low domina
    EXPECT(pos.second > 2.0 * pos.first);    // tilt>0 → high domina
}

void testBypass() {
    Resonator r;
    r.setParameter("mix", 0.0f);
    r.setParameter("decay", 0.9f);
    Ins g; g.in = noise(0.5f);
    const auto lo = run(r, 40, g, 0);
    bool eq = true;
    for (std::size_t i = 0; i < lo.size(); ++i) {
        std::uint32_t m = (std::uint32_t)i * 2654435761u + 12345u;
        m ^= m >> 15; m *= 2246822519u; m ^= m >> 13;
        const float want = 0.5f * ((float)(m & 0xffff) / 32768.0f - 1.0f);
        if (std::fabs(lo[i] - want) > 1e-6f) eq = false;
    }
    EXPECT(eq);
}

void testPosition() {
    Resonator r;
    r.setParameter("position", 0.5f);
    r.setParameter("freq", 200.0f);
    r.setParameter("partials", 8.0f);
    r.setParameter("decay", 0.5f);
    r.setParameter("mix", 1.0f);
    Ins g; g.in = noise(0.4f);
    const auto y = run(r, 200, g);
    // position=0,5 → o 2º parcial (par) some
    EXPECT(magAt(y, 400.0, 10000, 45000) < 0.3 * magAt(y, 200.0, 10000, 45000));
}

void testStrike() {
    Resonator r;
    r.setParameter("decay", 0.5f);
    r.setParameter("mix", 1.0f);
    Ins g;   // sem `in`
    g.strike = [](std::size_t n) { return (n >= 5000 && n < 5100) ? 1.0f : 0.0f; };
    const auto y = run(r, 120, g);
    EXPECT(rms(y, 6000, 20000) > 0.02);
}

void testAutonomous() {
    Resonator r;
    r.setParameter("decay", 0.8f);
    r.setParameter("mix", 1.0f);
    Ins g;   // NADA
    const auto y = run(r, 150, g);
    EXPECT(rms(y, 20000, 38000) > 0.004);
    for (float v : y) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
}

void testDeterminismAndStability() {
    Resonator a, b;
    for (Resonator* r : {&a, &b}) {
        r->setParameter("decay", 1.0f);
        r->setParameter("structure", 0.6f);
        r->setParameter("partials", 20.0f);
        r->setParameter("mix", 0.7f);
    }
    Ins g; g.in = noise(0.5f);
    const auto ya = run(a, 500, g), yb = run(b, 500, g);
    bool same = ya.size() == yb.size();
    for (std::size_t i = 0; same && i < ya.size(); ++i) same = ya[i] == yb[i];
    EXPECT(same);
    for (float v : ya) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
}

void testPanel() {
    Resonator r;
    check(validatePanel(r).empty(), "painel RESONATOR fecha");
    std::cout << renderAscii(r);
}

}  // namespace

int main() {
    testHarmonics();
    testDecayTime();
    testDamp();
    testInharmonic();
    testTiltCrosses();
    testBypass();
    testPosition();
    testStrike();
    testAutonomous();
    testDeterminismAndStability();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular RESONATOR tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
