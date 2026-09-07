// Regressão: o Motion Engine ("a mão caótica", [v]) NUNCA pode fazer a
// saída CLIPAR (|x| >= 1), mesmo com o MASTER no teto do slider (+12 dB)
// e o patch mais denso que o sorteador gera.
//
// O autor reportou clip com o VARIA ligado (seeds 597512815 / 625938148).
// A v3 fechou o loop na direção segura: duck protetor quando a energia
// passa de 0,82 + amplitudes por word-hint + janela relativa. Este teste
// reproduz o caminho do painel — grafo completo, seedPatch, motion.inhabit,
// laço de motion.tick a cada 33 ms — e garante que o limitador do MASTER
// segura em qualquer seed.
//
// Ver `dossies/ESTUDO_seed_composicao_generativa.md §3.7` e
// `feedback_motion_engine_chaotic_hand`.

#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchSeed.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::modular;
using rasgo::panel::MotionEngine;

namespace {

int g_failures = 0;
void check(const bool condition, const std::string& expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

// saída final do grafo (o painel usa uma cópia local idêntica)
struct Out final : Signal {
    Out() : Signal({{"in", PortKind::Audio, ""}},
                   {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "OUT"; }
    void process(const std::vector<const AudioBlock*>& in,
                 std::vector<AudioBlock>& out) noexcept override {
        if (in[0]) out[0].copyFrom(*in[0]);
        else out[0].clear();
    }
};

struct Probe {
    double peak = 0.0;
    long clips = 0;      // |x| >= 1.0
    double rmsMax = 0.0;
};

// monta o grafo como `apps/panel/panel_main.cpp` (1 de cada módulo do
// catálogo + voz-base mínima), aplica o seed, sobe o MASTER na mão,
// habita o Motion Engine e roda `seconds` de áudio.
Probe run(std::uint64_t seed, double seconds, float masterGainDb,
          bool motionOn) {
    const float sr = 48000.0f;
    const std::size_t block = 256;

    SignalGraph graph;
    std::vector<std::size_t> shown;
    std::vector<std::pair<std::string, std::size_t>> byType;
    auto at = [&](const std::string& t) -> std::size_t {
        for (const auto& p : byType) if (p.first == t) return p.second;
        return 0;
    };
    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types) {
            auto n = rasgo::panel::makeModule(t);
            if (!n) continue;
            byType.emplace_back(t, graph.add(std::move(n)));
        }
    for (const auto& p : byType) shown.push_back(p.second);
    const std::size_t sink = graph.add(std::make_unique<Out>());

    graph.node(at("CLOCK")).setParameter("bpm", 96.0f);
    graph.node(at("CLOCK")).setParameter("mult", 2.0f);
    graph.node(at("CLOCK")).setParameter("fill", 7.0f);
    graph.node(at("CLOCK")).setParameter("drift", 0.25f);
    graph.node(at("OSC")).setParameter("freq", 110.0f);
    graph.node(at("OSC")).setParameter("drift", 0.12f);
    graph.node(at("FILTER")).setParameter("cutoff", 420.0f);
    graph.node(at("FILTER")).setParameter("resonance", 0.35f);
    graph.node(at("ENVELOPE")).setParameter("mode", 1.0f);
    graph.node(at("ENVELOPE")).setParameter("attack", 0.006f);
    graph.node(at("ENVELOPE")).setParameter("decay", 0.30f);
    graph.node(at("MIXER")).setParameter("pan1", -0.35f);
    graph.connect(at("CLOCK"), 1, at("ENVELOPE"), 1);
    graph.connect(at("OSC"), 2, at("FILTER"), 0);
    graph.connect(at("FILTER"), 3, at("ENVELOPE"), 0);
    graph.connect(at("ENVELOPE"), 0, at("MIXER"), 0);
    graph.connect(at("MIXER"), 0, at("MASTER"), 0);
    graph.connect(at("MASTER"), 0, sink, 0);
    graph.connect(at("ENVELOPE"), 1, at("FILTER"), 1, true);

    try { rasgo::panel::seedPatch(graph, seed); } catch (...) {}

    // o músico sobe o MASTER na mão (o seed o deixa em −24 dB)
    for (std::size_t n = 0; n < graph.nodeCount(); ++n)
        if (graph.node(n).type() == "MASTER")
            graph.node(n).setParameter("gain", masterGainDb);

    graph.prepare(sr, 2, block);
    graph.setActiveOutput(sink);

    MotionEngine motion;
    motion.inhabit(graph, seed, shown);

    AudioBlock out(sr, 2, block);
    Probe pr;
    const long totalBlocks = static_cast<long>(seconds * sr / block);
    const long ticksEvery =
        static_cast<long>(std::lround(0.033 * sr / block));  // ~6 blocos
    long sinceTick = 0;

    for (long b = 0; b < totalBlocks; ++b) {
        graph.process(out, sink, 0);
        double s2 = 0.0;
        const std::size_t fr = out.frames();
        for (std::size_t c = 0; c < out.channels(); ++c)
            for (std::size_t f = 0; f < fr; ++f) {
                const float v = out.at(c, f);
                const double a = std::fabs(static_cast<double>(v));
                if (a > pr.peak) pr.peak = a;
                if (a >= 1.0) ++pr.clips;
                s2 += static_cast<double>(v) * v;
            }
        const double rms = std::sqrt(s2 / static_cast<double>(fr * out.channels()));
        if (rms > pr.rmsMax) pr.rmsMax = rms;

        if (motionOn && ++sinceTick >= ticksEvery) {
            sinceTick = 0;
            motion.refreshCables(graph);
            const float energy = std::min(1.0f, static_cast<float>(rms) * 2.2f);
            motion.tick(graph, 0.033f, energy);
        }
    }
    return pr;
}

// as duas seeds que o autor reportou clipando + uma varredura curta
const std::uint64_t kSeeds[] = {
    597512815ULL, 625938148ULL, 42ULL, 123456789ULL, 314159265ULL,
};

// Com o MASTER no teto (+12 dB) e o VARIA ligado, o limitador do MASTER
// tem de segurar: zero clips, pico abaixo de 0 dBFS em TODA seed.
void testNoClipAtMaxGainWithMotion() {
    for (const std::uint64_t s : kSeeds) {
        const Probe p = run(s, 6.0, 12.0f, /*motionOn=*/true);
        check(p.clips == 0,
              "seed " + std::to_string(s) + " VARIA ON +12 dB: zero clips (teve "
              + std::to_string(p.clips) + ")");
        check(p.peak < 1.0,
              "seed " + std::to_string(s) + " VARIA ON +12 dB: pico < 1.0 (foi "
              + std::to_string(p.peak) + ")");
    }
}

// Mesmo com um ganho absurdo (+24 dB acima do teto do slider), o
// limitador não deixa passar clip — a rede de segurança da v3.
void testNoClipEvenWayOverGain() {
    for (const std::uint64_t s : {597512815ULL, 625938148ULL}) {
        const Probe p = run(s, 6.0, 36.0f, /*motionOn=*/true);
        check(p.clips == 0,
              "seed " + std::to_string(s) + " VARIA ON +36 dB: zero clips (teve "
              + std::to_string(p.clips) + ")");
    }
}

// Sanidade: no ganho de fábrica (−24 dB) o patch fica bem abaixo do teto,
// com ou sem VARIA — e o VARIA não deve DERRUBAR o som a zero.
void testFactoryGainQuietAndAudible() {
    for (const std::uint64_t s : {625938148ULL, 123456789ULL}) {
        const Probe on = run(s, 5.0, -24.0f, true);
        const Probe off = run(s, 5.0, -24.0f, false);
        check(on.clips == 0 && off.clips == 0,
              "seed " + std::to_string(s) + " −24 dB: zero clips");
        check(on.peak < 0.5,
              "seed " + std::to_string(s) + " −24 dB VARIA ON: pico folgado (< 0.5)");
        check(on.rmsMax > 1e-5 || off.rmsMax > 1e-5,
              "seed " + std::to_string(s) + ": o patch produz som");
    }
}

}  // namespace

int main() {
    testNoClipAtMaxGainWithMotion();
    testNoClipEvenWayOverGain();
    testFactoryGainQuietAndAudible();

    if (g_failures == 0) {
        std::cout << "RASGO Modular motion no-clip tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
