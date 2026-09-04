// Teste isolado do Módulo 9 (MATTER) - antes de entrar num patch.
// Critérios do dossiê `dossies/09_matter.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Matter.hpp"
#include "dsp/EuclidClock.hpp"
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

// Bate uma vez (strike na amostra 0) e devolve `samples` da saída.
std::vector<float> strikeRing(Matter& m, const int samples,
                              const float continuousIn = 0.0f) {
    m.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), strike(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &strike, nullptr, nullptr};
    std::vector<float> r;
    int s = 0;
    while (static_cast<int>(r.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            in.at(0, i) = continuousIn;
            strike.at(0, i) = (s + static_cast<int>(i) == 1) ? 1.0f : 0.0f;
        }
        m.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.push_back(out[0].at(0, i));
        s += static_cast<int>(kBlock);
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * v[i];
    return std::sqrt(s / static_cast<double>(b - a));
}

// Altura percebida = periodicidade: pico da autocorrelação numa janela de
// lags em torno da fundamental esperada. Robusto a parciais fortes.
float measuredFreq(const std::vector<float>& v, std::size_t a, std::size_t b,
                   const float expectHz) {
    const int lagLo = static_cast<int>(kSampleRate / (expectHz * 1.5f));
    const int lagHi = static_cast<int>(kSampleRate / (expectHz * 0.66f));
    double best = -1e18;
    int bestLag = lagLo;
    for (int lag = lagLo; lag <= lagHi; ++lag) {
        double acc = 0.0;
        for (std::size_t i = a; i + lag < b && i + lag < v.size(); ++i)
            acc += static_cast<double>(v[i]) * v[i + lag];
        if (acc > best) { best = acc; bestLag = lag; }
    }
    return kSampleRate / static_cast<float>(bestLag);
}

void testRingsAndDecays() {
    Matter m;
    m.setParameter("freq", 160.0f);
    m.setParameter("damping", 0.4f);
    m.setParameter("exciter", 0.6f);
    const auto r = strikeRing(m, 220000);  // ~4,6 s
    check(rms(r, 500, 3000) > 0.01f, "há ressonância logo após o golpe");
    check(rms(r, 195000, 219000) < 0.35 * rms(r, 500, 3000),
          "a ressonância decai com o tempo");
    for (const float v : r)
        EXPECT(std::isfinite(v) && std::fabs(v) < 2.0f);
}

void testPitchTracks() {
    for (const float hz : {110.0f, 220.0f, 440.0f}) {
        Matter m;
        m.setParameter("freq", hz);
        m.setParameter("structure", 0.0f);   // harmônico -> periódico
        m.setParameter("damping", 0.2f);
        m.setParameter("brightness", 0.25f);  // favorece a fundamental
        m.setParameter("exciter", 0.7f);
        const auto r = strikeRing(m, 40000);
        const float f = measuredFreq(r, 2000, 18000, hz);
        check(f > 0.0f && std::fabs(f - hz) / hz < 0.08f,
              "a periodicidade da ressonância acompanha freq");
    }
}

void testDampingControlsDecay() {
    Matter wet, dry;
    wet.setParameter("freq", 200.0f);
    wet.setParameter("damping", 0.05f);   // decai devagar
    wet.setParameter("exciter", 0.6f);
    dry.setParameter("freq", 200.0f);
    dry.setParameter("damping", 0.9f);    // decai rápido
    dry.setParameter("exciter", 0.6f);
    const auto rw = strikeRing(wet, 40000);
    const auto rd = strikeRing(dry, 40000);
    const double lateW = rms(rw, 25000, 39000);
    const double lateD = rms(rd, 25000, 39000);
    check(lateW > lateD * 3.0, "damping baixo sustenta muito mais que damping alto");
}

void testStructureChangesTimbre() {
    Matter harm, bell;
    harm.setParameter("freq", 180.0f);
    harm.setParameter("structure", 0.0f);
    harm.setParameter("exciter", 0.6f);
    bell.setParameter("freq", 180.0f);
    bell.setParameter("structure", 1.0f);
    bell.setParameter("exciter", 0.6f);
    const auto rh = strikeRing(harm, 30000);
    const auto rb = strikeRing(bell, 30000);
    bool differ = false;
    for (std::size_t i = 2000; i < rh.size() && i < rb.size(); ++i)
        if (std::fabs(rh[i] - rb[i]) > 1.0e-3f) differ = true;
    EXPECT(differ);
    for (const float v : rb)
        EXPECT(std::isfinite(v));
}

void testStabilityUnderDrive() {
    Matter m;
    m.setParameter("damping", 0.0f);   // pior caso (r perto de 1)
    m.setParameter("exciter", 1.0f);
    m.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), strike(kSampleRate, 1, kBlock),
        fmod(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &strike, &fmod, nullptr};
    std::uint32_t rng = 22222u;
    for (int b = 0; b < 6000; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            in.at(0, i) = static_cast<float>(rng & 0xFFFF) / 32768.0f - 1.0f;
        }
        strike.at(0, 0) = (b % 40 == 0) ? 1.0f : 0.0f;
        fmod.at(0, 0) = std::sin(b * 0.02f) * 1.5f;
        m.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(out[0].at(0, i)) && std::fabs(out[0].at(0, i)) <= 1.5f);
    }
}

void testDeterminism() {
    Matter a, b;
    for (Matter* m : {&a, &b}) {
        m->setParameter("freq", 150.0f);
        m->setParameter("structure", 0.5f);
        m->setParameter("exciter", 0.8f);
    }
    const auto ra = strikeRing(a, 30000);
    const auto rb = strikeRing(b, 30000);
    bool identical = ra.size() == rb.size();
    for (std::size_t i = 0; identical && i < ra.size(); ++i)
        if (ra[i] != rb[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 100.0f);
    graph.node(clk).setParameter("mult", 2.0f);
    graph.node(clk).setParameter("fill", 5.0f);
    const auto body = graph.add(std::make_unique<Matter>());
    graph.node(body).setParameter("freq", 130.0f);
    graph.node(body).setParameter("damping", 0.3f);
    graph.node(body).setParameter("exciter", 0.7f);
    graph.connect(clk, 1, body, 1);   // euclid -> strike
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, body, 0);
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    Matter m;
    const std::string problem = validatePanel(m);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(m.panel().widgets.size() >= 12);
    std::cout << renderAscii(m);
}

}  // namespace

int main() {
    testRingsAndDecays();
    testPitchTracks();
    testDampingControlsDecay();
    testStructureChangesTimbre();
    testStabilityUnderDrive();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular MATTER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
