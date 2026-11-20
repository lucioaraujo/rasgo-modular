// INTERFERENCE (#94) — franjas de Young. Critérios de `dossies/94_interference.md`.

#include "dsp/Interference.hpp"

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

struct R { int fringes = 0, darks = 0; double contrast = 0; bool finite = true; };

R run(float speed, float third, float coher, double seconds) {
    Interference m;
    m.setParameter("speed", speed); m.setParameter("third", third);
    m.setParameter("coherence", coher); m.setParameter("ratio", 1.08f);
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr};
    R r; float pf = 0, pd = 0, lo = 1, hi = 0;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 4; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) r.finite = false;
            if (pf < 0.5f && out[1].at(0, i) >= 0.5f) ++r.fringes;
            if (pd < 0.5f && out[2].at(0, i) >= 0.5f) ++r.darks;
            pf = out[1].at(0, i); pd = out[2].at(0, i);
            if (b > blocks / 2) { lo = std::min(lo, out[0].at(0, i)); hi = std::max(hi, out[0].at(0, i)); }
        }
    }
    r.contrast = hi - lo;
    return r;
}

void testYoung() {
    // duas fontes: I = cos²(πx) — uma franja clara por unidade de x
    check(std::fabs(Interference::intensity(0.0, 1.08f, 0.0f) - 1.0f) < 1e-6f, "x = 0: franja clara (I = 1)");
    check(Interference::intensity(0.5, 1.08f, 0.0f) < 1e-6f, "x = 1/2: franja escura (I = 0)");
    const auto r = run(4.0f, 0.0f, 1.0f, 10.0);
    std::printf("SPEED 4, duas fontes, 10 s: %d franjas claras, %d escuras\n", r.fringes, r.darks);
    check(r.fringes >= 39 && r.fringes <= 41, "SPEED franjas por segundo");
    check(r.darks >= 39 && r.darks <= 41, "uma escura entre duas claras");
}

void testMoire() {
    const auto two = run(4.0f, 0.0f, 1.0f, 20.0), three = run(4.0f, 1.0f, 1.0f, 20.0);
    std::printf("THIRD 0: %d franjas em 20 s · THIRD 1: %d (o moiré agrupa e apaga)\n", two.fringes, three.fringes);
    check(three.fringes < two.fringes, "com a terceira fonte, só as franjas que somam as três passam o limiar");
    check(three.fringes > 0, "o moiré ainda tem franjas");
}

void testCoherence() {
    const auto laser = run(4.0f, 1.0f, 1.0f, 20.0), lamp = run(4.0f, 1.0f, 0.0f, 20.0);
    std::printf("COHER 1: %d franjas · COHER 0: %d franjas\n", laser.fringes, lamp.fringes);
    check(laser.fringes != lamp.fringes, "a coerência parcial muda o padrão");
    check(laser.finite && lamp.finite, "finito");
}

}  // namespace

int main() {
    testYoung();
    testMoire();
    testCoherence();
    if (g_failures == 0) std::printf("test_interference: ok\n");
    return g_failures == 0 ? 0 : 1;
}
