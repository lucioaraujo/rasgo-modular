// Teste isolado do Módulo 28 (SWITCH — chave sequencial).
// Critérios do dossiê `dossies/28_switch.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Switch.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
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
void near(double a, double b, double t = 0.01) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

// trem de pulsos: alto por `hi` a cada `period` amostras
struct Pulse {
    std::size_t period, hi, t = 0;
    float next() { const float v = (t % period) < hi ? 1.0f : 0.0f; ++t; return v; }
};
// pulso que começa BAIXO (offset de meio período) — a 1ª borda ↑ vem depois,
// então a posição inicial 0 é observável antes do primeiro avanço.
inline Pulse lowFirst(std::size_t period, std::size_t hi) {
    return Pulse{period, hi, period / 2};
}

struct In4 { float a = 0, b = 0, c = 0, d = 0; bool on[4] = {false, false, false, false}; };

struct Rec {
    std::vector<float> out, step;
    std::vector<int> pos;   // posição inferida de `step` (0..steps-1)
};

// roda `sw` por `blocks` blocos. `clk`/`rst`/`adr` = ponteiros opcionais.
Rec run(Switch& sw, int blocks, const In4& in, int steps,
        Pulse* clk = nullptr, Pulse* rst = nullptr,
        float* adr = nullptr, Pulse* adrPulse = nullptr) {
    sw.setParameter("steps", static_cast<float>(steps));
    sw.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock ba(kSr, 1, kB), bb(kSr, 1, kB), bc(kSr, 1, kB), bd(kSr, 1, kB);
    AudioBlock bclk(kSr, 1, kB), brst(kSr, 1, kB), badr(kSr, 1, kB);
    Rec r;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) {
            ba.at(0, k) = in.a; bb.at(0, k) = in.b;
            bc.at(0, k) = in.c; bd.at(0, k) = in.d;
            bclk.at(0, k) = clk ? clk->next() : 0.0f;
            brst.at(0, k) = rst ? rst->next() : 0.0f;
            if (adr) badr.at(0, k) = *adr;
            else if (adrPulse) badr.at(0, k) = adrPulse->next();
        }
        std::vector<const AudioBlock*> ins{
            in.on[0] ? &ba : nullptr, in.on[1] ? &bb : nullptr,
            in.on[2] ? &bc : nullptr, in.on[3] ? &bd : nullptr,
            clk ? &bclk : nullptr, rst ? &brst : nullptr,
            (adr || adrPulse) ? &badr : nullptr};
        sw.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float o = out[0].at(0, k), s = out[1].at(0, k);
            check(std::isfinite(o) && std::isfinite(s), "finito");
            r.out.push_back(o);
            r.step.push_back(s);
            r.pos.push_back(static_cast<int>(std::lround(s * (steps - 1))));
        }
    }
    return r;
}

void testDemux() {
    // dir=1: a entrada `a` (0.8) vai pra UMA saída conforme o passo; as
    // outras 3 ficam ~0. forward -> a saída ativa cicla.
    Switch sw;
    sw.setParameter("steps", 4.0f);
    sw.setParameter("dir", 1.0f);
    sw.setParameter("mode", 0.0f);
    sw.setParameter("slew", 0.0f);
    sw.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock a(kSr, 1, kB), clk(kSr, 1, kB);
    Pulse cp = lowFirst(3000, 200);
    // por bloco: qual das 4 saídas tem o sinal (|.|>0.4)
    std::vector<int> activeSeq;
    int prevActive = -2;
    for (int blk = 0; blk < 400; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) { a.at(0, k) = 0.8f; clk.at(0, k) = cp.next(); }
        std::vector<const AudioBlock*> ins{&a, nullptr, nullptr, nullptr,
                                           &clk, nullptr, nullptr};
        sw.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v[4] = {out[0].at(0, k), out[2].at(0, k),
                                out[3].at(0, k), out[4].at(0, k)};
            int hot = -1, hotN = 0;
            for (int j = 0; j < 4; ++j) {
                check(std::isfinite(v[j]), "demux finito");
                if (std::fabs(v[j]) > 0.4f) { hot = j; ++hotN; }
            }
            check(hotN <= 1, "demux: no máximo 1 saída ativa por vez");
            if (hot >= 0 && hot != prevActive) { activeSeq.push_back(hot); prevActive = hot; }
        }
    }
    // deve ter passado por todas as 4 saídas, em ordem forward
    bool sawAll[4] = {false, false, false, false};
    for (int x : activeSeq) sawAll[x] = true;
    check(sawAll[0] && sawAll[1] && sawAll[2] && sawAll[3],
          "demux forward: as 4 saídas recebem o sinal ao longo do ciclo");
}

// sequência de posições distintas na ordem em que aparecem
std::vector<int> visitOrder(const Rec& r) {
    std::vector<int> seq;
    for (int p : r.pos)
        if (seq.empty() || seq.back() != p) seq.push_back(p);
    return seq;
}

void testForwardCycles() {
    Switch sw;
    In4 in; in.a = 0.1f; in.b = 0.2f; in.c = 0.3f; in.d = 0.4f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk{4000, 200};
    const Rec r = run(sw, 400, in, 4, &clk);
    const auto seq = visitOrder(r);
    // começa em 0, cicla 0,1,2,3,0,1,...
    check(seq.size() >= 6, "várias trocas");
    for (std::size_t i = 0; i + 1 < seq.size(); ++i)
        check(seq[i + 1] == (seq[i] + 1) % 4, "forward: +1 mod 4");
    std::set<int> uniq(seq.begin(), seq.end());
    check(uniq.size() == 4, "forward visita as 4 posições");
}

void testPingpong() {
    Switch sw;
    sw.setParameter("mode", 1.0f);
    In4 in; in.a = 0.1f; in.b = 0.2f; in.c = 0.3f; in.d = 0.4f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk = lowFirst(4000, 200);
    const Rec r = run(sw, 500, in, 4, &clk);
    const auto seq = visitOrder(r);
    check(seq.size() >= 8, "várias trocas");
    // padrão 0,1,2,3,2,1,0,1,2,3,...
    const int expect[] = {0, 1, 2, 3, 2, 1, 0, 1};
    for (std::size_t i = 0; i < 8 && i < seq.size(); ++i)
        check(seq[i] == expect[i], "pingpong bate nas pontas e volta");
}

void testRandom() {
    Switch sw;
    sw.setParameter("mode", 2.0f);
    In4 in; in.a = 0.1f; in.b = 0.2f; in.c = 0.3f; in.d = 0.4f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk{3000, 150};
    const Rec r = run(sw, 600, in, 4, &clk);
    const auto seq = visitOrder(r);
    check(seq.size() >= 10, "várias trocas");
    for (std::size_t i = 0; i + 1 < seq.size(); ++i)
        check(seq[i] != seq[i + 1], "random: nunca repete a posição seguidas");
    std::set<int> uniq(seq.begin(), seq.end());
    check(uniq.size() == 4, "random cobre todas as posições");
}

void testAddrSelects() {
    Switch sw;
    In4 in; in.a = 0.1f; in.b = 0.2f; in.c = 0.3f; in.d = 0.4f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk{500, 50};      // clock rápido — deve ser ignorado
    // addr fixo apontando pra posição 2 (0.666 * 3 = 2)
    float adr = 0.666f;
    const Rec r = run(sw, 200, in, 4, &clk, nullptr, &adr);
    // ignora o clock e fica travado em pos 2 -> out ~ 0.3
    for (int p : std::vector<int>(r.pos.end() - 100, r.pos.end()))
        check(p == 2, "addr conectada trava a posição (clock ignorado)");
    near(r.out.back(), 0.3, 0.02);
}

void testReset() {
    Switch sw;
    In4 in; in.a = 0.1f; in.b = 0.2f; in.c = 0.3f; in.d = 0.4f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk{3000, 150};
    Pulse rst{20000, 200};   // reset esparso
    const Rec r = run(sw, 500, in, 4, &clk, &rst);
    // na janela logo após o pulso de reset (~amostra 20000) a posição é 0
    bool sawResetToZero = false;
    for (std::size_t i = 20000; i < r.pos.size() && i < 20500; ++i)
        if (r.pos[i] == 0) sawResetToZero = true;
    check(sawResetToZero, "reset volta a posição a 0");
}

void testGlideCrossfade() {
    // glide 0: a troca é abrupta (o slew de 1 ms ainda arredonda o degrau,
    // mas a derivada por amostra é grande). glide alto: crossfade -> derivada
    // pequena na troca.
    In4 in; in.a = -1.0f; in.d = 1.0f; in.on[0] = true; in.on[3] = true;
    Switch hard, soft;
    hard.setParameter("glide", 0.0f);
    hard.setParameter("slew", 0.0f);
    soft.setParameter("glide", 1.0f);
    soft.setParameter("slew", 0.0f);
    // steps=4 e addr pulando 0 <-> 1 -> alterna pos 0 (a=-1) e pos 3 (d=+1)
    Pulse a1{6000, 3000}, a2{6000, 3000};
    const Rec rh = run(hard, 400, in, 4, nullptr, nullptr, nullptr, &a1);
    const Rec rs = run(soft, 400, in, 4, nullptr, nullptr, nullptr, &a2);
    // ignora o transitório inicial (out_ parte de 0 e assenta na 1ª entrada)
    float maxDh = 0, maxDs = 0;
    for (std::size_t i = 4000; i < rh.out.size(); ++i)
        maxDh = std::max(maxDh, std::fabs(rh.out[i] - rh.out[i - 1]));
    for (std::size_t i = 4000; i < rs.out.size(); ++i)
        maxDs = std::max(maxDs, std::fabs(rs.out[i] - rs.out[i - 1]));
    check(maxDh > maxDs * 3.0f, "glide alto suaviza a troca (derivada menor)");
}

void testNoBleed() {
    // pos travada em 0 (addr=0): só `a` deve aparecer, b/c/d não vazam
    Switch sw;
    In4 in; in.a = 0.5f; in.b = 1.0f; in.c = -1.0f; in.d = 0.9f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    float adr = 0.0f;
    const Rec r = run(sw, 100, in, 4, nullptr, nullptr, &adr);
    near(r.out.back(), 0.5, 0.02);
    for (int p : r.pos) check(p == 0, "sem clock/addr>0 fica em 0");
}

void testStepOutput() {
    Switch sw;
    In4 in; for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse clk{4000, 200};
    const Rec r = run(sw, 400, in, 4, &clk);
    float lo = 1e9f, hi = -1e9f;
    for (float s : r.step) { lo = std::min(lo, s); hi = std::max(hi, s); }
    near(lo, 0.0);
    near(hi, 1.0);
}

void testDeterminism() {
    Switch a, b;
    for (Switch* s : {&a, &b}) {
        s->setParameter("mode", 2.0f);
        s->setParameter("glide", 0.4f);
        s->setParameter("slew", 0.3f);
    }
    In4 in; in.a = 0.1f; in.b = 0.7f; in.c = -0.3f; in.d = 0.5f;
    for (int i = 0; i < 4; ++i) in.on[i] = true;
    Pulse ca{2500, 120}, cb{2500, 120};
    const Rec ra = run(a, 300, in, 4, &ca);
    const Rec rb = run(b, 300, in, 4, &cb);
    bool same = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; same && i < ra.out.size(); ++i)
        if (ra.out[i] != rb.out[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 140.0f);
    const auto o1 = g.add(std::make_unique<Oscillator>());
    const auto o2 = g.add(std::make_unique<Oscillator>());
    g.node(o1).setParameter("freq", 220.0f);
    g.node(o2).setParameter("freq", 330.0f);
    const auto sw = g.add(std::make_unique<Switch>());
    const auto flt = g.add(std::make_unique<Filter>());
    g.connect(clk, 0, sw, 4);      // clock -> SWITCH.clock
    g.connect(o1, 0, sw, 0);       // OSC1 -> a
    g.connect(o2, 0, sw, 1);       // OSC2 -> b
    g.connect(sw, 0, flt, 0);      // SWITCH.out -> FILTER.in
    g.node(sw).setParameter("steps", 2.0f);
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 3000; ++b) {
        g.process(out, flt, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.05f && lo < -0.05f, "SWITCH roteia o áudio no grafo");
}

void testPanel() {
    Switch sw;
    const std::string problem = validatePanel(sw);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(sw.panel().widgets.size() >= 15);
    std::cout << renderAscii(sw);
}

}  // namespace

int main() {
    testForwardCycles();
    testPingpong();
    testRandom();
    testAddrSelects();
    testReset();
    testGlideCrossfade();
    testDemux();
    testNoBleed();
    testStepOutput();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_switch: OK\n";
    return g_failures == 0 ? 0 : 1;
}
