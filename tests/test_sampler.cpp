// Teste isolado do Módulo 48 (SAMPLER — matéria gravada como voz).
// Critérios do dossiê `dossies/48_sampler.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Sampler.hpp"
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

struct Ins { Gen trig, in, rec, pos, pitch; };

std::vector<float> run(Sampler& s, int blocks, const Ins& g) {
    s.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bt(kSr, 1, kB), bi(kSr, 1, kB), br(kSr, 1, kB),
        bp(kSr, 1, kB), bpi(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bt.at(0, k) = g.trig ? g.trig(n + k) : 0.0f;
            bi.at(0, k) = g.in ? g.in(n + k) : 0.0f;
            br.at(0, k) = g.rec ? g.rec(n + k) : 0.0f;
            bp.at(0, k) = g.pos ? g.pos(n + k) : 0.0f;
            bpi.at(0, k) = g.pitch ? g.pitch(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.trig ? &bt : nullptr, g.in ? &bi : nullptr, g.rec ? &br : nullptr,
            g.pos ? &bp : nullptr, g.pitch ? &bpi : nullptr};
        s.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
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
Gen recWindow(std::size_t a, std::size_t b) {
    return [a, b](std::size_t n) { return (n >= a && n < b) ? 1.0f : 0.0f; };
}
Gen trigAt(std::size_t at) {
    return [at](std::size_t n) { return (n >= at && n < at + 64) ? 1.0f : 0.0f; };
}

void testRecordThenPlay() {
    Sampler s;
    s.setParameter("speed", 0.0f);   // 1×
    Ins g;
    g.in = sine(200.0, 0.5f);
    g.rec = recWindow(1000, 9000);   // grava 8000 amostras
    g.trig = trigAt(12000);
    const auto r = run(s, 90, g);
    EXPECT(rms(r, 0, 11000) < 1e-4);          // nada antes do disparo
    EXPECT(magAt(r, 200.0, 12500, 18000) > 0.05);   // toca o que gravou
    EXPECT(magAt(r, 200.0, 12500, 18000) > 8.0 * magAt(r, 600.0, 12500, 18000));
}

void testVarispeed() {
    Sampler s;
    s.setParameter("speed", 0.5f);   // 2^(0,5·2) = 2×
    Ins g;
    g.in = sine(200.0, 0.5f);
    g.rec = recWindow(1000, 20000);
    g.trig = trigAt(24000);
    const auto r = run(s, 130, g);
    // 200 Hz gravado, tocado a 2× → 400 Hz
    EXPECT(magAt(r, 400.0, 25000, 35000) > 4.0 * magAt(r, 200.0, 25000, 35000));
    EXPECT(magAt(r, 400.0, 25000, 35000) > 0.03);
}

void testSlices() {
    Sampler s;
    s.setParameter("speed", 0.0f);
    s.setParameter("slices", 2.0f);
    // grava: 4000 amostras de 150 Hz, depois 4000 de 500 Hz
    auto twoTone = [](std::size_t n) -> float {
        if (n < 1000 || n >= 9000) return 0.0f;
        const double t = (double)(n - 1000);
        const double hz = (n - 1000 < 4000) ? 150.0 : 500.0;
        return static_cast<float>(0.5 * std::sin(2.0 * M_PI * hz * t / kSr));
    };
    auto play = [&](float pos) {
        Sampler sp;
        sp.setParameter("speed", 0.0f); sp.setParameter("slices", 2.0f);
        Ins g;
        g.in = twoTone;
        g.rec = recWindow(1000, 9000);
        g.trig = trigAt(12000);
        g.pos = [pos](std::size_t) { return pos; };
        return run(sp, 90, g);
    };
    const auto lo = play(0.0f);    // fatia 0 → 150 Hz
    const auto hi = play(0.99f);   // fatia 1 → 500 Hz
    EXPECT(magAt(lo, 150.0, 12500, 15000) > 3.0 * magAt(lo, 500.0, 12500, 15000));
    EXPECT(magAt(hi, 500.0, 12500, 15000) > 3.0 * magAt(hi, 150.0, 12500, 15000));
}

void testRepitchPreservesDuration() {
    auto play = [](float repitch) {
        Sampler s;
        s.setParameter("speed", 0.0f);
        s.setParameter("repitch", repitch);
        Ins g;
        g.in = sine(200.0, 0.5f);
        g.rec = recWindow(1000, 21000);   // fatia de ~20000 amostras
        g.trig = trigAt(24000);
        g.pitch = [](std::size_t) { return 1.0f; };   // +1 oitava
        return run(s, 160, g);
    };
    const auto varispeed = play(0.0f);   // pitch dobra a velocidade → dura ~10000
    const auto shifted = play(1.0f);     // velocidade solta → dura ~20000
    // ambos com pitch ~400 Hz
    EXPECT(magAt(shifted, 400.0, 25000, 35000) > magAt(shifted, 200.0, 25000, 35000));
    // repitch=1 ainda soando bem depois de 10000 amostras; varispeed já parou
    EXPECT(rms(shifted, 37000, 42000) > 3.0 * rms(varispeed, 37000, 42000));
}

void testLoop() {
    auto tail = [](bool loop) {
        Sampler s;
        s.setParameter("speed", 0.0f);
        s.setParameter("loop", loop ? 1.0f : 0.0f);
        Ins g;
        g.in = sine(300.0, 0.5f);
        g.rec = recWindow(1000, 4000);   // fatia curta (~3000)
        g.trig = trigAt(6000);
        return run(s, 90, g);
    };
    EXPECT(rms(tail(true), 18000, 22000) > 0.05);    // loop: ainda soa
    EXPECT(rms(tail(false), 18000, 22000) < 1e-4);   // one-shot: parou
}

void testWearBounded() {
    auto play = [](float wear) {
        Sampler s;
        s.setParameter("speed", 0.0f);
        s.setParameter("wear", wear);
        Ins g;
        g.in = sine(200.0, 0.5f);
        g.rec = recWindow(1000, 20000);
        g.trig = trigAt(24000);
        return run(s, 130, g);
    };
    const auto clean = play(0.0f);
    const auto worn = play(1.0f);
    // desgaste = bit-crush → mais lixo espectral fora dos 200 Hz
    const double cleanFloor = magAt(clean, 1300.0, 25000, 32000);
    const double wornFloor = magAt(worn, 1300.0, 25000, 32000);
    EXPECT(wornFloor > 2.0 * cleanFloor);
    for (float v : worn) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testDeterminism() {
    auto mk = [](Sampler& s) {
        s.setParameter("speed", 0.3f);
        s.setParameter("wear", 0.5f);
        s.setParameter("repitch", 0.4f);
    };
    Ins g;
    g.in = sine(180.0, 0.5f);
    g.rec = recWindow(500, 12000);
    g.trig = [](std::size_t n) {
        return ((n >= 14000 && n < 14064) || (n >= 26000 && n < 26064)) ? 1.0f : 0.0f;
    };
    g.pitch = [](std::size_t) { return 0.4f; };
    Sampler a; mk(a);
    Sampler b; mk(b);
    const auto ra = run(a, 90, g);
    const auto rb = run(b, 90, g);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testSetBuffer() {
    Sampler s;
    s.setParameter("speed", 0.0f);
    std::vector<float> buf(20000);
    for (std::size_t i = 0; i < buf.size(); ++i)
        buf[i] = 0.5f * std::sin(2.0 * M_PI * 250.0 * (double)i / kSr);
    s.setBuffer(buf, kSr);
    Ins g;
    g.trig = trigAt(2000);
    const auto r = run(s, 60, g);
    EXPECT(magAt(r, 250.0, 3000, 12000) > 0.05);
}

void testNoBuffer() {
    Sampler s;
    Ins g;
    g.trig = trigAt(1000);
    const auto r = run(s, 30, g);
    for (float v : r) EXPECT(v == 0.0f);
}

void testPanel() {
    Sampler s;
    check(validatePanel(s).empty(), "painel SAMPLER fecha");
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testRecordThenPlay();
    testVarispeed();
    testSlices();
    testRepitchPreservesDuration();
    testLoop();
    testWearBounded();
    testDeterminism();
    testSetBuffer();
    testNoBuffer();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SAMPLER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
