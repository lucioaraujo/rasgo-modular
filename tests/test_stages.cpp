// Teste isolado do Módulo 54 (STAGES — gerador de segmentos configuráveis).
// Critérios do dossiê `dossies/54_stages.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Stages.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <set>
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
struct Ins { Gen gate, reset, rateMod; };

std::vector<float> run(Stages& s, int blocks, const Ins& g, int outIdx = 0) {
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bg(kSr, 1, kB), br(kSr, 1, kB), bm(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bg.at(0, k) = g.gate ? g.gate(n + k) : 0.0f;
            br.at(0, k) = g.reset ? g.reset(n + k) : 0.0f;
            bm.at(0, k) = g.rateMod ? g.rateMod(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.gate ? &bg : nullptr, g.reset ? &br : nullptr,
            g.rateMod ? &bm : nullptr};
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return r;
}

int risingEdges(const std::vector<float>& v, std::size_t a, std::size_t b) {
    int e = 0;
    for (std::size_t i = a + 1; i < b && i < v.size(); ++i)
        if (v[i - 1] < 0.5f && v[i] >= 0.5f) ++e;
    return e;
}
Gen trigAt(std::size_t at) {
    return [at](std::size_t n) { return (n >= at && n < at + 64) ? 1.0f : 0.0f; };
}

void testLoopSmoothPeriodic() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("hold", 0.0f);
    s.setParameter("rate", 2.0f);
    s.setParameter("segments", 4.0f);
    Ins g;
    const auto r = run(s, 600, g);   // ~3,2 s
    const std::size_t per = 24000;   // 48000/2
    double err = 0.0; std::size_t cnt = 0;
    for (std::size_t i = 80000; i + per < r.size(); ++i, ++cnt)
        err += std::fabs(r[i] - r[i + per]);
    EXPECT(err / (double)cnt < 0.01);
    double maxd = 0.0;
    for (std::size_t i = 80001; i < r.size(); ++i)
        maxd = std::max(maxd, (double)std::fabs(r[i] - r[i - 1]));
    EXPECT(maxd < 0.02);
}

void testHoldStaircase() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("hold", 1.0f);
    s.setParameter("rate", 2.0f);
    s.setParameter("segments", 5.0f);
    Ins g;
    const auto r = run(s, 500, g);
    std::size_t flat = 0;
    for (std::size_t i = 60001; i < r.size(); ++i)
        if (r[i] == r[i - 1]) ++flat;
    EXPECT((double)flat / (double)(r.size() - 60001) > 0.97);
    // no máximo `segments` níveis dominantes (ignora os poucos samples de
    // fronteira contando só platôs de ≥ 50 samples)
    std::set<int> dom;
    std::size_t runLen = 1;
    for (std::size_t i = 60001; i < r.size(); ++i) {
        if (r[i] == r[i - 1]) { ++runLen; }
        else {
            if (runLen >= 50)
                dom.insert(static_cast<int>(std::lround(r[i - 1] * 500.0f)));
            runLen = 1;
        }
    }
    EXPECT(dom.size() <= 5);
}

void testStepAndEocPulses() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("rate", 2.0f);      // volta = 24000 samples
    s.setParameter("segments", 4.0f);
    Ins g;
    const auto eoc = run(s, 600, g, 1);
    const auto stp = run(s, 600, g, 2);
    // janela [24000, fim] ≈ 129600 samples ≈ 5,4 voltas
    const int ne = risingEdges(eoc, 24000, eoc.size());
    const int ns = risingEdges(stp, 24000, stp.size());
    EXPECT(ne >= 4 && ne <= 7);
    EXPECT(ns >= 4 * ne - 3 && ns <= 4 * ne + 4);
}

void testContourDirection() {
    auto levels = [](float contour) {
        Stages s;
        s.setParameter("loop", 1.0f);
        s.setParameter("hold", 1.0f);
        s.setParameter("rate", 1.0f);        // volta = 48000
        s.setParameter("segments", 4.0f);
        s.setParameter("contour", contour);
        Ins g;
        const auto r = run(s, 700, g);
        // meio de cada um dos 4 segmentos numa volta assentada (começa em 96000)
        std::vector<float> lv;
        for (int k = 0; k < 4; ++k)
            lv.push_back(r[96000 + (std::size_t)(k * 12000 + 6000)]);
        return lv;
    };
    const auto up = levels(0.0f);
    EXPECT(up[0] < up[1] && up[1] < up[2] && up[2] < up[3]);
    const auto down = levels(1.0f);
    EXPECT(down[0] > down[1] && down[1] > down[2] && down[2] > down[3]);
}

void testTilt() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("hold", 1.0f);
    s.setParameter("rate", 1.0f);
    s.setParameter("segments", 4.0f);
    s.setParameter("contour", 0.0f);
    s.setParameter("tilt", -0.85f);        // 1º segmento MUITO mais longo
    Ins g;
    const auto r = run(s, 500, g);
    // conta samples no nível 0 (segmento 0) vs no nível máx (último)
    std::size_t at0 = 0, atMax = 0;
    float mx = -2.0f;
    for (std::size_t i = 96000; i < 144000 && i < r.size(); ++i) mx = std::max(mx, r[i]);
    for (std::size_t i = 96000; i < 144000 && i < r.size(); ++i) {
        if (r[i] < 0.08f) ++at0;
        if (r[i] > mx - 0.03f) ++atMax;
    }
    EXPECT(at0 > 2 * atMax);
}

void testOneShot() {
    Stages s;
    s.setParameter("loop", 0.0f);
    s.setParameter("hold", 0.0f);
    s.setParameter("rate", 4.0f);          // volta em 12000 samples
    s.setParameter("segments", 3.0f);
    s.setParameter("contour", 0.0f);
    Ins g;
    g.gate = [](std::size_t n) {
        return ((n >= 5000 && n < 5200) || (n >= 60000 && n < 60200))
            ? 1.0f : 0.0f;
    };
    const auto r = run(s, 400, g);
    const float held = r[40000];
    EXPECT(held > 0.85f);              // congelou perto do nível final
    bool frozen = true;
    for (std::size_t i = 40000; i < 58000; ++i)
        if (std::fabs(r[i] - held) > 1e-4f) frozen = false;
    EXPECT(frozen);
    EXPECT(r[62000] < 0.6f);           // 2º gate reiniciou
}

void testReset() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("hold", 0.0f);
    s.setParameter("rate", 1.0f);
    s.setParameter("segments", 4.0f);
    s.setParameter("contour", 0.0f);
    Ins g;
    g.reset = trigAt(30000);
    const auto r = run(s, 200, g);
    EXPECT(std::fabs(r[30500]) < 0.15f);
}

void testCurve() {
    auto early = [](float curve) {
        Stages s;
        s.setParameter("loop", 1.0f);
        s.setParameter("hold", 0.0f);
        s.setParameter("rate", 1.0f);         // volta = 48000; seg 0 = [0, 24000)
        s.setParameter("segments", 2.0f);
        s.setParameter("contour", 0.0f);      // sobe 0 → 1
        s.setParameter("curve", curve);
        Ins g;
        const auto r = run(s, 420, g);
        // ~10% do segmento 0 numa volta assentada: 96000 + 2400
        return r[96000 + 2400];
    };
    EXPECT(early(-0.8f) > 0.25f);   // exp: começa rápido
    EXPECT(early(0.8f) < 0.06f);    // log: começa devagar
}

void testJitterDeterministic() {
    auto trace = [](float jit) {
        Stages s;
        s.setParameter("loop", 1.0f);
        s.setParameter("rate", 3.0f);
        s.setParameter("segments", 5.0f);
        s.setParameter("jitter", jit);
        Ins g;
        return run(s, 500, g);
    };
    const auto a = trace(0.5f), b = trace(0.5f);
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i) same = a[i] == b[i];
    EXPECT(same);
    const auto j = trace(0.5f);
    const std::size_t per = 16000;
    double d = 0.0; std::size_t cnt = 0;
    for (std::size_t i = 70000; i + per < j.size(); ++i, ++cnt)
        d += std::fabs(j[i] - j[i + per]);
    EXPECT(d / (double)cnt > 0.004);
    const auto z = trace(0.0f);
    double dz = 0.0; cnt = 0;
    for (std::size_t i = 70000; i + per < z.size(); ++i, ++cnt)
        dz += std::fabs(z[i] - z[i + per]);
    EXPECT(dz / (double)cnt < 0.002);
}

void testBounded() {
    Stages s;
    s.setParameter("loop", 1.0f);
    s.setParameter("jitter", 1.0f);
    s.setParameter("rate", 8.0f);
    s.setParameter("segments", 8.0f);
    s.setParameter("tilt", 0.9f);
    Ins g;
    const auto r = run(s, 200, g);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) <= 1.001f);
}

void testPanel() {
    Stages s;
    check(validatePanel(s).empty(), "painel STAGES fecha");
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testLoopSmoothPeriodic();
    testHoldStaircase();
    testStepAndEocPulses();
    testContourDirection();
    testTilt();
    testOneShot();
    testReset();
    testCurve();
    testJitterDeterministic();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular STAGES tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
