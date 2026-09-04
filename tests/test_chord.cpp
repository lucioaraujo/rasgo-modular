// Teste isolado do Módulo 26 (CHORD — VCO parafônico).
// Critérios do dossiê `dossies/26_chord.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Chord.hpp"
#include "dsp/Filter.hpp"
#include "dsp/StepSequencer.hpp"
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

constexpr float kSr = 48000.0f;
constexpr std::size_t kB = 128;

std::vector<float> run(Chord& ch, int blocks, float pitch = 0.0f,
                       float chordCv = 0.0f, bool hasCv = false) {
    ch.prepare(kSr, kB);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
    AudioBlock pv(kSr, 1, kB), cv(kSr, 1, kB);
    std::vector<float> o;
    for (std::size_t k = 0; k < kB; ++k) { pv.at(0, k) = pitch; cv.at(0, k) = chordCv; }
    std::vector<const AudioBlock*> ins{&pv, hasCv ? &cv : nullptr, nullptr};
    for (int b = 0; b < blocks; ++b) {
        ch.process(ins, out);
        for (std::size_t k = 0; k < kB; ++k) o.push_back(out[0].at(0, k));
    }
    return o;
}

// magnitude na frequência `hz`
double mag(const std::vector<float>& v, double hz) {
    double re = 0, im = 0;
    const double w = 6.283185307 * hz / kSr;
    const std::size_t n0 = v.size() / 4;   // pula o transitório
    for (std::size_t k = n0; k < v.size(); ++k) {
        re += v[k] * std::cos(w * k);
        im += v[k] * std::sin(w * k);
    }
    return std::sqrt(re * re + im * im) / (v.size() - n0);
}
double rms(const std::vector<float>& v) {
    double s = 0; for (float x : v) s += double(x) * x; return std::sqrt(s / v.size());
}
double semi(double f0, int st) { return f0 * std::pow(2.0, st / 12.0); }

void testMajorChord() {
    Chord ch;
    ch.setParameter("freq", 110.0f);
    ch.setParameter("chord", 3.0f / 9.0f);   // maior {0,4,7,12}
    ch.setParameter("voices", 3.0f);
    ch.setParameter("detune", 0.0f);
    const auto o = run(ch, 400);
    const double root = mag(o, 110.0);
    const double third = mag(o, semi(110.0, 4));
    const double fifth = mag(o, semi(110.0, 7));
    const double offNote = mag(o, semi(110.0, 5));   // não está no acorde
    check(root > offNote * 3 && third > offNote * 3 && fifth > offNote * 3,
          "acorde maior: energia na raiz, 3ª maior e 5ª");
}

void testMinorChord() {
    Chord maj, min;
    maj.setParameter("chord", 3.0f / 9.0f);
    min.setParameter("chord", 4.0f / 9.0f);   // menor {0,3,7,12}
    maj.setParameter("detune", 0.0f);
    min.setParameter("detune", 0.0f);
    const auto oM = run(maj, 400);
    const auto om = run(min, 400);
    check(mag(oM, semi(110.0, 4)) > mag(oM, semi(110.0, 3)) * 2, "maior tem 3ª maior");
    check(mag(om, semi(110.0, 3)) > mag(om, semi(110.0, 4)) * 2, "menor tem 3ª menor");
}

void testVoiceCount() {
    Chord two, four;
    two.setParameter("chord", 6.0f / 9.0f);   // maj7 {0,4,7,11}
    four.setParameter("chord", 6.0f / 9.0f);
    two.setParameter("voices", 2.0f);
    four.setParameter("voices", 4.0f);
    two.setParameter("detune", 0.0f);
    four.setParameter("detune", 0.0f);
    const auto o2 = run(two, 400);
    const auto o4 = run(four, 400);
    // a 7ª maior (st 11) só soa com >= 4 vozes
    check(mag(o4, semi(110.0, 11)) > mag(o2, semi(110.0, 11)) * 3,
          "voices=4 traz a 4ª nota (7ª maior)");
}

void testInversion() {
    Chord root, inv;
    root.setParameter("chord", 3.0f / 9.0f);
    inv.setParameter("chord", 3.0f / 9.0f);
    root.setParameter("voices", 3.0f);
    inv.setParameter("voices", 3.0f);
    root.setParameter("detune", 0.0f);
    inv.setParameter("detune", 0.0f);
    root.setParameter("inversion", 0.0f);
    inv.setParameter("inversion", 1.0f / 3.0f);   // sobe a nota grave 1 oitava
    const auto ro = run(root, 400);
    const auto io = run(inv, 400);
    // energia migra de 110 pra 220
    check(mag(ro, 110.0) > mag(ro, 220.0) && mag(io, 220.0) > mag(io, 110.0),
          "inversão sobe a fundamental uma oitava");
}

void testDetuneBeats() {
    Chord tight, wide;
    tight.setParameter("chord", 0.0f);        // uníssono
    wide.setParameter("chord", 0.0f);
    tight.setParameter("voices", 3.0f);
    wide.setParameter("voices", 3.0f);
    tight.setParameter("detune", 0.0f);
    wide.setParameter("detune", 0.8f);
    const auto t = run(tight, 600);
    const auto w = run(wide, 600);
    // batimento = modulação lenta da amplitude. Mede a variação do RMS
    // em janelas de ~40 ms.
    auto amMod = [](const std::vector<float>& v) {
        const std::size_t win = 2000;
        double lo = 1e9, hi = 0;
        for (std::size_t i = 0; i + win < v.size(); i += win) {
            double s = 0; for (std::size_t j = i; j < i + win; ++j) s += v[j] * v[j];
            const double r = std::sqrt(s / win);
            lo = std::min(lo, r); hi = std::max(hi, r);
        }
        return (hi - lo) / std::max(1e-9, hi);
    };
    check(amMod(w) > amMod(t) + 0.1, "detune -> batimento (AM lenta)");
}

void testWaveMorph() {
    Chord saw, tri;
    saw.setParameter("wave", 0.0f);
    tri.setParameter("wave", 1.0f);
    saw.setParameter("chord", 0.0f);
    tri.setParameter("chord", 0.0f);
    saw.setParameter("voices", 2.0f);
    tri.setParameter("voices", 2.0f);
    saw.setParameter("detune", 0.0f);
    tri.setParameter("detune", 0.0f);
    const auto s = run(saw, 400);
    const auto t = run(tri, 400);
    // serra tem harmônicos pares E ímpares fortes; triângulo cai muito
    // mais rápido -> menos energia no 2º harmônico relativa à fundamental
    const double s2 = mag(s, 220.0) / std::max(1e-9, mag(s, 110.0));
    const double t2 = mag(t, 220.0) / std::max(1e-9, mag(t, 110.0));
    check(s2 > t2, "serra tem mais 2º harmônico que triângulo");
}

void testOneVoltPerOctave() {
    Chord ch;
    ch.setParameter("chord", 0.0f);
    ch.setParameter("voices", 2.0f);
    ch.setParameter("detune", 0.0f);
    const auto lo = run(ch, 300, 0.0f);
    const auto hi = run(ch, 300, 1.0f);   // +1 oitava
    check(mag(lo, 110.0) > mag(lo, 220.0) && mag(hi, 220.0) > mag(hi, 110.0),
          "pitch +1 -> frequência dobra");
}

void testChordCv() {
    Chord ch;
    ch.setParameter("chord", 0.0f);
    ch.setParameter("voices", 3.0f);
    ch.setParameter("detune", 0.0f);
    // chord_cv desloca do índice 0 (uníssono) pra ~3 (maior)
    const auto o = run(ch, 400, 0.0f, 3.0f / 9.0f, true);
    check(mag(o, semi(110.0, 4)) > mag(o, semi(110.0, 5)) * 3,
          "chord_cv seleciona o acorde ao vivo");
}

void testDeterminismAndFinite() {
    Chord a, b;
    for (Chord* c : {&a, &b}) {
        c->setParameter("chord", 0.5f);
        c->setParameter("detune", 0.5f);
        c->setParameter("drift", 0.9f);
        c->setParameter("wave", 0.4f);
    }
    const auto oa = run(a, 300, 0.25f);
    const auto ob = run(b, 300, 0.25f);
    bool same = oa.size() == ob.size();
    for (std::size_t k = 0; k < oa.size(); ++k) {
        EXPECT(std::isfinite(oa[k]));
        if (same && oa[k] != ob[k]) same = false;
    }
    EXPECT(same);
    check(rms(oa) > 0.02, "produz som");
    double pk = 0; for (float x : oa) pk = std::max(pk, std::fabs((double)x));
    check(pk <= 1.06, "saída contida (≤ ~1,05)");
}

void testVoiceLeading() {
    // troca de acorde no meio do render. `voicing=1` -> as vozes DESLIZAM
    // pro tom mais próximo (~40 ms); `voicing=0` -> saltam. As saídas
    // divergem na janela pós-troca; ambas finitas e determinísticas.
    auto renderSwitch = [](float voicing) {
        Chord ch;
        ch.setParameter("freq", 220.0f);
        ch.setParameter("voices", 4.0f);
        ch.setParameter("detune", 0.0f);
        ch.setParameter("voicing", voicing);
        ch.prepare(kSr, kB);
        std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, kB));
        AudioBlock pv(kSr, 1, kB), cv(kSr, 1, kB);
        std::vector<float> o;
        for (int b = 0; b < 300; ++b) {
            // bloco 0-149: dim {0,3,6,9}; 150+: add9 {0,4,7,14}
            const float cc = b < 150 ? 8.0f / 9.0f : 9.0f / 9.0f;
            for (std::size_t k = 0; k < kB; ++k) { pv.at(0, k) = 0.0f; cv.at(0, k) = cc; }
            std::vector<const AudioBlock*> ins{&pv, &cv, nullptr};
            ch.process(ins, out);
            for (std::size_t k = 0; k < kB; ++k) o.push_back(out[0].at(0, k));
        }
        return o;
    };
    const auto par = renderSwitch(0.0f);
    const auto vl0 = renderSwitch(1.0f);
    const auto vl1 = renderSwitch(1.0f);
    // determinismo do modo voice-leading
    bool same = vl0.size() == vl1.size();
    for (std::size_t k = 0; same && k < vl0.size(); ++k)
        if (vl0[k] != vl1[k]) same = false;
    EXPECT(same);
    // divergência na janela ~40 ms depois da troca (bloco 150)
    const std::size_t a = 150 * kB, b = 150 * kB + static_cast<std::size_t>(kSr * 0.04);
    double d = 0.0;
    for (std::size_t k = a; k < b && k < par.size(); ++k)
        d = std::max(d, std::fabs((double)par[k] - vl1[k]));
    check(d > 0.05, "voicing=1 desliza (difere do salto paralelo pós-troca)");
    for (float x : vl1) EXPECT(std::isfinite(x));
}

void testInGraph() {
    SignalGraph g;
    const auto seq = g.add(std::make_unique<StepSequencer>());
    g.node(seq).setParameter("rate", 3.0f);
    const auto chord = g.add(std::make_unique<Chord>());
    g.node(chord).setParameter("chord", 0.4f);
    const auto flt = g.add(std::make_unique<Filter>());
    g.connect(seq, 0, chord, 0);      // pitch -> chord pitch
    g.connect(chord, 0, flt, 0);      // chord -> filter
    g.prepare(kSr, 1, kB);
    AudioBlock out(kSr, 1, kB);
    float pk = 0.0f;
    for (int b = 0; b < 1200; ++b) {
        g.process(out, flt, 3);
        for (std::size_t k = 0; k < kB; ++k) {
            EXPECT(std::isfinite(out.at(0, k)));
            pk = std::max(pk, std::fabs(out.at(0, k)));
        }
    }
    check(pk > 0.05f, "CHORD toca o acorde no grafo");
}

void testPanel() {
    Chord ch;
    const std::string problem = validatePanel(ch);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(ch.panel().widgets.size() >= 12);
    std::cout << renderAscii(ch);
}

}  // namespace

int main() {
    testMajorChord();
    testMinorChord();
    testVoiceCount();
    testInversion();
    testDetuneBeats();
    testWaveMorph();
    testOneVoltPerOctave();
    testChordCv();
    testDeterminismAndFinite();
    testVoiceLeading();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_chord: OK\n";
    return g_failures == 0 ? 0 : 1;
}
