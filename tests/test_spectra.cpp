// Teste isolado do Módulo 57 (SPECTRA — resíntese espectral).
// Critérios do dossiê `dossies/57_spectra.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Spectra.hpp"
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

std::vector<float> run(Spectra& s, int blocks, const Gen& in, int outIdx = 0,
                       const Gen& pitch = nullptr) {
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bp(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            if (in) bi.at(0, k) = in(n + k);
            if (pitch) bp.at(0, k) = pitch(n + k);
        }
        std::vector<const AudioBlock*> ins{
            in ? &bi : nullptr, pitch ? &bp : nullptr, nullptr};
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, k));
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < b && i < v.size(); ++i, ++c) s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
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

// energia numa banda (soma de |DFT| a passos de 4 Hz) — robusto ao
// pequeno erro de frequência da análise por banco
double band(const std::vector<float>& x, double f0, double f1,
            std::size_t a, std::size_t b) {
    double s = 0.0;
    for (double hz = f0; hz <= f1; hz += 4.0) s += magAt(x, hz, a, b);
    return s;
}

Gen sine(double hz, float amp = 0.7f) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(6.2831853 * hz * (double)n / kSr);
    };
}

// ---- testes ----

void testTracksSinglePartial() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    s.setParameter("voices", 8.0f);
    s.setParameter("blur", 0.2f);
    const auto y = run(s, 400, sine(330.0), 0);
    const double on = band(y, 300.0, 360.0, 60000, 100000);
    const double off = band(y, 600.0, 720.0, 60000, 100000)
                     + band(y, 120.0, 200.0, 60000, 100000);
    EXPECT(on > 0.08);                       // re-sintetiza perto de 330
    EXPECT(on > 4.0 * off + 1e-6);           // e concentra a energia lá
    EXPECT(rms(y, 60000, 100000) > 0.08);
}

void testTracksMultiplePartials() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    s.setParameter("voices", 10.0f);
    const Gen g = [](std::size_t n) {
        const double t = (double)n / kSr;
        return 0.4f * std::sin(6.2831853 * 200.0 * t)
             + 0.3f * std::sin(6.2831853 * 400.0 * t)
             + 0.25f * std::sin(6.2831853 * 600.0 * t);
    };
    const auto y = run(s, 400, g, 0);
    EXPECT(band(y, 180.0, 220.0, 60000, 100000) > 0.01);
    EXPECT(band(y, 380.0, 420.0, 60000, 100000) > 0.01);
    EXPECT(band(y, 580.0, 620.0, 60000, 100000) > 0.01);
    // pouca energia entre os parciais (não é ruído de banda larga)
    EXPECT(band(y, 900.0, 1100.0, 60000, 100000)
           < 0.6 * band(y, 380.0, 420.0, 60000, 100000));
}

void testShiftTransposes() {
    auto e = [](float sh, double f0, double f1) {
        Spectra s;
        s.setParameter("mix", 1.0f);
        s.setParameter("voices", 6.0f);
        s.setParameter("shift", sh);
        const auto y = run(s, 400, sine(300.0), 0);
        return band(y, f0, f1, 60000, 100000);
    };
    EXPECT(e(0.0f, 280.0, 320.0) > 0.05);          // sem shift: fica em 300
    EXPECT(e(1.0f, 560.0, 640.0) > 0.05);          // +1 oct: sobe pra ~600
    EXPECT(e(1.0f, 280.0, 320.0) < 0.4 * e(0.0f, 280.0, 320.0));
}

void testPitchCvTransposes() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    s.setParameter("voices", 6.0f);
    const auto y = run(s, 400, sine(250.0), 0,
                       [](std::size_t) { return 1.0f; });   // +1 oct por CV
    EXPECT(band(y, 460.0, 540.0, 60000, 100000) > 0.05);
    EXPECT(band(y, 230.0, 270.0, 60000, 100000)
           < band(y, 460.0, 540.0, 60000, 100000));
}

void testBlurSmearsAttack() {
    // blur alto = os alvos deslizam devagar → energia demora a assentar
    auto settle = [](float blur) {
        Spectra s;
        s.setParameter("mix", 1.0f);
        s.setParameter("voices", 6.0f);
        s.setParameter("blur", blur);
        const auto y = run(s, 500, sine(360.0), 0);
        const double early = rms(y, 3000, 12000);
        const double late = rms(y, 90000, 120000);
        return late > 1e-6 ? early / late : 1.0;
    };
    EXPECT(settle(0.9f) < 0.6 * settle(0.05f) + 1e-6);
}

void testFreezeHoldsAfterInputGone() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    s.setParameter("voices", 6.0f);
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB);
    // 150 blocos com um tom em 350 Hz
    for (int b = 0; b < 150; ++b) {
        for (std::size_t k = 0; k < kB; ++k)
            bi.at(0, k) = 0.7f * std::sin(6.2831853 * 350.0
                          * (double)(b * (int)kB + (int)k) / kSr);
        std::vector<const AudioBlock*> ins{&bi, nullptr, nullptr};
        s.process(ins, out);
    }
    // congela e TIRA a entrada
    s.setParameter("freeze", 1.0f);
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr};
    std::vector<float> yf;
    for (int b = 0; b < 150; ++b) {
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) yf.push_back(out[0].at(0, k));
    }
    EXPECT(rms(yf, 10000, 35000) > 0.05);                 // continua soando
    EXPECT(band(yf, 330.0, 370.0, 10000, 35000) > 0.05);  // ainda em ~350
}

void testAutonomousDrone() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    const auto y = run(s, 400, nullptr, 0);   // sem entrada
    EXPECT(rms(y, 60000, 100000) > 0.01);     // soa ao carregar
    for (float v : y) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testStereoSpread() {
    Spectra s;
    s.setParameter("mix", 1.0f);
    s.setParameter("voices", 8.0f);
    const auto l = run(s, 200, sine(300.0), 0);
    const auto rr = run(s, 200, sine(300.0), 1);
    double d = 0.0;
    for (std::size_t i = 0; i < l.size() && i < rr.size(); ++i)
        d += std::fabs(l[i] - rr[i]);
    EXPECT(d > 1.0);   // L ≠ R (vozes ímpares/pares panoramizadas)
}

void testMixBypass() {
    Spectra s;
    s.setParameter("mix", 0.0f);       // 100% seco
    const auto y = run(s, 60, sine(440.0, 0.5f), 0);
    for (std::size_t i = 30000; i < y.size(); ++i)
        EXPECT(std::fabs(y[i] - 0.5f * std::sin(6.2831853 * 440.0
               * (double)i / kSr)) < 1e-4f);
}

void testDeterminism() {
    auto mk = [](Spectra& s) {
        s.setParameter("voices", 7.0f);
        s.setParameter("jitter", 0.0f);
        s.setParameter("blur", 0.3f);
    };
    Spectra a; mk(a);
    Spectra b; mk(b);
    const auto ra = run(a, 200, sine(277.0), 0);
    const auto rb = run(b, 200, sine(277.0), 0);
    const auto la = run(a, 200, nullptr, 0);   // autônomo também determinístico
    const auto lb = run(b, 200, nullptr, 0);
    bool same = ra.size() == rb.size() && la.size() == lb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = (ra[i] == rb[i]) && (la[i] == lb[i]);
    EXPECT(same);
}

void testBounded() {
    Spectra s;
    s.setParameter("voices", 24.0f);
    s.setParameter("shift", 2.0f);
    s.setParameter("stretch", 1.0f);
    s.setParameter("tone", 1.0f);
    s.setParameter("jitter", 1.0f);
    const Gen g = [](std::size_t n) {
        const double t = (double)n / kSr;
        return 0.9f * (std::sin(6.2831853 * 110.0 * t)
                     + std::sin(6.2831853 * 550.0 * t)
                     + 0.5 * std::sin(6.2831853 * 3300.0 * t));
    };
    const auto y = run(s, 200, g, 0);
    for (float v : y) EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
}

void testPanel() {
    Spectra s;
    check(validatePanel(s).empty(), "painel SPECTRA fecha");
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testTracksSinglePartial();
    testTracksMultiplePartials();
    testShiftTransposes();
    testPitchCvTransposes();
    testBlurSmearsAttack();
    testFreezeHoldsAfterInputGone();
    testAutonomousDrone();
    testStereoSpread();
    testMixBypass();
    testDeterminism();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SPECTRA tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
