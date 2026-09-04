// Peça generativa do Rasgo Modular — marco 1 (Módulos 1 a 6 + fundação)
// ============================================================================
//
// Uma peça de ~40 s, DETERMINÍSTICA pela seed (os rng de cada módulo são
// semeados em prepare()), que NÃO se repete no tempo: a deriva do clock,
// o déjà-vu parcial do DECISION e os LFOs lentos com drift garantem um
// percurso sempre diferente sobre a mesma estrutura.
//
// Patch:
//
//   CLOCK (96 BPM, x2, E(7,16) rot 2, swing, drift, acento 4/6 OR)
//     ├─ euclid  ─▶ DECISION-altura .trigger
//     ├─ euclid  ─▶ ENVELOPE .gate
//     └─ accent  ─▶ DECISION-timbre .trigger
//
//   DECISION-altura (spread .7, shape .4, steps 5, slew .15, déjà-vu .6/6)
//     ├─ x ─▶ VOZ .rate_mod         (±1,4 oitava, quantizado -> melodia)
//     └─ y ─▶ FILTRO .spread_mod
//
//   DECISION-timbre (spread 1, shape 0, déjà-vu .3)
//     └─ x ─▶ FILTRO .cutoff_mod    (±2,5 oitava)
//
//   VOZ (FUNCTION 110 Hz, slope .6, drift .2)
//     └─ bi ─▶[Cable: RingMod com LFO-anel .2, conductance .9]─▶ FILTRO .in
//
//   LFO-espalha (FUNCTION 0,04 Hz) ─▶ param FILTRO.spread   (seções lentas)
//   ENVELOPE.env                    ─▶ param FILTRO.cutoff   (o contorno abre o filtro)
//
//   FILTRO (SVF 3 irmãs, reso .55, drive .3)
//     └─ all ─▶ ENVELOPE .in ─▶ out ─▶[Cable rompe aos 28 s, volta aos 34 s]─▶ saída
//
// Uso:  peca_generativa [saida.wav]

#include "core/SignalGraph.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
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
    const std::string outPath = argc > 1 ? argv[1] : "peca_generativa.wav";
    constexpr float sr = 48000.0f;
    constexpr std::size_t block = 128;
    constexpr float seconds = 40.0f;

    SignalGraph graph;

    const auto clock = graph.add(std::make_unique<EuclidClock>());
    graph.node(clock).setParameter("bpm", 96.0f);
    graph.node(clock).setParameter("mult", 2.0f);
    graph.node(clock).setParameter("length", 16.0f);
    graph.node(clock).setParameter("fill", 7.0f);
    graph.node(clock).setParameter("rotate", 2.0f);
    graph.node(clock).setParameter("swing", 0.22f);
    graph.node(clock).setParameter("drift", 0.35f);
    graph.node(clock).setParameter("gate_len", 0.35f);
    graph.node(clock).setParameter("accent_a", 4.0f);
    graph.node(clock).setParameter("accent_b", 6.0f);

    const auto pitchDec = graph.add(std::make_unique<Decision>());
    graph.node(pitchDec).setParameter("spread", 0.7f);
    graph.node(pitchDec).setParameter("shape", 0.4f);
    graph.node(pitchDec).setParameter("steps", 5.0f);
    graph.node(pitchDec).setParameter("slew", 0.15f);
    graph.node(pitchDec).setParameter("dejavu", 0.6f);
    graph.node(pitchDec).setParameter("loop_length", 6.0f);

    const auto timbreDec = graph.add(std::make_unique<Decision>());
    graph.node(timbreDec).setParameter("spread", 1.0f);
    graph.node(timbreDec).setParameter("shape", 0.0f);
    graph.node(timbreDec).setParameter("dejavu", 0.3f);
    graph.node(timbreDec).setParameter("slew", 0.4f);

    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 110.0f);
    graph.node(voice).setParameter("slope", 0.6f);
    graph.node(voice).setParameter("drift", 0.2f);

    const auto lfoRing = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoRing).setParameter("rate", 47.0f);

    const auto lfoSpread = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoSpread).setParameter("rate", 0.04f);

    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 420.0f);
    graph.node(filter).setParameter("resonance", 0.55f);
    graph.node(filter).setParameter("spread", 0.2f);
    graph.node(filter).setParameter("drive", 0.3f);

    const auto env = graph.add(std::make_unique<Envelope>());
    graph.node(env).setParameter("mode", 1.0f);       // AD / trigger
    graph.node(env).setParameter("attack", 0.008f);
    graph.node(env).setParameter("decay", 0.24f);
    graph.node(env).setParameter("curve", 0.72f);

    const auto sink = graph.add(std::make_unique<Out>());

    // clock -> decisões e envelope
    graph.connect(clock, 1, pitchDec, 0);   // euclid -> trigger
    graph.connect(clock, 1, env, 1);        // euclid -> gate
    graph.connect(clock, 2, timbreDec, 0);  // accent -> trigger

    // decisões -> parâmetros/portas
    graph.connect(pitchDec, 0, voice, 0, false, 1.4f);   // x -> rate_mod (melodia)
    graph.connect(pitchDec, 1, filter, 3, false, 0.5f);  // y -> spread_mod
    graph.connect(timbreDec, 0, filter, 1, false, 2.5f); // x -> cutoff_mod

    // voz -> filtro, com RELAÇÃO de cabo (RingMod, Módulo 3) e condução <1
    auto& voiceCable = graph.connect(voice, 1, filter, 0, false, 0.9f);
    voiceCable.setRelation(Relation::RingMod, lfoRing, 1, 0.2f);
    voiceCable.setConductance(0.9f);

    // modulação lenta de seção + o contorno abre o filtro
    // modulação ADITIVA sobre o knob: offset = piso desejado (0,25) − knob (0,2)
    graph.connectToParameter(lfoSpread, 1, filter, "spread", 0.35f, 0.25f - 0.2f);
    // o contorno do bloco ANTERIOR abre o filtro (feedback de 1 bloco -
    // evita o ciclo filter->env->filter e é lento o bastante pra isso não
    // se ouvir)
    // ADITIVO: piso 380 = knob 420 + offset (−40)
    graph.connectToParameter(env, 1, filter, "cutoff", 2600.0f, -40.0f,
                             /*feedback=*/true);

    // filtro -> envelope -> saída (cabo rompível)
    graph.connect(filter, 3, env, 0);       // all -> in
    auto& outCable = graph.connect(env, 0, sink, 0, false, 0.8f);

    graph.prepare(sr, 1, block);

    std::vector<float> mono;
    mono.reserve(static_cast<std::size_t>(sr * seconds));
    AudioBlock out(sr, 1, block);
    const std::size_t totalBlocks =
        static_cast<std::size_t>(sr * seconds) / block;
    const std::size_t ruptureBlock =
        static_cast<std::size_t>(sr * 28.0f) / block;
    const std::size_t reconnectBlock =
        static_cast<std::size_t>(sr * 34.0f) / block;

    float peak = 0.0f;
    for (std::size_t b = 0; b < totalBlocks; ++b) {
        if (b == ruptureBlock) {
            outCable.rupture();
            std::cout << "  [28 s] ruptura do cabo de saída - cicatriz\n";
        }
        if (b == reconnectBlock) {
            outCable.reconnect();
            std::cout << "  [34 s] reconecta\n";
        }
        graph.process(out, sink, 0);
        for (std::size_t i = 0; i < block; ++i) {
            const float v = out.at(0, i);
            mono.push_back(v);
            if (std::fabs(v) > peak) peak = std::fabs(v);
        }
    }

    // normaliza suave pra ~-1 dBFS (a peça é determinística; o ganho não
    // muda a forma)
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
