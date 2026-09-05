// Teste isolado do Módulo 37 (PLL) - antes de entrar num patch.
// Critérios do dossiê `dossies/37_pll.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Pll.hpp"
#include "dsp/Oscillator.hpp"
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

// out=0, ring=1, lock=2
std::vector<float> render(Pll& p, const int outIdx, const int samples,
                          const float refHz = 0.0f) {
    p.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kBlock));
    AudioBlock refBlock(kSr, 1, kBlock);
    std::vector<const AudioBlock*> in{
        nullptr, nullptr, refHz > 0.0f ? &refBlock : nullptr};
    std::vector<float> r;
    double refPhase = 0.0;
    while (static_cast<int>(r.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            if (refHz > 0.0f) {
                refBlock.at(0, i) = static_cast<float>(2.0 * refPhase - 1.0);
                refPhase += refHz / kSr;
                refPhase -= std::floor(refPhase);
            }
        }
        p.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, i));
    }
    return r;
}

double acFreq(const std::vector<float>& v, const std::size_t start,
              const std::size_t end) {
    // frequência por cruzamento de zero (mesmo espírito de test_oscillator.cpp)
    int crossings = 0;
    for (std::size_t i = start + 1; i < end && i < v.size(); ++i)
        if (v[i - 1] < 0.0f && v[i] >= 0.0f) ++crossings;
    const double seconds = static_cast<double>(end - start) / kSr;
    return crossings / seconds;
}

void testFreeRunningWithoutRef() {
    // sem REF plugado -- toca livre, e' um VCO de verdade, nao so um
    // efeito que depende de outro
    Pll p;
    p.setParameter("freq", 330.0f);
    const auto v = render(p, 0, 40000);
    for (const float x : v) EXPECT(std::isfinite(x) && std::fabs(x) <= 1.2f);
    const double f = acFreq(v, 5000, 38000);
    check(std::fabs(f - 330.0) / 330.0 < 0.05, "sem REF, toca na propria FREQ");
}

void testLocksToReference() {
    // FREQ moderadamente longe da referencia (dentro do alcance de
    // captura -- ver nota de ALCANCE DE CAPTURA no dossie/no header:
    // a correcao e' limitada a +-0,9, entao um descompasso extremo
    // (ex.: pedir a razao exata 2,0x com FREQ livre bem longe do
    // alvo) pode nao travar de vez, igual um PLL real -- medido por
    // sonda dedicada antes deste teste)
    Pll p;
    p.setParameter("freq", 220.0f);
    p.setParameter("ratio", 1.0f);
    p.setParameter("lock_gain", 1.0f);
    const auto v = render(p, 0, 60000, 300.0f);
    const double f = acFreq(v, 40000, 58000);
    check(std::fabs(f - 300.0) / 300.0 < 0.1,
          "com REF e LOCK_GAIN alto, a taxa efetiva trava na referencia");
}

void testRatioLocksToMultiple() {
    // FREQ livre já perto do alvo (400 Hz = 200x2) -- dentro do
    // alcance de captura, trava exato (medido por sonda dedicada)
    Pll p;
    p.setParameter("freq", 350.0f);
    p.setParameter("ratio", 2.0f);   // trava numa OITAVA acima da ref
    p.setParameter("lock_gain", 1.0f);
    const auto v = render(p, 0, 60000, 200.0f);
    const double f = acFreq(v, 40000, 58000);
    check(std::fabs(f - 400.0) / 400.0 < 0.1,
          "RATIO=2 trava numa oitava acima da referencia (nao 1:1)");
}

void testLockCvRisesWhenLocked() {
    Pll p;
    p.setParameter("freq", 220.0f);
    p.setParameter("ratio", 1.0f);
    p.setParameter("lock_gain", 1.0f);
    const auto lockLocked = render(p, 2, 60000, 220.0f);  // ja comeca perto
    float meanLocked = 0.0f;
    for (std::size_t i = 40000; i < 58000; ++i) meanLocked += lockLocked[i];
    meanLocked /= 18000.0f;

    Pll q;
    q.setParameter("freq", 220.0f);
    // sem referencia nenhuma -- lock so pode ser 0
    const auto lockFree = render(q, 2, 60000, 0.0f);
    float meanFree = 0.0f;
    for (std::size_t i = 40000; i < 58000; ++i) meanFree += lockFree[i];
    meanFree /= 18000.0f;

    check(meanLocked > meanFree, "LOCK sobe quando ha referencia e trava perto");
    check(meanFree == 0.0f, "sem referencia, LOCK fica em 0 (nada pra travar)");
}

void testFeedbackTypesStayBoundedAndFinite() {
    for (float ft = 0.0f; ft <= 5.0f; ft += 1.0f) {
        Pll p;
        p.setParameter("freq", 500.0f);
        p.setParameter("feedback_amount", 0.9f);
        p.setParameter("feedback_type", ft);
        const auto v = render(p, 0, 20000);
        for (const float x : v) EXPECT(std::isfinite(x) && std::fabs(x) <= 1.5f);
    }
}

void testRingIsZeroWithoutReference() {
    Pll p;
    p.setParameter("freq", 400.0f);
    const auto ringOut = render(p, 1, 10000);  // sem REF
    bool allZero = true;
    for (const float x : ringOut) if (x != 0.0f) allZero = false;
    check(allZero, "RING precisa de REF (referencia*saida) -- sem ref, fica em 0");
}

void testDeterminism() {
    Pll a, b;
    for (Pll* p : {&a, &b}) {
        p->setParameter("freq", 260.0f);
        p->setParameter("shape", 1.7f);
        p->setParameter("feedback_amount", 0.4f);
        p->setParameter("feedback_type", 3.0f);
    }
    const auto ra = render(a, 0, 30000, 180.0f);
    const auto rb = render(b, 0, 30000, 180.0f);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        if (ra[i] != rb[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph graph;
    const auto osc = graph.add(std::make_unique<Oscillator>());
    graph.node(osc).setParameter("freq", 220.0f);
    const auto pll = graph.add(std::make_unique<Pll>());
    graph.node(pll).setParameter("freq", 220.0f);
    graph.node(pll).setParameter("lock_gain", 0.6f);
    graph.connect(osc, 2, pll, 2);   // OSC.saw -> PLL.ref
    graph.prepare(kSr, 1, kBlock);
    AudioBlock out(kSr, 1, kBlock);
    for (int b = 0; b < 500; ++b) {
        graph.process(out, pll, 0);
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(out.at(0, i)));
    }
}

void testPanel() {
    Pll p;
    const std::string problem = validatePanel(p);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    std::cout << renderAscii(p);
}

}  // namespace

int main() {
    testFreeRunningWithoutRef();
    testLocksToReference();
    testRatioLocksToMultiple();
    testLockCvRisesWhenLocked();
    testFeedbackTypesStayBoundedAndFinite();
    testRingIsZeroWithoutReference();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) {
        std::cout << "RASGO Modular PLL tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
