// Teste isolado do Módulo 22 (LOGIC) - antes de entrar num patch.
// Critérios do dossiê `dossies/22_logic.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/Logic.hpp"
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

// gera um trem de pulsos: alto por `hi` amostras a cada `period` amostras.
struct Clock {
    std::size_t period, hi, t = 0;
    float next() {
        const float v = (t % period) < hi ? 1.0f : 0.0f;
        ++t;
        return v;
    }
};

// roda `blocks` blocos; devolve nº de bordas ↑ e o duty médio de uma saída.
struct Run {
    int edges = 0;
    double dutySum = 0.0;
    long dutyN = 0;
    std::vector<float> trace;   // saída[out] amostra a amostra (opcional)
};

Run runLogic(Logic& L, int outIdx, int blocks,
             Clock* clk = nullptr, Clock* a = nullptr, Clock* b = nullptr,
             bool keepTrace = false) {
    L.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock ic(kSr, 1, kB), ia(kSr, 1, kB), ib(kSr, 1, kB), ir(kSr, 1, kB);
    Run r;
    float prev = 0.0f;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) {
            ic.at(0, k) = clk ? clk->next() : 0.0f;
            ia.at(0, k) = a ? a->next() : 0.0f;
            ib.at(0, k) = b ? b->next() : 0.0f;
            ir.at(0, k) = 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            clk ? &ic : nullptr, a ? &ia : nullptr, b ? &ib : nullptr, &ir};
        L.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float v = out[static_cast<std::size_t>(outIdx)].at(0, k);
            check(std::isfinite(v) && (v == 0.0f || v == 1.0f), "saída em {0,1}");
            if (v > 0.5f && prev <= 0.5f) ++r.edges;
            r.dutySum += v; ++r.dutyN;
            if (keepTrace) r.trace.push_back(v);
            prev = v;
        }
    }
    return r;
}

void testDivideHalf() {
    Logic L;
    L.setParameter("divide", 2.0f);
    L.setParameter("multiply", 1.0f);
    Clock c{2400, 200};                 // 20 Hz, ~48 bordas em 100 blocos
    Clock ref{2400, 200};
    const int clkEdges = runLogic(L, 1, 100, nullptr, &ref).edges * 0;  // ignora
    (void)clkEdges;
    Logic L2; L2.setParameter("divide", 2.0f);
    Clock c2{2400, 200};
    const int divEdges = runLogic(L2, 0, 100, &c2).edges;
    // 100 blocos * 128 = 12800 amostras / 2400 = ~5.3 bordas de clock -> ÷2 ~ 2-3
    Logic L1; L1.setParameter("divide", 1.0f);
    Clock c1{2400, 200};
    const int passEdges = runLogic(L1, 0, 100, &c1).edges;
    check(divEdges >= 1 && divEdges <= passEdges - 1,
          "÷2 dá ~metade dos pulsos do ÷1");
    (void)c;
}

void testDividePassthrough() {
    Logic L;
    L.setParameter("divide", 1.0f);
    L.setParameter("multiply", 1.0f);
    L.setParameter("gate_len", 0.5f);
    Clock c{4800, 400};                 // 10 Hz
    const int e = runLogic(L, 0, 200, &c).edges;
    // 200*128 = 25600 / 4800 = ~5.3 -> 5 pulsos
    check(e >= 4 && e <= 6, "÷1 segue o clock (~5 pulsos)");
}

void testMultiplyDoubles() {
    Logic a, b;
    a.setParameter("multiply", 1.0f);
    b.setParameter("multiply", 2.0f);
    a.setParameter("divide", 1.0f);
    b.setParameter("divide", 1.0f);
    Clock ca{4800, 300}, cb{4800, 300};
    const int e1 = runLogic(a, 0, 300, &ca).edges;
    const int e2 = runLogic(b, 0, 300, &cb).edges;
    check(e2 >= e1 * 2 - 2 && e2 <= e1 * 2 + 2, "×2 dobra a contagem de pulsos");
}

void testGateLen() {
    Logic shortG, longG;
    shortG.setParameter("gate_len", 0.1f);
    longG.setParameter("gate_len", 0.9f);
    shortG.setParameter("divide", 1.0f);
    longG.setParameter("divide", 1.0f);
    Clock cs{4800, 100}, cl{4800, 100};
    const Run rs = runLogic(shortG, 0, 300, &cs);
    const Run rl = runLogic(longG, 0, 300, &cl);
    const double dutyS = rs.dutySum / rs.dutyN;
    const double dutyL = rl.dutySum / rl.dutyN;
    check(dutyL > dutyS * 3.0, "gate_len maior -> duty maior");
    check(dutyS < 0.25 && dutyL > 0.6, "duty medido perto de gate_len");
}

void testDelay() {
    // com delay alto o primeiro pulso do `div` sai mais tarde
    Logic noD, withD;
    noD.setParameter("divide", 1.0f);
    withD.setParameter("divide", 1.0f);
    withD.setParameter("delay", 0.5f);      // ~100 ms = 4800 amostras
    Clock c0{9600, 200}, c1{9600, 200};
    const Run r0 = runLogic(noD, 0, 200, &c0, nullptr, nullptr, true);
    const Run r1 = runLogic(withD, 0, 200, &c1, nullptr, nullptr, true);
    auto firstHigh = [](const std::vector<float>& t) {
        for (std::size_t i = 0; i < t.size(); ++i) if (t[i] > 0.5f) return (long)i;
        return -1L;
    };
    const long f0 = firstHigh(r0.trace);
    const long f1 = firstHigh(r1.trace);
    check(f0 >= 0 && f1 >= 0, "os dois têm pulso");
    check(f1 - f0 > 3000 && f1 - f0 < 6500, "delay ~0,5 -> atraso ~4800 amostras");
}

void testBooleanTruthTable() {
    // a e b como níveis DC; verifica and/or/xor
    auto combo = [](float av, float bv, int outIdx) {
        Logic L;
        L.prepare(kSr, kB);
        std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
        AudioBlock ic(kSr, 1, kB), ia(kSr, 1, kB), ib(kSr, 1, kB), ir(kSr, 1, kB);
        for (std::size_t k = 0; k < kB; ++k) { ia.at(0, k) = av; ib.at(0, k) = bv; }
        std::vector<const AudioBlock*> ins{nullptr, &ia, &ib, &ir};
        for (int i = 0; i < 4; ++i) L.process(ins, out);
        return out[static_cast<std::size_t>(outIdx)].at(0, 0);
    };
    // AND (idx 1)
    check(combo(1, 1, 1) == 1.0f && combo(1, 0, 1) == 0.0f
          && combo(0, 1, 1) == 0.0f && combo(0, 0, 1) == 0.0f, "AND");
    // OR (idx 2)
    check(combo(1, 1, 2) == 1.0f && combo(1, 0, 2) == 1.0f
          && combo(0, 1, 2) == 1.0f && combo(0, 0, 2) == 0.0f, "OR");
    // XOR (idx 3)
    check(combo(1, 1, 3) == 0.0f && combo(1, 0, 3) == 1.0f
          && combo(0, 1, 3) == 1.0f && combo(0, 0, 3) == 0.0f, "XOR");
}

void testFlipFlop() {
    Logic L;
    Clock a{2000, 150};                  // gera bordas em `a`
    const Run r = runLogic(L, 4, 200, nullptr, &a);
    // conta bordas de `a`: 200*128 = 25600 / 2000 = ~12 bordas
    // flip alterna a cada borda ↑ -> nº de bordas ↑ do flip ≈ nº bordas de a / 2
    check(r.edges >= 4 && r.edges <= 8, "flip alterna (~metade das bordas de a)");
}

void testFlipOnlyOnRisingEdge() {
    // `a` alto o tempo todo -> uma borda só -> flip vira 1 e fica
    Logic L;
    L.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock ia(kSr, 1, kB), ir(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) ia.at(0, k) = 1.0f;
    std::vector<const AudioBlock*> ins{nullptr, &ia, nullptr, &ir};
    for (int i = 0; i < 20; ++i) L.process(ins, out);
    check(out[4].at(0, 0) == 1.0f, "uma borda -> flip=1 e estável (não oscila no nível)");
}

void testReset() {
    Logic L;
    L.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock ia(kSr, 1, kB), ir(kSr, 1, kB);
    Clock a{1500, 100};
    // avança um pouco pra flip virar 1
    for (int blk = 0; blk < 40; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) { ia.at(0, k) = a.next(); ir.at(0, k) = 0.0f; }
        std::vector<const AudioBlock*> ins{nullptr, &ia, nullptr, &ir};
        L.process(ins, out);
    }
    // agora segura reset alto
    for (std::size_t k = 0; k < kB; ++k) { ia.at(0, k) = 1.0f; ir.at(0, k) = 1.0f; }
    std::vector<const AudioBlock*> ins{nullptr, &ia, nullptr, &ir};
    L.process(ins, out);
    check(out[4].at(0, kB - 1) == 0.0f, "reset zera o flip-flop");
}

void testInternalClock() {
    // sem `clock` conectado, `rate` gera `div` sozinho (modo autônomo)
    Logic L;
    L.setParameter("rate", 8.0f);
    L.setParameter("divide", 1.0f);
    const int e = runLogic(L, 0, 400, nullptr).edges;
    // 400*128 = 51200 / 48000 = ~1.07 s * 8 Hz = ~8-9 pulsos
    check(e >= 6 && e <= 11, "relógio interno produz `div` sem entrada");
}

void testDeterminism() {
    Logic a, b;
    for (Logic* L : {&a, &b}) {
        L->setParameter("rate", 5.0f);
        L->setParameter("divide", 3.0f);
        L->setParameter("multiply", 2.0f);
        L->setParameter("delay", 0.3f);
    }
    a.prepare(kSr, kB); b.prepare(kSr, kB);
    std::vector<AudioBlock> oa(5, AudioBlock(kSr, 1, kB)), ob(5, AudioBlock(kSr, 1, kB));
    AudioBlock z(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr, &z};
    bool same = true;
    for (int blk = 0; blk < 300; ++blk) {
        a.process(ins, oa); b.process(ins, ob);
        for (int o = 0; o < 5; ++o)
            for (std::size_t k = 0; k < kB; ++k)
                if (oa[static_cast<std::size_t>(o)].at(0, k)
                    != ob[static_cast<std::size_t>(o)].at(0, k)) same = false;
    }
    EXPECT(same);
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 120.0f);
    const auto logic = g.add(std::make_unique<Logic>());
    g.node(logic).setParameter("divide", 2.0f);
    const auto env = g.add(std::make_unique<Envelope>());
    g.node(env).setParameter("mode", 1.0f);
    g.connect(clk, 0, logic, 0);          // clock -> LOGIC.clock
    g.connect(logic, 0, env, 1);          // div -> ENVELOPE.gate
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float hi = 0.0f;
    for (int blk = 0; blk < 4000; ++blk) {
        g.process(out, env, 1);           // saída `env` (crua), `out` precisa de `in`
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            hi = std::max(hi, std::fabs(out.at(0, k)));
        }
    }
    check(hi > 0.05f, "LOGIC.div dispara o envelope no grafo");
}

void testPanel() {
    Logic L;
    const std::string problem = validatePanel(L);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(L.panel().widgets.size() >= 14);
    std::cout << renderAscii(L);
}

}  // namespace

int main() {
    testDivideHalf();
    testDividePassthrough();
    testMultiplyDoubles();
    testGateLen();
    testDelay();
    testBooleanTruthTable();
    testFlipFlop();
    testFlipOnlyOnRisingEdge();
    testReset();
    testInternalClock();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_logic: OK\n";
    return g_failures == 0 ? 0 : 1;
}
