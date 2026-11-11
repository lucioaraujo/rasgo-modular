// RELATE (#84) — os atos entre duas vozes. Critérios de `dossies/84_relate.md`.

#include "dsp/Relate.hpp"

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
constexpr std::size_t kB = 240;
constexpr double kPi = 3.14159265358979323846;

struct Res { double oa = 0, ob = 0, mix = 0, meet = 0; };

// A e B são senos (220/330 Hz) ligados por portões (função do tempo em s)
template <typename GA, typename GB>
std::vector<Res> run(Relate& m, double seconds, GA ga, GB gb, double slice = 0.1) {
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    AudioBlock a(kSr, 1, kB), b(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&a, &b};
    std::vector<Res> r;
    Res acc; long cnt = 0; long n = 0;
    const long per = long(slice * kSr);
    for (int blk = 0; blk < int(seconds * kSr / kB); ++blk) {
        for (std::size_t i = 0; i < kB; ++i, ++n) {
            const double t = double(n) / kSr;
            a.at(0, i) = ga(t) ? 0.5f * float(std::sin(2 * kPi * 220 * t)) : 0.0f;
            b.at(0, i) = gb(t) ? 0.5f * float(std::sin(2 * kPi * 330 * t)) : 0.0f;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            acc.oa += out[0].at(0, i) * out[0].at(0, i);
            acc.ob += out[1].at(0, i) * out[1].at(0, i);
            acc.mix += out[2].at(0, i) * out[2].at(0, i);
            acc.meet += out[3].at(0, i);
            if (++cnt == per) {
                r.push_back({std::sqrt(acc.oa / cnt), std::sqrt(acc.ob / cnt), std::sqrt(acc.mix / cnt), acc.meet / cnt});
                acc = Res{}; cnt = 0;
            }
        }
    }
    return r;
}

Relate make(float verb, float inten, float time) {
    Relate m;
    m.setParameter("verb", verb); m.setParameter("intensity", inten); m.setParameter("time", time);
    return m;
}

void testShadow() {
    // A soa de 0,5 a 1,0 s; B sempre. SOMBRA com TEMPO 0,3: B só soa ~0,8–1,3 s
    auto m = make(0, 1.0f, 0.3f);
    const auto r = run(m, 2.0, [](double t) { return t > 0.5 && t < 1.0; }, [](double) { return true; });
    std::printf("SOMBRA B: 0,3 s %.3f · 1,0 s %.3f · 1,6 s %.3f\n", r[3].ob, r[10].ob, r[16].ob);
    check(r[3].ob < 0.02 && r[10].ob > 0.15 && r[16].ob < 0.02, "SOMBRA: B soa onde A soou, um TEMPO depois");
}

void testHocket() {
    auto m = make(1, 1.0f, 0.1f);
    const auto r = run(m, 1.0, [](double) { return true; }, [](double t) { return t > 0.5; });
    check(r[2].oa > 0.3, "HOQUETO: A sozinho soa inteiro");
    check(r[8].oa < 0.25 && r[8].ob < 0.25, "HOQUETO: juntos, as duas se cedem");
}

void testMeetingAndRing() {
    auto m = make(4, 1.0f, 0.1f);
    const auto r = run(m, 1.0, [](double) { return true; }, [](double t) { return t > 0.5; });
    check(r[2].meet < 0.05 && r[8].meet > 0.6, "ENC: o encontro só existe quando as duas soam");
    check(r[2].mix < 0.02 && r[8].mix > 0.05, "ANEL: o MIX é o produto A·B");
}

}  // namespace

int main() {
    testShadow();
    testHocket();
    testMeetingAndRing();
    if (g_failures == 0) std::printf("test_relate: ok\n");
    return g_failures == 0 ? 0 : 1;
}
