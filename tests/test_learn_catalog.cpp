// Teste isolado do LearnCatalog (apps/panel/LearnCatalog.hpp) — o
// mecanismo do hover-learn descrito em
// `dossies/ESTUDO_seed_composicao_generativa.md §6`.

#include "panel/LearnCatalog.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::panel;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

void testKnownEntriesResolve() {
    const auto* e = lookupLearn("FILTER", "cutoff");
    EXPECT(e != nullptr);
    check(e && !e->quick.empty(), "FILTER.cutoff tem texto quick");
    check(e && !e->understand.empty(), "FILTER.cutoff tem texto understand");
    check(e && !e->explore.empty(), "FILTER.cutoff tem texto explore");

    const auto* j = lookupLearn("FILTER", "in:cutoff_mod");
    EXPECT(j != nullptr);
    check(j && !j->quick.empty(), "jack também resolve (in:cutoff_mod)");

    const auto* env = lookupLearn("ENVELOPE", "sustain");
    EXPECT(env != nullptr);
}

void testUnknownModuleReturnsNull() {
    check(lookupLearn("MODULO_QUE_NAO_EXISTE", "x") == nullptr,
          "módulo sem entrada -> nullptr (silencioso, não erro)");
}

void testUnknownParamOnKnownModuleReturnsNull() {
    check(lookupLearn("FILTER", "parametro_que_nao_existe") == nullptr,
          "param sem entrada num módulo conhecido -> nullptr");
}

// pra cada bind de `binds`, `lookupLearn(moduleType, bind)` existe e tem
// `quick` preenchido — widgets reais, tirados do `panel()` de cada
// módulo (src/dsp/*.hpp).
void expectAllDocumented(const char* moduleType,
                         const std::vector<std::string>& binds) {
    for (const auto& b : binds) {
        const auto* e = lookupLearn(moduleType, b);
        check(e != nullptr && e && !e->quick.empty(),
              "widget documentado (quick não vazio)");
    }
}

void testEveryRackDePartidaParamHasAtLeastQuick() {
    expectAllDocumented("FILTER", {
        "cutoff", "resonance", "spread", "drive",
        "in:in", "in:cutoff_mod", "in:res_mod", "in:spread_mod",
        "out:low", "out:center", "out:high", "out:all",
    });
    expectAllDocumented("ENVELOPE", {
        "attack", "decay", "sustain", "release", "curve", "mode",
        "in:in", "in:gate", "in:time_mod", "out:out", "out:env",
    });
    expectAllDocumented("OSC", {
        "freq", "fine", "pw", "fm_amount", "drift", "sub_2", "sync_enable",
        "prox",
        "in:pitch", "in:fm", "in:pwm", "in:sync",
        "out:sine", "out:tri", "out:saw", "out:pulse", "out:sub",
    });
    expectAllDocumented("VCA", {
        "level1", "level2", "cv1_amount", "cv2_amount",
        "response1", "response2", "drift",
        "in:in1", "in:cv1", "in:in2", "in:cv2",
        "out:out1", "out:out2", "out:sum",
    });
    expectAllDocumented("CLOCK", {
        "bpm", "mult", "length", "fill", "rotate", "swing", "drift",
        "gate_len", "accent_a", "accent_b", "accent_mode", "feel",
        "in:ext_clock", "in:reset", "in:bpm_mod",
        "out:clock", "out:euclid", "out:accent",
    });
}

void testEveryOutputChainAndDeepDiveParamHasAtLeastQuick() {
    expectAllDocumented("NOISE", {
        "rate", "slew", "spread", "poisson", "in:trigger", "in:in",
        "out:white", "out:pink", "out:brown", "out:sh", "out:smooth",
        "out:blue", "out:violet", "out:bit",
    });
    expectAllDocumented("DRIFT", {
        "rate", "depth", "momentum", "stride", "bias", "anchor",
        "in:advance", "in:rate_mod",
        "out:a", "out:b", "out:c", "out:d", "out:field", "out:event",
    });
    expectAllDocumented("MIXER", {
        "gain1", "gain2", "gain3", "gain4",
        "pan1", "pan2", "pan3", "pan4",
        "mute1", "mute2", "mute3", "mute4", "out_gain",
        "in:ch1", "in:ch2", "in:ch3", "in:ch4", "out:out",
    });
    expectAllDocumented("MASTER", {
        "gain", "width", "mono", "dc_block", "limit",
        "in:in", "out:out", "out:level",
    });
    expectAllDocumented("WASP", {
        "cutoff", "resonance", "mode", "drive", "grit", "bias", "drift",
        "in:in", "in:cutoff_mod", "in:res_mod", "out:out",
    });
}

// os 27 módulos restantes do catálogo (2026-09-05) — todo bind vem
// direto do `panel()` de cada `src/dsp/*.hpp`, não reescrito de memória.
void testRemainingCatalogModulesHaveAtLeastQuick() {
    expectAllDocumented("ABACUS", {
        "op", "modulus", "steps", "range", "rect_mode", "count_step",
        "pattern", "slew", "rate",
        "in:a", "in:b", "in:clock", "in:reset",
        "out:math", "out:quant", "out:rect", "out:p1", "out:p2", "out:carry",
    });
    expectAllDocumented("SIGNAL-IN", {
        "gain", "bend", "cc_num",
        "out:out", "out:r", "out:pitch", "out:gate", "out:vel", "out:cc",
    });
    expectAllDocumented("CHAOS", {
        "rate", "drive", "damping", "freeze",
        "in:reseed", "in:rate_mod", "out:out",
    });
    expectAllDocumented("CHORD", {
        "freq", "chord", "voices", "inversion", "voicing", "detune",
        "wave", "drift",
        "in:pitch", "in:chord_cv", "in:fm", "out:out",
    });
    expectAllDocumented("CONTROL", {
        "scale1", "scale2", "offset1", "offset2", "rectify1", "rectify2",
        "slew1", "slew2", "curve1", "curve2", "sum_mode", "drift",
        "in:in1", "in:in2", "out:out1", "out:out2", "out:sum",
    });
    expectAllDocumented("DECISION", {
        "rate", "bias", "spread", "shape", "steps", "slew", "dejavu",
        "loop_length",
        "in:trigger", "in:bias_mod", "in:spread_mod",
        "out:x", "out:y", "out:gate",
    });
    expectAllDocumented("FUNCTION", {
        "rate", "slope", "drift", "sync_enable",
        "in:rate_mod", "in:slope_mod", "in:sync", "out:uni", "out:bi",
    });
    expectAllDocumented("HARMONY", {
        "movement", "rate", "root_start", "scale_lo", "scale_hi", "hold",
        "in:advance", "in:reset", "out:root", "out:scale", "out:change",
    });
    expectAllDocumented("LOGIC", {
        "rate", "divide", "multiply", "gate_len", "delay",
        "in:clock", "in:a", "in:b", "in:reset",
        "out:div", "out:and", "out:or", "out:xor", "out:flip",
    });
    expectAllDocumented("LPG", {
        "mode", "response", "offset", "resonance", "bounce", "drift",
        "in:in", "in:strike", "in:cv", "out:out",
    });
    expectAllDocumented("MATRIX", {
        "g11", "g12", "g13", "g14", "g21", "g22", "g23", "g24",
        "g31", "g32", "g33", "g34", "g41", "g42", "g43", "g44",
        "level", "norm", "ring", "sat", "drift",
        "in:in1", "in:in2", "in:in3", "in:in4",
        "out:out1", "out:out2", "out:out3", "out:out4",
    });
    expectAllDocumented("MATTER", {
        "freq", "structure", "brightness", "damping", "position",
        "exciter", "mix",
        "in:in", "in:strike", "in:freq_mod", "in:struct_mod", "out:out",
    });
    expectAllDocumented("MEMORY", {
        "grain", "density", "position", "spray", "pitch", "feedback",
        "blend", "freeze",
        "in:in", "in:position_mod", "in:pitch_mod", "in:freeze_gate",
        "out:out",
    });
    expectAllDocumented("MULT", {
        "dual", "scale1", "offset1", "scale2", "offset2", "scale3",
        "offset3", "scale4", "offset4", "slew",
        "in:in", "in:in2",
        "out:out1", "out:out2", "out:out3", "out:out4",
    });
    expectAllDocumented("NOTE-OUT", {
        "in:gate", "in:pitch", "in:velocity", "in:accent",
        "out:gate_thru", "out:pitch_thru",
    });
    expectAllDocumented("PARAMETRIC", {
        "type1", "freq1", "gain1", "q1", "slope1",
        "type2", "freq2", "gain2", "q2", "slope2",
        "type3", "freq3", "gain3", "q3", "slope3",
        "type4", "freq4", "gain4", "q4", "slope4",
        "output", "drive", "mix",
        "in:in", "in:sweep", "in:amount", "out:out",
    });
    expectAllDocumented("PLL", {
        "freq", "fine", "shape", "ratio", "lock_gain", "fm_amount",
        "feedback_amount", "feedback_type",
        "in:pitch", "in:fm", "in:ref",
        "out:out", "out:ring", "out:lock",
    });
    expectAllDocumented("QUANTIZER", {
        "scale", "root", "range", "glide", "hysteresis",
        "in:cv", "in:transpose", "in:trigger",
        "out:pitch", "out:gate", "out:semitone",
    });
    expectAllDocumented("SH", {
        "rate", "slew1", "slew2", "slope", "track1", "track2", "spread",
        "correlation",
        "in:in1", "in:trig1", "in:in2", "in:trig2", "out:out1", "out:out2",
    });
    expectAllDocumented("SCOPE", {
        "trigger", "edge", "reject", "response", "hold",
        "in:in", "in:ext",
        "out:thru", "out:trig", "out:level", "out:bright", "out:pitch",
    });
    expectAllDocumented("SHAPE", {
        "ring", "fold", "symmetry", "wrap", "sat", "level", "drift",
        "in:in", "in:mod", "in:fold_mod", "out:out",
    });
    expectAllDocumented("SPACE", {
        "time", "taps", "spread", "feedback", "diffusion", "tone", "mod",
        "mix",
        "in:in", "in:time_mod", "in:feedback_mod", "out:out", "out:wet",
    });
    expectAllDocumented("SEQUENCE", {
        "length", "mode", "rate", "gate_len", "glide", "range",
        "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8",
        "g1", "g2", "g3", "g4", "g5", "g6", "g7", "g8",
        "in:clock", "in:reset", "out:pitch", "out:gate", "out:eos",
    });
    expectAllDocumented("STRING", {
        "freq", "decay", "damping", "position", "exciter", "drive", "mix",
        "in:in", "in:pluck", "in:freq_mod", "in:damp_mod", "out:out",
    });
    expectAllDocumented("SWITCH", {
        "steps", "mode", "dir", "glide", "slew",
        "in:a", "in:b", "in:c", "in:d", "in:clock", "in:reset", "in:addr",
        "out:out", "out:step", "out:out_b", "out:out_c", "out:out_d",
    });
    expectAllDocumented("TRIGSEQ", {
        "length", "rate", "map", "density1", "density2", "density3",
        "density4", "swing", "chaos", "ratchet", "fill_amt", "drift",
        "in:clock", "in:reset", "in:fill", "in:map_cv",
        "out:t1", "out:t2", "out:t3", "out:t4", "out:accent", "out:any",
    });
    expectAllDocumented("TURING", {
        "rate", "length", "lock", "mutate", "range", "steps", "offset",
        "in:clock", "in:lock_mod", "out:cv", "out:cv2", "out:pulse",
    });
}

// cada tipo do `moduleCatalog()` tem uma DEFINIÇÃO de módulo (o LEARN no
// hover do corpo/título — 2026-09-07), com `quick` preenchido.
void testEveryCatalogModuleHasBlurb() {
    const char* types[] = {
        "OSC", "WAVETABLE", "ADDITIVE", "OPERATOR", "PLL", "CHORD", "NOISE",
        "MATTER", "STRING", "DRUM", "SIGNAL-IN",
        "FILTER", "FORMANT", "WASP", "LPG", "VCA", "SHAPE", "PARAMETRIC",
        "GLIDE", "CONTROL", "CRUSH",
        "ENVELOPE", "FUNCTION", "DRIFT", "CHAOS", "SH",
        "CLOCK", "LOGIC", "TURING", "SEQUENCE", "TRIGSEQ",
        "QUANTIZER", "HARMONY", "ABACUS", "DECISION", "BOXCAR",
        "SWITCH", "MATRIX", "MULT", "PLANAR",
        "SPACE", "HALL", "LOOPER", "SWIRL", "MEMORY", "SAMPLER", "TURNTABLE",
        "MIXER", "MASTER", "SCOPE", "NOTE-OUT",
    };
    for (const char* t : types) {
        const auto* e = lookupLearnModule(t);
        check(e != nullptr && !e->quick.empty(),
              "módulo tem definição (quick não vazio)");
    }
    check(lookupLearnModule("AUDIO-IN") == lookupLearnModule("SIGNAL-IN"),
          "AUDIO-IN reusa a definição do SIGNAL-IN");
    check(lookupLearnModule("NAO_EXISTE") == nullptr,
          "módulo desconhecido -> nullptr");
}

}  // namespace

int main() {
    testKnownEntriesResolve();
    testUnknownModuleReturnsNull();
    testUnknownParamOnKnownModuleReturnsNull();
    testEveryRackDePartidaParamHasAtLeastQuick();
    testEveryOutputChainAndDeepDiveParamHasAtLeastQuick();
    testRemainingCatalogModulesHaveAtLeastQuick();
    testEveryCatalogModuleHasBlurb();
    if (g_failures == 0) {
        std::cout << "RASGO Modular learn catalog tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
