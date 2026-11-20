// PREDATOR (#97) — Lotka–Volterra. Critérios de `dossies/97_predator.md`.

#include "dsp/Predator.hpp"

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

struct R { int catches = 0, flees = 0; double inv0 = 0, inv1 = 0, lagSec = 0; bool finite = true; };

R run(float period, float hunt, float wild, double seconds) {
    Predator m;
    m.setParameter("period", period); m.setParameter("hunt", hunt); m.setParameter("wild", wild);
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr};
    R r; float pc = 0, pf = 0;
    r.inv0 = m.invariant();
    // o atraso entre os picos de presa e predador
    double tPrey = -1, tPred = -1, lagAcc = 0; int lagN = 0;
    float p1 = 0, p2 = 0, q1 = 0, q2 = 0;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 5; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) r.finite = false;
            if (pc < 0.5f && out[2].at(0, i) >= 0.5f) ++r.catches;
            if (pf < 0.5f && out[3].at(0, i) >= 0.5f) ++r.flees;
            pc = out[2].at(0, i); pf = out[3].at(0, i);
        }
        const double t = double(b + 1) * kB / kSr;
        const float py = out[0].at(0, kB - 1), pd = out[1].at(0, kB - 1);
        if (p1 > p2 && p1 >= py) tPrey = t;
        if (q1 > q2 && q1 >= pd && tPrey > 0) { tPred = t; lagAcc += tPred - tPrey; ++lagN; }
        p2 = p1; p1 = py; q2 = q1; q1 = pd;
    }
    r.inv1 = m.invariant();
    r.lagSec = lagN ? lagAcc / lagN : 0;
    return r;
}

void testClosedOrbit() {
    const auto r = run(10.0f, 0.0f, 0.0f, 100.0);
    std::printf("sem caça nem ruído, 10 ciclos: invariante %.4f → %.4f; fugas %d\n", r.inv0, r.inv1, r.flees);
    check(std::fabs(r.inv1 - r.inv0) / r.inv0 < 0.02, "a órbita fecha (o invariante de Lotka–Volterra não deriva)");
    check(r.flees >= 9 && r.flees <= 11, "um pico de presa por ciclo");
    check(r.lagSec > 0.5 && r.lagSec < 5.0, "o predador vem depois da presa");
    check(r.finite, "finito");
}

void testHunt() {
    const auto calm = run(20.0f, 0.0f, 0.0f, 60.0), hungry = run(20.0f, 1.0f, 0.0f, 60.0);
    std::printf("capturas em 60 s: HUNT 0 → %d · HUNT 1 → %d\n", calm.catches, hungry.catches);
    check(calm.catches == 0 && hungry.catches > 3, "com HUNT, o predador caça quando está em alta");
}

void testWildDeterminism() {
    const auto a = run(15.0f, 0.4f, 1.0f, 30.0), b = run(15.0f, 0.4f, 1.0f, 30.0);
    check(a.catches == b.catches && a.flees == b.flees, "determinístico, mesmo com WILD");
    check(a.finite, "finito com o ambiente agitado");
}

}  // namespace

int main() {
    testClosedOrbit();
    testHunt();
    testWildDeterminism();
    if (g_failures == 0) std::printf("test_predator: ok\n");
    return g_failures == 0 ? 0 : 1;
}
