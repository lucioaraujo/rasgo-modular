// Teste isolado do Módulo 46 (HALL — reverberação FDN).
// Critérios do dossiê `dossies/46_hall.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Hall.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <cstdint>
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

struct Rec { std::vector<float> l, r; };

Rec run(Hall& h, int blocks, Gen in, Gen frz = nullptr) {
    h.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock bi(kSr, 1, kB), bs(kSr, 1, kB), bd(kSr, 1, kB), bf(kSr, 1, kB);
    Rec r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bi.at(0, k) = in ? in(n + k) : 0.0f;
            bf.at(0, k) = frz ? frz(n + k) : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            in ? &bi : nullptr, nullptr, nullptr, frz ? &bf : nullptr};
        h.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.l.push_back(out[0].at(0, k));
            r.r.push_back(out[1].at(0, k));
        }
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
    double z = 0.0, s = 0.0;
    const double c = 1.0 - std::exp(-6.28318 * 4000.0 / kSr);
    for (std::size_t i = a; i < b && i < v.size(); ++i) {
        z += (v[i] - z) * c;
        const double hp = v[i] - z;
        s += hp * hp;
    }
    return std::sqrt(s / std::max<std::size_t>(1, b - a));
}

double corr(const std::vector<float>& a, const std::vector<float>& b,
            std::size_t s, std::size_t e) {
    double sa = 0, sb = 0, saa = 0, sbb = 0, sab = 0;
    const std::size_t n = e - s;
    for (std::size_t i = s; i < e; ++i) {
        sa += a[i]; sb += b[i]; saa += a[i] * a[i];
        sbb += b[i] * b[i]; sab += a[i] * b[i];
    }
    const double cov = sab / n - (sa / n) * (sb / n);
    const double va = saa / n - (sa / n) * (sa / n);
    const double vb = sbb / n - (sb / n) * (sb / n);
    return cov / std::sqrt(va * vb + 1e-20);
}

Gen noise(float amp, std::size_t until) {
    auto st = std::make_shared<std::uint32_t>(0x2468acef);
    return [st, amp, until](std::size_t n) {
        if (n >= until) return 0.0f;
        *st = *st * 1664525u + 1013904223u;
        return amp * (static_cast<float>(static_cast<std::int32_t>(*st)) / 2147483648.0f);
    };
}

void set(Hall& h, float size, float decay, float damp, float mod,
         float pre, float mix) {
    h.setParameter("size", size);   h.setParameter("decay", decay);
    h.setParameter("damp", damp);   h.setParameter("mod", mod);
    h.setParameter("pre", pre);     h.setParameter("mix", mix);
}

void testImpulseDecays() {
    Hall h;
    set(h, 0.5f, 0.25f, 0.4f, 0.2f, 0.0f, 1.0f);
    const auto r = run(h, 90, [](std::size_t n) { return n == 100 ? 1.0f : 0.0f; });
    const double e1 = rms(r.l, 3000, 6000);
    const double e2 = rms(r.l, 9000, 12000);
    const double e3 = rms(r.l, 18000, 21000);
    EXPECT(e1 > 1e-4);
    EXPECT(e2 < e1 && e3 < e2);   // cauda decai monotônica
}

void testLongerDecayLongerTail() {
    auto tailRms = [](float decay) {
        Hall h;
        set(h, 0.5f, decay, 0.3f, 0.2f, 0.0f, 1.0f);
        const auto r = run(h, 200, noise(0.3f, 4000));
        return rms(r.l, 40000, 46000);
    };
    EXPECT(tailRms(0.85f) > 4.0 * tailRms(0.3f));
}

void testDampDarkens() {
    auto hf = [](float damp) {
        Hall h;
        set(h, 0.5f, 0.7f, damp, 0.2f, 0.0f, 1.0f);
        const auto r = run(h, 120, noise(0.3f, 3000));
        const double t = rms(r.l, 12000, 20000);
        return hfEnergy(r.l, 12000, 20000) / (t + 1e-9);
    };
    EXPECT(hf(0.9f) < 0.6 * hf(0.1f));
}

void testFreezeHolds() {
    Hall h;
    set(h, 0.5f, 0.6f, 0.0f, 0.2f, 0.0f, 1.0f);
    // seno de 200 Hz nos primeiros 4000; freeze a partir de 4000
    auto sig = [](std::size_t n) {
        return n < 4000 ? 0.3f * std::sin(2.0 * M_PI * 200.0 * n / kSr) : 0.0f;
    };
    auto frz = [](std::size_t n) { return n >= 4000 ? 1.0f : 0.0f; };
    const auto r = run(h, 220, sig, frz);
    const double a = rms(r.l, 12000, 16000);
    const double b = rms(r.l, 44000, 48000);
    EXPECT(a > 1e-3);
    EXPECT(b > 0.4 * a && b < 2.5 * a);   // segura (não decai nem cresce)
}

void testMixDry() {
    Hall h;
    set(h, 0.5f, 0.5f, 0.4f, 0.3f, 0.2f, 0.0f);
    auto sig = noise(0.4f, 100000);
    const auto r = run(h, 12, sig);
    auto ref = noise(0.4f, 100000);
    for (std::size_t i = 0; i < r.l.size(); ++i) {
        const float d = ref(i);
        EXPECT(std::fabs(r.l[i] - d) < 1e-6f && std::fabs(r.r[i] - d) < 1e-6f);
    }
}

void testStereoDecorrelated() {
    Hall h;
    set(h, 0.6f, 0.7f, 0.3f, 0.3f, 0.0f, 1.0f);
    const auto r = run(h, 120, noise(0.3f, 6000));
    EXPECT(std::fabs(corr(r.l, r.r, 12000, 24000)) < 0.9);
}

void testPredelay() {
    auto onsetRms = [](float pre) {
        Hall h;
        set(h, 0.5f, 0.5f, 0.4f, 0.0f, pre, 1.0f);
        const auto r = run(h, 40, [](std::size_t n) { return n == 50 ? 1.0f : 0.0f; });
        // 1º eco da linha mais curta ~n 1290 sem pré-atraso; com pre=0,5
        // (~2880 amostras) só chega bem depois de 2800
        return rms(r.l, 1400, 2800);
    };
    EXPECT(onsetRms(0.5f) < 0.3 * onsetRms(0.0f));   // pré-atraso segura o molhado
}

void testDeterminism() {
    Hall a, b;
    set(a, 0.55f, 0.6f, 0.35f, 0.5f, 0.1f, 0.7f);
    set(b, 0.55f, 0.6f, 0.35f, 0.5f, 0.1f, 0.7f);
    const auto ra = run(a, 60, noise(0.3f, 8000));
    const auto rb = run(b, 60, noise(0.3f, 8000));
    bool same = ra.l.size() == rb.l.size();
    for (std::size_t i = 0; same && i < ra.l.size(); ++i)
        same = ra.l[i] == rb.l[i] && ra.r[i] == rb.r[i];
    EXPECT(same);
}

void testBounded() {
    for (float sz = 0.0f; sz <= 1.0f; sz += 0.25f) {
        Hall h;
        set(h, sz, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        const auto r = run(h, 120, noise(1.0f, 100000));
        for (std::size_t i = 0; i < r.l.size(); ++i) {
            EXPECT(std::isfinite(r.l[i]) && std::fabs(r.l[i]) < 1.01f);
            EXPECT(std::isfinite(r.r[i]) && std::fabs(r.r[i]) < 1.01f);
        }
    }
}

void testStableNoBlowup() {
    Hall h;
    set(h, 0.7f, 0.95f, 0.1f, 0.3f, 0.0f, 1.0f);
    const auto r = run(h, 250, noise(0.3f, 5000));
    const double early = rms(r.l, 10000, 14000);
    const double late = rms(r.l, 56000, 60000);
    EXPECT(late < early);   // decai (g_i < 1 mesmo em decay máximo, sem freeze)
    EXPECT(late > 1e-5);
}

void testPanel() {
    Hall h;
    check(validatePanel(h).empty(), "painel HALL fecha");
    std::cout << renderAscii(h);
}

}  // namespace

int main() {
    testImpulseDecays();
    testLongerDecayLongerTail();
    testDampDarkens();
    testFreezeHolds();
    testMixDry();
    testStereoDecorrelated();
    testPredelay();
    testDeterminism();
    testBounded();
    testStableNoBlowup();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular HALL tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
