// KEY (#73) — o tom de quem toca. Critérios do dossiê `dossies/73_key.md`.

#include "dsp/Key.hpp"

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
constexpr std::size_t kB = 256;

struct Note { int pc; int blocks; };
struct Run { int changes = 0; float root = 0, scale = 0, cert = 0; };

Run play(Key& m, const std::vector<Note>& notes, int reps = 1) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock pitch(kSr, 1, kB), gate(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&pitch, &gate, nullptr};
    Run r;
    float prev = 0.0f;
    for (int rep = 0; rep < reps; ++rep)
        for (const auto& n : notes)
            for (int b = 0; b < n.blocks; ++b) {
                for (std::size_t i = 0; i < kB; ++i) {
                    pitch.at(0, i) = static_cast<float>(n.pc) / 12.0f;
                    gate.at(0, i) = (b == 0 && i < kB / 2) ? 1.0f : (b == 0 ? 0.0f : 0.0f);
                }
                m.process(ins, out);
                for (std::size_t i = 0; i < kB; ++i) {
                    if (prev < 0.5f && out[3].at(0, i) >= 0.5f) ++r.changes;
                    prev = out[3].at(0, i);
                }
                r.root = out[0].at(0, kB - 1);
                r.scale = out[1].at(0, kB - 1);
                r.cert = out[2].at(0, kB - 1);
            }
    return r;
}

// Dó maior: tônica e dominante longas, o resto de passagem
const std::vector<Note> kCmaj{{0, 8}, {2, 2}, {4, 4}, {5, 2}, {7, 8}, {9, 2}, {11, 2}, {0, 8}};
// Lá menor (natural): Lá e Mi longos, Dó presente
const std::vector<Note> kAmin{{9, 8}, {11, 2}, {0, 4}, {2, 2}, {4, 8}, {5, 2}, {7, 2}, {9, 8}};

void testCMajor() {
    Key m; m.prepare(kSr, kB);
    const Run r = play(m, kCmaj, 3);
    check(m.tonic() == 0 && !m.minor(), "uma melodia em Dó maior dá Dó maior");
    check(std::fabs(r.root - 0.0f) < 1e-6f && std::fabs(r.scale - 1.0f / 11.0f) < 1e-6f,
          "ROOT/SCL no formato do QUANTIZER (Dó = 0, Maior = 1/11)");
    check(r.cert > 0.0f, "com certeza acima de zero");
}

void testAMinor() {
    Key m; m.prepare(kSr, kB);
    const Run r = play(m, kAmin, 3);
    check(m.tonic() == 9 && m.minor(), "uma melodia em Lá menor dá Lá menor");
    check(std::fabs(r.root - 9.0f / 12.0f) < 1e-6f && std::fabs(r.scale - 2.0f / 11.0f) < 1e-6f,
          "ROOT = 9/12 e SCL = 2/11 (menor natural)");
}

void testModulationWithHysteresis() {
    Key m;
    m.setParameter("memory", 4.0f);
    m.setParameter("stability", 4.0f);
    m.prepare(kSr, kB);
    play(m, kCmaj, 3);
    check(m.tonic() == 0, "começa em Dó");
    // a mesma melodia uma quinta acima: Sol maior
    std::vector<Note> g;
    for (const auto& n : kCmaj) g.push_back({(n.pc + 7) % 12, n.blocks});
    const Run r = play(m, g, 6);
    check(m.tonic() == 7 && !m.minor(), "depois de tocar em Sol, vai para Sol maior");
    check(r.changes >= 1 && r.changes <= 2, "e muda poucas vezes (histerese), com pulso em MUD");
}

void testChromaticIsUncertain() {
    Key m; m.prepare(kSr, kB);
    std::vector<Note> chroma;
    for (int k = 0; k < 12; ++k) chroma.push_back({k, 3});
    const Run r = play(m, chroma, 3);
    check(r.cert < 0.5f, "a escala cromática por igual deixa a certeza baixa");
}

void testResetForgets() {
    Key m; m.prepare(kSr, kB);
    play(m, kAmin, 2);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock rst(kSr, 1, kB);
    for (std::size_t i = 0; i < kB; ++i) rst.at(0, i) = 1.0f;
    std::vector<const AudioBlock*> ins{nullptr, nullptr, &rst};
    m.process(ins, out);
    check(m.tonic() == -1, "RST esquece o tom");
}

}  // namespace

int main() {
    testCMajor();
    testAMinor();
    testModulationWithHysteresis();
    testChromaticIsUncertain();
    testResetForgets();
    if (g_failures == 0) std::printf("test_key: OK\n");
    return g_failures == 0 ? 0 : 1;
}
