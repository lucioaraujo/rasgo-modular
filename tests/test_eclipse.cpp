// ECLIPSE (#99) — sinódico × dracônico. Critérios de `dossies/99_eclipse.md`.

#include "dsp/Eclipse.hpp"

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

// a sequência de luas novas com eclipse do Sol (1) ou sem (0), por pulsos de 1 dia
std::vector<int> lunations(int months) {
    Eclipse m;
    m.prepare(kSr, kB);
    AudioBlock clk(kSr, 1, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{&clk, nullptr};
    std::vector<int> seq;
    float pn = 0; bool sunInThis = false;
    const int days = int(months * Eclipse::kSynodic) + 2;
    for (int d = 0; d < days * 2; ++d) {   // dois blocos por dia: pulso e silêncio
        for (std::size_t i = 0; i < kB; ++i) clk.at(0, i) = (d % 2 == 0 && i < 8) ? 1.0f : 0.0f;
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            if (out[0].at(0, i) >= 0.5f) sunInThis = true;
            if (pn < 0.5f && out[2].at(0, i) >= 0.5f) { seq.push_back(sunInThis ? 1 : 0); }
            pn = out[2].at(0, i);
        }
        sunInThis = sunInThis && out[2].at(0, kB - 1) >= 0.5f;
    }
    return seq;
}

void testSaros() {
    const auto s = lunations(2 * 223 + 5);
    int first = 0, total = 0, match = 0;
    for (int i = 0; i < 223 && i < int(s.size()); ++i) first += s[std::size_t(i)];
    for (int i = 0; i + 223 < int(s.size()); ++i) {
        if (s[std::size_t(i)]) { ++total; if (s[std::size_t(i + 223)]) ++match; }
    }
    std::printf("eclipses do Sol num Saros (223 luas): %d; voltam 223 luas depois: %d de %d\n", first, match, total);
    check(first >= 25 && first <= 60, "algumas dezenas de eclipses do Sol por Saros");
    check(total > 0 && match >= int(0.9 * total), "o Saros: a sequência volta depois de 223 lunações");
}

void testSeasons() {
    const auto s = lunations(120);
    // eclipses vêm em temporadas: dois eclipses seguidos ficam a ~6 luas (ou 1 lua) um do outro
    int last = -100, okGap = 0, gaps = 0;
    for (int i = 0; i < int(s.size()); ++i)
        if (s[std::size_t(i)]) {
            if (last >= 0) { ++gaps; const int g = i - last; if (g == 1 || (g >= 5 && g <= 7)) ++okGap; }
            last = i;
        }
    std::printf("intervalos entre eclipses: %d de %d em 1 ou 5–7 lunações\n", okGap, gaps);
    check(gaps > 5 && okGap == gaps, "temporadas de eclipses a cada ~6 meses");
}

void testNodeDistance() {
    check(Eclipse::nodeDistance(0.0) < 1e-12 && Eclipse::nodeDistance(0.5) < 1e-12, "os dois nós");
    check(std::fabs(Eclipse::nodeDistance(0.25) - 0.25) < 1e-12, "a um quarto de volta, o mais longe dos nós");
}

}  // namespace

int main() {
    testNodeDistance();
    testSaros();
    testSeasons();
    if (g_failures == 0) std::printf("test_eclipse: ok\n");
    return g_failures == 0 ? 0 : 1;
}
