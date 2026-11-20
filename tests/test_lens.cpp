// LENS (#95) — foco e desfoque. Critérios de `dossies/95_lens.md`.

#include "dsp/Lens.hpp"

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

struct Out { std::vector<float> l, r, x; };

Out render(float focus, float blur, float depth, int kind, double seconds) {
    Lens m;
    m.setParameter("focus", focus); m.setParameter("blur", blur); m.setParameter("depth", depth);
    m.prepare(kSr, kB);
    AudioBlock in(kSr, 1, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    Out o; std::uint32_t s = 7; std::size_t n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n) {
            s = s * 1664525u + 1013904223u;
            float v = 0;
            if (kind == 0) v = 0.3f * (float(s >> 8) / 8388608.0f - 1.0f);           // ruído
            else if (kind == 1) v = (n % 24000 == 100) ? 1.0f : 0.0f;                // cliques
            in.at(0, i) = v;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) { o.x.push_back(in.at(0, i)); o.l.push_back(out[0].at(0, i)); o.r.push_back(out[1].at(0, i)); }
    }
    return o;
}

void testTransparent() {
    const auto o = render(800.0f, 0.0f, 1.0f, 0, 1.0);
    double e = 0; for (std::size_t i = 0; i < o.x.size(); ++i) e = std::max(e, double(std::fabs(o.l[i] - o.x[i])));
    std::printf("BLUR 0: maior diferença entre entrada e saída %.2e\n", e);
    check(e < 1e-6, "sem desfoque, a lente é transparente");
}

// quanto da energia de um clique chega depois de 20 ms (o espalhamento)
double smear(const std::vector<float>& y) {
    double early = 0, late = 0;
    for (std::size_t i = 24000; i < 48000; ++i) {
        const double v = double(y[i]) * y[i];
        if (i < 24100 + 960) early += v; else late += v;
    }
    return late / (early + late + 1e-12);
}

void testDefocusSmears() {
    const auto sharp = render(800.0f, 0.0f, 1.0f, 1, 1.0), soft = render(800.0f, 1.0f, 0.25f, 1, 1.0);
    const double a = smear(sharp.l), b = smear(soft.l);
    std::printf("clique: energia depois de 20 ms — nítido %.3f · fora de foco %.3f\n", a, b);
    check(b > a + 0.05, "fora de foco, o ataque se espalha no tempo");
}

void testDepthOfField() {
    check(Lens::defocus(3, Lens::bandHz(3), 1.0f) == 0.0f, "a faixa do foco fica nítida");
    check(Lens::defocus(7, Lens::bandHz(0), 1.0f) > 0.9f, "longe do foco, desfoque quase total");
    check(Lens::defocus(5, Lens::bandHz(3), 4.0f) < Lens::defocus(5, Lens::bandHz(3), 0.5f),
          "mais profundidade de campo, menos desfoque na mesma distância");
}

void testWidth() {
    const auto o = render(800.0f, 1.0f, 0.25f, 0, 2.0);
    double ll = 0, rr = 0, lr = 0;
    bool fin = true;
    for (std::size_t i = 24000; i < o.l.size(); ++i) {
        ll += double(o.l[i]) * o.l[i]; rr += double(o.r[i]) * o.r[i]; lr += double(o.l[i]) * o.r[i];
        if (!std::isfinite(o.l[i]) || std::fabs(o.l[i]) > 2.0f) fin = false;
    }
    const double corr = lr / std::sqrt(ll * rr);
    std::printf("ruído fora de foco: correlação L·R %.3f\n", corr);
    check(corr < 0.98, "o desfoque abre o estéreo");
    check(fin, "finito e com nível contido");
}

}  // namespace

int main() {
    testTransparent();
    testDefocusSmears();
    testDepthOfField();
    testWidth();
    if (g_failures == 0) std::printf("test_lens: ok\n");
    return g_failures == 0 ? 0 : 1;
}
