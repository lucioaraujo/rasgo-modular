// Auditoria de LAYOUT de painel (todos os módulos do catálogo).
// Modela a pegada REAL: knob/slider/toggle/jack + o rótulo de texto na
// posição em que o `panel_main.cpp` desenha (rótulo acima do knob/jack,
// abaixo do toggle, com a fonte de legenda menor). Zoom "médio" — o
// `panel_main` também roda um audit de sobreposição de widget ao abrir.
//
// Gate contra regressão: um módulo novo com knobs apertados / rótulo
// longo demais quebra este teste.

#include "core/SignalGraph.hpp"
#include "dsp/Abacus.hpp"
#include "dsp/AudioIn.hpp"
#include "dsp/Chaos.hpp"
#include "dsp/Chord.hpp"
#include "dsp/Control.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Drift.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Wavetable.hpp"
#include "dsp/Glide.hpp"
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

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;

struct Rect { float x, y, w, h; std::string tag; };

// mm por caractere do rótulo, no zoom "médio" (fonte de legenda ~9 px,
// g_s ~2 px/mm). `panel_main.cpp` desenha essa fonte menor pros rótulos.
// 2,8 (era 2,5) + folga menor: pega rótulos de jack "colados" que o
// modelo frouxo deixava passar (passe de ergonomia 2026-09-06).
constexpr float kCharMM = 2.8f;
constexpr float kKn = 9.0f;
constexpr float kTg = 4.4f;

std::vector<Rect> footprints(const std::string& type, const Panel& p) {
    std::vector<Rect> v;
    // MATRIX: os 16 g<jk> NÃO são desenhados como knobs — `panel_main.cpp`
    // os pula e desenha uma grade 4×4 clicável via `matrixCellMM`
    // (cx = 27 + k·17, cy = 35 + j·18, 16×17 mm) + um texto-guia
    // "IN↓ OUT→" em ~(50, 24). Modelar a pegada REAL, não os knobs.
    const bool matrixGrid = (type == "MATRIX");
    if (matrixGrid) {
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k) {
                const float cx = 27.0f + static_cast<float>(k) * 17.0f;
                const float cy = 35.0f + static_cast<float>(j) * 18.0f;
                v.push_back({cx - 8.0f, cy - 8.5f, 16.0f, 17.0f,
                             std::string("cell:g")
                                 + static_cast<char>('1' + j)
                                 + static_cast<char>('1' + k)});
            }
        v.push_back({50.0f, 16.5f, 11.0f * kCharMM, 4.0f, "rot:matrixhint"});
    }
    for (const auto& w : p.widgets) {
        const float tw = static_cast<float>(w.label.size()) * kCharMM;
        if (matrixGrid && w.kind == Widget::Kind::Knob
            && w.bind.size() == 3 && w.bind[0] == 'g')
            continue;   // já modelado como célula de grade acima
        switch (w.kind) {
        case Widget::Kind::Knob:
            v.push_back({w.x, w.y, kKn, kKn, "knob:" + w.label});
            v.push_back({w.x + kKn / 2 - tw / 2, w.y - 4.5f, tw, 4.0f,
                         "rot:" + w.label});
            break;
        case Widget::Kind::Slider:
            v.push_back({w.x, w.y, 8.0f, 32.0f, "sld:" + w.label});
            v.push_back({w.x + 4.0f - tw / 2, w.y + 32.0f, tw, 4.0f,
                         "rot:" + w.label});
            break;
        case Widget::Kind::Toggle:
            v.push_back({w.x, w.y, kTg, kTg, "tgl:" + w.label});
            v.push_back({w.x + kTg / 2 - tw / 2, w.y + kTg + 1.0f, tw, 4.0f,
                         "rot:" + w.label});
            break;
        case Widget::Kind::Jack:
            v.push_back({w.x - 2.6f, w.y - 2.6f, 5.2f, 5.2f, "jack:" + w.label});
            v.push_back({w.x - tw / 2, w.y - 2.6f - 4.5f, tw, 4.0f,
                         "rot:" + w.label});
            break;
        case Widget::Kind::Display:
            v.push_back({w.x - 0.5f, w.y,
                         (w.span > 1.0f ? w.span : 16.0f) + 1.0f, 16.5f, "disp"});
            break;
        case Widget::Kind::Label:
            v.push_back({w.x - 0.5f, w.y - 0.5f, tw, 4.0f, "hdr"});
            break;
        }
    }
    return v;
}

bool isText(const std::string& t) { return t.rfind("rot:", 0) == 0; }

bool overlap(const Rect& a, const Rect& b) {
    // folga de 0,3 mm — só um fio de respiro conta como "não colado"
    return a.x < b.x + b.w - 0.3f && b.x < a.x + a.w - 0.3f
        && a.y < b.y + b.h - 0.3f && b.y < a.y + a.h - 0.3f;
}

// Passe de ergonomia 2026-09-06: todos os módulos do catálogo revistos.
// O modelo apertado (kCharMM 2,8 / folga 0,3) é o gate contra regressão.
bool ergonomiaPendente(const std::string&) { return false; }

void audit(const char* type, const Panel& p) {
    const float W = static_cast<float>(p.hp) * 5.08f;
    const bool pend = ergonomiaPendente(type);
    const auto v = footprints(type, p);
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (isText(v[i].tag)
            && (v[i].x < -1.5f || v[i].x + v[i].w > W + 1.5f)) {
            std::cerr << "LAYOUT " << type << ": rótulo fora do painel: "
                      << v[i].tag << " (x " << v[i].x << ".."
                      << v[i].x + v[i].w << " / W " << W << ")\n";
            ++g_failures;
        }
        for (std::size_t j = i + 1; j < v.size(); ++j) {
            const std::string li = v[i].tag.substr(v[i].tag.find(':') + 1);
            const std::string lj = v[j].tag.substr(v[j].tag.find(':') + 1);
            if (li == lj && !li.empty()) continue;   // knob e o próprio rótulo
            if (!isText(v[i].tag) && !isText(v[j].tag)) continue;
            if (overlap(v[i], v[j])) {
                std::cerr << (pend ? "LAYOUT (pendente) " : "LAYOUT ") << type
                          << ": '" << v[i].tag << "' x '" << v[j].tag << "'\n";
                if (!pend) ++g_failures;
            }
        }
    }
}

}  // namespace

int main() {
    std::vector<std::pair<std::string, std::unique_ptr<Signal>>> mods;
    mods.emplace_back("OSC", std::make_unique<Oscillator>());
    mods.emplace_back("WAVETABLE", std::make_unique<Wavetable>());
    mods.emplace_back("NOISE", std::make_unique<Noise>());
    mods.emplace_back("CHORD", std::make_unique<Chord>());
    mods.emplace_back("FUNCTION", std::make_unique<FunctionGenerator>());
    mods.emplace_back("FILTER", std::make_unique<Filter>());
    mods.emplace_back("WASP", std::make_unique<Wasp>());
    mods.emplace_back("VCA", std::make_unique<Vca>());
    mods.emplace_back("CONTROL", std::make_unique<Control>());
    mods.emplace_back("GLIDE", std::make_unique<Glide>());
    mods.emplace_back("SH", std::make_unique<SampleHold>());
    mods.emplace_back("SCOPE", std::make_unique<Scope>());
    mods.emplace_back("SHAPE", std::make_unique<Shape>());
    mods.emplace_back("LPG", std::make_unique<Lpg>());
    mods.emplace_back("DECISION", std::make_unique<Decision>());
    mods.emplace_back("DRIFT", std::make_unique<Drift>());
    mods.emplace_back("ABACUS", std::make_unique<Abacus>());
    mods.emplace_back("CLOCK", std::make_unique<EuclidClock>());
    mods.emplace_back("ENVELOPE", std::make_unique<Envelope>());
    mods.emplace_back("LOGIC", std::make_unique<Logic>());
    mods.emplace_back("MEMORY", std::make_unique<Memory>());
    mods.emplace_back("TURING", std::make_unique<TuringLoop>());
    mods.emplace_back("MATTER", std::make_unique<Matter>());
    mods.emplace_back("SPACE", std::make_unique<Space>());
    mods.emplace_back("STRING", std::make_unique<StringVoice>());
    mods.emplace_back("QUANTIZER", std::make_unique<Quantizer>());
    mods.emplace_back("PARAMETRIC", std::make_unique<Parametric>());
    mods.emplace_back("HARMONY", std::make_unique<Harmony>());
    mods.emplace_back("SEQUENCE", std::make_unique<StepSequencer>());
    mods.emplace_back("SWITCH", std::make_unique<Switch>());
    mods.emplace_back("TRIGSEQ", std::make_unique<TrigSeq>());
    mods.emplace_back("MIXER", std::make_unique<Mixer>());
    mods.emplace_back("MATRIX", std::make_unique<Matrix>());
    mods.emplace_back("MULT", std::make_unique<Mult>());
    mods.emplace_back("MASTER", std::make_unique<Master>());
    mods.emplace_back("PLL", std::make_unique<Pll>());
    mods.emplace_back("CHAOS", std::make_unique<Chaos>());
    mods.emplace_back("NOTE-OUT", std::make_unique<NoteOut>());
    mods.emplace_back("AUDIO-IN", std::make_unique<AudioIn>());

    // regra: rótulo de knob/toggle no máximo 5 caracteres
    for (const auto& m : mods) {
        for (const auto& w : m.second->panel().widgets) {
            if ((w.kind == Widget::Kind::Knob || w.kind == Widget::Kind::Toggle
                 || w.kind == Widget::Kind::Slider)
                && w.label.size() > 5) {
                std::cerr << "LABEL " << m.first << ": rótulo > 5 chars: '"
                          << w.label << "'\n";
                ++g_failures;
            }
        }
        audit(m.first.c_str(), m.second->panel());
    }

    if (g_failures == 0) std::cout << "test_panel_layout: OK\n";
    return g_failures == 0 ? 0 : 1;
}
