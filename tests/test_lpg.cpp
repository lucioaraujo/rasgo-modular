// Teste isolado do Módulo 25 (LPG — low-pass gate a vactrol).
// Critérios do dossiê `dossies/25_lpg.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Lpg.hpp"
#include "dsp/Oscillator.hpp"
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

// aplica um golpe curto (gate alto por `gateBlocks`), depois solta.
// `in` = seno de `fin`. Devolve a saída amostra a amostra.
std::vector<float> strike(Lpg& lpg, int gateBlocks, int tailBlocks,
                          float fin = 200.0f, float cvLevel = 0.0f,
                          bool hasCv = false, bool hasStrike = true) {
    lpg.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock i(kSr, 1, kB), s(kSr, 1, kB), c(kSr, 1, kB);
    std::vector<float> o;
    double ph = 0.0;
    for (int b = 0; b < gateBlocks + tailBlocks; ++b) {
        const float gate = (b < gateBlocks) ? 1.0f : 0.0f;
        for (std::size_t k = 0; k < kB; ++k) {
            i.at(0, k) = std::sin(static_cast<float>(ph));
            ph += 6.283185307 * fin / kSr;
            s.at(0, k) = gate;
            c.at(0, k) = cvLevel;
        }
        std::vector<const AudioBlock*> ins{
            &i, hasStrike ? &s : nullptr, hasCv ? &c : nullptr};
        lpg.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) o.push_back(out[0].at(0, k));
    }
    return o;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0; for (std::size_t i = a; i < b && i < v.size(); ++i) s += double(v[i]) * v[i];
    return std::sqrt(s / std::max<std::size_t>(1, std::min(b, v.size()) - a));
}
// índice onde o RMS local cai abaixo de `frac` do pico
std::size_t decayTo(const std::vector<float>& v, double frac) {
    const std::size_t w = 512;
    double pk = 0;
    for (std::size_t i = 0; i + w < v.size(); i += w) pk = std::max(pk, rms(v, i, i + w));
    for (std::size_t i = 0; i + w < v.size(); i += w)
        if (rms(v, i, i + w) < pk * frac && i > w * 4) return i;
    return v.size();
}
double harm(const std::vector<float>& v, double f, int n) {
    double re = 0, im = 0;
    const double w = 6.283185307 * f * n / kSr;
    for (std::size_t k = 0; k < v.size(); ++k) { re += v[k] * std::cos(w * k); im += v[k] * std::sin(w * k); }
    return std::sqrt(re * re + im * im) / v.size();
}

void testAttackFasterThanRelease() {
    Lpg lpg;
    lpg.setParameter("mode", 0.5f);
    lpg.setParameter("response", 0.5f);
    const auto o = strike(lpg, 6, 200);
    // sobe: pico de RMS local alcançado cedo; desce: cai devagar
    const std::size_t peakAt = [&] {
        const std::size_t w = 512; double pk = 0; std::size_t at = 0;
        for (std::size_t i = 0; i + w < o.size(); i += w) {
            const double r = rms(o, i, i + w);
            if (r > pk) { pk = r; at = i; }
        }
        return at;
    }();
    const std::size_t down = decayTo(o, 0.1);
    check(peakAt < 6 * kB + 4 * kB, "sobe rápido (pico logo após o golpe)");
    check(down > peakAt + 20 * kB, "desce devagar (cauda longa)");
}

void testResponseLongerTail() {
    Lpg shortR, longR;
    shortR.setParameter("response", 0.1f);
    longR.setParameter("response", 0.9f);
    const auto s = strike(shortR, 6, 400);
    const auto l = strike(longR, 6, 400);
    check(decayTo(l, 0.1) > decayTo(s, 0.1) + 20 * kB,
          "response alto -> cauda mais longa");
}

void testModeFilterVsVca() {
    // mode=0 (filtro): corta agudo mas mantém corpo grave enquanto abre;
    // mode=1 (VCA): não colore, só o envelope de amplitude segue
    Lpg filt, vca;
    filt.setParameter("mode", 0.0f);
    filt.setParameter("response", 0.15f);
    vca.setParameter("mode", 1.0f);
    vca.setParameter("response", 0.15f);
    // fonte rica em agudo: usa uma frequência alta
    const auto fo = strike(filt, 8, 300, 2500.0f);
    const auto vo = strike(vca, 8, 300, 2500.0f);
    // logo após o golpe (env alto) vs no fim da cauda (env baixo):
    const double fEarly = harm({fo.begin() + 2 * kB, fo.begin() + 10 * kB}, 2500.0, 1);
    const double fLate = harm({fo.begin() + 200 * kB, fo.begin() + 300 * kB}, 2500.0, 1);
    // no modo filtro, o 2,5 kHz cai MUITO mais que a energia total quando fecha
    check(fLate < fEarly * 0.5, "mode=0: o agudo desaparece ao fechar (filtragem)");
    // no modo VCA o formato espectral se mantém (razão agudo/total ~constante)
    const double vEarly = harm({vo.begin() + 2 * kB, vo.begin() + 10 * kB}, 2500.0, 1)
        / std::max(1e-9, rms(vo, 2 * kB, 10 * kB));
    const double vLate = harm({vo.begin() + 40 * kB, vo.begin() + 90 * kB}, 2500.0, 1)
        / std::max(1e-9, rms(vo, 40 * kB, 90 * kB));
    check(std::fabs(vEarly - vLate) < vEarly * 0.4 + 0.05,
          "mode=1: espectro ~constante, só a amplitude segue");
}

void testOffsetPassesAtRest() {
    Lpg closed, open;
    open.setParameter("offset", 0.6f);
    const auto c = strike(closed, 0, 60);   // nunca golpeado
    const auto o = strike(open, 0, 60);
    check(rms(c, 20 * kB, 55 * kB) < 0.01, "offset=0: silêncio em repouso");
    check(rms(o, 20 * kB, 55 * kB) > 0.05, "offset>0: deixa passar em repouso");
}

void testCvOpens() {
    Lpg lpg;
    // sem strike, cv=1 -> abre
    const auto o = strike(lpg, 0, 40, 200.0f, 1.0f, /*hasCv=*/true, /*hasStrike=*/false);
    check(rms(o, 15 * kB, 38 * kB) > 0.1, "cv=1 abre o LPG sem strike");
}

void testBounce() {
    // `bounce` alto -> o env do vactrol passa ACIMA do regime por um
    // instante depois do golpe (overshoot). mode=1 (VCA puro), in = DC 1:
    // a saída ~ o próprio env, então o pico revela o overshoot.
    auto peakEnv = [](float bounce) {
        Lpg lpg;
        lpg.setParameter("mode", 1.0f);
        lpg.setParameter("response", 0.3f);
        lpg.setParameter("bounce", bounce);
        lpg.prepare(kSr, kB);
        std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
        AudioBlock i(kSr, 1, kB), s(kSr, 1, kB);
        float pk = 0.0f;
        for (int b = 0; b < 120; ++b) {
            for (std::size_t k = 0; k < kB; ++k) { i.at(0, k) = 1.0f; s.at(0, k) = 1.0f; }
            std::vector<const AudioBlock*> ins{&i, &s, nullptr};
            lpg.process(ins, out);
            for (std::size_t k = 0; k < kB; ++k)
                pk = std::max(pk, out[0].at(0, k));
        }
        return pk;
    };
    const float p0 = peakEnv(0.0f);
    const float pb = peakEnv(0.85f);
    check(std::fabs(p0 - 1.0f) < 0.02f, "bounce=0 -> sem overshoot (pico ~1)");
    check(pb > p0 + 0.04f, "bounce alto -> overshoot (pico > regime)");
}

void testResonanceStable() {
    Lpg lpg;
    lpg.setParameter("resonance", 0.95f);
    lpg.setParameter("mode", 0.0f);
    lpg.setParameter("offset", 0.7f);
    const auto o = strike(lpg, 4, 200, 300.0f);
    for (float x : o) EXPECT(std::isfinite(x));
    double pk = 0; for (float x : o) pk = std::max(pk, std::fabs((double)x));
    check(pk < 4.0, "ressonância alta: pico contido (estável)");
}

void testSilentWhenIdle() {
    Lpg lpg;
    const auto o = strike(lpg, 0, 80);
    check(rms(o, 10 * kB, 78 * kB) < 0.005, "sem strike e offset=0 -> silêncio");
}

void testDeterminism() {
    Lpg a, b;
    for (Lpg* p : {&a, &b}) {
        p->setParameter("response", 0.5f);
        p->setParameter("drift", 0.9f);
        p->setParameter("resonance", 0.4f);
    }
    const auto oa = strike(a, 6, 200);
    const auto ob = strike(b, 6, 200);
    bool same = oa.size() == ob.size();
    for (std::size_t k = 0; same && k < oa.size(); ++k) if (oa[k] != ob[k]) same = false;
    EXPECT(same);
}

void testDriftSmall() {
    Lpg d, s;
    d.setParameter("drift", 1.0f);
    d.setParameter("response", 0.5f);
    s.setParameter("drift", 0.0f);
    s.setParameter("response", 0.5f);
    const auto rd = strike(d, 6, 300);
    const auto rs = strike(s, 6, 300);
    const double a = rms(rd, 0, rd.size()), b = rms(rs, 0, rs.size());
    check(std::fabs(a - b) < 0.08 * b + 0.01, "drift altera pouco");
}

void testInGraph() {
    SignalGraph g;
    const auto clk = g.add(std::make_unique<EuclidClock>());
    g.node(clk).setParameter("bpm", 120.0f);
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 220.0f);
    const auto lpg = g.add(std::make_unique<Lpg>());
    g.node(lpg).setParameter("response", 0.5f);
    g.connect(osc, 0, lpg, 0);       // sine -> in
    g.connect(clk, 1, lpg, 1);       // euclid -> strike
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float pk = 0.0f, sMin = 1e9f;
    for (int b = 0; b < 3000; ++b) {
        g.process(out, lpg, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            pk = std::max(pk, std::fabs(out.at(0, k)));
            sMin = std::min(sMin, std::fabs(out.at(0, k)));
        }
    }
    check(pk > 0.05f, "LPG deixa passar quando golpeado");
    check(sMin < 0.02f, "LPG fecha entre os golpes");
}

void testPanel() {
    Lpg lpg;
    const std::string problem = validatePanel(lpg);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(lpg.panel().widgets.size() >= 10);
    std::cout << renderAscii(lpg);
}

}  // namespace

int main() {
    testAttackFasterThanRelease();
    testResponseLongerTail();
    testModeFilterVsVca();
    testOffsetPassesAtRest();
    testCvOpens();
    testBounce();
    testResonanceStable();
    testSilentWhenIdle();
    testDeterminism();
    testDriftSmall();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_lpg: OK\n";
    return g_failures == 0 ? 0 : 1;
}
