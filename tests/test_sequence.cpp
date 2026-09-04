// Teste isolado do Módulo 15 (SEQUENCE) - antes de entrar num patch.
// Critérios do dossiê `dossies/15_sequence.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/StepSequencer.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/FunctionGenerator.hpp"
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
    std::vector<float> pitch, gate, eos;
};

// Avança N passos por clock externo, lendo a saída logo após cada passo.
Steps stepN(StepSequencer& s, const int steps) {
    s.prepare(kSampleRate, 1);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 1));
    AudioBlock clk(kSampleRate, 1, 1);
    std::vector<const AudioBlock*> in{&clk, nullptr};
    Steps r;
    for (int i = 0; i < steps; ++i) {
        clk.at(0, 0) = 1.0f;
        s.process(in, out);
        r.pitch.push_back(out[0].at(0, 0));
        r.gate.push_back(out[1].at(0, 0));
        r.eos.push_back(out[2].at(0, 0));
        clk.at(0, 0) = 0.0f;
        s.process(in, out);
    }
    return r;
}

void setPattern(StepSequencer& s, const float* p, const float* g, int n) {
    for (int i = 0; i < n; ++i) {
        s.setParameter("p" + std::to_string(i + 1), p[i]);
        s.setParameter("g" + std::to_string(i + 1), g[i]);
    }
}

void testForward() {
    StepSequencer s;
    s.setParameter("length", 4.0f);
    s.setParameter("mode", 0.0f);
    s.setParameter("range", 1.0f);
    s.setParameter("glide", 0.0f);
    const float p[4] = {-0.5f, 0.0f, 0.5f, 1.0f};
    const float g[4] = {1, 1, 1, 1};
    setPattern(s, p, g, 4);
    const auto r = stepN(s, 12);
    // idx começa em 0, primeiro clock -> idx 1, etc: p1,p2,p3,p0,p1,...
    const float expect[12] = {0.0f, 0.5f, 1.0f, -0.5f, 0.0f, 0.5f,
                              1.0f, -0.5f, 0.0f, 0.5f, 1.0f, -0.5f};
    bool ok = true;
    for (int i = 0; i < 12; ++i)
        if (std::fabs(r.pitch[i] - expect[i]) > 1.0e-5f) ok = false;
    EXPECT(ok);
}

void testBackward() {
    StepSequencer s;
    s.setParameter("length", 4.0f);
    s.setParameter("mode", 1.0f);
    s.setParameter("range", 1.0f);
    const float p[4] = {-0.5f, 0.0f, 0.5f, 1.0f};
    const float g[4] = {1, 1, 1, 1};
    setPattern(s, p, g, 4);
    const auto r = stepN(s, 8);
    // idx 0 -> -1%4 = 3 -> 2 -> 1 -> 0 -> 3 ...
    const float expect[8] = {1.0f, 0.5f, 0.0f, -0.5f, 1.0f, 0.5f, 0.0f, -0.5f};
    bool ok = true;
    for (int i = 0; i < 8; ++i)
        if (std::fabs(r.pitch[i] - expect[i]) > 1.0e-5f) ok = false;
    EXPECT(ok);
}

void testPingpong() {
    StepSequencer s;
    s.setParameter("length", 4.0f);
    s.setParameter("mode", 2.0f);
    s.setParameter("range", 1.0f);
    const float p[4] = {0.0f, 0.1f, 0.2f, 0.3f};
    const float g[4] = {1, 1, 1, 1};
    setPattern(s, p, g, 4);
    const auto r = stepN(s, 8);
    // 0 ->1 ->2 ->3 ->2 ->1 ->0 ->1 ->2
    const float expect[8] = {0.1f, 0.2f, 0.3f, 0.2f, 0.1f, 0.0f, 0.1f, 0.2f};
    bool ok = true;
    for (int i = 0; i < 8; ++i)
        if (std::fabs(r.pitch[i] - expect[i]) > 1.0e-5f) ok = false;
    EXPECT(ok);
}

void testGatePattern() {
    StepSequencer s;
    s.setParameter("length", 4.0f);
    s.setParameter("mode", 0.0f);
    const float p[4] = {0, 0, 0, 0};
    const float g[4] = {1, 0, 1, 0};
    setPattern(s, p, g, 4);
    const auto r = stepN(s, 8);
    // após clock: idx 1(g=0), 2(g=1), 3(g=0), 0(g=1), 1(g=0)...
    const int expectGate[8] = {0, 1, 0, 1, 0, 1, 0, 1};
    bool ok = true;
    for (int i = 0; i < 8; ++i)
        if ((r.gate[i] > 0.5f) != (expectGate[i] != 0)) ok = false;
    EXPECT(ok);
}

void testEos() {
    // relógio interno a 30 Hz, length 3 -> 10 ciclos/s. eos pulsa ~3 ms.
    StepSequencer s;
    s.setParameter("length", 3.0f);
    s.setParameter("mode", 0.0f);
    s.setParameter("rate", 30.0f);
    const float p[3] = {0, 0, 0};
    const float g[3] = {1, 1, 1};
    setPattern(s, p, g, 3);
    s.prepare(kSampleRate, 64);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    int rising = 0;
    float prev = 0.0f;
    for (int b = 0; b < static_cast<int>(kSampleRate * 2.0f) / 64; ++b) {
        s.process(in, out);
        for (std::size_t i = 0; i < 64; ++i) {
            const float e = out[2].at(0, i);
            if (e > 0.5f && prev < 0.5f) ++rising;
            prev = e;
        }
    }
    // ~20 voltas a idx 0 em 2 s (30 Hz / 3 passos = 10 ciclos/s)
    check(rising >= 18 && rising <= 22, "eos pulsa uma vez por ciclo (length=3)");
}

void testGlide() {
    StepSequencer s;
    s.setParameter("length", 2.0f);
    s.setParameter("mode", 0.0f);
    s.setParameter("range", 1.0f);
    s.setParameter("glide", 0.5f);
    const float p[2] = {-1.0f, 1.0f};
    const float g[2] = {1, 1};
    setPattern(s, p, g, 2);
    s.prepare(kSampleRate, 64);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 64));
    AudioBlock clk(kSampleRate, 1, 64);
    std::vector<const AudioBlock*> in{&clk, nullptr};
    float prev = -1.0f, maxJump = 0.0f;
    long sample = 0;
    for (int b = 0; b < 4000; ++b) {
        for (std::size_t i = 0; i < 64; ++i, ++sample)
            clk.at(0, i) = (sample % 6000 < 100) ? 1.0f : 0.0f;
        s.process(in, out);
        for (std::size_t i = 0; i < 64; ++i) {
            if (b > 0)  // b==0 só semeia `prev`, sem medir o transiente
                maxJump = std::max(maxJump, std::fabs(out[0].at(0, i) - prev));
            prev = out[0].at(0, i);
        }
    }
    check(maxJump < 0.02f, "glide suaviza os saltos de altura entre passos");
}

void testInternalClockAndDeterminism() {
    StepSequencer a, b;
    for (StepSequencer* s : {&a, &b}) {
        s->setParameter("mode", 3.0f);   // random
        s->setParameter("rate", 8.0f);
        s->setParameter("range", 1.0f);
    }
    a.prepare(kSampleRate, 64);
    b.prepare(kSampleRate, 64);
    std::vector<AudioBlock> oa(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<AudioBlock> ob(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<const AudioBlock*> in{nullptr, nullptr};
    bool identical = true, moved = false;
    float first = -999.0f;
    for (int blk = 0; blk < 3000; ++blk) {
        a.process(in, oa);
        b.process(in, ob);
        for (std::size_t i = 0; i < 64; ++i) {
            if (oa[0].at(0, i) != ob[0].at(0, i)) identical = false;
            if (first == -999.0f) first = oa[0].at(0, i);
            if (std::fabs(oa[0].at(0, i) - first) > 1.0e-4f) moved = true;
        }
    }
    EXPECT(identical);
    EXPECT(moved);  // o relógio interno avançou o sequenciador
}

void testInGraph() {
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 120.0f);
    graph.node(clk).setParameter("mult", 4.0f);
    graph.node(clk).setParameter("fill", 16.0f);
    const auto seq = graph.add(std::make_unique<StepSequencer>());
    graph.node(seq).setParameter("range", 1.0f);
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 220.0f);
    graph.connect(clk, 0, seq, 0);          // clock -> clock
    graph.connect(seq, 0, voice, 0);        // pitch (oitavas) -> rate_mod
    graph.prepare(kSampleRate, 1, 128);
    AudioBlock out(kSampleRate, 1, 128);
    float peak = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, voice, 1);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 0.3f);
}

void testPanel() {
    StepSequencer s;
    const std::string problem = validatePanel(s);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(s.panel().widgets.size() >= 24);
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testForward();
    testBackward();
    testPingpong();
    testGatePattern();
    testEos();
    testGlide();
    testInternalClockAndDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SEQUENCE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
