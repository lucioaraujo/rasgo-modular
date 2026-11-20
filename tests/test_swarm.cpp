// SWARM (#98) — o cardume de Kuramoto. Critérios de `dossies/98_swarm.md`.

#include "dsp/Swarm.hpp"

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

struct R { double sync = 0; int g1 = 0, g2 = 0; bool finite = true; };

R run(float cohesion, double seconds, bool scareMid = false, double* afterScare = nullptr) {
    Swarm m;
    m.setParameter("rate", 2.0f); m.setParameter("spread", 0.12f); m.setParameter("cohesion", cohesion);
    m.prepare(kSr, kB);
    AudioBlock sc(kSr, 1, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{scareMid ? &sc : nullptr, nullptr, nullptr};
    R r; float p1 = 0, p2 = 0; double acc = 0; long n = 0;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t i = 0; i < kB; ++i) sc.at(0, i) = (b == blocks / 2 && i < 10) ? 1.0f : 0.0f;
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 6; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) r.finite = false;
            if (p1 < 0.5f && out[0].at(0, i) >= 0.5f) ++r.g1;
            if (p2 < 0.5f && out[1].at(0, i) >= 0.5f) ++r.g2;
            p1 = out[0].at(0, i); p2 = out[1].at(0, i);
        }
        if (b > blocks / 4 && (!scareMid || b < blocks / 2)) { acc += out[5].at(0, kB - 1); ++n; }
        if (scareMid && afterScare && b == blocks / 2 + 1) *afterScare = out[5].at(0, kB - 1);
    }
    r.sync = acc / double(n);
    return r;
}

void testCohesion() {
    const auto loose = run(0.0f, 30.0), school = run(1.0f, 30.0);
    std::printf("SYNC médio: COHES 0 → %.2f · COHES 1 → %.2f\n", loose.sync, school.sync);
    std::printf("pulsos em 30 s: soltos G1 %d / G2 %d · cardume G1 %d / G2 %d\n", loose.g1, loose.g2, school.g1, school.g2);
    check(school.sync > 0.95, "coeso: todos no mesmo pulso (homorritmia)");
    check(loose.sync < 0.7, "solto: cada um no seu tempo (heterofonia)");
    check(loose.g1 != loose.g2, "soltos, os andamentos diferem");
    check(std::abs(school.g1 - school.g2) <= 1, "coesos, o mesmo andamento");
    check(school.finite && loose.finite, "finito");
}

void testScare() {
    double after = 1.0;
    const auto r = run(1.0f, 30.0, true, &after);
    std::printf("susto: SYNC antes %.2f, logo depois %.2f\n", r.sync, after);
    check(after < r.sync - 0.2, "o susto espalha o cardume");
}

}  // namespace

int main() {
    testCohesion();
    testScare();
    if (g_failures == 0) std::printf("test_swarm: ok\n");
    return g_failures == 0 ? 0 : 1;
}
