// Teste isolado do Módulo 58 (SHIFTER — deslocador de frequência).
// Critérios do dossiê `dossies/58_shifter.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Shifter.hpp"
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

std::vector<float> run(Shifter& s, int blocks, const Gen& in, int outIdx = 0,
                       const Gen& mod = nullptr) {
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bm(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            if (in) bi.at(0, k) = in(n + k);
            if (mod) bm.at(0, k) = mod(n + k);
        }
        std::vector<const AudioBlock*> ins{in ? &bi : nullptr,
                                           mod ? &bm : nullptr};
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return r;
}

double magAt(const std::vector<float>& x, double hz, std::size_t a, std::size_t b) {
    double re = 0.0, im = 0.0, ws = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    const std::size_t n = b - a;
    for (std::size_t i = a; i < b && i < x.size(); ++i) {
        const double t = (double)(i - a);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * t / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        ws += w;
    }
    return std::sqrt(re * re + im * im) / ws;
}

Gen sine(double hz, float amp = 0.7f) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(6.2831853 * hz * (double)n / kSr);
    };
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c) s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
}

// ---- testes ----

void testZeroShiftPassesThrough() {
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 0.0f);
    const auto up = run(s, 200, sine(440.0, 0.5f), 0);
    // com atraso de M amostras; a energia fica em 440 e nada mais
    EXPECT(magAt(up, 440.0, 20000, 50000) > 0.15);
    EXPECT(magAt(up, 300.0, 20000, 50000) < 0.02);
    EXPECT(magAt(up, 600.0, 20000, 50000) < 0.02);
}

void testUpShiftMovesSpectrumUpBy_Hz() {
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 120.0f);
    const auto up = run(s, 200, sine(440.0), 0);      // saída `up`
    EXPECT(magAt(up, 560.0, 20000, 60000) > 0.1);     // 440 + 120
    EXPECT(magAt(up, 440.0, 20000, 60000) < 0.02);    // a original sumiu
    // rejeição de imagem (440 − 120 = 320) > ~25 dB
    EXPECT(magAt(up, 560.0, 20000, 60000)
           > 15.0 * magAt(up, 320.0, 20000, 60000));
}

void testDownShiftMovesSpectrumDown() {
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 120.0f);
    const auto dn = run(s, 200, sine(440.0), 1);      // saída `down`
    EXPECT(magAt(dn, 320.0, 20000, 60000) > 0.1);     // 440 − 120
    EXPECT(magAt(dn, 440.0, 20000, 60000) < 0.02);
    EXPECT(magAt(dn, 320.0, 20000, 60000)
           > 15.0 * magAt(dn, 560.0, 20000, 60000));
}

void testShiftIsAdditiveNotMultiplicative() {
    // um deslocamento FIXO em Hz: dois parciais harmônicos deixam de
    // ser harmônicos (300/600 → 400/700 com +100), não escalam juntos.
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 100.0f);
    const Gen g = [](std::size_t n) {
        const double t = (double)n / kSr;
        return 0.4f * std::sin(6.2831853 * 300.0 * t)
             + 0.4f * std::sin(6.2831853 * 600.0 * t);
    };
    const auto up = run(s, 200, g, 0);
    EXPECT(magAt(up, 400.0, 20000, 60000) > 0.05);    // 300 + 100
    EXPECT(magAt(up, 700.0, 20000, 60000) > 0.05);    // 600 + 100
    EXPECT(magAt(up, 600.0, 20000, 60000) < 0.03);    // NÃO 300·2
}

void testNegativeShiftSwapsOutputs() {
    auto e = [](float sh, int out, double hz) {
        Shifter s;
        s.setParameter("mix", 1.0f);
        s.setParameter("shift", sh);
        return magAt(run(s, 200, sine(500.0), out), hz, 20000, 60000);
    };
    // shift +150: `up` sobe pra 650
    EXPECT(e(+150.0f, 0, 650.0) > 0.1);
    // shift −150: `up` agora DESCE pra 350 (o sinal inverte os papéis)
    EXPECT(e(-150.0f, 0, 350.0) > 0.1);
    EXPECT(e(-150.0f, 0, 650.0) < 0.03);
}

void testShiftModCv() {
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 0.0f);
    // CV 0,2 = +200 Hz (escala 1000 Hz/unidade)
    const auto up = run(s, 200, sine(400.0), 0,
                        [](std::size_t) { return 0.2f; });
    EXPECT(magAt(up, 600.0, 20000, 60000) > 0.1);
    EXPECT(magAt(up, 400.0, 20000, 60000) < 0.03);
}

void testFeedbackClimbsAndStaysBounded() {
    Shifter s;
    s.setParameter("mix", 1.0f);
    s.setParameter("shift", 50.0f);
    s.setParameter("feedback", 0.75f);
    const auto up = run(s, 400, sine(300.0), 0);
    // barber pole: energia espalha por várias faixas acima de 300
    const double base = magAt(up, 300.0, 60000, 100000);
    const double higher = magAt(up, 500.0, 60000, 100000)
                        + magAt(up, 800.0, 60000, 100000)
                        + magAt(up, 1200.0, 60000, 100000);
    EXPECT(higher > base * 0.5);
    for (float v : up) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testMixBypass() {
    Shifter s;
    s.setParameter("mix", 0.0f);      // 100% seco (sem atraso no seco)
    s.setParameter("shift", 400.0f);
    const auto up = run(s, 60, sine(440.0, 0.5f), 0);
    for (std::size_t i = 20000; i < up.size(); ++i)
        EXPECT(std::fabs(up[i] - 0.5f * std::sin(6.2831853 * 440.0
               * (double)i / kSr)) < 1e-4f);
}

void testToneTilts() {
    auto hi = [](float tone) {
        Shifter s;
        s.setParameter("mix", 1.0f);
        s.setParameter("shift", 600.0f);
        s.setParameter("tone", tone);
        const auto up = run(s, 200, sine(2000.0), 0);
        return magAt(up, 2600.0, 20000, 60000);
    };
    EXPECT(hi(-0.9f) < 0.6f * hi(0.0f) + 1e-6f);   // tone baixo abafa o agudo
}

void testDeterminism() {
    auto mk = [](Shifter& s) {
        s.setParameter("shift", 233.0f);
        s.setParameter("feedback", 0.4f);
        s.setParameter("drift", 0.0f);
    };
    Shifter a; mk(a);
    Shifter b; mk(b);
    const auto ua = run(a, 200, sine(370.0), 0);
    const auto ub = run(b, 200, sine(370.0), 0);
    bool same = ua.size() == ub.size();
    for (std::size_t i = 0; same && i < ua.size(); ++i) same = ua[i] == ub[i];
    EXPECT(same);
}

void testDriftIsSeededAndModest() {
    Shifter a, b;
    a.setParameter("drift", 1.0f); a.setParameter("shift", 200.0f);
    b.setParameter("drift", 1.0f); b.setParameter("shift", 200.0f);
    const auto ra = run(a, 200, sine(300.0), 0);
    const auto rb = run(b, 200, sine(300.0), 0);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);                                   // drift é semeado
    Shifter c; c.setParameter("drift", 0.0f); c.setParameter("shift", 200.0f);
    const auto rc = run(c, 200, sine(300.0), 0);
    // com drift o RMS não deve colapsar nem explodir
    EXPECT(rms(ra, 20000, 60000) > 0.4 * rms(rc, 20000, 60000));
    EXPECT(rms(ra, 20000, 60000) < 2.5 * rms(rc, 20000, 60000));
}

void testBounded() {
    Shifter s;
    s.setParameter("shift", 2000.0f);
    s.setParameter("feedback", 0.95f);
    s.setParameter("tone", 1.0f);
    s.setParameter("drift", 1.0f);
    const Gen g = [](std::size_t n) {
        const double t = (double)n / kSr;
        return 0.9f * (std::sin(6.2831853 * 90.0 * t)
                     + std::sin(6.2831853 * 1700.0 * t));
    };
    const auto up = run(s, 200, g, 0);
    const auto dn = run(s, 200, g, 1);
    for (float v : up) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
    for (float v : dn) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
}

void testPanel() {
    Shifter s;
    check(validatePanel(s).empty(), "painel SHIFTER fecha");
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testZeroShiftPassesThrough();
    testUpShiftMovesSpectrumUpBy_Hz();
    testDownShiftMovesSpectrumDown();
    testShiftIsAdditiveNotMultiplicative();
    testNegativeShiftSwapsOutputs();
    testShiftModCv();
    testFeedbackClimbsAndStaysBounded();
    testMixBypass();
    testToneTilts();
    testDeterminism();
    testDriftIsSeededAndModest();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SHIFTER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
