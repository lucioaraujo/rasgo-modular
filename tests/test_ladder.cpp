// LADDER (#74) — o filtro do grave. Critérios do dossiê `dossies/74_ladder.md`.

#include "dsp/Ladder.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 256;
constexpr double kPi = 3.14159265358979323846;

struct Setup { float cut, res, drive, comp, diode; };

void configure(Ladder& m, const Setup& s) {
    m.setParameter("cutoff", s.cut);
    m.setParameter("resonance", s.res);
    m.setParameter("drive", s.drive);
    m.setParameter("comp", s.comp);
    m.setParameter("diode", s.diode);
    m.prepare(kSr, kB);
}

// roda `gen(n)` pela LADDER e devolve LP4 (e LP2 em `lp2`)
template <typename Gen>
std::vector<float> run(Ladder& m, const std::size_t n, Gen&& gen, std::vector<float>* lp2 = nullptr) {
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock in(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    std::vector<float> y;
    y.reserve(n);
    for (std::size_t done = 0; done < n; done += kB) {
        for (std::size_t i = 0; i < kB; ++i) in.at(0, i) = gen(done + i);
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            y.push_back(out[0].at(0, i));
            if (lp2) lp2->push_back(out[1].at(0, i));
        }
    }
    return y;
}

double rms(const std::vector<float>& v, std::size_t from) {
    double a = 0.0;
    for (std::size_t i = from; i < v.size(); ++i) a += double(v[i]) * v[i];
    return std::sqrt(a / double(v.size() - from));
}

double mean(const std::vector<float>& v, std::size_t from) {
    double a = 0.0;
    for (std::size_t i = from; i < v.size(); ++i) a += v[i];
    return a / double(v.size() - from);
}

// frequência por cruzamentos de zero ascendentes (interpolados)
double zcFreq(const std::vector<float>& v, std::size_t from) {
    double first = -1.0, last = -1.0;
    int n = 0;
    for (std::size_t i = from + 1; i < v.size(); ++i)
        if (v[i - 1] < 0.0f && v[i] >= 0.0f) {
            const double t = double(i - 1) + v[i - 1] / double(v[i - 1] - v[i]);
            if (first < 0.0) first = t; else ++n;
            last = t;
        }
    return n > 0 ? kSr * n / (last - first) : 0.0;
}

// amplitude da componente em `hz` (DFT de um ponto)
double toneAmp(const std::vector<float>& v, std::size_t from, double hz) {
    double re = 0.0, im = 0.0;
    const std::size_t n = v.size() - from;
    for (std::size_t i = 0; i < n; ++i) {
        const double ph = 2.0 * kPi * hz * double(i) / kSr;
        re += v[from + i] * std::cos(ph);
        im += v[from + i] * std::sin(ph);
    }
    return 2.0 * std::sqrt(re * re + im * im) / double(n);
}

void testCriticalTable() {
    const auto c0 = Ladder::critical(0.0f);
    check(std::fabs(c0.k - 4.0f) < 0.01f && std::fabs(c0.ratio - 1.0f) < 0.01f,
          "transistor: k_crit = 4 e pico no corte");
    const auto c1 = Ladder::critical(1.0f);
    std::printf("k_crit: c=0 %.3f (razão %.3f) · c=0,5 %.3f (%.3f) · c=1 %.3f (%.3f)\n",
                c0.k, c0.ratio, Ladder::critical(0.5f).k, Ladder::critical(0.5f).ratio,
                c1.k, c1.ratio);
    check(c1.k > c0.k, "diodo: o acoplamento pede mais laço para auto-oscilar");
}

// COMP = 1: ganho em DC 1 com qualquer RESO; COMP = 0: 1/(1+k)
void testBassCompensation() {
    for (const float diode : {0.0f, 1.0f}) {
        Ladder m;
        configure(m, {800.0f, 0.85f, 0.0f, 1.0f, diode});
        const auto y = run(m, 48000, [](std::size_t) { return 0.05f; });
        const double g = mean(y, 24000) / 0.05;
        Ladder m0;
        configure(m0, {800.0f, 0.85f, 0.0f, 0.0f, diode});
        const auto y0 = run(m0, 48000, [](std::size_t) { return 0.05f; });
        const double g0 = mean(y0, 24000) / 0.05;
        const double k = (0.85 + 0.25 * std::pow(0.85, 12.0)) * Ladder::critical(diode).k;
        std::printf("DC, DIODO %.0f: COMP 1 → %.3f · COMP 0 → %.3f (1/(1+k) = %.3f)\n",
                    diode, g, g0, 1.0 / (1.0 + k));
        check(std::fabs(g - 1.0) < 0.03, "COMP 1: o grave volta a ganho 1");
        check(std::fabs(g0 - 1.0 / (1.0 + k)) < 0.03, "COMP 0: o grave cai para 1/(1+k)");
    }
}

// RESO = 1 auto-oscila, afinado no CUT em qualquer DIODO, e não explode
void testSelfOscillationTuned() {
    for (const float diode : {0.0f, 0.5f, 1.0f})
        for (const float cut : {110.0f, 440.0f, 1760.0f}) {
            Ladder m;
            configure(m, {cut, 1.0f, 0.0f, 0.0f, diode});
            const auto y = run(m, 96000, [](std::size_t i) { return i < 64 ? 0.5f : 0.0f; });
            const double f = zcFreq(y, 48000);
            const double a = rms(y, 48000);
            const double cents = 1200.0 * std::log2(f / cut);
            std::printf("auto-oscilação DIODO %.1f CUT %4.0f: %.1f Hz (%+.1f cents), rms %.3f\n",
                        diode, cut, f, cents, a);
            check(a > 0.02, "RESO 1: auto-oscila");
            check(std::fabs(cents) < 25.0, "RESO 1: o pico fica afinado no CUT");
            float pk = 0.0f;
            for (const float v : y) pk = std::max(pk, std::fabs(v));
            check(pk <= 1.0f, "auto-oscilação limitada");
        }
}

// abaixo do limiar o filtro não oscila sozinho
void testBelowThresholdDecays() {
    Ladder m;
    configure(m, {440.0f, 0.9f, 0.0f, 0.0f, 0.6f});
    const auto y = run(m, 96000, [](std::size_t i) { return i < 64 ? 0.5f : 0.0f; });
    check(rms(y, 72000) < 1.0e-4, "RESO 0,9: a ressonância decai");
}

// LP4 cai 24 dB/oct, LP2 12 dB/oct (RESO 0): duas oitavas acima do corte
void testSlopes() {
    Ladder m;
    std::vector<float> lp2;
    configure(m, {500.0f, 0.0f, 0.0f, 0.0f, 0.0f});
    const auto y = run(m, 48000, [](std::size_t i) {
        return 0.05f * float(std::sin(2.0 * kPi * 2000.0 * double(i) / kSr)); }, &lp2);
    const double a4 = 20.0 * std::log10(toneAmp(y, 24000, 2000.0) / 0.05);
    const double a2 = 20.0 * std::log10(toneAmp(lp2, 24000, 2000.0) / 0.05);
    std::printf("2 oitavas acima do corte: LP4 %.1f dB, LP2 %.1f dB\n", a4, a2);
    check(a4 < -44.0 && a4 > -52.0, "LP4 ≈ −48 dB a duas oitavas (24 dB/oct)");
    check(a2 < -21.0 && a2 > -27.0, "LP2 ≈ −24 dB a duas oitavas (12 dB/oct)");
}

// DRIVE alto num seno agudo: o que não é harmônico (alias) fica baixo
void testAliasing() {
    Ladder m;
    configure(m, {18000.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    const double f0 = 4700.0;
    const auto y = run(m, 48000, [&](std::size_t i) {
        return 0.8f * float(std::sin(2.0 * kPi * f0 * double(i) / kSr)); });
    const std::size_t from = 8000;
    const double fund = toneAmp(y, from, f0);
    // alias dos ímpares: 3f, 5f… dobrados abaixo de Nyquist
    double worst = 0.0;
    for (int h = 3; h <= 9; h += 2) {
        double fa = std::fmod(h * f0, double(kSr));
        if (fa > kSr / 2) fa = kSr - fa;
        if (std::fabs(fa - f0) < 50.0 || h * f0 < kSr / 2) continue;
        worst = std::max(worst, toneAmp(y, from, fa));
    }
    const double db = 20.0 * std::log10(worst / fund);
    std::printf("alias pior com DRIVE 1 (seno 4,7 kHz): %.1f dB\n", db);
    check(db < -35.0, "DRIVE 1: alias abaixo de −35 dB");
}

// varredura dura: nada não finito, nada acima de 1
void testStabilitySweep() {
    std::uint32_t r = 12345u;
    bool ok = true;
    for (int s = 0; s < 40; ++s) {
        Ladder m;
        auto rnd = [&] { r = r * 1664525u + 1013904223u; return float(r >> 8) / 16777216.0f; };
        configure(m, {20.0f + 19980.0f * rnd() * rnd(), rnd(), rnd(), rnd(), rnd()});
        const auto y = run(m, 4096, [&](std::size_t) { return 4.0f * (rnd() - 0.5f); });
        for (const float v : y) if (!std::isfinite(v) || std::fabs(v) > 1.0f) ok = false;
    }
    check(ok, "varredura: finito e limitado");
}

}  // namespace

int main() {
    testCriticalTable();
    testBassCompensation();
    testSelfOscillationTuned();
    testBelowThresholdDecays();
    testSlopes();
    testAliasing();
    testStabilitySweep();
    if (g_failures == 0) std::printf("test_ladder: ok\n");
    return g_failures == 0 ? 0 : 1;
}
