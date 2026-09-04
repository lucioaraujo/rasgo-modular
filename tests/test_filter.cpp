// Teste isolado do Módulo 2 (FILTER) - critérios do dossiê
// `dossies/02_filtro.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
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
constexpr std::size_t kBlock = 128;

// Alimenta o filtro com uma senoide de `freq` e devolve o RMS da porta
// de saída `outPort` em regime.
float rmsResponse(Filter& f, const float freq, const std::size_t outPort) {
    f.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kBlock));
    AudioBlock in(kSr, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr, nullptr};
    double phase = 0.0;
    const double dp = 2.0 * M_PI * freq / kSr;
    double sumSq = 0.0;
    std::size_t counted = 0;
    const int blocks = 400;
    for (int b = 0; b < blocks; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            in.at(0, i) = static_cast<float>(std::sin(phase));
            phase += dp;
        }
        f.process(ins, out);
        if (b > blocks / 2)
            for (std::size_t i = 0; i < kBlock; ++i) {
                const float v = out[outPort].at(0, i);
                sumSq += static_cast<double>(v) * v;
                ++counted;
            }
    }
    return static_cast<float>(std::sqrt(sumSq / counted));
}

void testFrequencyResponse() {
    // spread = 0: low = LP, high = HP do mesmo corte (800 Hz).
    Filter f;
    f.setParameter("cutoff", 800.0f);
    f.setParameter("resonance", 0.1f);
    f.setParameter("spread", 0.0f);

    const float lowAt200 = rmsResponse(f, 200.0f, 0);   // LP passa
    const float lowAt5k = rmsResponse(f, 5000.0f, 0);   // LP corta
    const float highAt200 = rmsResponse(f, 200.0f, 2);  // HP corta
    const float highAt5k = rmsResponse(f, 5000.0f, 2);  // HP passa

    check(lowAt200 > lowAt5k * 4.0f, "LP: 200 Hz muito acima de 5 kHz");
    check(highAt5k > highAt200 * 4.0f, "HP: 5 kHz muito acima de 200 Hz");
    // banda passante perto de 0 dB (0,707 RMS pra senoide unitária)
    check(std::fabs(lowAt200 - 0.707f) < 0.15f, "LP banda passante ~0 dB");
}

void testFormantSpread() {
    // spread alto: low e high viram passa-banda afastados. A saída `low`
    // deve ter pico bem abaixo de 800 Hz e a `high` bem acima.
    Filter f;
    f.setParameter("cutoff", 800.0f);
    f.setParameter("resonance", 0.55f);
    f.setParameter("spread", 1.0f);  // ±2 oitavas -> 200 Hz e 3200 Hz

    const float lowAt200 = rmsResponse(f, 200.0f, 0);
    const float lowAt3200 = rmsResponse(f, 3200.0f, 0);
    const float highAt200 = rmsResponse(f, 200.0f, 2);
    const float highAt3200 = rmsResponse(f, 3200.0f, 2);
    check(lowAt200 > lowAt3200 * 2.0f, "spread: banda grave centrada em ~200 Hz");
    check(highAt3200 > highAt200 * 2.0f, "spread: banda aguda centrada em ~3200 Hz");
}

void testSelfOscillation() {
    // resonance ~1, entrada quase nula -> auto-oscila numa senoide na
    // frequência de corte.
    Filter f;
    f.setParameter("cutoff", 440.0f);
    f.setParameter("resonance", 1.0f);
    f.prepare(kSr, kBlock);
    std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kBlock));
    AudioBlock in(kSr, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr, nullptr};
    in.at(0, 0) = 0.01f;  // cutucão inicial
    std::vector<float> tail;
    for (int b = 0; b < 800; ++b) {
        f.process(ins, out);
        if (b == 0) in.at(0, 0) = 0.0f;
        if (b > 600)
            for (std::size_t i = 0; i < kBlock; ++i) tail.push_back(out[1].at(0, i));
    }
    float peak = 0.0f;
    int zc = 0;
    for (std::size_t i = 1; i < tail.size(); ++i) {
        peak = std::max(peak, std::fabs(tail[i]));
        if (tail[i - 1] < 0.0f && tail[i] >= 0.0f) ++zc;
    }
    check(peak > 0.05f && peak <= 1.0f, "auto-oscilação sustentada e limitada");
    const float freq = zc / (static_cast<float>(tail.size()) / kSr);
    check(std::fabs(freq - 440.0f) / 440.0f < 0.05f, "auto-oscila na frequência de corte (±5%)");
}

void testStabilityAndDeterminism() {
    // varredura de cutoff de 20 Hz a ~Nyquist com ruído, sem NaN nem estouro
    Filter a, b;
    a.setParameter("drive", 0.7f);
    b.setParameter("drive", 0.7f);
    auto run = [](Filter& f) {
        f.prepare(kSr, kBlock);
        std::vector<AudioBlock> out(4, AudioBlock(kSr, 1, kBlock));
        AudioBlock in(kSr, 1, kBlock);
        std::vector<const AudioBlock*> ins{&in, nullptr, nullptr, nullptr};
        std::uint32_t rng = 12345;
        std::vector<float> result;
        for (int blk = 0; blk < 2000; ++blk) {
            const float cutoff = 20.0f + (23000.0f * blk) / 2000.0f;
            f.setParameter("cutoff", cutoff);
            for (std::size_t i = 0; i < kBlock; ++i) {
                rng = rng * 1664525u + 1013904223u;
                in.at(0, i) = (static_cast<float>(rng >> 9) / 8388608.0f - 1.0f) * 0.5f;
            }
            f.process(ins, out);
            for (std::size_t i = 0; i < kBlock; i += 32)
                result.push_back(out[3].at(0, i));
        }
        return result;
    };
    const auto ra = run(a);
    const auto rb = run(b);
    bool finite = true, identical = ra.size() == rb.size();
    for (std::size_t i = 0; i < ra.size(); ++i) {
        if (!std::isfinite(ra[i]) || std::fabs(ra[i]) > 1.01f) finite = false;
        if (identical && ra[i] != rb[i]) identical = false;
    }
    EXPECT(finite);
    EXPECT(identical);
}

void testPanel() {
    Filter f;
    const std::string problem = validatePanel(f);
    check(problem.empty(), "painel do FILTER fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    std::cout << renderAscii(f);
}

void testInGraph() {
    // FUNCTION (voz) -> FILTER (cutoff modulado por outro FUNCTION) -> saída
    SignalGraph graph;
    const auto voice = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(voice).setParameter("rate", 120.0f);
    graph.node(voice).setParameter("slope", 0.7f);
    const auto lfo = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(lfo).setParameter("rate", 0.5f);
    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 600.0f);
    graph.node(filter).setParameter("resonance", 0.5f);

    struct Pass final : Signal {
        Pass() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}}) {}
        std::string type() const override { return "PASS"; }
        void process(const std::vector<const AudioBlock*>& i,
                     std::vector<AudioBlock>& o) noexcept override {
            if (i[0]) o[0].copyFrom(*i[0]); else o[0].clear();
        }
    };
    const auto sink = graph.add(std::make_unique<Pass>());

    graph.connect(voice, 1, filter, 0);          // bi -> in
    graph.connect(lfo, 1, filter, 1, false, 2.0f);  // bi -> cutoff_mod, ±2 oitavas
    graph.connect(filter, 3, sink, 0);           // all -> saída
    graph.prepare(kSr, 1, kBlock);

    AudioBlock out(kSr, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 200; ++b) {
        graph.process(out, sink, 0);
        for (std::size_t i = 0; i < kBlock; ++i) peak = std::max(peak, std::fabs(out.at(0, i)));
    }
    EXPECT(peak > 0.05f && peak <= 1.0f);
}

}  // namespace

int main() {
    testFrequencyResponse();
    testFormantSpread();
    testSelfOscillation();
    testStabilityAndDeterminism();
    testPanel();
    testInGraph();

    if (g_failures == 0) {
        std::cout << "RASGO Modular FILTER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
