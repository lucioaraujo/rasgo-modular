// VIA (#83) — o cabo com matéria. Critérios de `dossies/83_via.md`.

#include "dsp/Via.hpp"

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

std::vector<float> run(Via& m, double seconds, float hz, float amp = 0.5f) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 1, kB));
    AudioBlock in(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, nullptr};
    std::vector<float> y;
    long n = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        for (std::size_t i = 0; i < kB; ++i, ++n)
            in.at(0, i) = hz > 0 ? amp * float(std::sin(2.0 * kPi * hz * double(n) / kSr)) : amp;
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) y.push_back(out[0].at(0, i));
    }
    return y;
}

Via only(const char* port) {
    Via m;
    for (const char* p : {"res", "diode", "capacitor", "pulse", "transistor", "reflux", "speaker"})
        m.setParameter(p, std::string(p) == port ? 1.0f : 0.0f);
    m.setParameter("level", 1.0f);
    return m;
}

double tone(const std::vector<float>& v, std::size_t from, double hz) {
    double re = 0, im = 0;
    for (std::size_t n = from; n < v.size(); ++n) {
        const double ph = 2.0 * kPi * hz * double(n) / kSr;
        re += v[n] * std::cos(ph); im += v[n] * std::sin(ph);
    }
    return 2.0 * std::sqrt(re * re + im * im) / double(v.size() - from);
}

void testPorts() {
    auto r = only("res");
    const auto yr = run(r, 0.5, 220.0f);
    check(std::fabs(tone(yr, 4800, 220.0) - 0.5) < 0.01, "RES: passa o sinal");
    auto d = only("diode");
    const auto yd = run(d, 0.5, 220.0f);
    check(tone(yd, 4800, 440.0) > 0.2 && tone(yd, 4800, 220.0) < 0.05,
          "DIODO: a retificação dobra a frequência");
    auto c = only("capacitor");
    const auto ycl = run(c, 0.5, 2000.0f);
    check(tone(ycl, 4800, 2000.0) < 0.05, "CAP: o capacitor alisa o agudo (lembra devagar)");
    auto cd = only("capacitor");
    const auto ydc = run(cd, 1.0, 0.0f, 0.5f);
    check(std::fabs(ydc.back() - 0.5f) < 0.02, "CAP: um nível constante carrega o capacitor");
    auto p = only("pulse");
    const auto yp = run(p, 0.5, 110.0f);
    check(tone(yp, 4800, 330.0) > 0.25, "PULSO: vira borda (ímpares fortes)");
    auto s = only("speaker");
    s.setParameter("body", 110.0f);
    const auto ys = run(s, 1.0, 110.0f, 0.2f), ys2 = [&] { auto t = only("speaker"); t.setParameter("body", 110.0f); return run(t, 1.0, 330.0f, 0.2f); }();
    check(tone(ys, 9600, 110.0) > 3.0 * tone(ys2, 9600, 330.0), "FALANTE: o corpo ressoa em CORPO");
}

void testComposite() {
    Via m;
    m.setParameter("res", 1.0f); m.setParameter("reflux", 1.0f);
    m.setParameter("level", 1.0f);
    const auto y = run(m, 0.5, 220.0f);
    bool fin = true; for (float v : y) if (!std::isfinite(v) || std::fabs(v) > 2.0f) fin = false;
    check(fin, "portas compostas: finito e limitado");
    Via z;
    for (const char* p : {"res", "diode", "capacitor", "pulse", "transistor", "reflux", "speaker"})
        z.setParameter(p, 0.0f);
    const auto yz = run(z, 0.2, 220.0f);
    bool silent = true; for (float v : yz) if (v != 0.0f) silent = false;
    check(silent, "nenhuma porta aberta: silêncio");
}

}  // namespace

int main() {
    testPorts();
    testComposite();
    if (g_failures == 0) std::printf("test_via: ok\n");
    return g_failures == 0 ? 0 : 1;
}
