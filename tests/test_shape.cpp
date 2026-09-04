// Teste isolado do Módulo 24 (SHAPE — modelador de timbre).
// Critérios do dossiê `dossies/24_shape.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Shape.hpp"
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
constexpr std::size_t kB = 128;

// roda `blocks` blocos com in = seno de `fin` (amp `amp`) e, opcional,
// mod = seno de `fmod`. Devolve a saída amostra a amostra (após um
// "warm-up" de 200 ms pra o drift/estado assentar).
std::vector<float> run(Shape& sh, int blocks, float fin, float amp = 1.0f,
                       float fmod = 0.0f) {
    sh.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock i(kSr, 1, kB), m(kSr, 1, kB);
    std::vector<float> o;
    double pin = 0.0, pm = 0.0;
    const int warm = static_cast<int>(0.2 * kSr / kB);
    for (int b = 0; b < blocks + warm; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            i.at(0, k) = amp * std::sin(static_cast<float>(pin));
            m.at(0, k) = std::sin(static_cast<float>(pm));
            pin += 6.283185307 * fin / kSr;
            pm += 6.283185307 * fmod / kSr;
        }
        std::vector<const AudioBlock*> ins{&i, fmod > 0.0f ? &m : nullptr, nullptr};
        sh.process(ins, out);
        if (b >= warm)
            for (std::size_t k = 0; k < kB; ++k) o.push_back(out[0].at(0, k));
    }
    return o;
}

double rms(const std::vector<float>& v) {
    double s = 0; for (float x : v) s += double(x) * x; return std::sqrt(s / v.size());
}
double peak(const std::vector<float>& v) {
    double p = 0; for (float x : v) p = std::max(p, std::fabs((double)x)); return p;
}
// magnitude do harmônico `n` de `f` (Goertzel simplificado)
double harm(const std::vector<float>& v, double f, int n) {
    double re = 0, im = 0;
    const double w = 6.283185307 * f * n / kSr;
    for (std::size_t k = 0; k < v.size(); ++k) {
        re += v[k] * std::cos(w * k);
        im += v[k] * std::sin(w * k);
    }
    return std::sqrt(re * re + im * im) / v.size();
}
// energia de alta frequência: rms da diferença sucessiva
double hf(const std::vector<float>& v) {
    double s = 0;
    for (std::size_t k = 1; k < v.size(); ++k)
        s += (v[k] - v[k - 1]) * (v[k] - v[k - 1]);
    return std::sqrt(s / v.size());
}

void testRingMod() {
    Shape a, b;
    a.setParameter("ring", 1.0f);
    a.setParameter("sat", 0.0f);
    a.setParameter("level", 1.0f);
    const auto ro = run(a, 200, 300.0f, 0.8f, 220.0f);
    // ring=1 -> saída ~ in·mod: quase nada em 300 Hz, energia em 300±220
    const double f0 = harm(ro, 300.0, 1);
    const double diff = harm(ro, 80.0, 1);   // 300-220
    const double sum = harm(ro, 520.0, 1);   // 300+220
    check(f0 < 0.05, "ring: fundamental suprimida");
    check(diff > 0.1 && sum > 0.1, "ring: bandas soma e diferença presentes");

    b.setParameter("ring", 0.0f);
    b.setParameter("sat", 0.0f);
    b.setParameter("level", 1.0f);
    const auto bo = run(b, 200, 300.0f, 0.8f, 220.0f);
    check(harm(bo, 300.0, 1) > 0.3, "ring=0: passa a fundamental");
}

void testFoldAddsHarmonics() {
    Shape clean, folded;
    clean.setParameter("fold", 0.0f);
    folded.setParameter("fold", 0.9f);
    const auto c = run(clean, 200, 200.0f);
    const auto f = run(folded, 200, 200.0f);
    check(hf(f) > hf(c) * 2.0, "fold alto -> mais energia de alta freq (dobras)");
    check(peak(f) <= 1.05, "fold: saída limitada a ~±1");
}

void testSymmetryEvenHarmonic() {
    Shape sym0, sym;
    sym0.setParameter("fold", 0.6f);
    sym.setParameter("fold", 0.6f);
    sym.setParameter("symmetry", 0.6f);
    const auto a = run(sym0, 200, 220.0f);
    const auto b = run(sym, 200, 220.0f);
    // 2º harmônico: ~0 sem simetria, aparece com simetria
    check(harm(b, 220.0, 2) > harm(a, 220.0, 2) * 3.0 + 0.02,
          "symmetry -> 2º harmônico (energia par)");
}

void testWrapDiscontinuity() {
    Shape soft, wrapd;
    soft.setParameter("fold", 0.5f);
    soft.setParameter("wrap", 0.0f);
    wrapd.setParameter("fold", 0.5f);
    wrapd.setParameter("wrap", 1.0f);
    const auto s = run(soft, 200, 180.0f);
    const auto w = run(wrapd, 200, 180.0f);
    auto bigJumps = [](const std::vector<float>& v) {
        int n = 0;
        for (std::size_t k = 1; k < v.size(); ++k)
            if (std::fabs(v[k] - v[k - 1]) > 0.5f) ++n;
        return n;
    };
    check(bigJumps(w) > bigJumps(s) + 20, "wrap=1 -> descontinuidades");
}

void testSatFlattens() {
    Shape a, b;
    a.setParameter("sat", 0.0f);
    b.setParameter("sat", 1.0f);
    const auto ra = run(a, 200, 200.0f, 0.9f);
    const auto rb = run(b, 200, 200.0f, 0.9f);
    const double crestA = peak(ra) / std::max(1e-9, rms(ra));
    const double crestB = peak(rb) / std::max(1e-9, rms(rb));
    check(crestB < crestA - 0.05, "sat -> fator de crista menor (achatamento)");
}

void testLevel() {
    Shape half, full;
    half.setParameter("level", 0.4f);
    full.setParameter("level", 0.8f);
    const auto h = run(half, 150, 200.0f, 0.7f);
    const auto f = run(full, 150, 200.0f, 0.7f);
    check(std::fabs(rms(f) / rms(h) - 2.0) < 0.15, "level escala linear");
}

void testBypassNearTransparent() {
    Shape s;
    s.setParameter("fold", 0.0f);
    s.setParameter("wrap", 0.0f);
    s.setParameter("sat", 0.0f);
    s.setParameter("symmetry", 0.0f);
    s.setParameter("level", 1.0f);
    const auto o = run(s, 150, 200.0f, 0.4f);   // amplitude baixa -> tanh ~ id
    check(std::fabs(peak(o) - 0.4) < 0.06, "sem modelagem + amp baixa -> ~transparente");
}

void testDrift() {
    Shape d, s;
    d.setParameter("fold", 0.5f);
    d.setParameter("drift", 1.0f);
    s.setParameter("fold", 0.5f);
    s.setParameter("drift", 0.0f);
    const auto rd = run(d, 300, 200.0f);
    const auto rs = run(s, 300, 200.0f);
    check(std::fabs(rms(rd) - rms(rs)) < 0.06 * rms(rs) + 0.01,
          "drift altera pouco (< ~6%)");
    // determinismo com drift=0
    Shape a, b;
    for (Shape* p : {&a, &b}) { p->setParameter("fold", 0.7f); p->setParameter("sat", 0.3f); }
    const auto oa = run(a, 200, 210.0f), ob = run(b, 200, 210.0f);
    bool same = oa.size() == ob.size();
    for (std::size_t k = 0; same && k < oa.size(); ++k) if (oa[k] != ob[k]) same = false;
    check(same, "drift=0 -> dois renders byte-idênticos");
}

void testFiniteExtremes() {
    Shape s;
    s.setParameter("ring", 1.0f); s.setParameter("fold", 1.0f);
    s.setParameter("symmetry", 1.0f); s.setParameter("wrap", 1.0f);
    s.setParameter("sat", 1.0f); s.setParameter("level", 1.0f);
    const auto o = run(s, 200, 400.0f, 1.0f, 130.0f);
    for (float x : o) EXPECT(std::isfinite(x));
    check(peak(o) <= 1.06, "tudo no talo -> saída ainda contida");
}

void testAliasReduced() {
    // sinal periódico por não-linearidade sem memória -> só deve haver
    // energia em k*f0. Energia em bins NÃO-harmônicos = alias. Com ADAA +
    // 2× o piso de alias fica muito abaixo da fundamental (regressão:
    // pegaria a volta ao alias cru).
    auto floorAt = [](float f0, float fold, float wrap) {
        Shape s;
        s.setParameter("fold", fold);
        s.setParameter("wrap", wrap);
        s.setParameter("level", 1.0f);
        const auto o = run(s, 250, f0, 0.95f);
        const double fund = harm(o, f0, 1);
        double al = 0.0;
        int n = 0;
        for (double f = 1300.0; f < 20000.0; f += 1700.0) {
            const double kf = f / f0;
            if (std::fabs(kf - std::round(kf)) < 0.16) continue;  // harmônico
            al += harm(o, f, 1);
            ++n;
        }
        return (al / n) / (fund + 1e-9);
    };
    check(floorAt(3777.0f, 0.9f, 0.0f) < 0.01,
          "fold alto: alias < 1% da fundamental");
    check(floorAt(3777.0f, 0.5f, 1.0f) < 0.02,
          "wrap alto: alias contido");
}

void testInGraph() {
    SignalGraph g;
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 160.0f);
    const auto shape = g.add(std::make_unique<Shape>());
    g.node(shape).setParameter("fold", 0.6f);
    g.node(shape).setParameter("level", 0.9f);
    const auto flt = g.add(std::make_unique<Filter>());
    g.connect(osc, 0, shape, 0);       // sine -> in
    g.connect(shape, 0, flt, 0);       // out -> filter in
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    double p = 0.0;
    for (int b = 0; b < 800; ++b) {
        g.process(out, flt, 3);        // all
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            p = std::max(p, std::fabs((double)out.at(0, k)));
        }
    }
    check(p > 0.05, "SHAPE passa e enriquece o sinal no grafo");
}

void testPanel() {
    Shape s;
    const std::string problem = validatePanel(s);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(s.panel().widgets.size() >= 12);
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testRingMod();
    testFoldAddsHarmonics();
    testSymmetryEvenHarmonic();
    testWrapDiscontinuity();
    testSatFlattens();
    testLevel();
    testBypassNearTransparent();
    testDrift();
    testFiniteExtremes();
    testAliasReduced();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_shape: OK\n";
    return g_failures == 0 ? 0 : 1;
}
