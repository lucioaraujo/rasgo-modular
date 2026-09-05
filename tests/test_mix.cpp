// Teste isolado dos Módulos 16 (MIXER) e 17 (MASTER) - antes do patch.
// Critérios dos dossiês `dossies/16_mixer.md` e `17_master.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Master.hpp"
#include "dsp/Mixer.hpp"
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
void near(float a, float b, float t = 1.0e-3f) { check(std::fabs(a - b) < t, "near"); }

constexpr float kSr = 48000.0f;

// roda `mod` com blocos estéreo, entradas dadas por `feed`, e devolve
// (L, R) do último frame.
struct Frame { float l = 0.0f, r = 0.0f, aux = 0.0f; };

// --- MIXER -----------------------------------------------------------------

Frame mixOnce(Mixer& m, const float* chIn /*4*/, int blocks = 4) {
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    AudioBlock a(kSr, 1, 64), b(kSr, 1, 64), c(kSr, 1, 64), d(kSr, 1, 64);
    for (std::size_t i = 0; i < 64; ++i) {
        a.at(0, i) = chIn[0]; b.at(0, i) = chIn[1];
        c.at(0, i) = chIn[2]; d.at(0, i) = chIn[3];
    }
    std::vector<const AudioBlock*> in{&a, &b, &c, &d};
    for (int k = 0; k < blocks; ++k) m.process(in, out);
    return {out[0].at(0, 0), out[0].at(1, 0)};
}

void testMixerPanLaw() {
    Mixer m;
    const float in[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    // pan centro -> potência constante: L = R = cos(45°) = 0,707
    m.setParameter("pan1", 0.0f);
    Frame f = mixOnce(m, in);
    near(f.l, 0.70710678f);
    near(f.r, 0.70710678f);
    // pan totalmente à esquerda -> L = 1, R = 0
    m.setParameter("pan1", -1.0f);
    f = mixOnce(m, in);
    near(f.l, 1.0f);
    near(f.r, 0.0f);
    // pan totalmente à direita
    m.setParameter("pan1", 1.0f);
    f = mixOnce(m, in);
    near(f.l, 0.0f);
    near(f.r, 1.0f);
}

void testMixerGainAndMute() {
    Mixer m;
    const float in[4] = {1.0f, 1.0f, 0.0f, 0.0f};
    m.setParameter("pan1", -1.0f);   // ch1 na esquerda
    m.setParameter("pan2", 1.0f);    // ch2 na direita
    m.setParameter("gain1", -6.0206f);  // metade
    Frame f = mixOnce(m, in);
    near(f.l, 0.5f, 5.0e-3f);
    near(f.r, 1.0f);
    m.setParameter("mute2", 1.0f);
    f = mixOnce(m, in);
    near(f.r, 0.0f);
    // out_gain
    m.setParameter("mute2", 0.0f);
    m.setParameter("out_gain", -6.0206f);
    f = mixOnce(m, in);
    near(f.r, 0.5f, 5.0e-3f);
}

void testMixerMonoGraph() {
    Mixer m;
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 1, 64));  // bloco de saída mono
    AudioBlock a(kSr, 1, 64), z(kSr, 1, 64);
    for (std::size_t i = 0; i < 64; ++i) a.at(0, i) = 1.0f;
    std::vector<const AudioBlock*> in{&a, &z, &z, &z};
    m.process(in, out);
    // mono: saída = (L+R)/2 ; pan centro -> (0,707+0,707)/2 = 0,707
    near(out[0].at(0, 0), 0.70710678f);
}

// --- MASTER --------------------------------------------------------------

Frame masterOnce(Master& m, float l, float r, int blocks = 8) {
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 2, 64));
    AudioBlock in(kSr, 2, 64);
    for (std::size_t i = 0; i < 64; ++i) { in.at(0, i) = l; in.at(1, i) = r; }
    std::vector<const AudioBlock*> ins{&in};
    for (int k = 0; k < blocks; ++k) m.process(ins, out);
    return {out[0].at(0, 0), out[0].at(1, 0), out[1].at(0, 0)};
}

void testMasterWidth() {
    Master m;
    m.setParameter("dc_block", 0.0f);
    m.setParameter("gain", 0.0f);  // isola width do default de gain (-6 dB)
    // width 1 (normal): passa
    Frame f = masterOnce(m, 0.6f, 0.2f);
    near(f.l, 0.6f); near(f.r, 0.2f);
    // width 0 -> mono (mid): (0,6+0,2)/2 = 0,4 nos dois
    m.setParameter("width", 0.0f);
    f = masterOnce(m, 0.6f, 0.2f);
    near(f.l, 0.4f); near(f.r, 0.4f);
    // width 2 -> side dobrado: mid 0,4, side (0,2)*2 = 0,4 -> L 0,8 R 0,0
    m.setParameter("width", 2.0f);
    f = masterOnce(m, 0.6f, 0.2f);
    near(f.l, 0.8f); near(f.r, 0.0f);
}

void testMasterMonoAndGain() {
    Master m;
    m.setParameter("dc_block", 0.0f);
    m.setParameter("gain", 0.0f);  // isola mono do default de gain (-6 dB)
    m.setParameter("mono", 1.0f);
    Frame f = masterOnce(m, 0.5f, -0.1f);
    near(f.l, f.r);
    near(f.l, 0.2f);  // (0,5 + −0,1)/2
    m.setParameter("mono", 0.0f);
    m.setParameter("gain", -6.0206f);
    f = masterOnce(m, 0.4f, 0.4f);
    near(f.l, 0.2f, 5.0e-3f);
}

void testMasterDcBlock() {
    Master dc, no;
    dc.setParameter("dc_block", 1.0f);
    no.setParameter("dc_block", 0.0f);
    dc.setParameter("gain", 0.0f);  // isola dc_block do default de gain (-6 dB)
    no.setParameter("gain", 0.0f);
    // sinal com offset de DC 0,3 -> depois de assentar, o bloqueio tira o DC
    const Frame fdc = masterOnce(dc, 0.3f, 0.3f, 200);
    const Frame fno = masterOnce(no, 0.3f, 0.3f, 200);
    check(std::fabs(fdc.l) < 0.02f, "dc_block remove o offset de DC");
    check(std::fabs(fno.l - 0.3f) < 0.02f, "sem dc_block o offset passa");
}

void testMasterMute() {
    Master m;
    m.setParameter("dc_block", 0.0f);
    m.setParameter("gain", 0.0f);  // 0 dB: saída ~= entrada
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 2, 64));
    AudioBlock in(kSr, 2, 64);
    for (std::size_t i = 0; i < 64; ++i) { in.at(0, i) = 0.5f; in.at(1, i) = 0.5f; }
    std::vector<const AudioBlock*> ins{&in};

    for (int k = 0; k < 8; ++k) m.process(ins, out);
    near(out[0].at(0, 63), 0.5f, 3.0e-3f);   // toca normal

    // aciona o mute: RAMPA, não corte seco — o 1º sample ainda soa
    m.setParameter("mute", 1.0f);
    m.process(ins, out);
    check(std::fabs(out[0].at(0, 0)) > 0.1f, "mute usa rampa (não corta seco)");

    // depois de ~80 ms está em silêncio nos dois canais
    for (int k = 0; k < 60; ++k) m.process(ins, out);
    check(std::fabs(out[0].at(0, 63)) < 1.0e-3f, "mute silencia L");
    check(std::fabs(out[0].at(1, 63)) < 1.0e-3f, "mute silencia R");

    // desliga: volta a tocar
    m.setParameter("mute", 0.0f);
    for (int k = 0; k < 60; ++k) m.process(ins, out);
    near(out[0].at(0, 63), 0.5f, 3.0e-3f);
}

void testMasterLimitAndLevel() {
    Master m;
    m.setParameter("dc_block", 0.0f);
    m.setParameter("gain", 0.0f);  // isola limit do default de gain (-6 dB)
    m.setParameter("limit", 1.0f);
    Frame f = masterOnce(m, 3.0f, -3.0f);
    check(std::fabs(f.l) <= 1.01f && std::fabs(f.r) <= 1.01f,
          "limit segura o sinal perto de ±1");
    check(f.aux > 0.5f, "level (VU) acompanha o pico");
    // determinismo
    Master a, b;
    for (Master* x : {&a, &b}) x->setParameter("width", 1.4f);
    bool same = true;
    Master* pa = &a; Master* pb = &b;
    Frame fa, fb;
    for (int k = 0; k < 50; ++k) {
        fa = masterOnce(*pa, 0.5f, 0.2f, 1);
        fb = masterOnce(*pb, 0.5f, 0.2f, 1);
        if (fa.l != fb.l || fa.r != fb.r) same = false;
    }
    EXPECT(same);
}

// padrão de excelência RASGO: o limitador com look-ahead segura um patch
// quente SEM distorcer (o `tanh` instantâneo distorcia), e a guarda de
// finitude contém NaN/Inf. Ver `src/dsp/OutputStage.hpp`.
void testMasterExcellenceGuard() {
    Master m;
    m.setParameter("dc_block", 0.0f);
    m.setParameter("gain", 0.0f);  // isola o limitador do default de gain (-6 dB)
    m.setParameter("limit", 1.0f);
    m.prepare(kSr, 256);
    std::vector<AudioBlock> out(2, AudioBlock(kSr, 2, 256));
    AudioBlock in(kSr, 2, 256);
    std::vector<const AudioBlock*> ins{&in};
    double ph = 0.0, peak = 0.0;
    long overCeiling = 0, total = 0;
    for (int b = 0; b < 200; ++b) {
        for (std::size_t i = 0; i < 256; ++i) {
            // seno de 220 Hz a +8 dB (amplitude 2.5)
            float s = 2.5f * std::sin(static_cast<float>(ph));
            ph += 6.2831853 * 220.0 / kSr;
            if (b == 90 && i < 4) s = 7.0f;   // transiente feio
            in.at(0, i) = s;
            in.at(1, i) = s;
        }
        m.process(ins, out);
        if (b > 20)
            for (std::size_t i = 0; i < 256; ++i) {
                const double v = out[0].at(0, i);
                peak = std::max(peak, std::fabs(v));
                if (std::fabs(v) > 0.95) ++overCeiling;
                ++total;
            }
    }
    check(peak <= 0.9f, "look-ahead segura o pico abaixo do teto (~−1 dBFS)");
    check(overCeiling == 0, "nenhuma amostra estoura o teto");
    check(m.gainReductionDb() > 3.0f, "o limitador realmente reduziu o ganho");

    // guarda de finitude: NaN/Inf na entrada -> saída finita
    Master g;
    g.prepare(kSr, 64);
    std::vector<AudioBlock> o2(2, AudioBlock(kSr, 2, 64));
    AudioBlock bad(kSr, 2, 64);
    for (std::size_t i = 0; i < 64; ++i) {
        bad.at(0, i) = std::nanf("");
        bad.at(1, i) = i % 2 ? 1e30f : -1e30f;
    }
    std::vector<const AudioBlock*> bi{&bad};
    for (int k = 0; k < 8; ++k) g.process(bi, o2);
    bool finite = true;
    for (std::size_t i = 0; i < 64; ++i)
        if (!std::isfinite(o2[0].at(0, i)) || !std::isfinite(o2[0].at(1, i)))
            finite = false;
    EXPECT(finite);
}

void testInGraphStereo() {
    SignalGraph graph;
    const auto v1 = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(v1).setParameter("rate", 110.0f);
    const auto v2 = graph.add(std::make_unique<FunctionGenerator>());
    graph.node(v2).setParameter("rate", 165.0f);
    const auto mix = graph.add(std::make_unique<Mixer>());
    graph.node(mix).setParameter("pan1", -0.7f);
    graph.node(mix).setParameter("pan2", 0.7f);
    const auto mst = graph.add(std::make_unique<Master>());
    graph.connect(v1, 1, mix, 0);
    graph.connect(v2, 1, mix, 1);
    graph.connect(mix, 0, mst, 0);
    graph.prepare(kSr, 2, 128);
    AudioBlock out(kSr, 2, 128);
    float dL = 0.0f;
    for (int b = 0; b < 400; ++b) {
        graph.process(out, mst, 0);
        for (std::size_t i = 0; i < 128; ++i) {
            EXPECT(std::isfinite(out.at(0, i)) && std::isfinite(out.at(1, i)));
            dL = std::max(dL, std::fabs(out.at(0, i) - out.at(1, i)));
        }
    }
    EXPECT(dL > 0.01f);  // há imagem estéreo (L != R)
}

void testPanels() {
    Mixer mx; Master ms;
    check(validatePanel(mx).empty(), "painel MIXER fecha");
    check(validatePanel(ms).empty(), "painel MASTER fecha");
    std::cout << renderAscii(mx) << renderAscii(ms);
}

}  // namespace

int main() {
    testMixerPanLaw();
    testMixerGainAndMute();
    testMixerMonoGraph();
    testMasterWidth();
    testMasterMonoAndGain();
    testMasterDcBlock();
    testMasterMute();
    testMasterLimitAndLevel();
    testMasterExcellenceGuard();
    testInGraphStereo();
    testPanels();

    if (g_failures == 0) {
        std::cout << "RASGO Modular MIXER/MASTER tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
