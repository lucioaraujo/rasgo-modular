// Teste isolado do Módulo 59 (VCA4 — banco de 4 VCAs + mixer).
// Critérios do dossiê `dossies/59_vca4.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Vca4.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <functional>
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

using Gen = std::function<float(std::size_t)>;

struct Ins {
    Gen in[4] = {nullptr, nullptr, nullptr, nullptr};
    Gen cv[4] = {nullptr, nullptr, nullptr, nullptr};
};

// devolve os 5 canais de saída (out1..out4, mix) concatenados? não —
// devolve o canal `pick` (0..4).
std::vector<float> run(Vca4& v, int blocks, const Ins& g, int pick) {
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock bi[4] = {AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB),
                        AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB)};
    AudioBlock bc[4] = {AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB),
                        AudioBlock(kSr, 1, kB), AudioBlock(kSr, 1, kB)};
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        std::vector<const AudioBlock*> ins;
        for (int k = 0; k < 4; ++k) {
            for (std::size_t s = 0; s < kB; ++s) {
                if (g.in[k]) bi[k].at(0, s) = g.in[k](n + s);
                if (g.cv[k]) bc[k].at(0, s) = g.cv[k](n + s);
            }
            ins.push_back(g.in[k] ? &bi[k] : nullptr);
            ins.push_back(g.cv[k] ? &bc[k] : nullptr);
        }
        v.process(ins, out);
        for (std::size_t s = 0; s < kB; ++s)
            r.push_back(out[static_cast<std::size_t>(pick)].at(0, s));
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& x, std::size_t a) {
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < x.size(); ++i, ++c) s += x[i] * (double)x[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
}

Gen sine(double hz, float amp = 0.5f) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(6.2831853 * hz * (double)n / kSr);
    };
}
Gen dc(float v) { return [v](std::size_t) { return v; }; }

// ---- testes ----

void testPerChannelGain() {
    Vca4 v;
    v.setParameter("level1", 1.0f);
    v.setParameter("level2", 0.5f);
    v.setParameter("level3", 0.0f);
    Ins g;
    g.in[0] = g.in[1] = g.in[2] = sine(440.0);
    EXPECT(rms(run(v, 60, g, 0), 6000) > 0.30);              // level 1.0
    Vca4 v2; v2.setParameter("level1",1.0f); v2.setParameter("level2",0.5f);
    v2.setParameter("level3",0.0f);
    const double r2 = rms(run(v2, 60, g, 1), 6000);
    const double r1 = rms(run(v, 60, g, 0), 6000);
    EXPECT(std::fabs(r2 / r1 - 0.5) < 0.06);                 // level 0.5 = metade
    Vca4 v3; v3.setParameter("level3",0.0f);
    EXPECT(rms(run(v3, 60, g, 2), 6000) < 0.01);             // level 0 = mudo
}

void testCvOpensChannel() {
    Vca4 v;
    v.setParameter("level1", 0.0f);
    v.setParameter("cv1_amt", 1.0f);
    Ins g;
    g.in[0] = sine(300.0);
    g.cv[0] = dc(0.8f);                                       // CV abre o canal
    EXPECT(rms(run(v, 60, g, 0), 6000) > 0.25);
}

void testCvAttenuvert() {
    // cv_amt negativo INVERTE a ação da CV
    Vca4 a; a.setParameter("level1", 0.5f); a.setParameter("cv1_amt", -1.0f);
    Ins g; g.in[0] = sine(300.0); g.cv[0] = dc(0.5f);
    EXPECT(rms(run(a, 60, g, 0), 6000) < 0.02);              // 0.5 − 0.5 = 0
}

void testMixSumsChannels() {
    Vca4 v;
    v.setParameter("level1", 0.5f);
    v.setParameter("level2", 0.5f);
    v.setParameter("mix_gain", 1.0f);
    Ins g;
    g.in[0] = sine(200.0, 0.4f);
    g.in[1] = sine(700.0, 0.4f);
    const auto m = run(v, 60, g, 4);                         // saída `mix`
    // a mix carrega os DOIS parciais
    auto magAt = [&](double hz) {
        double re = 0, im = 0, ws = 0; const double wk = 2 * M_PI * hz / kSr;
        for (std::size_t i = 6000; i < m.size(); ++i) {
            const double w = 1.0;
            re += w * m[i] * std::cos(wk * i);
            im += w * m[i] * std::sin(wk * i);
            ws += w;
        }
        return std::sqrt(re * re + im * im) / ws;
    };
    EXPECT(magAt(200.0) > 0.05);
    EXPECT(magAt(700.0) > 0.05);
}

void testCurveExp() {
    // curve = 1 (exp): meio de curso dá MENOS que linear
    Vca4 lin; lin.setParameter("level1", 0.5f); lin.setParameter("curve", 0.0f);
    Vca4 exp; exp.setParameter("level1", 0.5f); exp.setParameter("curve", 1.0f);
    Ins g; g.in[0] = sine(440.0);
    EXPECT(rms(run(exp, 60, g, 0), 6000) < 0.6 * rms(run(lin, 60, g, 0), 6000));
}

void testDeterminismAndDrift() {
    auto mk = [](Vca4& v) {
        v.setParameter("level1", 0.6f);
        v.setParameter("level2", 0.4f);
        v.setParameter("drift", 1.0f);
    };
    Vca4 a; mk(a);
    Vca4 b; mk(b);
    Ins g; g.in[0] = sine(330.0); g.in[1] = sine(550.0);
    const auto ra = run(a, 100, g, 4);
    const auto rb = run(b, 100, g, 4);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);                                            // drift semeado
}

void testBounded() {
    Vca4 v;
    for (int k = 1; k <= 4; ++k)
        v.setParameter("level" + std::to_string(k), 1.0f);
    v.setParameter("mix_gain", 2.0f);
    v.setParameter("drift", 1.0f);
    Ins g;
    for (int k = 0; k < 4; ++k) g.in[k] = sine(110.0 * (k + 1), 0.9f);
    for (int pick = 0; pick < 5; ++pick) {
        const auto y = run(v, 80, g, pick);
        for (float x : y) EXPECT(std::isfinite(x) && std::fabs(x) < 1.05f);
    }
}

void testPanel() {
    Vca4 v;
    check(validatePanel(v).empty(), "painel VCA4 fecha");
    std::cout << renderAscii(v);
}

}  // namespace

int main() {
    testPerChannelGain();
    testCvOpensChannel();
    testCvAttenuvert();
    testMixSumsChannels();
    testCurveExp();
    testDeterminismAndDrift();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular VCA4 tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
