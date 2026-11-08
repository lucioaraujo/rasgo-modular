// Teste isolado do Módulo 64 (ARP — arpejador).
// Critérios do dossiê `dossies/64_arp.md`.

#include "core/SignalGraph.hpp"
#include "dsp/Arp.hpp"

#include <array>
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
struct Run { std::vector<float> pitch, gate, eos; };

Run run(Arp& a, int blocks, const Gen& clock = {}, const Gen& reset = {}, const Gen& root = {}, const Gen& chord = {}) {
    a.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bc(kSr, 1, kB), br(kSr, 1, kB), bo(kSr, 1, kB), bh(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bc.at(0, k) = clock ? clock(n + k) : 0.0f;
            br.at(0, k) = reset ? reset(n + k) : 0.0f;
            bo.at(0, k) = root ? root(n + k) : 0.0f;
            bh.at(0, k) = chord ? chord(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{clock ? &bc : nullptr, reset ? &br : nullptr,
                                           root ? &bo : nullptr, chord ? &bh : nullptr};
        a.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.pitch.push_back(out[0].at(0, k));
            r.gate.push_back(out[1].at(0, k));
            r.eos.push_back(out[2].at(0, k));
        }
        n += kB;
    }
    return r;
}

// As notas (em semitons, relativas à tônica) no início de cada gate.
std::vector<int> notesAtGates(const Run& r) {
    std::vector<int> v;
    for (std::size_t i = 0; i < r.gate.size(); ++i)
        if (r.gate[i] > 0.5f && (i == 0 || r.gate[i - 1] < 0.5f))
            v.push_back(static_cast<int>(std::lround(r.pitch[i] * 12.0f)));
    return v;
}
// A nota em cada passo do clock (com ou sem gate).
std::vector<int> notesAtSteps(const Run& r, std::size_t period, std::size_t count) {
    std::vector<int> v;
    for (std::size_t s = 0; s < count; ++s) v.push_back(static_cast<int>(std::lround(r.pitch[s * period + 10] * 12.0f)));
    return v;
}
Gen pulses(std::size_t period) { return [=](std::size_t n) { return (n % period) < 100 ? 1.0f : 0.0f; }; }

void testNotes() {
    std::array<int, 16> v{};
    EXPECT(Arp::notes(3, 0, 1, v) == 4 && v[0] == 0 && v[1] == 4 && v[2] == 7 && v[3] == 12);  // maior
    EXPECT(Arp::notes(0, 0, 3, v) == 3 && v[0] == 0 && v[1] == 12 && v[2] == 24);              // uníssono sem repetir
    EXPECT(Arp::notes(7, 1, 1, v) == 4 && v[0] == 3 && v[1] == 7 && v[2] == 10 && v[3] == 12); // min7, 1ª inversão
    EXPECT(Arp::notes(9, 0, 4, v) == 16 && v[15] == 14 + 36);                                  // add9 em 4 oitavas
}

void testModes() {
    const std::size_t period = 4800;
    auto steps = [&](int mode, std::size_t count) {
        Arp a;
        a.setParameter("chord", 3.0f); a.setParameter("octaves", 1.0f); a.setParameter("mode", static_cast<float>(mode));
        const auto r = run(a, static_cast<int>(period * count / kB) + 2, pulses(period));
        return notesAtSteps(r, period, count);
    };
    EXPECT((steps(0, 8) == std::vector<int>{0, 4, 7, 12, 0, 4, 7, 12}));       // sobe
    EXPECT((steps(1, 4) == std::vector<int>{12, 7, 4, 0}));                     // desce
    EXPECT((steps(2, 7) == std::vector<int>{0, 4, 7, 12, 7, 4, 0}));            // ida e volta
    EXPECT((steps(3, 8) == std::vector<int>{0, 4, 7, 12, 12, 7, 4, 0}));        // repetindo as pontas
    EXPECT((steps(6, 4) == std::vector<int>{4, 7, 0, 12}));                     // divergir: do centro para fora
    EXPECT((steps(7, 4) == std::vector<int>{12, 0, 7, 4}));                     // convergir
    // Embaralhado: cada ciclo é uma permutação das 4 notas.
    const auto sh = steps(5, 8);
    for (int c = 0; c < 2; ++c) {
        std::array<int, 4> seen{};
        for (int k = 0; k < 4; ++k) {
            const int x = sh[static_cast<std::size_t>(c * 4 + k)];
            seen[x == 0 ? 0 : x == 4 ? 1 : x == 7 ? 2 : 3] += 1;
        }
        EXPECT(seen[0] == 1 && seen[1] == 1 && seen[2] == 1 && seen[3] == 1);
    }
    // Aleatório e embaralhado são semeados: duas rodadas iguais.
    EXPECT(steps(4, 16) == steps(4, 16));
    EXPECT(steps(5, 16) == steps(5, 16));
}

void testAutonomousAndClock() {
    // Nada ligado: soa sozinho no relógio interno (RATE 6 Hz → 6 notas/s).
    Arp a;
    const auto r = run(a, static_cast<int>(48000 / kB) + 1);
    const auto g = notesAtGates(r);
    std::cout << "autônomo: " << g.size() << " notas em 1 s\n";
    EXPECT(g.size() >= 6 && g.size() <= 7);
    // Clock externo: uma nota por pulso; o gate dura ~GATE do período.
    Arp b;
    b.setParameter("gate_len", 0.25f);
    const auto rb = run(b, 200, pulses(9600));
    std::size_t high = 0;
    for (std::size_t i = 9600 * 2; i < 9600 * 3; ++i) high += rb.gate[i] > 0.5f;
    EXPECT(std::abs(static_cast<int>(high) - 2400) < 300);
    // EOS no início de cada ciclo (maior em 2 oitavas = 8 notas): 2000 blocos
    // = 512000 amostras = 53 passos de 9600 → ciclos nos passos 0, 8, …, 48.
    Arp e;
    const auto re = run(e, 2000, pulses(9600));
    std::size_t eos = (re.eos[0] > 0.5f) ? 1 : 0;
    for (std::size_t i = 1; i < re.eos.size(); ++i) eos += re.eos[i] > 0.5f && re.eos[i - 1] < 0.5f;
    EXPECT(eos == 7);
}

void testRootChordResetSkip() {
    const std::size_t period = 4800;
    // ROOT do HARMONY (semitom/12): +7/12 = uma quinta acima.
    Arp a;
    a.setParameter("octaves", 1.0f);
    const auto r = run(a, 100, pulses(period), {}, [](std::size_t) { return 7.0f / 12.0f; });
    EXPECT((notesAtSteps(r, period, 4) == std::vector<int>{7, 11, 14, 19}));
    // CHORD por CV: +1/9 por formato — de maior (3) para menor (4).
    Arp m;
    m.setParameter("octaves", 1.0f);
    const auto rm = run(m, 100, pulses(period), {}, {}, [](std::size_t) { return 1.0f / 9.0f; });
    EXPECT((notesAtSteps(rm, period, 4) == std::vector<int>{0, 3, 7, 12}));
    // RESET volta ao começo.
    Arp z;
    z.setParameter("octaves", 1.0f);
    const auto rz = run(z, 100, pulses(period), [&](std::size_t n) { return (n >= period * 2 + 2000 && n < period * 2 + 2100) ? 1.0f : 0.0f; });
    EXPECT((notesAtSteps(rz, period, 4) == std::vector<int>{0, 4, 7, 0}));  // reset depois do 3º passo
    // SKIP: ~metade dos passos sem gate, semeado (igual em duas rodadas).
    auto skipped = [&]() {
        Arp s;
        s.setParameter("skip", 0.5f);
        const auto rs = run(s, 1000, pulses(period));
        return notesAtGates(rs).size();
    };
    const std::size_t steps = 1000 * kB / period;
    const std::size_t played = skipped();
    std::cout << "skip 0,5: " << played << " notas de " << steps << " passos\n";
    EXPECT(played > steps * 3 / 10 && played < steps * 7 / 10);
    EXPECT(skipped() == played);
}

void testRobustness() {
    Arp a;
    a.setParameter("mode", 99.0f); a.setParameter("chord", -5.0f); a.setParameter("octaves", 99.0f);
    const auto r = run(a, 50, [](std::size_t n) { return n % 333 == 0 ? std::nanf("") : (n % 1000 < 50 ? 1.0f : 0.0f); },
                       {}, [](std::size_t n) { return n % 77 == 0 ? INFINITY : 0.0f; });
    bool finite = true;
    for (std::size_t i = 0; i < r.pitch.size(); ++i) finite = finite && std::isfinite(r.pitch[i]) && std::isfinite(r.gate[i]);
    EXPECT(finite);
}

}  // namespace

int main() {
    testNotes();
    testModes();
    testAutonomousAndClock();
    testRootChordResetSkip();
    testRobustness();
    if (g_failures == 0) std::cout << "ARP: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
