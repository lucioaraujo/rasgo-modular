// Teste isolado do Módulo 8 (TURING) - antes de entrar num patch.
// Critérios do dossiê `dossies/08_turing.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/TuringLoop.hpp"
#include "dsp/EuclidClock.hpp"
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

constexpr float kSampleRate = 48000.0f;

struct Steps {
    std::vector<float> cv, cv2, pulse;
};

// Avança N passos por clock externo (1 amostra/bloco: sobe, desce).
Steps stepN(TuringLoop& t, const int steps) {
    t.prepare(kSampleRate, 1);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 1));
    AudioBlock clk(kSampleRate, 1, 1);
    std::vector<const AudioBlock*> in{&clk, nullptr};
    Steps r;
    for (int i = 0; i < steps; ++i) {
        clk.at(0, 0) = 1.0f;
        t.process(in, out);
        r.cv.push_back(out[0].at(0, 0));
        r.cv2.push_back(out[1].at(0, 0));
        r.pulse.push_back(out[2].at(0, 0));
        clk.at(0, 0) = 0.0f;
        t.process(in, out);
    }
    return r;
}

double variance(const std::vector<float>& v, const std::size_t a) {
    double m = 0.0;
    std::size_t n = 0;
    for (std::size_t i = a; i < v.size(); ++i) { m += v[i]; ++n; }
    m /= static_cast<double>(n);
    double s = 0.0;
    for (std::size_t i = a; i < v.size(); ++i) s += (v[i] - m) * (v[i] - m);
    return s / static_cast<double>(n);
}

void testLockedLoop() {
    TuringLoop t;
    t.setParameter("lock", 1.0f);
    t.setParameter("length", 6.0f);
    t.setParameter("range", 1.0f);
    t.setParameter("steps", 1.0f);
    const Steps r = stepN(t, 80);
    bool periodic = true;
    for (std::size_t i = 20; i + 6 < r.cv.size(); ++i)
        if (std::fabs(r.cv[i] - r.cv[i + 6]) > 1.0e-6f) periodic = false;
    EXPECT(periodic);
    EXPECT(variance(r.cv, 20) > 1.0e-4);  // não é constante
}

void testUnlockedRandom() {
    TuringLoop t;
    t.setParameter("lock", 0.0f);
    t.setParameter("length", 6.0f);
    t.setParameter("range", 1.0f);
    const Steps r = stepN(t, 400);
    // não deve ser periódico com período 6
    int matches = 0;
    for (std::size_t i = 50; i + 6 < r.cv.size(); ++i)
        if (std::fabs(r.cv[i] - r.cv[i + 6]) < 1.0e-6f) ++matches;
    check(matches < 30, "lock=0 nao trava num laco de comprimento length");
    check(variance(r.cv, 50) > 0.02, "lock=0 tem variacao alta");
}

void testRangeAndSteps() {
    TuringLoop zero;
    zero.setParameter("range", 0.0f);
    zero.setParameter("offset", 0.25f);
    const Steps rz = stepN(zero, 60);
    bool allOffset = true;
    for (const float v : rz.cv)
        if (std::fabs(v - 0.25f) > 1.0e-5f) allOffset = false;
    EXPECT(allOffset);  // range=0 -> cv fixo no offset

    TuringLoop q;
    q.setParameter("lock", 0.0f);
    q.setParameter("range", 1.0f);
    q.setParameter("steps", 3.0f);
    const Steps rq = stepN(q, 300);
    const float levels[3] = {-1.0f, 0.0f, 1.0f};
    bool onGrid = true;
    for (const float v : rq.cv) {
        bool m = false;
        for (int k = 0; k < 3; ++k)
            if (std::fabs(v - levels[k]) < 1.0e-5f) m = true;
        if (!m) onGrid = false;
    }
    EXPECT(onGrid);
}

void testInternalClock() {
    TuringLoop t;
    t.setParameter("rate", 8.0f);
    t.setParameter("lock", 0.0f);
    t.setParameter("range", 1.0f);
    t.prepare(kSampleRate, 64);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    std::vector<float> cv;
    for (int b = 0; b < static_cast<int>(kSampleRate * 4.0f) / 64; ++b) {
        t.process(in, out);
        cv.push_back(out[0].at(0, 0));
    }
    check(variance(cv, 0) > 0.02, "relogio interno avanca o registrador");
}

void testDeterminism() {
    TuringLoop a, b;
    for (TuringLoop* t : {&a, &b}) {
        t->setParameter("lock", 0.4f);
        t->setParameter("length", 7.0f);
        t->setParameter("range", 1.0f);
    }
    const Steps ra = stepN(a, 500);
    const Steps rb = stepN(b, 500);
    bool identical = ra.cv.size() == rb.cv.size();
    for (std::size_t i = 0; identical && i < ra.cv.size(); ++i)
        if (ra.cv[i] != rb.cv[i] || ra.cv2[i] != rb.cv2[i]
            || ra.pulse[i] != rb.pulse[i])
            identical = false;
    EXPECT(identical);
}

void testPulseIsBit() {
    TuringLoop t;
    t.setParameter("lock", 0.3f);
    const Steps r = stepN(t, 200);
    for (const float p : r.pulse)
        EXPECT(p == 0.0f || p == 1.0f);
}

void testInGraph() {
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 120.0f);
    graph.node(clk).setParameter("mult", 4.0f);
    graph.node(clk).setParameter("fill", 16.0f);
    const auto tur = graph.add(std::make_unique<TuringLoop>());
    graph.node(tur).setParameter("lock", 0.7f);
    graph.node(tur).setParameter("range", 1.0f);
    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 500.0f);
    graph.connect(clk, 0, tur, 0);              // clock -> clock
    graph.connect(tur, 0, filter, 1, false, 2.0f);  // cv -> cutoff_mod
    graph.connect(tur, 1, filter, 0);          // cv2 -> in (material)
    graph.prepare(kSampleRate, 1, 128);
    AudioBlock out(kSampleRate, 1, 128);
    float peak = 0.0f;
    for (int b = 0; b < 1500; ++b) {
        graph.process(out, filter, 3);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    TuringLoop t;
    const std::string problem = validatePanel(t);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(t.panel().widgets.size() >= 12);
    std::cout << renderAscii(t);
}

}  // namespace

int main() {
    testLockedLoop();
    testUnlockedRandom();
    testRangeAndSteps();
    testInternalClock();
    testDeterminism();
    testPulseIsBit();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular TURING tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
