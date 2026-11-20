// PRESSURE (#91) — gás numa caixa. Critérios de `dossies/91_pressure.md`.

#include "dsp/Pressure.hpp"

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

struct R { double pres = 0; int hits = 0; bool finite = true; };

R run(Pressure& m, double seconds) {
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr};
    R r; float pg = 0; double acc = 0; long n = 0;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 5; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) r.finite = false;
            if (pg < 0.5f && out[0].at(0, i) >= 0.5f) ++r.hits;
            pg = out[0].at(0, i);
        }
        if (b > blocks / 3) { acc += out[1].at(0, kB - 1); ++n; }
    }
    r.pres = acc / double(n);
    return r;
}

Pressure make(float n, float temp, float volume, float attract = 0.0f) {
    Pressure m;
    m.setParameter("particles", n); m.setParameter("temp", temp);
    m.setParameter("volume", volume); m.setParameter("attract", attract);
    m.setParameter("gate_len", 0.002f);
    m.prepare(kSr, kB);
    return m;
}

void testTemperature() {
    auto cold = make(8, 0.15f, 1.0f), hot = make(8, 0.8f, 1.0f);
    const auto c = run(cold, 20.0), h = run(hot, 20.0);
    std::printf("frio (T 0,15): %d choques, P %.3f · quente (T 0,8): %d choques, P %.3f\n", c.hits, c.pres, h.hits, h.pres);
    check(h.hits > 2 * c.hits, "mais quente, mais choques na parede (o ritmo acelera)");
    check(h.pres > 2.0 * c.pres, "mais quente, mais pressão");
    check(c.finite && h.finite, "finito");
}

void testBoyle() {
    auto big = make(10, 0.4f, 1.0f), small = make(10, 0.4f, 0.4f);
    const auto b = run(big, 20.0), s = run(small, 20.0);
    std::printf("Boyle: VOL 1,0 → P %.3f · VOL 0,4 → P %.3f (razão %.2f)\n", b.pres, s.pres, s.pres / b.pres);
    check(s.pres > 1.5 * b.pres, "comprimir aumenta a pressão (P·V ≈ constante)");
}

void testAttraction() {
    auto ideal = make(16, 0.25f, 1.0f, 0.0f), sticky = make(16, 0.25f, 1.0f, 1.0f);
    const auto i = run(ideal, 20.0), s = run(sticky, 20.0);
    std::printf("ATTR 0 → P %.3f · ATTR 1 → P %.3f\n", i.pres, s.pres);
    check(s.pres < i.pres, "com atração, o gás real empurra menos a parede");
    check(s.finite, "finito com atração forte");
}

void testDeterminism() {
    auto a = make(6, 0.3f, 1.0f), b = make(6, 0.3f, 1.0f);
    check(run(a, 5.0).hits == run(b, 5.0).hits, "determinístico");
}

}  // namespace

int main() {
    testTemperature();
    testBoyle();
    testAttraction();
    testDeterminism();
    if (g_failures == 0) std::printf("test_pressure: ok\n");
    return g_failures == 0 ? 0 : 1;
}
