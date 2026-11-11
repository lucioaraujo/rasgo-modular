// FRONTEIRA (#79) — onde a célula melódica começa. Critérios de
// `dossies/79_fronteira.md`.

#include "dsp/Fronteira.hpp"

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
constexpr std::size_t kB = 240;   // 5 ms

struct N { int semi; double dur; double gap; };   // duração soando, pausa depois

struct Res { std::vector<int> boundaryAtNote; int ends = 0; };

Res play(Fronteira& m, const std::vector<N>& ns, double tail = 0.0) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock pitch(kSr, 1, kB), gate(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&pitch, &gate};
    Res r;
    float pb = 0, pe = 0;
    int idx = -1;
    auto runFor = [&](double secs, float semi, float g) {
        const int blocks = int(secs * kSr / kB + 0.5);
        for (int b = 0; b < blocks; ++b) {
            for (std::size_t i = 0; i < kB; ++i) { pitch.at(0, i) = semi / 12.0f; gate.at(0, i) = g; }
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) {
                if (pb < 0.5f && out[0].at(0, i) >= 0.5f) r.boundaryAtNote.push_back(idx);
                if (pe < 0.5f && out[3].at(0, i) >= 0.5f) ++r.ends;
                pb = out[0].at(0, i); pe = out[3].at(0, i);
            }
        }
    };
    for (const auto& n : ns) {
        ++idx;
        runFor(n.dur, float(n.semi), 1.0f);
        if (n.gap > 0) runFor(n.gap, float(n.semi), 0.0f);
    }
    if (tail > 0) runFor(tail, 0.0f, 0.0f);
    return r;
}

void testRhythmicGroups() {
    // grupos de 4 colcheias em grau conjunto + uma nota longa e uma pausa:
    // a fronteira cai na 1ª nota de cada grupo (detectada no ataque da 2ª)
    Fronteira m;
    m.prepare(kSr, kB);
    std::vector<N> ns;
    for (int g = 0; g < 5; ++g) {
        ns.push_back({0, 0.18, 0.02}); ns.push_back({2, 0.18, 0.02});
        ns.push_back({4, 0.18, 0.02}); ns.push_back({5, 0.6, 0.3});
    }
    const auto r = play(m, ns);
    int ok = 0;
    for (const int at : r.boundaryAtNote) if (at % 4 == 1) ++ok;   // detectada no ataque da 2ª do grupo
    std::printf("FRONT em notas:");
    for (int at : r.boundaryAtNote) std::printf(" %d", at);
    std::printf("\n");
    check(ok >= 3, "FRONT: a célula começa depois da nota longa e da pausa");
    check(int(r.boundaryAtNote.size()) <= ok + 1, "FRONT: sem fronteiras no meio dos grupos");
}

void testSilenceCloses() {
    Fronteira m;
    m.prepare(kSr, kB);
    const auto r = play(m, {{0, 0.2, 0.02}, {2, 0.2, 0.02}, {4, 0.2, 0.02}}, 2.5);
    check(r.ends == 1, "FIM: o silêncio longo fecha a frase na hora");
}

void testSteadyLineNoBoundary() {
    Fronteira m;
    m.prepare(kSr, kB);
    std::vector<N> ns(24, N{0, 0.18, 0.02});
    for (std::size_t i = 0; i < ns.size(); ++i) ns[i].semi = int(i % 2) * 2;
    const auto r = play(m, ns);
    check(r.boundaryAtNote.size() <= 1, "linha sem mudança: quase nenhuma fronteira");
}

}  // namespace

int main() {
    testRhythmicGroups();
    testSilenceCloses();
    testSteadyLineNoBoundary();
    if (g_failures == 0) std::printf("test_fronteira: ok\n");
    return g_failures == 0 ? 0 : 1;
}
