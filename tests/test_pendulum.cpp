// Teste isolado do Módulo 70 (PENDULUM — pêndulo duplo).
// Critérios do dossiê `dossies/70_pendulum.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Pendulum.hpp"

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
struct Run { std::vector<float> x1, y1, x2, y2, cross, flip; };

Run run(Pendulum& m, int blocks, const Gen& kick = {}, const Gen& speed = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock bk(kSr, 1, kB), bs(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) { bk.at(0, k) = kick ? kick(n + k) : 0.0f; bs.at(0, k) = speed ? speed(n + k) : 0.0f; }
        std::vector<const AudioBlock*> ins{kick ? &bk : nullptr, speed ? &bs : nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.x1.push_back(out[0].at(0, k)); r.y1.push_back(out[1].at(0, k)); r.x2.push_back(out[2].at(0, k));
            r.y2.push_back(out[3].at(0, k)); r.cross.push_back(out[4].at(0, k)); r.flip.push_back(out[5].at(0, k));
        }
        n += kB;
    }
    return r;
}
int edges(const std::vector<float>& g) {
    int n = 0;
    for (std::size_t i = 1; i < g.size(); ++i) n += g[i] > 0.5f && g[i - 1] < 0.5f;
    return n;
}

void testEnergyConservedWithoutDamping() {
    Pendulum m;
    m.setParameter("damp", 0.0f); m.setParameter("auto", 0.0f); m.setParameter("energy", 0.5f);
    m.prepare(kSr, kB);
    const double e0 = m.energy();
    run(m, static_cast<int>(10 * kSr / kB));   // run() chama prepare de novo: mesma partida
    const double e1 = m.energy();
    std::cout << "energia: " << e0 << " → " << e1 << " em 10 s (RK4, sem atrito)\n";
    EXPECT(std::fabs(e1 - e0) < 1e-3 * std::max(1.0, e0));
}

void testSmallSwingIsRegularBigSwingFlips() {
    // Pouca energia: oscila (CROSS regular), nunca dá a volta.
    Pendulum calm;
    calm.setParameter("energy", 0.0f); calm.setParameter("damp", 0.0f); calm.setParameter("auto", 0.0f);
    const auto rc = run(calm, static_cast<int>(10 * kSr / kB));
    EXPECT(edges(rc.flip) == 0);
    EXPECT(edges(rc.cross) >= 6);
    // Muita energia: o braço de baixo dá voltas (FLIP).
    Pendulum wild;
    wild.setParameter("energy", 1.0f); wild.setParameter("damp", 0.0f);
    const auto rw = run(wild, static_cast<int>(10 * kSr / kB));
    std::cout << "FLIP em 10 s com ENERGY 1: " << edges(rw.flip) << "\n";
    EXPECT(edges(rw.flip) >= 1);
}

void testAutoKeepsMovingAndDeterministic() {
    Pendulum m;
    m.setParameter("damp", 1.0f);   // muito atrito: sem AUTO pararia
    const auto r = run(m, static_cast<int>(30 * kSr / kB));
    float late = 0.0f;
    for (std::size_t i = static_cast<std::size_t>(25 * kSr); i < r.x2.size(); ++i) late = std::max(late, std::fabs(r.x2[i]));
    EXPECT(late > 0.1f);
    Pendulum b;
    b.setParameter("damp", 1.0f);
    EXPECT(run(b, static_cast<int>(30 * kSr / kB)).x2 == r.x2);
}

void testRangeAndRobustness() {
    Pendulum m;
    m.setParameter("ratio", 9.0f); m.setParameter("mass", -1.0f); m.setParameter("speed", 99.0f);
    const auto r = run(m, 400, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 5000 < 50 ? 1.0f : 0.0f); },
                       [](std::size_t n) { return n % 77 == 0 ? INFINITY : 3.0f; });
    bool ok = true;
    for (std::size_t i = 0; i < r.x2.size(); ++i)
        ok = ok && std::isfinite(r.x2[i]) && std::fabs(r.x1[i]) <= 1.0001f && std::fabs(r.x2[i]) <= 1.0001f && std::fabs(r.y2[i]) <= 1.0001f;
    EXPECT(ok);
}
}  // namespace

int main() {
    testEnergyConservedWithoutDamping();
    testSmallSwingIsRegularBigSwingFlips();
    testAutoKeepsMovingAndDeterministic();
    testRangeAndRobustness();
    if (g_failures == 0) std::cout << "PENDULUM: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
