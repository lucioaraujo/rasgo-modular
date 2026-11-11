// CUT-UP (#85) — o recorte do Navalha num buffer ao vivo. Critérios de
// `dossies/85_cutup.md`.

#include "dsp/CutUp.hpp"

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

// entrada: uma rampa que marca o tempo (o valor diz de quando é a amostra)
struct R { std::vector<float> y, act; };

R run(CutUp& m, double seconds, int gesture, double at) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock in(kSr, 1, kB), clk(kSr, 1, kB), g(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, &clk, nullptr, nullptr, nullptr, nullptr};
    ins[static_cast<std::size_t>(2 + gesture)] = &g;
    R r;
    long n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n) {
            const double t = double(n) / kSr;
            in.at(0, i) = float(std::fmod(t, 10.0) / 10.0);   // rampa lenta
            clk.at(0, i) = (n % 24000) < 240 ? 1.0f : 0.0f;    // 120 BPM
            g.at(0, i) = (std::fabs(t - at) < 0.0026) ? 1.0f : 0.0f;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) { r.y.push_back(out[0].at(0, i)); r.act.push_back(out[1].at(0, i)); }
    }
    return r;
}

void testStutterRepeatsLastBeat() {
    CutUp m;
    const auto r = run(m, 3.0, 0, 2.0);
    // durante o gesto (4 × 125 ms), a saída volta ao valor de ~0,5 s antes
    long on = 0; bool past = true;
    for (std::size_t i = 0; i < r.y.size(); ++i)
        if (r.act[i] > 0.5f) {
            ++on;
            const double t = double(i) / kSr;
            const double frag = std::fmod(t - 2.0, 0.125);
            if (frag > 0.005 && frag < 0.12 && r.y[i] > float((t - 0.4) / 10.0)) past = false;
        }
    std::printf("STUT: %.0f ms ativo\n", 1000.0 * on / kSr);
    check(std::fabs(on / kSr - 0.5) < 0.01, "STUT: 4 × tempo/4 = um tempo inteiro");
    check(past, "STUT: o que soa é o último tempo, repetido");
}

void testGapIsSilentWithEdges() {
    CutUp m;
    const auto r = run(m, 3.0, 3, 2.0);
    float mid = 1.0f, jump = 0.0f;
    for (std::size_t i = 1; i < r.y.size(); ++i) {
        const double t = double(i) / kSr;
        if (t > 2.1 && t < 2.4) mid = std::min(mid, std::fabs(r.y[i]) < 1e-6f ? 0.0f : 1.0f);
        jump = std::max(jump, std::fabs(r.y[i] - r.y[i - 1]));
    }
    check(mid == 0.0f, "GAP: um tempo de silêncio");
    std::printf("GAP: maior salto %.4f\n", jump);
    check(jump < 0.2f, "GAP: bordas de 3 ms, sem corte seco");
}

void testBurstUsesSeveralBeats() {
    CutUp m;
    const auto r = run(m, 4.0, 1, 3.5);
    float lo = 1e9f, hi = -1e9f;
    for (std::size_t i = 0; i < r.y.size(); ++i)
        if (r.act[i] > 0.5f) { lo = std::min(lo, r.y[i]); hi = std::max(hi, r.y[i]); }
    std::printf("BURST: fatias de %.3f a %.3f (a rampa vale %.3f agora)\n", lo, hi, 0.35);
    check(hi - lo > 0.1f, "BURST: fatias de vários tempos atrás, embaralhadas");
}

}  // namespace

int main() {
    testStutterRepeatsLastBeat();
    testGapIsSilentWithEdges();
    testBurstUsesSeveralBeats();
    if (g_failures == 0) std::printf("test_cutup: ok\n");
    return g_failures == 0 ? 0 : 1;
}
