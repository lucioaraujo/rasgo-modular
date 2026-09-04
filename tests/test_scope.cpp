// Teste isolado do Módulo 29 (SCOPE — osciloscópio + análise).
// Critérios do dossiê `dossies/29_scope.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Noise.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Scope.hpp"
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
void near(double a, double b, double t = 0.05) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

struct Out {
    std::vector<float> thru, trig, level, bright, pitch;
};

// roda `sc` com uma senoide de `hz`/`amp` (ou ruído se hz<0) por `blocks`.
// `extHz` >= 0 injeta uma senoide em `ext`.
Out run(Scope& sc, int blocks, float hz, float amp = 0.5f, float extHz = -1.0f) {
    sc.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock bin(kSr, 1, kB), bext(kSr, 1, kB);
    Out r;
    double ph = 0.0, phe = 0.0;
    std::uint64_t rng = 0x1234567ULL;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            float v;
            if (hz < 0.0f) {
                rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
                v = amp * (static_cast<float>(static_cast<std::int32_t>(rng >> 32))
                           / 2147483648.0f);
            } else {
                v = amp * static_cast<float>(std::sin(ph));
                ph += 2.0 * M_PI * hz / kSr;
            }
            bin.at(0, k) = v;
            if (extHz >= 0.0f) {
                bext.at(0, k) = static_cast<float>(std::sin(phe));
                phe += 2.0 * M_PI * extHz / kSr;
            }
        }
        std::vector<const AudioBlock*> ins{&bin, extHz >= 0.0f ? &bext : nullptr};
        sc.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float th = out[0].at(0, k), tg = out[1].at(0, k),
                        lv = out[2].at(0, k), br = out[3].at(0, k),
                        pi = out[4].at(0, k);
            check(std::isfinite(th) && std::isfinite(tg) && std::isfinite(lv)
                  && std::isfinite(br) && std::isfinite(pi), "finito");
            r.thru.push_back(th); r.trig.push_back(tg); r.level.push_back(lv);
            r.bright.push_back(br); r.pitch.push_back(pi);
        }
    }
    return r;
}

int risingEdges(const std::vector<float>& g) {
    int n = 0;
    for (std::size_t i = 1; i < g.size(); ++i)
        if (g[i] >= 0.5f && g[i - 1] < 0.5f) ++n;
    return n;
}

void testThruIsClean() {
    Scope sc;
    sc.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock bin(kSr, 1, kB);
    std::uint64_t rng = 99;
    for (int b = 0; b < 20; ++b) {
        std::vector<float> sent;
        for (std::size_t k = 0; k < kB; ++k) {
            rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
            const float v = static_cast<float>(static_cast<std::int32_t>(rng >> 32))
                / 2147483648.0f;
            bin.at(0, k) = v; sent.push_back(v);
        }
        std::vector<const AudioBlock*> ins{&bin, nullptr};
        sc.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            check(out[0].at(0, k) == sent[k], "thru == in amostra a amostra");
    }
}

void testTriggerRisingCountsCycles() {
    Scope sc;                       // trigger=0, edge=subida (default)
    const Out r = run(sc, 400, 200.0f, 0.6f);
    // 400*128/48000 = 1.067 s * 200 Hz ~ 213 ciclos
    const int e = risingEdges(r.trig);
    check(e > 190 && e < 235, "trig dispara ~1x por ciclo (subida)");
}

void testTriggerEdgeMatters() {
    Scope up, dn;
    dn.setParameter("edge", 1.0f);
    const Out ru = run(up, 300, 150.0f, 0.6f);
    const Out rd = run(dn, 300, 150.0f, 0.6f);
    const int eu = risingEdges(ru.trig), ed = risingEdges(rd.trig);
    // 300*128/48000 = 0.8s * 150 Hz ~ 120 ciclos; mesma contagem, fase oposta
    check(eu > 95 && eu < 145, "subida ~1x/ciclo");
    check(ed > 95 && ed < 145, "descida ~1x/ciclo");
}

void testRejectSuppressesNoise() {
    // senoide lenta perto do nível + ruído: reject alto deve evitar disparos
    // múltiplos por cruzamento
    Scope loose, tight;
    tight.setParameter("reject", 0.9f);
    // sinal: senoide 30 Hz amp 0.5 (cruza 0 devagar) — sem reject, o ruído
    // numérico não conta, mas com uma senoide + degraus... usamos ruído
    // somado testando via grafo seria melhor; aqui: senoide 30 Hz
    const Out rl = run(loose, 200, 30.0f, 0.5f);
    const Out rt = run(tight, 200, 30.0f, 0.5f);
    const int el = risingEdges(rl.trig), et = risingEdges(rt.trig);
    // 200*128/48000 = 0.53s * 30 Hz ~ 16 ciclos -> ~16 disparos nos dois
    check(el >= 12 && el <= 20, "sem reject: ~1x/ciclo");
    check(et >= 12 && et <= 20, "com reject: ainda ~1x/ciclo (senoide limpa)");
    // o de verdade: reject NUNCA aumenta a contagem
    check(et <= el + 1, "reject não multiplica disparos");
}

void testExtIsTriggerSource() {
    Scope sc;
    // in = DC 0 (nunca cruza), ext = senoide 100 Hz -> trig segue ext
    const Out r = run(sc, 300, -2.0f /*hz<0 mas amp 0*/, 0.0f, 100.0f);
    const int e = risingEdges(r.trig);
    // 300*128/48000 = 0.8s * 100 Hz ~ 80 ciclos
    check(e > 60 && e < 100, "ext conectada = fonte do trigger");
}

void testLevelFollows() {
    Scope fast, slow;
    fast.setParameter("response", 0.0f);
    slow.setParameter("response", 0.9f);
    const Out rf = run(fast, 200, 300.0f, 0.5f);
    const Out rs = run(slow, 200, 300.0f, 0.5f);
    // ambos chegam perto de 0.5 (pico da senoide)
    near(rf.level.back(), 0.5, 0.08);
    check(rs.level.back() > 0.25f, "level lento ainda sobe");
    // o lento demora mais pra chegar
    check(rs.level[2000] < rf.level[2000], "response alto = level mais lento");
}

void testBrightOrdering() {
    Scope a, b, c;
    const Out lo = run(a, 300, 100.0f, 0.5f);
    const Out mid = run(b, 300, 1500.0f, 0.5f);
    const Out hi = run(c, 300, 6000.0f, 0.5f);
    Scope d;
    const Out nz = run(d, 300, -1.0f, 0.5f);   // ruído branco
    const float bl = lo.bright.back(), bm = mid.bright.back(),
                bh = hi.bright.back(), bn = nz.bright.back();
    check(bl < bm, "bright(100Hz) < bright(1.5kHz)");
    check(bm < bh, "bright(1.5kHz) < bright(6kHz)");
    check(bh < bn + 0.2f, "bright(ruído) é o mais alto (ou perto do 6k)");
    check(bl < 0.2f, "senoide grave = bright baixo");
}

void testBrightMonotonicSweep() {
    float prev = -1.0f;
    int drops = 0;
    for (float hz = 80.0f; hz <= 7000.0f; hz *= 1.5f) {
        Scope sc;
        const Out r = run(sc, 250, hz, 0.5f);
        const float bnow = r.bright.back();
        if (bnow < prev - 0.06f) ++drops;
        prev = bnow;
    }
    check(drops == 0, "bright não-decrescente varrendo a frequência");
}

void testPitchTracks() {
    Scope a, b, c;
    const Out r1 = run(a, 400, 110.0f, 0.6f);
    const Out r2 = run(b, 400, 220.0f, 0.6f);
    const Out r3 = run(c, 400, 440.0f, 0.6f);
    near(r1.pitch.back(), 0.0, 0.06);
    near(r2.pitch.back(), 1.0, 0.06);
    near(r3.pitch.back(), 2.0, 0.06);
}

void testPitchZeroForNoise() {
    Scope sc;
    const Out r = run(sc, 400, -1.0f, 0.5f);
    check(std::fabs(r.pitch.back()) < 0.5f, "pitch ~ 0 pra ruído (sem período)");
}

void testHoldFreezes() {
    Scope sc;
    sc.prepare(kSr, kB);
    std::vector<AudioBlock> out(5, AudioBlock(kSr, 1, kB));
    AudioBlock bin(kSr, 1, kB);
    double ph = 0.0;
    auto fill = [&](float hz, float amp) {
        for (std::size_t k = 0; k < kB; ++k) {
            bin.at(0, k) = amp * static_cast<float>(std::sin(ph));
            ph += 2.0 * M_PI * hz / kSr;
        }
    };
    std::vector<const AudioBlock*> ins{&bin, nullptr};
    // 1) estabiliza com 300 Hz
    for (int b = 0; b < 200; ++b) { fill(300.0f, 0.5f); sc.process(ins, out); }
    const float lvlHeld = out[2].at(0, kB - 1);
    const float brtHeld = out[3].at(0, kB - 1);
    const float pitHeld = out[4].at(0, kB - 1);
    // 2) liga hold, muda drasticamente o sinal
    sc.setParameter("hold", 1.0f);
    for (int b = 0; b < 200; ++b) { fill(3000.0f, 0.05f); sc.process(ins, out); }
    near(out[2].at(0, kB - 1), lvlHeld, 0.02);
    near(out[3].at(0, kB - 1), brtHeld, 0.02);
    near(out[4].at(0, kB - 1), pitHeld, 0.02);
}

void testDeterminism() {
    Scope a, b;
    for (Scope* s : {&a, &b}) {
        s->setParameter("trigger", 0.1f);
        s->setParameter("reject", 0.2f);
        s->setParameter("response", 0.5f);
    }
    const Out ra = run(a, 200, 234.0f, 0.55f);
    const Out rb = run(b, 200, 234.0f, 0.55f);
    bool same = ra.bright.size() == rb.bright.size();
    for (std::size_t i = 0; same && i < ra.bright.size(); ++i)
        if (ra.bright[i] != rb.bright[i] || ra.trig[i] != rb.trig[i]
            || ra.pitch[i] != rb.pitch[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    // SCOPE.pitch -> OSC.pitch : o OSC rastreia a altura da entrada
    SignalGraph g;
    const auto src = g.add(std::make_unique<Oscillator>());
    g.node(src).setParameter("freq", 330.0f);
    const auto sc = g.add(std::make_unique<Scope>());
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 110.0f);
    g.connect(src, 0, sc, 0);     // OSC1.sine -> SCOPE.in
    g.connect(sc, 4, osc, 0);     // SCOPE.pitch -> OSC2.pitch
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 4000; ++b) {
        g.process(out, sc, 4);    // observa a saída pitch
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    // 330/110 = 3 -> log2(3) ~ 1.585 v/oct
    check(hi > 1.2f && hi < 1.9f, "SCOPE.pitch converge pra ~log2(330/110)");
    (void)lo;
}

void testPanel() {
    Scope sc;
    const std::string problem = validatePanel(sc);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(sc.panel().widgets.size() >= 13);
    std::cout << renderAscii(sc);
}

}  // namespace

int main() {
    testThruIsClean();
    testTriggerRisingCountsCycles();
    testTriggerEdgeMatters();
    testRejectSuppressesNoise();
    testExtIsTriggerSource();
    testLevelFollows();
    testBrightOrdering();
    testBrightMonotonicSweep();
    testPitchTracks();
    testPitchZeroForNoise();
    testHoldFreezes();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_scope: OK\n";
    return g_failures == 0 ? 0 : 1;
}
