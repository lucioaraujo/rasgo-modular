// COHERENCE (#72) — a ordem que se rompe. Critérios do dossiê
// `dossies/72_coherence.md`.

#include "dsp/Coherence.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 64;

// toca uma sequência de classes de altura, uma nota por bloco (gate no
// começo do bloco); devolve quantos pulsos de COLAPSO saíram
struct Run { int pulses = 0; float lastOrder = 0, lastPressure = 0; bool finite = true; };
Run play(Coherence& m, const std::vector<int>& pcs, const bool useGate = true) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock pitch(kSr, 1, kB), gate(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&pitch, useGate ? &gate : nullptr, nullptr};
    Run r;
    float prevCol = 0.0f;
    for (const int pc : pcs) {
        for (std::size_t i = 0; i < kB; ++i) {
            pitch.at(0, i) = static_cast<float>(pc) / 12.0f;
            gate.at(0, i) = i < kB / 2 ? 1.0f : 0.0f;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            const float c = out[2].at(0, i);
            if (prevCol < 0.5f && c >= 0.5f) ++r.pulses;
            prevCol = c;
            for (int k = 0; k < 4; ++k)
                if (!std::isfinite(out[static_cast<std::size_t>(k)].at(0, i))) r.finite = false;
        }
        // o pulso dura 10 ms (> 1 bloco de 64): espera ele acabar
        for (int extra = 0; extra < 10; ++extra) {
            for (std::size_t i = 0; i < kB; ++i) gate.at(0, i) = 0.0f;
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) {
                const float c = out[2].at(0, i);
                if (prevCol < 0.5f && c >= 0.5f) ++r.pulses;
                prevCol = c;
            }
        }
        r.lastOrder = out[0].at(0, kB - 1);
        r.lastPressure = out[1].at(0, kB - 1);
    }
    return r;
}

void testRepetitionBreaks() {
    Coherence m;
    m.setParameter("threshold", 0.7f);
    m.setParameter("patience", 6.0f);
    m.setParameter("scar", 4.0f);
    m.prepare(kSr, kB);
    // a mesma nota: a ordem vai a 1; a ruptura vem depois de PAC notas acima
    // do limiar (a 2ª nota já tem ordem 1)
    const Run r = play(m, std::vector<int>(7, 3));
    check(r.pulses == 1, "nota repetida: rompe uma vez em 7 notas (PAC 6)");
    check(r.finite, "saídas finitas");
}

void testVarietyDoesNotBreak() {
    Coherence m;
    m.prepare(kSr, kB);
    std::vector<int> all;
    for (int rep = 0; rep < 4; ++rep)
        for (int k = 0; k < 12; ++k) all.push_back((k * 7) % 12);   // as 12 por igual
    const Run r = play(m, all);
    check(r.pulses == 0, "as 12 notas por igual não rompem");
    check(r.lastOrder < 0.2f, "e a ordem fica baixa");
}

void testScarHolds() {
    Coherence m;
    m.setParameter("patience", 2.0f);
    m.setParameter("scar", 6.0f);
    m.prepare(kSr, kB);
    // 1ª ruptura na 3ª nota; com cicatriz 6, a próxima só depois de 6 notas
    // sem pressão + 2 de paciência
    const Run r = play(m, std::vector<int>(10, 5));
    check(r.pulses == 1, "a cicatriz segura uma nova ruptura logo em seguida");
    Coherence m2;
    m2.setParameter("patience", 2.0f);
    m2.setParameter("scar", 0.0f);
    m2.prepare(kSr, kB);
    const Run r2 = play(m2, std::vector<int>(10, 5));
    check(r2.pulses >= 3, "sem cicatriz, rompe de novo e de novo");
}

void testWithoutGateCountsChanges() {
    Coherence m;
    // um ostinato de 2 notas tem ordem 0,72 (previsível) e romperia: o
    // LIMIAR em 1 deixa este teste medir só a contagem das notas
    m.setParameter("threshold", 1.0f);
    m.prepare(kSr, kB);
    // sem GATE, uma nota conta quando a classe muda: alternar 2 notas
    std::vector<int> seq;
    for (int i = 0; i < 16; ++i) seq.push_back(i % 2 ? 0 : 7);
    play(m, seq, false);
    check(m.notes() >= 12, "sem GATE, cada mudança de classe é uma nota");
}

void testResetForgets() {
    Coherence m;
    m.prepare(kSr, kB);
    play(m, std::vector<int>(4, 9));
    check(m.notes() == 4, "4 notas na janela");
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock rst(kSr, 1, kB);
    for (std::size_t i = 0; i < kB; ++i) rst.at(0, i) = 1.0f;
    std::vector<const AudioBlock*> ins{nullptr, nullptr, &rst};
    m.process(ins, out);
    check(m.notes() == 0 && m.pressure() == 0.0f, "RST esquece a janela e a pressão");
}

}  // namespace

int main() {
    testRepetitionBreaks();
    testVarietyDoesNotBreak();
    testScarHolds();
    testWithoutGateCountsChanges();
    testResetForgets();
    if (g_failures == 0) std::printf("test_coherence: OK\n");
    return g_failures == 0 ? 0 : 1;
}
