#pragma once

// Catálogo de módulos do Rasgo Modular: nome de tipo -> construtor, e a
// listagem agrupada por família pra a paleta lateral do painel. Também
// serve de factory pra desserializar um `.rmp` (`SignalGraph::deserialize`).
//
// Específico do Rasgo Modular (não é regra RASGO comum).

#include "core/SignalGraph.hpp"
#include "dsp/Abacus.hpp"
#include "dsp/AudioIn.hpp"
#include "dsp/Chaos.hpp"
#include "dsp/Control.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Drift.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/Chord.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Harmony.hpp"
#include "dsp/Logic.hpp"
#include "dsp/Lpg.hpp"
#include "dsp/Master.hpp"
#include "dsp/Matrix.hpp"
#include "dsp/Matter.hpp"
#include "dsp/Memory.hpp"
#include "dsp/Mixer.hpp"
#include "dsp/Mult.hpp"
#include "dsp/Noise.hpp"
#include "dsp/NoteOut.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Parametric.hpp"
#include "dsp/Pll.hpp"
#include "dsp/Quantizer.hpp"
#include "dsp/SampleHold.hpp"
#include "dsp/Scope.hpp"
#include "dsp/Shape.hpp"
#include "dsp/Space.hpp"
#include "dsp/StepSequencer.hpp"
#include "dsp/StringVoice.hpp"
#include "dsp/Switch.hpp"
#include "dsp/TrigSeq.hpp"
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
    if (t == "AUDIO-IN")   return std::make_unique<AudioIn>();
    if (t == "CHAOS")      return std::make_unique<Chaos>();
    if (t == "PLL")        return std::make_unique<Pll>();
    if (t == "NOISE")      return std::make_unique<Noise>();
    if (t == "NOTE-OUT")   return std::make_unique<NoteOut>();
    if (t == "CHORD")      return std::make_unique<Chord>();
    if (t == "FUNCTION")   return std::make_unique<FunctionGenerator>();
    if (t == "FILTER")     return std::make_unique<Filter>();
    if (t == "VCA")        return std::make_unique<Vca>();
    if (t == "CONTROL")    return std::make_unique<Control>();
    if (t == "SH")         return std::make_unique<SampleHold>();
    if (t == "SCOPE")      return std::make_unique<Scope>();
    if (t == "SHAPE")      return std::make_unique<Shape>();
    if (t == "LPG")        return std::make_unique<Lpg>();
    if (t == "DECISION")   return std::make_unique<Decision>();
    if (t == "DRIFT")      return std::make_unique<Drift>();
    if (t == "CLOCK")      return std::make_unique<EuclidClock>();
    if (t == "ENVELOPE")   return std::make_unique<Envelope>();
    if (t == "LOGIC")      return std::make_unique<Logic>();
    if (t == "MEMORY")     return std::make_unique<Memory>();
    if (t == "TURING")     return std::make_unique<TuringLoop>();
    if (t == "MATTER")     return std::make_unique<Matter>();
    if (t == "SPACE")      return std::make_unique<Space>();
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
    return nullptr;
}

struct CatalogGroup {
    const char* family;
    std::vector<const char*> types;
};

// Agrupado por família (taxonomia do RASGO_MODULAR.md §4).
inline const std::vector<CatalogGroup>& moduleCatalog() {
    static const std::vector<CatalogGroup> c = {
        {"SOURCE",      {"OSC", "PLL", "CHORD", "NOISE", "FUNCTION", "AUDIO-IN"}},
        {"TIME",        {"CLOCK", "LOGIC", "ENVELOPE"}},
        {"DECISION",    {"DECISION", "DRIFT", "QUANTIZER", "HARMONY", "ABACUS", "CHAOS"}},
        {"SEQUENCE",    {"TURING", "SEQUENCE", "SWITCH", "TRIGSEQ"}},
        {"TRANSFORM",   {"FILTER", "WASP", "LPG", "VCA", "SHAPE", "CONTROL", "MULT", "SH", "PARAMETRIC"}},
        {"MATTER",      {"MATTER", "STRING"}},
        {"MEMORY",      {"MEMORY"}},
        {"SPACE",       {"SPACE"}},
        {"MIX",         {"MIXER", "MATRIX", "MASTER", "SCOPE", "NOTE-OUT"}},
    };
    return c;
}

}  // namespace rasgo::panel
