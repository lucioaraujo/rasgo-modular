// Fragmento de música do Rasgo Modular - Módulos 1 (FUNCTION) e 2
// (FILTER) + a fundação (SignalGraph + Cable com ruptura).
//
// Patch:
//   LFO A (FUNCTION 0,7 Hz, drift 0,6)  -- bi -->  rate_mod da VOZ (±0,9 oit)
//   LFO B (FUNCTION 0,15 Hz)            -- bi -->  cutoff_mod do FILTRO (±2 oit)
//   LFO C (FUNCTION 0,05 Hz)            -- bi -->  spread_mod do FILTRO
//   VOZ  (FUNCTION ~90 Hz, slope 0,72)  -- bi -->  FILTRO in
//   FILTRO (reso 0,55)                  -- all --> Cable --> saída
//
// O `drift` do LFO A + os LFOs lentos B/C dão um percurso generativo que
// nunca se repete no tempo mas é determinístico pela seed. Aos 5 s o Cable
// rompe: a voz não some, a cicatriz segura o último bloco e decai.
//
// Uso:  primeiro_fragmento [saida.wav]

#include "core/SignalGraph.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "io/WavWriter.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::modular;

namespace {
struct Out final : Signal {
    Out() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "OUT"; }
    void process(const std::vector<const AudioBlock*>& in,
                 std::vector<AudioBlock>& out) noexcept override {
        if (in[0]) out[0].copyFrom(*in[0]);
        else out[0].clear();
    }
};
}  // namespace

int main(int argc, char** argv) {
    const std::string outPath = argc > 1 ? argv[1] : "primeiro_fragmento.wav";
    constexpr float sr = 48000.0f;
    constexpr std::size_t block = 128;
    constexpr float seconds = 10.0f;

    SignalGraph graph;

    const auto lfoA = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoA).setParameter("rate", 0.7f);
    graph.node(lfoA).setParameter("drift", 0.6f);

    const auto lfoB = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoB).setParameter("rate", 0.15f);

    const auto lfoC = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoC).setParameter("rate", 0.05f);

    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 90.0f);
    graph.node(voice).setParameter("slope", 0.72f);

    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 500.0f);
    graph.node(filter).setParameter("resonance", 0.55f);

    const auto sink = graph.add(std::make_unique<Out>());

    graph.connect(lfoA, 1, voice, 0, false, 0.9f);   // rate_mod
    graph.connect(lfoB, 1, filter, 1, false, 2.0f);  // cutoff_mod
    graph.connect(lfoC, 1, filter, 3, false, 0.5f);  // spread_mod
    graph.connect(voice, 1, filter, 0);              // voz -> filtro
    auto& cable = graph.connect(filter, 3, sink, 0, false, 0.6f);  // all -> saída

    graph.prepare(sr, 1, block);

    std::vector<float> mono;
    mono.reserve(static_cast<std::size_t>(sr * seconds));
    AudioBlock out(sr, 1, block);
    const std::size_t totalBlocks = static_cast<std::size_t>(sr * seconds) / block;
    const std::size_t ruptureBlock = static_cast<std::size_t>(sr * 5.0f) / block;

    for (std::size_t b = 0; b < totalBlocks; ++b) {
        if (b == ruptureBlock) {
            cable.rupture();
            std::cout << "  [5 s] ruptura do cabo - cicatriz\n";
        }
        graph.process(out, sink, 0);
        for (std::size_t i = 0; i < block; ++i)
            mono.push_back(out.at(0, i));
    }

    if (writeWav16(outPath, mono, static_cast<std::uint32_t>(sr), 1)) {
        std::cout << "  render -> " << outPath << " (" << seconds << " s)\n";
        return 0;
    }
    std::cerr << "falha ao escrever " << outPath << '\n';
    return 1;
}
