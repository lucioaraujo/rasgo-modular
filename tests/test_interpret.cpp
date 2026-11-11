// INTERPRET (#87) — o intérprete melódico. Critérios de `dossies/87_interpret.md`.

#include "dsp/Interpret.hpp"

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
constexpr std::size_t kB = 240;

struct Note { float semi; double dur; double gate; };   // dur total, gate = tempo alto
struct Trace { std::vector<float> p, a; std::vector<int> gest; };

Trace play(Interpret& m, const std::vector<Note>& ns) {
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock pitch(kSr, 1, kB), gate(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&pitch, &gate, nullptr};
    Trace t;
    for (const auto& n : ns) {
        const int blocks = int(n.dur * kSr / kB + 0.5), on = int(n.gate * kSr / kB + 0.5);
        for (int b = 0; b < blocks; ++b) {
            for (std::size_t i = 0; i < kB; ++i) { pitch.at(0, i) = n.semi / 12.0f; gate.at(0, i) = b < on ? 1.0f : 0.0f; }
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) { t.p.push_back(out[0].at(0, i)); t.a.push_back(out[1].at(0, i)); }
        }
        t.gest.push_back(int(m.gesture()));
    }
    return t;
}

Interpret make() {
    Interpret m;
    m.setParameter("drift", 0.0f);
    m.prepare(kSr, kB);
    return m;
}

// tempo (s) para a altura chegar a 90 % do caminho
double settle(const std::vector<float>& p, std::size_t from, float a, float b) {
    for (std::size_t i = from; i < p.size(); ++i)
        if (std::fabs(p[i] - b) <= 0.1f * std::fabs(b - a)) return double(i - from) / kSr;
    return 1e9;
}

void testGlideAndOvershoot() {
    auto m = make();
    const auto t = play(m, {{0, 0.5, 0.45}, {2, 0.5, 0.45}, {0, 0.5, 0.45}, {12, 0.5, 0.45}});
    const std::size_t s1 = std::size_t(0.5 * kSr), s3 = std::size_t(1.5 * kSr);
    const double g2 = settle(t.p, s1, 0.0f, 2 / 12.0f), g12 = settle(t.p, s3, 0.0f, 1.0f);
    float peak = 0.0f; for (std::size_t i = s3; i < s3 + std::size_t(0.45 * kSr); ++i) peak = std::max(peak, t.p[i]);
    std::printf("glide: 2 st %.3f s · 12 st %.3f s · pico do salto %.3f (alvo 1,000)\n", g2, g12, peak);
    check(g12 > g2, "GLIDE: o salto grande desliza mais devagar que o passo");
    check(peak > 1.0f, "overshoot: o salto passa do alvo e assenta");
}

void testVibratoOnlyOnLongNotes() {
    auto m = make();
    const auto t = play(m, {{0, 0.15, 0.12}, {0, 1.2, 1.15}});
    float shortDev = 0, longDev = 0;
    for (std::size_t i = 0; i < std::size_t(0.15 * kSr); ++i) shortDev = std::max(shortDev, std::fabs(t.p[i]));
    for (std::size_t i = std::size_t(0.15 * kSr + 0.8 * kSr); i < std::size_t(0.15 * kSr + 1.1 * kSr); ++i)
        longDev = std::max(longDev, std::fabs(t.p[i]));
    std::printf("vibrato: curta %.5f · longa %.5f oct\n", shortDev, longDev);
    check(shortDev < 1e-4f && longDev > 0.002f, "VIB: progressivo, só nas notas longas");
}

void testArticulationByContext() {
    auto m = make();
    // rápidas → staccato; salto grande → marcato; ápice → tenuto
    const auto t = play(m, {{0, 0.12, 0.1}, {2, 0.12, 0.1}, {0, 0.12, 0.1}, {2, 0.12, 0.1},
                           {-9, 0.5, 0.4}, {-7, 0.5, 0.4}, {5, 0.5, 0.4}});
    std::printf("gestos:");
    for (int g : t.gest) std::printf(" %d", g);
    std::printf("\n");
    check(t.gest[2] == int(Interpret::Gesture::Staccato), "rápidas: staccato");
    check(t.gest[4] == int(Interpret::Gesture::Marcato), "salto grande: marcato");
    check(t.gest[6] == int(Interpret::Gesture::Tenuto) || t.gest[6] == int(Interpret::Gesture::Marcato),
          "o ápice: tenuto (ou marcato, se for salto)");
}

void testExpressionZeroIsRaw() {
    Interpret m;
    m.setParameter("expression", 0.0f); m.setParameter("drift", 0.0f);
    m.prepare(kSr, kB);
    const auto t = play(m, {{0, 0.5, 0.45}, {7, 0.5, 0.45}});
    check(std::fabs(t.p[std::size_t(0.5 * kSr) + 10] - 7.0f / 12.0f) < 1e-4f, "EXPR 0: a nota crua, sem glide");
}

}  // namespace

int main() {
    testGlideAndOvershoot();
    testVibratoOnlyOnLongNotes();
    testArticulationByContext();
    testExpressionZeroIsRaw();
    if (g_failures == 0) std::printf("test_interpret: ok\n");
    return g_failures == 0 ? 0 : 1;
}
