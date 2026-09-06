// Teste do DelayPitchShifter (src/dsp/PitchShift.hpp) — o porte do
// G09.pitchshift.pd via Navalha 2. Ver `dossies/ESTUDO_audio_sampling §2`.

#include "dsp/PitchShift.hpp"

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

std::vector<float> shiftSine(float freq, float ratio, int n) {
    DelayPitchShifter ps;
    ps.prepare(kSr);
    ps.setRatio(ratio);
    std::vector<float> r(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float in = 0.6f * std::sin(2.0 * M_PI * freq * i / kSr);
        r[static_cast<std::size_t>(i)] = ps.process(in);
    }
    return r;
}

// magnitude na freq `hz`, DFT com janela de Hann na 2ª metade (pula o
// transiente e mata o vazamento de janela retangular)
double magAt(const std::vector<float>& x, double hz) {
    const std::size_t a = x.size() / 2;
    const std::size_t n = x.size() - a;
    double re = 0.0, im = 0.0, ws = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    for (std::size_t i = a; i < x.size(); ++i) {
        const double t = static_cast<double>(i - a);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * t / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        ws += w;
    }
    return std::sqrt(re * re + im * im) / ws;
}

// varre uma faixa e devolve a freq do pico
double peakFreq(const std::vector<float>& x, double lo, double hi) {
    double bf = lo, bm = -1.0;
    for (double f = lo; f <= hi; f += 2.0) {
        const double m = magAt(x, f);
        if (m > bm) { bm = m; bf = f; }
    }
    return bf;
}

void testUpOctave() {
    const auto r = shiftSine(200.0f, 2.0f, 48000);
    EXPECT(std::fabs(peakFreq(r, 300.0, 500.0) - 400.0) < 12.0);
    EXPECT(magAt(r, 400.0) > 5.0 * magAt(r, 200.0));
    EXPECT(magAt(r, 400.0) > 0.1);
}

void testDownOctave() {
    const auto r = shiftSine(400.0f, 0.5f, 48000);
    EXPECT(std::fabs(peakFreq(r, 150.0, 300.0) - 200.0) < 8.0);
    EXPECT(magAt(r, 200.0) > 5.0 * magAt(r, 400.0));
    EXPECT(magAt(r, 200.0) > 0.1);
}

void testFifthUp() {
    // +7 semitons ≈ ×1,4983
    const auto r = shiftSine(200.0f, 1.4983f, 48000);
    EXPECT(std::fabs(peakFreq(r, 250.0, 360.0) - 300.0) < 10.0);
}

void testUnityKeepsPitch() {
    const auto r = shiftSine(300.0f, 1.0f, 48000);
    EXPECT(magAt(r, 300.0) > 5.0 * magAt(r, 600.0));
    EXPECT(magAt(r, 300.0) > 0.15);
}

void testClampAndBounded() {
    DelayPitchShifter ps;
    ps.prepare(kSr);
    ps.setRatio(100.0f);   // clampa em 4
    for (int i = 0; i < 20000; ++i) {
        const float y = ps.process(0.9f * std::sin(0.02 * i));
        EXPECT(std::isfinite(y) && std::fabs(y) < 2.0f);
    }
}

void testDeterminism() {
    const auto a = shiftSine(300.0f, 1.5f, 12000);
    const auto b = shiftSine(300.0f, 1.5f, 12000);
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i) same = a[i] == b[i];
    EXPECT(same);
}

}  // namespace

int main() {
    testUpOctave();
    testDownOctave();
    testFifthUp();
    testUnityKeepsPitch();
    testClampAndBounded();
    testDeterminism();

    if (g_failures == 0) {
        std::cout << "RASGO Modular pitch shift tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
