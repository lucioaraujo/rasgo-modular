// Teste isolado do Módulo 18 (OSC) - antes de entrar num patch.
// Critérios do dossiê `dossies/18_oscilador.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Oscillator.hpp"
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
constexpr std::size_t kBlock = 128;

// Renderiza `samples` amostras de uma saída do OSC. `pitch`/`fmHz`/`syncHz`
// alimentam as entradas (0 = sem entrada).
std::vector<float> render(Oscillator& o, const int outIdx, const int samples,
                          const float pitch = 0.0f, const float fmHz = 0.0f,
                          const float syncHz = 0.0f) {
    o.prepare(kSampleRate, kBlock);
    std::vector<AudioBlock> out(5, AudioBlock(kSampleRate, 1, kBlock));
    AudioBlock pin(kSampleRate, 1, kBlock), fin(kSampleRate, 1, kBlock),
        win(kSampleRate, 1, kBlock), sin_(kSampleRate, 1, kBlock);
    std::vector<const AudioBlock*> ins{
        pitch != 0.0f ? &pin : nullptr,
        fmHz != 0.0f ? &fin : nullptr,
        nullptr,
        syncHz != 0.0f ? &sin_ : nullptr};
    std::vector<float> r;
    long t = 0;
    double fmPhase = 0.0;
    const double syncStep = syncHz > 0.0f ? syncHz / kSampleRate : 0.0;
    double syncPhase = 0.0;
    float prevSyncOut = 0.0f;
    while (static_cast<int>(r.size()) < samples) {
        for (std::size_t i = 0; i < kBlock; ++i) {
            pin.at(0, i) = pitch;
            fin.at(0, i) = static_cast<float>(std::sin(fmPhase));
            fmPhase += 6.283185307 * fmHz / kSampleRate;
            // trem de pulsos de sync
            float s = 0.0f;
            if (syncHz > 0.0f) {
                syncPhase += syncStep;
                if (syncPhase >= 1.0) { syncPhase -= 1.0; s = 1.0f; }
                (void)prevSyncOut;
            }
            sin_.at(0, i) = s;
        }
        o.process(ins, out);
        for (std::size_t i = 0; i < kBlock; ++i)
            r.push_back(out[static_cast<std::size_t>(outIdx)].at(0, i));
        t += static_cast<long>(kBlock);
    }
    return r;
}

double rms(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i] * v[i];
    return std::sqrt(s / static_cast<double>(b - a));
}
double mean(const std::vector<float>& v, std::size_t a, std::size_t b) {
    double s = 0.0;
    for (std::size_t i = a; i < b && i < v.size(); ++i) s += v[i];
    return s / static_cast<double>(b - a);
}

// afinação por autocorrelação, janela em torno de `expectHz`
float acFreq(const std::vector<float>& v, std::size_t a, std::size_t b,
             const float expectHz) {
    const int lagLo = std::max(2, static_cast<int>(kSampleRate / (expectHz * 1.6f)));
    const int lagHi = static_cast<int>(kSampleRate / (expectHz * 0.62f));
    double best = -1e18;
    int bestLag = lagLo;
    for (int lag = lagLo; lag <= lagHi; ++lag) {
        double acc = 0.0;
        for (std::size_t i = a; i + static_cast<std::size_t>(lag) < b
                 && i + static_cast<std::size_t>(lag) < v.size(); ++i)
            acc += static_cast<double>(v[i]) * v[i + static_cast<std::size_t>(lag)];
        if (acc > best) { best = acc; bestLag = lag; }
    }
    return kSampleRate / static_cast<float>(bestLag);
}

void testPitchTracking() {
    for (const float hz : {110.0f, 220.0f, 440.0f}) {
        Oscillator o;
        o.setParameter("freq", hz);
        const auto saw = render(o, 2, 20000);
        const float f = acFreq(saw, 2000, 18000, hz);
        check(std::fabs(f - hz) / hz < 0.02f, "OSC afina em freq (1 V/oct)");
        for (const float v : saw)
            EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
    }
    // 1 V/oct: pitch = +1 dobra
    Oscillator o;
    o.setParameter("freq", 200.0f);
    const auto up = render(o, 2, 20000, 1.0f);
    const float f = acFreq(up, 2000, 18000, 400.0f);
    check(std::fabs(f - 400.0f) / 400.0f < 0.03f, "pitch = +1 V dobra a frequência");
}

void testWaveShapes() {
    Oscillator o;
    o.setParameter("freq", 220.0f);
    const auto sine = render(o, 0, 8000);
    const auto pulse = render(o, 3, 8000);
    // seno: correlação alta com uma senoide de referência na mesma freq
    double num = 0.0, da = 0.0, db = 0.0;
    const double step = 6.283185307 * 220.0 / kSampleRate;
    double ph = 500.0 * step;
    for (std::size_t i = 500; i < 7500; ++i) {
        const double ref = std::sin(ph);
        ph += step;
        num += sine[i] * ref;
        da += sine[i] * sine[i];
        db += ref * ref;
    }
    const double corr = num / std::sqrt(da * db + 1e-12);
    check(corr > 0.9, "saída 'sine' é praticamente senoidal");
    // pulso: só valores perto de ±1
    int offLevel = 0;
    for (std::size_t i = 500; i < 7500; ++i)
        if (std::fabs(std::fabs(pulse[i]) - 1.0f) > 0.25f) ++offLevel;
    check(offLevel < 200, "saída 'pulse' é binível (±1)");
}

void testPwm() {
    Oscillator narrow, wide;
    narrow.setParameter("freq", 200.0f);
    narrow.setParameter("pw", 0.2f);
    wide.setParameter("freq", 200.0f);
    wide.setParameter("pw", 0.8f);
    const auto pn = render(narrow, 3, 12000);
    const auto pw = render(wide, 3, 12000);
    const double mn = mean(pn, 1000, 11000);
    const double mw = mean(pw, 1000, 11000);
    // pw = 0,2 -> média ~ -0,6 ; pw = 0,8 -> ~ +0,6
    check(mn < -0.35 && mw > 0.35, "razão cíclica do pulso segue `pw`");
    check(mw - mn > 0.8, "PWM varre a razão cíclica");
}

void testSubOctave() {
    Oscillator o1, o2;
    o1.setParameter("freq", 300.0f);
    o1.setParameter("sub_2", 0.0f);
    o2.setParameter("freq", 300.0f);
    o2.setParameter("sub_2", 1.0f);
    const auto s1 = render(o1, 4, 24000);
    const auto s2 = render(o2, 4, 24000);
    const float f1 = acFreq(s1, 2000, 22000, 150.0f);
    const float f2 = acFreq(s2, 2000, 22000, 75.0f);
    check(std::fabs(f1 - 150.0f) / 150.0f < 0.05f, "sub uma oitava abaixo");
    check(std::fabs(f2 - 75.0f) / 75.0f < 0.06f, "sub_2 = duas oitavas abaixo");
}

void testHardSync() {
    Oscillator synced, free_;
    synced.setParameter("freq", 320.0f);
    synced.setParameter("sync_enable", 1.0f);
    free_.setParameter("freq", 320.0f);
    free_.setParameter("sync_enable", 0.0f);
    const auto a = render(synced, 2, 24000, 0.0f, 0.0f, 100.0f);
    const auto b = render(free_, 2, 24000, 0.0f, 0.0f, 100.0f);
    const float fa = acFreq(a, 3000, 22000, 100.0f);
    const float fb = acFreq(b, 3000, 22000, 320.0f);
    check(std::fabs(fa - 100.0f) / 100.0f < 0.06f,
          "hard sync: saída periódica na taxa do `sync`");
    check(fb > 240.0f, "sem sync_enable a saída ignora o `sync`");
}

void testThroughZeroFm() {
    Oscillator dry, wet;
    dry.setParameter("freq", 300.0f);
    dry.setParameter("fm_amount", 0.0f);
    wet.setParameter("freq", 300.0f);
    wet.setParameter("fm_amount", 0.9f);
    const auto rd = render(dry, 2, 16000, 0.0f, 140.0f);
    const auto rw = render(wet, 2, 16000, 0.0f, 140.0f);
    // fm_amount = 0 -> a entrada de FM não muda nada
    bool same = rd.size() == rw.size();
    // (comparação com o render sem FM ligada de fato)
    Oscillator noFm;
    noFm.setParameter("freq", 300.0f);
    const auto rn = render(noFm, 2, 16000);
    for (std::size_t i = 0; same && i < rd.size(); ++i)
        if (std::fabs(rd[i] - rn[i]) > 1e-6f) same = false;
    check(same, "fm_amount = 0: entrada de FM é inerte");
    // fm_amount alto -> limitada e finita (TZFM não diverge)
    for (const float v : rw)
        EXPECT(std::isfinite(v) && std::fabs(v) < 1.3f);
    check(rms(rw, 2000, 15000) > 0.1, "TZFM produz sinal");
}

void testAntiAlias() {
    // serra a 9 kHz: harmônicos dobrariam pra banda baixa numa serra naïve.
    // A versão com PolyBLEP deve ter MENOS energia sub-4 kHz.
    Oscillator o;
    o.setParameter("freq", 9000.0f);
    const auto blep = render(o, 2, 24000);
    // serra naïve pela mesma fase
    std::vector<float> naive(blep.size());
    double ph = 0.0;
    const double dp = 9000.0 / kSampleRate;
    for (std::size_t i = 0; i < naive.size(); ++i) {
        naive[i] = static_cast<float>(2.0 * ph - 1.0);
        ph += dp;
        ph -= std::floor(ph);
    }
    // passa-baixa de 1 polo ~3 kHz nos dois, compara RMS
    auto lowRms = [](const std::vector<float>& v) {
        const float c = std::exp(-6.2831853f * 3000.0f / kSampleRate);
        float y = 0.0f;
        double s = 0.0;
        std::size_t n = 0;
        for (std::size_t i = 0; i < v.size(); ++i) {
            y = v[i] * (1.0f - c) + c * y;
            if (i > 2000) { s += y * y; ++n; }
        }
        return std::sqrt(s / static_cast<double>(n));
    };
    const double lb = lowRms(blep), ln = lowRms(naive);
    std::cerr << "  [antialias] banda baixa blep=" << lb << " naive=" << ln << '\n';
    check(lb < ln * 0.85 && lb < 0.18,
          "PolyBLEP corta o alias dobrado da serra (banda baixa menor)");
    for (const float v : blep) EXPECT(std::isfinite(v) && std::fabs(v) < 1.2f);
}

void testDrift() {
    Oscillator drifting, still_;
    drifting.setParameter("freq", 220.0f);
    drifting.setParameter("drift", 1.0f);
    still_.setParameter("freq", 220.0f);
    still_.setParameter("drift", 0.0f);
    const auto rd = render(drifting, 2, 96000);
    const auto rs = render(still_, 2, 96000);
    // drift != 0 muda a saída, mas a afinação segue perto do nominal
    bool differ = false;
    for (std::size_t i = 0; i < rd.size() && i < rs.size(); ++i)
        if (std::fabs(rd[i] - rs[i]) > 1e-4f) { differ = true; break; }
    check(differ, "`drift` > 0 altera a saída");
    const float f = acFreq(rd, 60000, 92000, 220.0f);
    check(std::fabs(f - 220.0f) / 220.0f < 0.05f,
          "`drift` fica dentro de ~½ semitom");
}

void testDeterminism() {
    Oscillator a, b;
    for (Oscillator* o : {&a, &b}) {
        o->setParameter("freq", 173.0f);
        o->setParameter("drift", 0.7f);
        o->setParameter("pw", 0.3f);
    }
    const auto ra = render(a, 3, 40000);
    const auto rb = render(b, 3, 40000);
    bool identical = ra.size() == rb.size();
    for (std::size_t i = 0; identical && i < ra.size(); ++i)
        if (ra[i] != rb[i]) identical = false;
    EXPECT(identical);
}

void testInGraph() {
    SignalGraph graph;
    const auto osc = graph.add(std::make_unique<Oscillator>());
    graph.node(osc).setParameter("freq", 130.0f);
    const auto flt = graph.add(std::make_unique<Filter>());
    graph.node(flt).setParameter("cutoff", 900.0f);
    graph.connect(osc, 2, flt, 0);          // saw -> filtro
    graph.prepare(kSampleRate, 1, kBlock);
    AudioBlock out(kSampleRate, 1, kBlock);
    float peak = 0.0f;
    for (int b = 0; b < 2000; ++b) {
        graph.process(out, flt, 3);          // 'all'
        for (std::size_t i = 0; i < kBlock; ++i) {
            EXPECT(std::isfinite(out.at(0, i)));
            peak = std::max(peak, std::fabs(out.at(0, i)));
        }
    }
    EXPECT(peak > 1.0e-2f);
}

void testPanel() {
    Oscillator o;
    const std::string problem = validatePanel(o);
    check(problem.empty(), "descrição de painel fecha");
    if (!problem.empty()) std::cerr << "  " << problem << '\n';
    EXPECT(o.panel().widgets.size() >= 16);
    std::cout << renderAscii(o);
}

}  // namespace

int main() {
    testPitchTracking();
    testWaveShapes();
    testPwm();
    testSubOctave();
    testHardSync();
    testThroughZeroFm();
    testAntiAlias();
    testDrift();
    testDeterminism();
    testInGraph();
    testPanel();
    if (g_failures == 0) std::cout << "test_oscillator: OK\n";
    return g_failures == 0 ? 0 : 1;
}
