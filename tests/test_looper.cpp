// Teste isolado do Módulo 41 (LOOPER — delay com hold/reverse/fita).
// Critérios do dossiê `dossies/41_looper.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Looper.hpp"
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

struct Rec { std::vector<float> out, wet; };

// `src(frame)` = entrada de áudio; `frz`/`rev` opcionais (gate por amostra)
Rec run(Looper& lp, int blocks, std::function<float(std::size_t)> src,
        std::function<float(std::size_t)> frz = nullptr,
        std::function<float(std::size_t)> rev = nullptr) {
    lp.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bf(kSr, 1, kB), br(kSr, 1, kB);
    Rec r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = src ? src(n + k) : 0.0f;
            bf.at(0, k) = frz ? frz(n + k) : 0.0f;
            br.at(0, k) = rev ? rev(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            &bi, nullptr, frz ? &bf : nullptr, rev ? &br : nullptr};
        lp.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.out.push_back(out[0].at(0, k));
            r.wet.push_back(out[1].at(0, k));
        }
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& x, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < x.size(); ++i) s += x[i] * (double)x[i];
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
}

// energia acima de ~4 kHz: RMS do sinal menos o passa-baixa 1 polo dele
double hfEnergy(const std::vector<float>& x, std::size_t a, std::size_t b) {
    double z = 0.0, s = 0.0;
    const double c = 1.0 - std::exp(-6.28318 * 4000.0 / kSr);
    for (std::size_t i = a; i < b && i < x.size(); ++i) {
        z += (x[i] - z) * c;
        const double hp = x[i] - z;
        s += hp * hp;
    }
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
}

void testSimpleEcho() {
    Looper lp;
    lp.setParameter("time", 0.05f);      // 2400 amostras
    lp.setParameter("feedback", 0.5f);
    lp.setParameter("age", 0.0f);
    lp.setParameter("mix", 1.0f);        // só molhado, pra o eco ficar claro
    // impulso de 1 amostra na 100
    const auto r = run(lp, 30, [](std::size_t f) { return f == 100 ? 1.0f : 0.0f; });
    // 1º eco por volta de 100 + 2400
    double p1 = 0.0; std::size_t at1 = 0;
    for (std::size_t i = 2300; i < 2700; ++i)
        if (std::fabs(r.out[i]) > p1) { p1 = std::fabs(r.out[i]); at1 = i; }
    EXPECT(p1 > 0.2);
    EXPECT(at1 > 2400 && at1 < 2620);
    // 2º eco por volta de 100 + 4800, mais fraco
    double p2 = 0.0;
    for (std::size_t i = 4700; i < 5100; ++i)
        p2 = std::max(p2, (double)std::fabs(r.out[i]));
    EXPECT(p2 > 0.05 && p2 < p1);
}

void testFeedbackBounded() {
    Looper lp;
    lp.setParameter("time", 0.02f);
    lp.setParameter("feedback", 1.1f);   // auto-oscila
    lp.setParameter("age", 0.0f);
    lp.setParameter("mix", 1.0f);
    const auto r = run(lp, 60, [](std::size_t f) {
        return f < 4000 ? 0.3f * std::sin(2.0 * M_PI * 220.0 * f / kSr) : 0.0f;
    });
    for (float v : r.out) EXPECT(std::isfinite(v) && std::fabs(v) < 2.5f);
    // ainda tem cauda bem depois do sinal parar (não morreu)
    EXPECT(rms(r.out, 12000, 15000) > 0.02);
}

void testHoldLoops() {
    Looper lp;
    lp.setParameter("time", 0.05f);
    lp.setParameter("feedback", 0.3f);
    lp.setParameter("age", 0.0f);
    lp.setParameter("mix", 1.0f);
    // seno tocando; hold liga na amostra 6000; input novo some na 9000
    auto sig = [](std::size_t f) {
        return f < 9000 ? 0.3f * std::sin(2.0 * M_PI * 300.0 * f / kSr) : 0.0f;
    };
    auto frz = [](std::size_t f) { return f >= 6000 ? 1.0f : 0.0f; };
    const auto r = run(lp, 90, sig, frz);
    // muito depois (amostra ~18000) — com a entrada zerada há tempo — o
    // loop segue soando e NÃO decaiu
    const double e = rms(r.out, 18000, 20000);
    EXPECT(e > 0.08);
    // periódico com ~2400 amostras: correlação alta a lag 2400
    double num = 0.0, d0 = 0.0, d1 = 0.0;
    for (std::size_t i = 15000; i + 2400 < 20000; ++i) {
        num += r.out[i] * r.out[i + 2400];
        d0 += r.out[i] * r.out[i];
        d1 += r.out[i + 2400] * r.out[i + 2400];
    }
    EXPECT(num / std::sqrt(d0 * d1 + 1e-12) > 0.9);
}

void testReverseNoClick() {
    Looper lp;
    lp.setParameter("time", 0.05f);
    lp.setParameter("feedback", 0.0f);
    lp.setParameter("age", 0.0f);
    lp.setParameter("mix", 1.0f);
    lp.setParameter("reverse", 1.0f);
    // rampa ascendente de frequência (chirp) na entrada
    const auto r = run(lp, 60, [](std::size_t f) {
        const double t = f / kSr;
        return 0.3f * std::sin(2.0 * M_PI * (200.0 + 40.0 * t) * t);
    });
    // sem clique: nenhuma diferença amostra-a-amostra gigante
    float maxJump = 0.0f;
    for (std::size_t i = 5000; i < r.out.size(); ++i)
        maxJump = std::max(maxJump, std::fabs(r.out[i] - r.out[i - 1]));
    EXPECT(maxJump < 0.35f);
    EXPECT(rms(r.out, 6000, 12000) > 0.02);   // tem saída
}

void testAgeDarkens() {
    auto tail = [](float age) {
        Looper lp;
        lp.setParameter("time", 0.03f);
        lp.setParameter("feedback", 0.7f);
        lp.setParameter("age", age);
        lp.setParameter("mix", 1.0f);
        // rajada de ruído branco
        std::uint32_t s = 12345;
        auto nz = [&s](std::size_t) {
            s = s * 1664525u + 1013904223u;
            return (float)((int)s / 2147483648.0) * 0.3f;
        };
        // ruído só nos primeiros 3000; depois a cauda é só eco realimentado
        std::size_t k = 0;
        return run(lp, 50, [&](std::size_t f) {
            (void)f; return f < 3000 ? nz(k++) : 0.0f;
        });
    };
    const auto young = tail(0.0f);
    const auto old = tail(1.0f);
    // a cauda do eco (bem depois do ruído) tem MENOS agudo com age alto
    const double hy = hfEnergy(young.wet, 8000, 11000);
    const double ho = hfEnergy(old.wet, 8000, 11000);
    EXPECT(ho < hy * 0.7);
}

void testMixDry() {
    Looper lp;
    lp.setParameter("mix", 0.0f);
    lp.setParameter("feedback", 0.5f);
    const auto r = run(lp, 10, [](std::size_t f) {
        return 0.4f * std::sin(2.0 * M_PI * 440.0 * f / kSr);
    });
    for (std::size_t i = 0; i < r.out.size(); ++i) {
        const float dry = 0.4f * std::sin(2.0 * M_PI * 440.0 * i / kSr);
        EXPECT(std::fabs(r.out[i] - dry) < 1e-4f);
    }
}

void testDeterminism() {
    Looper a, b;
    for (Looper* x : {&a, &b}) {
        x->setParameter("time", 0.07f);
        x->setParameter("feedback", 0.6f);
        x->setParameter("age", 0.5f);
    }
    auto sig = [](std::size_t f) { return 0.3f * std::sin(0.03 * f); };
    const auto ra = run(a, 25, sig);
    const auto rb = run(b, 25, sig);
    bool same = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; same && i < ra.out.size(); ++i)
        same = ra.out[i] == rb.out[i];
    EXPECT(same);
}

void testPanel() {
    Looper lp;
    check(validatePanel(lp).empty(), "painel LOOPER fecha");
    std::cout << renderAscii(lp);
}

}  // namespace

int main() {
    testSimpleEcho();
    testFeedbackBounded();
    testHoldLoops();
    testReverseNoClick();
    testAgeDarkens();
    testMixDry();
    testDeterminism();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular LOOPER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
