// Peça generativa 4 do Rasgo Modular — protótipo do Motion Engine
// ============================================================================
//
// Primeira peça com uma camada de COMPOSIÇÃO acima do patch — o "menor
// passo testável" do estudo
// `dossies/ESTUDO_seed_composicao_generativa.md §3.5`: um `MotionEngine`
// (`apps/panel/MotionEngine.hpp`) move parâmetros no tempo, por CIMA do
// patch estático e das modulações por cabo já existentes, sem apagá-las
// — graças ao motor aditivo (`setParameterBase`, `connectToParameter`
// aditivo, 2026-09-04).
//
// Patch (como `peca_generativa`, simplificado — o foco aqui é o motion):
//
//   CLOCK (88 BPM, E(7,16))
//     ├─ euclid ─▶ DECISION.trigger
//     └─ euclid ─▶ ENVELOPE.gate
//
//   DECISION.x ─▶ VOZ.rate_mod (melodia quantizada pela oitava)
//   VOZ.bi ─▶[Cable RingMod com LFO-anel]─▶ FILTRO.in
//   LFO-espalha ─▶ FILTRO.spread (cabo — como nas peças anteriores)
//   FILTRO.all ─▶ ENVELOPE.in ─▶ saída
//
// Motion Engine (roda 1×/bloco, fora do áudio, escreve via
// `setParameterBase` — nunca dentro de `process()`):
//
//   WALK       filter.resonance   — deriva lenta COM ALVO (rearmado por
//                                    probabilidade, desliza até lá) —
//                                    diferente do `drift` cego de
//                                    módulo: tem destino, não só treme.
//   OSCILLATE  env.curve          — a forma do envelope "respira"
//                                    devagar ao longo da peça.
//   ATTRACT    voice.slope        — persegue `filter.resonance` (outro
//                                    módulo, outro parâmetro) — a
//                                    "coreografia paramétrica" descrita
//                                    no estudo.
//   WALK       filter.spread      — soma-se por CIMA da modulação já
//                                    existente do LFO-espalha (cabo,
//                                    `connectToParameter`) — prova viva
//                                    da aditividade: as duas escritas no
//                                    mesmo parâmetro convivem, nenhuma
//                                    apaga a outra.
//
// Determinístico: cada `Binding` do Motion Engine carrega sua própria
// seed xorshift; `dt` é sempre `block/sr` (nunca relógio de parede).
//
// Também escreve a SYSTEM SCORE (`apps/panel/ScoreRecorder.hpp`,
// `dossies/ESTUDO_seed_composicao_generativa.md §5`) — um registro de
// texto das conexões do patch e das mudanças de parâmetro que o Motion
// Engine faz ao vivo, com `t` sempre `amostra/sr` (nunca relógio de
// parede — o registro é tão determinístico quanto o áudio).
//
// Uso: peca_generativa_4 [saida.wav]

#include "core/SignalGraph.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "io/WavWriter.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/ScoreRecorder.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::modular;
using rasgo::panel::MotionEngine;
using rasgo::panel::ScoreRecorder;

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
    const std::string outPath = argc > 1 ? argv[1] : "peca_generativa_4.wav";
    constexpr float sr = 48000.0f;
    constexpr std::size_t block = 128;
    constexpr float seconds = 42.0f;

    SignalGraph graph;

    const auto clock = graph.add(std::make_unique<EuclidClock>());
    graph.node(clock).setParameter("bpm", 88.0f);
    graph.node(clock).setParameter("mult", 2.0f);
    graph.node(clock).setParameter("length", 16.0f);
    graph.node(clock).setParameter("fill", 7.0f);
    graph.node(clock).setParameter("rotate", 3.0f);
    graph.node(clock).setParameter("swing", 0.18f);
    graph.node(clock).setParameter("drift", 0.25f);
    graph.node(clock).setParameter("gate_len", 0.35f);

    const auto pitchDec = graph.add(std::make_unique<Decision>());
    graph.node(pitchDec).setParameter("spread", 0.65f);
    graph.node(pitchDec).setParameter("shape", 0.35f);
    graph.node(pitchDec).setParameter("steps", 5.0f);
    graph.node(pitchDec).setParameter("slew", 0.15f);
    graph.node(pitchDec).setParameter("dejavu", 0.5f);
    graph.node(pitchDec).setParameter("loop_length", 6.0f);

    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 96.0f);
    graph.node(voice).setParameter("slope", 0.5f);
    graph.node(voice).setParameter("drift", 0.15f);

    const auto lfoRing = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoRing).setParameter("rate", 51.0f);

    const auto lfoSpread = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfoSpread).setParameter("rate", 0.045f);

    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 950.0f);
    graph.node(filter).setParameter("resonance", 0.3f);   // base do Motion (WALK)
    graph.node(filter).setParameter("spread", 0.15f);      // base do Motion (WALK)
    graph.node(filter).setParameter("drive", 0.25f);

    const auto env = graph.add(std::make_unique<Envelope>());
    graph.node(env).setParameter("mode", 1.0f);   // AD / trigger
    graph.node(env).setParameter("attack", 0.01f);
    graph.node(env).setParameter("decay", 0.28f);
    graph.node(env).setParameter("curve", 0.55f);  // base do Motion (OSCILLATE)

    const auto sink = graph.add(std::make_unique<Out>());

    graph.connect(clock, 1, pitchDec, 0);   // euclid -> trigger
    graph.connect(clock, 1, env, 1);        // euclid -> gate
    graph.connect(pitchDec, 0, voice, 0, false, 1.3f);   // x -> rate_mod

    auto& voiceCable = graph.connect(voice, 1, filter, 0, false, 0.9f);
    voiceCable.setRelation(Relation::RingMod, lfoRing, 1, 0.18f);
    voiceCable.setConductance(0.9f);

    // modulação de seção já existente por CABO — o Motion Engine vai
    // somar EM CIMA da mesma base (`spread`), aditivamente
    graph.connectToParameter(lfoSpread, 1, filter, "spread", 0.3f, 0.0f);

    graph.connect(filter, 3, env, 0);   // all -> in
    graph.connect(env, 0, sink, 0, false, 0.85f);

    graph.prepare(sr, 1, block);

    // ---- SYSTEM SCORE: registra a topologia inicial (t=0) --------------
    ScoreRecorder score;
    score.connection(0.0, clock, 1, pitchDec, 0);
    score.connection(0.0, clock, 1, env, 1);
    score.connection(0.0, pitchDec, 0, voice, 0);
    score.connection(0.0, voice, 1, filter, 0);
    score.modulation(0.0, lfoSpread, 1, filter, "spread", 0.3f, 0.0f);
    score.connection(0.0, filter, 3, env, 0);
    score.connection(0.0, env, 0, sink, 0);

    // ---- Motion Engine: a camada de composição ------------------------
    MotionEngine motion;
    {
        MotionEngine::Binding b;
        b.node = filter; b.paramId = "resonance";
        b.behavior = MotionEngine::Behavior::Walk;
        b.lo = 0.15f; b.hi = 0.65f; b.rateHz = 0.05f;
        b.seed = 0x9E3779B97F4A7C15ULL;
        motion.add(b);
    }
    {
        MotionEngine::Binding b;
        b.node = env; b.paramId = "curve";
        b.behavior = MotionEngine::Behavior::Oscillate;
        b.lo = 0.30f; b.hi = 0.85f; b.rateHz = 0.024f;
        b.seed = 0xD1B54A32D192ED03ULL;
        motion.add(b);
    }
    {
        // ATTRACT: outro módulo, outro parâmetro, perseguindo o WALK acima
        MotionEngine::Binding b;
        b.node = voice; b.paramId = "slope";
        b.behavior = MotionEngine::Behavior::Attract;
        b.lo = 0.0f; b.hi = 1.0f;
        b.nodeB = filter; b.paramIdB = "resonance"; b.gain = 0.15f;
        b.seed = 0x2545F4914F6CDD1DULL;
        motion.add(b);
    }
    {
        // soma-se por cima da modulação do LFO-espalha (cabo) no mesmo
        // parâmetro — a prova da aditividade
        MotionEngine::Binding b;
        b.node = filter; b.paramId = "spread";
        b.behavior = MotionEngine::Behavior::Walk;
        b.lo = 0.0f; b.hi = 0.55f; b.rateHz = 0.04f;
        b.seed = 0xA0761D6478BD642FULL;
        motion.add(b);
    }

    std::vector<float> mono;
    mono.reserve(static_cast<std::size_t>(sr * seconds));
    AudioBlock out(sr, 1, block);
    const std::size_t totalBlocks =
        static_cast<std::size_t>(sr * seconds) / block;
    const float dt = static_cast<float>(block) / sr;

    // registra mudanças de parâmetro só quando passam de um limiar — a
    // SYSTEM SCORE é um registro de eventos, não um dump em taxa de
    // controle (senão seriam ~15 mil linhas por parâmetro)
    struct Tracked { std::size_t node; const char* paramId; float last; };
    std::array<Tracked, 4> tracked{{
        {filter, "resonance", graph.parameterUserValue(filter, "resonance")},
        {env, "curve", graph.parameterUserValue(env, "curve")},
        {voice, "slope", graph.parameterUserValue(voice, "slope")},
        {filter, "spread", graph.parameterUserValue(filter, "spread")},
    }};
    constexpr float kLogThreshold = 0.03f;

    float peak = 0.0f;
    for (std::size_t b = 0; b < totalBlocks; ++b) {
        motion.tick(graph, dt);
        const double t = static_cast<double>(b) * block / sr;
        for (auto& tr : tracked) {
            const float now = graph.parameterUserValue(tr.node, tr.paramId);
            if (std::fabs(now - tr.last) > kLogThreshold) {
                score.parameterChange(t, tr.node, tr.paramId, tr.last, now);
                tr.last = now;
            }
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

    bool ok = writeWav16(outPath, mono, static_cast<std::uint32_t>(sr), 1);
    std::string scorePath = outPath;
    const std::string suffix = ".wav";
    if (scorePath.size() >= suffix.size()
        && scorePath.compare(scorePath.size() - suffix.size(), suffix.size(),
                             suffix) == 0)
        scorePath.resize(scorePath.size() - suffix.size());
    scorePath += ".score.txt";
    std::ofstream scoreFile(scorePath);
    if (scoreFile) scoreFile << score.toText();
    else ok = false;

    if (ok) {
        std::cout << "  render -> " << outPath << " (" << seconds
                  << " s, pico bruto " << peak << ", "
                  << motion.bindingCount() << " ligações de motion, "
                  << score.eventCount() << " eventos de partitura -> "
                  << scorePath << ")\n";
        return 0;
    }
    std::cerr << "falha ao escrever " << outPath << " / " << scorePath << '\n';
    return 1;
}
