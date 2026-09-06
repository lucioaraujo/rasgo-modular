// Teste isolado do Módulo 39 (GLIDE — portamento por nota).
// Critérios do dossiê `dossies/39_glide.md` §3/§5.

#include "core/SignalGraph.hpp"
#include "dsp/Glide.hpp"
#include "io/AsciiPanel.hpp"

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
void near(double a, double b, double t = 1e-4) {
    check(std::fabs(a - b) < t, "near");
    if (std::fabs(a - b) >= t) std::cerr << "   " << a << " vs " << b << '\n';
}

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

struct Rec { std::vector<float> out, mov, done; };

// roda `blocks` blocos; `pitch(frame)` / `slide(frame)` / `gate(frame)`
// (nullptr = porta desconectada). Devolve as 3 saídas amostra a amostra.
Rec run(Glide& g, int blocks,
        float (*pitch)(std::size_t),
        float (*slide)(std::size_t) = nullptr,
        float (*gate)(std::size_t) = nullptr) {
    g.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB), bs(kSr, 1, kB), bg(kSr, 1, kB);
    Rec r;
    std::size_t n = 0;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t k = 0; k < kB; ++k) {
            if (pitch) bp.at(0, k) = pitch(n + k);
            if (slide) bs.at(0, k) = slide(n + k);
            if (gate) bg.at(0, k) = gate(n + k);
        }
        std::vector<const AudioBlock*> ins{
            pitch ? &bp : nullptr, slide ? &bs : nullptr, gate ? &bg : nullptr};
        g.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) {
            r.out.push_back(out[0].at(0, k));
            r.mov.push_back(out[1].at(0, k));
            r.done.push_back(out[2].at(0, k));
        }
        n += kB;
    }
    return r;
}

// degrau: 0 nas primeiras 200 amostras, depois 1,0
float stepUp(std::size_t f) { return f < 200 ? 0.0f : 1.0f; }
float high(std::size_t) { return 1.0f; }
float low(std::size_t) { return 0.0f; }
// sobe pra 1,0 e SEGURA até assentar (6000 amostras), depois cai pra 0
float settleThenDown(std::size_t f) { return f < 6000 ? 1.0f : 0.0f; }

void testMode0AlwaysGlides() {
    Glide g;
    g.setParameter("time", 0.05f);   // 50 ms
    g.setParameter("mode", 0.0f);
    g.setParameter("curve", 0.0f);   // linear: rate constante
    const Rec r = run(g, 40, stepUp);
    // logo depois do degrau (amostra 210) já saiu de 0 mas não chegou a 1
    EXPECT(r.out[210] > 0.0f && r.out[210] < 1.0f);
    // monotônico crescente durante o deslize
    bool mono = true;
    for (std::size_t i = 201; i < 200 + 3000; ++i)
        if (r.out[i] + 1e-6f < r.out[i - 1]) mono = false;
    EXPECT(mono);
    // ~50 ms depois (2400 amostras) chegou perto de 1
    near(r.out[200 + 2500], 1.0f, 0.02);
    // e antes disso não tinha chegado
    EXPECT(r.out[200 + 1000] < 0.9f);
}

void testTimeZeroJumps() {
    Glide g;
    g.setParameter("time", 0.0f);
    g.setParameter("mode", 0.0f);
    const Rec r = run(g, 4, stepUp);
    near(r.out[199], 0.0f);
    near(r.out[200], 1.0f);   // salto seco na amostra do degrau
    near(r.out[201], 1.0f);
}

void testMode1SlideGated() {
    Glide g;
    g.setParameter("time", 0.05f);
    g.setParameter("curve", 0.0f);   // linear: assenta exato em `time`
    g.setParameter("mode", 1.0f);
    // sem slide -> segue a entrada amostra a amostra (salta)
    Rec r = run(g, 4, stepUp, low);
    near(r.out[200], 1.0f);
    // com slide alto -> desliza
    r = run(g, 40, stepUp, high);
    EXPECT(r.out[210] > 0.0f && r.out[210] < 1.0f);
    near(r.out[200 + 2500], 1.0f, 0.02);   // linear, 52 ms > 50 ms -> chegou
}

void testMode2Legato() {
    Glide g;
    g.setParameter("time", 0.05f);
    g.setParameter("mode", 2.0f);
    // gate sempre alto (nota sustentada) -> a mudança de pitch em 200
    // acontece SEM borda de gate -> desliza
    Rec r = run(g, 40, stepUp, nullptr, high);
    EXPECT(r.out[210] > 0.0f && r.out[210] < 1.0f);
    // gate que SOBE junto com o degrau -> ataque destacado -> salta
    r = run(g, 4, stepUp, nullptr, stepUp);
    near(r.out[200], 1.0f);
}

void testFallAsymmetry() {
    Glide up;   up.setParameter("time", 0.1f); up.setParameter("fall", 0.0f);
    Glide down; down.setParameter("time", 0.1f); down.setParameter("fall", -1.0f);
    up.setParameter("mode", 0.0f); down.setParameter("mode", 0.0f);
    up.setParameter("curve", 0.0f); down.setParameter("curve", 0.0f);
    const Rec ru = run(up, 80, settleThenDown);   // assenta em 1,0, depois desce (fall 0)
    const Rec rd = run(down, 80, settleThenDown); // idem (fall -1 -> descida 6× mais rápida)
    // 10 ms depois do degrau: o de fall -1 já desceu bem mais
    const std::size_t at = 6000 + 480;
    EXPECT(ru.out[6000 - 1] > 0.98f);             // subiu e assentou
    EXPECT(rd.out[at] < ru.out[at] - 0.2f);
}

void testCurveLinearVsExp() {
    Glide lin; lin.setParameter("time", 0.1f); lin.setParameter("curve", 0.0f);
    Glide exp; exp.setParameter("time", 0.1f); exp.setParameter("curve", 1.0f);
    lin.setParameter("mode", 0.0f); exp.setParameter("mode", 0.0f);
    const Rec rl = run(lin, 130, stepUp);
    const Rec re = run(exp, 130, stepUp);
    // no começo têm a MESMA inclinação; ao chegar em `time` (4800 amostras)
    // o linear assentou e o exponencial (1 polo, τ = time) está a ~63%
    const std::size_t atTime = 200 + 4800;
    near(rl.out[atTime], 1.0f, 0.01);
    EXPECT(re.out[atTime] > 0.55f && re.out[atTime] < 0.72f);
    // o exponencial chega perto de 1 só bem depois (~3 τ ≈ 95%)
    EXPECT(re.out[200 + 4800 * 3] > 0.92f);
    for (float v : rl.out) EXPECT(std::isfinite(v));
    for (float v : re.out) EXPECT(std::isfinite(v));
}

void testMovingAndDone() {
    Glide g;
    g.setParameter("time", 0.03f);
    g.setParameter("mode", 0.0f);
    const Rec r = run(g, 40, stepUp);
    EXPECT(r.mov[199] < 0.5f);              // parado antes do degrau
    EXPECT(r.mov[220] > 0.5f);              // movendo logo depois
    // achou o pulso de done UMA vez, depois do deslize
    int pulses = 0;
    bool inPulse = false;
    for (float v : r.done) {
        if (v > 0.5f && !inPulse) { ++pulses; inPulse = true; }
        else if (v < 0.5f) inPulse = false;
    }
    EXPECT(pulses == 1);
    near(r.mov[r.mov.size() - 1], 0.0f);    // parado no fim
}

void testNoPitchHolds() {
    Glide g;
    g.setParameter("mode", 0.0f);
    // primeiro dá um valor, depois desconecta pitch
    g.prepare(kSr, kB);
    std::vector<AudioBlock> out(3, AudioBlock(kSr, 1, kB));
    AudioBlock bp(kSr, 1, kB);
    for (std::size_t k = 0; k < kB; ++k) bp.at(0, k) = 0.7f;
    std::vector<const AudioBlock*> withP{&bp, nullptr, nullptr};
    for (int b = 0; b < 20; ++b) g.process(withP, out);
    const float held = out[0].at(0, kB - 1);
    std::vector<const AudioBlock*> noP{nullptr, nullptr, nullptr};
    g.process(noP, out);
    near(out[0].at(0, kB - 1), held);        // congela no último valor, não salta pra 0
}

void testDeterminism() {
    Glide a, b;
    for (Glide* x : {&a, &b}) {
        x->setParameter("time", 0.07f);
        x->setParameter("fall", 0.3f);
        x->setParameter("curve", 0.6f);
    }
    const Rec ra = run(a, 30, stepUp, high, high);
    const Rec rb = run(b, 30, stepUp, high, high);
    bool same = ra.out.size() == rb.out.size();
    for (std::size_t i = 0; same && i < ra.out.size(); ++i)
        same = ra.out[i] == rb.out[i];
    EXPECT(same);
}

void testPanel() {
    Glide g;
    check(validatePanel(g).empty(), "painel GLIDE fecha");
    std::cout << renderAscii(g);
}

}  // namespace

int main() {
    testMode0AlwaysGlides();
    testTimeZeroJumps();
    testMode1SlideGated();
    testMode2Legato();
    testFallAsymmetry();
    testCurveLinearVsExp();
    testMovingAndDone();
    testNoPitchHolds();
    testDeterminism();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular GLIDE tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
