// Teste isolado do Módulo 38 (NOTE-OUT) - antes de entrar num patch.
// Critérios do dossiê `dossies/38_note_out.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/NoteOut.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr std::size_t kBlock = 64;

void testNoCompletedNoteWithoutGate() {
    NoteOut n;
    n.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kBlock));
    std::vector<const AudioBlock*> in{nullptr, nullptr, nullptr, nullptr};
    for (int b = 0; b < 100; ++b) n.process(in, out);
    NoteOut::CompletedNote cn;
    check(!n.takeCompletedNote(cn), "sem GATE nenhum, nunca ha nota completa");
}

void testDetectsNoteOnOff() {
    NoteOut n;
    n.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kBlock));
    AudioBlock gate(kSr, 1, kBlock), pitch(kSr, 1, kBlock);
    std::vector<const AudioBlock*> in{&gate, &pitch, nullptr, nullptr};

    // gate baixo por 2 blocos
    for (int b = 0; b < 2; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) { gate.at(0, i) = 0.0f; pitch.at(0, i) = 0.3f; }
        n.process(in, out);
    }
    // gate sobe -- nota-liga com pitch=0.3
    for (int b = 0; b < 10; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) { gate.at(0, i) = 1.0f; pitch.at(0, i) = 0.3f; }
        n.process(in, out);
    }
    NoteOut::CompletedNote cn;
    check(!n.takeCompletedNote(cn), "nota ainda ligada (gate alto) -- nao completa ainda");
    // gate desce -- fecha a nota
    for (std::size_t i = 0; i < kBlock; ++i) gate.at(0, i) = 0.0f;
    n.process(in, out);
    check(n.takeCompletedNote(cn), "nota completa apos gate descer");
    check(std::fabs(cn.pitch - 0.3f) < 1e-6f, "pitch capturado no instante do gate subir");
    check(cn.velocity == 1.0f, "sem VEL conectada, velocity default = 1.0");
    check(!cn.accent, "sem ACC conectada, accent default = false");
    const double expectedSeconds = 10.0 * kBlock / kSr;
    check(std::fabs(cn.durationSeconds - expectedSeconds) < 1e-6,
          "duracao bate com os blocos de gate alto");
    check(!n.takeCompletedNote(cn), "so' devolve 1 vez -- consumida");
}

void testVelocityAndAccentSampledAtNoteOn() {
    NoteOut n;
    n.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kBlock));
    AudioBlock gate(kSr, 1, kBlock), pitch(kSr, 1, kBlock), vel(kSr, 1, kBlock), acc(kSr, 1, kBlock);
    std::vector<const AudioBlock*> in{&gate, &pitch, &vel, &acc};

    for (std::size_t i = 0; i < kBlock; ++i) { gate.at(0, i) = 0.0f; pitch.at(0, i) = -0.2f; vel.at(0, i) = 0.6f; acc.at(0, i) = 1.0f; }
    n.process(in, out);
    for (std::size_t i = 0; i < kBlock; ++i) { gate.at(0, i) = 1.0f; }
    n.process(in, out);   // nota-liga aqui, com vel=0,6 e acc=1
    // muda vel/acc DEPOIS do ataque -- nao deve afetar a nota ja iniciada
    for (int b = 0; b < 5; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) { vel.at(0, i) = 0.1f; acc.at(0, i) = 0.0f; }
        n.process(in, out);
    }
    for (std::size_t i = 0; i < kBlock; ++i) gate.at(0, i) = 0.0f;
    n.process(in, out);
    NoteOut::CompletedNote cn;
    EXPECT(n.takeCompletedNote(cn));
    check(std::fabs(cn.velocity - 0.6f) < 1e-6f, "velocity amostrada no instante do gate subir, nao depois");
    check(cn.accent, "accent amostrado no instante do gate subir, nao depois");
}

void testThruPassesGateAndPitchUnchanged() {
    NoteOut n;
    n.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kBlock));
    AudioBlock gate(kSr, 1, kBlock), pitch(kSr, 1, kBlock);
    std::vector<const AudioBlock*> in{&gate, &pitch, nullptr, nullptr};
    for (std::size_t i = 0; i < kBlock; ++i) { gate.at(0, i) = (i % 2 == 0) ? 1.0f : 0.0f; pitch.at(0, i) = 0.42f; }
    n.process(in, out);
    for (std::size_t i = 0; i < kBlock; ++i) {
        check(out[0].at(0, i) == gate.at(0, i), "gate_thru == gate (copia exata)");
        check(out[1].at(0, i) == pitch.at(0, i), "pitch_thru == pitch (copia exata)");
    }
}

void testDeterminism() {
    NoteOut a, b;
    a.prepare(kSr, kBlock);
    b.prepare(kSr, kBlock);
    AudioBlock gate(kSr, 1, kBlock), pitch(kSr, 1, kBlock);
    std::vector<const AudioBlock*> in{&gate, &pitch, nullptr, nullptr};
    std::vector<AudioBlock> oa(2, AudioBlock(kSr, 1, kBlock)), ob(2, AudioBlock(kSr, 1, kBlock));
    bool same = true;
    for (int step = 0; step < 20; ++step) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            gate.at(0, i) = (step % 4 < 2) ? 1.0f : 0.0f;
            pitch.at(0, i) = 0.1f * step;
        }
        a.process(in, oa);
        b.process(in, ob);
        for (std::size_t i = 0; i < kBlock; ++i)
            if (oa[0].at(0, i) != ob[0].at(0, i) || oa[1].at(0, i) != ob[1].at(0, i))
                same = false;
    }
    EXPECT(same);
}

void testPanel() {
    NoteOut n;
    const std::string problem = validatePanel(n);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    std::cout << renderAscii(n);
}

}  // namespace

int main() {
    testNoCompletedNoteWithoutGate();
    testDetectsNoteOnOff();
    testVelocityAndAccentSampledAtNoteOn();
    testThruPassesGateAndPitchUnchanged();
    testDeterminism();
    testPanel();
    if (g_failures == 0) {
        std::cout << "RASGO Modular NOTE-OUT tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
