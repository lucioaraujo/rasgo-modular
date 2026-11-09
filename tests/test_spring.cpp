// Teste isolado do Módulo 68 (SPRING — reverb de mola).
// Critérios do dossiê `dossies/68_spring.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Spring.hpp"

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
struct Run { std::vector<float> out, wet; };

Run run(Spring& m, int blocks, const Gen& in = {}, const Gen& kick = {}, const Gen& tcv = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bk(kSr, 1, kB), bt(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = in ? in(n + k) : 0.0f;
            bk.at(0, k) = kick ? kick(n + k) : 0.0f;
            bt.at(0, k) = tcv ? tcv(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{in ? &bi : nullptr, kick ? &bk : nullptr, tcv ? &bt : nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) { r.out.push_back(out[0].at(0, k)); r.wet.push_back(out[1].at(0, k)); }
        n += kB;
    }
    return r;
}
double energy(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double e = 0.0;
    for (std::size_t i = a; i < std::min(b, v.size()); ++i) e += static_cast<double>(v[i]) * v[i];
    return e;
}
Gen impulse() { return [](std::size_t n) { return n == 0 ? 1.0f : 0.0f; }; }

void testTailAndDecay() {
    Spring m;   // DECAY 0,85: cauda que decai, finita, sem explodir
    const auto r = run(m, static_cast<int>(3 * kSr / kB), impulse());
    const double e1 = energy(r.wet, 0, static_cast<std::size_t>(0.3 * kSr));
    const double e2 = energy(r.wet, static_cast<std::size_t>(1.0 * kSr), static_cast<std::size_t>(1.3 * kSr));
    const double e3 = energy(r.wet, static_cast<std::size_t>(2.5 * kSr), static_cast<std::size_t>(3.0 * kSr));
    std::cout << "energia da cauda: " << e1 << " → " << e2 << " → " << e3 << "\n";
    EXPECT(e1 > 1e-4 && e2 < e1 && e3 < e2);
    // DECAY maior → cauda mais longa
    Spring l;
    l.setParameter("decay", 0.96f);
    const auto rl = run(l, static_cast<int>(3 * kSr / kB), impulse());
    EXPECT(energy(rl.wet, static_cast<std::size_t>(1.0 * kSr), static_cast<std::size_t>(1.3 * kSr)) > e2);
}

void testEchoAtLoopLength() {
    // TENS 0 → laço de 25 ms: o primeiro eco da mola 1 depois de ~25 ms (mais
    // o atraso dos all-pass). Nada de wet antes de 20 ms.
    Spring m;
    m.setParameter("tension", 0.0f);
    const auto r = run(m, 40, impulse());
    double early = 0.0;
    for (std::size_t i = 0; i < static_cast<std::size_t>(0.020 * kSr); ++i) early += std::fabs(r.wet[i]);
    EXPECT(early < 1e-6);
    EXPECT(energy(r.wet, static_cast<std::size_t>(0.020 * kSr), static_cast<std::size_t>(0.08 * kSr)) > 1e-5);
}

void testMixAndKick() {
    // MIX 0 → OUT = IN exato.
    Spring dry;
    dry.setParameter("mix", 0.0f);
    const auto r = run(dry, 20, [](std::size_t n) { return 0.3f * std::sin(0.01f * static_cast<float>(n)); });
    bool same = true;
    for (std::size_t i = 0; i < r.out.size(); ++i)
        same = same && std::fabs(r.out[i] - 0.3f * std::sin(0.01f * static_cast<float>(i))) < 1e-6f;
    EXPECT(same);
    // KICK sem entrada: o tanque soa sozinho.
    Spring k;
    const auto rk = run(k, 200, {}, [](std::size_t n) { return n >= 1000 && n < 1100 ? 1.0f : 0.0f; });
    EXPECT(energy(rk.wet, 1000, 40000) > 1e-3);
    Spring q;
    const auto rq = run(q, 200, {}, {});
    EXPECT(energy(rq.wet, 0, rq.wet.size()) == 0.0);
}

void testTensionCvAndRobustness() {
    Spring m;
    const auto r = run(m, 400, [](std::size_t n) { return (n % 4800) == 0 ? 1.0f : 0.0f; },
                       {}, [](std::size_t n) { return std::sin(static_cast<float>(n) * 0.0005f); });
    bool ok = true;
    for (float v : r.out) ok = ok && std::isfinite(v) && std::fabs(v) <= 4.0f;
    EXPECT(ok);
    Spring z;
    z.setParameter("decay", 5.0f); z.setParameter("damp", -3.0f); z.setParameter("drip", 9.0f);
    const auto rz = run(z, 400, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 50 == 0 ? 1.0f : 0.0f); },
                        [](std::size_t n) { return n % 77 == 0 ? INFINITY : 0.0f; });
    ok = true;
    for (float v : rz.out) ok = ok && std::isfinite(v) && std::fabs(v) <= 4.0f;
    EXPECT(ok);
}

}  // namespace

int main() {
    testTailAndDecay();
    testEchoAtLoopLength();
    testMixAndKick();
    testTensionCvAndRobustness();
    if (g_failures == 0) std::cout << "SPRING: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
