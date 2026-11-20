// REFRACT (#93) — Snell em quatro camadas. Critérios de `dossies/93_refract.md`.

#include "dsp/Refract.hpp"

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

void testSnell() {
    std::array<float, 4> n{1.5f, 1.33f, 2.0f, 1.2f}, a{};
    float lat = 0;
    const int r = Refract::trace(30.0f, n, a, lat);
    // n1 sen θ1 = n2 sen θ2
    const double inv1 = 1.5 * std::sin(30.0 * 0.0174533), inv2 = 1.33 * std::sin(a[1] * 0.0174533);
    std::printf("θ1 30° no vidro → água %.1f°, camada 3 %.1f°, camada 4 %.1f° (passagens %d)\n", a[1], a[2], a[3], r);
    check(std::fabs(inv1 - inv2) < 1e-4, "Snell: n·sen θ se conserva");
    check(r == 4, "a 30° o raio sai do vidro ao ar");
}

void testCriticalAngle() {
    // vidro → ar: ângulo crítico arcsen(1/1,5) = 41,8°
    std::array<float, 4> n{1.5f, 1.5f, 1.5f, 1.5f}, a{};
    float lat = 0;
    const int below = Refract::trace(41.0f, n, a, lat);
    const int above = Refract::trace(43.0f, n, a, lat);
    std::printf("vidro → ar: 41° %d passagens · 43° %d passagens\n", below, above);
    check(below == 4 && above < 4, "reflexão total acima do ângulo crítico (41,8°)");
}

int tirCount(float shimmer, double seconds) {
    Refract m;
    m.setParameter("n1", 1.5f); m.setParameter("n2", 1.5f); m.setParameter("n3", 1.5f); m.setParameter("n4", 1.5f);
    m.setParameter("rate", 0.5f); m.setParameter("range", 45.0f); m.setParameter("shimmer", shimmer);
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr};
    int n = 0; float pg = 0; bool fin = true;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            for (int k = 0; k < 6; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) fin = false;
            if (pg < 0.5f && out[4].at(0, i) >= 0.5f) ++n;
            pg = out[4].at(0, i);
        }
    }
    check(fin, "finito");
    return n;
}

void testSweepAndShimmer() {
    const int clean = tirCount(0.0f, 10.0), shim = tirCount(1.0f, 10.0);
    std::printf("varredura ±45° a 0,5 Hz em 10 s: TIR %d (limpo) · %d (miragem)\n", clean, shim);
    // a varredura passa pelo crítico duas vezes por ciclo (±): 10 entradas em TIR
    check(clean >= 9 && clean <= 11, "a varredura entra em reflexão total duas vezes por ciclo");
    check(shim > clean, "a miragem liga e desliga a reflexão perto do crítico");
}

}  // namespace

int main() {
    testSnell();
    testCriticalAngle();
    testSweepAndShimmer();
    if (g_failures == 0) std::printf("test_refract: ok\n");
    return g_failures == 0 ? 0 : 1;
}
