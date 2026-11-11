// APOCRIFO (#77) — o patch cita o próprio passado. Critérios do dossiê
// `dossies/77_apocrifo.md`.

#include "dsp/Apocrifo.hpp"

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

// a entrada marca o tempo pela AMPLITUDE: um seno de 200 Hz cuja amplitude
// cresce 1/60 por segundo — a amplitude da saída diz de quando ela veio
float amp(double t) { return float(0.05 + 0.9 * t / 60.0); }

struct Result { float livePeak = 0, quotePeak = 0; long activeSamples = 0; float maxJump = 0; float age = 0; };

Result run(Apocrifo& m, double seconds, double trigAt) {
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 2, kB));
    AudioBlock in(kSr, 2, kB), trig(kSr, 1, kB);
    std::vector<const AudioBlock*> ins{&in, &trig};
    Result r;
    const long n = long(seconds * kSr);
    float prev = 0.0f;
    std::vector<float> act;
    for (long done = 0; done < n; done += long(kB)) {
        for (std::size_t i = 0; i < kB; ++i) {
            const double t = double(done + long(i)) / kSr;
            const float x = amp(t) * float(std::sin(2.0 * kPi * 200.0 * t));
            in.at(0, i) = x; in.at(1, i) = x;
            trig.at(0, i) = (std::fabs(t - trigAt) < 0.001) ? 1.0f : 0.0f;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i) {
            const double t = double(done + long(i)) / kSr;
            const float y = out[0].at(0, i);
            r.maxJump = std::max(r.maxJump, std::fabs(y - prev));
            prev = y;
            if (out[1].at(0, i) >= 0.5f) {
                ++r.activeSamples;
                act.push_back(std::fabs(y));
                r.livePeak = std::max(r.livePeak, amp(t));
                r.age = out[2].at(0, i);
            }
        }
    }
    // o miolo da citação (sem a espera pelo zero nem as bordas)
    const std::size_t a = act.size() / 5, b = act.size() - act.size() / 5;
    for (std::size_t k = a; k < b; ++k) r.quotePeak = std::max(r.quotePeak, act[k]);
    return r;
}

Apocrifo make(float mode, float minAge = 20.0f, float len = 0.5f) {
    Apocrifo m;
    m.setParameter("mode", mode);
    m.setParameter("min_age", minAge);
    m.setParameter("length", len);
    m.setParameter("chance", 0.0f);
    m.setParameter("depth", 1.0f);
    m.prepare(kSr, kB);
    return m;
}

void testNoPastNoQuote() {
    auto m = make(1.0f);
    const auto r = run(m, 12.0, 10.0);   // aos 10 s ainda não há 20 s de passado
    check(m.citations() == 0 && r.activeSamples == 0, "sem passado suficiente, não cita");
}

void testCutQuotesTheDistantPast() {
    auto m = make(1.0f);
    const auto r = run(m, 36.0, 35.0);
    std::printf("CORTE: pico citado %.3f, pico do presente %.3f, idade %.2f\n",
                r.quotePeak, r.livePeak, r.age * 60.0f);
    check(m.citations() == 1 && r.activeSamples > 0, "CORTE: cita");
    // a amplitude citada é a de ≥ 20 s atrás: ≤ amp(35 − 20)
    check(r.quotePeak <= amp(15.0) + 0.02f, "CORTE: o trecho vem de pelo menos IDADE atrás");
    check(r.age * 60.0f >= 20.0f, "CORTE: a saída IDADE diz de quão longe veio");
    // as emendas em zero: nenhum salto maior que o de um seno de 200 Hz cheio
    const float sineStep = float(2.0 * kPi * 200.0 / kSr) * 1.0f;
    std::printf("CORTE: maior salto %.4f (limite %.4f)\n", r.maxJump, 2.0f * sineStep);
    check(r.maxJump < 2.0f * sineStep, "CORTE: emendas sem estalo");
}

void testTearReversedAndDucked() {
    auto m = make(0.0f);
    const auto r = run(m, 36.0, 35.0);
    check(m.citations() == 1, "CORROÍDA: cita");
    // com PROF 1 o presente some sob a citação: o pico é o do passado
    check(r.quotePeak <= amp(15.0) + 0.05f, "CORROÍDA: o presente abaixa, o passado aparece");
}

void testParenthesisIsNow() {
    auto m = make(2.0f, 20.0f, 1.0f);
    const auto r = run(m, 6.0, 5.0);   // o parêntese não precisa de passado longo
    check(m.citations() == 1 && r.activeSamples > 0, "PARÊNTESE: abre já");
    check(std::fabs(r.quotePeak - amp(5.0)) < 0.03f, "PARÊNTESE: gagueja o agora");
}

void testChanceAndDeterminism() {
    Apocrifo a, b;
    for (Apocrifo* m : {&a, &b}) {
        m->setParameter("mode", 2.0f);
        m->setParameter("chance", 1.0f);
        m->setParameter("length", 0.2f);
        m->setParameter("seed", 3.0f);
        m->prepare(kSr, kB);
    }
    run(a, 60.0, -1.0);
    run(b, 60.0, -1.0);
    std::printf("CHANCE 1: %ld citações em 60 s\n", a.citations());
    check(a.citations() >= 2 && a.citations() <= 14, "CHANCE 1: ~uma a cada 10 s");
    check(a.citations() == b.citations(), "mesma SEED, mesmas citações");
}

}  // namespace

int main() {
    testNoPastNoQuote();
    testCutQuotesTheDistantPast();
    testTearReversedAndDucked();
    testParenthesisIsNow();
    testChanceAndDeterminism();
    if (g_failures == 0) std::printf("test_apocrifo: ok\n");
    return g_failures == 0 ? 0 : 1;
}
