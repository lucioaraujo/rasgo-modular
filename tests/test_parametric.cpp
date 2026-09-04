// Teste isolado do Módulo 13 (PARAMETRIC) - antes de entrar num patch.
// Critérios do dossiê `dossies/13_parametric.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Parametric.hpp"
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
constexpr std::size_t kBlock = 128;

// ganho (dB) do EQ numa frequência: entra senoide, mede RMS saída/entrada
// em regime (descarta transiente).
float gainDbAt(Parametric& eq, const float freq) {
    eq.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    const int total = 24000;
    double sumIn = 0.0, sumOut = 0.0;
    int n = 0, phase = 0;
    while (n < total) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            // amplitude pequena: o soft-clip de segurança (±1) não deve
            // interferir na medida de resposta em frequência
            in.at(0, i) = 0.2f * std::sin(6.2831853f * freq
                                          * static_cast<float>(phase++)
                                          / kSampleRate);
        }
        eq.process(ins, out);
        if (n > 8000)  // pula o transiente
            for (std::size_t i = 0; i < kBlock; ++i) {
                sumIn += in.at(0, i) * in.at(0, i);
                sumOut += out[0].at(0, i) * out[0].at(0, i);
            }
        n += static_cast<int>(kBlock);
    }
    return 20.0f * std::log10(std::sqrt(sumOut / sumIn));
}

void allOff(Parametric& eq) {
    for (const char* n : {"1", "2", "3", "4"})
        eq.setParameter(std::string("type") + n, 0.0f);
}

void testFlatWhenOff() {
    Parametric eq;
    allOff(eq);
    eq.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    std::uint32_t r = 12345u;
    bool exact = true;
    for (int b = 0; b < 200; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            r ^= r << 13; r ^= r >> 17; r ^= r << 5;
            in.at(0, i) = static_cast<float>(r & 0xFFFF) / 32768.0f - 1.0f;
        }
        eq.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            if (std::fabs(out[0].at(0, i) - in.at(0, i)) > 1.0e-6f) exact = false;
    }
    EXPECT(exact);  // todos os estágios Off -> passa intacto
}

void testPeakBoost() {
    Parametric eq;
    allOff(eq);
    eq.setParameter("type2", 3.0f);   // Peak
    eq.setParameter("freq2", 1000.0f);
    eq.setParameter("gain2", 12.0f);
    eq.setParameter("q2", 3.0f);
    const float atPeak = gainDbAt(eq, 1000.0f);
    const float atFar = gainDbAt(eq, 100.0f);
    check(std::fabs(atPeak - 12.0f) < 1.5f, "Peak +12 dB dá ~+12 dB na frequência");
    check(std::fabs(atFar) < 1.5f, "longe do peak o ganho é ~0 dB");
}

void testPeakCut() {
    Parametric eq;
    allOff(eq);
    eq.setParameter("type1", 3.0f);
    eq.setParameter("freq1", 2000.0f);
    eq.setParameter("gain1", -18.0f);
    eq.setParameter("q1", 4.0f);
    check(gainDbAt(eq, 2000.0f) < -10.0f, "Peak -18 dB corta forte na frequência");
}

void testLowCut() {
    Parametric eq;
    allOff(eq);
    eq.setParameter("type1", 1.0f);   // LowCut
    eq.setParameter("freq1", 500.0f);
    eq.setParameter("q1", 0.7f);
    check(gainDbAt(eq, 80.0f) < -12.0f, "LowCut atenua bem abaixo do corte");
    check(std::fabs(gainDbAt(eq, 4000.0f)) < 1.5f, "LowCut passa bem acima do corte");
}

void testCutSlopes() {
    // LowCut @ 500 Hz: slope 1 (12 dB/oct) vs slope 3 (48 dB/oct) - uma
    // oitava abaixo do corte a inclinação de 48 dB atenua muito mais.
    Parametric s12, s48;
    for (Parametric* eq : {&s12, &s48}) {
        allOff(*eq);
        eq->setParameter("type1", 1.0f);   // LowCut
        eq->setParameter("freq1", 500.0f);
        eq->setParameter("q1", 0.7f);
    }
    s12.setParameter("slope1", 1.0f);
    s48.setParameter("slope1", 3.0f);
    const float at250_12 = gainDbAt(s12, 250.0f);
    const float at250_48 = gainDbAt(s48, 250.0f);
    check(at250_48 < at250_12 - 20.0f,
          "slope 48 dB/oct atenua muito mais que 12 dB/oct uma oitava abaixo");
    check(std::fabs(gainDbAt(s48, 4000.0f)) < 1.5f,
          "slope 48 ainda passa transparente acima do corte");
}

void testLowShelf() {
    Parametric eq;
    allOff(eq);
    eq.setParameter("type1", 2.0f);   // LowShelf
    eq.setParameter("freq1", 200.0f);
    eq.setParameter("gain1", 6.0f);
    check(std::fabs(gainDbAt(eq, 50.0f) - 6.0f) < 1.5f, "LowShelf +6 dB nos graves");
    check(std::fabs(gainDbAt(eq, 6000.0f)) < 1.5f, "LowShelf não toca nos agudos");
}

void testSweepMovesBands() {
    Parametric eq;
    allOff(eq);
    eq.setParameter("type2", 3.0f);
    eq.setParameter("freq2", 500.0f);
    eq.setParameter("gain2", 12.0f);
    eq.setParameter("q2", 3.0f);
    const float noSweep = gainDbAt(eq, 500.0f);
    // sweep +1 oitava -> o peak deve estar agora em ~1000 Hz, não 500
    eq.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), sw(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &sw, nullptr};
    double sumIn = 0.0, sumOut = 0.0;
    int n = 0, phase = 0;
    while (n < 24000) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            in.at(0, i) = 0.2f * std::sin(6.2831853f * 1000.0f
                                          * static_cast<float>(phase++)
                                          / kSampleRate);
            sw.at(0, i) = 1.0f;  // +1 oitava
        }
        eq.process(ins, out);
        if (n > 8000)
            for (std::size_t i = 0; i < kBlock; ++i) {
                sumIn += in.at(0, i) * in.at(0, i);
                sumOut += out[0].at(0, i) * out[0].at(0, i);
            }
        n += static_cast<int>(kBlock);
    }
    const float at1kWithSweep = 20.0f * std::log10(std::sqrt(sumOut / sumIn));
    check(at1kWithSweep > 6.0f, "sweep +1 oit move o peak de 500 para ~1000 Hz");
    check(noSweep > 6.0f, "sem sweep o peak está em 500 Hz");
}

void testStability() {
    Parametric eq;
    eq.setParameter("type1", 1.0f);
    eq.setParameter("type2", 3.0f);
    eq.setParameter("gain2", 24.0f);
    eq.setParameter("q2", 10.0f);
    eq.setParameter("type3", 4.0f);
    eq.setParameter("gain3", -24.0f);
    eq.setParameter("type4", 5.0f);
    eq.setParameter("drive", 0.8f);
    eq.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), sw(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &sw, nullptr};
    std::uint32_t r = 99u;
    for (int b = 0; b < 5000; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            r ^= r << 13; r ^= r >> 17; r ^= r << 5;
            in.at(0, i) = (static_cast<float>(r & 0xFFFF) / 32768.0f - 1.0f) * 0.7f;
        }
        sw.at(0, 0) = std::sin(b * 0.03f) * 2.5f;
        eq.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(out[0].at(0, i)) && std::fabs(out[0].at(0, i)) <= 1.05f);
    }
}

void testDeterminism() {
    Parametric a, b;
    for (Parametric* eq : {&a, &b}) {
        eq->setParameter("type1", 2.0f);
        eq->setParameter("gain1", 4.0f);
        eq->setParameter("type3", 3.0f);
        eq->setParameter("gain3", -6.0f);
        eq->setParameter("drive", 0.3f);
    }
    a.prepare(kSampleRate, kBlock);
    b.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> oa(1, AudioBlock(kSampleRate, 1, kBlock));
    std::vector<AudioBlock> ob(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    std::uint32_t r = 7u;
    bool identical = true;
    for (int bk = 0; bk < 500; ++bk) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            r ^= r << 13; r ^= r >> 17; r ^= r << 5;
            in.at(0, i) = static_cast<float>(r & 0xFFFF) / 32768.0f - 1.0f;
        }
        a.process(ins, oa);
        b.process(ins, ob);
        for (std::size_t i = 0; i < kBlock; ++i)
            if (oa[0].at(0, i) != ob[0].at(0, i)) identical = false;
    }
    EXPECT(identical);
}

void testInGraph() {
    SignalGraph graph;
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 150.0f);
    const auto eq = graph.add(std::make_unique<Parametric>());
    graph.node(eq).setParameter("type2", 3.0f);
    graph.node(eq).setParameter("gain2", 9.0f);
    graph.connect(voice, 1, eq, 0);
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, eq, 0);
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 0.1f);
}

void testPanel() {
    Parametric eq;
    const std::string problem = validatePanel(eq);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(eq.panel().widgets.size() >= 20);
    std::cout << renderAscii(eq);
}

}  // namespace

int main() {
    testFlatWhenOff();
    testPeakBoost();
    testPeakCut();
    testLowCut();
    testCutSlopes();
    testLowShelf();
    testSweepMovesBands();
    testStability();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular PARAMETRIC tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
