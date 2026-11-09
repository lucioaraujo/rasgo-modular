// Teste isolado do Módulo 66 (MARKOV — cadeia de Markov que aprende e improvisa).
// Critérios do dossiê `dossies/66_markov.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Markov.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <set>
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
struct Run { std::vector<float> pitch, gate, surp; };

Run run(Markov& m, int blocks, const Gen& clock = {}, const Gen& lp = {}, const Gen& lg = {}) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bc(kSr, 1, kB), bp(kSr, 1, kB), bg(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bc.at(0, k) = clock ? clock(n + k) : 0.0f;
            bp.at(0, k) = lp ? lp(n + k) : 0.0f;
            bg.at(0, k) = lg ? lg(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{clock ? &bc : nullptr, lp ? &bp : nullptr, lg ? &bg : nullptr, nullptr};
        m.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.pitch.push_back(out[0].at(0, k)); r.gate.push_back(out[1].at(0, k)); r.surp.push_back(out[2].at(0, k));
        }
        n += kB;
    }
    return r;
}
std::vector<int> notesAtGates(const Run& r) {
    std::vector<int> v;
    for (std::size_t i = 0; i < r.gate.size(); ++i)
        if (r.gate[i] > 0.5f && (i == 0 || r.gate[i - 1] < 0.5f))
            v.push_back(Markov::pitchClass(r.pitch[i]));
    return v;
}

void testAutonomousPentatonic() {
    Markov m;   // nada ligado: caminhada pentatônica no relógio interno (5 Hz)
    const auto r = run(m, static_cast<int>(4 * kSr / kB));
    const auto v = notesAtGates(r);
    std::cout << "autônomo: " << v.size() << " notas em 4 s\n";
    EXPECT(v.size() >= 18 && v.size() <= 22);
    std::set<int> seen(v.begin(), v.end());
    for (int pc : seen) EXPECT(pc == 0 || pc == 2 || pc == 4 || pc == 7 || pc == 9);
    EXPECT(seen.size() >= 3);
    Markov b;
    EXPECT(notesAtGates(run(b, static_cast<int>(4 * kSr / kB))) == v);   // determinístico
}

void testLearnsAndRepeats() {
    // Ensina C → E → G → C … (0, 4, 7) com MEM 1 e toca com TEMP 0 (sempre a
    // mais provável): a saída só pode andar nesse ciclo.
    Markov m;
    m.setParameter("memory", 1.0f);
    m.setParameter("temp", 0.0f);
    const int seq[3] = {0, 4, 7};
    const std::size_t period = 2400;   // uma nota ensinada a cada 50 ms
    const auto r = run(m, static_cast<int>(6 * kSr / kB), {},
        [&](std::size_t n) { return static_cast<float>(seq[(n / period) % 3]) / 12.0f; },
        [&](std::size_t n) { return (n % period) < 200 ? 1.0f : 0.0f; });
    EXPECT(m.probability(0, 4) > 0.5f && m.probability(4, 7) > 0.5f && m.probability(7, 0) > 0.5f);
    const auto v = notesAtGates(r);
    int bad = 0;
    for (std::size_t i = v.size() / 2 + 1; i < v.size(); ++i) {
        const int a = v[i - 1], b = v[i];
        const bool ok = (a == 0 && b == 4) || (a == 4 && b == 7) || (a == 7 && b == 0);
        bad += !ok;
    }
    EXPECT(bad == 0);
    // TEMP 0 → surpresa 0
    float maxS = 0.0f;
    for (std::size_t i = r.surp.size() / 2; i < r.surp.size(); ++i) maxS = std::max(maxS, r.surp[i]);
    EXPECT(maxS < 1e-6f);
}

void testTemperatureRaisesSurprise() {
    auto meanSurprise = [](float temp) {
        Markov m;
        m.setParameter("temp", temp);
        m.setParameter("rate", 20.0f);
        const auto r = run(m, static_cast<int>(6 * kSr / kB));
        double s = 0.0;
        for (float x : r.surp) s += x;
        return s / static_cast<double>(r.surp.size());
    };
    const double lo = meanSurprise(0.3f), hi = meanSurprise(2.0f);
    std::cout << "surpresa média: TEMP 0,3 → " << lo << ", TEMP 2 → " << hi << "\n";
    EXPECT(hi > lo + 0.1);
}

void testFreezeAndRobustness() {
    Markov m;
    m.setParameter("learn", 0.0f);   // LEARN desligado: ensina nada
    m.setParameter("memory", 1.0f);
    run(m, 200, {}, [](std::size_t n) { return (n / 2400) % 2 ? 1.0f / 12.0f : 0.0f; },
        [](std::size_t n) { return (n % 2400) < 100 ? 1.0f : 0.0f; });
    EXPECT(m.probability(0, 1) == 0.0f);
    Markov z;
    z.setParameter("temp", 99.0f); z.setParameter("octave", -40.0f);
    const auto r = run(z, 100, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 900 < 40 ? 1.0f : 0.0f); },
                       [](std::size_t n) { return n % 77 == 0 ? INFINITY : 0.3f; },
                       [](std::size_t n) { return (n % 500) < 50 ? 1.0f : 0.0f; });
    bool finite = true;
    for (std::size_t i = 0; i < r.pitch.size(); ++i) finite = finite && std::isfinite(r.pitch[i]) && std::isfinite(r.surp[i]);
    EXPECT(finite);
}

}  // namespace

int main() {
    testAutonomousPentatonic();
    testLearnsAndRepeats();
    testTemperatureRaisesSurprise();
    testFreezeAndRobustness();
    if (g_failures == 0) std::cout << "MARKOV: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
