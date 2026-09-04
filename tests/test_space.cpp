// Teste isolado do Módulo 10 (SPACE) - antes de entrar num patch.
// Critérios do dossiê `dossies/10_space.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Space.hpp"
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
constexpr std::size_t kBlock = 128;

struct Run {
    std::vector<float> out, wet;
};

// impulseAt < 0: sem impulso. `port` 0 = out, 1 = wet.
template <typename Fn>
Run run(Space& sp, const int samples, Fn input) {
    sp.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> outs(2, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    Run r;
    int s = 0;
    while (static_cast<int>(r.out.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i)
            in.at(0, i) = input(s + static_cast<int>(i));
        sp.process(ins, outs);
        for (std::size_t i = 0; i < kBlock; ++i) {
            r.out.push_back(outs[0].at(0, i));
            r.wet.push_back(outs[1].at(0, i));
        }
        s += static_cast<int>(kBlock);
    }
    return r;
}

std::size_t peakIndex(const std::vector<float>& v, std::size_t a, std::size_t b) {
    std::size_t best = a;
    float bv = 0.0f;
    for (std::size_t i = a; i < b && i < v.size(); ++i)
        if (std::fabs(v[i]) > bv) { bv = std::fabs(v[i]); best = i; }
    return best;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * v[i];
    return std::sqrt(s / static_cast<double>(b - a));
}

float noise(int i) {
    std::uint32_t x = static_cast<std::uint32_t>(i) * 2654435761u + 999u;
    x ^= x >> 13; x *= 0x5bd1e995u; x ^= x >> 15;
    return static_cast<float>(x & 0xFFFF) / 32768.0f - 1.0f;
}

void testEchoDelay() {
    Space sp;
    sp.setParameter("time", 0.05f);       // 2400 amostras
    sp.setParameter("taps", 1.0f);
    sp.setParameter("spread", 0.0f);
    sp.setParameter("feedback", 0.0f);
    sp.setParameter("diffusion", 0.0f);
    sp.setParameter("mod", 0.0f);
    const Run r = run(sp, 8000, [](int i) { return i == 0 ? 1.0f : 0.0f; });
    const std::size_t p = peakIndex(r.wet, 100, 6000);
    check(std::abs(static_cast<int>(p) - 2400) < 40,
          "primeira tomada chega em ~time*sr amostras");
}

void testFeedbackRepeats() {
    Space sp;
    sp.setParameter("time", 0.03f);   // 1440
    sp.setParameter("taps", 1.0f);
    sp.setParameter("spread", 0.0f);
    sp.setParameter("feedback", 0.7f);
    sp.setParameter("diffusion", 0.0f);
    sp.setParameter("mod", 0.0f);
    const Run r = run(sp, 12000, [](int i) { return i == 0 ? 1.0f : 0.0f; });
    // deve haver energia em vários múltiplos do atraso (ecos que se repetem)
    const double e1 = rms(r.wet, 1300, 1600);
    const double e3 = rms(r.wet, 4200, 4500);
    const double e6 = rms(r.wet, 8500, 8800);
    check(e1 > 0.0 && e3 > 0.0 && e6 > 0.0, "os ecos se repetem");
    check(e3 < e1 && e6 < e3, "e decaem com a realimentação");
}

void testMixDryExact() {
    Space sp;
    sp.setParameter("mix", 0.0f);
    sp.setParameter("feedback", 0.6f);
    const Run r = run(sp, 6000, [](int i) { return 0.5f * noise(i); });
    bool exact = true;
    for (std::size_t i = 0; i < r.out.size(); ++i)
        if (std::fabs(r.out[i] - 0.5f * noise(static_cast<int>(i))) > 1.0e-6f)
            exact = false;
    EXPECT(exact);  // mix=0 -> porta `out` == entrada
}

void testStabilityHighFeedback() {
    Space sp;
    sp.setParameter("time", 0.2f);
    sp.setParameter("feedback", 0.95f);
    sp.setParameter("diffusion", 0.8f);
    sp.setParameter("mod", 0.5f);
    sp.setParameter("tone", 0.7f);
    const Run r = run(sp, 300000, [](int i) {
        return i < 4800 ? 0.6f * noise(i) : 0.0f;  // um sopro no começo
    });
    for (const float v : r.out)
        EXPECT(std::isfinite(v) && std::fabs(v) <= 1.5f);
    // a cauda deve decair, não crescer
    const double early = rms(r.out, 10000, 40000);
    const double late = rms(r.out, 250000, 290000);
    check(late <= early + 0.05, "a cauda não cresce (realimentação estável)");
}

void testToneAffectsTail() {
    Space bright, dark;
    for (Space* sp : {&bright, &dark}) {
        sp->setParameter("time", 0.04f);
        sp->setParameter("feedback", 0.8f);
        sp->setParameter("mod", 0.0f);
    }
    bright.setParameter("tone", 1.0f);
    dark.setParameter("tone", 0.0f);
    const Run rb = run(bright, 40000, [](int i) { return i < 2400 ? noise(i) : 0.0f; });
    const Run rd = run(dark, 40000, [](int i) { return i < 2400 ? noise(i) : 0.0f; });
    // medida grosseira de brilho: energia de alta frequência (diferença
    // de amostras vizinhas) na cauda
    auto hf = [](const std::vector<float>& v) {
        double s = 0.0;
        for (std::size_t i = 20001; i < 38000 && i < v.size(); ++i)
            s += (v[i] - v[i - 1]) * (v[i] - v[i - 1]);
        return s;
    };
    check(hf(rb.wet) > hf(rd.wet) * 1.3, "tone alto mantém mais agudo na cauda");
}

void testDeterminism() {
    Space a, b;
    for (Space* sp : {&a, &b}) {
        sp->setParameter("feedback", 0.6f);
        sp->setParameter("diffusion", 0.5f);
        sp->setParameter("mod", 0.3f);
    }
    const Run ra = run(a, 40000, [](int i) { return 0.4f * noise(i); });
    const Run rb = run(b, 40000, [](int i) { return 0.4f * noise(i); });
    bool identical = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; identical && i < ra.out.size(); ++i)
        if (ra.out[i] != rb.out[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    SignalGraph graph;
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 160.0f);
    const auto space = graph.add(std::make_unique<Space>());
    graph.node(space).setParameter("feedback", 0.5f);
    graph.node(space).setParameter("mix", 0.5f);
    graph.connect(voice, 1, space, 0);
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, space, 0);
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    Space sp;
    const std::string problem = validatePanel(sp);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(sp.panel().widgets.size() >= 13);
    std::cout << renderAscii(sp);
}

}  // namespace

int main() {
    testEchoDelay();
    testFeedbackRepeats();
    testMixDryExact();
    testStabilityHighFeedback();
    testToneAffectsTail();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SPACE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
