// COMPARADOR (#80) — o limiar como acontecimento. Critérios de
// `dossies/80_comparador.md`.

#include "dsp/Comparador.hpp"

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

struct Run { float mix = 0, inv = 0; int rises = 0, crosses = 0; };

template <typename FA, typename FB>
Run run(Comparador& m, int blocks, FA fa, FB fb) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock a(kSr, 1, kB), b(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&a, &b, nullptr};
    Run r;
    float pc = 0, px = 0;
    long n = 0;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t i = 0; i < kB; ++i, ++n) { a.at(0, i) = fa(n); b.at(0, i) = fb(n); }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            if (pc < 0.5f && out[2].at(0, i) >= 0.5f) ++r.rises;
            if (px < 0.5f && out[3].at(0, i) >= 0.5f) ++r.crosses;
            pc = out[2].at(0, i); px = out[3].at(0, i);
        }
        r.mix = out[0].at(0, kB - 1);
        r.inv = out[1].at(0, kB - 1);
    }
    return r;
}

Comparador make(float pa, float pb, float ofs, float lim, float hyst) {
    Comparador m;
    m.setParameter("pol_a", pa); m.setParameter("pol_b", pb);
    m.setParameter("offset", ofs); m.setParameter("threshold", lim);
    m.setParameter("hysteresis", hyst);
    m.prepare(kSr, kB);
    return m;
}

void testPolarizingMix() {
    auto sum = make(1, 1, 0, 0, 0);
    const auto s = run(sum, 10, [](long) { return 0.3f; }, [](long) { return 0.2f; });
    check(std::fabs(s.mix - 0.5f) < 1e-4f && std::fabs(s.inv + 0.5f) < 1e-4f, "somar: A + B; INV = −MIX");
    auto opp = make(1, -1, 0, 0, 0);
    const auto o = run(opp, 10, [](long) { return 0.3f; }, [](long) { return 0.2f; });
    check(std::fabs(o.mix - 0.1f) < 1e-4f, "opor: A − B");
    auto nul = make(1, -1, 0, 0, 0);
    const auto z = run(nul, 10, [](long n) { return float(std::sin(n * 0.01)); },
                       [](long n) { return float(std::sin(n * 0.01)); });
    check(std::fabs(z.mix) < 1e-5f, "anular: o mesmo sinal oposto dá zero");
    auto off = make(0, 0, 0.4f, 0, 0);
    check(std::fabs(run(off, 10, [](long) { return 1.0f; }, [](long) { return 1.0f; }).mix - 0.4f) < 1e-4f,
          "OFS no zero: só o deslocamento");
}

void testComparatorHysteresis() {
    // um seno lento com ruído: sem histerese, a borda tremula; com, não
    auto noisy = [](long n) {
        return float(0.8 * std::sin(2.0 * 3.14159265 * 2.0 * n / kSr)
                     + 0.05 * std::sin(n * 1.7) + 0.04 * std::sin(n * 2.9));
    };
    auto zero = [](long) { return 0.0f; };
    auto plain = make(1, 0, 0, 0, 0);
    const auto p = run(plain, int(kSr / kB), noisy, zero);   // 1 s = 2 ciclos
    auto hyst = make(1, 0, 0, 0, 0.3f);
    const auto h = run(hyst, int(kSr / kB), noisy, zero);
    std::printf("bordas de subida em 1 s: sem histerese %d, com %d\n", p.rises, h.rises);
    check(h.rises == 2, "HIST: uma subida por ciclo, sem tremular");
    check(p.rises > h.rises, "sem histerese, o ruído faz o limiar tremular");
    check(h.crosses == 4, "CRUZ: um pulso em cada borda (2 ciclos = 4)");
}

}  // namespace

int main() {
    testPolarizingMix();
    testComparatorHysteresis();
    if (g_failures == 0) std::printf("test_comparador: ok\n");
    return g_failures == 0 ? 0 : 1;
}
