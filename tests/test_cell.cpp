// Teste isolado do Módulo 65 (CELL — autômato celular como sequenciador).
// Critérios do dossiê `dossies/65_cell.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Cell.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 256;
using Gen = std::function<float(std::size_t)>;
struct Run { std::vector<float> pitch, gate, dens, eos; };

Run run(Cell& m, int blocks, const Gen& clock = {}, const Gen& reset = {}, const Gen& rule = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock bc(kSr, 1, kB), br(kSr, 1, kB), bu(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bc.at(0, k) = clock ? clock(n + k) : 0.0f;
            br.at(0, k) = reset ? reset(n + k) : 0.0f;
            bu.at(0, k) = rule ? rule(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{clock ? &bc : nullptr, reset ? &br : nullptr, rule ? &bu : nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.pitch.push_back(out[0].at(0, k)); r.gate.push_back(out[1].at(0, k));
            r.dens.push_back(out[2].at(0, k)); r.eos.push_back(out[3].at(0, k));
        }
        n += kB;
    }
    return r;
}
int edges(const std::vector<float>& g) {
    int n = g.empty() ? 0 : (g[0] > 0.5f);
    for (std::size_t i = 1; i < g.size(); ++i) n += g[i] > 0.5f && g[i - 1] < 0.5f;
    return n;
}

void testRules() {
    // Regra 90 (XOR dos vizinhos) a partir de uma célula: o triângulo de Sierpinski.
    std::uint32_t r = 1U << 8;
    r = Cell::step(r, 90, 16);
    EXPECT(r == ((1U << 7) | (1U << 9)));
    r = Cell::step(r, 90, 16);
    EXPECT(r == ((1U << 6) | (1U << 10)));
    // Regra 0 mata tudo; regra 255 enche tudo; a borda é circular.
    EXPECT(Cell::step(0xABCDU, 0, 16) == 0U);
    EXPECT(Cell::step(0U, 255, 16) == 0xFFFFU);
    EXPECT(Cell::step(1U, 90, 8) == ((1U << 1) | (1U << 7)));   // vizinha da 0 é a 7
}

void testAutonomousAndDeterministic() {
    Cell a;   // RATE 4 Hz, 2 s → ~8 gerações, alguns gates
    const auto r = run(a, static_cast<int>(2 * kSr / kB));
    const int g = edges(r.gate);
    std::cout << "autônomo: " << g << " gates em 2 s\n";
    EXPECT(g >= 1 && g <= 9);
    bool finite = true, inRange = true;
    for (std::size_t i = 0; i < r.pitch.size(); ++i) {
        finite = finite && std::isfinite(r.pitch[i]) && std::isfinite(r.dens[i]);
        inRange = inRange && r.dens[i] >= 0.0f && r.dens[i] <= 1.0f && r.pitch[i] >= 0.0f && r.pitch[i] <= 2.34f;
    }
    EXPECT(finite && inRange);
    Cell b;
    const auto r2 = run(b, static_cast<int>(2 * kSr / kB));
    EXPECT(r.gate == r2.gate && r.pitch == r2.pitch);
}

void testNeverDies() {
    // Regra 0 com MUT 0: a linha morreria a cada geração — renasce, com EOS.
    Cell m;
    m.setParameter("rule", 0.0f); m.setParameter("mutate", 0.0f); m.setParameter("rate", 20.0f);
    const auto r = run(m, static_cast<int>(2 * kSr / kB));
    EXPECT(edges(r.eos) >= 10);
    float maxD = 0.0f;
    for (float d : r.dens) maxD = std::max(maxD, d);
    EXPECT(maxD > 0.0f);
}

void testClockAndRuleCv() {
    // Clock externo: uma geração por pulso (100 pulsos → os gates não passam de 100).
    Cell m;
    const auto r = run(m, 400, [](std::size_t n) { return (n % 1000) < 50 ? 1.0f : 0.0f; });
    const int g = edges(r.gate);
    EXPECT(g > 5 && g <= 103);
    // RULE CV: 1 → +255 (regra 30 + 255 → 255 = tudo vivo → linha "parada" → renasce).
    Cell u;
    u.setParameter("mutate", 0.0f);
    const auto ru = run(u, 400, [](std::size_t n) { return (n % 1000) < 50 ? 1.0f : 0.0f; }, {},
                        [](std::size_t) { return 1.0f; });
    EXPECT(edges(ru.eos) >= 1);
}

void testRobustness() {
    Cell m;
    m.setParameter("cells", 99.0f); m.setParameter("rule", -3.0f); m.setParameter("head", 7.0f);
    const auto r = run(m, 100, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 900 < 40 ? 1.0f : 0.0f); },
                       {}, [](std::size_t n) { return n % 71 == 0 ? INFINITY : 0.2f; });
    bool finite = true;
    for (std::size_t i = 0; i < r.pitch.size(); ++i) finite = finite && std::isfinite(r.pitch[i]) && std::isfinite(r.gate[i]);
    EXPECT(finite);
}

}  // namespace

int main() {
    testRules();
    testAutonomousAndDeterministic();
    testNeverDies();
    testClockAndRuleCv();
    testRobustness();
    if (g_failures == 0) std::cout << "CELL: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
