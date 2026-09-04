// Teste isolado do Módulo 14 (HARMONY) - antes de entrar num patch.
// Critérios do dossiê `dossies/14_harmony.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Harmony.hpp"
#include "dsp/Quantizer.hpp"
#include "dsp/TuringLoop.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

constexpr float kSampleRate = 48000.0f;

struct Steps {
    std::vector<int> root, scale;
    std::vector<float> change;
};

Steps stepN(Harmony& h, const int steps) {
    h.prepare(kSampleRate, 1);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 1));
    AudioBlock adv(kSampleRate, 1, 1);
    std::vector<const AudioBlock*> in{&adv, nullptr};
    Steps r;
    for (int i = 0; i < steps; ++i) {
        adv.at(0, 0) = 1.0f;
        h.process(in, out);
        r.root.push_back(static_cast<int>(std::lround(out[0].at(0, 0) * 12.0f)));
        r.scale.push_back(static_cast<int>(std::lround(out[1].at(0, 0) * 11.0f)));
        r.change.push_back(out[2].at(0, 0));
        adv.at(0, 0) = 0.0f;
        h.process(in, out);
    }
    return r;
}

void testColtrane() {
    Harmony h;
    h.setParameter("movement", 0.0f);
    h.setParameter("root_start", 0.0f);
    const auto r = stepN(h, 9);
    // +4 st a cada passo, mod 12: 4, 8, 0, 4, 8, 0, ...
    const int expected[9] = {4, 8, 0, 4, 8, 0, 4, 8, 0};
    bool ok = true;
    for (int i = 0; i < 9; ++i)
        if (r.root[i] != expected[i]) ok = false;
    EXPECT(ok);
}

void testBackdoor() {
    Harmony h;
    h.setParameter("movement", 5.0f);
    h.setParameter("root_start", 0.0f);
    const auto r = stepN(h, 6);
    const int expected[6] = {2, 4, 6, 8, 10, 0};
    bool ok = true;
    for (int i = 0; i < 6; ++i)
        if (r.root[i] != expected[i]) ok = false;
    EXPECT(ok);
}

void testModalInterchangeRootFixed() {
    Harmony h;
    h.setParameter("movement", 3.0f);
    h.setParameter("root_start", 5.0f);
    h.setParameter("scale_lo", 1.0f);
    h.setParameter("scale_hi", 6.0f);
    const auto r = stepN(h, 40);
    bool rootFixed = true;
    for (const int rt : r.root)
        if (rt != 5) rootFixed = false;
    EXPECT(rootFixed);   // intercâmbio modal: a raiz nunca se move
    int distinctScales = 0, last = -1;
    for (const int sc : r.scale) {
        if (sc != last) ++distinctScales;
        last = sc;
    }
    EXPECT(distinctScales >= 3);  // ...mas o modo muda
}

void testModalJazzMostlyStatic() {
    Harmony h;
    h.setParameter("movement", 4.0f);
    h.setParameter("root_start", 0.0f);
    const auto r = stepN(h, 200);
    int rootChanges = 0, last = 0;
    for (const int rt : r.root) {
        if (rt != last) ++rootChanges;
        last = rt;
    }
    check(rootChanges < 80, "jazz modal: a raiz muda raramente (~30% dos passos)");
    EXPECT(rootChanges > 0);
}

void testChangeGate() {
    Harmony h;
    h.setParameter("movement", 5.0f);   // sempre muda
    const auto r = stepN(h, 20);
    int pulses = 0;
    for (const float c : r.change)
        if (c > 0.5f) ++pulses;
    check(pulses >= 15, "change pulsa quando o centro tonal avança");
}

void testResetReturns() {
    Harmony h;
    h.setParameter("movement", 5.0f);
    h.setParameter("root_start", 3.0f);
    h.prepare(kSampleRate, 1);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 1));
    AudioBlock adv(kSampleRate, 1, 1), rst(kSampleRate, 1, 1);
    std::vector<const AudioBlock*> in{&adv, &rst};
    for (int i = 0; i < 5; ++i) {
        adv.at(0, 0) = 1.0f; rst.at(0, 0) = 0.0f; h.process(in, out);
        adv.at(0, 0) = 0.0f; h.process(in, out);
    }
    check(std::lround(out[0].at(0, 0) * 12.0f) != 3, "andou pra longe de root_start");
    rst.at(0, 0) = 1.0f; h.process(in, out);
    check(std::lround(out[0].at(0, 0) * 12.0f) == 3, "reset volta a root_start");
}

void testInternalClock() {
    Harmony h;
    h.setParameter("movement", 5.0f);
    h.setParameter("rate", 2.0f);   // 2 avanços/s (máximo do parâmetro)
    h.setParameter("root_start", 0.0f);
    h.prepare(kSampleRate, 64);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    std::vector<int> roots;
    for (int b = 0; b < static_cast<int>(kSampleRate * 5.0f) / 64; ++b) {
        h.process(in, out);
        roots.push_back(static_cast<int>(std::lround(out[0].at(0, 0) * 12.0f)));
    }
    int changes = 0, last = 0;
    for (const int rt : roots) { if (rt != last) ++changes; last = rt; }
    check(changes >= 6, "relogio interno avanca o centro tonal (~10 em 5 s a 2 Hz)");
}

void testDeterminism() {
    Harmony a, b;
    for (Harmony* h : {&a, &b}) {
        h->setParameter("movement", 2.0f);
        h->setParameter("scale_lo", 1.0f);
        h->setParameter("scale_hi", 8.0f);
        h->setParameter("hold", 0.3f);
    }
    const auto ra = stepN(a, 300);
    const auto rb = stepN(b, 300);
    bool identical = ra.root.size() == rb.root.size();
    for (std::size_t i = 0; identical && i < ra.root.size(); ++i)
        if (ra.root[i] != rb.root[i] || ra.scale[i] != rb.scale[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    // TURING.cv -> QUANTIZER.cv ; HARMONY -> QUANTIZER.root/scale (params)
    SignalGraph graph;
    const auto tur = graph.add(std::make_unique<TuringLoop>());
    graph.node(tur).setParameter("rate", 6.0f);
    graph.node(tur).setParameter("range", 1.0f);
    const auto harm = graph.add(std::make_unique<Harmony>());
    graph.node(harm).setParameter("movement", 0.0f);
    graph.node(harm).setParameter("rate", 0.5f);
    const auto quant = graph.add(std::make_unique<Quantizer>());
    graph.node(quant).setParameter("range", 2.0f);
    graph.connect(tur, 0, quant, 0);                       // cv -> cv
    graph.connectToParameter(harm, 0, quant, "root", 12.0f);   // root
    graph.connectToParameter(harm, 1, quant, "scale", 11.0f);  // scale
    graph.prepare(kSampleRate, 1, 128);
    AudioBlock out(kSampleRate, 1, 128);
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, quant, 0);
        for (std::size_t i = 0; i < 128; ++i)
            EXPECT(std::isfinite(out.at(0, i)));
    }
    // o pitch quantizado é sempre um semitom inteiro válido
    const float semi = out.at(0, 0) * 12.0f;
    EXPECT(std::fabs(semi - std::lround(semi)) < 1.0e-3f);
}

void testPanel() {
    Harmony h;
    const std::string problem = validatePanel(h);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(h.panel().widgets.size() >= 11);
    std::cout << renderAscii(h);
}

}  // namespace

int main() {
    testColtrane();
    testBackdoor();
    testModalInterchangeRootFixed();
    testModalJazzMostlyStatic();
    testChangeGate();
    testResetReturns();
    testInternalClock();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular HARMONY tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
