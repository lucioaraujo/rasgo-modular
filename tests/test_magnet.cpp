// MAGNET (#89) — uma grade de ímãs (Ising). Critérios de `dossies/89_magnet.md`.

#include "dsp/Magnet.hpp"

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

struct R { float absMag = 0, energy = 0; int gates = 0; bool finite = true; };

R run(Magnet& m, double seconds) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr, nullptr};
    R r; float pg = 0; double acc = 0; long n = 0;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 4; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) r.finite = false;
            if (pg < 0.5f && out[0].at(0, i) >= 0.5f) ++r.gates;
            pg = out[0].at(0, i);
        }
        if (b > blocks / 2) { acc += std::fabs(out[1].at(0, kB - 1)); ++n; r.energy = out[2].at(0, kB - 1); }
    }
    r.absMag = float(acc / double(n));
    return r;
}

Magnet make(float temp, float coupling) {
    Magnet m;
    m.setParameter("temp", temp); m.setParameter("coupling", coupling);
    m.setParameter("rate", 30.0f); m.setParameter("sweep", 1.0f);
    m.prepare(kSr, kB);
    return m;
}

void testPhaseTransition() {
    auto cold = make(1.0f, 1.0f), hot = make(5.5f, 1.0f);
    const auto c = run(cold, 30.0), h = run(hot, 30.0);
    std::printf("|MAG| médio: frio (T 1,0) %.2f · quente (T 5,5) %.2f\n", c.absMag, h.absMag);
    check(c.absMag > 0.7f, "abaixo de Tc os ímãs se alinham (domínios grandes)");
    check(h.absMag < 0.35f, "acima de Tc o ruído vence");
    check(c.energy < h.energy, "frio: energia mais baixa");
    check(c.finite && h.finite, "finito");
}

void testAntiferromagnet() {
    auto af = make(1.0f, -1.0f);
    const auto a = run(af, 30.0);
    std::printf("antiferro (COUPL −1, T 1,0): |MAG| %.2f, energia %.2f\n", a.absMag, a.energy);
    check(a.absMag < 0.3f && a.energy < 0.3f, "COUPL negativo: tabuleiro de xadrez (magnetização ~0, energia baixa)");
}

void testGatesAndDeterminism() {
    auto a = make(2.3f, 1.0f), b = make(2.3f, 1.0f);
    const auto ra = run(a, 5.0), rb = run(b, 5.0);
    check(ra.gates > 20 && ra.gates == rb.gates, "GT: a cabeça lê a grade; o mesmo começo, o mesmo ritmo");
}

}  // namespace

int main() {
    testPhaseTransition();
    testAntiferromagnet();
    testGatesAndDeterminism();
    if (g_failures == 0) std::printf("test_magnet: ok\n");
    return g_failures == 0 ? 0 : 1;
}
