// Teste isolado do ScoreRecorder (apps/panel/ScoreRecorder.hpp) —
// SYSTEM SCORE, descrito em
// `dossies/ESTUDO_seed_composicao_generativa.md §5`.

#include "panel/ScoreRecorder.hpp"

#include <iostream>

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

void testRecordsInOrder() {
    ScoreRecorder rec;
    rec.connection(0.0, 0, 1, 2, 0);
    rec.modulation(0.0, 3, 1, 2, "cutoff", 400.0f, -20.0f);
    rec.parameterChange(1.5, 2, "cutoff", 380.0f, 412.5f);
    EXPECT(rec.eventCount() == 3);
    EXPECT(rec.events()[0].type == RasgoEvent::Type::Connection);
    EXPECT(rec.events()[1].type == RasgoEvent::Type::Modulation);
    EXPECT(rec.events()[2].type == RasgoEvent::Type::ParameterChange);
    check(rec.events()[2].time == 1.5, "tempo preservado exatamente como passado");
}

void testTextFormatContainsFields() {
    ScoreRecorder rec;
    rec.connection(0.0, 5, 0, 7, 2);
    rec.modulation(0.25, 1, 0, 7, "spread", 0.3f, 0.0f);
    rec.parameterChange(0.5, 7, "spread", 0.15f, 0.22f);
    const std::string txt = rec.toText();
    check(txt.find("rasgo-system-score 1") == 0, "cabeçalho do formato");
    check(txt.find("connection 5:0 -> 7:2") != std::string::npos,
          "linha de conexão legível");
    check(txt.find("modulation 1:0 -> 7.spread") != std::string::npos,
          "linha de modulação legível");
    check(txt.find("param 7.spread 0.15") != std::string::npos,
          "linha de mudança de parâmetro legível");
}

void testDeterministicText() {
    // mesmos eventos, na mesma ordem -> mesmo texto, byte a byte
    auto build = []() {
        ScoreRecorder rec;
        rec.connection(0.0, 0, 0, 1, 0);
        for (int i = 0; i < 50; ++i) {
            const double t = i * 0.128;
            rec.parameterChange(t, 1, "resonance",
                               0.3f + 0.01f * i, 0.3f + 0.01f * (i + 1));
        }
        return rec;
    };
    const auto a = build();
    const auto b = build();
    EXPECT(a.toText() == b.toText());
    EXPECT(a.eventCount() == 51);
}

void testNoteEvent() {
    ScoreRecorder rec;
    rec.connection(0.0, 0, 0, 1, 0);
    rec.note(1.25, 9, 0.5f, 0.8f, 0.333, true);
    EXPECT(rec.eventCount() == 2);
    EXPECT(rec.events()[1].type == RasgoEvent::Type::Note);
    check(rec.events()[1].time == 1.25, "tempo da nota preservado (inicio)");
    check(rec.events()[1].notePitch == 0.5f, "pitch preservado");
    check(rec.events()[1].noteVelocity == 0.8f, "velocity preservada");
    check(rec.events()[1].noteDuration == 0.333, "duracao preservada");
    check(rec.events()[1].noteAccent, "acento preservado");
    const std::string txt = rec.toText();
    check(txt.find("note 9 pitch=0.5") != std::string::npos,
          "linha de nota legivel (pitch)");
    check(txt.find("velocity=0.8") != std::string::npos,
          "linha de nota legivel (velocity)");
    check(txt.find("accent=1") != std::string::npos,
          "linha de nota legivel (acento)");
}

void testClear() {
    ScoreRecorder rec;
    rec.parameterChange(0.0, 0, "x", 0.0f, 1.0f);
    EXPECT(rec.eventCount() == 1);
    rec.clear();
    EXPECT(rec.eventCount() == 0);
    EXPECT(rec.toText().find("rasgo-system-score 1") == 0);
}

}  // namespace

int main() {
    testRecordsInOrder();
    testTextFormatContainsFields();
    testDeterministicText();
    testNoteEvent();
    testClear();
    if (g_failures == 0) {
        std::cout << "RASGO Modular score recorder tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
