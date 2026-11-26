#pragma once

// O CONTROLE PRINCIPAL de cada módulo (26/11/2026, frente de human polish;
// gramática G5: hierarquia): o knob que diz o que o módulo é — a frequência
// de um oscilador, o corte de um filtro, a taxa de um LFO. Ele ganha o corpo
// INVERTIDO (claro, ponteiro escuro; ideia do autor: "knobs com cores
// invertidas") e uma ESCALA em volta (11 marcas, de -135° a +135°).
// Módulos de controles simétricos (VCA4, MIXER, MULT…) não têm um principal.
// Uma tabela só, lida pelo app, pela página (pelo motor, `"pr"`) e pelo
// plugin VCV.

#include <string>
#include <unordered_map>

namespace rasgo::ui {

inline constexpr unsigned kPrimaryBody = 0xd8dde6;     // prata
inline constexpr unsigned kPrimaryPointer = 0x131a1a;  // o fundo do rack
inline constexpr int kScaleTicks = 11;
inline constexpr float kScaleInner = 0.5f, kScaleOuter = 1.2f;   // mm além do raio

inline const std::string& primaryControl(const std::string& type) {
    static const std::unordered_map<std::string, std::string> tabela{
        {"ADDITIVE", "freq"},
        {"CHORD", "freq"},
        {"DRUM", "decay"},
        {"GENDYN", "freq"},
        {"MATTER", "freq"},
        {"OPERATOR", "freq"},
        {"OSC", "freq"},
        {"PLL", "freq"},
        {"PULSAR", "freq"},
        {"RUNGLER", "freq_a"},
        {"SIGNAL-IN", "gain"},
        {"SPECTRA", "voices"},
        {"STRING", "freq"},
        {"WAVETABLE", "freq"},
        {"CRUSH", "rate"},
        {"DYNAMICS", "thresh"},
        {"ELASTIC", "tune"},
        {"FILTER", "cutoff"},
        {"FORMANT", "vowel"},
        {"GLIDE", "time"},
        {"LADDER", "cutoff"},
        {"LPG", "response"},
        {"RESONATOR", "freq"},
        {"SHAPE", "fold"},
        {"SHIFTER", "shift"},
        {"TAPE", "speed"},
        {"VOCODER", "bands"},
        {"WASP", "cutoff"},
        {"BOUNCE", "gravity"},
        {"CHAOS", "rate"},
        {"DRIFT", "rate"},
        {"EPICYCLE", "rate"},
        {"FORM", "grammar"},
        {"FUNCTION", "rate"},
        {"MAGNET", "rate"},
        {"MOUSSE", "rate"},
        {"ORBIT", "speed"},
        {"PENDULUM", "speed"},
        {"PREDATOR", "size"},
        {"PRESSURE", "temp"},
        {"REFRACT", "rate"},
        {"SH", "rate"},
        {"STAGES", "rate"},
        {"SUPPLY", "time"},
        {"ARP", "rate"},
        {"BUDGET", "ceiling"},
        {"CELL", "rule"},
        {"CLOCK", "bpm"},
        {"ECLIPSE", "speed"},
        {"INTERFERENCE", "speed"},
        {"LOGIC", "rate"},
        {"SEASON", "day"},
        {"SEQUENCE", "rate"},
        {"SWARM", "rate"},
        {"TEMPO", "accel"},
        {"TRIGSEQ", "rate"},
        {"TURING", "rate"},
        {"VOLTA", "rate"},
        {"ABACUS", "rate"},
        {"ACCENT", "metric"},
        {"BLEFE", "truthfulness"},
        {"BOXCAR", "rate"},
        {"DECISION", "rate"},
        {"FRONTEIRA", "threshold"},
        {"HARMONY", "rate"},
        {"INTERPRET", "expression"},
        {"KEY", "stability"},
        {"MARKOV", "rate"},
        {"PHRASE", "length"},
        {"QUANTIZER", "scale"},
        {"PLANAR", "rate"},
        {"RELATE", "time"},
        {"SWITCH", "steps"},
        {"VIA", "body"},
        {"APOCRIFO", "min_age"},
        {"CUTUP", "chance"},
        {"HALL", "size"},
        {"LENS", "focus"},
        {"LOOPER", "time"},
        {"MEMORY", "grain"},
        {"PRISM", "disp"},
        {"SAMPLER", "speed"},
        {"SPACE", "time"},
        {"SPRING", "decay"},
        {"SWIRL", "rate"},
        {"TURNTABLE", "speed"},
        {"LISTEN", "sens"},
    };
    static const std::string nada;
    const auto it = tabela.find(type);
    return it == tabela.end() ? nada : it->second;
}

// É o controle principal? Alguns módulos têm DOIS, gêmeos (autor,
// 27/11/2026: o RUNGLER são dois osciladores que se modulam — FRQA e FRQB).
inline bool isPrimaryControl(const std::string& type, const std::string& bind) {
    if (bind.empty()) return false;
    if (bind == primaryControl(type)) return true;
    return type == "RUNGLER" && bind == "freq_b";
}

}  // namespace rasgo::ui
