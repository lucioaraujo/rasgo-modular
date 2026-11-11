// SUPPLY (#88) — a alimentação variável. Critérios de `dossies/88_supply.md`.

#include "dsp/Supply.hpp"

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

struct R { float rail = 0, maxDev = 0; int fails = 0; };

R run(Supply& m, double seconds, float loadAmp) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock load(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&load, nullptr};
    R r; float pf = 0; long n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n) load.at(0, i) = loadAmp * float(std::sin(0.03 * double(n)));
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            if (pf < 0.5f && out[5].at(0, i) >= 0.5f) ++r.fails;
            pf = out[5].at(0, i);
            if (b * kB > std::size_t(seconds * kSr / 2))
                r.maxDev = std::max(r.maxDev, std::fabs(out[1].at(0, i) - out[2].at(0, i)));
        }
        r.rail = out[0].at(0, kB - 1);
    }
    return r;
}

void testLoadSagsRail() {
    Supply quiet, loud;
    for (Supply* s : {&quiet, &loud}) { s->setParameter("energy", 0.8f); s->setParameter("sag", 0.8f); s->setParameter("time", 0.3f); }
    const auto q = run(quiet, 4.0, 0.0f), l = run(loud, 4.0, 0.9f);
    std::printf("TRILHO: sem carga %.3f, com carga %.3f\n", q.rail, l.rail);
    check(std::fabs(q.rail - 0.8f) < 0.01f, "sem carga, o trilho é a ENERGIA");
    check(l.rail < 0.35f, "com carga, o trilho cede (a bateria sob esforço)");
    Supply weak;
    weak.setParameter("energy", 0.5f); weak.setParameter("sag", 0.8f); weak.setParameter("time", 0.3f);
    const auto w = run(weak, 4.0, 0.9f);
    check(w.rail < 0.25f && w.fails >= 1, "FALHA: o apagão quando o trilho afunda abaixo de 0,25");
}

void testWanderOnlyWhenLow() {
    Supply full, low;
    full.setParameter("energy", 1.0f); full.setParameter("wander", 1.0f);
    low.setParameter("energy", 0.1f); low.setParameter("wander", 1.0f);
    const auto f = run(full, 30.0, 0.0f), l = run(low, 30.0, 0.0f);
    std::printf("DERIVA (D1 − D2): trilho cheio %.4f, baixo %.4f oct\n", f.maxDev, l.maxDev);
    check(f.maxDev < 1e-4f, "trilho cheio: as vozes andam juntas");
    check(l.maxDev > 0.01f && l.maxDev <= 0.1f, "trilho baixo: as vozes se desencontram (até ~60 cents)");
}

}  // namespace

int main() {
    testLoadSagsRail();
    testWanderOnlyWhenLow();
    if (g_failures == 0) std::printf("test_supply: ok\n");
    return g_failures == 0 ? 0 : 1;
}
