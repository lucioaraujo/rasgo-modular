// Teste isolado do Módulo 4 (DECISION) - antes de entrar num patch.
// Critérios do dossiê `dossies/04_decisao.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Decision.hpp"
#include "dsp/Filter.hpp"
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

// Dispara N passos por trigger externo (1 amostra por bloco: sobe, desce),
// devolvendo o valor de cada saída logo APÓS cada passo.
struct StepResult {
    std::vector<float> x, y, gate;
};

StepResult stepN(Decision& d, const int steps) {
    d.prepare(kSampleRate, 1);
    std::vector<AudioBlock> out;
    out.emplace_back(kSampleRate, 1, 1);
    out.emplace_back(kSampleRate, 1, 1);
    out.emplace_back(kSampleRate, 1, 1);
    AudioBlock trig(kSampleRate, 1, 1);
    std::vector<const AudioBlock*> in{&trig, nullptr, nullptr};
    StepResult r;
    for (int i = 0; i < steps; ++i) {
        trig.at(0, 0) = 1.0f;  // borda de subida -> um passo
        d.process(in, out);
        r.x.push_back(out[0].at(0, 0));
        r.y.push_back(out[1].at(0, 0));
        r.gate.push_back(out[2].at(0, 0));
        trig.at(0, 0) = 0.0f;  // volta pra baixo (arma a próxima borda)
        d.process(in, out);
    }
    return r;
}

double mean(const std::vector<float>& v) {
    double s = 0.0;
    for (const float x : v) s += x;
    return s / static_cast<double>(v.size());
}

double stddev(const std::vector<float>& v) {
    const double m = mean(v);
    double s = 0.0;
    for (const float x : v) s += (x - m) * (x - m);
    return std::sqrt(s / static_cast<double>(v.size()));
}

void testBiasCalibration() {
    for (const float bias : {0.0f, 1.0f, 0.5f, 0.8f}) {
        Decision d;
        d.setParameter("bias", bias);
        const auto r = stepN(d, 4000);
        int high = 0;
        for (const float g : r.gate)
            if (g > 0.5f) ++high;
        const double frac = static_cast<double>(high) / r.gate.size();
        if (bias == 0.0f)
            EXPECT(high == 0);
        else if (bias == 1.0f)
            EXPECT(high == static_cast<int>(r.gate.size()));
        else
            check(std::fabs(frac - bias) < 0.05,
                  "frequencia de gate=1 bate com bias (+-5%)");
    }
}

void testSpreadZero() {
    Decision d;
    d.setParameter("spread", 0.0f);
    d.setParameter("slew", 0.0f);
    const auto r = stepN(d, 500);
    bool allZero = true;
    for (const float x : r.x)
        if (std::fabs(x) > 1.0e-6f) allZero = false;
    for (const float y : r.y)
        if (std::fabs(y) > 1.0e-6f) allZero = false;
    EXPECT(allZero);
}

void testDistributionShape() {
    Decision uni;
    uni.setParameter("spread", 1.0f);
    uni.setParameter("shape", 0.0f);
    const auto ru = stepN(uni, 6000);
    // uniforme em [-1,1]: media ~0, desvio ~0,577
    check(std::fabs(mean(ru.x)) < 0.05, "distribuicao uniforme centrada em 0");
    check(std::fabs(stddev(ru.x) - 0.577) < 0.08, "desvio ~0,577 (uniforme)");

    Decision bell;
    bell.setParameter("spread", 1.0f);
    bell.setParameter("shape", 1.0f);
    const auto rb = stepN(bell, 6000);
    check(std::fabs(mean(rb.x)) < 0.05, "distribuicao sino centrada em 0");
    check(stddev(rb.x) < stddev(ru.x) - 0.15,
          "sino tem desvio claramente menor que a uniforme");
    check(stddev(rb.x) > 0.15, "sino nao colapsa no centro");
}

void testSteps() {
    Decision d;
    d.setParameter("spread", 1.0f);
    d.setParameter("steps", 3.0f);
    const auto r = stepN(d, 2000);
    const float levels[3] = {-1.0f, 0.0f, 1.0f};
    bool onGrid = true;
    int distinct[3] = {0, 0, 0};
    for (const float x : r.x) {
        bool matched = false;
        for (int k = 0; k < 3; ++k)
            if (std::fabs(x - levels[k]) < 1.0e-5f) {
                matched = true;
                ++distinct[k];
            }
        if (!matched) onGrid = false;
    }
    EXPECT(onGrid);
    EXPECT(distinct[0] > 0 && distinct[1] > 0 && distinct[2] > 0);
}

void testDejavuLoop() {
    Decision d;
    d.setParameter("spread", 1.0f);
    d.setParameter("shape", 0.0f);
    d.setParameter("slew", 0.0f);
    d.setParameter("dejavu", 1.0f);
    d.setParameter("loop_length", 4.0f);
    const auto r = stepN(d, 120);
    // depois do buffer cheio (16 passos) e travado, x tem periodo 4
    bool periodic = true;
    for (std::size_t i = 40; i + 4 < r.x.size(); ++i)
        if (std::fabs(r.x[i] - r.x[i + 4]) > 1.0e-6f) periodic = false;
    EXPECT(periodic);
    // ...e nao e trivialmente constante
    bool varies = false;
    for (std::size_t i = 40; i + 1 < r.x.size(); ++i)
        if (std::fabs(r.x[i] - r.x[i + 1]) > 1.0e-4f) varies = true;
    EXPECT(varies);
}

void testDeterminism() {
    Decision a, b;
    a.setParameter("spread", 1.0f);
    a.setParameter("shape", 0.5f);
    a.setParameter("dejavu", 0.4f);
    b.setParameter("spread", 1.0f);
    b.setParameter("shape", 0.5f);
    b.setParameter("dejavu", 0.4f);
    const auto ra = stepN(a, 3000);
    const auto rb = stepN(b, 3000);
    bool identical = ra.x.size() == rb.x.size();
    for (std::size_t i = 0; identical && i < ra.x.size(); ++i)
        if (ra.x[i] != rb.x[i] || ra.y[i] != rb.y[i]
            || ra.gate[i] != rb.gate[i])
            identical = false;
    EXPECT(identical);
}

void testSlewLimitsDerivative() {
    Decision d;
    d.setParameter("spread", 1.0f);
    d.setParameter("slew", 0.5f);   // ~250 ms
    d.setParameter("rate", 3.0f);
    d.prepare(kSampleRate, 64);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 1, 64));
    std::vector<const AudioBlock*> in{nullptr, nullptr, nullptr};  // clock interno
    float prev = 0.0f;
    float maxJump = 0.0f;
    for (int b = 0; b < 4000; ++b) {
        d.process(in, out);
        for (std::size_t i = 0; i < 64; ++i) {
            const float v = out[0].at(0, i);
            maxJump = std::max(maxJump, std::fabs(v - prev));
            prev = v;
        }
    }
    check(maxJump < 0.02f, "slew limita o salto por amostra de x");
}

void testNoAllocAndFinite() {
    Decision d;
    d.setParameter("spread", 1.0f);
    d.setParameter("shape", 0.7f);
    d.setParameter("dejavu", 0.5f);
    d.setParameter("slew", 0.3f);
    d.prepare(kSampleRate, 128);
    std::vector<AudioBlock> out(3, AudioBlock(kSampleRate, 2, 128));
    AudioBlock trig(kSampleRate, 2, 128);
    std::vector<const AudioBlock*> in{&trig, nullptr, nullptr};
    for (int b = 0; b < 5000; ++b) {
        trig.at(0, 0) = (b % 5 == 0) ? 1.0f : 0.0f;
        d.process(in, out);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out[0].at(0, i)));
            EXPECT(std::isfinite(out[1].at(0, i)));
            const float g = out[2].at(0, i);
            EXPECT(g == 0.0f || g == 1.0f);
        }
    }
}

void testInGraph() {
    // DECISION.x (clock interno) -> cutoff_mod de um FILTER; a voz é o
    // próprio y. Prova a integração e a modulação de parâmetro/porta.
    SignalGraph graph;
    const auto dec = graph.add(std::make_unique<Decision>());
    graph.node(dec).setParameter("rate", 6.0f);
    graph.node(dec).setParameter("spread", 1.0f);
    const auto filter = graph.add(std::make_unique<Filter>());
    graph.node(filter).setParameter("cutoff", 400.0f);
    graph.node(filter).setParameter("resonance", 0.7f);
    graph.connect(dec, 1, filter, 0);          // y -> in (material)
    graph.connect(dec, 0, filter, 1, false, 2.0f);  // x -> cutoff_mod
    graph.prepare(kSampleRate, 1, 128);

    AudioBlock out(kSampleRate, 1, 128);
    float peak = 0.0f;
    for (int b = 0; b < 400; ++b) {
        graph.process(out, filter, 3);  // saída "all"
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);  // algo está soando
}

void testPanel() {
    Decision d;
    const std::string problem = validatePanel(d);
    check(problem.empty(), "descricao de painel fecha (todo bind resolve)");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    const Panel p = d.panel();
    EXPECT(p.hp > 0);
    EXPECT(p.widgets.size() >= 14);
    std::cout << renderAscii(d);
}

}  // namespace

int main() {
    testBiasCalibration();
    testSpreadZero();
    testDistributionShape();
    testSteps();
    testDejavuLoop();
    testDeterminism();
    testSlewLimitsDerivative();
    testNoAllocAndFinite();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular DECISION tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
