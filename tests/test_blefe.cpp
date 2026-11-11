// BLEFE (#76) — o jogo do sinal. Critérios do dossiê `dossies/76_blefe.md`.

#include "dsp/Blefe.hpp"

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

struct Count { int sig = 0, truth = 0, conf = 0, resp = 0, bet = 0; float trust = 0, lastResp = 0; };

// `n` sinais, um a cada `gap` blocos
Count run(Blefe& m, int n, int gap, bool resetMid = false) {
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock in(kSr, 1, kB), rst(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, &rst};
    Count c;
    float prev[6] = {0, 0, 0, 0, 0, 0};
    for (int s = 0; s < n; ++s)
        for (int b = 0; b < gap; ++b) {
            for (std::size_t i = 0; i < kB; ++i) {
                in.at(0, i) = (b == 0 && i < 24) ? 1.0f : 0.0f;
                rst.at(0, i) = (resetMid && s == n / 2 && b == 1 && i < 24) ? 1.0f : 0.0f;
            }
            m.process(ins, out);
            for (std::size_t i = 0; i < kB; ++i) {
                const float v[6] = {out[0].at(0, i), out[1].at(0, i), out[2].at(0, i),
                                    out[3].at(0, i), out[4].at(0, i), out[5].at(0, i)};
                if (prev[0] < 0.5f && v[0] >= 0.5f) ++c.sig;
                if (prev[1] < 0.5f && v[1] >= 0.5f) ++c.truth;
                if (prev[2] < 0.5f && v[2] >= 0.5f) ++c.conf;
                if (prev[4] <= 0.0f && v[4] > 0.0f) { ++c.resp; c.lastResp = v[4]; }
                if (prev[5] < 0.5f && v[5] >= 0.5f) ++c.bet;
                for (int k = 0; k < 6; ++k) prev[k] = v[k];
            }
            c.trust = out[3].at(0, kB - 1);
        }
    return c;
}

Blefe make(float truth, float seed = 0.0f) {
    Blefe m;
    m.setParameter("truthfulness", truth);
    m.setParameter("window", 0.1f);
    m.setParameter("memory", 0.2f);
    m.setParameter("threshold", 0.35f);
    m.setParameter("seed", seed);
    m.prepare(kSr, kB);
    return m;
}

void testSignalAlwaysTheSame() {
    auto m = make(0.5f);
    const auto c = run(m, 200, 40);   // janela 0,1 s < 0,2 s entre sinais
    check(c.sig == 200, "SIG: todo gatilho vira um sinal, verdade ou blefe");
    const double rate = double(c.truth) / c.sig;
    std::printf("VERD 0,5: %d verdades de %d (%.2f)\n", c.truth, c.sig, rate);
    check(rate > 0.4 && rate < 0.6, "VERD: a taxa de verdade segue o knob");
    check(c.conf == c.truth, "CONF: só a verdade confirma");
}

void testTrustFollowsHonesty() {
    auto honest = make(1.0f);
    const auto h = run(honest, 40, 40);
    check(h.trust > 0.95f && h.bet == 0, "honesto: confiança alta, sem traição");
    auto liar = make(0.0f);
    const auto l = run(liar, 40, 40);
    check(l.trust < 0.05f, "mentiroso: confiança baixa");
    check(l.bet == 0, "sem confiança construída, não há traição");
}

void testBetrayalAndForgiveness() {
    // honesto primeiro (constrói), depois mente: trai uma vez e se recusa
    auto m = make(1.0f);
    run(m, 30, 40);
    m.setParameter("truthfulness", 0.0f);
    const auto c = run(m, 30, 40);
    check(c.bet == 1, "TRAI: uma traição, depois de a confiança existir");
    const auto after = run(m, 10, 40);
    check(m.betrayed() && after.resp == 0, "RESP: recusa depois da traição");
    // RST: o perdão
    m.setParameter("truthfulness", 1.0f);
    const auto f = run(m, 30, 40, true);
    check(!m.betrayed() && f.lastResp > 0.5f, "RST: o perdão devolve a resposta");
}

void testDeterministic() {
    auto a = make(0.6f, 7.0f), b = make(0.6f, 7.0f), c = make(0.6f, 8.0f);
    const auto ra = run(a, 100, 40), rb = run(b, 100, 40), rc = run(c, 100, 40);
    check(ra.truth == rb.truth && ra.trust == rb.trust, "mesma SEED, mesmo jogo");
    check(ra.truth != rc.truth || ra.trust != rc.trust, "SEED diferente, outro jogo");
}

}  // namespace

int main() {
    testSignalAlwaysTheSame();
    testTrustFollowsHonesty();
    testBetrayalAndForgiveness();
    testDeterministic();
    if (g_failures == 0) std::printf("test_blefe: ok\n");
    return g_failures == 0 ? 0 : 1;
}
