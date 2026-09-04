// Teste isolado do Módulo 33 (MATRIX — matriz de roteamento 4×4).
// Critérios do dossiê `dossies/33_matrix.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Matrix.hpp"
#include "dsp/Noise.hpp"
#include "dsp/Oscillator.hpp"
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
void near(double a, double b, double t = 1e-4) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

struct Out { std::vector<float> o[4]; };

// 4 fontes: f[j](t) -> in_j. nullptr = porta livre.
Out run(Matrix& m, int blocks,
        float (*f0)(std::size_t), float (*f1)(std::size_t),
        float (*f2)(std::size_t), float (*f3)(std::size_t)) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock b[4] = {AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB),
                       AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB)};
    float (*fs[4])(std::size_t) = {f0, f1, f2, f3};
    Out r;
    std::size_t g = 0;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k)
            for (int j = 0; j < 4; ++j)
                b[j].at(0, k) = fs[j] ? fs[j](g + k) : 0.0f;
        std::vector<const AudioBlock*> ins{
            fs[0] ? &b[0] : nullptr, fs[1] ? &b[1] : nullptr,
            fs[2] ? &b[2] : nullptr, fs[3] ? &b[3] : nullptr};
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

float rampA(std::size_t t) { return -0.8f + 1.6f * ((t % 5000) / 5000.0f); }
float sineB(std::size_t t) { return 0.6f * std::sin(t * 0.02f); }
float sineC(std::size_t t) { return 0.4f * std::sin(t * 0.005f + 1.0f); }
float dcD(std::size_t)      { return 0.3f; }
float dc05(std::size_t)     { return 0.5f; }

void zeroDiag(Matrix& m) {
    for (int i = 0; i < 4; ++i)
        m.setParameter(std::string("g") + char('1' + i) + char('1' + i), 0.0f);
}

void testIdentity() {
    Matrix m;   // padrão: g11=g22=g33=g44=1, sat=0
    const Out r = run(m, 30, rampA, sineB, sineC, dcD);
    for (std::size_t i = 0; i < r.o[0].size(); i += 41) {
        near(r.o[0][i], rampA(i), 1e-5);
        near(r.o[1][i], sineB(i), 1e-5);
        near(r.o[2][i], sineC(i), 1e-5);
        near(r.o[3][i], dcD(i), 1e-5);
    }
}

void testSingleCell() {
    Matrix m; zeroDiag(m);
    m.setParameter("g21", 1.0f);   // in2 -> out1
    const Out r = run(m, 30, rampA, sineB, nullptr, nullptr);
    for (std::size_t i = 0; i < r.o[0].size(); i += 37) {
        near(r.o[0][i], sineB(i), 1e-5);
        near(r.o[1][i], 0.0, 1e-6);
    }
}

void testNegative() {
    Matrix m;
    m.setParameter("g11", -1.0f);
    const Out r = run(m, 20, rampA, nullptr, nullptr, nullptr);
    for (std::size_t i = 0; i < r.o[0].size(); i += 29)
        near(r.o[0][i], -rampA(i), 1e-5);
}

void testSum() {
    Matrix m; zeroDiag(m);
    m.setParameter("g11", 0.5f);
    m.setParameter("g21", 0.5f);
    const Out r = run(m, 20, rampA, sineB, nullptr, nullptr);
    for (std::size_t i = 0; i < r.o[0].size(); i += 31)
        near(r.o[0][i], 0.5 * (rampA(i) + sineB(i)), 1e-5);
}

void testLevel() {
    Matrix m;
    m.setParameter("level", 2.0f);
    const Out r = run(m, 20, rampA, nullptr, nullptr, nullptr);
    for (std::size_t i = 0; i < r.o[0].size(); i += 29)
        near(r.o[0][i], 2.0 * rampA(i), 1e-5);
}

void testNorm() {
    Matrix raw, nrm; zeroDiag(raw); zeroDiag(nrm);
    for (Matrix* m : {&raw, &nrm}) {
        m->setParameter("g11", 1.0f);
        m->setParameter("g21", 1.0f);   // 2 células a 1 na coluna 1
    }
    nrm.setParameter("norm", 1.0f);
    const Out rr = run(raw, 20, dc05, dc05, nullptr, nullptr);
    const Out rn = run(nrm, 20, dc05, dc05, nullptr, nullptr);
    near(rr.o[0].back(), 1.0, 1e-4);   // soma: 0.5 + 0.5
    near(rn.o[0].back(), 0.5, 1e-4);   // média: (0.5+0.5)/2
}

void testSat() {
    Matrix hard, off;
    for (Matrix* m : {&hard, &off}) m->setParameter("level", 2.0f);
    hard.setParameter("sat", 0.9f);
    // entrada 0.9 * level 2 = 1.8 -> sat deve puxar pra baixo
    const Out rh = run(hard, 20, [](std::size_t) { return 0.9f; },
                       nullptr, nullptr, nullptr);
    const Out ro = run(off, 20, [](std::size_t) { return 0.9f; },
                       nullptr, nullptr, nullptr);
    near(ro.o[0].back(), 1.8, 1e-4);
    check(std::fabs(rh.o[0].back()) < 1.4f, "sat alto limita a saída");
    check(std::fabs(rh.o[0].back()) > 0.7f, "sat não zera");
}

void testDeterminismNoDrift() {
    Matrix a, b;
    for (Matrix* m : {&a, &b}) {
        m->setParameter("g12", 0.3f);
        m->setParameter("g31", -0.4f);
        m->setParameter("drift", 0.0f);
    }
    const Out ra = run(a, 40, rampA, sineB, sineC, dcD);
    const Out rb = run(b, 40, rampA, sineB, sineC, dcD);
    bool same = true;
    for (int o = 0; o < 4 && same; ++o)
        for (std::size_t i = 0; i < ra.o[o].size() && same; ++i)
            if (ra.o[o][i] != rb.o[o][i]) same = false;
    EXPECT(same);
    // com entrada DC e drift=0, a saída é constante
    const Out dc = run(a, 40, dcD, nullptr, nullptr, nullptr);
    for (std::size_t i = 500; i < dc.o[0].size(); i += 97)
        near(dc.o[0][i], dc.o[0][500], 1e-6);
}

void testDriftMoves() {
    Matrix m;
    m.setParameter("drift", 0.8f);
    const Out r = run(m, 2200, dcD, nullptr, nullptr, nullptr);  // in DC, ~6 s
    float lo = 1e9f, hi = -1e9f;
    for (std::size_t i = 2000; i < r.o[0].size(); ++i) {
        lo = std::min(lo, r.o[0][i]);
        hi = std::max(hi, r.o[0][i]);
    }
    check(hi - lo > 0.01f, "drift alto: a célula respira (saída DC varia)");
}

void testRingMod() {
    // `ring = 1` na coluna 1 com g11 = g21 = 1: out1 = in1·in2 (ring-mod
    // de 4 quadrantes). Duas senoides f1, f2 -> energia em |f1±f2|, pouca
    // nas fundamentais. `ring = 0` -> soma linear (fundamentais passam).
    auto fA = [](std::size_t t) { return 0.7f * std::sin(t * 0.03f); };   // ~229 Hz
    auto fB = [](std::size_t t) { return 0.7f * std::sin(t * 0.017f); };  // ~130 Hz
    auto mag = [](const std::vector<float>& v, double w) {
        double re = 0, im = 0;
        for (std::size_t k = v.size() / 3; k < v.size(); ++k) {
            re += v[k] * std::cos(w * k); im += v[k] * std::sin(w * k);
        }
        return std::sqrt(re * re + im * im) / (v.size() * 2 / 3);
    };
    Matrix lin, rng;
    for (Matrix* m : {&lin, &rng}) {
        zeroDiag(*m);
        m->setParameter("g11", 1.0f);
        m->setParameter("g21", 1.0f);
    }
    rng.setParameter("ring", 1.0f);
    const Out rl = run(lin, 200, fA, fB, nullptr, nullptr);
    const Out rr = run(rng, 200, fA, fB, nullptr, nullptr);
    const double f1 = mag(rr.o[0], 0.03), f2 = mag(rr.o[0], 0.017);
    const double sumB = mag(rr.o[0], 0.047), difB = mag(rr.o[0], 0.013);
    check(sumB > 0.05 && difB > 0.05, "ring=1: bandas soma e diferença");
    check(f1 < sumB && f2 < sumB, "ring=1: fundamentais abaixo das bandas");
    check(mag(rl.o[0], 0.03) > 0.1, "ring=0: fundamental passa (soma linear)");
}

void testColumnIndependence() {
    Matrix base, alt;
    const Out rb = run(base, 20, rampA, sineB, sineC, dcD);
    alt.setParameter("g12", 0.9f);   // mexe na coluna 2
    alt.setParameter("g32", -0.7f);
    const Out ra = run(alt, 20, rampA, sineB, sineC, dcD);
    for (std::size_t i = 0; i < rb.o[0].size(); i += 53)
        near(ra.o[0][i], rb.o[0][i], 1e-6);   // out1 (coluna 1) intacto
    // mas out2 mudou
    bool o2changed = false;
    for (std::size_t i = 0; i < rb.o[1].size(); ++i)
        if (std::fabs(ra.o[1][i] - rb.o[1][i]) > 1e-4f) o2changed = true;
    check(o2changed, "mexer na coluna 2 muda out2");
}

void testNothingConnected() {
    Matrix m;
    const Out r = run(m, 20, nullptr, nullptr, nullptr, nullptr);
    for (int o = 0; o < 4; ++o)
        for (float v : r.o[o]) check(v == 0.0f, "sem entrada -> saída 0");
}

void testInGraph() {
    SignalGraph g;
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 180.0f);
    const auto noi = g.add(std::make_unique<Noise>());
    const auto mtx = g.add(std::make_unique<Matrix>());
    g.node(mtx).setParameter("g12", 0.5f);   // osc também vai pra out2
    g.node(mtx).setParameter("g21", 0.4f);   // noise também vai pra out1
    const auto f1 = g.add(std::make_unique<Filter>());
    const auto f2 = g.add(std::make_unique<Filter>());
    g.connect(osc, 2, mtx, 0);      // saw -> in1
    g.connect(noi, 0, mtx, 1);      // white -> in2
    g.connect(mtx, 0, f1, 0);       // out1 -> FILTER1
    g.connect(mtx, 1, f2, 0);       // out2 -> FILTER2
    g.prepare(kSr, 1, kB);
    AudioBlock o1(kSr, 1, kB), o2(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 2000; ++b) {
        g.process(o1, f1, 0);
        g.process(o2, f2, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(o1.at(0, k)) && std::isfinite(o2.at(0, k)));
            lo = std::min({lo, o1.at(0, k), o2.at(0, k)});
            hi = std::max({hi, o1.at(0, k), o2.at(0, k)});
        }
    }
    check(hi > 0.05f && lo < -0.05f, "MATRIX mistura no grafo");
}

void testPanel() {
    Matrix m;
    const std::string problem = validatePanel(m);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(m.panel().widgets.size() >= 25);
    std::cout << renderAscii(m);
}

}  // namespace

int main() {
    testIdentity();
    testSingleCell();
    testNegative();
    testSum();
    testLevel();
    testNorm();
    testSat();
    testDeterminismNoDrift();
    testDriftMoves();
    testRingMod();
    testColumnIndependence();
    testNothingConnected();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_matrix: OK\n";
    return g_failures == 0 ? 0 : 1;
}
