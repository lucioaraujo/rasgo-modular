// Teste isolado do Módulo 47 (DRUM — voz de percussão).
// Critérios do dossiê `dossies/47_drum.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Drum.hpp"
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

std::vector<float> run(Drum& d, int blocks, Gen gate = nullptr,
                       Gen acc = nullptr, Gen tone = nullptr) {
    d.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bg(kSr, 1, kB), ba(kSr, 1, kB), bt(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bg.at(0, k) = gate ? gate(n + k) : 0.0f;
            ba.at(0, k) = acc ? acc(n + k) : 0.0f;
            bt.at(0, k) = tone ? tone(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            gate ? &bg : nullptr, acc ? &ba : nullptr, tone ? &bt : nullptr};
        d.process(ins, out);
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

double hfEnergy(const std::vector<float>& v, std::size_t a, std::size_t b) {
    // 2 polos de passa-alta a ~2,5 kHz (mais seletivo que 1 polo)
    double z1 = 0.0, z2 = 0.0, s = 0.0;
    const double c = 1.0 - std::exp(-6.28318 * 2500.0 / kSr);
    for (std::size_t i = a; i < b && i < v.size(); ++i) {
        z1 += (v[i] - z1) * c;
        const double h1 = v[i] - z1;
        z2 += (h1 - z2) * c;
        const double h2 = h1 - z2;
        s += h2 * h2;
    }
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
}

double magAt(const std::vector<float>& x, double hz, std::size_t a, std::size_t b) {
    double re = 0.0, im = 0.0, wsum = 0.0;
    const double wk = 2.0 * M_PI * hz / kSr;
    const std::size_t n = b - a;
    for (std::size_t i = a; i < b && i < x.size(); ++i) {
        const double tt = (double)(i - a);
        const double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * tt / (double)(n - 1));
        re += w * x[i] * std::cos(wk * (double)i);
        im += w * x[i] * std::sin(wk * (double)i);
        wsum += w;
    }
    return std::sqrt(re * re + im * im) / wsum;
}

int zc(const std::vector<float>& v, std::size_t a, std::size_t b) {
    int z = 0;
    for (std::size_t i = a + 1; i < b && i < v.size(); ++i)
        if ((v[i - 1] < 0.0f) != (v[i] < 0.0f)) ++z;
    return z;
}

// gate: pulso de 64 amostras em `at`
Gen pulseAt(std::size_t at) {
    return [at](std::size_t n) { return (n >= at && n < at + 64) ? 1.0f : 0.0f; };
}

void testGateOneHit() {
    Drum d;
    d.setParameter("tone", 60.0f);
    d.setParameter("decay", 0.4f);
    const auto r = run(d, 60, pulseAt(2000));
    EXPECT(rms(r, 0, 1800) < 1e-5);              // silêncio antes
    EXPECT(rms(r, 2100, 4000) > 0.05);           // golpe
    EXPECT(rms(r, 12000, 14000) < 0.5 * rms(r, 2100, 4000));   // decai
}

void testDecayLength() {
    auto tail = [](float decay) {
        Drum d;
        d.setParameter("tone", 60.0f);
        d.setParameter("decay", decay);
        const auto r = run(d, 90, pulseAt(1000));
        return rms(r, 16000, 20000);
    };
    EXPECT(tail(0.8f) > 5.0 * tail(0.2f));
}

void testBendSweepsPitch() {
    Drum d;
    d.setParameter("tone", 80.0f);
    d.setParameter("bend", 1.0f);
    d.setParameter("decay", 0.5f);
    d.setParameter("snap", 0.0f);
    const auto r = run(d, 60, pulseAt(1000));
    const int early = zc(r, 1050, 1550);    // logo após o golpe (agudo)
    const int late = zc(r, 6000, 6500);     // depois (voltou ao tone)
    EXPECT(early > late * 2);               // a altura varreu pra baixo
}

void testSnapAddsNoise() {
    auto hf = [](float snap) {
        Drum d;
        d.setParameter("tone", 60.0f);
        d.setParameter("snap", snap);
        d.setParameter("map", 0.5f);
        const auto r = run(d, 40, pulseAt(1000));
        return hfEnergy(r, 1000, 2000);
    };
    EXPECT(hf(1.0f) > 3.0 * hf(0.0f));
}

void testMapBrightens() {
    auto hf = [](float map) {
        Drum d;
        d.setParameter("tone", 80.0f);
        d.setParameter("decay", 0.5f);
        d.setParameter("snap", 0.2f);
        d.setParameter("map", map);
        const auto r = run(d, 40, pulseAt(1000));
        const double t = rms(r, 1000, 4000);
        return hfEnergy(r, 1000, 4000) / (t + 1e-9);
    };
    EXPECT(hf(1.0f) > 1.4 * hf(0.0f));
}

void testDriveHarmonicsBounded() {
    auto mk = [](Drum& d, float drive) {
        d.setParameter("tone", 200.0f); d.setParameter("bend", 0.0f);
        d.setParameter("snap", 0.0f);   d.setParameter("map", 0.0f);
        d.setParameter("decay", 0.6f);  d.setParameter("drive", drive);
    };
    Drum clean;  mk(clean, 0.0f);
    Drum dirty;  mk(dirty, 1.0f);
    const auto rc = run(clean, 40, pulseAt(1000));
    const auto rd = run(dirty, 40, pulseAt(1000));
    // sem snap/bend: corpo senoidal a 200 Hz; drive (tanh) cria harmônicos
    // ímpares. Razão (h5+h7+h9)/h1 sobe muito.
    auto odd = [](const std::vector<float>& v) {
        const double h1 = magAt(v, 200.0, 1100, 6000) + 1e-9;
        return (magAt(v, 1000.0, 1100, 6000) + magAt(v, 1400.0, 1100, 6000)
                + magAt(v, 1800.0, 1100, 6000)) / h1;
    };
    EXPECT(odd(rd) > 4.0 * odd(rc));
    for (float v : rd) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
}

void testRollSelfTriggers() {
    Drum d;
    d.setParameter("tone", 120.0f);
    d.setParameter("decay", 0.2f);
    d.setParameter("roll", 0.3f);
    const auto r = run(d, 120, nullptr);   // sem gate
    EXPECT(rms(r, 4000, 24000) > 0.03);    // toca sozinho
    // periódico: energia com vales (vários golpes, não um só sustentado)
    const double e1 = rms(r, 6000, 6600);
    const double e2 = rms(r, 20000, 20600);
    EXPECT(e1 > 0.01 && e2 > 0.01);        // ainda batendo lá na frente
}

void testAccentLouder() {
    Drum soft;
    soft.setParameter("tone", 60.0f);
    const auto rs = run(soft, 40, pulseAt(1000),
                        [](std::size_t) { return 0.3f; });
    Drum loud;
    loud.setParameter("tone", 60.0f);
    const auto rl = run(loud, 40, pulseAt(1000),
                        [](std::size_t) { return 1.5f; });
    EXPECT(rms(rl, 1100, 4000) > 1.5 * rms(rs, 1100, 4000));
}

void testDeterminism() {
    auto mk = [](Drum& d) {
        d.setParameter("tone", 70.0f);
        d.setParameter("decay", 0.5f);
        d.setParameter("drift", 0.5f);
        d.setParameter("roll", 0.25f);
        d.setParameter("map", 0.4f);
    };
    Drum a; mk(a);
    Drum b; mk(b);
    const auto ra = run(a, 60, pulseAt(500));
    const auto rb = run(b, 60, pulseAt(500));
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i) same = ra[i] == rb[i];
    EXPECT(same);
}

void testBounded() {
    Drum d;
    d.setParameter("tone", 900.0f);
    d.setParameter("bend", 1.0f);
    d.setParameter("decay", 1.0f);
    d.setParameter("snap", 1.0f);
    d.setParameter("map", 1.0f);
    d.setParameter("drive", 1.0f);
    d.setParameter("roll", 1.0f);
    d.setParameter("drift", 1.0f);
    const auto r = run(d, 120, nullptr, [](std::size_t) { return 2.0f; });
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.05f);
    EXPECT(rms(r, 10000, 20000) > 0.01);
}

void testPanel() {
    Drum d;
    check(validatePanel(d).empty(), "painel DRUM fecha");
    std::cout << renderAscii(d);
}

}  // namespace

int main() {
    testGateOneHit();
    testDecayLength();
    testBendSweepsPitch();
    testSnapAddsNoise();
    testMapBrightens();
    testDriveHarmonicsBounded();
    testRollSelfTriggers();
    testAccentLouder();
    testDeterminism();
    testBounded();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular DRUM tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
