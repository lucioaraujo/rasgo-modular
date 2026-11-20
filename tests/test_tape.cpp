// TAPE (#96) — a fita magnética. Critérios de `dossies/96_tape.md`.

#include "dsp/Tape.hpp"

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

struct Set { float drive = 0.4f, speed = 1.0f, wow = 0.0f, flutter = 0.0f, hiss = 0.0f, wear = 0.0f; };

std::vector<float> render(const Set& s, float amp, float hz, double seconds, int* drops = nullptr) {
    Tape m;
    m.setParameter("drive", s.drive); m.setParameter("speed", s.speed); m.setParameter("wow", s.wow);
    m.setParameter("flutter", s.flutter); m.setParameter("hiss", s.hiss); m.setParameter("wear", s.wear);
    m.prepare(kSr, kB);
    AudioBlock in(kSr, 1, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    std::vector<float> y; std::size_t n = 0; float pg = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n)
            in.at(0, i) = amp * float(std::sin(2.0 * 3.14159265358979 * hz * double(n) / kSr));
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            y.push_back(out[0].at(0, i));
            if (drops && pg < 0.5f && out[1].at(0, i) >= 0.5f) ++*drops;
            pg = out[1].at(0, i);
        }
    }
    return y;
}

double goertzel(const std::vector<float>& y, std::size_t a, std::size_t b, double hz) {
    const double w = 2.0 * 3.14159265358979 * hz / kSr, cw = 2.0 * std::cos(w);
    double s1 = 0, s2 = 0;
    for (std::size_t i = a; i < b; ++i) { const double s0 = y[i] + cw * s1 - s2; s2 = s1; s1 = s0; }
    return s1 * s1 + s2 * s2 - cw * s1 * s2;
}

double rms(const std::vector<float>& y, std::size_t a, std::size_t b) {
    double e = 0; for (std::size_t i = a; i < b; ++i) e += double(y[i]) * y[i];
    return std::sqrt(e / double(b - a));
}

void testSaturation() {
    Set clean; clean.drive = 0.0f;
    Set hot; hot.drive = 1.0f;
    const auto a = render(clean, 0.8f, 200.0, 1.0), b = render(hot, 0.8f, 200.0, 1.0);
    const double ha = goertzel(a, 4800, 48000, 600.0) / goertzel(a, 4800, 48000, 200.0);
    const double hb = goertzel(b, 4800, 48000, 600.0) / goertzel(b, 4800, 48000, 200.0);
    std::printf("3º harmônico / fundamental: DRIVE 0 %.2e · DRIVE 1 %.2e\n", ha, hb);
    check(hb > 10.0 * ha, "o DRIVE satura (mais harmônicos)");
    check(rms(b, 4800, 48000) < 1.0, "a saturação arredonda: o nível não explode");
}

void testHeadLoss() {
    Set slow; slow.speed = 0.0f; slow.drive = 0.0f;
    Set fast; fast.speed = 2.0f; fast.drive = 0.0f;
    const auto a = render(slow, 0.2f, 12000.0, 1.0), b = render(fast, 0.2f, 12000.0, 1.0);
    std::printf("12 kHz: 7,5 ips RMS %.3f · 30 ips RMS %.3f\n", rms(a, 4800, 48000), rms(b, 4800, 48000));
    check(rms(a, 4800, 48000) < 0.8 * rms(b, 4800, 48000), "a fita lenta perde mais agudo");
}

// a variação do período (zero a zero) do seno de 440 Hz
double periodJitter(const std::vector<float>& y) {
    std::vector<double> z;
    for (std::size_t i = 4800; i + 1 < y.size(); ++i)
        if (y[i] < 0 && y[i + 1] >= 0) z.push_back(double(i) - y[i] / (y[i + 1] - y[i]));
    double m = 0, v = 0; std::size_t n = 0;
    for (std::size_t i = 1; i < z.size(); ++i) { m += z[i] - z[i - 1]; ++n; }
    m /= double(n);
    for (std::size_t i = 1; i < z.size(); ++i) v += (z[i] - z[i - 1] - m) * (z[i] - z[i - 1] - m);
    return std::sqrt(v / double(n)) / m;
}

void testWowFlutter() {
    Set still; still.drive = 0.0f;
    Set moving; moving.drive = 0.0f; moving.wow = 1.0f; moving.flutter = 1.0f;
    const double a = periodJitter(render(still, 0.5f, 440.0, 3.0)), b = periodJitter(render(moving, 0.5f, 440.0, 3.0));
    std::printf("variação do período: parada %.2e · WOW+FLUT 1 %.2e\n", a, b);
    check(b > 20.0 * a + 1e-4, "o transporte faz a altura oscilar");
}

void testHissAndWear() {
    Set h; h.hiss = 1.0f;
    const auto s = render(h, 0.0f, 100.0, 1.0);
    const double floor = rms(s, 4800, 48000);
    std::printf("silêncio com HISS 1: RMS %.4f (%.1f dBFS)\n", floor, 20 * std::log10(floor));
    check(floor > 1e-4 && floor < 0.02, "o chiado fica por baixo, audível e discreto");
    int drops = 0;
    Set w; w.wear = 1.0f; w.drive = 0.0f;
    const auto y = render(w, 0.5f, 300.0, 20.0, &drops);
    bool fin = true; for (float v : y) if (!std::isfinite(v)) fin = false;
    std::printf("WEAR 1, 20 s: %d dropouts\n", drops);
    check(drops >= 6 && drops <= 30, "a fita gasta falha algumas vezes por segundo, não o tempo todo");
    check(fin, "finito");
}

}  // namespace

int main() {
    testSaturation();
    testHeadLoss();
    testWowFlutter();
    testHissAndWear();
    if (g_failures == 0) std::printf("test_tape: ok\n");
    return g_failures == 0 ? 0 : 1;
}
