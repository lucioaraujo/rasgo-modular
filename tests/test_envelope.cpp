// Teste isolado do Módulo 6 (ENVELOPE) - antes de entrar num patch.
// Critérios do dossiê `dossies/06_envelope.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Envelope.hpp"
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
constexpr std::size_t kBlock = 64;

struct Trace {
    std::vector<float> env, out;
};

// gateHighSamples: nº de amostras iniciais com gate=1 (depois 0).
Trace run(Envelope& e, const int samples, const int gateHighSamples,
          const float audioIn = 0.0f) {
    e.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), gate(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &gate, nullptr};
    Trace t;
    int s = 0;
    while (static_cast<int>(t.env.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            in.at(0, i) = audioIn;
            gate.at(0, i) = (s + static_cast<int>(i) < gateHighSamples) ? 1.0f : 0.0f;
        }
        e.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i) {
            t.env.push_back(out[1].at(0, i));
            t.out.push_back(out[0].at(0, i));
        }
        s += static_cast<int>(kBlock);
    }
    return t;
}

float peak(const std::vector<float>& v) {
    float p = 0.0f;
    for (const float x : v) p = std::max(p, std::fabs(x));
    return p;
}

void testADShape() {
    Envelope e;
    e.setParameter("mode", 1.0f);      // trigger / AD
    e.setParameter("attack", 0.01f);   // 480 amostras
    e.setParameter("decay", 0.1f);     // 4800 amostras
    e.setParameter("sustain", 0.0f);
    const Trace t = run(e, 12000, 32);  // gate curto: trigger

    check(t.env[50] < t.env[460], "envelope sobe durante o ataque");
    check(t.env[470] > 0.9f && t.env[470] <= 1.001f, "pico ~1 ao fim do ataque");
    check(std::fabs(peak(t.env) - 1.0f) < 0.02f, "pico do envelope ~= level (1)");
    check(t.env[520] > t.env[3000], "decai depois do pico");
    check(t.env[8000] < 0.03f, "volta a ~0 depois do decay");
}

void testSustainHold() {
    Envelope e;
    e.setParameter("mode", 0.0f);       // gated / ASR
    e.setParameter("attack", 0.005f);
    e.setParameter("decay", 0.02f);
    e.setParameter("sustain", 0.5f);
    e.setParameter("release", 0.05f);
    const Trace t = run(e, 40000, 20000);  // gate alto por ~0,42 s

    // no fim do gate, o envelope está segurando o sustain
    check(std::fabs(t.env[18000] - 0.5f) < 0.03f, "segura no sustain com gate alto");
    // depois do gate baixo, cai pra ~0
    check(t.env[38000] < 0.03f, "release leva a ~0 depois do gate baixo");
}

void testVca() {
    Envelope full;
    full.setParameter("mode", 1.0f);
    full.setParameter("attack", 0.01f);
    full.setParameter("decay", 0.05f);
    full.setParameter("release", 0.02f);
    full.setParameter("vca_depth", 1.0f);
    const Trace tf = run(full, 12000, 32, /*audioIn=*/1.0f);
    // out = in * env -> segue o envelope
    check(std::fabs(peak(tf.out) - 1.0f) < 0.02f, "VCA depth=1: pico da saída ~ pico do env");
    check(tf.out[10000] < 0.05f, "VCA depth=1: saída fecha quando o env fecha");

    Envelope bypass;
    bypass.setParameter("vca_depth", 0.0f);
    const Trace tb = run(bypass, 4000, 32, 1.0f);
    bool allUnity = true;
    for (const float v : tb.out)
        if (std::fabs(v - 1.0f) > 1.0e-5f) allUnity = false;
    EXPECT(allUnity);  // depth=0: áudio passa intacto
}

void testCurve() {
    // mesmo tempo de ataque, curva côncava (1) sobe mais rápido no começo
    // que a convexa (0).
    Envelope concave, convex;
    for (Envelope* e : {&concave, &convex}) {
        e->setParameter("mode", 1.0f);
        e->setParameter("attack", 0.05f);  // 2400 amostras
        e->setParameter("decay", 0.5f);
    }
    concave.setParameter("curve", 1.0f);
    convex.setParameter("curve", 0.0f);
    const Trace tcc = run(concave, 4000, 32);
    const Trace tcv = run(convex, 4000, 32);
    // a ~1/4 do ataque
    check(tcc.env[600] > tcv.env[600] + 0.05f,
          "curva côncava sobe mais rápido no início do ataque");
}

void testTimeMod() {
    Envelope base;
    base.setParameter("mode", 1.0f);
    base.setParameter("attack", 0.05f);
    base.setParameter("decay", 0.5f);
    const Trace tb = run(base, 6000, 32);

    // time_mod = 1 -> tempos dobram -> pico chega mais tarde
    Envelope slow;
    slow.setParameter("mode", 1.0f);
    slow.setParameter("attack", 0.05f);
    slow.setParameter("decay", 0.5f);
    slow.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), gate(kSampleRate, 1, kBlock),
        tmod(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &gate, &tmod};
    std::vector<float> env;
    int s = 0;
    while (env.size() < 6000) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            gate.at(0, i) = (s + static_cast<int>(i) < 32) ? 1.0f : 0.0f;
            tmod.at(0, i) = 1.0f;  // x2
        }
        slow.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i) env.push_back(out[1].at(0, i));
        s += static_cast<int>(kBlock);
    }
    // base atinge ~pico em ~2400; slow deve estar claramente abaixo aí
    check(env[2400] < tb.env[2400] - 0.15f, "time_mod estica os tempos");
}

void testLevel() {
    Envelope e;
    e.setParameter("mode", 1.0f);
    e.setParameter("attack", 0.01f);
    e.setParameter("decay", 0.3f);
    e.setParameter("level", 0.5f);
    const Trace t = run(e, 6000, 32);
    check(std::fabs(peak(t.env) - 0.5f) < 0.02f, "level escala o pico do env out");
}

void testDeterminismAndFinite() {
    Envelope a, b;
    for (Envelope* e : {&a, &b}) {
        e->setParameter("attack", 0.02f);
        e->setParameter("decay", 0.1f);
        e->setParameter("sustain", 0.3f);
        e->setParameter("curve", 0.7f);
    }
    const Trace ta = run(a, 20000, 5000, 0.7f);
    const Trace tb = run(b, 20000, 5000, 0.7f);
    bool identical = ta.env.size() == tb.env.size();
    for (std::size_t i = 0; identical && i < ta.env.size(); ++i)
        if (ta.env[i] != tb.env[i] || ta.out[i] != tb.out[i]) identical = false;
    EXPECT(identical);
    for (const float v : ta.env)
        EXPECT(std::isfinite(v) && v >= -1.0e-6f && v <= 1.001f);

    // muitos disparos rápidos, estéreo, sem NaN nem alocação
    Envelope r;
    r.setParameter("attack", 0.001f);
    r.setParameter("decay", 0.005f);
    r.prepare(kSampleRate, 128);
    std::vector<AudioBlock> out(2, AudioBlock(kSampleRate, 2, 128));
    AudioBlock in(kSampleRate, 2, 128), gate(kSampleRate, 2, 128);
    std::vector<const AudioBlock*> ins{&in, &gate, nullptr};
    long sample = 0;
    for (int bk = 0; bk < 4000; ++bk) {
        for (std::size_t i = 0; i < 128; ++i, ++sample) {
            in.at(0, i) = 0.5f;
            gate.at(0, i) = (sample % 500 < 50) ? 1.0f : 0.0f;
        }
        r.process(ins, out);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out[0].at(0, i)));
            EXPECT(std::isfinite(out[1].at(0, i)));
        }
    }
}

void testInGraph() {
    // CLOCK.euclid -> ENVELOPE.gate ; FUNCTION (voz) -> ENVELOPE.in ;
    // ENVELOPE.out -> saída. O ritmo articula a voz.
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 120.0f);
    graph.node(clk).setParameter("mult", 4.0f);
    graph.node(clk).setParameter("fill", 5.0f);
    graph.node(clk).setParameter("gate_len", 0.3f);
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 220.0f);
    const auto env = graph.add(std::make_unique<Envelope>());
    graph.node(env).setParameter("mode", 1.0f);
    graph.node(env).setParameter("attack", 0.005f);
    graph.node(env).setParameter("decay", 0.12f);
    graph.connect(clk, 1, env, 1);       // euclid -> gate
    graph.connect(voice, 1, env, 0);     // bi -> in
    graph.prepare(kSampleRate, 1, kBlock);

    AudioBlock out(kSampleRate, 1, kBlock);
    float loud = 0.0f, quiet = 1.0f;
    for (int b = 0; b < 4000; ++b) {
        graph.process(out, env, 0);
        float blockPeak = 0.0f;
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            blockPeak = std::max(blockPeak, std::fabs(out.at(0, i)));
        }
        loud = std::max(loud, blockPeak);
        quiet = std::min(quiet, blockPeak);
    }
    EXPECT(loud > 0.2f);        // a voz soa nos ataques
    EXPECT(quiet < 0.05f);      // e fecha entre os pulsos (articulação)
}

void testPanel() {
    Envelope e;
    const std::string problem = validatePanel(e);
    check(problem.empty(), "descricao de painel fecha (todo bind resolve)");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    const Panel p = e.panel();
    EXPECT(p.hp > 0);
    EXPECT(p.widgets.size() >= 13);
    std::cout << renderAscii(e);
}

}  // namespace

int main() {
    testADShape();
    testSustainHold();
    testVca();
    testCurve();
    testTimeMod();
    testLevel();
    testDeterminismAndFinite();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular ENVELOPE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
