// BUDGET (#86) — o orçamento de eventos. Critérios de `dossies/86_budget.md`.

#include "dsp/Budget.hpp"

#include <cstdio>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 240;

// voz k dispara `rate[k]` gates por segundo; conta o que passa, por voz
std::array<int, 4> run(Budget& m, double seconds, std::array<double, 4> rate) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    std::array<AudioBlock, 4> g{AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB),
                                AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB)};
    std::vector<const AudioBlock*> ins{&g[0], &g[1], &g[2], &g[3], nullptr};
    std::array<int, 4> passed{};
    std::array<float, 4> prev{};
    long n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n)
            for (int k = 0; k < 4; ++k) {
                const long per = rate[static_cast<std::size_t>(k)] > 0 ? long(kSr / rate[static_cast<std::size_t>(k)]) : 0;
                g[static_cast<std::size_t>(k)].at(0, i) = per > 0 && ((n + 37 * k) % per) < 480 ? 1.0f : 0.0f;
            }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i)
            for (int k = 0; k < 4; ++k) {
                const float v = out[static_cast<std::size_t>(k)].at(0, i);
                if (prev[static_cast<std::size_t>(k)] < 0.5f && v >= 0.5f) ++passed[static_cast<std::size_t>(k)];
                prev[static_cast<std::size_t>(k)] = v;
            }
    }
    return passed;
}

void testCompeteCapsTotal() {
    Budget m;
    m.setParameter("ceiling", 8.0f); m.setParameter("cycle", 1.0f); m.setParameter("mode", 0.0f);
    const auto p = run(m, 4.0, {20.0, 2.0, 0.0, 0.0});   // a voz 1 fala muito
    const int total = p[0] + p[1];
    std::printf("COMPETE: voz1 %d, voz2 %d (teto 8/s, 4 s)\n", p[0], p[1]);
    check(total <= 32 && total >= 28, "COMPETE: o total respeita o teto por ciclo");
    check(p[1] <= 4, "COMPETE: a voz que fala muito leva quase tudo");
}

void testCooperateShares() {
    Budget m;
    m.setParameter("ceiling", 8.0f); m.setParameter("cycle", 1.0f); m.setParameter("mode", 1.0f);
    const auto p = run(m, 8.0, {20.0, 3.5, 0.0, 0.0});
    std::printf("COOPERA: voz1 %d, voz2 %d\n", p[0], p[1]);
    check(p[1] >= 2 * 7 - 2, "COOPERA: a voz quieta tem a sua parte garantida");
    check(p[0] + p[1] <= 64, "COOPERA: o total respeita o teto");
}

}  // namespace

int main() {
    testCompeteCapsTotal();
    testCooperateShares();
    if (g_failures == 0) std::printf("test_budget: ok\n");
    return g_failures == 0 ? 0 : 1;
}
