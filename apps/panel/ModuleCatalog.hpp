#pragma once

// Catálogo de módulos do Rasgo Modular: nome de tipo -> construtor, e a
// listagem agrupada por família pra a paleta lateral do painel. Também
// serve de factory pra desserializar um `.rmp` (`SignalGraph::deserialize`).
//
// Específico do Rasgo Modular (não é regra RASGO comum).

#include "core/SignalGraph.hpp"
#include "dsp/Abacus.hpp"
#include "dsp/Additive.hpp"
#include "dsp/Boxcar.hpp"
#include "dsp/SignalIn.hpp"
#include "dsp/Chaos.hpp"
#include "dsp/Control.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Drift.hpp"
#include "dsp/Drum.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/Chord.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Formant.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Wavetable.hpp"
#include "dsp/Glide.hpp"
#include "dsp/Hall.hpp"
#include "dsp/Harmony.hpp"
#include "dsp/Logic.hpp"
#include "dsp/Looper.hpp"
#include "dsp/Lpg.hpp"
#include "dsp/Master.hpp"
#include "dsp/Matrix.hpp"
#include "dsp/Matter.hpp"
#include "dsp/Memory.hpp"
#include "dsp/Mixer.hpp"
#include "dsp/Mult.hpp"
#include "dsp/Noise.hpp"
#include "dsp/NoteOut.hpp"
#include "dsp/Operator.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Parametric.hpp"
#include "dsp/Planar.hpp"
#include "dsp/Pll.hpp"
#include "dsp/Quantizer.hpp"
#include "dsp/SampleHold.hpp"
#include "dsp/Sampler.hpp"
#include "dsp/Scope.hpp"
#include "dsp/Shape.hpp"
#include "dsp/Space.hpp"
#include "dsp/StepSequencer.hpp"
#include "dsp/StringVoice.hpp"
#include "dsp/Switch.hpp"
#include "dsp/TrigSeq.hpp"
#include "dsp/Turntable.hpp"
#include "dsp/TuringLoop.hpp"
#include "dsp/Vca.hpp"
#include "dsp/Wasp.hpp"

#include <memory>
#include <string>
#include <vector>

namespace rasgo::panel {

inline std::unique_ptr<rasgo::modular::Signal> makeModule(const std::string& t) {
    using namespace rasgo::modular;
    if (t == "OSC")        return std::make_unique<Oscillator>();
    if (t == "WAVETABLE")  return std::make_unique<Wavetable>();
    if (t == "ADDITIVE")   return std::make_unique<Additive>();
    if (t == "OPERATOR")   return std::make_unique<Operator>();
    if (t == "SIGNAL-IN" || t == "AUDIO-IN")
        return std::make_unique<SignalIn>();   // AUDIO-IN = alias de migração
    if (t == "CHAOS")      return std::make_unique<Chaos>();
    if (t == "PLL")        return std::make_unique<Pll>();
    if (t == "NOISE")      return std::make_unique<Noise>();
    if (t == "NOTE-OUT")   return std::make_unique<NoteOut>();
    if (t == "CHORD")      return std::make_unique<Chord>();
    if (t == "FUNCTION")   return std::make_unique<FunctionGenerator>();
    if (t == "FILTER")     return std::make_unique<Filter>();
    if (t == "FORMANT")    return std::make_unique<Formant>();
    if (t == "GLIDE")      return std::make_unique<Glide>();
    if (t == "VCA")        return std::make_unique<Vca>();
    if (t == "CONTROL")    return std::make_unique<Control>();
    if (t == "SH")         return std::make_unique<SampleHold>();
    if (t == "SAMPLER")    return std::make_unique<Sampler>();
    if (t == "TURNTABLE")  return std::make_unique<Turntable>();
    if (t == "SCOPE")      return std::make_unique<Scope>();
    if (t == "SHAPE")      return std::make_unique<Shape>();
    if (t == "LPG")        return std::make_unique<Lpg>();
    if (t == "DECISION")   return std::make_unique<Decision>();
    if (t == "BOXCAR")     return std::make_unique<Boxcar>();
    if (t == "DRIFT")      return std::make_unique<Drift>();
    if (t == "CLOCK")      return std::make_unique<EuclidClock>();
    if (t == "ENVELOPE")   return std::make_unique<Envelope>();
    if (t == "LOGIC")      return std::make_unique<Logic>();
    if (t == "MEMORY")     return std::make_unique<Memory>();
    if (t == "LOOPER")     return std::make_unique<Looper>();
    if (t == "TURING")     return std::make_unique<TuringLoop>();
    if (t == "MATTER")     return std::make_unique<Matter>();
    if (t == "DRUM")       return std::make_unique<Drum>();
    if (t == "SPACE")      return std::make_unique<Space>();
    if (t == "HALL")       return std::make_unique<Hall>();
    if (t == "STRING")     return std::make_unique<StringVoice>();
    if (t == "QUANTIZER")  return std::make_unique<Quantizer>();
    if (t == "PARAMETRIC") return std::make_unique<Parametric>();
    if (t == "HARMONY")    return std::make_unique<Harmony>();
    if (t == "SEQUENCE")   return std::make_unique<StepSequencer>();
    if (t == "SWITCH")     return std::make_unique<Switch>();
    if (t == "TRIGSEQ")    return std::make_unique<TrigSeq>();
    if (t == "ABACUS")     return std::make_unique<Abacus>();
    if (t == "WASP")       return std::make_unique<Wasp>();
    if (t == "MIXER")      return std::make_unique<Mixer>();
    if (t == "MASTER")     return std::make_unique<Master>();
    if (t == "MATRIX")     return std::make_unique<Matrix>();
    if (t == "MULT")       return std::make_unique<Mult>();
    if (t == "PLANAR")     return std::make_unique<Planar>();
    return nullptr;
}

struct CatalogGroup {
    const char* family;
    std::vector<const char*> types;
};

// Agrupado por família (taxonomia consolidada — `RASGO_MODULAR.md §4`,
// revisão 2026-09-06). 8 famílias de trabalho, cruzadas com a literatura
// Eurorack (ModularGrid function tags, Patch & Tweak, Doepfer A-100):
//   SOURCE     gerar        (inclui vozes de modelagem física — MATTER/STRING)
//   TRANSFORM  transformar  (modifica um sinal que passa — áudio ou CV)
//   MODULATE   mover        (GERA um sinal de controle)
//   TIME       marcar tempo (clock, lógica de clock, sequenciadores)
//   DECISION   decidir      (escolhe um valor — quantiza, harmoniza, calcula)
//   ROUTE      rotear       (chave, matriz, múltiplo, morph)
//   SPACE      espacializar/lembrar (delay, reverb, granular)
//   OUT        misturar/medir/enviar
// A ORDEM aqui é a ordem de instanciação dos nós no painel → o que cada
// `RASGO_SEED=N` produz e o doador do CROSS. Reordenada nesta revisão;
// `tests/test_seed_patch.cpp` revalida (caminho audível, sem exceção,
// saída finita).
inline const std::vector<CatalogGroup>& moduleCatalog() {
    static const std::vector<CatalogGroup> c = {
        {"SOURCE",    {"OSC", "WAVETABLE", "ADDITIVE", "OPERATOR", "PLL", "CHORD", "NOISE", "MATTER", "STRING", "DRUM", "SIGNAL-IN"}},
        {"TRANSFORM", {"FILTER", "FORMANT", "WASP", "LPG", "VCA", "SHAPE", "PARAMETRIC", "GLIDE", "CONTROL"}},
        {"MODULATE",  {"ENVELOPE", "FUNCTION", "DRIFT", "CHAOS", "SH"}},
        {"TIME",      {"CLOCK", "LOGIC", "TURING", "SEQUENCE", "TRIGSEQ"}},
        {"DECISION",  {"QUANTIZER", "HARMONY", "ABACUS", "DECISION", "BOXCAR"}},
        {"ROUTE",     {"SWITCH", "MATRIX", "MULT", "PLANAR"}},
        {"SPACE",     {"SPACE", "HALL", "LOOPER", "MEMORY", "SAMPLER", "TURNTABLE"}},
        {"OUT",       {"MIXER", "MASTER", "SCOPE", "NOTE-OUT"}},
    };
    return c;
}

}  // namespace rasgo::panel
