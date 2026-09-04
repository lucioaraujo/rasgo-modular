// Teste isolado do Módulo 21 (CONTROL) - antes de entrar num patch.
// Critérios do dossiê `dossies/21_control.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Control.hpp"
#include "dsp/FunctionGenerator.hpp"
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
    if (std::fabs(a - b) >= t)
        std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

// último valor de uma saída com in1 (e opcionalmente in2) DC constante.
float dc(Control& ctl, const int outIdx, const float in1,
         const float in2 = 0.0f, const bool hasIn2 = false, int blocks = 200) {
    ctl.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB), i2(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) { i1.at(0, k) = in1; i2.at(0, k) = in2; }
    std::vector<const AudioBlock*> ins{&i1, hasIn2 ? &i2 : nullptr};
    for (int b = 0; b < blocks; ++b) ctl.process(ins, out);
    return out[static_cast<std::size_t>(outIdx)].at(0, 0);
}

void testScale() {
    Control c;
    c.setParameter("scale1", 1.0f);
    near(dc(c, 0, 0.4f), 0.4);
    c.setParameter("scale1", 2.0f);
    near(dc(c, 0, 0.4f), 0.8);
    c.setParameter("scale1", 0.5f);
    near(dc(c, 0, 0.4f), 0.2);
    c.setParameter("scale1", -1.0f);        // inverte
    near(dc(c, 0, 0.4f), -0.4);
}

void testScaleZeroIsVoltageSource() {
    Control c;
    c.setParameter("scale1", 0.0f);
    c.setParameter("offset1", 0.3f);
    // sem sinal na entrada importa: scale 0 -> saída = offset
    near(dc(c, 0, 0.9f), 0.3);
    near(dc(c, 0, 0.0f), 0.3);
}

void testOffset() {
    Control c;
    c.setParameter("scale1", 1.0f);
    c.setParameter("offset1", -0.25f);
    near(dc(c, 0, 0.5f), 0.25);
}

void testRectifyFull() {
    Control c;
    c.setParameter("rectify1", 1.0f);
    near(dc(c, 0, -0.6f), 0.6);    // |x|
    near(dc(c, 0, 0.6f), 0.6);
}

void testRectifyHalf() {
    Control c;
    c.setParameter("rectify1", 0.5f);
    near(dc(c, 0, -0.6f), 0.0);    // meia-onda: negativo zerado
    near(dc(c, 0, 0.6f), 0.6);     // positivo intacto
}

void testSlewInstant() {
    Control c;
    c.setParameter("slew1", 0.0f);
    // slew 0 -> a saída acompanha o alvo em poucos blocos
    near(dc(c, 0, 0.7f, 0.0f, false, 4), 0.7);
}

void testSlewRamp() {
    Control c;
    c.setParameter("slew1", 0.5f);        // ~0,5 s
    c.setParameter("curve1", 0.0f);       // linear
    c.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 0.0f;
    std::vector<const AudioBlock*> ins{&i1, nullptr};
    for (int b = 0; b < 4; ++b) c.process(ins, out);   // assenta em 0
    for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 1.0f;  // degrau
    float prev = out[0].at(0, kB - 1);
    float maxStep = 0.0f, last = prev;
    bool monotonic = true;
    for (int b = 0; b < 60; ++b) {
        c.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v = out[0].at(0, k);
            maxStep = std::max(maxStep, std::fabs(v - prev));
            if (v < last - 1e-4f) monotonic = false;
            prev = v; last = v;
        }
    }
    check(maxStep < 0.001f, "slew alto: derivada por amostra pequena (rampa)");
    check(monotonic, "slew sobe monotônico, sem overshoot");
    check(out[0].at(0, kB - 1) > 0.1f && out[0].at(0, kB - 1) < 0.95f,
          "a ~160 ms está no meio da rampa (subiu mas não chegou)");
}

void testCurveLinearVsExp() {
    // linear: inclinação ~constante pra saltos de tamanhos diferentes.
    // exponencial: inclinação decai ao se aproximar do alvo.
    auto slopeAfterStep = [](float curve, float target) {
        Control c;
        c.setParameter("slew1", 0.5f);
        c.setParameter("curve1", curve);
        c.prepare(kSr, kB);
        std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
        AudioBlock i1(kSr, 1, kB);
        std::vector<const AudioBlock*> ins{&i1, nullptr};
        for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 0.0f;
        for (int b = 0; b < 4; ++b) c.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = target;
        c.process(ins, out);                 // primeiro bloco após o degrau
        return std::fabs(out[0].at(0, 1) - out[0].at(0, 0)); // inclin. inicial
    };
    const float linSmall = slopeAfterStep(0.0f, 0.4f);
    const float linBig = slopeAfterStep(0.0f, 1.0f);
    const float expSmall = slopeAfterStep(1.0f, 0.4f);
    const float expBig = slopeAfterStep(1.0f, 1.0f);
    // linear: inclinação igual pros dois saltos
    near(linSmall, linBig, std::max(1e-6, linBig * 0.15));
    // exponencial: salto maior -> inclinação inicial proporcionalmente maior
    check(expBig > expSmall * 2.0f, "exp: inclinação inicial escala com o salto");
}

void testEnvelopeFollower() {
    // retifica + slew num seno de áudio -> segue a amplitude
    Control c;
    c.setParameter("rectify1", 1.0f);
    c.setParameter("slew1", 0.25f);
    c.setParameter("curve1", 1.0f);       // RC
    c.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&i1, nullptr};
    double ph = 0.0;
    float envLo = 1e9f, envHi = -1e9f;
    for (int b = 0; b < 500; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            i1.at(0, k) = 0.8f * std::sin(static_cast<float>(ph));
            ph += 6.2831853 * 200.0 / kSr;
        }
        c.process(ins, out);
        if (b > 200)
            for (std::size_t k = 0; k < kB; ++k) {
                EXPECT(std::isfinite(out[0].at(0, k)));
                envLo = std::min(envLo, out[0].at(0, k));
                envHi = std::max(envHi, out[0].at(0, k));
            }
    }
    // envoltória de |0.8·sin| tem média 2/π·0.8 ≈ 0.51; com slew fica
    // perto disso, com ripple pequeno e sem parte negativa
    check(envLo > 0.2f && envHi < 0.75f, "segue a amplitude (~0,5) com ripple");
}

void testSum() {
    Control soma, media;
    soma.setParameter("sum_mode", 0.0f);
    near(soma.type() == "CONTROL" ? dc(soma, 2, 0.6f, -0.2f, true) : 0.0, 0.4);
    media.setParameter("sum_mode", 1.0f);
    near(dc(media, 2, 0.6f, -0.2f, true), 0.2);        // (0.6-0.2)/2
    // modo soma satura no teto
    Control clip;
    clip.setParameter("sum_mode", 0.0f);
    near(dc(clip, 2, 0.9f, 0.9f, true), 1.0);          // 1.8 -> clamp 1.0
}

void testDrift() {
    Control d, s;
    d.setParameter("offset1", 0.4f);
    d.setParameter("scale1", 0.0f);
    d.setParameter("drift", 1.0f);
    s.setParameter("offset1", 0.4f);
    s.setParameter("scale1", 0.0f);
    s.setParameter("drift", 0.0f);
    d.prepare(kSr, kB); s.prepare(kSr, kB);
    std::vector<AudioBlock> od(3, AudioBlock(kSr, 1, kB)), os(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&i1, nullptr};
    bool differ = false;
    float lo = 1.0f, hi = 0.0f;
    for (int b = 0; b < 1200; ++b) {
        d.process(ins, od); s.process(ins, os);
        for (std::size_t k = 0; k < kB; ++k) {
            if (std::fabs(od[0].at(0, k) - os[0].at(0, k)) > 1e-5f) differ = true;
            if (b > 200) { lo = std::min(lo, od[0].at(0, k)); hi = std::max(hi, od[0].at(0, k)); }
        }
    }
    check(differ, "`drift` > 0 altera a saída");
    check(hi - lo < 0.05f, "`drift` fica pequeno (< ~3%)");
    near(s.type().empty() ? 0.0 : os[0].at(0, 0), 0.4);  // sem drift = exato
}

void testDeterminismAndFinite() {
    Control a, b;
    for (Control* c : {&a, &b}) {
        c->setParameter("scale1", 1.3f);
        c->setParameter("offset1", 0.2f);
        c->setParameter("drift", 0.9f);
        c->setParameter("slew1", 0.3f);
        c->setParameter("rectify1", 0.7f);
    }
    a.prepare(kSr, kB); b.prepare(kSr, kB);
    std::vector<AudioBlock> oa(3, AudioBlock(kSr, 1, kB)), ob(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    double ph = 0.0;
    bool same = true;
    for (int bl = 0; bl < 300; ++bl) {
        for (std::size_t k = 0; k < kB; ++k) {
            i1.at(0, k) = std::sin(static_cast<float>(ph));
            ph += 6.2831853 * 90.0 / kSr;
        }
        std::vector<const AudioBlock*> ins{&i1, nullptr};
        a.process(ins, oa); b.process(ins, ob);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(oa[0].at(0, k)) && std::isfinite(oa[2].at(0, k)));
            if (oa[0].at(0, k) != ob[0].at(0, k)) same = false;
        }
    }
    EXPECT(same);
}

void testInGraph() {
    // SEQUENCE de altura -> CONTROL (slew = portamento) -> mede que a saída
    // não salta seco entre passos
    SignalGraph graph;
    const auto lfo = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfo).setParameter("rate", 6.0f);
    const auto ctl = graph.add(std::make_unique<Control>());
    graph.node(ctl).setParameter("scale1", 1.0f);
    graph.node(ctl).setParameter("slew1", 0.4f);
    graph.node(ctl).setParameter("curve1", 0.5f);
    const auto osc = graph.add(std::make_unique<Oscillator>());
    graph.connect(lfo, 1, ctl, 0);       // bi -> in1
    graph.connect(ctl, 0, osc, 0);       // out1 -> pitch
    graph.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float prev = 0.0f, maxJump = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, ctl, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            maxJump = std::max(maxJump, std::fabs(out.at(0, k) - prev));
            prev = out.at(0, k);
        }
    }
    check(maxJump < 0.01f, "slew no grafo: a CV desliza, não salta");
}

void testPanel() {
    Control c;
    const std::string problem = validatePanel(c);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(c.panel().widgets.size() >= 15);
    std::cout << renderAscii(c);
}

}  // namespace

int main() {
    testScale();
    testScaleZeroIsVoltageSource();
    testOffset();
    testRectifyFull();
    testRectifyHalf();
    testSlewInstant();
    testSlewRamp();
    testCurveLinearVsExp();
    testEnvelopeFollower();
    testSum();
    testDrift();
    testDeterminismAndFinite();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_control: OK\n";
    return g_failures == 0 ? 0 : 1;
}
