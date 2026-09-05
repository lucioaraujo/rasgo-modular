// Teste isolado do Módulo 36 (CHAOS) - antes de entrar num patch.
// Critérios do dossiê `dossies/36_chaos.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Chaos.hpp"
#include "dsp/Filter.hpp"
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

constexpr float kSr = 48000.0f;
constexpr std::size_t kBlock = 64;

std::vector<float> render(Chaos& c, const int samples) {
    c.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kBlock));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    std::vector<float> r;
    while (static_cast<int>(r.size()) < samples) {
        c.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i) r.push_back(out[0].at(0, i));
    }
    return r;
}

void testBoundedAndFinite() {
    Chaos c;
    c.setParameter("rate", 8.0f);
    c.setParameter("drive", 0.9f);
    c.setParameter("damping", 0.2f);
    const auto v = render(c, 96000);
    for (const float x : v) EXPECT(std::isfinite(x) && std::fabs(x) <= 1.0f);
}

void testCrossesBothWells() {
    // sem o chute periódico o sistema fica preso num poço só (achado do
    // próprio Antitotem) -- confere que a saída de verdade visita os
    // dois lados de 0, não só um
    Chaos c;
    c.setParameter("rate", 6.0f);
    c.setParameter("drive", 0.7f);
    c.setParameter("damping", 0.3f);
    const auto v = render(c, static_cast<int>(kSr * 6.0f));
    bool sawPositive = false, sawNegative = false;
    for (const float x : v) {
        if (x > 0.3f) sawPositive = true;
        if (x < -0.3f) sawNegative = true;
    }
    check(sawPositive && sawNegative,
          "a trajetoria visita os dois pocos (nao fica presa num so)");
}

void testFreezeHoldsExactly() {
    Chaos c;
    c.setParameter("rate", 10.0f);
    c.setParameter("drive", 0.8f);
    c.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kBlock));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    for (int b = 0; b < 200; ++b) c.process(in, out);  // deixa evoluir
    c.setParameter("freeze", 1.0f);
    c.process(in, out);
    const float held = out[0].at(0, kBlock - 1);
    bool same = true;
    for (int b = 0; b < 500; ++b) {
        c.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            if (out[0].at(0, i) != held) same = false;
    }
    check(same, "freeze segura a trajetoria exatamente parada");
}

void testReseedJumps() {
    // compara uma tomada COM reseed no meio contra uma tomada de
    // controle SEM reseed, a partir do mesmo estado inicial -- o
    // reseed muda x E y ao mesmo tempo, então mesmo quando o salto
    // instantâneo em x é pequeno por acaso (o sorteio pode calhar perto
    // do valor anterior), a trajetória subsequente diverge da que
    // continuaria sem o reseed.
    Chaos withReseed, control;
    for (Chaos* c : {&withReseed, &control}) {
        c->setParameter("rate", 2.0f);
        c->setParameter("drive", 0.1f);
        c->prepare(kSr, kBlock);
    }
    std::vector<AudioBlock> outR(1, AudioBlock(kSr, 1, kBlock));
    std::vector<AudioBlock> outC(1, AudioBlock(kSr, 1, kBlock));
    AudioBlock trig(kSr, 1, kBlock), zero(kSr, 1, kBlock);
    std::vector<const AudioBlock*> inR{&trig, nullptr};
    std::vector<const AudioBlock*> inC{&zero, nullptr};
    for (std::size_t i = 0; i < kBlock; ++i) { trig.at(0, i) = 0.0f; zero.at(0, i) = 0.0f; }
    for (int b = 0; b < 50; ++b) { withReseed.process(inR, outR); control.process(inC, outC); }
    for (std::size_t i = 0; i < kBlock; ++i) trig.at(0, i) = 1.0f;
    withReseed.process(inR, outR);
    control.process(inC, outC);
    for (std::size_t i = 0; i < kBlock; ++i) trig.at(0, i) = 0.0f;
    float maxDiff = 0.0f;
    for (int b = 0; b < 200; ++b) {
        withReseed.process(inR, outR);
        control.process(inC, outC);
        for (std::size_t i = 0; i < kBlock; ++i)
            maxDiff = std::max(maxDiff,
                std::fabs(outR[0].at(0, i) - outC[0].at(0, i)));
    }
    check(maxDiff > 0.1f,
          "reseed faz a trajetoria divergir da que continuaria sem ele");
}

void testDeterminism() {
    Chaos a, b;
    for (Chaos* c : {&a, &b}) {
        c->setParameter("rate", 12.0f);
        c->setParameter("drive", 0.65f);
        c->setParameter("damping", 0.25f);
    }
    const auto ra = render(a, 40000);
    const auto rb = render(b, 40000);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        if (ra[i] != rb[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph graph;
    const auto ch = graph.add(std::make_unique<Chaos>());
    graph.node(ch).setParameter("rate", 20.0f);
    graph.node(ch).setParameter("drive", 0.8f);
    const auto f = graph.add(std::make_unique<Filter>());
    graph.connect(ch, 0, f, 0);
    graph.prepare(kSr, 1, kBlock);
    AudioBlock out(kSr, 1, kBlock);
    for (int b = 0; b < 500; ++b) {
        graph.process(out, f, 3);  // ALL
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(out.at(0, i)));
    }
}

void testPanel() {
    Chaos c;
    const std::string problem = validatePanel(c);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    std::cout << renderAscii(c);
}

}  // namespace

int main() {
    testBoundedAndFinite();
    testCrossesBothWells();
    testFreezeHoldsExactly();
    testReseedJumps();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) {
        std::cout << "RASGO Modular CHAOS tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
