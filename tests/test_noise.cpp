// Teste isolado do Módulo 19 (NOISE) - antes de entrar num patch.
// Critérios do dossiê `dossies/19_ruido.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Noise.hpp"
#include "dsp/EuclidClock.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;
constexpr std::size_t kBlock = 128;

// renderiza `samples` de uma saída. `trigHz`>0 injeta um trem de pulsos;
// `feedInput` liga a entrada `in` a uma senoide de 3 Hz.
std::vector<float> render(Noise& n, const int outIdx, const int samples,
                          const float trigHz = 0.0f,
                          const bool feedInput = false) {
    n.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(8, AudioBlock(kSr, 1, kBlock));
    AudioBlock tin(kSr, 1, kBlock), sin_(kSr, 1, kBlock);
    std::vector<const AudioBlock*> ins{
        trigHz > 0.0f ? &tin : nullptr,
        feedInput ? &sin_ : nullptr};
    std::vector<float> r;
    long t = 0;
    double trPhase = 0.0;
    const double trStep = trigHz > 0.0f ? trigHz / kSr : 0.0;
    double inPhase = 0.0;
    while (static_cast<int>(r.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            float tr = 0.0f;
            if (trigHz > 0.0f) {
                trPhase += trStep;
                if (trPhase >= 1.0) { trPhase -= 1.0; tr = 1.0f; }
            }
            tin.at(0, i) = tr;
            sin_.at(0, i) = 0.7f * std::sin(inPhase);
            inPhase += 6.2831853 * 3.0 / kSr;
        }
        n.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, i));
        t += static_cast<long>(kBlock);
    }
    return r;
}

double mean(const std::vector<float>& v) {
    double s = 0.0;
    for (const float x : v) s += x;
    return s / static_cast<double>(v.size());
}
double variance(const std::vector<float>& v) {
    const double m = mean(v);
    double s = 0.0;
    for (const float x : v) s += (x - m) * (x - m);
    return s / static_cast<double>(v.size());
}
// energia depois de um passa-baixa / passa-alta de 1 polo simples
double lowEnergy(const std::vector<float>& v, float hz) {
    const float c = std::exp(-6.2831853f * hz / kSr);
    float y = 0.0f;
    double s = 0.0;
    for (std::size_t i = 0; i < v.size(); ++i) {
        y = v[i] * (1.0f - c) + c * y;
        if (i > 2000) s += y * y;
    }
    return s;
}
double highEnergy(const std::vector<float>& v, float hz) {
    const float c = std::exp(-6.2831853f * hz / kSr);
    float y = 0.0f, xPrev = 0.0f;
    double s = 0.0;
    for (std::size_t i = 0; i < v.size(); ++i) {
        y = c * (y + v[i] - xPrev);
        xPrev = v[i];
        if (i > 2000) s += y * y;
    }
    return s;
}
int steps(const std::vector<float>& v, std::size_t a, std::size_t b) {
    int c = 0;
    for (std::size_t i = a + 1; i < b && i < v.size(); ++i)
        if (v[i] != v[i - 1]) ++c;
    return c;
}

void testWhiteFlat() {
    Noise nw, np;
    const auto w = render(nw, 0, 60000);
    const auto pk = render(np, 1, 60000);
    check(std::fabs(mean(w)) < 0.02, "branco tem média ~0");
    // uniforme [-1,1) tem variância 1/3
    check(std::fabs(variance(w) - 0.3333) < 0.04, "branco: variância ~1/3");
    // branco = descorrelacionado: autocorrelação no lag 1 ~ 0
    double ac = 0.0;
    for (std::size_t i = 1; i < w.size(); ++i) ac += w[i] * w[i - 1];
    ac /= static_cast<double>(w.size());
    check(std::fabs(ac) < 0.03, "branco: descorrelacionado (autocorr lag 1 ~0)");
    // branco tem MAIS energia de banda alta que o rosa
    check(highEnergy(w, 4000.0f) > highEnergy(pk, 4000.0f) * 2.0,
          "branco: mais agudo que o rosa");
    for (const float v : w) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testPinkTilt() {
    Noise n;
    const auto p = render(n, 1, 60000);
    const double lo = lowEnergy(p, 500.0f), hi = highEnergy(p, 2000.0f);
    check(lo > hi * 3.0, "rosa: banda baixa domina (~−3 dB/oit)");
    for (const float v : p) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testBrownTilt() {
    Noise n;
    const auto br = render(n, 2, 60000);
    const auto pk = render(n, 1, 60000);
    const double brRatio = lowEnergy(br, 300.0f) / (highEnergy(br, 2000.0f) + 1e-12);
    const double pkRatio = lowEnergy(pk, 300.0f) / (highEnergy(pk, 2000.0f) + 1e-12);
    check(brRatio > pkRatio * 2.0, "brown cai mais forte que o rosa");
    for (const float v : br) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testBlueTilt() {
    Noise n;
    const auto bl = render(n, 5, 60000);
    const auto w = render(n, 0, 60000);
    check(highEnergy(bl, 4000.0f) > highEnergy(w, 4000.0f),
          "azul: mais agudo que o branco (diferenciado)");
    for (const float v : bl) EXPECT(std::isfinite(v) && std::fabs(v) <= 1.0f);
}

void testVioletBrighterThanBlue() {
    Noise n;
    const auto vl = render(n, 6, 60000);
    const auto bl = render(n, 5, 60000);
    check(highEnergy(vl, 6000.0f) > highEnergy(bl, 6000.0f) * 0.8,
          "violeta: ainda mais agudo (ou pelo menos não mais escuro) que o azul");
    for (const float v : vl) EXPECT(std::isfinite(v) && std::fabs(v) <= 1.0f);
}

void testBitHardBipolar() {
    Noise n;
    const auto b = render(n, 7, 20000);
    bool onlyBipolar = true;
    for (const float v : b) if (v != 1.0f && v != -1.0f) onlyBipolar = false;
    check(onlyBipolar, "bit: só assume +-1, nunca valor intermediário");
    // troca rápido, a taxa de áudio (não travado no pulso de trigger)
    check(steps(b, 1000, 19000) > 5000, "bit: alterna rápido, a taxa de áudio");
}

void testSampleHold() {
    Noise n;
    const auto sh = render(n, 3, 48000, 20.0f);   // 20 pulsos/s por 1 s
    const int st = steps(sh, 1000, 47000);
    check(st >= 16 && st <= 24, "S&H muda ~1x por pulso");
    // segura entre pulsos: janelas longas de valor constante
    int constRun = 0, maxRun = 0;
    for (std::size_t i = 2000; i < 46000; ++i) {
        if (sh[i] == sh[i - 1]) ++constRun; else constRun = 0;
        maxRun = std::max(maxRun, constRun);
    }
    check(maxRun > 1200, "S&H realmente SEGURA o valor entre pulsos");
}

void testSHFromInput() {
    Noise n;
    const auto sh = render(n, 3, 48000, 12.0f, /*feedInput=*/true);
    // a entrada é 0.7*sin(2π*3t); o S&H amostrado dela fica em [-0.75, 0.75]
    // e NÃO parece ruído branco (variância menor, valores correlacionados)
    bool inRange = true;
    for (const float v : sh) if (std::fabs(v) > 0.78f) inRange = false;
    check(inRange, "S&H amostra a entrada `in` quando conectada");
    check(variance(sh) < 0.35, "S&H da senoide tem variância baixa");
}

void testSmoothSlew() {
    Noise slow, fast;
    slow.setParameter("slew", 1.0f);
    slow.setParameter("rate", 8.0f);
    fast.setParameter("slew", 0.0f);
    fast.setParameter("rate", 8.0f);
    const auto s = render(slow, 4, 48000);
    const auto f = render(fast, 4, 48000);
    double dSlow = 0.0, dFast = 0.0;
    for (std::size_t i = 3000; i < 46000; ++i) {
        dSlow += std::fabs(s[i] - s[i - 1]);
        dFast += std::fabs(f[i] - f[i - 1]);
    }
    check(dSlow < dFast * 0.2, "slew alto = smooth varia bem devagar");
    check(steps(f, 3000, 46000) < 400, "slew 0 = smooth dá degraus");
}

void testSpread() {
    Noise flat, bell;
    flat.setParameter("spread", 0.0f);
    flat.setParameter("rate", 50.0f);
    bell.setParameter("spread", 1.0f);
    bell.setParameter("rate", 50.0f);
    const auto a = render(flat, 3, 96000);
    const auto b = render(bell, 3, 96000);
    check(variance(b) < variance(a) * 0.75,
          "spread=1 concentra o S&H perto de 0 (variância menor)");
}

void testInternalClock() {
    Noise n;
    n.setParameter("rate", 25.0f);
    const auto sh = render(n, 3, 48000);   // sem trigger -> relógio interno
    const int st = steps(sh, 1000, 47000);
    check(st >= 20 && st <= 30, "relógio interno: ~25 degraus/s");
}

void testDeterminism() {
    Noise a, b;
    for (Noise* n : {&a, &b}) {
        n->setParameter("rate", 17.0f);
        n->setParameter("slew", 0.5f);
        n->setParameter("spread", 0.6f);
    }
    const auto ra = render(a, 4, 40000);
    const auto rb = render(b, 4, 40000);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        if (ra[i] != rb[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 120.0f);
    graph.node(clk).setParameter("mult", 2.0f);
    graph.node(clk).setParameter("fill", 5.0f);
    const auto ns = graph.add(std::make_unique<Noise>());
    graph.connect(clk, 1, ns, 0);   // euclid -> trigger
    graph.prepare(kSr, 1, kBlock);
    AudioBlock out(kSr, 1, kBlock);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, ns, 3);   // sh
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            lo = std::min(lo, out.at(0, i));
            hi = std::max(hi, out.at(0, i));
        }
    }
    EXPECT(hi - lo > 0.2f);   // o S&H realmente varia no patch
}

void testPanel() {
    Noise n;
    const std::string problem = validatePanel(n);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(n.panel().widgets.size() >= 10);
    std::cout << renderAscii(n);
}

}  // namespace

int main() {
    testWhiteFlat();
    testPinkTilt();
    testBrownTilt();
    testBlueTilt();
    testVioletBrighterThanBlue();
    testBitHardBipolar();
    testSampleHold();
    testSHFromInput();
    testSmoothSlew();
    testSpread();
    testInternalClock();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_noise: OK\n";
    return g_failures == 0 ? 0 : 1;
}
