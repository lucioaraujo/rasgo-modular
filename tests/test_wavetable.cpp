// Teste isolado do Módulo 40 (WAVETABLE — oscilador de tabela procedural).
// Critérios do dossiê `dossies/40_wavetable.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Wavetable.hpp"
#include "io/AsciiPanel.hpp"

#include <array>
#include <cmath>
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

struct In {
    float pitch = 0.0f;   // oitavas (1 V/oct)
    bool hasPitch = false;
    float fm = 0.0f;
    bool hasFm = false;
    const std::vector<float>* cap = nullptr;   // sinal de captura por amostra
    const std::vector<float>* grab = nullptr;  // trigger por amostra
};

std::vector<float> render(Wavetable& w, int blocks, const In& in = {}) {
    w.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB), bf(kSr, 1, kB), bc(kSr, 1, kB), bg(kSr, 1, kB);
    std::vector<float> r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            bp.at(0, k) = in.pitch;
            bf.at(0, k) = in.fm;
            bc.at(0, k) = in.cap && (n + k) < in.cap->size() ? (*in.cap)[n + k] : 0.0f;
            bg.at(0, k) = in.grab && (n + k) < in.grab->size() ? (*in.grab)[n + k] : 0.0f;
        }
        std::vector<const AudioBlock*> ins{
            in.hasPitch ? &bp : nullptr, nullptr,
            in.hasFm ? &bf : nullptr,
            in.cap ? &bc : nullptr,
            in.grab ? &bg : nullptr};
        w.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) r.push_back(out[0].at(0, k));
        n += kB;
    }
    return r;
}

// magnitude da harmônica h de `f0` no sinal (DFT ponto a ponto, 2ª metade
// do buffer pra o oscilador já ter assentado a fase)
double mag(const std::vector<float>& x, double f0, int h) {
    const std::size_t start = x.size() / 2;
    double re = 0.0, im = 0.0;
    const double wk = 2.0 * M_PI * f0 * h / kSr;
    for (std::size_t i = start; i < x.size(); ++i) {
        re += x[i] * std::cos(wk * static_cast<double>(i));
        im += x[i] * std::sin(wk * static_cast<double>(i));
    }
    const double n = static_cast<double>(x.size() - start);
    return std::sqrt(re * re + im * im) / n;
}

int zeroCross(const std::vector<float>& x) {
    int z = 0;
    for (std::size_t i = 1 + x.size() / 2; i < x.size(); ++i)
        if ((x[i - 1] < 0.0f) != (x[i] < 0.0f)) ++z;
    return z;
}

void testSineAtPos1() {
    Wavetable w;
    w.setParameter("freq", 220.0f);
    w.setParameter("pos", 1.0f);
    w.setParameter("warp", 0.0f);
    const auto r = render(w, 40);
    const double m1 = mag(r, 220.0, 1);
    const double m2 = mag(r, 220.0, 2);
    const double m3 = mag(r, 220.0, 3);
    EXPECT(m1 > 0.2);
    EXPECT(m2 / m1 < 0.03);   // quase sem 2ª harmônica -> seno
    EXPECT(m3 / m1 < 0.03);
}

void testSawAtPos0() {
    Wavetable w;
    w.setParameter("freq", 110.0f);
    w.setParameter("pos", 0.0f);
    w.setParameter("warp", 0.0f);
    const auto r = render(w, 40);
    const double m1 = mag(r, 110.0, 1);
    const double m2 = mag(r, 110.0, 2);
    const double m3 = mag(r, 110.0, 3);
    // serra: harmônicas ~1/h, todas presentes, decaindo
    EXPECT(m2 / m1 > 0.25 && m2 / m1 < 0.75);
    EXPECT(m3 / m1 > 0.15 && m3 / m1 < 0.6);
    EXPECT(m2 > m3);   // decai
}

std::array<double, 4> spec(float pos) {
    Wavetable w;
    w.setParameter("freq", 110.0f);
    w.setParameter("pos", pos);
    w.setParameter("warp", 0.0f);
    const auto r = render(w, 40);
    const double m1 = mag(r, 110.0, 1) + 1e-12;
    return {m1, mag(r, 110.0, 2) / m1, mag(r, 110.0, 3) / m1,
            mag(r, 110.0, 4) / m1};
}

void testRecipeFrames() {
    const auto saw = spec(0.0f);         // serra: pares presentes
    const auto sq = spec(1.0f / 3.0f);   // quadrada: só ímpares
    const auto sine = spec(1.0f);        // seno puro
    EXPECT(saw[1] > 0.25);               // serra tem 2ª harmônica forte
    EXPECT(sq[1] < 0.12);                // quadrada quase sem 2ª (par)
    EXPECT(sq[2] > 0.15);                // mas tem 3ª (ímpar)
    EXPECT(sine[1] < 0.03 && sine[2] < 0.03 && sine[3] < 0.03);
    // serra e seno geram formas de onda claramente diferentes
    Wavetable a; a.setParameter("freq", 110.0f); a.setParameter("pos", 0.0f);
    Wavetable b; b.setParameter("freq", 110.0f); b.setParameter("pos", 1.0f);
    const auto ra = render(a, 40), rb = render(b, 40);
    double diff = 0.0;
    for (std::size_t i = ra.size() / 2; i < ra.size(); ++i)
        diff += std::fabs(ra[i] - rb[i]);
    EXPECT(diff / (ra.size() / 2) > 0.1);
}

void testOneVoltPerOctave() {
    Wavetable lo; lo.setParameter("freq", 100.0f); lo.setParameter("pos", 0.5f);
    Wavetable hi; hi.setParameter("freq", 100.0f); hi.setParameter("pos", 0.5f);
    const auto rl = render(lo, 30);
    In up; up.hasPitch = true; up.pitch = 1.0f;   // +1 oitava
    const auto rh = render(hi, 30, up);
    const int zl = zeroCross(rl), zh = zeroCross(rh);
    EXPECT(zh > zl * 3 / 2);   // ~2× (com folga)
}

void testWarpAddsBrightness() {
    // tabela de seno (pos 1) + warp -> distorção de fase cria harmônicas
    Wavetable a; a.setParameter("freq", 110.0f); a.setParameter("pos", 1.0f); a.setParameter("warp", 0.0f);
    Wavetable b; b.setParameter("freq", 110.0f); b.setParameter("pos", 1.0f); b.setParameter("warp", 0.8f);
    const auto ra = render(a, 40);
    const auto rb = render(b, 40);
    const double a1 = mag(ra, 110.0, 1) + 1e-12;
    const double b1 = mag(rb, 110.0, 1) + 1e-12;
    // sem warp: seno puro (quase nada acima da 1ª). Com warp: harmônicas altas
    EXPECT(mag(ra, 110.0, 3) / a1 < 0.05);
    EXPECT(mag(rb, 110.0, 3) / b1 > 0.15);
    EXPECT(mag(rb, 110.0, 5) / b1 > 0.05);
    for (float v : rb) EXPECT(std::fabs(v) < 1.2f);
}

void testHighFreqBounded() {
    // afinação alta -> mip agressivo -> sem lixo, saída limitada
    Wavetable w;
    w.setParameter("freq", 6000.0f);
    w.setParameter("pos", 0.0f);   // serra: o pior caso
    const auto r = render(w, 30);
    for (float v : r) EXPECT(std::isfinite(v) && std::fabs(v) < 1.1f);
    // a fundamental ainda está lá
    EXPECT(mag(r, 6000.0, 1) > 0.05);
}

void testFmShifts() {
    Wavetable a; a.setParameter("freq", 200.0f); a.setParameter("pos", 0.5f);
    Wavetable b; b.setParameter("freq", 200.0f); b.setParameter("pos", 0.5f);
    b.setParameter("fm_amount", 0.5f);
    const auto ra = render(a, 30);
    In fm; fm.hasFm = true; fm.fm = 0.3f;   // FM DC positiva -> sobe a taxa
    const auto rb = render(b, 30, fm);
    EXPECT(zeroCross(rb) > zeroCross(ra));
}

void testCapture() {
    // captura: um seno de 1024 amostras -> vira o quadro do topo de pos
    std::vector<float> capSig(4096), grabSig(4096, 0.0f);
    for (std::size_t i = 0; i < capSig.size(); ++i)
        capSig[i] = 0.5f * std::sin(2.0 * M_PI * static_cast<double>(i) / 1024.0);
    for (int i = 300; i < 320; ++i) grabSig[static_cast<std::size_t>(i)] = 1.0f;

    Wavetable w;
    w.setParameter("freq", 110.0f);
    w.setParameter("pos", 1.0f);   // topo -> deveria virar o quadro capturado
    w.setParameter("warp", 0.0f);
    In in; in.cap = &capSig; in.grab = &grabSig;
    const auto r = render(w, 40, in);
    // o quadro capturado é um seno -> a saída em pos=1 continua ~senoidal
    // (mas agora vinda do buffer capturado, não da tabela) — 2ª harm baixa
    EXPECT(mag(r, 110.0, 1) > 0.15);
    EXPECT(mag(r, 110.0, 2) / (mag(r, 110.0, 1) + 1e-12) < 0.1);

    // sem `capture` conectado, `grab` é no-op: pos=1 é o seno da tabela
    Wavetable w2;
    w2.setParameter("freq", 110.0f);
    w2.setParameter("pos", 1.0f);
    In gOnly; gOnly.grab = &grabSig;   // grab mas sem cap
    const auto r2 = render(w2, 40, gOnly);
    EXPECT(mag(r2, 110.0, 2) / (mag(r2, 110.0, 1) + 1e-12) < 0.05);
}

void testDeterminism() {
    Wavetable a, b;
    for (Wavetable* x : {&a, &b}) {
        x->setParameter("freq", 175.0f);
        x->setParameter("pos", 0.4f);
        x->setParameter("warp", 0.35f);
        x->setParameter("drift", 0.0f);
    }
    const auto ra = render(a, 25);
    const auto rb = render(b, 25);
    bool same = ra.size() == rb.size();
    for (std::size_t i = 0; same && i < ra.size(); ++i)
        same = ra[i] == rb[i];
    EXPECT(same);
}

void testPanel() {
    Wavetable w;
    check(validatePanel(w).empty(), "painel WAVETABLE fecha");
    std::cout << renderAscii(w);
}

}  // namespace

int main() {
    testSineAtPos1();
    testSawAtPos0();
    testRecipeFrames();
    testOneVoltPerOctave();
    testWarpAddsBrightness();
    testHighFreqBounded();
    testFmShifts();
    testCapture();
    testDeterminism();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular WAVETABLE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
