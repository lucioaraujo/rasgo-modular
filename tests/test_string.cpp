// Teste isolado do Módulo 11 (STRING) - antes de entrar num patch.
// Critérios do dossiê `dossies/11_string.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/StringVoice.hpp"
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

std::vector<float> pluckRing(StringVoice& s, const int samples,
                             const float bow = 0.0f) {
    s.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), pk(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &pk, nullptr, nullptr};
    std::vector<float> r;
    int t = 0;
    while (static_cast<int>(r.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            in.at(0, i) = bow;
            pk.at(0, i) = (t + static_cast<int>(i) == 1) ? 1.0f : 0.0f;
        }
        s.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.push_back(out[0].at(0, i));
        t += static_cast<int>(kBlock);
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * v[i];
    return std::sqrt(s / static_cast<double>(b - a));
}

float acFreq(const std::vector<float>& v, std::size_t a, std::size_t b,
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

void testPluckPitch() {
    for (const float hz : {110.0f, 220.0f, 330.0f}) {
        StringVoice s;
        s.setParameter("freq", hz);
        s.setParameter("decay", 0.85f);
        s.setParameter("damping", 0.35f);
        s.setParameter("exciter", 0.7f);
        const auto r = pluckRing(s, 30000);
        const float f = acFreq(r, 1500, 22000, hz);
        check(std::fabs(f - hz) / hz < 0.06f, "a corda soa na altura de freq");
        for (const float v : r)
            EXPECT(std::isfinite(v) && std::fabs(v) < 2.0f);
    }
}

void testDecayControlsSustain() {
    StringVoice longS, shortS;
    longS.setParameter("freq", 160.0f);
    longS.setParameter("decay", 1.0f);
    longS.setParameter("exciter", 0.7f);
    shortS.setParameter("freq", 160.0f);
    shortS.setParameter("decay", 0.1f);
    shortS.setParameter("exciter", 0.7f);
    const auto rl = pluckRing(longS, 90000);
    const auto rs = pluckRing(shortS, 90000);
    check(rms(rl, 70000, 89000) > rms(rs, 70000, 89000) * 4.0,
          "decay alto sustenta muito mais");
}

void testDampingBrightness() {
    StringVoice bright, dark;
    bright.setParameter("freq", 150.0f);
    bright.setParameter("decay", 0.8f);
    bright.setParameter("damping", 0.02f);
    bright.setParameter("exciter", 0.7f);
    dark.setParameter("freq", 150.0f);
    dark.setParameter("decay", 0.8f);
    dark.setParameter("damping", 0.95f);
    dark.setParameter("exciter", 0.7f);
    const auto rb = pluckRing(bright, 20000);
    const auto rd = pluckRing(dark, 20000);
    auto hf = [](const std::vector<float>& v) {
        double s = 0.0;
        for (std::size_t i = 6001; i < 18000 && i < v.size(); ++i)
            s += (v[i] - v[i - 1]) * (v[i] - v[i - 1]);
        return s;
    };
    check(hf(rb) > hf(rd) * 1.5, "damping baixo = mais agudo na corda");
}

void testBowedStability() {
    StringVoice s;
    s.setParameter("decay", 1.0f);
    s.setParameter("damping", 0.1f);
    s.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(1, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock in(kSampleRate, 1, kBlock), pk(kSampleRate, 1, kBlock),
        fm(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{&in, &pk, &fm, nullptr};
    for (int b = 0; b < 6000; ++b) {
        for (std::size_t i = 0; i < kBlock; ++i)
            in.at(0, i) = 0.05f;  // arco contínuo
        pk.at(0, 0) = (b % 90 == 0) ? 1.0f : 0.0f;
        fm.at(0, 0) = std::sin(b * 0.015f) * 1.2f;
        s.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            EXPECT(std::isfinite(out[0].at(0, i))
                   && std::fabs(out[0].at(0, i)) <= 1.6f);
    }
}

void testDeterminism() {
    StringVoice a, b;
    for (StringVoice* s : {&a, &b}) {
        s->setParameter("freq", 130.0f);
        s->setParameter("decay", 0.8f);
        s->setParameter("position", 0.2f);
        s->setParameter("exciter", 0.8f);
    }
    const auto ra = pluckRing(a, 30000);
    const auto rb = pluckRing(b, 30000);
    bool identical = ra.size() == rb.size();
    for (std::size_t i = 0; identical && i < ra.size(); ++i)
        if (ra[i] != rb[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    SignalGraph graph;
    const auto clk = graph.add(std::make_unique<EuclidClock>());
    graph.node(clk).setParameter("bpm", 110.0f);
    graph.node(clk).setParameter("mult", 2.0f);
    graph.node(clk).setParameter("fill", 5.0f);
    const auto str = graph.add(std::make_unique<StringVoice>());
    graph.node(str).setParameter("freq", 147.0f);
    graph.node(str).setParameter("decay", 0.8f);
    graph.node(str).setParameter("exciter", 0.7f);
    graph.connect(clk, 1, str, 1);   // euclid -> pluck
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 3000; ++b) {
        graph.process(out, str, 0);
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-3f);
}

void testPanel() {
    StringVoice s;
    const std::string problem = validatePanel(s);
    check(problem.empty(), "descricao de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(s.panel().widgets.size() >= 12);
    std::cout << renderAscii(s);
}

}  // namespace

int main() {
    testPluckPitch();
    testDecayControlsSustain();
    testDampingBrightness();
    testBowedStability();
    testDeterminism();
    testInGraph();
    testPanel();

    if (g_failures == 0) {
        std::cout << "RASGO Modular STRING tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
