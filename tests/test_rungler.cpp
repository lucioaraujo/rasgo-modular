// RUNGLER (#102) — Hordijk. Critérios de `dossies/102_rungler.md`.

#include "dsp/Rungler.hpp"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++g_failures; }
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 256;

// a sequência de valores do RUNG, um por passo do registrador
std::vector<float> steps(float fa, float fb, float mod, float loop, double seconds, bool* finite = nullptr) {
    Rungler m;
    m.setParameter("freq_a", fa); m.setParameter("freq_b", fb);
    m.setParameter("mod_a", mod); m.setParameter("mod_b", mod); m.setParameter("loop", loop);
    m.prepare(kSr, kB);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kB));
    std::vector<const AudioBlock*> ins{nullptr, nullptr, nullptr};
    std::vector<float> s; long last = 0;
    for (int b = 0; b < int(seconds * kSr / kB); ++b) {
        m.process(ins, out);
        for (std::size_t i = 0; i < kB; ++i)
            for (int k = 0; k < 4; ++k) if (finite && !std::isfinite(out[std::size_t(k)].at(0, i))) *finite = false;
        if (m.steps() != last) { s.push_back(out[3].at(0, kB - 1)); last = m.steps(); }
    }
    return s;
}

bool periodic(const std::vector<float>& s, std::size_t p, std::size_t from) {
    for (std::size_t i = from; i + p < s.size(); ++i) if (s[i] != s[i + p]) return false;
    return true;
}

void testDac() {
    check(Rungler::dac(0x00) == 0.0f && Rungler::dac(0xE0) == 1.0f, "três bits: de 0 a 1");
    std::set<float> v; for (int r = 0; r < 256; ++r) v.insert(Rungler::dac(std::uint8_t(r)));
    check(v.size() == 8, "oito degraus");
}

void testLoop() {
    // relógio A lento o bastante para cada bloco ver no máximo um passo
    const auto locked = steps(20.0f, 230.0f, 0.5f, 1.0f, 8.0), free = steps(20.0f, 230.0f, 0.5f, 0.0f, 8.0);
    std::printf("passos em 8 s: %zu (LOOP 1) · %zu (LOOP 0)\n", locked.size(), free.size());
    check(periodic(locked, 8, 8), "LOOP 1: o padrão de 8 passos se repete para sempre");
    check(!periodic(free, 8, 8), "LOOP 0: o bit de B entra e o padrão muda");
}

void testChaosFinite() {
    bool fin = true;
    const auto s = steps(900.0f, 1300.0f, 1.0f, 0.0f, 3.0, &fin);
    std::set<float> v(s.begin(), s.end());
    std::printf("em áudio, MOD 1: %zu passos, %zu degraus distintos\n", s.size(), v.size());
    check(fin, "finito com o laço todo aberto");
    check(v.size() >= 6, "o caos visita quase todos os degraus");
}

}  // namespace

int main() {
    testDac();
    testLoop();
    testChaosFinite();
    if (g_failures == 0) std::printf("test_rungler: ok\n");
    return g_failures == 0 ? 0 : 1;
}
