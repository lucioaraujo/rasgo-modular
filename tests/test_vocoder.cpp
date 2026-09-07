// Teste isolado do Módulo 60 (VOCODER — vocoder de N bandas).
// Critérios do dossiê `dossies/60_vocoder.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Vocoder.hpp"
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

std::vector<float> run(Vocoder& v, int blocks, const Gen& carrier,
                       const Gen& mod, const Gen& pitch = nullptr) {
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bc(kSr, 1, kB), bm(kSr, 1, kB), bp(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            if (carrier) bc.at(0, k) = carrier(n + k);
            if (mod) bm.at(0, k) = mod(n + k);
            if (pitch) bp.at(0, k) = pitch(n + k);
        }
        std::vector<const AudioBlock*> ins{
            carrier ? &bc : nullptr, mod ? &bm : nullptr,
            pitch ? &bp : nullptr};
        v.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
        n += kB;
    }
    return r;
}

double band(const std::vector<float>& x, double f0, double f1, std::size_t a) {
    double s = 0.0;
    for (double hz = f0; hz <= f1; hz += 20.0) {
        double re = 0, im = 0, ws = 0;
        const double wk = 2.0 * M_PI * hz / kSr;
        for (std::size_t i = a; i < x.size(); ++i) {
            const double t = (double)(i - a);
            const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * t
                             / (double)(x.size() - a - 1));
            re += w * x[i] * std::cos(wk * (double)i);
            im += w * x[i] * std::sin(wk * (double)i);
            ws += w;
        }
        s += std::sqrt(re * re + im * im) / ws;
    }
    return s;
}
double rms(const std::vector<float>& x, std::size_t a) {
    double s = 0.0; std::size_t c = 0;
    for (std::size_t i = a; i < x.size(); ++i, ++c) s += x[i] * (double)x[i];
    return std::sqrt(s / std::max<std::size_t>(1, c));
}

Gen noise(float amp) {
    auto s = std::make_shared<std::uint64_t>(0xC0FFEEu);
    return [s, amp](std::size_t) {
        *s ^= *s << 13; *s ^= *s >> 7; *s ^= *s << 17;
        return amp * static_cast<float>(static_cast<std::int32_t>(*s >> 32))
             / 2147483648.0f;
    };
}
Gen sine(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        return amp * std::sin(6.2831853 * hz * (double)n / kSr);
    };
}
Gen saw(double hz, float amp) {
    return [hz, amp](std::size_t n) {
        const double ph = std::fmod((double)n * hz / kSr, 1.0);
        return amp * static_cast<float>(2.0 * ph - 1.0);
    };
}
// "sílabas": ruído com envelope de 4 Hz
Gen syllables(float amp) {
    auto s = std::make_shared<std::uint64_t>(0xBEEF01u);
    return [s, amp](std::size_t n) {
        *s ^= *s << 13; *s ^= *s >> 7; *s ^= *s << 17;
        const float wn = static_cast<float>(static_cast<std::int32_t>(*s >> 32))
                       / 2147483648.0f;
        const double t = (double)n / kSr;
        const float env = (std::fmod(t, 0.25) < 0.12) ? 1.0f : 0.03f;
        return amp * wn * env;
    };
}

// ---- testes ----

void testShapesCarrierSpectrum() {
    // portadora de espectro PLANO (ruído), modulador = seno em 1200 Hz →
    // a saída concentra a energia na banda do modulador
    Vocoder v;
    v.setParameter("mix", 1.0f);
    v.setParameter("sibilance", 0.0f);
    const auto y = run(v, 400, noise(0.5f), sine(1200.0, 0.6f));
    const double on = band(y, 1050.0, 1400.0, 60000);
    const double lo = band(y, 300.0, 550.0, 60000);
    const double hi = band(y, 2800.0, 3400.0, 60000);
    EXPECT(on > 0.05);
    EXPECT(on > 2.5 * lo);
    EXPECT(on > 2.5 * hi);
}

void testTracksModulatorEnvelope() {
    Vocoder v;
    v.setParameter("mix", 1.0f);
    const auto y = run(v, 400, saw(110.0, 0.5f), syllables(0.5f));
    double loud = 0.0, quiet = 0.0; std::size_t nl = 0, nq = 0;
    for (std::size_t i = y.size() / 3; i < y.size(); ++i) {
        const double t = (double)i / kSr;
        if (std::fmod(t, 0.25) < 0.12) { loud += y[i] * (double)y[i]; ++nl; }
        else { quiet += y[i] * (double)y[i]; ++nq; }
    }
    const double lr = std::sqrt(loud / std::max<std::size_t>(1, nl));
    const double qr = std::sqrt(quiet / std::max<std::size_t>(1, nq));
    EXPECT(lr > 0.02);
    EXPECT(lr > 2.0 * qr);
}

void testShiftMovesFormants() {
    // modulador com energia em 800 Hz; SHIFT positivo sobe a re-síntese
    auto e = [](float sh, double f0, double f1) {
        Vocoder v;
        v.setParameter("mix", 1.0f);
        v.setParameter("sibilance", 0.0f);
        v.setParameter("shift", sh);
        const auto y = run(v, 400, noise(0.5f), sine(800.0, 0.6f));
        return band(y, f0, f1, 60000);
    };
    EXPECT(e(0.0f, 650.0, 950.0) > 0.05);          // sem shift: fica ~800
    EXPECT(e(0.7f, 1200.0, 1900.0) > e(0.0f, 1200.0, 1900.0) * 1.5f);
}

void testFreezeHolds() {
    Vocoder v;
    v.setParameter("mix", 1.0f);
    v.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bc(kSr, 1, kB), bm(kSr, 1, kB);
    const Gen car = saw(110.0, 0.5f);
    const Gen md = sine(1000.0, 0.6f);
    std::size_t n = 0;
    for (int b = 0; b < 150; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bc.at(0, k) = car(n + k); bm.at(0, k) = md(n + k);
        }
        std::vector<const AudioBlock*> ins{&bc, &bm, nullptr};
        v.process(ins, out);
        n += kB;
    }
    v.setParameter("freeze", 1.0f);
    std::vector<float> yf;
    for (int b = 0; b < 150; ++b) {
        for (std::size_t k = 0; k < kB; ++k) bc.at(0, k) = car(n + k);
        std::vector<const AudioBlock*> ins{&bc, nullptr, nullptr};  // sem mod
        v.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) yf.push_back(out[0].at(0, k));
        n += kB;
    }
    EXPECT(rms(yf, 5000) > 0.02);                   // continua soando
    EXPECT(band(yf, 850.0, 1200.0, 5000) > 0.03);   // ainda ~1000 Hz
}

void testAutonomousCarrier() {
    Vocoder v;
    v.setParameter("mix", 1.0f);
    const auto y = run(v, 300, nullptr, syllables(0.5f));   // sem carrier
    EXPECT(rms(y, 30000) > 0.02);                   // a serra interna fala
    for (float x : y) EXPECT(std::isfinite(x) && std::fabs(x) < 1.05f);
}

void testMixBypass() {
    Vocoder v;
    v.setParameter("mix", 0.0f);      // portadora seca
    const auto y = run(v, 60, sine(440.0, 0.4f), syllables(0.5f));
    for (std::size_t i = 8000; i < y.size(); ++i)
        EXPECT(std::fabs(y[i] - 0.4f * std::sin(6.2831853 * 440.0
               * (double)i / kSr)) < 1e-4f);
}

void testDeterminism() {
    auto mk = [](Vocoder& v) {
        v.setParameter("bands", 12.0f);
        v.setParameter("shift", 0.3f);
    };
    Vocoder a; mk(a);
    Vocoder b; mk(b);
    const auto ra = run(a, 200, saw(140.0, 0.5f), syllables(0.4f));
    const auto rb = run(b, 200, saw(140.0, 0.5f), syllables(0.4f));
    const auto la = run(a, 200, nullptr, syllables(0.4f));
    const auto lb = run(b, 200, nullptr, syllables(0.4f));
    bool same = ra.size() == rb.size() && la.size() == lb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = (ra[i] == rb[i]) && (la[i] == lb[i]);
    EXPECT(same);
}

void testBounded() {
    Vocoder v;
    v.setParameter("bands", 20.0f);
    v.setParameter("shift", 1.0f);
    v.setParameter("sibilance", 1.0f);
    v.setParameter("attack", 0.0f);
    const auto y = run(v, 200, noise(1.0f), noise(1.0f));
    for (float x : y) EXPECT(std::isfinite(x) && std::fabs(x) < 1.1f);
}

void testPanel() {
    Vocoder v;
    check(validatePanel(v).empty(), "painel VOCODER fecha");
    std::cout << renderAscii(v);
}

}  // namespace

int main() {
    testShapesCarrierSpectrum();
    testTracksModulatorEnvelope();
    testShiftMovesFormants();
    testFreezeHolds();
    testAutonomousCarrier();
    testMixBypass();
    testDeterminism();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular VOCODER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
