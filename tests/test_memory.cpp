// Teste isolado do Módulo 7 (MEMORY) - antes de entrar num patch.
// Critérios do dossiê `dossies/07_memory.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Memory.hpp"
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

// Roda `samples` amostras. `input(i)` fornece o sinal de entrada;
// `frozenAfter` >= 0 liga o parâmetro freeze a partir dessa amostra.
struct Run {
    std::vector<float> out;
};

template <typename Fn>
Run run(Memory& m, const int samples, Fn input, const int frozenAfter = -1) {
    m.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> outs(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr, nullptr};
    Run r;
    int s = 0;
    while (static_cast<int>(r.out.size()) < samples) {
        if (frozenAfter >= 0 && s >= frozenAfter)
            m.setParameter("freeze", 1.0f);
        for (std::size_t i = 0; i < kBlock; ++i)
            in.at(0, i) = input(s + static_cast<int>(i));
        m.process(ins, outs);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.out.push_back(outs[0].at(0, i));
        s += static_cast<int>(kBlock);
    }
    return r;
}

double rms(const std::vector<float>& v, const std::size_t a, const std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * v[i];
    return std::sqrt(s / static_cast<double>(b - a));
}

float noise(int i) {
    std::uint32_t x = static_cast<std::uint32_t>(i) * 2654435761u + 12345u;
    x ^= x >> 13; x *= 0x5bd1e995u; x ^= x >> 15;
    return static_cast<float>(x & 0xFFFF) / 32768.0f - 1.0f;
}

void testCaptureAndPlayback() {
    Memory m;
    m.setParameter("blend", 1.0f);
    m.setParameter("density", 40.0f);
    const Run r = run(m, 60000, [](int i) { return 0.6f * noise(i); });
    check(rms(r.out, 20000, 60000) > 0.02f, "MEMORY produz textura granular do que entrou");
    for (const float v : r.out)
        EXPECT(std::isfinite(v) && std::fabs(v) < 4.0f);
}

void testFreezeHolds() {
    Memory m;
    m.setParameter("blend", 1.0f);
    m.setParameter("density", 40.0f);
    // 1 s de ruído; depois congela e a entrada vai a zero por 1 s
    const Run r = run(m, 96000,
                      [](int i) { return i < 48000 ? 0.6f * noise(i) : 0.0f; },
                      /*frozenAfter=*/48000);
    const double before = rms(r.out, 30000, 47000);
    const double after = rms(r.out, 60000, 95000);
    check(before > 0.02f, "há som antes do freeze");
    check(after > 0.4 * before,
          "freeze segura a textura mesmo com entrada em silêncio");
}

void testBlend() {
    Memory dryOnly;
    dryOnly.setParameter("blend", 0.0f);
    const Run rd = run(dryOnly, 20000, [](int i) { return 0.5f * noise(i); });
    bool exactDry = true;
    for (std::size_t i = 0; i < rd.out.size(); ++i)
        if (std::fabs(rd.out[i] - 0.5f * noise(static_cast<int>(i))) > 1.0e-6f)
            exactDry = false;
    EXPECT(exactDry);  // blend=0 -> saída == entrada
}

void testPitchStable() {
    for (const float st : {-12.0f, -5.0f, 7.0f, 19.0f}) {
        Memory m;
        m.setParameter("blend", 1.0f);
        m.setParameter("pitch", st);
        const Run r = run(m, 40000, [](int i) { return 0.5f * noise(i); });
        bool ok = true;
        for (const float v : r.out)
            if (!std::isfinite(v) || std::fabs(v) > 4.0f) ok = false;
        check(ok, "pitch shift sem NaN nem estouro");
    }
}

void testDeterminism() {
    Memory a, b;
    for (Memory* m : {&a, &b}) {
        m->setParameter("blend", 0.8f);
        m->setParameter("spray", 0.5f);
        m->setParameter("feedback", 0.3f);
        m->setParameter("pitch", 3.0f);
    }
    const Run ra = run(a, 40000, [](int i) { return 0.4f * noise(i); });
    const Run rb = run(b, 40000, [](int i) { return 0.4f * noise(i); });
    bool identical = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; identical && i < ra.out.size(); ++i)
        if (ra.out[i] != rb.out[i]) identical = false;
    EXPECT(identical);
}

void testNoAllocFinite() {
    Memory m;
    m.setParameter("blend", 0.7f);
    m.setParameter("feedback", 0.6f);
    m.setParameter("density", 90.0f);
    m.prepare(kSampleRate, 256);
    std::vector<AudioBlock> outs(1, AudioBlock(kSampleRate, 2, 256));
    AudioBlock in(kSampleRate, 2, 256), fz(kSampleRate, 2, 256);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr, &fz};
    for (int b = 0; b < 3000; ++b) {
        for (std::size_t i = 0; i < 256; ++i)
            in.at(0, i) = 0.5f * noise(b * 256 + static_cast<int>(i));
        fz.at(0, 0) = (b % 200 < 60) ? 1.0f : 0.0f;
        m.process(ins, outs);
        for (std::size_t i = 0; i < 256; ++i)
            EXPECT(std::isfinite(outs[0].at(0, i)));
    }
}

void testInGraph() {
    SignalGraph graph;
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 180.0f);
    const auto mem = graph.add(std::make_unique<Memory>());
    graph.node(mem).setParameter("blend", 0.9f);
    graph.node(mem).setParameter("pitch", 7.0f);
    graph.connect(voice, 1, mem, 0);
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, mem, 0);
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    Memory m;
    const std::string problem = validatePanel(m);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(m.panel().widgets.size() >= 13);
    std::cout << renderAscii(m);
}

}  // namespace

int main() {
    testCaptureAndPlayback();
    testFreezeHolds();
    testBlend();
    testPitchStable();
    testDeterminism();
    testNoAllocFinite();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular MEMORY tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
