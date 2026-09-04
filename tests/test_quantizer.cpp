// Teste isolado do Módulo 12 (QUANTIZER) - antes de entrar num patch.
// Critérios do dossiê `dossies/12_quantizer.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Quantizer.hpp"
#include "dsp/TuringLoop.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "io/AsciiPanel.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool condition, const char* const expression) {
    if (!condition) {
        std::cerr << "CHECK FALHOU: " << expression << '\n';
        ++g_failures;
    }
}
#define EXPECT(x) check((x), #x)

constexpr float kSampleRate = 48000.0f;
constexpr std::size_t kBlock = 64;

// devolve `pitch` (oitavas) para cada valor de cv em `cvs`, deixando o
// módulo assentar entre valores.
std::vector<float> quantizeRamp(Quantizer& q, const std::vector<float>& cvs) {
    q.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock cv(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&cv, nullptr, nullptr};
    std::vector<float> pitches;
    for (const float value : cvs) {
        for (int b = 0; b < 8; ++b) {
            for (std::size_t i = 0; i < kBlock; ++i) cv.at(0, i) = value;
            q.process(ins, out);
        }
        pitches.push_back(out[0].at(0, kBlock - 1));
    }
    return pitches;
}

bool inMajorScale(int semi) {
    const int deg = ((semi % 12) + 12) % 12;
    for (int d : {0, 2, 4, 5, 7, 9, 11})
        if (deg == d) return true;
    return false;
}

void testSnapsToScale() {
    Quantizer q;
    q.setParameter("scale", 1.0f);   // Major
    q.setParameter("root", 0.0f);
    q.setParameter("range", 3.0f);
    q.setParameter("glide", 0.0f);
    q.setParameter("hysteresis", 0.0f);
    std::vector<float> cvs;
    for (int i = -20; i <= 20; ++i) cvs.push_back(i / 20.0f);
    const auto p = quantizeRamp(q, cvs);
    bool allInScale = true;
    for (const float oct : p) {
        const float semiF = oct * 12.0f;
        const int semi = static_cast<int>(std::lround(semiF));
        if (std::fabs(semiF - semi) > 1.0e-3f) allInScale = false;  // é semitom inteiro
        if (!inMajorScale(semi)) allInScale = false;
    }
    EXPECT(allInScale);
    // monotônico não-decrescente ao subir a CV
    bool monotone = true;
    for (std::size_t i = 1; i < p.size(); ++i)
        if (p[i] < p[i - 1] - 1.0e-4f) monotone = false;
    EXPECT(monotone);
}

void testRootShifts() {
    Quantizer q0, q3;
    q0.setParameter("scale", 1.0f);
    q0.setParameter("range", 2.0f);
    q0.setParameter("hysteresis", 0.0f);
    q3.setParameter("scale", 1.0f);
    q3.setParameter("root", 3.0f);
    q3.setParameter("range", 2.0f);
    q3.setParameter("hysteresis", 0.0f);
    std::vector<float> cvs;
    for (int i = 0; i <= 20; ++i) cvs.push_back(i / 20.0f);
    const auto a = quantizeRamp(q0, cvs);
    const auto b = quantizeRamp(q3, cvs);
    // a escala com root 3 deve conter notas ≡ (grau maior + 3) mod 12
    bool ok = true;
    for (const float oct : b) {
        const int semi = static_cast<int>(std::lround(oct * 12.0f));
        const int deg = ((semi - 3) % 12 + 12) % 12;
        bool m = false;
        for (int d : {0, 2, 4, 5, 7, 9, 11}) if (deg == d) m = true;
        if (!m) ok = false;
    }
    EXPECT(ok);
    (void)a;
}

void testSampleHold() {
    Quantizer q;
    q.setParameter("scale", 0.0f);   // cromática (toda mudança de cv poderia mover)
    q.setParameter("range", 4.0f);
    q.setParameter("glide", 0.0f);
    q.setParameter("hysteresis", 0.0f);
    q.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock cv(kSampleRate, 1, kBlock), trig(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&cv, nullptr, &trig};
    std::vector<float> pitches;
    for (int b = 0; b < 400; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            cv.at(0, i) = std::sin(b * 0.3f) * 0.8f;  // varia sempre
            trig.at(0, i) = (b % 40 == 0 && i < 4) ? 1.0f : 0.0f;
        }
        q.process(ins, out);
        pitches.push_back(out[0].at(0, kBlock - 1));
    }
    // entre gatilhos (b não múltiplo de 40) o pitch não muda
    int changesBetweenTriggers = 0;
    for (std::size_t b = 1; b < pitches.size(); ++b)
        if (b % 40 != 0 && b % 40 != 1
            && std::fabs(pitches[b] - pitches[b - 1]) > 1.0e-5f)
            ++changesBetweenTriggers;
    EXPECT(changesBetweenTriggers == 0);
    // ...e muda em alguns gatilhos
    int distinct = 0;
    for (std::size_t b = 1; b < pitches.size(); ++b)
        if (std::fabs(pitches[b] - pitches[b - 1]) > 1.0e-5f) ++distinct;
    EXPECT(distinct >= 3);
}

void testGlideIsGradual() {
    Quantizer q;
    q.setParameter("scale", 1.0f);
    q.setParameter("range", 5.0f);
    q.setParameter("glide", 0.5f);
    q.setParameter("hysteresis", 0.0f);
    q.prepare(kSampleRate, 256);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 256));
    AudioBlock cv(kSampleRate, 1, 256);
    std::vector<const AudioBlock*> ins{&cv, nullptr, nullptr};
    float prev = 0.0f, maxJump = 0.0f;
    for (int b = 0; b < 4000; ++b) {
        const float v = (b / 500) % 2 == 0 ? -0.9f : 0.9f;  // saltos grandes
        for (std::size_t i = 0; i < 256; ++i) cv.at(0, i) = v;
        q.process(ins, out);
        for (std::size_t i = 0; i < 256; ++i) {
            const float p = out[0].at(0, i);
            maxJump = std::max(maxJump, std::fabs(p - prev));
            prev = p;
        }
    }
    check(maxJump < 0.02f, "glide suaviza os saltos de altura");
}

void testDeterminism() {
    Quantizer a, b;
    for (Quantizer* q : {&a, &b}) {
        q->setParameter("scale", 3.0f);
        q->setParameter("root", 2.0f);
        q->setParameter("range", 3.0f);
        q->setParameter("glide", 0.2f);
    }
    std::vector<float> cvs;
    for (int i = 0; i < 60; ++i) cvs.push_back(std::sin(i * 0.4f));
    const auto pa = quantizeRamp(a, cvs);
    const auto pb = quantizeRamp(b, cvs);
    bool identical = pa.size() == pb.size();
    for (std::size_t i = 0; identical && i < pa.size(); ++i)
        if (pa[i] != pb[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    // TURING.cv -> QUANTIZER.cv -> voz.rate_mod  (melodia em escala)
    SignalGraph graph;
    const auto tur = graph.add(std::make_unique<TuringLoop>());
    graph.node(tur).setParameter("rate", 6.0f);
    graph.node(tur).setParameter("lock", 0.6f);
    graph.node(tur).setParameter("range", 1.0f);
    const auto quant = graph.add(std::make_unique<Quantizer>());
    graph.node(quant).setParameter("scale", 7.0f);  // pentatônica menor
    graph.node(quant).setParameter("range", 2.0f);
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 220.0f);
    graph.connect(tur, 0, quant, 0);        // cv -> cv
    graph.connect(quant, 0, voice, 0);      // pitch (oitavas) -> rate_mod
    graph.prepare(kSampleRate, 1, 128);
    AudioBlock out(kSampleRate, 1, 128);
    float peak = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, voice, 1);  // "bi"
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 0.3f);
}

void testPanel() {
    Quantizer q;
    const std::string problem = validatePanel(q);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(q.panel().widgets.size() >= 11);
    EXPECT(Quantizer::scales().size() == 12);
    std::cout << renderAscii(q);
}

}  // namespace

int main() {
    testSnapsToScale();
    testRootShifts();
    testSampleHold();
    testGlideIsGradual();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular QUANTIZER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
