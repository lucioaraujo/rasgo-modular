// SEASON (#100) — dia, noite e estação. Critérios de `dossies/100_season.md`.

#include "dsp/Season.hpp"

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

void testAstronomy() {
    const double eq = Season::daylight(0.25, 0.0), eqW = Season::daylight(0.75, 0.0);
    const double s60 = Season::daylight(0.25, 60.0), w60 = Season::daylight(0.75, 60.0);
    const double polarS = Season::daylight(0.25, 75.0), polarW = Season::daylight(0.75, 75.0);
    std::printf("dia (fração): equador verão %.2f inverno %.2f · 60° verão %.2f inverno %.2f · 75° %.2f / %.2f\n",
                eq, eqW, s60, w60, polarS, polarW);
    check(std::fabs(eq - 0.5) < 0.01 && std::fabs(eqW - 0.5) < 0.01, "no equador, 12 h o ano inteiro");
    check(s60 > 0.7 && w60 < 0.3, "a 60°, dias longos no verão e curtos no inverno (~18,5 h / ~5,5 h)");
    check(polarS == 1.0 && polarW == 0.0, "acima do círculo polar: sol da meia-noite e noite polar");
    check(std::fabs(Season::daylight(0.0, 60.0) - 0.5) < 0.02, "no equinócio, 12 h em qualquer latitude");
}

void testDaysAndSouth() {
    for (int south = 0; south < 2; ++south) {
        Season m;
        m.setParameter("day", 5.0f); m.setParameter("year", 16.0f);
        m.setParameter("latitude", 50.0f); m.setParameter("hemisphere", float(south));
        m.prepare(kSr, kB);
        std::vector<AudioBlock> out(7, AudioBlock(kSr, 1, kB));
        std::vector<const AudioBlock*> ins{nullptr, nullptr};
        int dawns = 0; float pd = 0; double litSummer = 0, nSummer = 0; bool fin = true;
        const int blocks = int(80.0 * kSr / kB);   // 16 dias de 5 s = 1 ano
        for (int b = 0; b < blocks; ++b) {
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) {
                for (int k = 0; k < 7; ++k) if (!std::isfinite(out[std::size_t(k)].at(0, i))) fin = false;
                if (pd < 0.5f && out[2].at(0, i) >= 0.5f) ++dawns;
                pd = out[2].at(0, i);
            }
            const float yr = out[4].at(0, kB - 1);
            if (yr > 0.1875f && yr < 0.3125f) { litSummer += out[1].at(0, kB - 1); ++nSummer; }   // 2 dias inteiros
        }
        std::printf("%s: %d nasceres em 16 dias; luz em junho %.2f\n", south ? "sul" : "norte", dawns, litSummer / nSummer);
        check(dawns >= 16 && dawns <= 17, "um nascer por dia");
        if (!south) check(litSummer / nSummer > 0.6, "norte: junho é verão, dia longo");
        else check(litSummer / nSummer < 0.4, "sul: junho é inverno, dia curto");
        check(fin, "finito");
    }
}

}  // namespace

int main() {
    testAstronomy();
    testDaysAndSouth();
    if (g_failures == 0) std::printf("test_season: ok\n");
    return g_failures == 0 ? 0 : 1;
}
