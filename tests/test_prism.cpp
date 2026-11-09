// Teste isolado do Módulo 71 (PRISM — o som através de um prisma).
// Critérios do dossiê `dossies/71_prism.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Prism.hpp"

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
struct Run { std::vector<float> outL, outR, wetL; };

Run run(Prism& m, int blocks, const Gen& in, std::size_t ch = 1) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, ch, kB));
    AudioBlock bi(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) bi.at(0, k) = in(n + k);
        std::vector<const AudioBlock*> ins{&bi, nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.outL.push_back(out[0].at(0, k)); r.outR.push_back(out[0].at(ch - 1, k)); r.wetL.push_back(out[1].at(0, k));
        }
        n += kB;
    }
    return r;
}
// O instante (s) do pico do envelope de um seno em rajada que passou pelo prisma.
double arrival(float hz, float curve) {
    Prism p;
    p.setParameter("disp", 300.0f); p.setParameter("curve", curve); p.setParameter("feedback", 0.0f); p.setParameter("mix", 1.0f);
    const auto r = run(p, 200, [hz](std::size_t n) {
        return n < 2400 ? std::sin(6.2831853f * hz * static_cast<float>(n) / kSr) * 0.5f : 0.0f; });
    std::size_t best = 0; float peak = 0.0f;
    for (std::size_t i = 0; i < r.wetL.size(); ++i) if (std::fabs(r.wetL[i]) > peak) { peak = std::fabs(r.wetL[i]); best = i; }
    return static_cast<double>(best) / kSr;
}

void testDispersion() {
    EXPECT(Prism::shape(0, 1.0f) == 0.0f && Prism::shape(5, 1.0f) == 1.0f);
    EXPECT(Prism::shape(0, -1.0f) == 1.0f && Prism::shape(5, -1.0f) == 0.0f);
    EXPECT(Prism::shape(2, 0.0f) == 0.5f);
    // Normal: o agudo chega depois do grave; anômala: o contrário.
    const double lowN = arrival(120.0f, 1.0f), highN = arrival(4700.0f, 1.0f);
    const double lowA = arrival(120.0f, -1.0f), highA = arrival(4700.0f, -1.0f);
    std::cout << "normal: grave " << lowN << " s, agudo " << highN << " s; anômala: grave " << lowA << ", agudo " << highA << "\n";
    EXPECT(highN > lowN + 0.15);
    EXPECT(lowA > highA + 0.15);
}

void testFanAndMix() {
    // Estéreo com SPREAD 1: o grave sai mais à esquerda, o agudo mais à direita.
    auto energyLR = [](float hz) {
        Prism p;
        p.setParameter("spread", 1.0f); p.setParameter("mix", 1.0f); p.setParameter("feedback", 0.0f);
        const auto r = run(p, 200, [hz](std::size_t n) { return 0.3f * std::sin(6.2831853f * hz * static_cast<float>(n) / kSr); }, 2);
        double l = 0, rr = 0;
        for (std::size_t i = r.outL.size() / 2; i < r.outL.size(); ++i) { l += r.outL[i] * r.outL[i]; rr += r.outR[i] * r.outR[i]; }
        return std::pair<double, double>{l, rr};
    };
    const auto lo = energyLR(120.0f), hi = energyLR(11000.0f);
    EXPECT(lo.first > lo.second && hi.second > hi.first);
    // MIX 0 = a entrada exata.
    Prism d;
    d.setParameter("mix", 0.0f);
    const auto r = run(d, 20, [](std::size_t n) { return 0.2f * std::sin(0.02f * static_cast<float>(n)); });
    bool same = true;
    for (std::size_t i = 0; i < r.outL.size(); ++i) same = same && std::fabs(r.outL[i] - 0.2f * std::sin(0.02f * static_cast<float>(i))) < 1e-6f;
    EXPECT(same);
}

void testFeedbackStableAndRobust() {
    Prism p;
    p.setParameter("feedback", 0.85f); p.setParameter("disp", 10.0f);
    const auto r = run(p, static_cast<int>(10 * kSr / kB), [](std::size_t n) { return n < 100 ? 1.0f : 0.0f; });
    double tail = 0.0;
    for (std::size_t i = static_cast<std::size_t>(9 * kSr); i < r.wetL.size(); ++i) tail += std::fabs(r.wetL[i]);
    EXPECT(std::isfinite(tail) && tail < 1.0);
    Prism z;
    z.setParameter("disp", -5.0f); z.setParameter("feedback", 9.0f); z.setParameter("curve", 7.0f);
    const auto rz = run(z, 400, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 77 == 0 ? INFINITY : 0.5f); }, 2);
    bool ok = true;
    for (float v : rz.outL) ok = ok && std::isfinite(v) && std::fabs(v) <= 4.0f;
    EXPECT(ok);
}
}  // namespace

int main() {
    testDispersion();
    testFanAndMix();
    testFeedbackStableAndRobust();
    if (g_failures == 0) std::cout << "PRISM: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
