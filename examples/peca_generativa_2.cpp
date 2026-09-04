// Peça generativa 2 do Rasgo Modular — marco 2 (Módulos 1 a 8 + matriz +
// constelação)
// ============================================================================
//
// ~50 s, determinística por seed, sem repetição no tempo. Acrescenta à
// peça 1 os Módulos 7 (MEMORY) e 8 (TURING) e usa a CONSTELAÇÃO: MEMORY
// tem uma posição que oscila no campo; com raio largo, isso é uma
// respiração global suave da mistura (dip de ~30%), não um corte.
//
// Patch:
//
//   CLOCK (88 BPM x2, E(9,16) rot 3, swing, drift, acento 4/6)
//     ├─ euclid ─▶ TURING.clock  e  ENVELOPE.gate
//     └─ accent ─▶ DECISION-timbre.trigger
//
//   TURING (lock 0,72 modulado por DECISION-lock lento; steps 6)
//     ├─ cv  ─▶ VOZ.rate_mod        (melodia cristalizável)
//     └─ cv2 ─▶ FILTRO.spread_mod
//
//   DECISION-lock (0,08 Hz) ─▶ TURING.lock_mod   (o instrumento decide
//                                                 quando travar / soltar)
//   DECISION-timbre           ─▶ FILTRO.cutoff_mod
//
//   VOZ (82 Hz, drift) ─▶[Cable RingMod c/ LFO-anel, conductance 0,9]─▶ FILTRO.in
//   FILTRO.all ─▶ ENVELOPE.in ─▶ ENVELOPE.out ─▶ MEMORY.in
//   FREEZE-LFO (0,05 Hz) ─▶ MEMORY.freeze_gate   (seções suspensas)
//   DECISION-lock.y       ─▶ MEMORY.position_mod
//   MEMORY.out ─▶[Cable rompe 27 s, volta 30,5 s]─▶ saída
//   ENVELOPE.env (bloco anterior) ─▶ param FILTRO.cutoff
//
//   CONSTELAÇÃO: todos os nós na origem; MEMORY tem distância oscilante
//   (~22 s, raio largo) → toda a mistura (entradas e saída) some e volta.
//
// Uso:  peca_generativa_2 [saida.wav]

#include "core/SignalGraph.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Memory.hpp"
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
}  // namespace

int main(int argc, char** argv) {
    const std::string outPath = argc > 1 ? argv[1] : "peca_generativa_2.wav";
    constexpr float sr = 48000.0f;
    constexpr std::size_t block = 128;
    constexpr float seconds = 50.0f;

    SignalGraph graph;

    const auto clock = graph.add(std::make_unique<EuclidClock>());
    graph.node(clock).setParameter("bpm", 88.0f);
    graph.node(clock).setParameter("mult", 2.0f);
    graph.node(clock).setParameter("length", 16.0f);
    graph.node(clock).setParameter("fill", 9.0f);
    graph.node(clock).setParameter("rotate", 3.0f);
    graph.node(clock).setParameter("swing", 0.18f);
    graph.node(clock).setParameter("drift", 0.3f);
    graph.node(clock).setParameter("gate_len", 0.35f);
    graph.node(clock).setParameter("accent_a", 4.0f);
    graph.node(clock).setParameter("accent_b", 6.0f);

    const auto turing = graph.add(std::make_unique<TuringLoop>());
    graph.node(turing).setParameter("lock", 0.72f);
    graph.node(turing).setParameter("length", 6.0f);
    graph.node(turing).setParameter("range", 1.0f);
    graph.node(turing).setParameter("steps", 6.0f);
    graph.node(turing).setParameter("offset", -0.1f);

    const auto lockDec = graph.add(std::make_unique<Decision>());
    graph.node(lockDec).setParameter("rate", 0.08f);
    graph.node(lockDec).setParameter("spread", 0.5f);
    graph.node(lockDec).setParameter("shape", 0.3f);
    graph.node(lockDec).setParameter("slew", 0.5f);

    const auto timbreDec = graph.add(std::make_unique<Decision>());
    graph.node(timbreDec).setParameter("spread", 1.0f);
    graph.node(timbreDec).setParameter("shape", 0.0f);
    graph.node(timbreDec).setParameter("dejavu", 0.3f);
    graph.node(timbreDec).setParameter("slew", 0.35f);

    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 82.0f);
    graph.node(voice).setParameter("slope", 0.62f);
    graph.node(voice).setParameter("drift", 0.2f);

    const auto lfoRing = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoRing).setParameter("rate", 53.0f);

    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 380.0f);
    graph.node(filter).setParameter("resonance", 0.5f);
    graph.node(filter).setParameter("spread", 0.25f);
    graph.node(filter).setParameter("drive", 0.3f);

    const auto env = graph.add(std::make_unique<Envelope>());
    graph.node(env).setParameter("mode", 1.0f);
    graph.node(env).setParameter("attack", 0.006f);
    graph.node(env).setParameter("decay", 0.26f);
    graph.node(env).setParameter("curve", 0.7f);

    const auto freezeLfo = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(freezeLfo).setParameter("rate", 0.05f);
    graph.node(freezeLfo).setParameter("slope", 0.6f);

    const auto mem = graph.add(std::make_unique<Memory>());
    graph.node(mem).setParameter("grain", 0.12f);
    graph.node(mem).setParameter("density", 28.0f);
    graph.node(mem).setParameter("position", 0.3f);
    graph.node(mem).setParameter("spray", 0.25f);
    graph.node(mem).setParameter("pitch", -12.0f);
    graph.node(mem).setParameter("feedback", 0.35f);
    graph.node(mem).setParameter("blend", 0.55f);

    const auto sink = graph.add(std::make_unique<Out>());

    graph.connect(clock, 1, turing, 0);   // euclid -> turing.clock
    graph.connect(clock, 1, env, 1);      // euclid -> env.gate
    graph.connect(clock, 2, timbreDec, 0);  // accent -> timbre trigger

    graph.connect(turing, 0, voice, 0, false, 1.3f);   // cv -> rate_mod
    graph.connect(turing, 1, filter, 3, false, 0.4f);  // cv2 -> spread_mod
    graph.connect(lockDec, 0, turing, 1, false, 0.5f); // x -> lock_mod
    graph.connect(timbreDec, 0, filter, 1, false, 2.5f);  // x -> cutoff_mod

    auto& voiceCable = graph.connect(voice, 1, filter, 0, false, 0.9f);
    voiceCable.setRelation(Relation::RingMod, lfoRing, 1, 0.18f);
    voiceCable.setConductance(0.9f);

    graph.connect(filter, 3, env, 0);        // all -> env.in
    graph.connect(env, 0, mem, 0);           // env.out -> mem.in
    graph.connect(freezeLfo, 0, mem, 3);     // uni -> freeze_gate
    graph.connect(lockDec, 1, mem, 1, false, 0.3f);  // y -> position_mod
    auto& outCable = graph.connect(mem, 0, sink, 0, false, 0.85f);

    // ADITIVO: piso 360 = knob 380 + offset (−20)
    graph.connectToParameter(env, 1, filter, "cutoff", 2400.0f, -20.0f,
                             /*feedback=*/true);

    // Constelação: todos na origem, MEMORY com distância oscilante.
    for (std::size_t id = 0; id < graph.nodeCount(); ++id)
        graph.setNodePosition(id, 0.0f, 0.0f);

    graph.prepare(sr, 1, block);

    std::vector<float> mono;
    mono.reserve(static_cast<std::size_t>(sr * seconds));
    AudioBlock out(sr, 1, block);
    const std::size_t totalBlocks =
        static_cast<std::size_t>(sr * seconds) / block;
    const std::size_t ruptureBlock =
        static_cast<std::size_t>(sr * 27.0f) / block;
    const std::size_t reconnectBlock =
        static_cast<std::size_t>(sr * 30.5f) / block;
    const float blocksPerSecond = sr / block;

    float peak = 0.0f;
    for (std::size_t b = 0; b < totalBlocks; ++b) {
        if (b == ruptureBlock) {
            outCable.rupture();
            std::cout << "  [27 s] ruptura do cabo de saída - cicatriz\n";
        }
        if (b == reconnectBlock) {
            outCable.reconnect();
            std::cout << "  [30,5 s] reconecta\n";
        }
        // constelação: MEMORY respira no campo (~22 s de ciclo). Toda a
        // saída passa por MEMORY, então isto é uma respiração global suave
        // (raio largo) - dip de ~30%, não um corte.
        if (b % 24 == 0) {
            const float t = static_cast<float>(b) / blocksPerSecond;
            const float dist =
                1.2f * (0.5f - 0.5f * std::cos(6.2831853f * t / 22.0f));
            graph.setNodePosition(mem, dist, 0.0f);
            graph.applyConstellation(2.0f);
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
