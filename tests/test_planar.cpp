// Teste isolado do Módulo 43 (PLANAR — morph vetorial XY).
// Critérios do dossiê `dossies/43_planar.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Planar.hpp"
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

// A varredura de `x` do teste de CV, em ESCOPO DE ARQUIVO — e a razão é
// portabilidade, não organização.
//
// Como variável local, esta constante não tem forma que agrade aos três
// compiladores. Sendo `const` e capturada pela lambda, o Clang acusa
// `-Wunused-lambda-capture`; sendo `const` e não capturada, o MSVC acusa
// C3493. Trocar por `constexpr` deveria bastar pelo padrão (ler uma
// constante não é odr-use, logo não exige captura), mas o MSVC recusa
// assim mesmo — tentei, e a CI mostrou o mesmo C3493 na rodada seguinte.
//
// Nome em escopo de arquivo encerra a discussão: não existe captura de
// variável não-local, em compilador nenhum. Dois erros meus nesta mesma
// linha ensinaram que o caminho não era achar o adjetivo certo, e sim
// tirar o nome do escopo onde a captura é sequer uma pergunta.
constexpr int kSweepBlocks = 24;
constexpr std::size_t kSweepTotal =
    static_cast<std::size_t>(kSweepBlocks) * kB;

using Gen = std::function<float(std::size_t)>;

struct Rec { std::vector<float> out, xo, yo; };

Rec run(Planar& p, int blocks,
        Gen a = nullptr, Gen b = nullptr, Gen c = nullptr, Gen d = nullptr,
        Gen x = nullptr, Gen y = nullptr, Gen gate = nullptr) {
    p.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock ba(kSr, 1, kB), bb(kSr, 1, kB), bc(kSr, 1, kB), bd(kSr, 1, kB),
        bx(kSr, 1, kB), by(kSr, 1, kB), bg(kSr, 1, kB);
    Rec r;
    std::size_t n = 0;
    for (int blk = 0; blk < blocks; ++blk) {
        for (std::size_t k = 0; k < kB; ++k) {
            ba.at(0, k) = a ? a(n + k) : 0.0f;
            bb.at(0, k) = b ? b(n + k) : 0.0f;
            bc.at(0, k) = c ? c(n + k) : 0.0f;
            bd.at(0, k) = d ? d(n + k) : 0.0f;
            bx.at(0, k) = x ? x(n + k) : 0.0f;
            by.at(0, k) = y ? y(n + k) : 0.0f;
            bg.at(0, k) = gate ? gate(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            a ? &ba : nullptr, b ? &bb : nullptr, c ? &bc : nullptr,
            d ? &bd : nullptr, x ? &bx : nullptr, y ? &by : nullptr,
            gate ? &bg : nullptr};
        p.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.out.push_back(out[0].at(0, k));
            r.xo.push_back(out[1].at(0, k));
            r.yo.push_back(out[2].at(0, k));
        }
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
}

Gen sine(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(2.0 * M_PI * hz * (double)n / kSr);
    };
}

void testCornerPure() {
    Planar p;
    p.setParameter("x", 0.0f);
    p.setParameter("y", 0.0f);
    p.setParameter("curve", 0.0f);
    const auto r = run(p, 20, sine(300.0, 0.6), sine(500.0, 0.6),
                       sine(700.0, 0.6), sine(900.0, 0.6));
    // x=y=0 -> wa=1 -> só A
    const auto ref = run(p, 20, sine(300.0, 0.6));
    double err = 0.0;
    for (std::size_t i = 2000; i < r.out.size(); ++i)
        err = std::max(err, (double)std::fabs(r.out[i] - ref.out[i]));
    EXPECT(err < 0.02);
}

void testConstantPower() {
    auto four = [](Planar& p) {
        return run(p, 30, sine(200.0, 0.3), sine(337.0, 0.3),
                   sine(511.0, 0.3), sine(743.0, 0.3));
    };
    Planar corner; corner.setParameter("x", 0.0f); corner.setParameter("y", 0.0f);
    const double rc = rms(four(corner).out, 4000, 7000);

    Planar cenLin;
    cenLin.setParameter("x", 0.5f); cenLin.setParameter("y", 0.5f);
    cenLin.setParameter("curve", 0.0f);
    const double rl = rms(four(cenLin).out, 4000, 7000);

    Planar cenEqp;
    cenEqp.setParameter("x", 0.5f); cenEqp.setParameter("y", 0.5f);
    cenEqp.setParameter("curve", 1.0f);
    const double re = rms(four(cenEqp).out, 4000, 7000);

    EXPECT(rl < 0.62 * rc && rl > 0.38 * rc);   // linear: ~−6 dB no centro
    EXPECT(re > 0.8 * rc && re < 1.25 * rc);     // potência constante: ~0 dB
}

void testSweepAtoB() {
    Planar p;
    p.setParameter("x", 0.0f);   // knob no canto; a CV faz a varredura
    p.setParameter("y", 0.0f);
    p.setParameter("curve", 0.0f);
    p.setParameter("smooth", 0.0f);
    // A = +0,8 DC, B = −0,8 DC; x varre 0→1 pela CV
    // `kSweepBlocks`/`kSweepTotal` vivem no escopo de arquivo — ver a
    // explicação lá em cima: é o que impede a lambda de precisar de
    // captura em qualquer compilador.
    const auto r = run(p, kSweepBlocks,
                       [](std::size_t) { return 0.8f; },
                       [](std::size_t) { return -0.8f; },
                       nullptr, nullptr,
                       [](std::size_t n) {
                           return static_cast<float>(n)
                                / static_cast<float>(kSweepTotal);
                       });
    EXPECT(r.out[40] > 0.7f);                  // x≈0 -> A
    EXPECT(std::fabs(r.out[kSweepTotal / 2]) < 0.1f); // x≈0,5 -> meio
    EXPECT(r.out[kSweepTotal - 40] < -0.7f);          // x≈1 -> B
}

void testSmoothGlide() {
    auto step = [](std::size_t n) { return n >= 5000 ? 1.0f : 0.0f; };
    Planar fast;
    fast.setParameter("x", 0.0f); fast.setParameter("smooth", 0.0f);
    const auto rf = run(fast, 120, nullptr, nullptr, nullptr, nullptr, step);
    EXPECT(rf.xo[5000 + 60] > 0.9f);    // sem smooth: praticamente instantâneo

    Planar slow;
    slow.setParameter("x", 0.0f); slow.setParameter("smooth", 0.5f);
    const auto rs = run(slow, 120, nullptr, nullptr, nullptr, nullptr, step);
    EXPECT(rs.xo[5000 + 200] < 0.3f);   // com smooth: mal saiu do lugar
    EXPECT(rs.xo[5000 + 3000] < 0.85f); // ainda subindo dezenas de ms depois
    EXPECT(rs.xo.back() > 0.9f);        // mas chega (τ≈0,13 s, ~0,5 s de folga)
}

void testDrift() {
    Planar moving;
    moving.setParameter("x", 0.5f); moving.setParameter("y", 0.5f);
    moving.setParameter("drift", 0.6f);
    const auto rm = run(moving, 200);
    double dev = 0.0;
    for (float v : rm.xo) dev = std::max(dev, (double)std::fabs(v - 0.5f));
    EXPECT(dev > 0.05);

    Planar still;
    still.setParameter("x", 0.5f); still.setParameter("y", 0.5f);
    still.setParameter("drift", 0.0f);
    const auto rs = run(still, 60);
    for (float v : rs.xo) EXPECT(std::fabs(v - 0.5f) < 1e-4f);
}

void testGestureLoop() {
    Planar p;
    p.setParameter("x", 0.0f);   // knob no canto; a CV desenha o gesto
    p.setParameter("y", 0.5f);
    p.setParameter("smooth", 0.0f);
    p.setParameter("rate", 0.5f);   // 1×
    // gate alto [1000, 5000); x varre 0→1 nesse intervalo
    auto gate = [](std::size_t n) { return (n >= 1000 && n < 5000) ? 1.0f : 0.0f; };
    auto xcv = [](std::size_t n) {
        if (n < 1000 || n >= 5000) return 0.0f;
        return static_cast<float>(n - 1000) / 4000.0f;
    };
    const auto r = run(p, 60, nullptr, nullptr, nullptr, nullptr, xcv, nullptr, gate);
    // durante a gravação, x_out segue a varredura
    EXPECT(std::fabs(r.xo[3000] - 0.5f) < 0.12f);
    // depois de soltar: o gesto toca em loop (recLen ~125 quadros = ~4000 amostras)
    const std::size_t base = 5000;
    EXPECT(std::fabs(r.xo[base + 2000] - 0.5f) < 0.18f);   // meio do gesto
    EXPECT(r.xo[base + 3700] > 0.75f);                     // fim do gesto
    EXPECT(std::fabs(r.xo[base + 4000 + 2000] - 0.5f) < 0.18f);  // loop repete
}

void testShortTapClears() {
    Planar p;
    p.setParameter("x", 0.7f);
    p.setParameter("y", 0.2f);
    auto gate = [](std::size_t n) { return (n >= 1000 && n < 1020) ? 1.0f : 0.0f; };
    const auto r = run(p, 30, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gate);
    // toque curto não deixa gesto -> x_out fica no knob
    EXPECT(std::fabs(r.xo.back() - 0.7f) < 1e-3f);
    EXPECT(std::fabs(r.yo.back() - 0.2f) < 1e-3f);
}

void testXYOut() {
    Planar p;
    p.setParameter("x", 0.3f);
    p.setParameter("y", 0.8f);
    const auto r = run(p, 20);
    EXPECT(std::fabs(r.xo.back() - 0.3f) < 1e-3f);
    EXPECT(std::fabs(r.yo.back() - 0.8f) < 1e-3f);
}

void testDeterminism() {
    auto mk = [](Planar& p) {
        p.setParameter("x", 0.4f); p.setParameter("y", 0.6f);
        p.setParameter("curve", 0.5f); p.setParameter("drift", 0.4f);
        p.setParameter("rate", 0.6f); p.setParameter("smooth", 0.3f);
    };
    auto gate = [](std::size_t n) { return (n >= 500 && n < 3000) ? 1.0f : 0.0f; };
    Planar a; mk(a);
    Planar b; mk(b);
    const auto ra = run(a, 30, sine(220, 0.4), sine(330, 0.4), sine(440, 0.4),
                        sine(550, 0.4), nullptr, nullptr, gate);
    const auto rb = run(b, 30, sine(220, 0.4), sine(330, 0.4), sine(440, 0.4),
                        sine(550, 0.4), nullptr, nullptr, gate);
    bool same = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; same && i < ra.out.size(); ++i)
        same = ra.out[i] == rb.out[i] && ra.xo[i] == rb.xo[i];
    EXPECT(same);
}

void testBounded() {
    Planar p;
    p.setParameter("x", 0.5f); p.setParameter("y", 0.5f);
    p.setParameter("curve", 1.0f); p.setParameter("drift", 1.0f);
    p.setParameter("rate", 1.0f);
    const auto r = run(p, 50, sine(200, 1.0), sine(410, 1.0),
                       sine(730, 1.0), sine(1050, 1.0));
    for (float v : r.out) EXPECT(std::isfinite(v) && std::fabs(v) < 1.5f);
}

void testAutonomous() {
    Planar p;
    p.setParameter("x", 0.5f); p.setParameter("y", 0.5f);
    p.setParameter("drift", 0.5f); p.setParameter("smooth", 0.3f);
    const auto r = run(p, 80, sine(180, 0.4), sine(300, 0.4),
                       sine(470, 0.4), sine(690, 0.4));
    EXPECT(rms(r.out, 10000, 18000) > 0.02);
    for (float v : r.out) EXPECT(std::isfinite(v));
}

void testPanel() {
    Planar p;
    check(validatePanel(p).empty(), "painel PLANAR fecha");
    std::cout << renderAscii(p);
}

}  // namespace

int main() {
    testCornerPure();
    testConstantPower();
    testSweepAtoB();
    testSmoothGlide();
    testDrift();
    testGestureLoop();
    testShortTapClears();
    testXYOut();
    testDeterminism();
    testBounded();
    testAutonomous();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular PLANAR tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
