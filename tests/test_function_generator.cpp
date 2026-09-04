// Teste isolado do Módulo 1 (FUNCTION) - antes de entrar num patch.
// Critérios do dossiê `dossies/01_gerador_de_funcao.md` §7.

#include "core/SignalGraph.hpp"
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

// Renderiza N amostras da saída pedida ("uni"/"bi", canal 0) num vetor.
std::vector<float> render(FunctionGenerator& gen, const std::size_t samples,
                          const std::size_t outPort) {
    std::vector<AudioBlock> outputs(2, AudioBlock(kSampleRate, 1, kBlock));
    std::vector<const AudioBlock*> inputs(3, nullptr);
    std::vector<float> result;
    result.reserve(samples);
    gen.prepare(kSampleRate, kBlock);
    while (result.size() < samples) {
        gen.process(inputs, outputs);
        for (std::size_t i = 0; i < kBlock && result.size() < samples; ++i)
            result.push_back(outputs[outPort].at(0, i));
    }
    return result;
}

// frequência fundamental por contagem de cruzamentos de zero ascendentes
float measuredFrequency(const std::vector<float>& signal) {
    int crossings = 0;
    std::size_t first = 0, last = 0;
    for (std::size_t i = 1; i < signal.size(); ++i) {
        if (signal[i - 1] < 0.0f && signal[i] >= 0.0f) {
            if (crossings == 0) first = i;
            last = i;
            ++crossings;
        }
    }
    if (crossings < 2) return 0.0f;
    const float cycles = static_cast<float>(crossings - 1);
    const float seconds = static_cast<float>(last - first) / kSampleRate;
    return cycles / seconds;
}

double magnitudeAt(const std::vector<float>& signal, const double freq) {
    double re = 0.0, im = 0.0;
    const double w = 2.0 * M_PI * freq / kSampleRate;
    for (std::size_t n = 0; n < signal.size(); ++n) {
        re += signal[n] * std::cos(w * n);
        im -= signal[n] * std::sin(w * n);
    }
    return std::sqrt(re * re + im * im) / signal.size();
}

void testFrequencyTracks() {
    for (const float hz : {0.5f, 20.0f, 220.0f, 2000.0f}) {
        FunctionGenerator gen;
        gen.setParameter("rate", hz);
        gen.setParameter("slope", 0.5f);
        const auto sig = render(gen, static_cast<std::size_t>(kSampleRate * 4), 1);
        const float measured = measuredFrequency(sig);
        check(std::fabs(measured - hz) / hz < 0.02f, "frequência bate com rate (±2%)");
    }
}

void testShapeBounds() {
    for (const float slope : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) {
        FunctionGenerator gen;
        gen.setParameter("rate", 300.0f);
        gen.setParameter("slope", slope);
        const auto uni = render(gen, 20000, 0);
        const auto bi = render(gen, 20000, 1);
        bool ok = true;
        for (const float v : uni)
            if (!std::isfinite(v) || v < -0.001f || v > 1.001f) ok = false;
        for (const float v : bi)
            if (!std::isfinite(v) || v < -1.5f || v > 1.5f) ok = false;
        check(ok, "uni em [0,1], bi limitado, sem NaN em toda a varredura de slope");
    }
}

void testDriftDeterminism() {
    // drift = 0 -> dois renders byte-idênticos
    FunctionGenerator a, b;
    a.setParameter("rate", 100.0f);
    b.setParameter("rate", 100.0f);
    const auto ra = render(a, 30000, 1);
    const auto rb = render(b, 30000, 1);
    bool identical = ra.size() == rb.size();
    for (std::size_t i = 0; identical && i < ra.size(); ++i)
        if (ra[i] != rb[i]) identical = false;
    EXPECT(identical);

    // drift > 0 -> reprodutível com a mesma seed (prepare reseta o rng)
    FunctionGenerator c, d;
    c.setParameter("rate", 100.0f);
    c.setParameter("drift", 0.8f);
    d.setParameter("rate", 100.0f);
    d.setParameter("drift", 0.8f);
    const auto rc = render(c, 60000, 1);
    const auto rd = render(d, 60000, 1);
    bool repro = rc.size() == rd.size();
    for (std::size_t i = 0; repro && i < rc.size(); ++i)
        if (rc[i] != rd[i]) repro = false;
    EXPECT(repro);

    // drift > 0 realmente muda o sinal em relação a drift = 0
    FunctionGenerator e;
    e.setParameter("rate", 100.0f);
    e.setParameter("drift", 0.8f);
    const auto re = render(e, 60000, 1);
    bool different = false;
    for (std::size_t i = 0; i < re.size() && i < ra.size(); ++i)
        if (std::fabs(re[i] - ra[i]) > 1.0e-4f) different = true;
    EXPECT(different);
}

void testAntiAliasing() {
    // dente-de-serra a ~1000 Hz: energia nas frequências de alias
    // (imagens rebatidas do harmônico acima de Nyquist) deve ficar baixa.
    FunctionGenerator gen;
    const float f0 = 1000.0f;
    gen.setParameter("rate", f0);
    gen.setParameter("slope", 1.0f);  // serra pura
    const auto sig = render(gen, 1 << 15, 1);

    const double fundamental = magnitudeAt(sig, f0);
    // 24º harmônico = 24 kHz = Nyquist; o 25º (25 kHz) rebate em 23 kHz.
    const double alias23k = magnitudeAt(sig, 23000.0);
    const double alias22k = magnitudeAt(sig, 22000.0);  // do 26º harmônico
    const double aliasDb23 = 20.0 * std::log10(alias23k / fundamental + 1e-12);
    const double aliasDb22 = 20.0 * std::log10(alias22k / fundamental + 1e-12);
    std::cout << "  alias @23k = " << aliasDb23 << " dB, @22k = " << aliasDb22
              << " dB (rel. fundamental)\n";
    check(aliasDb23 < -40.0, "alias @23 kHz abaixo de -40 dB (PolyBLEP)");
    check(aliasDb22 < -40.0, "alias @22 kHz abaixo de -40 dB (PolyBLEP)");
}

void testNoAllocInProcess() {
    // Sem instrumentação real de alocador aqui - mas garante que process()
    // roda muitos blocos sem crescer estado nem lançar.
    FunctionGenerator gen;
    gen.setParameter("rate", 440.0f);
    gen.setParameter("drift", 0.5f);
    gen.setParameter("sync_enable", 1.0f);
    std::vector<AudioBlock> outputs(2, AudioBlock(kSampleRate, 2, kBlock));
    AudioBlock syncBlock(kSampleRate, 2, kBlock);
    std::vector<const AudioBlock*> inputs{nullptr, nullptr, &syncBlock};
    gen.prepare(kSampleRate, kBlock);
    for (int b = 0; b < 5000; ++b) {
        if (b % 7 == 0) syncBlock.at(0, 0) = 1.0f;
        else syncBlock.at(0, 0) = 0.0f;
        gen.process(inputs, outputs);
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(outputs[1].at(0, i)));
    }
}

void testInSignalGraph() {
    // FUNCTION -> Cable -> saída, com uma ruptura. Prova a integração.
    SignalGraph graph;
    const auto osc = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(osc).setParameter("rate", 110.0f);
    graph.node(osc).setParameter("slope", 0.7f);
    // um nó de passagem só pra ter um Cable rompível
    struct Passthrough final : Signal {
        Passthrough() : Signal({{"in", PortKind::Audio, ""}},
                               {{"out", PortKind::Audio, ""}}) {}
        std::string type() const override { return "TEST.PASS"; }
        void process(const std::vector<const AudioBlock*>& in,
                     std::vector<AudioBlock>& out) noexcept override {
            if (in[0]) out[0].copyFrom(*in[0]);
            else out[0].clear();
        }
    };
    const auto pass = graph.add(std::make_unique<Passthrough>());
    auto& cable = graph.connect(osc, 1, pass, 0);  // porta "bi"
    graph.prepare(kSampleRate, 1, kBlock);

    AudioBlock out(kSampleRate, 1, kBlock);
    graph.process(out, pass, 0);
    float peak = 0.0f;
    for (std::size_t i = 0; i < kBlock; ++i) peak = std::max(peak, std::fabs(out.at(0, i)));
    EXPECT(peak > 0.3f);  // o oscilador está soando através do cabo

    cable.rupture();
    for (int b = 0; b < 3000; ++b) graph.process(out, pass, 0);
    float scarPeak = 0.0f;
    for (std::size_t i = 0; i < kBlock; ++i) scarPeak = std::max(scarPeak, std::fabs(out.at(0, i)));
    EXPECT(scarPeak < 1.0e-4f);  // a cicatriz decaiu
}

void testPanel() {
    FunctionGenerator gen;
    const std::string problem = validatePanel(gen);
    check(problem.empty(), "descrição de painel fecha (todo bind resolve)");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    const Panel p = gen.panel();
    EXPECT(p.hp > 0);
    EXPECT(p.widgets.size() >= 9);  // 4 controles + 3 in + 2 out no mínimo
    std::cout << renderAscii(gen);

    // auto-layout do padrão também fecha
    struct Bare final : Signal {
        Bare() : Signal({{"in", PortKind::Audio, ""}},
                        {{"out", PortKind::Audio, ""}},
                        {{"amount", 0.0f, 10.0f, 1.0f, ""}}) {}
        std::string type() const override { return "BARE"; }
        void process(const std::vector<const AudioBlock*>&,
                     std::vector<AudioBlock>& o) noexcept override { o[0].clear(); }
    } bare;
    EXPECT(validatePanel(bare).empty());
}

}  // namespace

int main() {
    testFrequencyTracks();
    testShapeBounds();
    testDriftDeterminism();
    testAntiAliasing();
    testNoAllocInProcess();
    testInSignalGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular FUNCTION generator tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
