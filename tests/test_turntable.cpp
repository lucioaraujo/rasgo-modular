// Teste isolado do Módulo 50 (TURNTABLE — toca-discos com prato de inércia).
// Critérios do dossiê `dossies/50_turntable.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Turntable.hpp"
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
struct Ins { Gen trig, in, rec, scratch, brake; };

std::vector<float> run(Turntable& t, int blocks, const Ins& g) {
    t.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bt(kSr, 1, kB), bi(kSr, 1, kB), br(kSr, 1, kB),
        bs(kSr, 1, kB), bb(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bt.at(0, k) = g.trig ? g.trig(n + k) : 0.0f;
            bi.at(0, k) = g.in ? g.in(n + k) : 0.0f;
            br.at(0, k) = g.rec ? g.rec(n + k) : 0.0f;
            bs.at(0, k) = g.scratch ? g.scratch(n + k) : 0.0f;
            bb.at(0, k) = g.brake ? g.brake(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            g.trig ? &bt : nullptr, g.in ? &bi : nullptr, g.rec ? &br : nullptr,
            g.scratch ? &bs : nullptr, g.brake ? &bb : nullptr};
        t.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
        n += kB;
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * (double)v[i];
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
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
int zc(const std::vector<float>& v, std::size_t a, std::size_t b) {
    int z = 0;
    for (std::size_t i = a + 1; i < b && i < v.size(); ++i)
        if ((v[i - 1] < 0.0f) != (v[i] < 0.0f)) ++z;
    return z;
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

// grava um seno de 200 Hz em [500, 24000) — a gravação TERMINA antes de
// qualquer `trig` (não dá pra tocar e gravar ao mesmo tempo, por design,
// igual ao SAMPLER: `recLen_` só é definido na descida do `rec`).
Ins recorded() {
    Ins g;
    g.in = sine(200.0, 0.5f);
    g.rec = recWindow(500, 24000);
    return g;
}

void testPlaysAtSpeed() {
    Turntable t;
    t.setParameter("speed", 0.0f);   // 1×
    t.setParameter("torque", 1.0f);  // sobe rápido
    Ins g = recorded();
    g.trig = trigAt(25000);
    const auto r = run(t, 240, g);
    EXPECT(magAt(r, 200.0, 30000, 44000) > 0.05);
    EXPECT(magAt(r, 200.0, 30000, 44000) > 6.0 * magAt(r, 400.0, 30000, 44000));
}

void testTorqueWow() {
    auto play = [](float torque) {
        Turntable t;
        t.setParameter("speed", 0.0f);
        t.setParameter("torque", torque);
        Ins g = recorded();
        g.trig = trigAt(25000);
        return run(t, 340, g);
    };
    const auto slow = play(0.05f);   // motor fraco → sobe devagar
    const auto fast = play(1.0f);
    // torque baixo: o prato ainda está subindo quando a agulha cai; a
    // nota começa grave e sobe (mais cruzamentos de zero mais tarde).
    const int early = zc(slow, 25500, 29500);
    const int late = zc(slow, 58000, 62000);
    EXPECT(late > 2 * early + 10);
    // torque alto: já está na velocidade quase de imediato
    EXPECT(zc(fast, 25500, 29500) > 3 * early);
}

void testBrakeStops() {
    Turntable t;
    t.setParameter("speed", 0.0f);
    t.setParameter("torque", 1.0f);
    t.setParameter("friction", 0.8f);
    Ins g = recorded();
    g.trig = trigAt(25000);
    g.brake = [](std::size_t n) { return n >= 40000 ? 1.0f : 0.0f; };
    const auto r = run(t, 260, g);
    EXPECT(rms(r, 30000, 38000) > 0.05);         // tocando antes do freio
    EXPECT(rms(r, 52000, 62000) < 0.02);         // parou depois (AC-coupled)
    // e a altura desceu no caminho (ZC caindo)
    EXPECT(zc(r, 40200, 42200) > zc(r, 44000, 46000));
}

void testScratchSmears() {
    Turntable clean;
    clean.setParameter("speed", 0.0f); clean.setParameter("torque", 1.0f);
    clean.setParameter("start", 0.4f);
    Ins gc = recorded();
    gc.trig = trigAt(25000);
    const auto rc = run(clean, 240, gc);

    Turntable scr;
    scr.setParameter("speed", 0.0f); scr.setParameter("torque", 1.0f);
    scr.setParameter("grab", 1.0f); scr.setParameter("start", 0.4f);
    Ins gs = recorded();
    gs.trig = trigAt(25000);
    gs.scratch = [](std::size_t n) {
        return (n >= 25000)
            ? 0.6f * (float)std::sin(2.0 * M_PI * 3.0 * (double)n / kSr)
            : 0.0f;
    };
    const auto rs = run(scr, 240, gs);
    // scratch modula muito a altura → o pico de 200 Hz borra
    EXPECT(magAt(rs, 200.0, 30000, 55000) < 0.4 * magAt(rc, 200.0, 30000, 55000));
    EXPECT(rms(rs, 30000, 55000) > 0.02);   // mas ainda tem som
    for (float v : rs) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testPitchBend() {
    Turntable t;
    t.setParameter("speed", 0.0f); t.setParameter("torque", 1.0f);
    t.setParameter("grab", 1.0f); t.setParameter("start", 0.5f);
    Ins g = recorded();
    g.trig = trigAt(25000);
    // empurrãozinho pra trás entre 30000 e 45000 (beatmatch)
    g.scratch = [](std::size_t n) {
        return (n >= 30000 && n < 45000) ? -0.1f : 0.0f;
    };
    const auto r = run(t, 260, g);
    // durante o nudge a altura desce; depois volta
    const int during = zc(r, 33000, 40000);
    const int after = zc(r, 50000, 57000);
    EXPECT(during < after);
    EXPECT(after > 0);
}

void testReverse() {
    Turntable t;
    t.setParameter("speed", -1.0f);  // −2× (disco pra trás)
    t.setParameter("torque", 1.0f);
    t.setParameter("start", 0.7f);   // agulha adiantada, pra ter pra onde recuar
    Ins g = recorded();
    g.trig = trigAt(25000);
    const auto r = run(t, 260, g);
    EXPECT(rms(r, 30000, 40000) > 0.03);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testWearBounded() {
    auto play = [](float wear) {
        Turntable t;
        t.setParameter("speed", 0.0f); t.setParameter("torque", 1.0f);
        t.setParameter("wear", wear);
        Ins g = recorded();
        g.trig = trigAt(25000);
        return run(t, 260, g);
    };
    // energia entre as parciais (o tom é 200 Hz; 1500–2300 Hz seria vazio)
    auto hiBand = [](const std::vector<float>& r) {
        double s = 0.0;
        for (double hz : {1500.0, 1700.0, 1900.0, 2100.0, 2300.0})
            s += magAt(r, hz, 30000, 55000);
        return s;
    };
    const auto clean = play(0.0f);
    const auto worn = play(1.0f);
    // os estalos de vinil enchem essa banda que estava vazia
    EXPECT(hiBand(worn) > 5.0 * hiBand(clean) + 1e-4);
    for (float v : worn) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testDeterminism() {
    auto mk = [](Turntable& t) {
        t.setParameter("speed", 0.2f); t.setParameter("torque", 0.5f);
        t.setParameter("wear", 0.6f); t.setParameter("grab", 0.8f);
    };
    Ins g = recorded();
    g.trig = trigAt(25000);
    g.scratch = [](std::size_t n) {
        return 0.3f * (float)std::sin(0.001 * (double)n);
    };
    Turntable a; mk(a);
    Turntable b; mk(b);
    const auto ra = run(a, 200, g);
    const auto rb = run(b, 200, g);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testLoop() {
    Turntable t;
    t.setParameter("speed", 0.0f); t.setParameter("torque", 1.0f);
    t.setParameter("loop", 1.0f);
    Ins g;
    g.in = sine(300.0, 0.5f);
    g.rec = recWindow(500, 8000);   // groove curto (~7500)
    g.trig = trigAt(10000);
    const auto r = run(t, 200, g);
    EXPECT(rms(r, 30000, 40000) > 0.05);   // groove travado, ainda soando
}

void testSetBufferAndNoBuffer() {
    Turntable empty;
    Ins g;
    g.trig = trigAt(1000);
    const auto re = run(empty, 40, g);
    for (float v : re) EXPECT(v == 0.0f);

    Turntable t;
    t.setParameter("speed", 0.0f); t.setParameter("torque", 1.0f);
    std::vector<float> buf(60000);
    for (std::size_t i = 0; i < buf.size(); ++i)
        buf[i] = 0.5f * std::sin(2.0 * M_PI * 250.0 * (double)i / kSr);
    t.setBuffer(buf, kSr);
    const auto r = run(t, 200, g);
    EXPECT(magAt(r, 250.0, 8000, 30000) > 0.05);
}

void testPanel() {
    Turntable t;
    check(validatePanel(t).empty(), "painel TURNTABLE fecha");
    std::cout << renderAscii(t);
}

}  // namespace

int main() {
    testPlaysAtSpeed();
    testTorqueWow();
    testBrakeStops();
    testScratchSmears();
    testPitchBend();
    testReverse();
    testWearBounded();
    testDeterminism();
    testLoop();
    testSetBufferAndNoBuffer();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular TURNTABLE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
