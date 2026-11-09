// Teste isolado do Módulo 69 (MOUSSE — deformação de um material mole).
// Critérios do dossiê `dossies/69_mousse.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Mousse.hpp"

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
struct Run { std::vector<float> depth, speed, rest; };

Run run(Mousse& m, int blocks, const Gen& hit = {}, const Gen& press = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bh(kSr, 1, kB), bp(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) { bh.at(0, k) = hit ? hit(n + k) : 0.0f; bp.at(0, k) = press ? press(n + k) : 0.0f; }
        std::vector<const AudioBlock*> ins{hit ? &bh : nullptr, press ? &bp : nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) { r.depth.push_back(out[0].at(0, k)); r.speed.push_back(out[1].at(0, k)); r.rest.push_back(out[2].at(0, k)); }
        n += kB;
    }
    return r;
}
std::size_t at(double s) { return static_cast<std::size_t>(s * kSr); }
Gen never() { return [](std::size_t) { return 0.0f; }; }

void testPressSinksFastAndReturnsSlowly() {
    // Pressão de 1 s: afunda; solta: volta mais devagar do que afundou.
    Mousse m;
    m.setParameter("slow", 0.8f); m.setParameter("visc", 0.5f); m.setParameter("recover", 4.0f);
    const auto r = run(m, static_cast<int>(6 * kSr / kB), never(),
                       [](std::size_t n) { return n < static_cast<std::size_t>(kSr) ? 0.6f : 0.0f; });
    const float peak = r.depth[at(1.0)];
    EXPECT(peak > 0.3f);
    std::size_t down = 0, up = 0;
    while (down < at(1.0) && r.depth[down] < 0.5f * peak) ++down;
    while (at(1.0) + up < r.depth.size() && r.depth[at(1.0) + up] > 0.5f * peak) ++up;
    std::cout << "meia descida " << down / kSr << " s, meia volta " << up / kSr << " s\n";
    EXPECT(up > 2 * down);
    EXPECT(r.rest[at(0.0) + 10] > 0.5f && r.rest[at(0.5)] < 0.5f);
}

void testStiffnessAndFatigue() {
    // Mais duro → afunda menos com a mesma pressão.
    Mousse soft, hard;
    soft.setParameter("stiff", 0.0f); hard.setParameter("stiff", 1.0f);
    const Gen p = [](std::size_t) { return 0.3f; };
    EXPECT(run(soft, 400, never(), p).depth.back() > run(hard, 400, never(), p).depth.back() + 0.1f);
    // Fadiga: golpes seguidos afundam mais que o primeiro.
    Mousse f;
    f.setParameter("fatigue", 1.0f); f.setParameter("slow", 0.0f); f.setParameter("force", 0.15f);
    const auto r = run(f, static_cast<int>(3 * kSr / kB), [](std::size_t n) { return (n % 12000) < 100 ? 1.0f : 0.0f; });
    float first = 0.0f, later = 0.0f;
    for (std::size_t i = 0; i < 6000; ++i) first = std::max(first, r.depth[i]);
    for (std::size_t i = 4 * 12000; i < 4 * 12000 + 6000; ++i) later = std::max(later, r.depth[i]);
    EXPECT(later > first + 0.05f);
    EXPECT(f.softness() > 0.0f);
}

void testAutonomousAndRobust() {
    Mousse a;   // nada ligado: tocada sozinha em RATE, determinística
    const auto r = run(a, static_cast<int>(6 * kSr / kB));
    float peak = 0.0f;
    for (float d : r.depth) peak = std::max(peak, d);
    EXPECT(peak > 0.1f);
    Mousse b;
    EXPECT(run(b, static_cast<int>(6 * kSr / kB)).depth == r.depth);
    Mousse z;
    z.setParameter("visc", -9.0f); z.setParameter("stiff", 9.0f); z.setParameter("rate", 1e9f);
    const auto rz = run(z, 200, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : 1.0f; },
                        [](std::size_t n) { return n % 77 == 0 ? INFINITY : 5.0f; });
    bool ok = true;
    for (std::size_t i = 0; i < rz.depth.size(); ++i) ok = ok && std::isfinite(rz.depth[i]) && rz.depth[i] >= 0.0f && rz.depth[i] <= 1.0f && std::isfinite(rz.speed[i]);
    EXPECT(ok);
}
}  // namespace

int main() {
    testPressSinksFastAndReturnsSlowly();
    testStiffnessAndFatigue();
    testAutonomousAndRobust();
    if (g_failures == 0) std::cout << "MOUSSE: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
