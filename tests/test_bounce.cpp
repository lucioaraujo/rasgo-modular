// Teste isolado do Módulo 67 (BOUNCE — bolas que quicam).
// Critérios do dossiê `dossies/67_bounce.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Bounce.hpp"

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
struct Run { std::vector<float> gate, acc, h1, h2, h3; };

Run run(Bounce& m, int blocks, const Gen& drop = {}, const Gen& grav = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock bd(kSr, 1, kB), bg(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bd.at(0, k) = drop ? drop(n + k) : 0.0f;
            bg.at(0, k) = grav ? grav(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{drop ? &bd : nullptr, grav ? &bg : nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.gate.push_back(out[0].at(0, k)); r.acc.push_back(out[1].at(0, k));
            r.h1.push_back(out[2].at(0, k)); r.h2.push_back(out[3].at(0, k)); r.h3.push_back(out[4].at(0, k));
        }
        n += kB;
    }
    return r;
}
std::vector<std::size_t> onsets(const std::vector<float>& g) {
    std::vector<std::size_t> v;
    for (std::size_t i = 1; i < g.size(); ++i) if (g[i] > 0.5f && g[i - 1] < 0.5f) v.push_back(i);
    return v;
}

void testPhysicsOneBall() {
    // Uma bola, sem vento, sem relançar: a 1ª queda leva sqrt(2h/g); cada
    // intervalo entre quiques encolhe pelo fator ELAST.
    Bounce m;
    m.setParameter("balls", 1.0f); m.setParameter("wind", 0.0f); m.setParameter("auto", 0.0f);
    m.setParameter("gravity", 9.8f); m.setParameter("elastic", 0.7f);
    const auto r = run(m, static_cast<int>(4 * kSr / kB));
    const auto on = onsets(r.gate);
    EXPECT(on.size() >= 6);
    if (on.size() >= 4) {
        const double t0 = static_cast<double>(on[0]) / kSr;
        std::cout << "1ª queda: " << t0 << " s (teoria " << std::sqrt(2.0 / 9.8) << ")\n";
        EXPECT(std::fabs(t0 - std::sqrt(2.0 / 9.8)) < 0.01);
        const double i1 = static_cast<double>(on[2] - on[1]), i0 = static_cast<double>(on[1] - on[0]);
        EXPECT(std::fabs(i1 / i0 - 0.7) < 0.03);
    }
    // o acento cai a cada quique
    if (on.size() >= 3) EXPECT(r.acc[on[2] + 5] < r.acc[on[1] + 5] && r.acc[on[1] + 5] < r.acc[on[0] + 5]);
    // a bola para (sem AUTO): no fim, sem gates e altura 0
    EXPECT(r.h1.back() == 0.0f);
}

void testAutoRelaunchAndDeterminism() {
    Bounce m;   // padrão: 3 bolas, AUTO ligado — toca para sempre
    m.setParameter("elastic", 0.5f);
    const auto r = run(m, static_cast<int>(20 * kSr / kB));
    const auto on = onsets(r.gate);
    std::size_t late = 0;
    for (auto i : on) late += i > static_cast<std::size_t>(15 * kSr);
    std::cout << "20 s com AUTO: " << on.size() << " quiques, " << late << " nos últimos 5 s\n";
    EXPECT(late >= 3);
    Bounce b;
    b.setParameter("elastic", 0.5f);
    EXPECT(onsets(run(b, static_cast<int>(20 * kSr / kB)).gate) == on);
}

void testDropAndGravityCv() {
    // DROP relança; GRAV +0,5 → gravidade ×2 → queda √2 mais rápida.
    Bounce slow, fast;
    for (Bounce* m : {&slow, &fast}) { m->setParameter("balls", 1.0f); m->setParameter("wind", 0.0f); }
    const auto rs = run(slow, 200);
    const auto rf = run(fast, 200, {}, [](std::size_t) { return 0.5f; });
    const auto os = onsets(rs.gate), of = onsets(rf.gate);
    EXPECT(!os.empty() && !of.empty());
    if (!os.empty() && !of.empty())
        EXPECT(std::fabs(static_cast<double>(os[0]) / static_cast<double>(of[0]) - std::sqrt(2.0)) < 0.02);
    Bounce d;
    d.setParameter("balls", 1.0f); d.setParameter("auto", 0.0f); d.setParameter("elastic", 0.3f);
    const auto rd = run(d, static_cast<int>(6 * kSr / kB), [](std::size_t n) {
        return (n >= static_cast<std::size_t>(4 * kSr) && n < static_cast<std::size_t>(4 * kSr) + 100) ? 1.0f : 0.0f; });
    std::size_t after = 0;
    for (auto i : onsets(rd.gate)) after += i > static_cast<std::size_t>(4 * kSr);
    EXPECT(after >= 1);
}

void testRobustness() {
    Bounce m;
    m.setParameter("balls", 9.0f); m.setParameter("gravity", -5.0f); m.setParameter("elastic", 3.0f);
    const auto r = run(m, 200, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : 0.0f; },
                       [](std::size_t n) { return n % 77 == 0 ? INFINITY : -9.0f; });
    bool ok = true;
    for (std::size_t i = 0; i < r.h1.size(); ++i)
        ok = ok && std::isfinite(r.h1[i]) && std::isfinite(r.acc[i]) && r.h1[i] >= 0.0f && r.h1[i] <= 1.0f;
    EXPECT(ok);
}

}  // namespace

int main() {
    testPhysicsOneBall();
    testAutoRelaunchAndDeterminism();
    testDropAndGravityCv();
    testRobustness();
    if (g_failures == 0) std::cout << "BOUNCE: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
