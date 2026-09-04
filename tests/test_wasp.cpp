// Teste isolado do Módulo 32 (WASP — filtro áspero).
// Critérios do dossiê `dossies/32_wasp.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Master.hpp"
#include "dsp/Oscillator.hpp"
#include "dsp/Wasp.hpp"
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

struct Stats {
    std::vector<float> y;
    double rms = 0, peak = 0, mean = 0, posPeak = 0, negPeak = 0;
};

// roda `w` alimentado por senoide de `hz`/`amp` (hz<=0 -> silêncio) por
// `blocks`. `cmod` = valor DC opcional em cutoff_mod.
Stats run(Wasp& w, int blocks, float hz, float amp = 0.5f,
          bool useCmod = false, float cmod = 0.0f) {
    w.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bin(kSr, 1, kB), bc(kSr, 1, kB);
    Stats s;
    double ph = 0.0, sum2 = 0.0, sum = 0.0;
    long n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            float v = 0.0f;
            if (hz > 0.0f) {
                v = amp * static_cast<float>(std::sin(ph));
                ph += 2.0 * M_PI * hz / kSr;
            }
            bin.at(0, k) = v;
            bc.at(0, k) = cmod;
        }
        std::vector<const AudioBlock*> ins{&bin, useCmod ? &bc : nullptr,
                                           nullptr};
        w.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            const float o = out[0].at(0, k);
            check(std::isfinite(o), "finito");
            s.y.push_back(o);
            // ignora o transitório inicial nas estatísticas
            if (b >= 8) {
                sum2 += static_cast<double>(o) * o;
                sum += o;
                s.peak = std::max(s.peak, static_cast<double>(std::fabs(o)));
                s.posPeak = std::max(s.posPeak, static_cast<double>(o));
                s.negPeak = std::min(s.negPeak, static_cast<double>(o));
                ++n;
            }
        }
    }
    if (n > 0) {
        s.rms = std::sqrt(sum2 / n);
        s.mean = sum / n;
    }
    return s;
}

void testLowPass() {
    Wasp w;
    w.setParameter("mode", 0.0f);
    w.setParameter("cutoff", 300.0f);
    w.setParameter("resonance", 0.2f);
    w.setParameter("grit", 0.0f);
    const Stats lo = run(w, 60, 150.0f, 0.5f);
    Wasp w2;
    w2.setParameter("mode", 0.0f);
    w2.setParameter("cutoff", 300.0f);
    w2.setParameter("resonance", 0.2f);
    w2.setParameter("grit", 0.0f);
    const Stats hi = run(w2, 60, 3000.0f, 0.5f);
    check(lo.rms > 0.3, "LP: grave passa");
    check(hi.rms < lo.rms * 0.35, "LP: agudo é cortado");
}

void testHighPass() {
    Wasp lo, hi;
    for (Wasp* w : {&lo, &hi}) {
        w->setParameter("mode", 1.0f);
        w->setParameter("cutoff", 1500.0f);
        w->setParameter("resonance", 0.2f);
        w->setParameter("grit", 0.0f);
    }
    const Stats rl = run(lo, 60, 200.0f, 0.5f);
    const Stats rh = run(hi, 60, 6000.0f, 0.5f);
    check(rh.rms > 0.3, "HP: agudo passa");
    check(rl.rms < rh.rms * 0.35, "HP: grave é cortado");
}

void testModeSweep() {
    // razão energia-aguda / energia-grave cresce com `mode`
    float prev = -1.0f;
    int drops = 0;
    for (float m = 0.0f; m <= 1.0f; m += 0.25f) {
        Wasp lo, hi;
        for (Wasp* w : {&lo, &hi}) {
            w->setParameter("mode", m);
            w->setParameter("cutoff", 1000.0f);
            w->setParameter("resonance", 0.15f);
            w->setParameter("grit", 0.0f);
        }
        const Stats rl = run(lo, 50, 150.0f, 0.5f);
        const Stats rh = run(hi, 50, 7000.0f, 0.5f);
        const float ratio = static_cast<float>(rh.rms / (rl.rms + 1e-6));
        if (ratio < prev - 0.05f) ++drops;
        prev = ratio;
    }
    check(drops == 0, "mode 0->1: razão agudo/grave não-decrescente");
}

void testResonancePeak() {
    Wasp on, off;
    for (Wasp* w : {&on, &off}) {
        w->setParameter("mode", 0.0f);
        w->setParameter("cutoff", 800.0f);
        w->setParameter("grit", 0.0f);
    }
    on.setParameter("resonance", 0.9f);
    off.setParameter("resonance", 0.9f);
    const Stats atFc = run(on, 60, 800.0f, 0.3f);
    const Stats above = run(off, 60, 3200.0f, 0.3f);
    check(atFc.rms > above.rms * 2.0, "resonance: pico em torno da fc");
}

void testSelfOscillates() {
    Wasp lo, hi;
    for (Wasp* w : {&lo, &hi}) {
        w->setParameter("resonance", 1.0f);
        w->setParameter("cutoff", 400.0f);
    }
    lo.setParameter("grit", 0.1f);
    hi.setParameter("grit", 0.95f);
    const Stats rl = run(lo, 160, -1.0f);   // sem entrada
    const Stats rh = run(hi, 160, -1.0f);
    check(rl.rms > 0.05 && rh.rms > 0.03, "auto-oscila sem entrada");
    for (float v : rl.y) check(std::isfinite(v) && std::fabs(v) < 1.0f,
                               "auto-oscilação finita e limitada");
    for (float v : rh.y) check(std::isfinite(v) && std::fabs(v) < 1.0f,
                               "auto-oscilação finita e limitada");
    // `grit` interage com o ciclo-limite (muda a amplitude/forma)
    check(std::fabs(rl.rms - rh.rms) > 0.05,
          "grit muda a auto-oscilação");
}

void testGritAddsHarmonics() {
    Wasp clean, dirty;
    for (Wasp* w : {&clean, &dirty}) {
        w->setParameter("mode", 0.0f);
        w->setParameter("cutoff", 9000.0f);
        w->setParameter("resonance", 0.1f);
        w->setParameter("drive", 0.1f);   // ganho 1x -> o sinal fica no joelho
    }
    clean.setParameter("grit", 0.0f);     // th = 1.0 -> sinal 0.6 passa limpo
    dirty.setParameter("grit", 1.0f);     // th = 0.15 -> comprime
    const Stats rc = run(clean, 60, 400.0f, 0.6f);
    const Stats rd = run(dirty, 60, 400.0f, 0.6f);
    const double crestC = rc.peak / (rc.rms + 1e-9);
    const double crestD = rd.peak / (rd.rms + 1e-9);
    check(crestD < crestC - 0.03, "grit alto achata a onda (menos crista)");
}

void testBiasAsymmetryAndDc() {
    // energia do 2º harmônico (par) relativa à fundamental: ~0 sem bias
    // (ceifamento simétrico só dá ímpares), sobe com bias
    const double f0 = 500.0;
    auto h2ratio = [&](const std::vector<float>& y, std::size_t from) {
        double c1 = 0, s1 = 0, c2 = 0, s2 = 0;
        for (std::size_t i = from; i < y.size(); ++i) {
            const double t = static_cast<double>(i) / kSr;
            c1 += y[i] * std::cos(2 * M_PI * f0 * t);
            s1 += y[i] * std::sin(2 * M_PI * f0 * t);
            c2 += y[i] * std::cos(2 * 2 * M_PI * f0 * t);
            s2 += y[i] * std::sin(2 * 2 * M_PI * f0 * t);
        }
        const double h1 = std::sqrt(c1 * c1 + s1 * s1);
        const double h2 = std::sqrt(c2 * c2 + s2 * s2);
        return h2 / (h1 + 1e-9);
    };
    Wasp flat, tilt;
    for (Wasp* w : {&flat, &tilt}) {
        w->setParameter("mode", 0.0f);
        w->setParameter("cutoff", 3000.0f);
        w->setParameter("resonance", 0.2f);
        w->setParameter("drive", 0.6f);
        w->setParameter("grit", 0.5f);
    }
    tilt.setParameter("bias", 0.85f);
    const Stats rf = run(flat, 60, static_cast<float>(f0), 0.7f);
    const Stats rt = run(tilt, 60, static_cast<float>(f0), 0.7f);
    check(std::fabs(rt.mean) < 0.02, "bias: bloqueador de DC mantém média ~0");
    const double hf = h2ratio(rf.y, 1500), ht = h2ratio(rt.y, 1500);
    check(ht > hf + 0.04 && ht > 0.05,
          "bias: 2º harmônico (par) sobe com a assimetria");
}

void testDriveNoBlowup() {
    Wasp w;
    w.setParameter("drive", 8.0f);
    w.setParameter("resonance", 1.0f);
    w.setParameter("grit", 1.0f);
    w.setParameter("bias", 0.9f);
    const Stats s = run(w, 80, 220.0f, 1.0f);
    for (float v : s.y) check(std::isfinite(v) && std::fabs(v) <= 0.96f,
                              "drive extremo: finito e limitado");
}

void testCutoffMod() {
    Wasp base, up;
    for (Wasp* w : {&base, &up}) {
        w->setParameter("mode", 0.0f);
        w->setParameter("cutoff", 500.0f);
        w->setParameter("resonance", 0.15f);
        w->setParameter("grit", 0.0f);
    }
    // senoide a 1000 Hz: com fc=500 é cortada; com cutoff_mod=+1 (fc=1000)
    // passa muito mais
    const Stats b = run(base, 60, 1000.0f, 0.5f, true, 0.0f);
    const Stats u = run(up, 60, 1000.0f, 0.5f, true, 1.0f);
    check(u.rms > b.rms * 1.5, "cutoff_mod +1 oitava dobra a fc efetiva");
}

void testFiniteExtreme() {
    for (float res : {0.0f, 0.5f, 1.0f})
        for (float grit : {0.0f, 1.0f})
            for (float mode : {0.0f, 0.5f, 1.0f})
                for (float drv : {0.1f, 8.0f}) {
                    Wasp w;
                    w.setParameter("resonance", res);
                    w.setParameter("grit", grit);
                    w.setParameter("mode", mode);
                    w.setParameter("drive", drv);
                    w.setParameter("cutoff", 12000.0f);
                    w.setParameter("bias", -1.0f);
                    const Stats s = run(w, 30, 300.0f, 0.9f);
                    for (float v : s.y)
                        check(std::isfinite(v), "sem NaN/Inf em extremos");
                }
}

void testAliasReduced() {
    // entrada senoidal periódica por não-linearidade -> só k*f0. Energia
    // em bins não-harmônicos = alias. Com o núcleo a 2× o piso fica bem
    // abaixo da fundamental (regressão: pegaria a volta ao rate base).
    Wasp w;
    w.setParameter("mode", 0.0f);
    w.setParameter("cutoff", 6000.0f);
    w.setParameter("resonance", 0.3f);
    w.setParameter("drive", 6.0f);
    w.setParameter("grit", 0.95f);
    const float f0 = 2500.0f;
    const Stats s = run(w, 80, f0, 0.8f);
    auto goertzel = [&](double f) {
        double re = 0, im = 0;
        const double wgt = 2 * M_PI * f / kSr;
        for (std::size_t i = 0; i < s.y.size(); ++i) {
            re += s.y[i] * std::cos(wgt * i);
            im += s.y[i] * std::sin(wgt * i);
        }
        return std::sqrt(re * re + im * im) / s.y.size();
    };
    const double fund = goertzel(f0);
    double al = 0.0;
    int n = 0;
    for (double f = 900.0; f < 20000.0; f += 1100.0) {
        const double kf = f / f0;
        if (std::fabs(kf - std::round(kf)) < 0.16) continue;
        al += goertzel(f);
        ++n;
    }
    check((al / n) / (fund + 1e-9) < 0.02,
          "grit/drive altos: alias < 2% da fundamental");
}

void testDeterminism() {
    Wasp a, b;
    for (Wasp* w : {&a, &b}) {
        w->setParameter("resonance", 0.7f);
        w->setParameter("grit", 0.6f);
        w->setParameter("bias", 0.3f);
        w->setParameter("drive", 2.0f);
        w->setParameter("drift", 0.5f);
    }
    const Stats ra = run(a, 100, 330.0f, 0.6f);
    const Stats rb = run(b, 100, 330.0f, 0.6f);
    bool same = ra.y.size() == rb.y.size();
    for (std::size_t i = 0; same && i < ra.y.size(); ++i)
        if (ra.y[i] != rb.y[i]) same = false;
    EXPECT(same);
}

void testInGraph() {
    SignalGraph g;
    const auto osc = g.add(std::make_unique<Oscillator>());
    g.node(osc).setParameter("freq", 110.0f);
    const auto w = g.add(std::make_unique<Wasp>());
    g.node(w).setParameter("cutoff", 900.0f);
    g.node(w).setParameter("resonance", 0.6f);
    const auto mst = g.add(std::make_unique<Master>());
    g.connect(osc, 2, w, 0);       // saw -> WASP.in
    g.connect(w, 0, mst, 0);       // WASP.out -> MASTER
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float lo = 1e9f, hi = -1e9f;
    for (int b = 0; b < 2000; ++b) {
        g.process(out, mst, 0);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            lo = std::min(lo, out.at(0, k)); hi = std::max(hi, out.at(0, k));
        }
    }
    check(hi > 0.05f && lo < -0.05f, "WASP filtra a voz no grafo");
}

void testPanel() {
    Wasp w;
    const std::string problem = validatePanel(w);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(w.panel().widgets.size() >= 11);
    std::cout << renderAscii(w);
}

}  // namespace

int main() {
    testLowPass();
    testHighPass();
    testModeSweep();
    testResonancePeak();
    testSelfOscillates();
    testGritAddsHarmonics();
    testBiasAsymmetryAndDc();
    testDriveNoBlowup();
    testCutoffMod();
    testFiniteExtreme();
    testAliasReduced();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_wasp: OK\n";
    return g_failures == 0 ? 0 : 1;
}
