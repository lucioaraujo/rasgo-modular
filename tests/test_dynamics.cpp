// Teste isolado do Módulo 63 (DYNAMICS — compressor e gate com sidechain).
// Critérios do dossiê `dossies/63_dynamics.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Dynamics.hpp"

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
constexpr double kPi = 3.14159265358979323846;

using Gen = std::function<float(std::size_t)>;
struct Run { std::vector<float> out, env, gr; };

Run run(Dynamics& d, int blocks, const Gen& in, const Gen& side = {}, const Gen& thr = {}) {
    d.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bs(kSr, 1, kB), bt(kSr, 1, kB);
    Run r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = in ? in(n + k) : 0.0f;
            bs.at(0, k) = side ? side(n + k) : 0.0f;
            bt.at(0, k) = thr ? thr(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{in ? &bi : nullptr, side ? &bs : nullptr, thr ? &bt : nullptr};
        d.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.out.push_back(out[0].at(0, k));
            r.env.push_back(out[1].at(0, k));
            r.gr.push_back(out[2].at(0, k));
        }
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    check(b <= v.size(), "janela dentro do áudio gerado");
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c) s += v[i] * static_cast<double>(v[i]);
    return std::sqrt(s / static_cast<double>(std::max<std::size_t>(1, c)));
}
double db(double x) { return 20.0 * std::log10(std::max(x, 1e-12)); }
Gen sine(float amp, float hz) {
    return [=](std::size_t n) { return amp * static_cast<float>(std::sin(2.0 * kPi * hz * n / kSr)); };
}

void testStaticCurve() {
    // Joelho duro, 4:1, limiar −18: 12 dB acima viram 3 → redução de 9 dB.
    EXPECT(Dynamics::staticGainDb(-30.0f, -18.0f, 4.0f, 0.0f, false) == 0.0f);
    EXPECT(std::fabs(Dynamics::staticGainDb(-6.0f, -18.0f, 4.0f, 0.0f, false) + 9.0f) < 1e-4f);
    // Ratio no máximo = limitador: tudo acima do limiar some.
    EXPECT(std::fabs(Dynamics::staticGainDb(-6.0f, -18.0f, 20.0f, 0.0f, false) + 12.0f) < 1e-4f);
    // Joelho suave: contínuo nas duas bordas e mais suave no meio.
    const float w = 8.0f;
    EXPECT(std::fabs(Dynamics::staticGainDb(-22.0f, -18.0f, 4.0f, w, false)) < 1e-4f);
    EXPECT(std::fabs(Dynamics::staticGainDb(-14.0f, -18.0f, 4.0f, w, false)
                     - Dynamics::staticGainDb(-14.0f, -18.0f, 4.0f, 0.0f, false)) < 1e-3f);
    EXPECT(Dynamics::staticGainDb(-18.0f, -18.0f, 4.0f, w, false) < 0.0f);
    // Gate: acima do limiar passa; abaixo, expande para baixo (até −80 dB).
    EXPECT(Dynamics::staticGainDb(-10.0f, -30.0f, 4.0f, 0.0f, true) == 0.0f);
    EXPECT(std::fabs(Dynamics::staticGainDb(-40.0f, -30.0f, 4.0f, 0.0f, true) + 30.0f) < 1e-4f);
    EXPECT(Dynamics::staticGainDb(-120.0f, -30.0f, 20.0f, 0.0f, true) >= -80.0f);
}

void testCompressesSteadyTone() {
    // Seno de −6 dBFS (pico 0,5) em RMS, limiar −18, 4:1, sem joelho:
    // redução de ~9 dB na saída.
    Dynamics d;
    d.setParameter("thresh", -18.0f); d.setParameter("ratio", 4.0f); d.setParameter("knee", 0.0f);
    d.setParameter("attack", 1.0f); d.setParameter("release", 50.0f); d.setParameter("detect", 1.0f);
    const auto r = run(d, 400, sine(0.5f, 220.0f));
    const double inDb = db(0.5 / std::sqrt(2.0));
    const double outDb = db(rms(r.out, r.out.size() - 48000, r.out.size()));
    std::cout << "compressor: entrada " << inDb << " dB, saída " << outDb << " dB\n";
    EXPECT(std::fabs((inDb - outDb) - 9.0) < 1.0);
    EXPECT(r.gr.back() > 0.3f && r.gr.back() < 0.45f);  // 9 dB / 24
    // Makeup devolve o nível.
    Dynamics m;
    m.setParameter("thresh", -18.0f); m.setParameter("ratio", 4.0f); m.setParameter("knee", 0.0f);
    m.setParameter("attack", 1.0f); m.setParameter("release", 50.0f); m.setParameter("detect", 1.0f);
    m.setParameter("makeup", 9.0f);
    const auto rm = run(m, 400, sine(0.5f, 220.0f));
    EXPECT(std::fabs(db(rms(rm.out, rm.out.size() - 48000, rm.out.size())) - inDb) < 1.0);
}

void testAttackAndRelease() {
    // Silêncio → tom forte: a redução cresce em ~ataque e volta em ~release.
    Dynamics d;
    d.setParameter("thresh", -30.0f); d.setParameter("ratio", 20.0f); d.setParameter("knee", 0.0f);
    d.setParameter("attack", 20.0f); d.setParameter("release", 200.0f); d.setParameter("detect", 1.0f);
    const std::size_t on = 9600, off = 9600 + 48000;
    const auto r = run(d, 600, [&](std::size_t n) {
        return (n >= on && n < off) ? 0.5f * static_cast<float>(std::sin(2.0 * kPi * 220.0 * n / kSr)) : 0.0f;
    });
    const float settled = r.gr[off - 1];
    EXPECT(settled > 0.5f);
    EXPECT(r.gr[on + 480] < settled * 0.9f);              // 10 ms: ainda subindo
    EXPECT(r.gr[on + 9600] > settled * 0.9f);             // 200 ms: chegou
    EXPECT(r.gr[off + 4800] > 0.2f * settled);            // 100 ms depois: ainda soltando
    EXPECT(r.gr[off + 48000] < 0.05f);                    // 1 s depois: solto
}

void testGate() {
    Dynamics d;
    d.setParameter("mode", 1.0f); d.setParameter("thresh", -30.0f); d.setParameter("ratio", 8.0f);
    d.setParameter("knee", 0.0f); d.setParameter("attack", 1.0f); d.setParameter("release", 20.0f);
    d.setParameter("detect", 1.0f);
    const auto quiet = run(d, 200, sine(0.01f, 220.0f));   // −40 dB: abaixo do limiar
    EXPECT(db(rms(quiet.out, quiet.out.size() - 24000, quiet.out.size())) < -40.0 - 3.0 - 30.0);
    const auto loud = run(d, 200, sine(0.3f, 220.0f));     // acima: passa
    EXPECT(std::fabs(db(rms(loud.out, loud.out.size() - 24000, loud.out.size())) - db(0.3 / std::sqrt(2.0))) < 0.5);
}

void testSidechainAndModulation() {
    // A entrada é um tom baixo (−26 dB), abaixo do limiar; o sidechain bate forte em
    // rajadas — a entrada abaixa só quando ele bate.
    const auto pulses = [](std::size_t n) {
        const bool hit = (n % 24000) < 4800;
        return hit ? 0.8f * static_cast<float>(std::sin(2.0 * kPi * 60.0 * n / kSr)) : 0.0f;
    };
    Dynamics d;
    d.setParameter("thresh", -24.0f); d.setParameter("ratio", 10.0f); d.setParameter("knee", 0.0f);
    d.setParameter("attack", 2.0f); d.setParameter("release", 60.0f);
    const auto r = run(d, 600, sine(0.05f, 440.0f), pulses);
    const std::size_t base = 48000 * 2;
    const double during = rms(r.out, base + 1200, base + 4800);
    const double between = rms(r.out, base + 14400, base + 24000);
    std::cout << "sidechain: durante o golpe " << db(during) << " dB, entre golpes " << db(between) << " dB\n";
    EXPECT(db(between) - db(during) > 10.0);
    EXPECT(r.gr[base + 4000] > 0.3f && r.gr[base + 20000] < 0.05f);
    EXPECT(r.env[base + 4000] > r.env[base + 20000] + 0.3f);
    // SIDE = 0: o detector ouve só a entrada (tom baixo) → nada abaixa.
    Dynamics flat;
    flat.setParameter("thresh", -24.0f); flat.setParameter("ratio", 10.0f); flat.setParameter("side_mix", 0.0f);
    flat.setParameter("knee", 0.0f);
    const auto rf = run(flat, 600, sine(0.05f, 440.0f), pulses);
    for (std::size_t i = 0; i < rf.gr.size(); i += 997) EXPECT(rf.gr[i] < 1e-3f);
    // Sem entrada e só com o sidechain: silêncio na saída, mas ENV e GR se
    // movem — o módulo como gerador de modulação.
    Dynamics mod;
    mod.setParameter("thresh", -24.0f); mod.setParameter("ratio", 10.0f);
    const auto rmod = run(mod, 300, {}, pulses);
    double outEnergy = 0; for (float x : rmod.out) outEnergy += x * x;
    EXPECT(outEnergy == 0.0);
    EXPECT(rmod.gr[48000 + 4000] > 0.3f && rmod.env[48000 + 4000] > 0.5f);
    // CV no limiar: +1 sobe 24 dB → o tom de −6 dB deixa de ser comprimido.
    Dynamics cv;
    cv.setParameter("thresh", -18.0f); cv.setParameter("ratio", 4.0f); cv.setParameter("detect", 1.0f);
    const auto rcv = run(cv, 200, sine(0.5f, 220.0f), {}, [](std::size_t) { return 1.0f; });
    EXPECT(rcv.gr.back() < 1e-3f);
}

void testBreathAndDrift() {
    // Golpes a cada 0,25 s, release no máximo (2 s): sem BREATH a redução
    // quase não solta entre golpes; com BREATH = 1 o release segue o
    // intervalo (~0,15 s) e solta.
    const auto hits = [](std::size_t n) {
        const bool hit = (n % 12000) < 1200;
        return hit ? 0.7f * static_cast<float>(std::sin(2.0 * kPi * 110.0 * n / kSr)) : 0.0f;
    };
    auto setup = [](Dynamics& d, float breath, float drift) {
        d.setParameter("thresh", -24.0f); d.setParameter("ratio", 10.0f); d.setParameter("knee", 0.0f);
        d.setParameter("attack", 1.0f); d.setParameter("release", 2000.0f);
        d.setParameter("breath", breath); d.setParameter("drift", drift);
    };
    Dynamics a, b;
    setup(a, 0.0f, 0.0f); setup(b, 1.0f, 0.0f);
    const auto ra = run(a, 1000, hits), rb = run(b, 1000, hits);
    const std::size_t before = 12000 * 20 - 50;  // logo antes de um golpe
    std::cout << "breath: GR antes do golpe — fixo " << ra.gr[before] << ", respirando " << rb.gr[before] << '\n';
    EXPECT(ra.gr[before] > 0.4f);
    EXPECT(rb.gr[before] < 0.5f * ra.gr[before]);
    // Drift: semeado — duas rodadas iguais; diferente de drift = 0.
    Dynamics c1, c2;
    setup(c1, 0.0f, 0.8f); setup(c2, 0.0f, 0.8f);
    const auto r1 = run(c1, 300, hits), r2 = run(c2, 300, hits);
    EXPECT(r1.out == r2.out);
    EXPECT(r1.out != ra.out);
}

void testRobustness() {
    Dynamics d;
    const auto r = run(d, 50, [](std::size_t n) {
        return n % 100 == 0 ? std::nanf("") : (n % 77 == 0 ? INFINITY : 0.5f);
    });
    bool finite = true;
    for (std::size_t i = 0; i < r.out.size(); ++i)
        finite = finite && std::isfinite(r.out[i]) && std::isfinite(r.env[i]) && std::isfinite(r.gr[i]);
    EXPECT(finite);
    // Sem nada ligado: tudo zero.
    Dynamics e;
    const auto z = run(e, 20, {});
    double energy = 0; for (float x : z.out) energy += x * x;
    EXPECT(energy == 0.0);
}

}  // namespace

int main() {
    testStaticCurve();
    testCompressesSteadyTone();
    testAttackAndRelease();
    testGate();
    testSidechainAndModulation();
    testBreathAndDrift();
    testRobustness();
    if (g_failures == 0) std::cout << "DYNAMICS: todos os testes passaram\n";
    return g_failures == 0 ? 0 : 1;
}
