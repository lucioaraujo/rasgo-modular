// Teste do SUM (src/dsp/Sum.hpp): soma com atenuversores, offset, INV, AVG
// (média das ligadas), SAT suave, teto ±8, entradas não finitas.

#include "dsp/Sum.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {
int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
bool near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

AudioBlock constBlock(float v) {
    AudioBlock b(48000.0f, 2, 64);
    for (std::size_t i = 0; i < 64; ++i) { b.at(0, i) = v; b.at(1, i) = v; }
    return b;
}

void testSumInvAvg() {
    Summer s; s.prepare(48000.0f, 64);
    s.setParameter("amt2", -0.5f);
    s.setParameter("offset", 0.1f);
    AudioBlock a = constBlock(0.4f), b = constBlock(0.2f);
    std::vector<AudioBlock> out(3, AudioBlock(48000.0f, 2, 64));
    s.process({&a, &b, nullptr, nullptr}, out);
    // 0,4·1 + 0,2·(−0,5) + 0,1 = 0,4
    check(near(out[0].at(0, 10), 0.4f), "SUM = Σ amt·in + offset");
    check(near(out[1].at(0, 10), -0.4f), "INV = −SUM");
    // (0,4 − 0,1)/2 + 0,1 = 0,25
    check(near(out[2].at(0, 10), 0.25f), "AVG = média das ligadas + offset");
}

void testSoftAndCeiling() {
    Summer s; s.prepare(48000.0f, 64);
    AudioBlock big = constBlock(5.0f), nan = constBlock(std::nanf(""));
    std::vector<AudioBlock> out(3, AudioBlock(48000.0f, 2, 64));
    s.process({&big, &big, &nan, nullptr}, out);
    check(near(out[0].at(0, 5), 8.0f), "teto duro ±8");
    s.setParameter("soft", 1.0f);
    s.process({&big, &big, &nan, nullptr}, out);
    check(out[0].at(0, 5) <= 1.0f && out[0].at(0, 5) > 0.99f, "SAT: saturação suave até ±1");
    check(std::isfinite(out[2].at(0, 5)), "entrada não finita vira 0");
}

void testSilentWithoutInputs() {
    Summer s; s.prepare(48000.0f, 64);
    std::vector<AudioBlock> out(3, AudioBlock(48000.0f, 2, 64));
    s.process({nullptr, nullptr, nullptr, nullptr}, out);
    check(out[0].at(0, 3) == 0.0f && out[2].at(0, 3) == 0.0f, "sem entrada: zero");
    check(s.type() == "SUM", "tipo");
}
}  // namespace

int main() {
    testSumInvAvg();
    testSoftAndCeiling();
    testSilentWithoutInputs();
    if (g_failures == 0) { std::cout << "RASGO Modular SUM tests passed\n"; return 0; }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
