// Teste isolado do Módulo 20 (VCA) - antes de entrar num patch.
// Critérios do dossiê `dossies/20_vca.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Vca.hpp"
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

struct Ins { float in1, cv1, in2, cv2; bool hasCv1, hasCv2, hasIn2; };

// devolve o último valor de uma saída depois de `blocks` blocos com
// entradas DC constantes.
float dc(Vca& v, const int outIdx, const Ins& s, int blocks = 60) {
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB), c1(kSr, 1, kB), i2(kSr, 1, kB), c2(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) {
        i1.at(0, k) = s.in1; c1.at(0, k) = s.cv1;
        i2.at(0, k) = s.in2; c2.at(0, k) = s.cv2;
    }
    std::vector<const AudioBlock*> ins{
        &i1, s.hasCv1 ? &c1 : nullptr, s.hasIn2 ? &i2 : nullptr,
        s.hasCv2 ? &c2 : nullptr};
    for (int b = 0; b < blocks; ++b) v.process(ins, out);
    return out[static_cast<std::size_t>(outIdx)].at(0, 0);
}

void testLinearGain() {
    Vca v;
    v.setParameter("level1", 0.5f);
    near(dc(v, 0, {1.0f, 0, 0, 0, false, false, false}), 0.5);
    v.setParameter("level1", 0.0f);
    near(dc(v, 0, {1.0f, 0, 0, 0, false, false, false}), 0.0);
    v.setParameter("level1", 1.0f);
    near(dc(v, 0, {0.5f, 0, 0, 0, false, false, false}), 0.5);
}

void testCvAttenuvert() {
    Vca v;
    v.setParameter("level1", 0.0f);
    v.setParameter("cv1_amount", 1.0f);
    // knob 0, CV 0.5 atenuvertida por +1 -> ganho 0.5
    near(dc(v, 0, {1.0f, 0.5f, 0, 0, true, false, false}), 0.5);
    // cv_amount -1 com CV +0.5 -> ganho -0.5 -> clampado a 0
    v.setParameter("cv1_amount", -1.0f);
    near(dc(v, 0, {1.0f, 0.5f, 0, 0, true, false, false}), 0.0);
    // cv_amount -1 com CV -0.5 -> ganho +0.5
    near(dc(v, 0, {1.0f, -0.5f, 0, 0, true, false, false}), 0.5);
    // knob soma: knob 0.3 + CV atenuvertida 0.4 -> 0.7
    v.setParameter("level1", 0.3f);
    v.setParameter("cv1_amount", 1.0f);
    near(dc(v, 0, {1.0f, 0.4f, 0, 0, true, false, false}), 0.7);
}

void testResponseCurve() {
    Vca lin, exp_;
    lin.setParameter("level1", 0.5f);
    lin.setParameter("response1", 0.0f);
    exp_.setParameter("level1", 0.5f);
    exp_.setParameter("response1", 1.0f);
    const float gl = dc(lin, 0, {1.0f, 0, 0, 0, false, false, false});
    const float ge = dc(exp_, 0, {1.0f, 0, 0, 0, false, false, false});
    near(gl, 0.5);
    check(ge < gl * 0.3f, "response alto encurva (0,5^4 ~ 0,06)");
}

void testSmoothing() {
    Vca v;
    v.setParameter("level1", 0.0f);
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 1.0f;
    std::vector<const AudioBlock*> ins{&i1, nullptr, nullptr, nullptr};
    for (int b = 0; b < 8; ++b) v.process(ins, out);      // assenta em 0
    v.setParameter("level1", 1.0f);                        // degrau abrupto
    float prev = out[0].at(0, kB - 1);
    float maxJump = 0.0f;
    for (int b = 0; b < 20; ++b) {
        v.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            maxJump = std::max(maxJump, std::fabs(out[0].at(0, k) - prev));
            prev = out[0].at(0, k);
        }
    }
    check(maxJump < 0.02f, "degrau de ganho suavizado (sem estalo)");
}

void testSoftSat() {
    Vca v;
    v.setParameter("level1", 1.0f);
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    double ph = 0.0;
    float peak = 0.0f;
    std::vector<const AudioBlock*> ins{&i1, nullptr, nullptr, nullptr};
    for (int b = 0; b < 400; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            i1.at(0, k) = 1.6f * std::sin(static_cast<float>(ph));
            ph += 6.2831853 * 220.0 / kSr;
        }
        v.process(ins, out);
        if (b > 20)
            for (std::size_t k = 0; k < kB; ++k) {
                EXPECT(std::isfinite(out[0].at(0, k)));
                peak = std::max(peak, std::fabs(out[0].at(0, k)));
            }
    }
    check(peak <= 1.01f, "softSat segura AM forte perto de ±1");
}

void testSum() {
    Vca v;
    v.setParameter("level1", 0.5f);
    v.setParameter("level2", 0.5f);
    const float s = dc(v, 2, {0.6f, 0, -0.2f, 0, false, false, true});
    near(s, 0.6 * 0.5 + (-0.2) * 0.5);   // 0.2
}

void testChannelsIndependent() {
    Vca v;
    v.setParameter("level1", 1.0f);
    v.setParameter("level2", 0.0f);
    near(dc(v, 0, {0.5f, 0, 0.5f, 0, false, false, true}), 0.5);
    near(dc(v, 1, {0.5f, 0, 0.5f, 0, false, false, true}), 0.0);
}

void testDrift() {
    Vca d, s;
    d.setParameter("level1", 0.7f);
    d.setParameter("drift", 1.0f);
    s.setParameter("level1", 0.7f);
    s.setParameter("drift", 0.0f);
    d.prepare(kSr, kB); s.prepare(kSr, kB);
    std::vector<AudioBlock> od(3, AudioBlock(kSr, 1, kB)), os(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 1.0f;
    std::vector<const AudioBlock*> ins{&i1, nullptr, nullptr, nullptr};
    bool differ = false;
    float lo = 1.0f, hi = 0.0f;
    for (int b = 0; b < 800; ++b) {
        d.process(ins, od); s.process(ins, os);
        for (std::size_t k = 0; k < kB; ++k) {
            if (std::fabs(od[0].at(0, k) - os[0].at(0, k)) > 1e-4f) differ = true;
            if (b > 100) { lo = std::min(lo, od[0].at(0, k)); hi = std::max(hi, od[0].at(0, k)); }
        }
    }
    check(differ, "`drift` > 0 altera a saída");
    check(hi - lo < 0.07f && lo > 0.63f, "`drift` fica < ~5%");
}

void testDeterminism() {
    Vca a, b;
    for (Vca* v : {&a, &b}) {
        v->setParameter("level1", 0.6f);
        v->setParameter("drift", 0.8f);
        v->setParameter("response1", 0.4f);
    }
    a.prepare(kSr, kB); b.prepare(kSr, kB);
    std::vector<AudioBlock> oa(3, AudioBlock(kSr, 1, kB)), ob(3, AudioBlock(kSr, 1, kB));
    AudioBlock i1(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) i1.at(0, k) = 0.5f;
    std::vector<const AudioBlock*> ins{&i1, nullptr, nullptr, nullptr};
    bool same = true;
    for (int bl = 0; bl < 300 && same; ++bl) {
        a.process(ins, oa); b.process(ins, ob);
        for (std::size_t k = 0; k < kB; ++k)
            if (oa[0].at(0, k) != ob[0].at(0, k)) same = false;
    }
    EXPECT(same);
}

void testInGraph() {
    SignalGraph graph;
    const auto osc = graph.add(std::make_unique<Oscillator>());
    graph.node(osc).setParameter("freq", 180.0f);
    const auto lfo = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfo).setParameter("rate", 4.0f);
    const auto vca = graph.add(std::make_unique<Vca>());
    graph.node(vca).setParameter("level1", 0.2f);
    graph.node(vca).setParameter("cv1_amount", 0.6f);
    graph.connect(osc, 2, vca, 0);    // saw -> in1
    graph.connect(lfo, 0, vca, 1);    // uni -> cv1
    graph.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, vca, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k));
            hi = std::max(hi, out.at(0, k));
        }
    }
    EXPECT(hi > 0.05f && lo < -0.05f);   // o VCA deixa passar áudio modulado
}

void testPanel() {
    Vca v;
    const std::string problem = validatePanel(v);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(v.panel().widgets.size() >= 12);
    std::cout << renderAscii(v);
}

}  // namespace

int main() {
    testLinearGain();
    testCvAttenuvert();
    testResponseCurve();
    testSmoothing();
    testSoftSat();
    testSum();
    testChannelsIndependent();
    testDrift();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_vca: OK\n";
    return g_failures == 0 ? 0 : 1;
}
