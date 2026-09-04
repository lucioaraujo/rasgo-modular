// Teste isolado do Módulo 31 (ABACUS — aritmética binária de CV).
// Critérios do dossiê `dossies/31_abacus.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Abacus.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Oscillator.hpp"
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
void near(double a, double b, double t = 1e-4) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

struct Pulse {
    std::size_t period, hi, t = 0;
    float next() { const float v = (t % period) < hi ? 1.0f : 0.0f; ++t; return v; }
};

struct Rec {
    std::vector<float> math, quant, rect, p1, p2, carry;
    int carryEdges = 0, p1Edges = 0;
};

// roda com a/b como funções opcionais de t (amostra global)
Rec run(Abacus& ab, int blocks,
        std::function<float(std::size_t)> fa = nullptr,
        std::function<float(std::size_t)> fb = nullptr,
        Pulse* clk = nullptr, Pulse* rst = nullptr) {
    ab.prepare(kSr, kB);
    std::vector<AudioBlock> out(6, AudioBlock(kSr, 1, kB));
    AudioBlock ba(kSr, 1, kB), bb(kSr, 1, kB), bc(kSr, 1, kB), br(kSr, 1, kB);
    Rec r;
    float pc = 0.0f, pp1 = 0.0f;
    std::size_t g = 0;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) {
            ba.at(0, k) = fa ? fa(g + k) : 0.0f;
            bb.at(0, k) = fb ? fb(g + k) : 0.0f;
            bc.at(0, k) = clk ? clk->next() : 0.0f;
            br.at(0, k) = rst ? rst->next() : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            fa ? &ba : nullptr, fb ? &bb : nullptr,
            clk ? &bc : nullptr, rst ? &br : nullptr};
        ab.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float mm = out[0].at(0, k), qq = out[1].at(0, k),
                        rr = out[2].at(0, k), a1 = out[3].at(0, k),
                        a2 = out[4].at(0, k), cy = out[5].at(0, k);
            check(std::isfinite(mm) && std::isfinite(qq) && std::isfinite(rr)
                  && std::isfinite(a1) && std::isfinite(a2) && std::isfinite(cy),
                  "finito");
            r.math.push_back(mm); r.quant.push_back(qq); r.rect.push_back(rr);
            r.p1.push_back(a1); r.p2.push_back(a2); r.carry.push_back(cy);
            if (cy >= 0.5f && pc < 0.5f) ++r.carryEdges;
            if (a1 >= 0.5f && pp1 < 0.5f) ++r.p1Edges;
            pc = cy; pp1 = a1;
        }
        g += kB;
    }
    return r;
}

void testAddExact() {
    Abacus ab;
    ab.setParameter("op", 0.0f);
    auto ramp = [](std::size_t t) { return -1.0f + 2.0f * (t % 4000) / 4000.0f; };
    auto dc = [](std::size_t) { return 0.2f; };
    const Rec r = run(ab, 40, ramp, dc);
    for (std::size_t i = 0; i < r.math.size(); i += 37)
        near(r.math[i], ramp(i) + 0.2, 1e-4);
}

void testSubtractMultiply() {
    Abacus s, m;
    s.setParameter("op", 1.0f);
    m.setParameter("op", 2.0f);
    auto a = [](std::size_t t) { return 0.5f * std::sin(t * 0.01f); };
    auto b = [](std::size_t) { return 0.3f; };
    const Rec rs = run(s, 20, a, b);
    const Rec rm = run(m, 20, a, b);
    for (std::size_t i = 0; i < rs.math.size(); i += 29) {
        near(rs.math[i], a(i) - 0.3, 1e-4);
        near(rm.math[i], a(i) * 0.3, 1e-4);
    }
}

void testBitwise() {
    // op 4-7: `a`/`b` viram inteiros de 5 bits (±range → 0..31), a lógica
    // roda nos bits, o resultado volta a ±range.
    auto val = [](long ir) { return ir / 31.0 * 2.0 - 1.0; };
    struct Case { float op, a, b; long expect; };
    const Case cs[] = {
        {4.0f, 1.0f, 0.0f, 31 & 16},   // AND  (31, 16)
        {5.0f, -1.0f, 0.0f, 0 | 16},   // OR   (0, 16)
        {6.0f, 1.0f, 1.0f, 31 ^ 31},   // XOR  -> 0
        {7.0f, -1.0f, -1.0f, (~(0 & 0)) & 31},  // NAND -> 31
        {7.0f, 1.0f, 1.0f, (~(31 & 31)) & 31},  // NAND -> 0
    };
    for (const Case& c : cs) {
        Abacus ab;
        ab.setParameter("op", c.op);
        ab.setParameter("range", 1.0f);
        const Rec r = run(ab, 12, [&](std::size_t) { return c.a; },
                          [&](std::size_t) { return c.b; });
        near(r.math.back(), val(c.expect), 2e-3);
    }
    // op 4 difere de op 0 (add) no mesmo sinal
    Abacus land, ladd;
    land.setParameter("op", 4.0f);
    ladd.setParameter("op", 0.0f);
    auto a = [](std::size_t) { return 0.7f; };
    auto b = [](std::size_t) { return 0.4f; };
    check(std::fabs(run(land, 12, a, b).math.back()
                    - run(ladd, 12, a, b).math.back()) > 0.05f,
          "op bit a bit != aritmética");
}

void testModulo() {
    Abacus ab;
    ab.setParameter("op", 3.0f);
    ab.setParameter("range", 1.0f);
    auto a = [](std::size_t t) { return 3.0f * (t % 6000) / 6000.0f; };  // 0..3
    const Rec r = run(ab, 60, a);
    for (std::size_t i = 100; i < r.math.size(); i += 31) {
        check(r.math[i] >= 0.0f && r.math[i] < 1.0001f, "resto em [0,1)");
        near(r.math[i], a(i) - std::floor(a(i)), 1e-3);
    }
}

void testQuantizeLevels() {
    Abacus ab;
    ab.setParameter("steps", 4.0f);
    ab.setParameter("range", 2.0f);
    auto a = [](std::size_t t) { return -2.0f + 4.0f * (t % 8000) / 8000.0f; };
    const Rec r = run(ab, 80, a);
    std::set<int> levels;
    int plateau = 0, maxPlateau = 0;
    for (std::size_t i = 1; i < r.quant.size(); ++i) {
        levels.insert(static_cast<int>(std::lround(r.quant[i] * 100.0f)));
        if (std::fabs(r.quant[i] - r.quant[i - 1]) < 1e-6f) ++plateau;
        else { maxPlateau = std::max(maxPlateau, plateau); plateau = 0; }
    }
    check(levels.size() <= 9, "quant: só ~9 níveis (steps=4, range=2)");
    check(maxPlateau > 200, "quant: patamares constantes entre trocas");
}

void testRectModes() {
    auto a = [](std::size_t t) { return std::sin(t * 0.02f); };
    for (int mode = 0; mode < 4; ++mode) {
        Abacus ab;
        ab.setParameter("rect_mode", static_cast<float>(mode));
        ab.setParameter("range", 1.0f);
        const Rec r = run(ab, 20, a);
        for (std::size_t i = 0; i < r.rect.size(); i += 23) {
            const float x = a(i);
            float want;
            if (mode == 0) want = x > 0 ? x : 0.0f;
            else if (mode == 1) want = x < 0 ? x : 0.0f;
            else if (mode == 2) want = std::fabs(x);
            else want = x > 1e-4f ? 1.0f : (x < -1e-4f ? -1.0f : 0.0f);
            near(r.rect[i], want, 1e-4);
        }
    }
}

void testCarryPeriod() {
    Abacus ab;
    ab.setParameter("count_step", 1.0f);
    ab.setParameter("modulus", 8.0f);
    Pulse clk{2000, 60};
    const Rec r = run(ab, 300, nullptr, nullptr, &clk);
    // 300*128/48000 = 0.8 s / (2000/48000 = 41.7 ms) ~ 19 tiques
    // carry a cada 8 -> ~2 (mas com o "started" o 1º tique não conta)
    // usar mais tiques:
    Abacus a2;
    a2.setParameter("modulus", 8.0f);
    Pulse c2{600, 40};
    const Rec r2 = run(a2, 400, nullptr, nullptr, &c2);
    // 400*128/48000 = 1.067 s / 12.5 ms ~ 85 tiques -> ~10 carries
    check(r2.carryEdges >= 8 && r2.carryEdges <= 12,
          "carry ~1x a cada 8 tiques (count_step=1)");
    (void)r;
}

void testCarryStep2() {
    Abacus a2, an;
    a2.setParameter("modulus", 8.0f);
    a2.setParameter("count_step", 2.0f);
    an.setParameter("modulus", 8.0f);
    an.setParameter("count_step", -1.0f);
    Pulse c2{600, 40}, cn{600, 40};
    const Rec r2 = run(a2, 400, nullptr, nullptr, &c2);
    const Rec rn = run(an, 400, nullptr, nullptr, &cn);
    // ~85 tiques: step=2 -> carry a cada 4 -> ~20 ; step=-1 -> a cada 8 -> ~10
    check(r2.carryEdges > 15 && r2.carryEdges < 25, "count_step=2 -> carry 2x mais");
    check(rn.carryEdges >= 8 && rn.carryEdges <= 12,
          "count_step=-1 -> contador anda pra trás, carry ainda 1x a cada 8");
}

void testP1Divides() {
    Abacus ab;
    ab.setParameter("pattern", 0.0f);   // bitA = 0 -> p1 = c & 1
    ab.setParameter("count_step", 1.0f);
    Pulse clk{800, 40};
    const Rec r = run(ab, 400, nullptr, nullptr, &clk);
    // ~64 tiques -> p1 alterna a cada tique -> ~32 bordas de subida
    check(r.p1Edges >= 26 && r.p1Edges <= 38, "p1 (bitA=0) divide o clock por 2");
}

void testAutonomousSource() {
    // sem `a` conectado -> quant sai da rampa do contador -> varia
    Abacus ab;
    ab.setParameter("steps", 6.0f);
    ab.setParameter("modulus", 12.0f);
    Pulse clk{700, 40};
    const Rec r = run(ab, 400, nullptr, nullptr, &clk);
    float lo = 1e9f, hi = -1e9f;
    for (float v : r.quant) { lo = std::min(lo, v); hi = std::max(hi, v); }
    check(hi - lo > 0.3f, "sem `a`: quant vem do contador (varia)");
    std::set<int> levels;
    for (float v : r.quant) levels.insert(static_cast<int>(std::lround(v * 100)));
    check(levels.size() >= 3, "sem `a`: quant assume vários degraus");
}

void testInternalClock() {
    Abacus ab;
    ab.setParameter("rate", 20.0f);
    ab.setParameter("modulus", 4.0f);
    const Rec r = run(ab, 400, nullptr, nullptr, nullptr);  // sem clock
    // 1.067 s * 20 Hz = ~21 tiques / modulus 4 -> ~5 carries
    check(r.carryEdges >= 3 && r.carryEdges <= 8,
          "relógio interno faz o contador andar");
}

void testReset() {
    Abacus ab;
    ab.setParameter("modulus", 8.0f);
    Pulse clk{600, 40};
    Pulse rst{600 * 6, 40};   // reset a cada 6 tiques
    const Rec r = run(ab, 400, nullptr, nullptr, &clk, &rst);
    Abacus ref;
    ref.setParameter("modulus", 8.0f);
    Pulse c2{600, 40};
    const Rec r0 = run(ref, 400, nullptr, nullptr, &c2);
    // reset frequente antes de 8 -> o contador nunca chega a estourar
    check(r.carryEdges < r0.carryEdges, "reset zera o contador (menos carries)");
}

void testSlew() {
    Abacus fast, slow;
    fast.setParameter("slew", 0.0f);
    slow.setParameter("slew", 0.6f);
    fast.setParameter("steps", 4.0f);
    slow.setParameter("steps", 4.0f);
    // degrau brusco em `a`
    auto a = [](std::size_t t) { return t < 3000 ? -1.5f : 1.5f; };
    const Rec rf = run(fast, 60, a);
    const Rec rs = run(slow, 60, a);
    float maxDf = 0, maxDs = 0;
    for (std::size_t i = 1; i < rf.quant.size(); ++i)
        maxDf = std::max(maxDf, std::fabs(rf.quant[i] - rf.quant[i - 1]));
    for (std::size_t i = 1; i < rs.quant.size(); ++i)
        maxDs = std::max(maxDs, std::fabs(rs.quant[i] - rs.quant[i - 1]));
    check(maxDf > 0.4f, "slew=0 -> quant salta");
    check(maxDs < 0.02f, "slew alto -> quant desliza (derivada pequena)");
}

void testDeterminism() {
    Abacus a, b;
    for (Abacus* x : {&a, &b}) {
        x->setParameter("op", 2.0f);
        x->setParameter("modulus", 7.0f);
        x->setParameter("count_step", 2.0f);
        x->setParameter("slew", 0.3f);
        x->setParameter("pattern", 0.6f);
    }
    auto fa = [](std::size_t t) { return std::sin(t * 0.007f); };
    Pulse ca{511, 30}, cb{511, 30};
    const Rec ra = run(a, 200, fa, nullptr, &ca);
    const Rec rb = run(b, 200, fa, nullptr, &cb);
    bool same = ra.math.size() == rb.math.size();
    for (std::size_t i = 0; same && i < ra.math.size(); ++i)
        if (ra.math[i] != rb.math[i] || ra.quant[i] != rb.quant[i]
            || ra.carry[i] != rb.carry[i] || ra.p1[i] != rb.p1[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 150.0f);
    const auto ab = g.add(std::make_unique<Abacus>());
    g.node(ab).setParameter("steps", 5.0f);
    g.node(ab).setParameter("modulus", 6.0f);
    const auto env = g.add(std::make_unique<Envelope>());
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 110.0f);
    g.connect(clk, 0, ab, 2);      // clock -> ABACUS.clock
    g.connect(ab, 1, osc, 0);      // quant -> OSC.pitch
    g.connect(osc, 2, env, 0);     // saw -> ENVELOPE.in
    g.connect(ab, 5, env, 1);      // carry -> ENVELOPE.gate
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 4000; ++b) {
        g.process(out, env, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.05f && lo < -0.05f, "ABACUS aciona a voz no grafo");
}

void testPanel() {
    Abacus ab;
    const std::string problem = validatePanel(ab);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(ab.panel().widgets.size() >= 18);
    std::cout << renderAscii(ab);
}

}  // namespace

int main() {
    testAddExact();
    testSubtractMultiply();
    testBitwise();
    testModulo();
    testQuantizeLevels();
    testRectModes();
    testCarryPeriod();
    testCarryStep2();
    testP1Divides();
    testAutonomousSource();
    testInternalClock();
    testReset();
    testSlew();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_abacus: OK\n";
    return g_failures == 0 ? 0 : 1;
}
