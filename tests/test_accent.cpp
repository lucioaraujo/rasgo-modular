// ACCENT (#75) — o campo de acento. Critérios do dossiê `dossies/75_accent.md`.

#include "dsp/Accent.hpp"

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
constexpr std::size_t kB = 240;   // 5 ms

struct Note { float pitch; int blocks; float acc = 0.0f; };
struct Out { float emph, vca, fc, dec, sweepPeak, level; bool gt; };

// cada nota: gate alto no 1º bloco; lê as saídas no fim da nota
std::vector<Out> play(Accent& m, const std::vector<Note>& notes) {
    std::vector<AudioBlock> out(7, AudioBlock(kSr, 1, kB));
    AudioBlock gate(kSr, 1, kB), pitch(kSr, 1, kB), acc(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&gate, &pitch, &acc, nullptr};
    std::vector<Out> r;
    for (const auto& n : notes) {
        Out o{};
        bool gtSeen = false;
        for (int b = 0; b < n.blocks; ++b) {
            for (std::size_t i = 0; i < kB; ++i) {
                gate.at(0, i) = b == 0 ? 1.0f : 0.0f;
                pitch.at(0, i) = n.pitch;
                acc.at(0, i) = b == 0 ? n.acc : 0.0f;
            }
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) {
                o.sweepPeak = std::max(o.sweepPeak, out[4].at(0, i));
                gtSeen = gtSeen || out[5].at(0, i) >= 0.5f;
            }
        }
        o.emph = out[0].at(0, kB - 1);
        o.vca = out[1].at(0, kB - 1);
        o.fc = out[2].at(0, kB - 1);
        o.dec = out[3].at(0, kB - 1);
        o.level = out[6].at(0, kB - 1);
        o.gt = gtSeen;
        r.push_back(o);
    }
    return r;
}

Accent make(float metric, float leap, float apex, float ext, float agogic, float synco,
            float cycle, float inherit, float suppress) {
    Accent m;
    m.setParameter("metric", metric);
    m.setParameter("leap", leap);
    m.setParameter("apex", apex);
    m.setParameter("external", ext);
    m.setParameter("agogic", agogic);
    m.setParameter("synco", synco);
    m.setParameter("cycle", cycle);
    m.setParameter("inherit", inherit);
    m.setParameter("suppress", suppress);
    m.setParameter("rotate", 0.0f);
    m.setParameter("depth", 1.0f);
    m.prepare(kSr, kB);
    return m;
}

void testMetricHierarchy() {
    check(Accent::metricWeight(0, 8) == 1.0f, "tempo forte = 1");
    check(std::fabs(Accent::metricWeight(4, 8) - 0.5f) < 1e-6f, "ciclo de 8: o 5º pesa ½");
    check(std::fabs(Accent::metricWeight(2, 8) - 0.25f) < 1e-6f, "ciclo de 8: o 3º pesa ¼");
    check(std::fabs(Accent::metricWeight(3, 6) - 0.5f) < 1e-6f, "ciclo de 6: 3+3");
    auto m = make(1, 0, 0, 0, 0, 0, 4, 0, 0);
    std::vector<Note> ns(8, Note{0.0f, 20});
    const auto r = play(m, ns);
    check(r[0].emph > 0.99f && r[4].emph > 0.99f, "MÉTR: o 1º de cada ciclo é forte");
    check(r[1].emph < 0.3f && r[2].emph > r[1].emph, "MÉTR: hierarquia dentro do ciclo");
}

void testLeapAndApex() {
    auto m = make(0, 1, 0, 0, 0, 0, 4, 0, 0);
    const auto r = play(m, {{0.0f, 20}, {1.0f / 12, 20}, {1.0f, 20}, {1.0f, 20}});
    check(r[1].emph < 0.15f, "SALTO: um semitom quase não pesa");
    check(r[2].emph > 0.85f, "SALTO: uma oitava ou mais pesa inteiro");
    check(r[3].emph < 0.01f, "SALTO: nota repetida não pesa");
    auto a = make(0, 0, 1, 0, 0, 0, 4, 0, 0);
    const auto q = play(a, {{0.2f, 20}, {0.1f, 20}, {0.5f, 20}, {0.3f, 20}});
    check(q[2].emph > 0.99f, "ÁPICE: a mais aguda das recentes");
    check(q[1].emph < 0.01f && q[3].emph < 0.01f, "ÁPICE: as outras não");
}

void testExternalSyncoAgogic() {
    auto m = make(0, 0, 0, 1, 0, 0, 4, 0, 0);
    const auto r = play(m, {{0, 20, 1.0f}, {0, 20, 0.0f}});
    check(r[0].emph > 0.99f && r[1].emph < 0.01f, "EXT: o acento de fora passa");
    auto s = make(0, 0, 0, 0, 0, 1, 4, 0, 0);
    const auto q = play(s, {{0, 20, 1.0f}, {0, 20, 1.0f}});
    check(q[0].emph < 0.01f, "SÍNC: acento de fora no tempo forte não é síncope");
    check(q[1].emph > 0.7f, "SÍNC: acento de fora no tempo fraco é");
    auto g = make(0, 0, 0, 0, 1, 0, 4, 0, 0);
    const auto t = play(g, {{0, 20}, {0, 20}, {0, 20}, {0, 80}, {0, 20}});
    check(t[4].emph > 0.5f, "AGÓG: a nota depois de uma longa é acentuada");
    check(t[2].emph < 0.01f, "AGÓG: durações iguais não acentuam");
}

void testInheritAndSuppress() {
    // herança: a nota depois do forte guarda uma cauda que desbota
    auto h = make(1, 0, 0, 0, 0, 0, 8, 1, 0);
    const auto r = play(h, {{0, 20}, {0, 20}, {0, 20}});
    check(r[1].emph > r[2].emph && r[1].emph > 0.4f, "HERANÇA: a cauda passa e desbota");
    // supressão: dois acentos de fora seguidos — o segundo perde
    auto s = make(0, 0, 0, 1, 0, 0, 4, 0, 1);
    const auto q = play(s, {{0, 20, 1.0f}, {0, 20, 1.0f}});
    check(q[1].emph < 0.2f, "SUPRESSÃO: o acento logo depois de um forte custa");
}

void testOutputsAndStacking() {
    auto m = make(0, 0, 0, 1, 0, 0, 4, 0, 0);
    // acentos seguidos e curtos (60 ms) empilham; um isolado não
    const auto r = play(m, {{0, 12, 1}, {0, 12, 1}, {0, 12, 1}, {0, 200, 0}, {0, 12, 1}});
    std::printf("SWP: %.2f %.2f %.2f … isolado %.2f\n",
                r[0].sweepPeak, r[1].sweepPeak, r[2].sweepPeak, r[4].sweepPeak);
    check(r[2].sweepPeak > r[0].sweepPeak + 0.2f && r[2].sweepPeak > r[1].sweepPeak, "SWP: acentos seguidos empilham aos poucos");
    check(std::fabs(r[4].sweepPeak - r[0].sweepPeak) < 0.05f, "SWP: um acento sozinho não");
    check(std::fabs(r[0].vca - 1.0f) < 1e-4f && std::fabs(r[3].vca - 0.6f) < 1e-4f,
          "VCA: 0,6 + 0,4·ênf");
    check(std::fabs(r[0].fc - 2.0f) < 1e-4f && std::fabs(r[0].dec + 2.0f) < 1e-4f,
          "FC +2 oitavas, DEC −2 em ênf 1");
    check(r[0].gt && !r[3].gt, "GT só nos acentos");
    check(r[0].level == 1.0f && r[3].level == 0.0f, "NÍV em degraus");
}

void testRotation() {
    Accent m;
    m.setParameter("metric", 1.0f);
    m.setParameter("leap", 0.0f); m.setParameter("apex", 0.0f);
    m.setParameter("external", 0.0f); m.setParameter("agogic", 0.0f);
    m.setParameter("synco", 0.0f); m.setParameter("inherit", 0.0f);
    m.setParameter("suppress", 0.0f); m.setParameter("cycle", 4.0f);
    m.setParameter("rotate", 1.0f);
    m.prepare(kSr, kB);
    std::vector<Note> ns(12, Note{0.0f, 10});
    const auto r = play(m, ns);
    // ROT 1: o forte anda uma posição a cada ciclo
    check(r[0].emph > 0.99f && r[4].emph < 0.99f, "ROT: o tempo forte migra");
    check(m.rotation() > 0, "ROT: a rotação avançou");
}

}  // namespace

int main() {
    testMetricHierarchy();
    testLeapAndApex();
    testExternalSyncoAgogic();
    testInheritAndSuppress();
    testOutputsAndStacking();
    testRotation();
    if (g_failures == 0) std::printf("test_accent: ok\n");
    return g_failures == 0 ? 0 : 1;
}
