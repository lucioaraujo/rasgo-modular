// Teste isolado do Módulo 27 (DRIFT — campo de deriva).
// Critérios do dossiê `dossies/27_drift.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Drift.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Oscillator.hpp"
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
constexpr std::size_t kB = 256;

struct Rec { std::vector<float> a, b, c, d, field; int events = 0; };

Rec run(Drift& dr, double seconds, const std::vector<float>* adv = nullptr) {
    dr.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock av(kSr, 1, kB);
    Rec r;
    const int blocks = static_cast<int>(seconds * kSr / kB);
    float prevEv = 0.0f;
    std::size_t ai = 0;
    for (int bl = 0; bl < blocks; ++bl) {
        for (std::size_t k = 0; k < kB; ++k)
            av.at(0, k) = adv ? (*adv)[ai++ % adv->size()] : 0.0f;
        std::vector<const AudioBlock*> ins{adv ? &av : nullptr, nullptr};
        dr.process(ins, out);
        for (std::size_t k = 0; k < kB; k += 16) {
            r.a.push_back(out[0].at(0, k));
            r.b.push_back(out[1].at(0, k));
            r.c.push_back(out[2].at(0, k));
            r.d.push_back(out[3].at(0, k));
            r.field.push_back(out[4].at(0, k));
        }
        for (std::size_t k = 0; k < kB; ++k) {
            const float ev = out[5].at(0, k);
            if (ev > 0.5f && prevEv <= 0.5f) ++r.events;
            prevEv = ev;
        }
    }
    return r;
}

double corr(const std::vector<float>& x, const std::vector<float>& y) {
    double mx = 0, my = 0;
    for (std::size_t i = 0; i < x.size(); ++i) { mx += x[i]; my += y[i]; }
    mx /= x.size(); my /= y.size();
    double n = 0, dx = 0, dy = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        n += (x[i] - mx) * (y[i] - my);
        dx += (x[i] - mx) * (x[i] - mx);
        dy += (y[i] - my) * (y[i] - my);
    }
    return (dx < 1e-9 || dy < 1e-9) ? 0.0 : n / std::sqrt(dx * dy);
}
double maxAbs(const std::vector<float>& v) {
    double m = 0; for (float x : v) m = std::max(m, (double)std::fabs(x)); return m;
}

void testSlowMovement() {
    Drift dr;
    dr.setParameter("rate", 0.15f);
    dr.setParameter("depth", 0.8f);
    const Rec r = run(dr, 40.0);
    // amostras a cada 16 samples -> ~0.33 ms. |Δ| entre amostras minúsculo.
    double maxStep = 0;
    for (std::size_t i = 1; i < r.a.size(); ++i)
        maxStep = std::max(maxStep, (double)std::fabs(r.a[i] - r.a[i - 1]));
    check(maxStep < 0.003, "as saídas se movem devagar (deriva, não modulação)");
    // mas ao longo de 40 s, percorre um caminho
    double lo = 9, hi = -9;
    for (float x : r.a) { lo = std::min(lo, (double)x); hi = std::max(hi, (double)x); }
    check(hi - lo > 0.15, "a deriva percorre um caminho ao longo do tempo");
}

void testDepthZero() {
    Drift dr;
    dr.setParameter("depth", 0.0f);
    dr.setParameter("bias", 0.3f);
    const Rec r = run(dr, 5.0);
    for (float x : r.a) near(x, 0.3, 0.01);
    for (float x : r.field) near(x, 0.3, 0.01);
}

void testRateAffectsEvents() {
    Drift slow, fast;
    slow.setParameter("rate", 0.1f);
    fast.setParameter("rate", 0.8f);
    const Rec rs = run(slow, 20.0);
    const Rec rf = run(fast, 20.0);
    check(rf.events > rs.events * 3, "rate maior -> mais tiques (`event`)");
    check(rs.events >= 1, "rate baixo ainda tica");
}

void testMomentum() {
    // momentum alto -> a velocidade persiste -> a série tem autocorrelação
    // lag-longo maior (tendências) que momentum baixo.
    Drift hi, lo;
    hi.setParameter("rate", 3.5f); hi.setParameter("momentum", 0.97f);
    hi.setParameter("depth", 0.85f); hi.setParameter("stride", 0.0f);
    lo.setParameter("rate", 3.5f); lo.setParameter("momentum", 0.0f);
    lo.setParameter("depth", 0.85f); lo.setParameter("stride", 0.0f);
    const Rec rh = run(hi, 45.0);
    const Rec rl = run(lo, 45.0);
    // momentum alto -> menos reversões de direção (tendências longas)
    auto reversals = [](const std::vector<float>& v) {
        int n = 0;
        for (std::size_t i = 2; i < v.size(); ++i) {
            const float d1 = v[i - 1] - v[i - 2], d2 = v[i] - v[i - 1];
            if (d1 * d2 < -1e-10f) ++n;
        }
        return n;
    };
    check(reversals(rl.field) > reversals(rh.field) * 2,
          "momentum alto -> tendências persistem (menos reversões)");
}

void testStrideCorrelation() {
    Drift together, apart;
    together.setParameter("rate", 2.0f); together.setParameter("stride", 0.0f);
    together.setParameter("depth", 0.8f); together.setParameter("momentum", 0.5f);
    apart.setParameter("rate", 2.0f); apart.setParameter("stride", 1.0f);
    apart.setParameter("depth", 0.8f); apart.setParameter("momentum", 0.5f);
    const Rec rt = run(together, 40.0);
    const Rec ra = run(apart, 40.0);
    check(corr(rt.a, rt.d) > 0.9, "stride=0 -> saídas andam juntas");
    check(corr(ra.a, ra.d) < 0.6, "stride=1 -> saídas descorrelacionam");
}

void testAdvanceCadence() {
    // `advance` conectada: o campo só muda na borda ↑
    Drift dr;
    dr.setParameter("rate", 0.9f);   // ignorado quando advance conectada
    dr.setParameter("depth", 0.8f);
    // 6 pulsos ao longo de 12 s
    std::vector<float> adv(static_cast<std::size_t>(12.0 * kSr), 0.0f);
    for (int p = 0; p < 6; ++p) {
        const std::size_t at = (adv.size() / 6) * static_cast<std::size_t>(p);
        for (std::size_t k = at; k < at + 200 && k < adv.size(); ++k) adv[k] = 1.0f;
    }
    const Rec r = run(dr, 12.0, &adv);
    check(r.events >= 5 && r.events <= 7, "anda ~1x por borda de advance");
    // entre pulsos o campo tem platôs (a saída desliza pro alvo e para)
    double maxStep = 0;
    for (std::size_t i = 1; i < r.field.size(); ++i)
        maxStep = std::max(maxStep, (double)std::fabs(r.field[i] - r.field[i - 1]));
    check(maxStep < 0.01, "cadência por advance: transições suaves");
}

void testBias() {
    Drift dr;
    dr.setParameter("bias", -0.5f);
    dr.setParameter("depth", 0.3f);
    dr.setParameter("momentum", 0.2f);
    const Rec r = run(dr, 25.0);
    double m = 0; for (float x : r.a) m += x; m /= r.a.size();
    check(m < -0.15, "bias desloca o centro da deriva");
}

void testFiniteBounded() {
    Drift dr;
    dr.setParameter("rate", 1.0f);
    dr.setParameter("depth", 1.0f);
    dr.setParameter("momentum", 1.0f);
    const Rec r = run(dr, 40.0);
    for (const auto* v : {&r.a, &r.b, &r.c, &r.d, &r.field}) {
        for (float x : *v) EXPECT(std::isfinite(x));
        check(maxAbs(*v) <= 1.02, "saída em [-1,1]");
    }
}

void testDeterminism() {
    Drift x, y;
    for (Drift* d : {&x, &y}) {
        d->setParameter("rate", 0.35f);
        d->setParameter("depth", 0.7f);
        d->setParameter("momentum", 0.6f);
        d->setParameter("stride", 0.5f);
    }
    const Rec rx = run(x, 15.0);
    const Rec ry = run(y, 15.0);
    bool same = rx.a.size() == ry.a.size();
    for (std::size_t i = 0; same && i < rx.a.size(); ++i)
        if (rx.a[i] != ry.a[i] || rx.field[i] != ry.field[i]) same = false;
    EXPECT(same);
}

void testAnchor() {
    // `anchor` alto -> a deriva volta a marcos gravados: a trajetória
    // difere da de `anchor=0` e o campo VAGA MENOS (fica mais preso).
    // Determinístico e limitado.
    auto mk = [](float anchor) {
        Drift d;
        d.setParameter("rate", 3.0f);
        d.setParameter("depth", 0.9f);
        d.setParameter("momentum", 0.55f);
        d.setParameter("stride", 0.2f);
        d.setParameter("anchor", anchor);
        return d;
    };
    Drift a0 = mk(0.0f), aA = mk(0.9f), aB = mk(0.9f);
    const Rec r0 = run(a0, 30.0), rA = run(aA, 30.0), rB = run(aB, 30.0);
    // determinismo
    bool same = rA.field.size() == rB.field.size();
    for (std::size_t i = 0; same && i < rA.field.size(); ++i)
        if (rA.field[i] != rB.field[i]) same = false;
    EXPECT(same);
    // difere de anchor=0
    double diff = 0.0;
    for (std::size_t i = 0; i < r0.field.size() && i < rA.field.size(); ++i)
        diff = std::max(diff, std::fabs((double)r0.field[i] - rA.field[i]));
    check(diff > 0.02, "anchor muda a trajetória da deriva");
    // vaga menos: desvio-padrão do campo menor
    auto sd = [](const std::vector<float>& v) {
        double m = 0; for (float x : v) m += x; m /= v.size();
        double s = 0; for (float x : v) s += (x - m) * (x - m);
        return std::sqrt(s / v.size());
    };
    check(sd(rA.field) < sd(r0.field) + 1e-4, "anchor: o campo vaga <= sem anchor");
    for (float x : rA.field) EXPECT(std::isfinite(x) && std::fabs(x) <= 1.05f);
}

void testInGraph() {
    SignalGraph g;
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 110.0f);
    const auto flt = g.add(std::make_unique<Filter>());
    g.node(flt).setParameter("cutoff", 400.0f);
    const auto dr = g.add(std::make_unique<Drift>());
    g.node(dr).setParameter("rate", 0.3f);
    g.node(dr).setParameter("depth", 0.9f);
    g.connect(osc, 2, flt, 0);
    g.connectToParameter(dr, 0, flt, "cutoff", 3000.0f, 1200.0f);
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    std::vector<float> rms;
    for (int blk = 0; blk < static_cast<int>(20 * kSr / kB); ++blk) {
        g.process(out, flt, 3);
        double m = 0;
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            m += out.at(0, k) * out.at(0, k);
        }
        if (blk % 20 == 0) rms.push_back(std::sqrt(m / kB));
    }
    double lo = 9, hi = 0;
    for (float x : rms) { lo = std::min(lo, (double)x); hi = std::max(hi, (double)x); }
    check(hi > lo * 1.3 + 0.001, "DRIFT faz o corte (e o timbre) andar em minutos");
}

void testPanel() {
    Drift dr;
    const std::string problem = validatePanel(dr);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(dr.panel().widgets.size() >= 12);
    std::cout << renderAscii(dr);
}

}  // namespace

int main() {
    testSlowMovement();
    testDepthZero();
    testRateAffectsEvents();
    testMomentum();
    testStrideCorrelation();
    testAdvanceCadence();
    testBias();
    testFiniteBounded();
    testDeterminism();
    testAnchor();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_drift: OK\n";
    return g_failures == 0 ? 0 : 1;
}
