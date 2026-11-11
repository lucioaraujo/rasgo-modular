// EPICYCLE (#81) — círculos dentro de círculos. Critérios de `dossies/81_epicycle.md`.

#include "dsp/Epicycle.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 256;
constexpr double kPi = 3.14159265358979323846;

struct Out { std::vector<float> x, y; int gates = 0; float maxAbs = 0; };

Out run(Epicycle& m, double seconds) {
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr};
    Out o;
    float pg = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            o.x.push_back(out[0].at(0, i)); o.y.push_back(out[1].at(0, i));
            o.maxAbs = std::max({o.maxAbs, std::fabs(out[0].at(0, i)), std::fabs(out[1].at(0, i))});
            if (pg < 0.5f && out[4].at(0, i) >= 0.5f) ++o.gates;
            pg = out[4].at(0, i);
        }
    }
    return o;
}

double tone(const std::vector<float>& v, double hz) {
    double re = 0, im = 0;
    for (std::size_t n = 0; n < v.size(); ++n) {
        const double ph = 2.0 * kPi * hz * double(n) / kSr;
        re += v[n] * std::cos(ph); im += v[n] * std::sin(ph);
    }
    return 2.0 * std::sqrt(re * re + im * im) / double(v.size());
}

Epicycle make(float rate, float circles, float mode, float detune = 0.0f) {
    Epicycle m;
    m.setParameter("rate", rate); m.setParameter("circles", circles);
    m.setParameter("mode", mode); m.setParameter("detune", detune);
    m.prepare(kSr, kB);
    return m;
}

void testOneCircleIsACircle() {
    auto m = make(2.0f, 1.0f, 1.0f);
    const auto o = run(m, 3.0);
    bool unit = true;
    for (std::size_t i = 0; i < o.x.size(); ++i)
        if (std::fabs(std::hypot(o.x[i], o.y[i]) - 1.0) > 1e-3) unit = false;
    check(unit, "um círculo: |z| = 1");
    check(o.gates == 5, "GT: uma volta = um pulso (2 Hz × 3 s; a largada no zero não conta)");
}

void testSeriesShapes() {
    // SERRA: o 2º harmônico vale metade do 1º; QUADRADA: sem harmônicos pares
    auto saw = make(100.0f, 8.0f, 1.0f);
    const auto s = run(saw, 1.0);
    const double r21 = tone(s.y, 200.0) / tone(s.y, 100.0);
    auto sq = make(100.0f, 8.0f, 0.0f);
    const auto q = run(sq, 1.0);
    const double e2 = tone(q.y, 200.0) / tone(q.y, 100.0);
    const double r31 = tone(q.y, 300.0) / tone(q.y, 100.0);
    std::printf("SERRA h2/h1 %.3f · QUADRADA h2/h1 %.4f h3/h1 %.3f\n", r21, e2, r31);
    check(std::fabs(r21 - 0.5) < 0.01, "SERRA: raios 1/k (h2 = h1/2)");
    check(e2 < 0.01 && std::fabs(r31 - 1.0 / 3.0) < 0.01, "QUADRADA: só ímpares, raios 1/k (o resto é vazamento da janela)");
    check(s.maxAbs <= 1.0f && q.maxAbs <= 1.0f, "X/Y normalizados");
}

void testFibonacciChoicesDiffer() {
    auto fr = make(100.0f, 5.0f, 2.0f), fv = make(100.0f, 5.0f, 3.0f);
    const auto a = run(fr, 1.0), b = run(fv, 1.0);
    // FIB-RAIO: velocidade 4 tem raio 1/F₄ = 1/5; FIB-VELOC: há componente em 5·f e 8·f
    check(tone(a.y, 400.0) > 0.01 && tone(b.y, 400.0) < 1e-3, "FIB-RAIO tem 4·f; FIB-VELOC não");
    check(tone(b.y, 800.0) > 0.01 && tone(a.y, 800.0) < 1e-3, "FIB-VELOC tem 8·f (F₅); FIB-RAIO não");
}

void testDetuneNeverCloses() {
    auto d = make(5.0f, 4.0f, 1.0f, 1.0f), z = make(5.0f, 4.0f, 1.0f, 0.0f);
    const auto a = run(d, 2.0), b = run(z, 2.0);
    const std::size_t per = std::size_t(kSr / 5.0f);
    double da = 0, db = 0;
    for (std::size_t i = 0; i + per < a.x.size(); ++i) {
        da += std::fabs(a.x[i + per] - a.x[i]);
        db += std::fabs(b.x[i + per] - b.x[i]);
    }
    check(db / double(b.x.size()) < 1e-4 && da / double(a.x.size()) > 0.01,
          "DESV 0 fecha a figura a cada volta; DESV 1 não");
}

}  // namespace

int main() {
    testOneCircleIsACircle();
    testSeriesShapes();
    testFibonacciChoicesDiffer();
    testDetuneNeverCloses();
    if (g_failures == 0) std::printf("test_epicycle: ok\n");
    return g_failures == 0 ? 0 : 1;
}
