// Teste isolado do Módulo 42 (ADDITIVE — oscilador aditivo / espectral).
// Critérios do dossiê `dossies/42_additive.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Additive.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
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

struct In {
    float pitch = 0.0f;   bool hasPitch = false;   // oitavas (1 V/oct)
    float fm = 0.0f;      bool hasFm = false;
    float tilt = 0.0f;    bool hasTilt = false;
    float stretch = 0.0f; bool hasStretch = false;
};

std::vector<float> render(Additive& a, int blocks, const In& in = {}) {
    a.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB), bf(kSr, 1, kB), bt(kSr, 1, kB), bs(kSr, 1, kB);
    std::vector<float> r;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bp.at(0, k) = in.pitch;
            bf.at(0, k) = in.fm;
            bt.at(0, k) = in.tilt;
            bs.at(0, k) = in.stretch;
        }
        std::vector<const AudioBlock*> ins{
            in.hasPitch ? &bp : nullptr,
            in.hasTilt ? &bt : nullptr,
            in.hasStretch ? &bs : nullptr,
            in.hasFm ? &bf : nullptr};
        a.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
    }
    return r;
}

// magnitude do sinal na frequência `hz` (DFT ponto a ponto, 2ª metade)
double magAt(const std::vector<float>& x, double hz) {
    const std::size_t start = x.size() / 2;
    double re = 0.0, im = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    for (std::size_t i = start; i < x.size(); ++i) {
        re += x[i] * std::cos(wk * static_cast<double>(i));
        im += x[i] * std::sin(wk * static_cast<double>(i));
    }
    const double n = static_cast<double>(x.size() - start);
    return std::sqrt(re * re + im * im) / n;
}

double rms(const std::vector<float>& x) {
    double s = 0.0;
    const std::size_t start = x.size() / 2;
    for (std::size_t i = start; i < x.size(); ++i) s += x[i] * (double)x[i];
    return std::sqrt(s / static_cast<double>(x.size() - start));
}

constexpr double kF0 = 110.0;

void testHarmonicDecay() {
    Additive a;
    a.setParameter("freq", 110.0f);
    a.setParameter("tilt", 0.5f);
    a.setParameter("odd", 0.0f);
    const auto r = render(a, 40);
    const double m1 = magAt(r, kF0);
    const double m2 = magAt(r, kF0 * 2);
    const double m3 = magAt(r, kF0 * 3);
    EXPECT(m1 > 0.05);
    EXPECT(m2 < m1 && m2 > 0.01);   // série cheia, decrescente
    EXPECT(m3 < m2);
}

void testTiltBrightens() {
    Additive dark;
    dark.setParameter("freq", 110.0f); dark.setParameter("tilt", 0.0f);
    Additive bright;
    bright.setParameter("freq", 110.0f); bright.setParameter("tilt", 1.0f);
    const auto rd = render(dark, 40);
    const auto rb = render(bright, 40);
    const double ratioD = magAt(rd, kF0 * 8) / (magAt(rd, kF0) + 1e-9);
    const double ratioB = magAt(rb, kF0 * 8) / (magAt(rb, kF0) + 1e-9);
    EXPECT(ratioB > ratioD * 3.0);   // brilho: harmônica alta mais forte, relativa
}

void testOddOnly() {
    Additive a;
    a.setParameter("freq", 110.0f);
    a.setParameter("tilt", 0.7f);
    a.setParameter("odd", 1.0f);
    const auto r = render(a, 40);
    const double m1 = magAt(r, kF0);
    const double m2 = magAt(r, kF0 * 2);
    const double m3 = magAt(r, kF0 * 3);
    const double m4 = magAt(r, kF0 * 4);
    EXPECT(m2 < 0.05 * m1);   // pares somem
    EXPECT(m4 < 0.05 * m3);
    EXPECT(m3 > 0.1 * m1);    // ímpares ficam
}

void testEvenOnly() {
    Additive a;
    a.setParameter("freq", 110.0f);
    a.setParameter("tilt", 0.7f);
    a.setParameter("odd", -1.0f);
    const auto r = render(a, 40);
    const double m2 = magAt(r, kF0 * 2);
    const double m3 = magAt(r, kF0 * 3);
    const double m5 = magAt(r, kF0 * 5);
    EXPECT(m3 < 0.05 * m2);   // ímpares somem
    EXPECT(m5 < 0.05 * m2);
}

void testStretchSharp() {
    Additive harm;
    harm.setParameter("freq", 110.0f); harm.setParameter("tilt", 1.0f);
    harm.setParameter("stretch", 0.0f);
    Additive str;
    str.setParameter("freq", 110.0f); str.setParameter("tilt", 1.0f);
    str.setParameter("stretch", 1.0f);
    const auto r0 = render(harm, 40);
    const auto r1 = render(str, 40);
    // parcial 16: harmônico fica em 16·f0; esticado sobe pra ~16,96·f0
    EXPECT(magAt(r0, 16 * kF0) > magAt(r0, 17 * kF0));
    EXPECT(magAt(r1, 17 * kF0) > magAt(r1, 16 * kF0));
}

void testComb() {
    Additive flat;
    flat.setParameter("freq", 110.0f); flat.setParameter("tilt", 1.0f);
    flat.setParameter("comb", 0.0f);
    Additive comb;
    comb.setParameter("freq", 110.0f); comb.setParameter("tilt", 1.0f);
    comb.setParameter("comb", 1.0f);
    const auto rf = render(flat, 40);
    const auto rc = render(comb, 40);
    // comb=1 -> 12 dentes -> parcial 8 cai num vale (cos(3π) = -1)
    EXPECT(magAt(rc, 8 * kF0) < 0.15 * magAt(rf, 8 * kF0));
    // parcial 4 fica num pico relativo -> preservado
    EXPECT(magAt(rc, 4 * kF0) > 0.4 * magAt(rf, 4 * kF0));
}

void testPitchTracking() {
    Additive a;
    a.setParameter("freq", 110.0f);
    a.setParameter("tilt", 0.5f);
    const auto r0 = render(a, 40);
    Additive b;
    b.setParameter("freq", 110.0f);
    b.setParameter("tilt", 0.5f);
    const auto r1 = render(b, 40, In{1.0f, true, 0, false, 0, false, 0, false});
    EXPECT(magAt(r1, 220.0) > 0.5 * magAt(r0, 110.0));
    EXPECT(magAt(r1, 110.0) < 0.3 * magAt(r1, 220.0));
}

void testNyquistClean() {
    Additive a;
    a.setParameter("freq", 6000.0f);
    a.setParameter("tilt", 1.0f);
    const auto r = render(a, 40);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 0.98f);
    // energia entre parciais (só apareceria por aliasing) baixa
    const double inter = magAt(r, 15000.0);
    const double part = magAt(r, 6000.0);
    EXPECT(inter < 0.05 * part);
}

void testFmLinear() {
    Additive a;
    a.setParameter("freq", 110.0f);
    a.setParameter("tilt", 0.5f);
    a.setParameter("fm_amount", 1.0f);
    const auto r = render(a, 40, In{0, false, 0.5f, true, 0, false, 0, false});
    // f0 · (1 + 0,5·1·4) = f0·3 -> fundamental sobe pra ~330
    EXPECT(magAt(r, 330.0) > magAt(r, 110.0));
}

void testDeterminism() {
    Additive a, b;
    for (Additive* x : {&a, &b}) {
        x->setParameter("freq", 130.0f);
        x->setParameter("tilt", 0.6f);
        x->setParameter("stretch", 0.4f);
        x->setParameter("comb", 0.3f);
        x->setParameter("drift", 0.5f);
    }
    const auto ra = render(a, 30);
    const auto rb = render(b, 30);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = ra[i] == rb[i];
    EXPECT(same);
}

void testBounded() {
    Additive a;
    a.setParameter("freq", 4000.0f);
    a.setParameter("tilt", 0.0f);
    a.setParameter("odd", 1.0f);
    a.setParameter("stretch", -1.0f);
    a.setParameter("comb", 1.0f);
    a.setParameter("drift", 1.0f);
    const auto r = render(a, 50);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 0.98f);
}

void testAutonomous() {
    Additive a;
    a.setParameter("freq", 90.0f);
    a.setParameter("tilt", 0.5f);
    a.setParameter("drift", 0.6f);
    const auto r = render(a, 60);
    EXPECT(rms(r) > 0.02);   // soa sozinho, sem nenhum cabo
    for (float v : r) EXPECT(std::isfinite(v));
}

void testPanel() {
    Additive a;
    check(validatePanel(a).empty(), "painel ADDITIVE fecha");
    std::cout << renderAscii(a);
}

}  // namespace

int main() {
    testHarmonicDecay();
    testTiltBrightens();
    testOddOnly();
    testEvenOnly();
    testStretchSharp();
    testComb();
    testPitchTracking();
    testNyquistClean();
    testFmLinear();
    testDeterminism();
    testBounded();
    testAutonomous();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular ADDITIVE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
