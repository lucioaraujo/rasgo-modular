// Teste isolado do Módulo 30 (TRIGSEQ — grade de trigs / percussão).
// Critérios do dossiê `dossies/30_trigseq.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Lpg.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/TrigSeq.hpp"
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

struct Pulse {
    std::size_t period, hi, t = 0;
    float next() { const float v = (t % period) < hi ? 1.0f : 0.0f; ++t; return v; }
};

struct Rec {
    std::vector<float> o[6];              // saídas t1..t4, accent, any
    int edges[6] = {0, 0, 0, 0, 0, 0};
    std::vector<std::size_t> edgePos[6];  // amostra de cada borda de subida
};

// roda `ts` por `blocks` blocos. `clk` opcional (senão relógio interno).
Rec run(TrigSeq& ts, int blocks, Pulse* clk = nullptr,
        Pulse* rst = nullptr, bool fillHi = false) {
    ts.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock bclk(kSr, 1, kB), brst(kSr, 1, kB), bfill(kSr, 1, kB),
        bmap(kSr, 1, kB);
    Rec r;
    float prev[6] = {0, 0, 0, 0, 0, 0};
    std::size_t globalSample = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bclk.at(0, k) = clk ? clk->next() : 0.0f;
            brst.at(0, k) = rst ? rst->next() : 0.0f;
            bfill.at(0, k) = fillHi ? 1.0f : 0.0f;
            bmap.at(0, k) = 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            clk ? &bclk : nullptr, rst ? &brst : nullptr,
            fillHi ? &bfill : nullptr, nullptr};
        ts.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            for (int o = 0; o < 6; ++o) {
                const float v = out[o].at(0, k);
                check(std::isfinite(v), "finito");
                check(v == 0.0f || v == 1.0f, "saída é 0/1");
                if (v >= 0.5f && prev[o] < 0.5f) {
                    ++r.edges[o];
                    r.edgePos[o].push_back(globalSample + k);
                }
                prev[o] = v;
                r.o[o].push_back(v);
            }
            ++globalSample;
        }
        globalSample = static_cast<std::size_t>(b + 1) * kB;
    }
    return r;
}

// nº de degraus de subida em `n` loops de 16 passos, dado clock de período P
int loopsBlocks(int loops, std::size_t period) {
    const std::size_t samples = static_cast<std::size_t>(loops) * 16 * period + period;
    return static_cast<int>(samples / kB) + 2;
}

void set(TrigSeq& ts) {
    ts.setParameter("map", 0.0f);       // caractere 0 (straight) puro
    ts.setParameter("chaos", 0.0f);
    ts.setParameter("swing", 0.0f);
    ts.setParameter("ratchet", 0.0f);
    ts.setParameter("drift", 0.0f);
}

void testDensityZeroOnlyDownbeats() {
    TrigSeq ts; set(ts);
    ts.setParameter("density1", 0.0f);
    Pulse clk{2000, 80};
    const Rec r = run(ts, loopsBlocks(3, 2000), &clk);
    // char 0 kick: peso 255 nos passos 0 e 8 -> 2 por loop, ~6 em 3 loops
    check(r.edges[0] >= 5 && r.edges[0] <= 7, "density=0 -> só os downbeats (t1)");
}

void testDensityOneAllWeighted() {
    TrigSeq ts; set(ts);
    ts.setParameter("density1", 1.0f);
    Pulse clk{2000, 80};
    const Rec r = run(ts, loopsBlocks(3, 2000), &clk);
    // char 0 kick: peso>0 nos passos 0,6,8,14 -> 4 por loop, ~12 em 3 loops
    check(r.edges[0] >= 10 && r.edges[0] <= 14, "density=1 -> todos os passos com peso (t1)");
}

void testDensityMonotonic() {
    int prev = -1;
    for (float d = 0.0f; d <= 1.0f; d += 0.25f) {
        TrigSeq ts; set(ts);
        ts.setParameter("density1", d);
        Pulse clk{2000, 80};
        const Rec r = run(ts, loopsBlocks(3, 2000), &clk);
        check(r.edges[0] >= prev, "t1 não-decrescente com density1");
        prev = r.edges[0];
    }
}

void testMapChangesPattern() {
    TrigSeq a, b;
    for (TrigSeq* t : {&a, &b}) {
        set(*t);
        t->setParameter("density1", 0.6f);
        t->setParameter("density2", 0.6f);
        t->setParameter("density3", 0.6f);
        t->setParameter("density4", 0.6f);
    }
    a.setParameter("map", 0.0f);   // straight
    b.setParameter("map", 1.0f);   // sparse
    Pulse ca{2000, 80}, cb{2000, 80};
    const Rec ra = run(a, loopsBlocks(3, 2000), &ca);
    const Rec rb = run(b, loopsBlocks(3, 2000), &cb);
    const int ta = ra.edges[0] + ra.edges[1] + ra.edges[2] + ra.edges[3];
    const int tb = rb.edges[0] + rb.edges[1] + rb.edges[2] + rb.edges[3];
    check(ta != tb, "map muda o padrão (contagem total de trigs difere)");
    check(ta > tb, "straight é mais denso que sparse");
}

void testSwingDelays() {
    // com todas as densidades no máximo, `any` dispara em quase todo passo;
    // com swing, os passos ímpares atrasam -> os intervalos alternam
    TrigSeq straight, swung;
    for (TrigSeq* t : {&straight, &swung}) {
        set(*t);
        t->setParameter("map", 0.5f);
        for (const char* d : {"density1", "density2", "density3", "density4"})
            t->setParameter(d, 1.0f);
    }
    swung.setParameter("swing", 0.7f);
    Pulse c1{2000, 40}, c2{2000, 40};
    const Rec rs = run(straight, loopsBlocks(3, 2000), &c1);
    const Rec rw = run(swung, loopsBlocks(3, 2000), &c2);
    auto spread = [](const std::vector<std::size_t>& p) {
        long minD = 1 << 30, maxD = 0;
        for (std::size_t i = 2; i < p.size(); ++i) {
            const long d = static_cast<long>(p[i] - p[i - 1]);
            if (d < minD) minD = d;
            if (d > maxD) maxD = d;
        }
        return maxD - minD;
    };
    check(rs.edgePos[5].size() > 10 && rw.edgePos[5].size() > 10,
          "trigs suficientes em `any`");
    check(spread(rw.edgePos[5]) > spread(rs.edgePos[5]) + 300,
          "swing torna os intervalos desiguais (passos ímpares atrasam)");
}

void testChaosDeterministic() {
    TrigSeq a, b;
    for (TrigSeq* t : {&a, &b}) {
        set(*t);
        t->setParameter("chaos", 0.0f);
        t->setParameter("density1", 0.5f);
    }
    Pulse ca{1900, 60}, cb{1900, 60};
    const Rec ra = run(a, loopsBlocks(3, 1900), &ca);
    const Rec rb = run(b, loopsBlocks(3, 1900), &cb);
    bool same = true;
    for (int o = 0; o < 6 && same; ++o) {
        if (ra.o[o].size() != rb.o[o].size()) same = false;
        for (std::size_t i = 0; same && i < ra.o[o].size(); ++i)
            if (ra.o[o][i] != rb.o[o][i]) same = false;
    }
    EXPECT(same);
}

void testChaosAddsHits() {
    TrigSeq dry, wild;
    for (TrigSeq* t : {&dry, &wild}) {
        set(*t);
        t->setParameter("density1", 0.3f);
        t->setParameter("density2", 0.3f);
        t->setParameter("density3", 0.3f);
        t->setParameter("density4", 0.3f);
    }
    wild.setParameter("chaos", 0.85f);
    Pulse c1{1900, 60}, c2{1900, 60};
    const Rec rd = run(dry, loopsBlocks(4, 1900), &c1);
    const Rec rw = run(wild, loopsBlocks(4, 1900), &c2);
    const int td = rd.edges[0] + rd.edges[1] + rd.edges[2] + rd.edges[3];
    const int tw = rw.edges[0] + rw.edges[1] + rw.edges[2] + rw.edges[3];
    check(tw > td, "chaos alto adiciona trigs (notas-fantasma)");
}

void testFillBoosts() {
    TrigSeq lo, hi;
    for (TrigSeq* t : {&lo, &hi}) {
        set(*t);
        for (const char* d : {"density1", "density2", "density3", "density4"})
            t->setParameter(d, 0.25f);
        t->setParameter("fill_amt", 0.9f);
    }
    Pulse c1{1900, 60}, c2{1900, 60};
    const Rec rl = run(lo, loopsBlocks(4, 1900), &c1, nullptr, false);
    const Rec rh = run(hi, loopsBlocks(4, 1900), &c2, nullptr, true);
    const int tl = rl.edges[0] + rl.edges[1] + rl.edges[2] + rl.edges[3];
    const int th = rh.edges[0] + rh.edges[1] + rh.edges[2] + rh.edges[3];
    check(th > tl, "fill alto -> densidade sobe em todas as linhas");
}

void testRatchet() {
    TrigSeq ts; set(ts);
    ts.setParameter("density1", 1.0f);
    ts.setParameter("density2", 1.0f);
    ts.setParameter("ratchet", 1.0f);
    Pulse clk{2400, 60};
    const Rec r = run(ts, loopsBlocks(4, 2400), &clk);
    // com ratchet, aparecem trigs da mesma linha separados por ~período/3
    // (~800 amostras) — bem menos que o passo (2400)
    int shortGaps = 0;
    for (int o = 0; o < 2; ++o)
        for (std::size_t i = 1; i < r.edgePos[o].size(); ++i) {
            const long d = static_cast<long>(r.edgePos[o][i] - r.edgePos[o][i - 1]);
            if (d > 200 && d < 1500) ++shortGaps;
        }
    check(shortGaps >= 3, "ratchet gera rajadas (intervalos curtos na mesma linha)");
}

void testAccentCoincidence() {
    TrigSeq ts; set(ts);
    for (const char* d : {"density1", "density2", "density3", "density4"})
        ts.setParameter(d, 0.8f);
    Pulse clk{2000, 60};
    const Rec r = run(ts, loopsBlocks(3, 2000), &clk);
    // em toda borda de accent, pelo menos 2 saídas de linha estão altas
    int bad = 0;
    for (std::size_t pos : r.edgePos[4]) {
        int on = 0;
        for (int l = 0; l < 4; ++l)
            if (pos < r.o[l].size() && r.o[l][pos] >= 0.5f) ++on;
        if (on < 2) ++bad;
    }
    check(r.edges[4] > 0, "accent dispara em algum momento");
    check(bad == 0, "accent só quando >=2 linhas coincidem");
}

void testAnyIsOr() {
    TrigSeq ts; set(ts);
    for (const char* d : {"density1", "density2", "density3", "density4"})
        ts.setParameter(d, 0.7f);
    // ratchet=0, swing=0 -> `any` sobe/desce junto com as linhas
    Pulse clk{2000, 60};
    const Rec r = run(ts, loopsBlocks(3, 2000), &clk);
    int mism = 0;
    for (std::size_t i = 0; i < r.o[5].size(); ++i) {
        const bool anyLane = r.o[0][i] >= 0.5f || r.o[1][i] >= 0.5f
            || r.o[2][i] >= 0.5f || r.o[3][i] >= 0.5f;
        if ((r.o[5][i] >= 0.5f) != anyLane) ++mism;
    }
    check(mism == 0, "any == OR das 4 linhas (sem ratchet/swing)");
}

void testInternalClock() {
    TrigSeq ts; set(ts);
    ts.setParameter("rate", 15.0f);
    ts.setParameter("density3", 1.0f);   // chimbal nos passos pares (char 0)
    const Rec r = run(ts, 400, nullptr);  // sem clock -> relógio interno
    // 400*128/48000 = 1.067 s * 15 Hz = ~16 passos; chimbal em passo par -> ~8
    check(r.edges[2] >= 5 && r.edges[2] <= 12,
          "relógio interno gera passos sem clock");
}

void testReset() {
    TrigSeq ts; set(ts);
    ts.setParameter("density1", 0.0f);   // t1 só no passo 0 e 8
    Pulse clk{2000, 60};
    Pulse rst{2000 * 5, 60};              // reset a cada 5 passos
    const Rec r = run(ts, loopsBlocks(4, 2000), &clk, &rst);
    // com reset frequente ao passo 0, o downbeat (t1) dispara mais vezes
    // que sem reset
    TrigSeq ref; set(ref);
    ref.setParameter("density1", 0.0f);
    Pulse c2{2000, 60};
    const Rec r0 = run(ref, loopsBlocks(4, 2000), &c2);
    check(r.edges[0] > r0.edges[0], "reset volta ao passo 0 (mais downbeats)");
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 140.0f);
    const auto tsq = g.add(std::make_unique<TrigSeq>());
    g.node(tsq).setParameter("density1", 0.8f);
    const auto lpg = g.add(std::make_unique<Lpg>());
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 90.0f);
    g.connect(clk, 0, tsq, 0);      // clock -> TRIGSEQ.clock
    g.connect(osc, 2, lpg, 0);      // OSC saw -> LPG.in
    g.connect(tsq, 0, lpg, 1);      // t1 -> LPG.strike
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 4000; ++b) {
        g.process(out, lpg, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.02f, "TRIGSEQ dispara a voz percussiva no grafo");
    (void)lo;
}

void testDeterminismWithChaos() {
    TrigSeq a, b;
    for (TrigSeq* t : {&a, &b}) {
        t->setParameter("map", 0.4f);
        t->setParameter("chaos", 0.5f);
        t->setParameter("ratchet", 0.4f);
        t->setParameter("swing", 0.3f);
        t->setParameter("drift", 0.5f);
    }
    Pulse ca{2100, 70}, cb{2100, 70};
    const Rec ra = run(a, loopsBlocks(4, 2100), &ca);
    const Rec rb = run(b, loopsBlocks(4, 2100), &cb);
    bool same = true;
    for (int o = 0; o < 6 && same; ++o)
        for (std::size_t i = 0; i < ra.o[o].size() && same; ++i)
            if (ra.o[o][i] != rb.o[o][i]) same = false;
    EXPECT(same);
}

void testPanel() {
    TrigSeq ts;
    const std::string problem = validatePanel(ts);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(ts.panel().widgets.size() >= 20);
    std::cout << renderAscii(ts);
}

}  // namespace

int main() {
    testDensityZeroOnlyDownbeats();
    testDensityOneAllWeighted();
    testDensityMonotonic();
    testMapChangesPattern();
    testSwingDelays();
    testChaosDeterministic();
    testChaosAddsHits();
    testFillBoosts();
    testRatchet();
    testAccentCoincidence();
    testAnyIsOr();
    testInternalClock();
    testReset();
    testInGraph();
    testDeterminismWithChaos();
    testPanel();
    if (g_failures == 0) std::cout << "test_trigseq: OK\n";
    return g_failures == 0 ? 0 : 1;
}
