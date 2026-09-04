// Peça generativa 3 do Rasgo Modular — marco 2 (Módulos físicos + escala +
// EQ + barramento semântico)
// ============================================================================
//
// ~55 s, determinística por seed. Uma melodia de corda numa escala
// (TURING → QUANTIZER → STRING) sobre um baixo percussivo modal
// (MATTER), passada por um EQ paramétrico e um espaço. Demonstra o
// BARRAMENTO SEMÂNTICO: o movimento da melodia (qualidade "Motion")
// controla a realimentação do SPACE; a energia do ritmo (qualidade
// "Energy") controla o realce de presença do EQ. Nenhum cabo pra isso.
//
// Patch:
//   CLOCK (84 BPM x2, E(9,16), swing, drift, acento 3/5)
//     ├─ euclid ─▶ TURING.clock, QUANTIZER.trigger, STRING.pluck
//     └─ accent ─▶ MATTER.strike
//   TURING.cv ─▶ QUANTIZER.cv          (déjà-vu 0,7 → riff cristalizável)
//   QUANTIZER.pitch (oitavas) ─▶ STRING.freq_mod   (Eólio, tônica ré)
//   STRING.out + MATTER.out ─▶ [MIX] ─▶ PARAMETRIC.in ─▶ SPACE.in ─▶ saída
//   LFO (0,05 Hz) ─▶ PARAMETRIC.sweep
//   [Cable rompe aos 31 s, volta aos 35 s] no PARAMETRIC → SPACE
//
//   BARRAMENTO SEMÂNTICO:
//     TURING.cv     ─contribui▶ Motion  (peso 2)
//     CLOCK.euclid  ─contribui▶ Energy  (peso 1)
//     Motion  ─segue▶ SPACE.feedback    (0,12 + 0,45·Motion)
//     Energy  ─segue▶ PARAMETRIC.gain3  (0 + 5·Energy dB)
//
// Uso:  peca_generativa_3 [saida.wav]

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Matter.hpp"
#include "dsp/Parametric.hpp"
#include "dsp/Quantizer.hpp"
#include "dsp/Space.hpp"
#include "dsp/StringVoice.hpp"
#include "dsp/TuringLoop.hpp"
#include "io/WavWriter.hpp"

#include <cmath>
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
struct Mix2 final : Signal {
    Mix2() : Signal({{"a", PortKind::Audio, ""}, {"b", PortKind::Audio, ""}},
                    {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "MIX2"; }
    void process(const std::vector<const AudioBlock*>& in,
                 std::vector<AudioBlock>& out) noexcept override {
        const std::size_t f = out[0].frames();
        for (std::size_t i = 0; i < f; ++i) {
            const float a = in[0] ? in[0]->at(0, i) : 0.0f;
            const float b = in[1] ? in[1]->at(0, i) : 0.0f;
            for (std::size_t c = 0; c < out[0].channels(); ++c)
                out[0].at(c, i) = a + b;
        }
    }
};
}  // namespace

int main(int argc, char** argv) {
    const std::string outPath = argc > 1 ? argv[1] : "peca_generativa_3.wav";
    constexpr float sr = 48000.0f;
    constexpr std::size_t block = 128;
    constexpr float seconds = 55.0f;

    SignalGraph graph;

    const auto clock = graph.add(std::make_unique<EuclidClock>());
    graph.node(clock).setParameter("bpm", 84.0f);
    graph.node(clock).setParameter("mult", 2.0f);
    graph.node(clock).setParameter("length", 16.0f);
    graph.node(clock).setParameter("fill", 9.0f);
    graph.node(clock).setParameter("rotate", 2.0f);
    graph.node(clock).setParameter("swing", 0.2f);
    graph.node(clock).setParameter("drift", 0.3f);
    graph.node(clock).setParameter("gate_len", 0.3f);
    graph.node(clock).setParameter("accent_a", 3.0f);
    graph.node(clock).setParameter("accent_b", 5.0f);

    const auto turing = graph.add(std::make_unique<TuringLoop>());
    graph.node(turing).setParameter("lock", 0.7f);
    graph.node(turing).setParameter("length", 7.0f);
    graph.node(turing).setParameter("range", 1.0f);
    graph.node(turing).setParameter("steps", 1.0f);

    const auto quant = graph.add(std::make_unique<Quantizer>());
    graph.node(quant).setParameter("scale", 2.0f);   // Eólio
    graph.node(quant).setParameter("root", 2.0f);    // ré
    graph.node(quant).setParameter("range", 2.0f);
    graph.node(quant).setParameter("glide", 0.06f);
    graph.node(quant).setParameter("hysteresis", 0.25f);

    const auto str = graph.add(std::make_unique<StringVoice>());
    graph.node(str).setParameter("freq", 220.0f);
    graph.node(str).setParameter("decay", 0.78f);
    graph.node(str).setParameter("damping", 0.42f);
    graph.node(str).setParameter("position", 0.16f);
    graph.node(str).setParameter("exciter", 0.6f);

    const auto matter = graph.add(std::make_unique<Matter>());
    graph.node(matter).setParameter("freq", 66.0f);
    graph.node(matter).setParameter("structure", 0.15f);
    graph.node(matter).setParameter("brightness", 0.45f);
    graph.node(matter).setParameter("damping", 0.5f);
    graph.node(matter).setParameter("position", 0.2f);
    graph.node(matter).setParameter("exciter", 0.6f);

    const auto lfo = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfo).setParameter("rate", 0.05f);

    const auto mix = graph.add(std::make_unique<Mix2>());

    const auto eq = graph.add(std::make_unique<Parametric>());
    graph.node(eq).setParameter("type1", 1.0f);   // LowCut
    graph.node(eq).setParameter("freq1", 45.0f);
    graph.node(eq).setParameter("type2", 3.0f);   // Peak -
    graph.node(eq).setParameter("freq2", 260.0f);
    graph.node(eq).setParameter("gain2", -3.0f);
    graph.node(eq).setParameter("q2", 1.4f);
    graph.node(eq).setParameter("type3", 3.0f);   // Peak + (segue Energy)
    graph.node(eq).setParameter("freq3", 2600.0f);
    graph.node(eq).setParameter("q3", 1.2f);
    graph.node(eq).setParameter("type4", 4.0f);   // HighShelf
    graph.node(eq).setParameter("freq4", 8000.0f);
    graph.node(eq).setParameter("gain4", 2.0f);
    graph.node(eq).setParameter("output", -2.0f);

    const auto space = graph.add(std::make_unique<Space>());
    graph.node(space).setParameter("time", 0.33f);
    graph.node(space).setParameter("taps", 4.0f);
    graph.node(space).setParameter("spread", 0.7f);
    graph.node(space).setParameter("diffusion", 0.55f);
    graph.node(space).setParameter("tone", 0.55f);
    graph.node(space).setParameter("mix", 0.4f);
    graph.node(space).setParameter("feedback", 0.12f);  // knob = piso do follower (aditivo, offset 0)

    const auto sink = graph.add(std::make_unique<Out>());

    graph.connect(clock, 1, turing, 0);   // euclid -> turing.clock
    graph.connect(clock, 1, quant, 2);    // euclid -> quantizer.trigger
    graph.connect(clock, 1, str, 1);      // euclid -> string.pluck
    graph.connect(clock, 2, matter, 1);   // accent -> matter.strike
    graph.connect(turing, 0, quant, 0);   // cv -> quantizer.cv
    graph.connect(quant, 0, str, 2);      // pitch (oitavas) -> string.freq_mod
    graph.connect(str, 0, mix, 0);        // string -> mix.a
    graph.connect(matter, 0, mix, 1, false, 0.8f);  // matter -> mix.b
    graph.connect(mix, 0, eq, 0);         // mix -> eq.in
    graph.connect(lfo, 1, eq, 1, false, 0.5f);  // lfo -> eq.sweep
    auto& eqCable = graph.connect(eq, 0, space, 0);  // eq -> space.in
    graph.connect(space, 0, sink, 0, false, 0.9f);   // space -> saída

    // barramento semântico
    graph.contributeQuality(turing, 0, SignalGraph::Quality::Motion, 2.0f);
    graph.contributeQuality(clock, 1, SignalGraph::Quality::Energy, 1.0f);
    graph.followQuality(SignalGraph::Quality::Motion, space, "feedback",
                        0.45f, 0.0f);
    graph.followQuality(SignalGraph::Quality::Energy, eq, "gain3", 5.0f, 0.0f);  // gain3 knob = 0 dB

    graph.prepare(sr, 1, block);

    std::vector<float> mono;
    mono.reserve(static_cast<std::size_t>(sr * seconds));
    AudioBlock out(sr, 1, block);
    const std::size_t totalBlocks =
        static_cast<std::size_t>(sr * seconds) / block;
    const std::size_t ruptureBlock =
        static_cast<std::size_t>(sr * 31.0f) / block;
    const std::size_t reconnectBlock =
        static_cast<std::size_t>(sr * 35.0f) / block;

    float peak = 0.0f;
    for (std::size_t b = 0; b < totalBlocks; ++b) {
        if (b == ruptureBlock) {
            eqCable.rupture();
            std::cout << "  [31 s] ruptura EQ->espaço - cicatriz\n";
        }
        if (b == reconnectBlock) {
            eqCable.reconnect();
            std::cout << "  [35 s] reconecta\n";
        }
        graph.process(out, sink, 0);
        for (std::size_t i = 0; i < block; ++i) {
            const float v = out.at(0, i);
            mono.push_back(v);
            if (std::fabs(v) > peak) peak = std::fabs(v);
        }
    }

    if (peak > 1.0e-6f) {
        const float g = 0.89f / peak;
        for (float& v : mono) v *= g;
    }

    if (writeWav16(outPath, mono, static_cast<std::uint32_t>(sr), 1)) {
        std::cout << "  render -> " << outPath << " (" << seconds
                  << " s, pico bruto " << peak << ")\n";
        return 0;
    }
    std::cerr << "falha ao escrever " << outPath << '\n';
    return 1;
}
