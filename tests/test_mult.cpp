// Teste isolado do Módulo 34 (MULT — múltiplo processado).
// Critérios do dossiê `dossies/34_mult.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Mult.hpp"
#include "dsp/Vca.hpp"
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
void near(double a, double b, double t = 1e-5) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

struct Out { std::vector<float> o[4]; };

Out run(Mult& m, int blocks, float (*f)(std::size_t)) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock bin(kSr, 1, kB);
    Out r;
    std::size_t g = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k)
            bin.at(0, k) = f ? f(g + k) : 0.0f;
        std::vector<const AudioBlock*> ins{f ? &bin : nullptr, nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            for (int o = 0; o < 4; ++o) {
                check(std::isfinite(out[o].at(0, k)), "finito");
                r.o[o].push_back(out[o].at(0, k));
            }
        g += kB;
    }
    return r;
}

float ramp(std::size_t t) { return -0.9f + 1.8f * ((t % 4000) / 4000.0f); }
float step3k(std::size_t t) { return t < 3000 ? -0.6f : 0.6f; }

void testPassThrough() {
    Mult m;   // padrão: scale=1, offset=0, slew=0
    const Out r = run(m, 20, ramp);
    for (std::size_t i = 0; i < r.o[0].size(); i += 31)
        for (int o = 0; o < 4; ++o)
            near(r.o[o][i], ramp(i));
}

void testInvert() {
    Mult m;
    m.setParameter("scale2", -1.0f);
    const Out r = run(m, 20, ramp);
    for (std::size_t i = 0; i < r.o[1].size(); i += 29) {
        near(r.o[1][i], -ramp(i));
        near(r.o[0][i], ramp(i));   // out1 intacto
    }
}

void testOffset() {
    Mult m;
    m.setParameter("offset3", 0.4f);
    const Out r = run(m, 20, ramp);
    for (std::size_t i = 0; i < r.o[2].size(); i += 27)
        near(r.o[2][i], ramp(i) + 0.4f);
}

void testScaleOffset() {
    Mult m;
    m.setParameter("scale4", 0.5f);
    m.setParameter("offset4", -0.2f);
    const Out r = run(m, 20, ramp);
    for (std::size_t i = 0; i < r.o[3].size(); i += 23)
        near(r.o[3][i], 0.5f * ramp(i) - 0.2f, 1e-5);
}

void testIndependence() {
    Mult a, b;
    const Out ra = run(a, 15, ramp);
    b.setParameter("scale1", -1.7f);
    b.setParameter("offset1", 0.5f);
    const Out rb = run(b, 15, ramp);
    for (std::size_t i = 0; i < ra.o[1].size(); i += 37)
        for (int o = 1; o < 4; ++o)
            near(rb.o[o][i], ra.o[o][i]);   // out2..4 intactos
    bool o1changed = false;
    for (std::size_t i = 0; i < ra.o[0].size(); ++i)
        if (std::fabs(rb.o[0][i] - ra.o[0][i]) > 1e-4f) o1changed = true;
    check(o1changed, "mexer scale1/offset1 muda out1");
}

void testVoltageSource() {
    Mult m;
    m.setParameter("offset1", 0.3f);
    m.setParameter("offset2", -0.5f);
    m.setParameter("offset3", 0.0f);
    m.setParameter("offset4", 0.9f);
    const Out r = run(m, 20, nullptr);   // sem `in`
    near(r.o[0].back(), 0.3);
    near(r.o[1].back(), -0.5);
    near(r.o[2].back(), 0.0);
    near(r.o[3].back(), 0.9);
}

void testSlewFollows() {
    Mult m;   // slew=0
    const Out r = run(m, 40, step3k);
    // out acompanha in sem atraso: o degrau em t=3000 aparece na saída
    // no mesmo lugar
    bool jumped = false;
    for (std::size_t i = 2999; i < 3002 && i < r.o[0].size(); ++i)
        if (std::fabs(r.o[0][i] - r.o[0][i - 1]) > 1.0f) jumped = true;
    check(jumped, "slew=0: a saída segue o degrau da entrada na hora");
}

void testSlewGlides() {
    Mult fast, slow;
    slow.setParameter("slew", 0.6f);
    const Out rf = run(fast, 40, step3k);
    const Out rs = run(slow, 40, step3k);
    float maxDf = 0, maxDs = 0;
    for (std::size_t i = 1; i < rf.o[0].size(); ++i)
        maxDf = std::max(maxDf, std::fabs(rf.o[0][i] - rf.o[0][i - 1]));
    for (std::size_t i = 1; i < rs.o[0].size(); ++i)
        maxDs = std::max(maxDs, std::fabs(rs.o[0][i] - rs.o[0][i - 1]));
    check(maxDf > 1.0f, "slew=0 -> degrau grande");
    check(maxDs < 0.02f, "slew alto -> glide (derivada por amostra pequena)");
}

void testDual() {
    // dual on: out1/out2 seguem `in` (0.5), out3/out4 seguem `in2` (-0.3).
    // dual off: os 4 seguem `in`.
    Mult on, off;
    on.setParameter("dual", 1.0f);
    off.setParameter("dual", 0.0f);
    for (Mult* m : {&on, &off}) m->prepare(kSr, kB);
    std::vector<AudioBlock> o1(4, AudioBlock(kSr, 1, kB));
    std::vector<AudioBlock> o2(4, AudioBlock(kSr, 1, kB));
    AudioBlock a(kSr, 1, kB), b(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) { a.at(0, k) = 0.5f; b.at(0, k) = -0.3f; }
    std::vector<const AudioBlock*> ins{&a, &b};
    for (int i = 0; i < 40; ++i) { on.process(ins, o1); off.process(ins, o2); }
    check(std::fabs(o1[0].at(0, 0) - 0.5f) < 1e-3f, "dual: out1 = in");
    check(std::fabs(o1[2].at(0, 0) - (-0.3f)) < 1e-3f, "dual: out3 = in2");
    check(std::fabs(o2[2].at(0, 0) - 0.5f) < 1e-3f, "sem dual: out3 = in");
}

void testInGraph() {
    SignalGraph g;
    const auto lfo = g.add(std::make_unique<FunctionGenerator>());
    g.node(lfo).setParameter("rate", 3.0f);
    const auto mlt = g.add(std::make_unique<Mult>());
    g.node(mlt).setParameter("scale2", -1.0f);   // out2 = -in
    g.node(mlt).setParameter("offset3", 0.5f);
    const auto vca = g.add(std::make_unique<Vca>());
    g.connect(lfo, 0, mlt, 0);      // função -> MULT.in
    g.connect(mlt, 1, vca, 1);      // out2 (invertido) -> VCA.cv
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 3000; ++b) {
        g.process(out, mlt, 0);    // observa out1 (= a própria função)
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.1f && lo < 0.9f, "MULT distribui a função no grafo");
}

void testDeterminism() {
    Mult a, b;
    for (Mult* m : {&a, &b}) {
        m->setParameter("scale1", 1.3f);
        m->setParameter("offset2", -0.3f);
        m->setParameter("slew", 0.4f);
    }
    const Out ra = run(a, 60, ramp);
    const Out rb = run(b, 60, ramp);
    bool same = true;
    for (int o = 0; o < 4 && same; ++o)
        for (std::size_t i = 0; i < ra.o[o].size() && same; ++i)
            if (ra.o[o][i] != rb.o[o][i]) same = false;
    EXPECT(same);
}

void testPanel() {
    Mult m;
    const std::string problem = validatePanel(m);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(m.panel().widgets.size() >= 14);
    std::cout << renderAscii(m);
}

}  // namespace

int main() {
    testPassThrough();
    testInvert();
    testOffset();
    testScaleOffset();
    testIndependence();
    testVoltageSource();
    testSlewFollows();
    testSlewGlides();
    testDual();
    testInGraph();
    testDeterminism();
    testPanel();
    if (g_failures == 0) std::cout << "test_mult: OK\n";
    return g_failures == 0 ? 0 : 1;
}
