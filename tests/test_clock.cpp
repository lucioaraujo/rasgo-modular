// Teste isolado do Módulo 5 (CLOCK) - antes de entrar num patch.
// Critérios do dossiê `dossies/05_clock.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Decision.hpp"
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
constexpr std::size_t kBlock = 64;

struct Counts {
    int clock = 0, euclid = 0, accent = 0;
};

// Roda `seconds` do clock interno; conta bordas de subida de cada saída.
Counts runInternal(EuclidClock& c, const float seconds) {
    c.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    std::vector<const AudioBlock*> in{nullptr, nullptr, nullptr};
    Counts n;
    float pc = 0.0f, pe = 0.0f, pa = 0.0f;
    const int blocks = static_cast<int>(kSampleRate * seconds) / kBlock;
    for (int b = 0; b < blocks; ++b) {
        c.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i) {
            const float vc = out[0].at(0, i), ve = out[1].at(0, i),
                        va = out[2].at(0, i);
            if (pc < 0.5f && vc >= 0.5f) ++n.clock;
            if (pe < 0.5f && ve >= 0.5f) ++n.euclid;
            if (pa < 0.5f && va >= 0.5f) ++n.accent;
            pc = vc; pe = ve; pa = va;
        }
    }
    return n;
}

void testInternalTempo() {
    {
        EuclidClock c;
        c.setParameter("bpm", 120.0f);
        c.setParameter("mult", 2.0f);  // 4 passos/s
        c.setParameter("fill", 32.0f);  // euclid = todo passo
        const Counts n = runInternal(c, 4.0f);  // ~16 passos
        check(n.clock >= 15 && n.clock <= 18, "clock interno ~4 Hz (120 BPM x2)");
        check(n.euclid >= 15 && n.euclid <= 18, "fill cheio -> euclid em todo passo");
    }
    {
        EuclidClock c;
        c.setParameter("bpm", 60.0f);
        c.setParameter("mult", 1.0f);  // 1 passo/s
        c.setParameter("fill", 32.0f);
        const Counts n = runInternal(c, 10.0f);
        check(n.clock >= 9 && n.clock <= 12, "clock interno ~1 Hz (60 BPM x1)");
    }
}

// bordas de subida de `clock`, em índice de amostra (não só contagem) --
// pra medir a REGULARIDADE dos intervalos (glitch precisa ser irregular).
std::vector<long> clockEdgeFrames(EuclidClock& c, const float seconds) {
    c.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    std::vector<const AudioBlock*> in{nullptr, nullptr, nullptr};
    std::vector<long> edges;
    float pc = 0.0f;
    long frame = 0;
    const int blocks = static_cast<int>(kSampleRate * seconds) / kBlock;
    for (int b = 0; b < blocks; ++b) {
        c.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i) {
            const float vc = out[0].at(0, i);
            if (pc < 0.5f && vc >= 0.5f) edges.push_back(frame);
            pc = vc;
            ++frame;
        }
    }
    return edges;
}

void testFeelTuplet() {
    EuclidClock straight, triplet, quintuplet;
    for (EuclidClock* c : {&straight, &triplet, &quintuplet}) {
        c->setParameter("bpm", 60.0f);
        c->setParameter("mult", 1.0f);
        c->setParameter("fill", 32.0f);
    }
    triplet.setParameter("feel", 1.0f);      // tercina, razão 3
    quintuplet.setParameter("feel", 2.0f);   // quintina, razão 5
    const Counts ns = runInternal(straight, 8.0f);
    const Counts nt = runInternal(triplet, 8.0f);
    const Counts nq = runInternal(quintuplet, 8.0f);
    // teórico: 60 BPM x mult=1 = 1 passo/s -> 8 passos em 8 s; tercina
    // (razão 3) e quintina (razão 5) escalam a mesma base
    check(ns.clock >= 7 && ns.clock <= 9, "reto: ~1 Hz (60 BPM x1)");
    check(nt.clock >= 22 && nt.clock <= 26, "tercina: ~3x o passo reto (razão 3)");
    check(nq.clock >= 38 && nq.clock <= 42, "quintina: ~5x o passo reto (razão 5)");
}

void testFeelGlitchIsIrregular() {
    EuclidClock straight, glitch;
    for (EuclidClock* c : {&straight, &glitch}) {
        c->setParameter("bpm", 120.0f);
        c->setParameter("mult", 2.0f);
        c->setParameter("fill", 32.0f);
    }
    glitch.setParameter("feel", 6.0f);
    const auto se = clockEdgeFrames(straight, 6.0f);
    const auto ge = clockEdgeFrames(glitch, 6.0f);
    EXPECT(se.size() > 10);
    EXPECT(ge.size() > 5);
    auto intervalVariance = [](const std::vector<long>& e) {
        if (e.size() < 3) return 0.0;
        std::vector<double> iv;
        for (std::size_t i = 1; i < e.size(); ++i)
            iv.push_back(static_cast<double>(e[i] - e[i - 1]));
        double mean = 0.0;
        for (const double v : iv) mean += v;
        mean /= static_cast<double>(iv.size());
        double var = 0.0;
        for (const double v : iv) var += (v - mean) * (v - mean);
        return var / static_cast<double>(iv.size());
    };
    check(intervalVariance(ge) > intervalVariance(se) * 5.0,
          "glitch: intervalo entre passos bem mais irregular que reto");
}

void testFeelDeterminism() {
    EuclidClock a, b;
    for (EuclidClock* c : {&a, &b}) {
        c->setParameter("bpm", 133.0f);
        c->setParameter("mult", 1.5f);
        c->setParameter("fill", 5.0f);
        c->setParameter("feel", 6.0f);  // glitch -- o caso com RNG
    }
    const auto ea = clockEdgeFrames(a, 5.0f);
    const auto eb = clockEdgeFrames(b, 5.0f);
    EXPECT(ea == eb);
}

void testEuclidDensity() {
    EuclidClock c;
    c.setParameter("bpm", 120.0f);
    c.setParameter("mult", 2.0f);   // 4 Hz
    c.setParameter("length", 8.0f);
    c.setParameter("fill", 3.0f);
    const Counts n = runInternal(c, 20.0f);  // 4 Hz * 20 s = 80 passos = 10 ciclos de 8
    // euclid = fill/length dos passos -> 10 * 3 = 30
    check(n.clock >= 78 && n.clock <= 82, "≈80 passos em 20 s (4 Hz)");
    check(std::abs(n.euclid - 30) <= 2, "E(3,8): ≈3/8 dos passos são onset");
}

void testAccentAndOr() {
    // 4 Hz, 24 passos em 6 s. accent_a=4, accent_b=3.
    {
        EuclidClock c;
        c.setParameter("bpm", 120.0f);
        c.setParameter("mult", 2.0f);
        c.setParameter("fill", 0.0f);   // isola o acento
        c.setParameter("accent_a", 4.0f);
        c.setParameter("accent_b", 3.0f);
        c.setParameter("accent_mode", 0.0f);  // OR
        const Counts n = runInternal(c, 6.0f);
        // union de multiplos de 4 e de 3 em [0,24) = 12
        check(std::abs(n.accent - 12) <= 2, "acento OR de divisores 4 e 3");
        check(n.euclid == 0, "fill=0 -> nenhum onset euclidiano");
    }
    {
        EuclidClock c;
        c.setParameter("bpm", 120.0f);
        c.setParameter("mult", 2.0f);
        c.setParameter("fill", 0.0f);
        c.setParameter("accent_a", 4.0f);
        c.setParameter("accent_b", 3.0f);
        c.setParameter("accent_mode", 1.0f);  // AND -> multiplos de 12
        const Counts n = runInternal(c, 6.0f);
        check(std::abs(n.accent - 2) <= 1, "acento AND -> so multiplos de 12");
    }
}

void testDriftDeterminism() {
    EuclidClock a, b;
    a.setParameter("drift", 0.0f);
    b.setParameter("drift", 0.0f);
    const Counts na = runInternal(a, 8.0f);
    const Counts nb = runInternal(b, 8.0f);
    EXPECT(na.clock == nb.clock && na.euclid == nb.euclid);

    EuclidClock c, d;
    c.setParameter("drift", 0.9f);
    d.setParameter("drift", 0.9f);
    const Counts nc = runInternal(c, 8.0f);
    const Counts nd = runInternal(d, 8.0f);
    EXPECT(nc.clock == nd.clock);  // mesma seed -> mesma deriva
}

void testExternalClock() {
    // ext_clock a ~10 Hz (borda a cada 4800 amostras); o CLOCK deve
    // avançar um passo por borda.
    EuclidClock c;
    c.setParameter("fill", 32.0f);  // euclid segue todo passo
    c.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock ext(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> in{&ext, nullptr, nullptr};
    int edges = 0, euclid = 0;
    float pe = 0.0f;
    long sample = 0;
    const int blocks = static_cast<int>(kSampleRate * 5.0f) / kBlock;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i, ++sample) {
            const bool high = (sample % 4800) < 200;  // pulso curto ~10 Hz
            ext.at(0, i) = high ? 1.0f : 0.0f;
            if (high && (sample % 4800) == 0) ++edges;
        }
        c.process(in, out);
        for (std::size_t i = 0; i < kBlock; ++i) {
            const float ve = out[1].at(0, i);
            if (pe < 0.5f && ve >= 0.5f) ++euclid;
            pe = ve;
        }
    }
    check(std::abs(euclid - edges) <= 2, "euclid segue as bordas do ext_clock");
    check(edges >= 48 && edges <= 52, "≈50 bordas externas em 5 s");
}

void testResetRestartsPattern() {
    EuclidClock c;
    c.setParameter("bpm", 120.0f);
    c.setParameter("mult", 2.0f);
    c.setParameter("length", 5.0f);
    c.setParameter("fill", 2.0f);
    c.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock reset(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> in{nullptr, &reset, nullptr};

    // roda um tempo sem reset
    for (int b = 0; b < 200; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) reset.at(0, i) = 0.0f;
        c.process(in, out);
    }
    // pulso de reset e um bloco: o passo 0 é sempre onset (fill>0)
    for (std::size_t i = 0; i < kBlock; ++i) reset.at(0, i) = (i < 4) ? 1.0f : 0.0f;
    c.process(in, out);
    bool euclidHighEarly = false;
    for (std::size_t i = 4; i < 20; ++i)
        if (out[1].at(0, i) > 0.5f) euclidHighEarly = true;
    EXPECT(euclidHighEarly);
}

void testNoAllocFinite() {
    EuclidClock c;
    c.setParameter("drift", 0.6f);
    c.setParameter("swing", 0.5f);
    c.prepare(kSampleRate, 128);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 2, 128));
    AudioBlock ext(kSampleRate, 2, 128), reset(kSampleRate, 2, 128);
    std::vector<const AudioBlock*> in{&ext, &reset, nullptr};
    long sample = 0;
    for (int b = 0; b < 4000; ++b) {
        for (std::size_t i = 0; i < 128; ++i, ++sample) {
            ext.at(0, i) = (sample % 3000 < 100) ? 1.0f : 0.0f;
            reset.at(0, i) = (sample % 97000 < 50) ? 1.0f : 0.0f;
        }
        c.process(in, out);
        for (std::size_t i = 0; i < 128; ++i)
            for (int o = 0; o < 3; ++o) {
                const float v = out[o].at(0, i);
                EXPECT(v == 0.0f || v == 1.0f);
            }
    }
}

void testInGraph() {
    // CLOCK.euclid -> DECISION.trigger ; DECISION.x -> FILTER.cutoff_mod ;
    // DECISION.y -> FILTER.in. O ritmo dirige as decisões.
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 140.0f);
    graph.node(clk).setParameter("mult", 4.0f);
    graph.node(clk).setParameter("fill", 5.0f);
    const auto dec = graph.add(std::make_unique<Decision>());
    graph.node(dec).setParameter("spread", 1.0f);
    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 500.0f);
    graph.connect(clk, 1, dec, 0);              // euclid -> trigger
    graph.connect(dec, 1, filter, 0);           // y -> in
    graph.connect(dec, 0, filter, 1, false, 2.0f);  // x -> cutoff_mod
    graph.prepare(kSampleRate, 1, 128);

    AudioBlock out(kSampleRate, 1, 128);
    float peak = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, filter, 3);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    EuclidClock c;
    const std::string problem = validatePanel(c);
    check(problem.empty(), "descricao de painel fecha (todo bind resolve)");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    const Panel p = c.panel();
    EXPECT(p.hp > 0);
    EXPECT(p.widgets.size() >= 17);
    std::cout << renderAscii(c);
}

}  // namespace

int main() {
    testInternalTempo();
    testFeelTuplet();
    testFeelGlitchIsIrregular();
    testFeelDeterminism();
    testEuclidDensity();
    testAccentAndOr();
    testDriftDeterminism();
    testExternalClock();
    testResetRestartsPattern();
    testNoAllocFinite();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular CLOCK tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
