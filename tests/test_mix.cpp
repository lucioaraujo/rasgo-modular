// Teste isolado dos Módulos 16 (MIXER) e 17 (MASTER) - antes do patch.
// Critérios dos dossiês `dossies/16_mixer.md` e `17_master.md` §7.

#include "core/SignalGraph.hpp"
#include "dsp/Master.hpp"
#include "dsp/Mixer.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "io/AsciiPanel.hpp"

#include <algorithm>
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

// RMS de L e R com um seno de `hz` no canal 1 (o resto mudo), depois de
// assentar rampa e filtro
Frame mixSine(Mixer& m, const float hz, int blocks = 200) {
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    AudioBlock a(kSr, 1, 64), z(kSr, 1, 64);
    for (std::size_t i = 0; i < 64; ++i) z.at(0, i) = 0.0f;
    std::vector<const AudioBlock*> in{&a, &z, &z, &z};
    double ph = 0.0, el = 0.0, er = 0.0; long n = 0;
    for (int k = 0; k < blocks; ++k) {
        for (std::size_t i = 0; i < 64; ++i, ph += 2.0 * M_PI * hz / kSr)
            a.at(0, i) = static_cast<float>(std::sin(ph));
        m.process(in, out);
        if (k >= blocks / 2)
            for (std::size_t i = 0; i < 64; ++i, ++n) {
                el += out[0].at(0, i) * out[0].at(0, i);
                er += out[0].at(1, i) * out[0].at(1, i);
            }
    }
    return {static_cast<float>(std::sqrt(el / n)), static_cast<float>(std::sqrt(er / n))};
}

void testMixerPanLaw() {
    Mixer m;
    // agudo (4 kHz): a lei de potência constante de sempre
    m.setParameter("pan1", 0.0f);
    Frame f = mixSine(m, 4000.0f);
    near(f.l, 0.70710678f * 0.70710678f, 6e-3f);   // RMS de seno × 0,707
    near(f.r, 0.70710678f * 0.70710678f, 6e-3f);
    m.setParameter("pan1", -1.0f);
    f = mixSine(m, 4000.0f);
    check(f.l > 0.68f && f.r < 0.04f, "agudo segue o pan: todo à esquerda");
    m.setParameter("pan1", 1.0f);
    f = mixSine(m, 4000.0f);
    check(f.r > 0.68f && f.l < 0.04f, "agudo segue o pan: todo à direita");
}

// GRAVE NO CENTRO (10/11/2026): abaixo de ~150 Hz o canal fica no centro
// mesmo com o PAN no extremo; no centro, nada muda.
void testMixerGraveNoCentro() {
    Mixer m;
    m.setParameter("pan1", -1.0f);
    const Frame g = mixSine(m, 40.0f);
    const float difDb = 20.0f * std::log10(std::max(g.l, 1e-6f) / std::max(g.r, 1e-6f));
    check(difDb < 3.0f, "grave de 40 Hz quase igual nos dois lados com PAN todo à esquerda");
    // constante (DC) é todo grave: centro exato
    const float in[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    m.setParameter("pan1", -1.0f);
    const Frame d = mixOnce(m, in, 400);
    near(d.l, d.r, 2e-3f);
    // PAN no centro: igual à lei de antes, em qualquer frequência
    m.setParameter("pan1", 0.0f);
    for (const float hz : {40.0f, 150.0f, 1000.0f}) {
        const Frame c = mixSine(m, hz);
        near(c.l, c.r, 1e-3f);
        near(c.l, 0.5f, 1.2e-2f);   // janela não é múltiplo do período em 40 Hz
    }
}

// a RAMPA do PAN: girar de um extremo ao outro entre dois blocos não vira
// degrau na saída
void testMixerPanSemClique() {
    Mixer m;
    m.setParameter("pan1", -1.0f);
    m.prepare(kSr, 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    AudioBlock a(kSr, 1, 64), z(kSr, 1, 64);
    for (std::size_t i = 0; i < 64; ++i) z.at(0, i) = 0.0f;
    std::vector<const AudioBlock*> in{&a, &z, &z, &z};
    double ph = 0.0; float last = 0.0f, maxStep = 0.0f;
    for (int k = 0; k < 400; ++k) {
        if (k == 200) m.setParameter("pan1", 1.0f);
        for (std::size_t i = 0; i < 64; ++i, ph += 2.0 * M_PI * 2000.0 / kSr)
            a.at(0, i) = static_cast<float>(0.5 * std::sin(ph));
        m.process(in, out);
        for (std::size_t i = 0; i < 64; ++i) {
            if (k > 10) maxStep = std::max(maxStep, std::fabs(out[0].at(0, i) - last));
            last = out[0].at(0, i);
        }
    }
    // seno de 2 kHz a 0,5: o passo natural entre amostras é ~0,13
    check(maxStep < 0.16f, "pan de um extremo ao outro sem degrau na saída");
}

// --- PALCO (10/11/2026) ------------------------------------------------------
// roda o MIXER com 4 senos (níveis dados), CV de pan opcional, e devolve o
// lugar de cada canal ao fim; acumula o maior |lugar| visto e a excursão.
struct Palco { float pos[4] = {0, 0, 0, 0}, maxAbs = 0.0f, excur[4] = {0, 0, 0, 0}, cog = 0.0f;
               bool finite = true; };
Palco runPalco(Mixer& m, const float level[4], const double seconds, const float panCv = 0.0f) {
    m.prepare(kSr, 256);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 256));
    AudioBlock ch[4] = {AudioBlock(kSr, 1, 256), AudioBlock(kSr, 1, 256),
                        AudioBlock(kSr, 1, 256), AudioBlock(kSr, 1, 256)};
    AudioBlock cv(kSr, 1, 256);
    const float hz[4] = {440.0f, 660.0f, 990.0f, 1320.0f};
    std::vector<const AudioBlock*> in{&ch[0], &ch[1], &ch[2], &ch[3], &cv, &cv, &cv, &cv};
    Palco r; float lo[4] = {9, 9, 9, 9}, hi[4] = {-9, -9, -9, -9};
    double ph = 0.0;
    const long nb = static_cast<long>(seconds * kSr / 256);
    for (long b = 0; b < nb; ++b) {
        for (std::size_t i = 0; i < 256; ++i, ph += 1.0 / kSr) {
            for (int c = 0; c < 4; ++c)
                ch[c].at(0, i) = level[c] * static_cast<float>(std::sin(2.0 * M_PI * hz[c] * ph));
            cv.at(0, i) = panCv * static_cast<float>(std::sin(2.0 * M_PI * 0.2 * ph));
        }
        m.process(in, out);
        for (std::size_t i = 0; i < 256; ++i)
            if (!std::isfinite(out[0].at(0, i)) || !std::isfinite(out[0].at(1, i))) r.finite = false;
        for (int c = 0; c < 4; ++c) {
            const float p = m.stagePosition(c);
            r.maxAbs = std::max(r.maxAbs, std::fabs(p));
            if (b > nb / 4) { lo[c] = std::min(lo[c], p); hi[c] = std::max(hi[c], p); }
        }
    }
    for (int c = 0; c < 4; ++c) { r.pos[c] = m.stagePosition(c); r.excur[c] = hi[c] - lo[c]; }
    r.cog = m.stageCentre();
    return r;
}

void testPalcoTetoEMovimento() {
    // os quatro trajetos: nunca além de 0,55; todos andam; saída finita
    for (int mode = 1; mode <= 4; ++mode) {
        Mixer m;
        m.setParameter("stage", static_cast<float>(mode));
        m.setParameter("stage_rate", 1.0f);      // período de 6 s
        m.setParameter("stage_width", 1.0f);
        m.setParameter("pan1", 0.5f); m.setParameter("pan2", -0.5f);
        m.setParameter("pan3", 0.3f); m.setParameter("pan4", -0.3f);
        const float lv[4] = {0.2f, 0.2f, 0.2f, 0.2f};
        const Palco r = runPalco(m, lv, 14.0, mode == 4 ? 1.0f : 0.0f);
        check(r.finite, "palco: saída finita");
        check(r.maxAbs <= Mixer::kStageMax + 1e-5f, "palco: nunca além da abertura máxima 0,55");
        float moved = 0.0f;
        for (float e : r.excur) moved = std::max(moved, e);
        check(moved > 0.1f, "palco: o trajeto anda");
    }
}

void testPalcoMobile() {
    // sem vento (ABERT 0): canal 1 forte à esquerda, canal 2 fraco ao
    // centro. O móbile equilibra o centro de gravidade em ±0,10 (com
    // folga da mola) — e quem anda é o leve.
    Mixer m;
    m.setParameter("stage", 1.0f);
    m.setParameter("stage_width", 0.0f);
    m.setParameter("pan1", -0.4f); m.setParameter("pan2", 0.0f);
    const float lv[4] = {0.4f, 0.1f, 0.0f, 0.0f};
    const Palco r = runPalco(m, lv, 12.0);
    check(std::fabs(r.cog) <= 0.13f, "móbile: centro de gravidade equilibrado");
    check(std::fabs(r.pos[1] - 0.0f) > 2.0f * std::fabs(r.pos[0] + 0.4f), "móbile: o leve se move mais");
    check(r.pos[1] > 0.1f, "móbile: o leve vai para o lado oposto ao pesado");
}

void testPalcoManualIntocado() {
    // MANUAL: o PAN é o lugar, nada se move, o contrapeso desliga
    Mixer m;
    m.setParameter("pan1", -0.8f); m.setParameter("pan2", 0.3f);
    const float lv[4] = {0.4f, 0.1f, 0.0f, 0.0f};
    const Palco r = runPalco(m, lv, 4.0);
    near(r.pos[0], -0.8f, 1e-5f);
    near(r.pos[1], 0.3f, 1e-5f);
    check(r.excur[0] < 1e-6f && r.excur[1] < 1e-6f, "manual: nada se move sozinho");
}

void testMixerGainAndMute() {
    // (desde 10/11/2026 um sinal constante é todo GRAVE e fica no centro;
    // ganho e mute são testados com os canais no centro)
    Mixer m;
    const float in[4] = {1.0f, 1.0f, 0.0f, 0.0f};
    m.setParameter("gain1", -6.0206f);  // metade
    m.setParameter("gain2", -60.0f);    // ch2 calado pelo ganho
    Frame f = mixOnce(m, in, 400);
    near(f.l, 0.5f * 0.70710678f, 5.0e-3f);
    near(f.r, 0.5f * 0.70710678f, 5.0e-3f);
    m.setParameter("gain2", 0.0f);
    m.setParameter("mute2", 1.0f);      // ch2 calado pelo mute
    f = mixOnce(m, in, 400);
    near(f.l, 0.5f * 0.70710678f, 5.0e-3f);
    // out_gain
    m.setParameter("out_gain", -6.0206f);
    f = mixOnce(m, in, 400);
    near(f.r, 0.25f * 0.70710678f, 5.0e-3f);
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
    // `total` existia contado e nunca verificado — o Clang do macOS
    // acusou como "set but not used". Apagá-lo seria o fácil; ele na
    // verdade é o guarda que faltava: sem esta linha, se o laço interno
    // nunca rodasse, `overCeiling == 0` passaria VAZIO e o teste diria
    // que o limitador está bom sem ter olhado uma amostra sequer.
    check(total > 0, "o laço de medição realmente rodou (teste não-vazio)");
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
    testPalcoTetoEMovimento();
    testPalcoMobile();
    testPalcoManualIntocado();
    testMixerPanLaw();
    testMixerGraveNoCentro();
    testMixerPanSemClique();
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
