// ORBIT (#90) — planetas em gravidade. Critérios de `dossies/90_orbit.md`.

#include "dsp/Orbit.hpp"

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

struct R { int conj = 0, peri = 0; std::vector<double> periAt; bool bounded = true; };

R run(Orbit& m, double seconds) {
    std::vector<AudioBlock> out(7, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr};
    R r; float pc = 0, pp = 0; long n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i, ++n) {
            for (int k = 0; k < 7; ++k) { const float v = out[std::size_t(k)].at(0, i); if (!std::isfinite(v) || std::fabs(v) > 1.0f) r.bounded = false; }
            if (pc < 0.5f && out[5].at(0, i) >= 0.5f) ++r.conj;
            if (pp < 0.5f && out[6].at(0, i) >= 0.5f) { ++r.peri; r.periAt.push_back(double(n) / kSr); }
            pc = out[5].at(0, i); pp = out[6].at(0, i);
        }
    }
    return r;
}

void testKeplerAndEnergy() {
    Orbit m;
    m.setParameter("mass", 0.0f); m.setParameter("ecc", 0.3f); m.setParameter("speed", 1.0f);
    m.prepare(kSr, kB);
    const double e0 = m.energy(0.0);
    const auto r = run(m, 20.0);
    const double e1 = m.energy(0.0);
    const double drift = std::fabs((e1 - e0) / e0);
    double period = 0;
    if (r.periAt.size() >= 3) period = (r.periAt.back() - r.periAt.front()) / double(r.periAt.size() - 1);
    std::printf("ORBIT: período do planeta 1 %.2f s · deriva de energia %.2e · conjunções %d\n", period, drift, r.conj);
    check(drift < 1e-3, "leapfrog: a energia não deriva (MASS 0)");
    // começa no afélio: com ECC 0,3 o semieixo é r/1,3 e, pela 3ª lei de
    // Kepler, o período é 2 s × (1/1,3)^1,5 ≈ 1,35 s
    const double kepler = 2.0 * std::pow(1.0 / 1.3, 1.5);
    check(r.peri >= 8 && std::fabs(period - kepler) < 0.1, "ECC 0,3: o período que Kepler prevê");
    check(r.conj >= 3, "CONJ: os planetas se alinham com o sol");
    check(r.bounded, "as saídas ficam em −1..1");
}

void testChaosStaysBounded() {
    Orbit m;
    m.setParameter("mass", 1.0f); m.setParameter("ecc", 0.8f); m.setParameter("speed", 4.0f);
    m.prepare(kSr, kB);
    const auto r = run(m, 20.0);
    check(r.bounded, "MASS 1 e ECC alto: caos, mas o sistema nunca se perde");
}

}  // namespace

int main() {
    testKeplerAndEnergy();
    testChaosStaysBounded();
    if (g_failures == 0) std::printf("test_orbit: ok\n");
    return g_failures == 0 ? 0 : 1;
}
