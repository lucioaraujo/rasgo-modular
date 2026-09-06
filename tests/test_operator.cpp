// Teste isolado do Módulo 44 (OPERATOR — voz FM multi-operador).
// Critérios do dossiê `dossies/44_operator.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Operator.hpp"
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
constexpr std::size_t kB = 256;

struct In { float pitch = 0.0f; bool hasPitch = false;
            float idx = 0.0f;   bool hasIdx = false; };

std::vector<float> render(Operator& o, int blocks, const In& in = {}) {
    o.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB), bi(kSr, 1, kB);
    std::vector<float> r;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bp.at(0, k) = in.pitch;
            bi.at(0, k) = in.idx;
        }
        std::vector<const AudioBlock*> ins{
            in.hasPitch ? &bp : nullptr, in.hasIdx ? &bi : nullptr};
        o.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
    }
    return r;
}

double magAt(const std::vector<float>& x, double hz) {
    const std::size_t start = x.size() / 2;
    const std::size_t n = x.size() - start;
    double re = 0.0, im = 0.0, wsum = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    for (std::size_t i = start; i < x.size(); ++i) {
        const double t = (double)(i - start);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * t / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        wsum += w;
    }
    return std::sqrt(re * re + im * im) / wsum;
}

double centroid(const std::vector<float>& x, double f0) {
    double num = 0.0, den = 0.0;
    for (int h = 1; h <= 24; ++h) {
        const double m = magAt(x, f0 * h);
        num += m * h; den += m;
    }
    return den > 1e-12 ? num / den : 0.0;
}

double rms(const std::vector<float>& x) {
    double s = 0.0;
    const std::size_t a = x.size() / 2;
    for (std::size_t i = a; i < x.size(); ++i) s += x[i] * (double)x[i];
    return std::sqrt(s / (double)(x.size() - a));
}

int zc(const std::vector<float>& x) {
    int z = 0;
    for (std::size_t i = 1 + x.size() / 2; i < x.size(); ++i)
        if ((x[i - 1] < 0.0f) != (x[i] < 0.0f)) ++z;
    return z;
}

constexpr double kF0 = 110.0;

void testAdditiveStack() {
    // algo 7 (aditivo), sem FM, razões 1/2/3 -> pilha harmônica tipo órgão
    Operator o;
    o.setParameter("freq", 110.0f);
    o.setParameter("algo", 7.0f);
    o.setParameter("index", 0.0f);
    o.setParameter("ratio_b", 3.0f);   // idx 3 -> razão 2
    o.setParameter("ratio_c", 5.0f);   // idx 5 -> razão 3
    o.setParameter("ratio_d", 1.0f);   // idx 1 -> razão 1
    const auto r = render(o, 40);
    // portadoras em f0 (op A e op D), 2·f0 (op B), 3·f0 (op C)
    EXPECT(magAt(r, kF0) > 0.1);
    EXPECT(magAt(r, kF0 * 2) > 0.05);
    EXPECT(magAt(r, kF0 * 3) > 0.05);
    EXPECT(magAt(r, kF0 * 2.5) < 0.02);   // nada entre as parciais (sem FM)
}

void testChainMakesHarmonics() {
    auto run = [](float index) {
        Operator o;
        o.setParameter("freq", 110.0f);
        o.setParameter("algo", 0.0f);   // A→B→C→D, portadora D
        o.setParameter("ratio_b", 1.0f);
        o.setParameter("ratio_c", 1.0f);
        o.setParameter("ratio_d", 1.0f);
        o.setParameter("index", index);
        return render(o, 40);
    };
    const auto clean = run(0.0f);
    const auto rich = run(0.8f);
    const double c1 = magAt(clean, kF0);
    EXPECT(c1 > 0.3);
    EXPECT(magAt(clean, kF0 * 3) < 0.05 * c1);     // sem FM: portadora D = senóide pura
    const double cenClean = centroid(clean, kF0);
    const double cenRich = centroid(rich, kF0);
    std::cerr << "  [chain] centroid clean=" << cenClean
              << " rich=" << cenRich << '\n';
    EXPECT(cenClean < 1.4);                        // energia toda na fundamental
    EXPECT(cenRich > 2.5);                         // FM espalha pros agudos
    EXPECT(cenRich > 2.0 * cenClean);
}

void testIndexBrightens() {
    auto cen = [](float index) {
        Operator o;
        o.setParameter("freq", 110.0f);
        o.setParameter("algo", 0.0f);
        o.setParameter("ratio_b", 1.0f);
        o.setParameter("ratio_c", 1.0f);
        o.setParameter("ratio_d", 1.0f);
        o.setParameter("index", index);
        return centroid(render(o, 40), kF0);
    };
    EXPECT(cen(0.2f) < cen(0.5f));
    EXPECT(cen(0.5f) < cen(0.85f));
}

void testRatioInharmonic() {
    Operator harm;
    harm.setParameter("freq", 110.0f); harm.setParameter("algo", 0.0f);
    harm.setParameter("ratio_b", 1.0f); harm.setParameter("index", 0.6f);
    Operator inh;
    inh.setParameter("freq", 110.0f); inh.setParameter("algo", 0.0f);
    inh.setParameter("ratio_b", 4.0f);   // idx 4 -> razão 2,5
    inh.setParameter("index", 0.6f);
    const auto rh = render(harm, 40);
    const auto ri = render(inh, 40);
    // razão 2,5: bandas laterais em f0 ± k·2,5·f0 -> energia em 2,5·f0
    EXPECT(magAt(ri, 2.5 * kF0) > 0.01);
    EXPECT(magAt(ri, 2.5 * kF0) > 4.0 * magAt(rh, 2.5 * kF0));
}

void testFeedbackSaws() {
    // algo 7, todas as razões 1, sem FM: a saída expõe o operador A cru.
    // feedback leva a senóide de A a um dente-de-serra -> harmônicos.
    auto run = [](float feedback) {
        Operator o;
        o.setParameter("freq", 120.0f);
        o.setParameter("algo", 7.0f);
        o.setParameter("ratio_b", 1.0f);
        o.setParameter("ratio_c", 1.0f);
        o.setParameter("ratio_d", 1.0f);
        o.setParameter("index", 0.0f);
        o.setParameter("feedback", feedback);
        return render(o, 40);
    };
    const auto no = run(0.0f);
    const auto yes = run(0.6f);
    const double cn = centroid(no, 120.0), cy = centroid(yes, 120.0);
    std::cerr << "  [fbk] centroid no=" << cn << " yes=" << cy << '\n';
    EXPECT(cn < 1.3);           // sem feedback: 4 senóides idênticas
    EXPECT(cy > 1.5 * cn);      // feedback -> A vira serra -> mais harmônicos
    for (float v : yes) EXPECT(std::isfinite(v) && std::fabs(v) < 1.5f);
}

void testPitchTracking() {
    Operator o;
    o.setParameter("freq", 110.0f);
    o.setParameter("algo", 7.0f);
    o.setParameter("index", 0.0f);
    o.setParameter("ratio_b", 1.0f);
    o.setParameter("ratio_c", 1.0f);
    o.setParameter("ratio_d", 1.0f);
    const auto lo = render(o, 40);
    Operator p;
    p.setParameter("freq", 110.0f);
    p.setParameter("algo", 7.0f);
    p.setParameter("index", 0.0f);
    p.setParameter("ratio_b", 1.0f);
    p.setParameter("ratio_c", 1.0f);
    p.setParameter("ratio_d", 1.0f);
    const auto hi = render(p, 40, In{1.0f, true, 0.0f, false});
    const double ratio = (double)zc(hi) / std::max(1, zc(lo));
    EXPECT(ratio > 1.8 && ratio < 2.2);
}

void testAllAlgosBounded() {
    for (int a = 0; a < 8; ++a) {
        Operator o;
        o.setParameter("freq", 160.0f);
        o.setParameter("algo", (float)a);
        o.setParameter("index", 0.7f);
        o.setParameter("feedback", 0.4f);
        o.setParameter("ratio_b", 3.0f);
        o.setParameter("ratio_c", 5.0f);
        o.setParameter("ratio_d", 2.0f);
        const auto r = render(o, 30);
        bool ok = true;
        for (float v : r) if (!std::isfinite(v) || std::fabs(v) >= 1.5f) ok = false;
        check(ok, "algo dá saída finita e < 1,5");
        EXPECT(rms(r) > 0.01);   // todo algoritmo soa
    }
}

void testDeterminism() {
    Operator a, b;
    for (Operator* x : {&a, &b}) {
        x->setParameter("freq", 130.0f);
        x->setParameter("algo", 3.0f);
        x->setParameter("index", 0.55f);
        x->setParameter("feedback", 0.3f);
        x->setParameter("drift", 0.5f);
        x->setParameter("ratio_c", 4.0f);
    }
    const auto ra = render(a, 30);
    const auto rb = render(b, 30);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testBoundedExtreme() {
    Operator o;
    o.setParameter("freq", 3000.0f);
    o.setParameter("algo", 0.0f);
    o.setParameter("index", 1.0f);
    o.setParameter("feedback", 1.0f);
    o.setParameter("drift", 1.0f);
    o.setParameter("ratio_b", 9.0f);
    o.setParameter("ratio_c", 9.0f);
    o.setParameter("ratio_d", 9.0f);
    const auto r = render(o, 50);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.5f);
}

void testPanel() {
    Operator o;
    check(validatePanel(o).empty(), "painel OPERATOR fecha");
    std::cout << renderAscii(o);
}

}  // namespace

int main() {
    testAdditiveStack();
    testChainMakesHarmonics();
    testIndexBrightens();
    testRatioInharmonic();
    testFeedbackSaws();
    testPitchTracking();
    testAllAlgosBounded();
    testDeterminism();
    testBoundedExtreme();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular OPERATOR tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
