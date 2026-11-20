// ELASTIC (#92) — massas e molas. Critérios de `dossies/92_elastic.md`.

#include "dsp/Elastic.hpp"

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

// a saída L de `seconds`, com ruído na entrada (ou nada)
std::vector<float> render(Elastic& m, double seconds, float noise) {
    AudioBlock in(kSr, 1, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{noise > 0 ? &in : nullptr, nullptr, nullptr};
    std::vector<float> y;
    std::uint32_t s = 12345;
    const int blocks = int(seconds * kSr / kB);
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t i = 0; i < kB; ++i) {
            s = s * 1664525u + 1013904223u;
            in.at(0, i) = noise * (float(s >> 8) / 8388608.0f - 1.0f);
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) y.push_back(out[0].at(0, i));
    }
    return y;
}

double goertzel(const std::vector<float>& y, std::size_t a, std::size_t b, double hz) {
    const double w = 2.0 * 3.14159265358979 * hz / kSr, cw = 2.0 * std::cos(w);
    double s1 = 0, s2 = 0;
    for (std::size_t i = a; i < b; ++i) { const double s0 = y[i] + cw * s1 - s2; s2 = s1; s1 = s0; }
    return s1 * s1 + s2 * s2 - cw * s1 * s2;
}

double rms(const std::vector<float>& y, std::size_t a, std::size_t b) {
    double e = 0; for (std::size_t i = a; i < b; ++i) e += double(y[i]) * y[i];
    return std::sqrt(e / double(b - a));
}

Elastic make(float masses, float tune, float damp, float stretch = 0.0f) {
    Elastic m;
    m.setParameter("masses", masses); m.setParameter("tune", tune);
    m.setParameter("damping", damp); m.setParameter("stretch", stretch);
    m.setParameter("mix", 1.0f);
    m.prepare(kSr, kB);
    return m;
}

void testModes() {
    auto m = make(4, 200.0f, 0.2f);
    const auto y = render(m, 1.0, 0.0f);   // só o dedilhado da abertura
    const double f1 = Elastic::modeHz(200.0f, 4, 1);
    const double on = goertzel(y, 0, 24000, f1), off = goertzel(y, 0, 24000, f1 * 1.3);
    std::printf("modo 1 teórico %.1f Hz: energia no modo / fora dele = %.1f\n", f1, on / off);
    check(on > 5.0 * off, "o dedilhado soa nos modos da corrente");
}

void testDamping() {
    auto lo = make(8, 180.0f, 0.1f), hi = make(8, 180.0f, 0.9f);
    const auto a = render(lo, 1.5, 0.0f), b = render(hi, 1.5, 0.0f);
    const double ta = rms(a, 48000, 72000) / rms(a, 0, 4800), tb = rms(b, 48000, 72000) / rms(b, 0, 4800);
    std::printf("cauda depois de 1 s: DAMP 0,1 → %.3f · DAMP 0,9 → %.5f\n", ta, tb);
    check(ta > 10.0 * tb, "mais DAMP, cauda mais curta");
}

void testLevelAndStretch() {
    auto m = make(8, 180.0f, 0.3f, 1.0f);
    const auto y = render(m, 3.0, 0.5f);
    const double r = rms(y, 48000, y.size());
    bool fin = true; for (float v : y) if (!std::isfinite(v) || std::fabs(v) > 1.01f) fin = false;
    std::printf("ruído 0,5 na entrada, STRCH 1: RMS de saída %.3f\n", r);
    check(r > 0.02 && r < 0.9, "o nível de saída fica na faixa do de entrada");
    check(fin, "finito e dentro de ±1 com a mola endurecida");
}

}  // namespace

int main() {
    testModes();
    testDamping();
    testLevelAndStretch();
    if (g_failures == 0) std::printf("test_elastic: ok\n");
    return g_failures == 0 ? 0 : 1;
}
