// GENDYN (#101) — síntese estocástica dinâmica. Critérios de `dossies/101_gendyn.md`.

#include "dsp/Gendyn.hpp"

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

std::vector<float> render(float freq, float band, float ampStep, bool frozen, double seconds) {
    Gendyn m;
    m.setParameter("freq", freq); m.setParameter("band", band); m.setParameter("amp_step", ampStep);
    m.prepare(kSr, kB);
    AudioBlock fz(kSr, 1, kB);
    for (std::size_t i = 0; i < kB; ++i) fz.at(0, i) = frozen ? 1.0f : 0.0f;
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, frozen ? &fz : nullptr, nullptr};
    std::vector<float> y;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) y.push_back(out[0].at(0, i));
    }
    return y;
}

// a frequência dominante pela autocorrelação (entre 40 e 1000 Hz)
double pitchOf(const std::vector<float>& y) {
    const std::size_t a = 9600, n = 9600;
    std::vector<double> ac(1200, 0.0);
    double best = 0;
    for (std::size_t L = 48; L < 1200; ++L) {
        double s = 0; for (std::size_t i = a; i < a + n; ++i) s += double(y[i]) * y[i + L];
        ac[L] = s; best = std::max(best, s);
    }
    for (std::size_t L = 49; L + 1 < 1200; ++L)   // o primeiro pico perto do máximo
        if (ac[L] >= 0.9 * best && ac[L] >= ac[L - 1] && ac[L] >= ac[L + 1]) return kSr / double(L);
    return 0.0;
}

void testPitchedBreath() {
    const auto y = render(220.0f, 0.0f, 0.05f, false, 1.0);
    const double hz = pitchOf(y);
    std::printf("FREQ 220, BAND 0: altura medida %.1f Hz\n", hz);
    check(std::fabs(hz - 220.0) < 12.0, "com BAND 0, uma nota afinada");
}

void testWalkVsFreeze() {
    // 120 Hz: período inteiro de 400 amostras (a comparação não escorrega)
    const auto live = render(120.0f, 0.0f, 0.3f, false, 1.0), ice = render(120.0f, 0.0f, 0.3f, true, 1.0);
    // a diferença entre um período e o de 50 períodos depois (~0,45 s)
    const std::size_t P = 400;
    auto change = [&](const std::vector<float>& y) {
        double d = 0, e = 0;
        for (std::size_t i = 4800; i < 4800 + P * 4; ++i) { d += std::fabs(y[i] - y[i + 50 * P]); e += std::fabs(y[i]); }
        return d / (e + 1e-9);
    };
    const double cl = change(live), ci = change(ice);
    std::printf("mudança em 50 períodos: vivo %.3f · congelado %.4f\n", cl, ci);
    check(ci < 0.05, "FRZ: a onda se repete");
    check(cl > 10.0 * ci, "sem FRZ, a onda nunca se repete");
}

void testBoundsAndLevel() {
    const auto y = render(80.0f, 1.0f, 1.0f, false, 3.0);
    bool fin = true; double peak = 0;
    for (float v : y) { if (!std::isfinite(v)) fin = false; peak = std::max(peak, double(std::fabs(v))); }
    std::printf("BAND 1, AMP 1: pico %.2f\n", peak);
    check(fin && peak <= 1.05, "as barreiras seguram o passeio (finito, sem estouro)");
}

}  // namespace

int main() {
    testPitchedBreath();
    testWalkVsFreeze();
    testBoundsAndLevel();
    if (g_failures == 0) std::printf("test_gendyn: ok\n");
    return g_failures == 0 ? 0 : 1;
}
