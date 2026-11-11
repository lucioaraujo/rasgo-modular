// POLYGON (#82) — polígonos inscritos num ciclo. Critérios de `dossies/82_polygon.md`.

#include "dsp/Polygon.hpp"

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

struct Count { int g[4] = {0, 0, 0, 0}; int coinc = 0, eos = 0; };

Count run(Polygon& m, double seconds, double clockHz = 0.0) {
    std::vector<AudioBlock> out(7, AudioBlock(kSr, 1, kB));
    AudioBlock clk(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{clockHz > 0 ? &clk : nullptr, nullptr};
    Count c;
    float prev[7] = {0, 0, 0, 0, 0, 0, 0};
    long n = 0;
    const long per = clockHz > 0 ? long(kSr / clockHz) : 1;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n) clk.at(0, i) = (n % per) < 240 ? 1.0f : 0.0f;
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 6; ++k) {
                const float v = out[static_cast<std::size_t>(k)].at(0, i);
                if (prev[k] < 0.5f && v >= 0.5f) {
                    if (k < 4) ++c.g[k]; else if (k == 4) ++c.coinc; else ++c.eos;
                }
                prev[k] = v;
            }
        }
    }
    return c;
}

void testThreeAgainstTwo() {
    Polygon m;
    m.setParameter("n1", 3.0f); m.setParameter("n2", 2.0f); m.setParameter("cycle", 1.0f);
    m.prepare(kSr, kB);
    const auto c = run(m, 3.02);
    std::printf("3:2 em 3 s: G1 %d G2 %d COINC %d EOS %d\n", c.g[0], c.g[1], c.coinc, c.eos);
    check(c.g[0] == 9 + 1 && c.g[1] == 6 + 1, "3:2 — 3 e 2 vértices por ciclo (com o do recomeço)");
    check(c.coinc == 4, "COINC: só no vértice comum (o começo de cada ciclo)");
    check(c.eos == 3, "EOS: no fim de cada ciclo");
}

void testPhaseRotationRemovesCoincidence() {
    Polygon m;
    m.setParameter("n1", 3.0f); m.setParameter("n2", 2.0f); m.setParameter("cycle", 1.0f);
    m.setParameter("phi1", 0.25f);   // um quarto de lado: 1/12, 5/12, 9/12
    m.prepare(kSr, kB);
    const auto c = run(m, 3.02);
    check(c.coinc == 0, "F1 um quarto de lado: o triângulo gira e não coincide mais com o segmento");
    check(c.g[0] == 9, "F1: os mesmos vértices, girados");
}

void testClockMode() {
    Polygon m;
    m.setParameter("n1", 4.0f); m.setParameter("n2", 0.0f); m.setParameter("steps", 8.0f);
    m.prepare(kSr, kB);
    const auto c = run(m, 4.02, 4.0);   // relógio a 4 Hz, ciclo = 8 pulsos = 2 s
    std::printf("relógio: G1 %d EOS %d\n", c.g[0], c.eos);
    check(c.eos == 2, "CLK: o ciclo dura PASSOS pulsos");
    check(c.g[0] >= 8 && c.g[0] <= 9, "CLK: 4 vértices por ciclo");
}

}  // namespace

int main() {
    testThreeAgainstTwo();
    testPhaseRotationRemovesCoincidence();
    testClockMode();
    if (g_failures == 0) std::printf("test_polygon: ok\n");
    return g_failures == 0 ? 0 : 1;
}
