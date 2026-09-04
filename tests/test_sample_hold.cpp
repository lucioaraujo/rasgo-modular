// Teste isolado do Módulo 23 (SH — sample & hold duplo).
// Critérios do dossiê `dossies/23_sample_hold.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/SampleHold.hpp"
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
void near(double a, double b, double t = 0.01) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

// trem de pulsos: alto por `hi` a cada `period` amostras
struct Pulse { std::size_t period, hi, t = 0;
    float next() { const float v = (t % period) < hi ? 1.0f : 0.0f; ++t; return v; } };

struct Rec {
    int steps1 = 0;
    std::vector<float> o1, o2;
};

Rec run(SampleHold& sh, int blocks, Pulse* t1 = nullptr, Pulse* t2 = nullptr,
        const float in1 = 0.0f, const bool hasIn1 = false,
        const float in2 = 0.0f, const bool hasIn2 = false) {
    sh.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB), c1(kSr, 1, kB), i2(kSr, 1, kB), c2(kSr, 1, kB);
    Rec r;
    float prev1 = 0.0f;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            i1.at(0, k) = in1; i2.at(0, k) = in2;
            c1.at(0, k) = t1 ? t1->next() : 0.0f;
            c2.at(0, k) = t2 ? t2->next() : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            hasIn1 ? &i1 : nullptr, t1 ? &c1 : nullptr,
            hasIn2 ? &i2 : nullptr, t2 ? &c2 : nullptr};
        sh.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v = out[0].at(0, k);
            check(std::isfinite(v) && std::isfinite(out[1].at(0, k)), "finito");
            if (std::fabs(v - prev1) > 1e-4f) ++r.steps1;
            prev1 = v;
            r.o1.push_back(v); r.o2.push_back(out[1].at(0, k));
        }
    }
    return r;
}

double corr(const std::vector<float>& a, const std::vector<float>& b) {
    double ma = 0, mb = 0;
    for (std::size_t i = 0; i < a.size(); ++i) { ma += a[i]; mb += b[i]; }
    ma /= a.size(); mb /= b.size();
    double num = 0, da = 0, db = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        num += (a[i] - ma) * (b[i] - mb);
        da += (a[i] - ma) * (a[i] - ma);
        db += (b[i] - mb) * (b[i] - mb);
    }
    return (da < 1e-9 || db < 1e-9) ? 0.0 : num / std::sqrt(da * db);
}

void testStepsMatchPulses() {
    SampleHold sh;
    Pulse t{3000, 100};   // ~16 Hz -> ~10 pulsos em 200 blocos * 128 / 48000 = 0.53s
    const Rec r = run(sh, 200, &t);
    // nº de degraus <= nº de pulsos (o 1º valor pode já ser 0)
    check(r.steps1 >= 3 && r.steps1 <= 10, "degraus ~= pulsos");
}

void testHoldsBetweenPulses() {
    SampleHold sh;
    Pulse t{6000, 80};    // pulsos raros
    const Rec r = run(sh, 200, &t);
    // dentro de janelas longas o valor não muda (slew=0)
    int constRuns = 0, cur = 0;
    for (std::size_t i = 1; i < r.o1.size(); ++i) {
        if (std::fabs(r.o1[i] - r.o1[i - 1]) < 1e-6f) { if (++cur == 2000) ++constRuns; }
        else cur = 0;
    }
    check(constRuns >= 2, "segura o valor (platôs longos entre pulsos)");
}

void testSamplesExternalIn() {
    SampleHold sh;
    Pulse t{2000, 100};
    const Rec r = run(sh, 60, &t, nullptr, 0.42f, true);
    // depois de alguns pulsos, out1 == 0.42
    near(r.o1.back(), 0.42);
}

void testSlew() {
    // fonte interna (acaso) -> cada pulso segura um valor DIFERENTE, então
    // o degrau (slew=0) vs o glide (slew alto) aparece de verdade
    SampleHold fast, slow;
    fast.setParameter("slew1", 0.0f);
    slow.setParameter("slew1", 0.6f);
    Pulse tf{2000, 80}, ts{2000, 80};
    const Rec rf = run(fast, 300, &tf);
    const Rec rs = run(slow, 300, &ts);
    float maxDf = 0, maxDs = 0;
    for (std::size_t i = 1; i < rf.o1.size(); ++i)
        maxDf = std::max(maxDf, std::fabs(rf.o1[i] - rf.o1[i - 1]));
    for (std::size_t i = 1; i < rs.o1.size(); ++i)
        maxDs = std::max(maxDs, std::fabs(rs.o1[i] - rs.o1[i - 1]));
    check(maxDf > 0.5f, "slew=0 -> degrau (salto grande)");
    check(maxDs < 0.01f, "slew alto -> glide (derivada por amostra pequena)");
}

void testAsymmetricSlope() {
    // `track` alto + `in` onda quadrada ±0.6: `slope`>0 -> sobe devagar,
    // desce rápido. Mede o |Δy| máximo por amostra em cada metade.
    SampleHold sh;
    sh.setParameter("track1", 1.0f);
    sh.setParameter("slew1", 0.5f);
    sh.setParameter("slope", 0.8f);
    sh.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB), c1(kSr, 1, kB), i2(kSr, 1, kB), c2(kSr, 1, kB);
    const std::size_t half = 4000;
    float prev = 0.0f, maxRise = 0.0f, maxFall = 0.0f;
    std::size_t g = 0;
    for (int b = 0; b < 200; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            const bool up = ((g + k) / half) % 2 == 0;
            i1.at(0, k) = up ? 0.6f : -0.6f;
            c1.at(0, k) = 1.0f;
        }
        std::vector<const AudioBlock*> ins{&i1, &c1, nullptr, nullptr};
        sh.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v = out[0].at(0, k);
            const float d = v - prev;
            if (d > 0.0f) maxRise = std::max(maxRise, d);
            else maxFall = std::max(maxFall, -d);
            prev = v;
        }
        g += kB;
    }
    check(maxFall > maxRise * 1.8f,
          "slope>0 -> descida bem mais rápida que a subida");
}

void testTrackAndHold() {
    SampleHold sh;
    sh.setParameter("track1", 1.0f);
    sh.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB), c1(kSr, 1, kB);
    // gate alto: out segue in (0.7); depois gate baixo: out congela em 0.7
    for (std::size_t k = 0; k < kB; ++k) { i1.at(0, k) = 0.7f; c1.at(0, k) = 1.0f; }
    std::vector<const AudioBlock*> ins{&i1, &c1, nullptr, nullptr};
    for (int b = 0; b < 10; ++b) sh.process(ins, out);
    near(out[0].at(0, kB - 1), 0.7);
    // agora in muda pra 0.2 mas gate cai -> deve congelar em 0.7
    for (std::size_t k = 0; k < kB; ++k) { i1.at(0, k) = 0.2f; c1.at(0, k) = 0.0f; }
    for (int b = 0; b < 10; ++b) sh.process(ins, out);
    near(out[0].at(0, kB - 1), 0.7);
}

void testSpreadNarrows() {
    SampleHold flat, bell;
    bell.setParameter("spread", 1.0f);
    Pulse tf{800, 60}, tb{800, 60};   // pulsos densos -> muitas amostras
    const Rec rf = run(flat, 400, &tf);
    const Rec rb = run(bell, 400, &tb);
    auto var = [](const std::vector<float>& v) {
        double m = 0; for (float x : v) m += x; m /= v.size();
        double s = 0; for (float x : v) s += (x - m) * (x - m); return s / v.size();
    };
    check(var(rb.o1) < var(rf.o1) * 0.8, "spread=1 concentra o acaso (variância menor)");
}

void testCorrelation() {
    Pulse ti[6];
    for (auto& p : ti) p = {700, 50};
    SampleHold same, mirror, indep;
    same.setParameter("correlation", 1.0f);
    mirror.setParameter("correlation", -1.0f);
    indep.setParameter("correlation", 0.0f);
    const Rec rs = run(same, 400, &ti[0], &ti[1]);
    const Rec rm = run(mirror, 400, &ti[2], &ti[3]);
    const Rec ri = run(indep, 400, &ti[4], &ti[5]);
    check(corr(rs.o1, rs.o2) > 0.95, "correlation=1 -> canais idênticos");
    check(corr(rm.o1, rm.o2) < -0.95, "correlation=-1 -> canais espelhados");
    check(std::fabs(corr(ri.o1, ri.o2)) < 0.3, "correlation=0 -> descorrelacionados");
}

void testInternalClock() {
    SampleHold sh;
    sh.setParameter("rate", 20.0f);
    const Rec r = run(sh, 300, nullptr);   // sem trig -> relógio interno
    // 300*128/48000 = 0.8s * 20 Hz = ~16 degraus
    check(r.steps1 >= 10 && r.steps1 <= 22, "relógio interno gera degraus sem trig");
}

void testDeterminism() {
    SampleHold a, b;
    for (SampleHold* s : {&a, &b}) {
        s->setParameter("rate", 7.0f);
        s->setParameter("spread", 0.6f);
        s->setParameter("correlation", 0.5f);
        s->setParameter("slew1", 0.3f);
    }
    const Rec ra = run(a, 200, nullptr);
    const Rec rb = run(b, 200, nullptr);
    bool same = ra.o1.size() == rb.o1.size();
    for (std::size_t i = 0; same && i < ra.o1.size(); ++i)
        if (ra.o1[i] != rb.o1[i] || ra.o2[i] != rb.o2[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 130.0f);
    const auto sh = g.add(std::make_unique<SampleHold>());
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.connect(clk, 1, sh, 1);       // euclid -> trig1
    g.connect(sh, 0, osc, 0);       // out1 -> pitch
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 3000; ++b) {
        g.process(out, osc, 2);    // saw
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.1f && lo < -0.1f, "SH.out1 varia a altura do OSC no grafo");
}

void testPanel() {
    SampleHold sh;
    const std::string problem = validatePanel(sh);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(sh.panel().widgets.size() >= 13);
    std::cout << renderAscii(sh);
}

}  // namespace

int main() {
    testStepsMatchPulses();
    testHoldsBetweenPulses();
    testSamplesExternalIn();
    testSlew();
    testAsymmetricSlope();
    testTrackAndHold();
    testSpreadNarrows();
    testCorrelation();
    testInternalClock();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_sample_hold: OK\n";
    return g_failures == 0 ? 0 : 1;
}
