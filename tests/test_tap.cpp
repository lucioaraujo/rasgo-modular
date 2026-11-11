// TAP (#78) — o andamento batido por quem toca. Critérios de `dossies/78_tap.md`.

#include "dsp/Tap.hpp"

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

struct Out { std::vector<double> clk, one; };

// batidas nos instantes `taps` (s); roda até `seconds`
Out run(Tap& m, const std::vector<double>& taps, double seconds) {
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock in(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, nullptr};
    Out o;
    float pc = 0, po = 0;
    const long n = long(seconds * kSr);
    for (long done = 0; done < n; done += long(kB)) {
        for (std::size_t i = 0; i < kB; ++i) {
            const long s = done + long(i);
            float v = 0.0f;
            for (const double t : taps) if (s >= long(t * kSr) && s < long(t * kSr) + 240) v = 1.0f;
            in.at(0, i) = v;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            const double t = double(done + long(i)) / kSr;
            if (pc < 0.5f && out[0].at(0, i) >= 0.5f) o.clk.push_back(t);
            if (po < 0.5f && out[1].at(0, i) >= 0.5f) o.one.push_back(t);
            pc = out[0].at(0, i); po = out[1].at(0, i);
        }
    }
    return o;
}

void testTempoFromTaps() {
    Tap m;
    m.prepare(kSr, kB);
    const auto o = run(m, {1.0, 1.5, 2.0, 2.5}, 6.0);
    std::printf("TAP: %.2f BPM\n", m.bpm());
    check(std::fabs(m.bpm() - 120.0) < 0.5, "4 batidas a 0,5 s = 120 BPM");
    // o relógio segue sozinho: depois de 2,5 s, um tempo a cada 0,5 s
    int after = 0; bool even = true;
    for (std::size_t k = 1; k < o.clk.size(); ++k)
        if (o.clk[k] > 2.6) { ++after; if (std::fabs(o.clk[k] - o.clk[k - 1] - 0.5) > 0.002) even = false; }
    check(after >= 6 && even, "o relógio segue no andamento batido");
    // o "um" é a 1ª batida: UM em 1,0 e a cada 4 tempos (3,0; 5,0)
    bool oneOk = false;
    for (const double t : o.one) if (std::fabs(t - 3.0) < 0.01) oneOk = true;
    check(oneOk, "UM: a 1ª batida é o tempo forte, e volta a cada COMP");
}

void testIrregularRefused() {
    Tap m;
    m.setParameter("spread", 0.2f);
    m.prepare(kSr, kB);
    run(m, {1.0, 1.5, 1.8, 2.2}, 3.0);   // intervalos 0,5 / 0,3 / 0,4
    check(!m.locked(), "batidas irregulares não valem (melhor falhar que inventar)");
}

void testNewCountReplaces() {
    Tap m;
    m.prepare(kSr, kB);
    run(m, {0.5, 1.0, 1.5, 2.0, 6.0, 6.6667, 7.3333, 8.0}, 9.0);
    std::printf("contagem nova: %.2f BPM\n", m.bpm());
    check(std::fabs(m.bpm() - 90.0) < 0.5, "uma contagem nova (depois de uma pausa) troca o andamento");
}

}  // namespace

int main() {
    testTempoFromTaps();
    testIrregularRefused();
    testNewCountReplaces();
    if (g_failures == 0) std::printf("test_tap: ok\n");
    return g_failures == 0 ? 0 : 1;
}
