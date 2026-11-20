// Teste isolado do `SIGNAL-IN` (src/dsp/SignalIn.hpp) — evolução do
// `AUDIO-IN`: dois anéis SPSC (áudio via `pushSamples`, MIDI via
// `pushMidi`) + voz monofônica. Single-threaded (chama os dois lados na
// ordem certa) — prova a LÓGICA (índices, resync, pilha de notas), não a
// segurança entre threads (essa vem do `std::atomic` acquire/release).

#include "dsp/SignalIn.hpp"
#include "dsp/EuclidClock.hpp"

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

constexpr float kSr = 48000.0f;

// 6 saídas: out(L) r pitch gate vel cc
std::vector<AudioBlock> mkOut(std::size_t frames) {
    return std::vector<AudioBlock>(6, AudioBlock(kSr, 2, frames));
}

// ==== ÁUDIO (herdado do AUDIO-IN) ====

void testSilentWithoutFeed() {
    SignalIn a;
    a.setParameter("listen", 1.0f);   // ON (v0.1.3)
    a.prepare(kSr, 64);
    auto out = mkOut(64);
    for (int b = 0; b < 20; ++b) a.process({}, out);
    for (std::size_t i = 0; i < 64; ++i) {
        check(out[0].at(0, i) == 0.0f, "sem feed: áudio L em silêncio");
        check(out[1].at(0, i) == 0.0f, "sem feed: áudio R em silêncio");
        check(out[3].at(0, i) == 0.0f, "sem MIDI: gate em 0");
    }
}

void testAudioRoundTrip() {
    SignalIn a;
    a.setParameter("listen", 1.0f);   // ON (v0.1.3)
    a.prepare(kSr, 64);
    std::vector<float> feed(64 * 2);
    for (std::size_t i = 0; i < 64; ++i) {
        feed[i * 2] = static_cast<float>(i) / 64.0f;
        feed[i * 2 + 1] = -static_cast<float>(i) / 64.0f;
    }
    a.pushSamples(feed.data(), 64);
    auto out = mkOut(64);
    a.process({}, out);
    bool same = true;
    for (std::size_t i = 0; i < 64; ++i) {
        if (out[0].at(0, i) != feed[i * 2]) same = false;
        if (out[1].at(0, i) != feed[i * 2 + 1]) same = false;
    }
    check(same, "áudio: entra por pushSamples, sai idêntico (L→out, R→r)");
}

void testGainApplied() {
    SignalIn a;
    a.setParameter("listen", 1.0f);   // ON (v0.1.3)
    a.prepare(kSr, 64);
    a.setParameter("gain", 0.5f);
    std::vector<float> feed(64 * 2, 0.0f);
    for (std::size_t i = 0; i < 64; ++i) feed[i * 2] = 0.8f;
    a.pushSamples(feed.data(), 64);
    auto out = mkOut(64);
    a.process({}, out);
    check(std::fabs(out[0].at(0, 0) - 0.4f) < 1e-6f, "gain 0,5 (0,8→0,4)");
}

void testUnderrunSilentNotGarbage() {
    SignalIn a;
    a.setParameter("listen", 1.0f);   // ON (v0.1.3)
    a.prepare(kSr, 64);
    std::vector<float> feed(30 * 2, 0.0f);
    for (std::size_t i = 0; i < 30; ++i) feed[i * 2] = 1.0f;
    a.pushSamples(feed.data(), 30);
    auto out = mkOut(64);
    a.process({}, out);
    bool ok30 = true, rest0 = true;
    for (std::size_t i = 0; i < 30; ++i) if (out[0].at(0, i) != 1.0f) ok30 = false;
    for (std::size_t i = 30; i < 64; ++i) if (out[0].at(0, i) != 0.0f) rest0 = false;
    check(ok30 && rest0, "underrun: chegou certo, resto em silêncio");
}

void testOverflowResyncs() {
    SignalIn a;
    a.setParameter("listen", 1.0f);   // ON (v0.1.3)
    a.prepare(kSr, 64);
    const std::size_t total = 65536 * 3;
    std::vector<float> chunk(1000 * 2);
    std::size_t pushed = 0;
    while (pushed < total) {
        const std::size_t n = std::min<std::size_t>(1000, total - pushed);
        for (std::size_t i = 0; i < n; ++i) {
            chunk[i * 2] = static_cast<float>(pushed + i);
            chunk[i * 2 + 1] = 0.0f;
        }
        a.pushSamples(chunk.data(), n);
        pushed += n;
    }
    auto out = mkOut(64);
    a.process({}, out);
    for (std::size_t i = 0; i < 64; ++i)
        EXPECT(std::isfinite(out[0].at(0, i)));
    const float lo = static_cast<float>(total) - 65536.0f;
    check(out[0].at(0, 0) >= lo - 1.0f && out[0].at(0, 0) < (float)total,
          "estouro: resincroniza pra janela recente, não lê dado sobrescrito");
}

// ==== MIDI ====

void render(SignalIn& s, int blocks, std::vector<AudioBlock>& out) {
    for (int b = 0; b < blocks; ++b) s.process({}, out);
}

void testNoteOnGatePitchVel() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    s.prepare(kSr, 128);
    auto out = mkOut(128);
    render(s, 4, out);
    EXPECT(out[3].at(0, 127) == 0.0f);   // gate baixo

    s.pushMidi(0x90, 72, 100);   // nota 72 (C5), vel 100
    render(s, 30, out);
    // gate subiu (rampa de 1 ms — depois de 30 blocos já está em 1)
    EXPECT(out[3].at(0, 127) > 0.95f);
    // pitch: (72 - 60)/12 = 1,0 oitava
    EXPECT(std::fabs(out[2].at(0, 0) - 1.0f) < 1e-4f);
    // vel: 100/127
    EXPECT(std::fabs(out[4].at(0, 0) - 100.0f / 127.0f) < 1e-4f);

    s.pushMidi(0x80, 72, 0);     // note off
    render(s, 30, out);
    EXPECT(out[3].at(0, 127) < 0.05f);   // gate desceu
}

void testLastNotePriorityStack() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    s.prepare(kSr, 128);
    auto out = mkOut(128);
    s.pushMidi(0x90, 60, 80);   // C4
    render(s, 5, out);
    EXPECT(std::fabs(out[2].at(0, 0) - 0.0f) < 1e-4f);
    s.pushMidi(0x90, 67, 80);   // G4 por cima → -7... (67-60)/12
    render(s, 5, out);
    EXPECT(std::fabs(out[2].at(0, 0) - 7.0f / 12.0f) < 1e-4f);
    s.pushMidi(0x80, 67, 0);    // solta o G4 → volta pro C4
    render(s, 5, out);
    EXPECT(std::fabs(out[2].at(0, 0) - 0.0f) < 1e-4f);
    EXPECT(out[3].at(0, 0) > 0.9f);   // gate segue alto (C4 ainda pressionado)
    s.pushMidi(0x80, 60, 0);
    render(s, 5, out);
    EXPECT(out[3].at(0, 127) < 0.05f);
}

void testNoteOnZeroVelIsOff() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    s.prepare(kSr, 128);
    auto out = mkOut(128);
    s.pushMidi(0x90, 64, 90);
    render(s, 5, out);
    EXPECT(out[3].at(0, 0) > 0.9f);
    s.pushMidi(0x90, 64, 0);   // note-on vel 0 = note-off
    render(s, 5, out);
    EXPECT(out[3].at(0, 127) < 0.05f);
}

void testPitchBend() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    s.prepare(kSr, 128);
    s.setParameter("bend", 12.0f);   // ±1 oitava no fundo de escala
    auto out = mkOut(128);
    s.pushMidi(0x90, 60, 80);
    render(s, 3, out);
    EXPECT(std::fabs(out[2].at(0, 0)) < 1e-4f);
    s.pushMidi(0xE0, 0, 96);   // bend máximo: (96<<7 | 0) = 12288; (12288-8192)/8192 = 0,5
    render(s, 3, out);
    // +0,5 · 12 semitons / 12 = +0,5 oitava
    EXPECT(std::fabs(out[2].at(0, 0) - 0.5f) < 2e-3f);
    s.pushMidi(0xE0, 0, 64);   // centro
    render(s, 3, out);
    EXPECT(std::fabs(out[2].at(0, 0)) < 5e-3f);
}

void testCcFollowsSelectedNumber() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    s.prepare(kSr, 128);
    s.setParameter("cc_num", 7.0f);   // volume
    auto out = mkOut(128);
    s.pushMidi(0xB0, 1, 127);   // CC1 (mod) — ignorado
    render(s, 3, out);
    EXPECT(out[5].at(0, 0) == 0.0f);
    s.pushMidi(0xB0, 7, 64);    // CC7 — segue
    render(s, 3, out);
    EXPECT(std::fabs(out[5].at(0, 0) - 64.0f / 127.0f) < 1e-4f);
}

void testDeterminism() {
    auto run = []() {
        SignalIn s;
        s.setParameter("listen", 1.0f);   // ON (v0.1.3)
        s.prepare(kSr, 64);
        std::vector<float> feed(64 * 2);
        for (std::size_t i = 0; i < feed.size(); ++i)
            feed[i] = static_cast<float>(i % 7) * 0.1f - 0.3f;
        s.pushSamples(feed.data(), 64);
        s.pushMidi(0x90, 65, 100);
        s.pushMidi(0xE0, 10, 70);
        s.pushMidi(0xB0, 1, 40);
        auto out = mkOut(64);
        s.process({}, out);
        std::vector<float> r;
        for (int o = 0; o < 6; ++o)
            for (std::size_t i = 0; i < 64; ++i) r.push_back(out[static_cast<std::size_t>(o)].at(0, i));
        return r;
    };
    const auto a = run();
    const auto b = run();
    bool same = a.size() == b.size();
    for (std::size_t i = 0; same && i < a.size(); ++i) same = a[i] == b[i];
    EXPECT(same);
}

void testType() {
    SignalIn s;
    s.setParameter("listen", 1.0f);   // ON (v0.1.3)
    EXPECT(s.type() == "SIGNAL-IN");
}

}  // namespace

// ON desligado (o padrão, v0.1.3): nada de fora entra — nem áudio nem
// MIDI — e uma nota pendente não dispara ao ligar.
void testDesligadoNaoDeixaEntrar() {
    SignalIn s;
    check(s.parameterValue("listen") == 0.0f, "ON nasce desligado");
    s.prepare(kSr, 64);
    std::vector<float> feed(64 * 2, 0.5f);
    s.pushSamples(feed.data(), 64);
    s.pushMidi(0x90, 72, 100);
    auto out = mkOut(64);
    s.process({}, out);
    bool mudo = true;
    for (std::size_t i = 0; i < 64; ++i)
        for (int o = 0; o < 6; ++o)
            if (out[static_cast<std::size_t>(o)].at(0, i) != 0.0f) mudo = false;
    check(mudo, "desligado: todas as saídas em zero");
    s.setParameter("listen", 1.0f);
    for (int b = 0; b < 20; ++b) s.process({}, out);
    check(out[3].at(0, 63) == 0.0f, "nota de antes de ligar não dispara");
    s.pushMidi(0x90, 72, 100);
    for (int b = 0; b < 20; ++b) s.process({}, out);
    check(out[3].at(0, 63) > 0.5f, "ligado: nota nova abre o gate");
}

// ==== MIDI CLOCK (20/11/2026) ====
// Simula a placa: os clocks "chegam" no seu instante verdadeiro e só são
// entregues no começo do PERÍODO da placa seguinte (como no app: o MIDI vai
// para o anel e o áudio o entrega a cada callback, que roda vários
// sub-blocos de 256 de uma vez).
struct ClockRun {
    std::vector<long> edges;     // bordas de subida do CLK (amostra)
    std::vector<long> rstEdges;
    float lastRun = 0.0f;
};

ClockRun runClock(SignalIn& s, double bpmA, double bpmB, double changeAt,
                  long total, int devPeriod, bool start = true, long stopAt = -1,
                  bool stamps = true) {
    const int blk = 256;
    std::vector<AudioBlock> out(9, AudioBlock(kSr, 2, blk));
    ClockRun r;
    double next = 0.0;              // instante verdadeiro do próximo clock
    bool started = false;
    float prevClk = 0.0f, prevRst = 0.0f;
    for (long base = 0; base < total; base += blk) {
        if (base % devPeriod == 0) {            // começo de um período da placa
            if (start && !started) { s.pushMidi(0xFA, 0, 0); started = true; }
            if (stopAt >= 0 && base >= stopAt) s.pushMidi(0xFC, 0, 0);
            while (next < static_cast<double>(base)) {
                // com carimbo: o instante verdadeiro (como o app calcula do
                // timestamp do JUCE); sem carimbo: o começo do bloco
                s.pushMidi(0xF8, 0, 0,
                           stamps ? static_cast<float>(next - static_cast<double>(base)) : 0.0f);
                const double bpm = next < changeAt ? bpmA : bpmB;
                next += kSr * 60.0 / (bpm * 24.0);
            }
        }
        s.process({}, out);
        for (int f = 0; f < blk; ++f) {
            const float c = out[6].at(0, static_cast<std::size_t>(f));
            const float q = out[8].at(0, static_cast<std::size_t>(f));
            if (prevClk < 0.5f && c >= 0.5f) r.edges.push_back(base + f);
            if (prevRst < 0.5f && q >= 0.5f) r.rstEdges.push_back(base + f);
            prevClk = c; prevRst = q;
        }
        r.lastRun = out[7].at(0, blk - 1);
    }
    return r;
}

double worstDev(const ClockRun& r, double expect, long after) {
    double w = 0.0;
    for (std::size_t i = 1; i < r.edges.size(); ++i)
        if (r.edges[i - 1] > after)
            w = std::max(w, std::abs(static_cast<double>(r.edges[i] - r.edges[i - 1]) - expect));
    return w;
}

void testMidiClockSteady() {
    // com carimbo de tempo: períodos de placa de 256 e de 1024 amostras;
    // sem carimbo (o tempo é o começo do bloco), o de 256
    struct Caso { int dev; bool stamps; double tol; };
    for (const Caso k : {Caso{256, true, 8.0}, Caso{1024, true, 8.0}, Caso{256, false, 12.0}}) {
        const int dev = k.dev;
        SignalIn s; s.setParameter("listen", 1.0f); s.prepare(kSr, 256);
        const ClockRun r = runClock(s, 120.0, 120.0, 1e18, static_cast<long>(kSr * 6), dev,
                                    true, -1, k.stamps);
        // colcheia a 120 BPM = 12000 amostras
        check(r.edges.size() >= 20, "MIDI clock: pulsos de colcheia");
        const double w = worstDev(r, 12000.0, static_cast<long>(kSr * 2));
        if (w > k.tol) std::cerr << "  periodo " << dev << ": desvio " << w << '\n';
        check(w <= k.tol, "MIDI clock: intervalos estáveis (tremor da placa filtrado)");
        check(r.lastRun > 0.5f, "MIDI clock: RUN alto tocando");
        check(r.rstEdges.size() == 1, "MIDI clock: um RST no START");
        check(!r.edges.empty() && !r.rstEdges.empty()
              && r.rstEdges[0] == r.edges[0] + 1, "MIDI clock: RST uma amostra depois do 1º pulso");
        check(std::abs(s.midiBpm() - 120.0f) < 0.5f, "MIDI clock: andamento medido");
    }
}

void testMidiClockTempoChange() {
    SignalIn s; s.setParameter("listen", 1.0f); s.prepare(kSr, 256);
    const ClockRun r = runClock(s, 120.0, 140.0, kSr * 3.0, static_cast<long>(kSr * 9), 512);
    const double expect = kSr * 60.0 / 140.0 / 2.0;     // colcheia a 140
    const double w = worstDev(r, expect, static_cast<long>(kSr * 5));
    if (w > 4.0) std::cerr << "  mudança: desvio " << w << '\n';
    check(w <= 4.0, "MIDI clock: segue a mudança de andamento");
}

void testMidiClockStopAndDiv() {
    SignalIn s; s.setParameter("listen", 1.0f); s.setParameter("clk_div", 2.0f);
    s.prepare(kSr, 256);
    const long stopAt = static_cast<long>(kSr * 3);
    const ClockRun r = runClock(s, 120.0, 120.0, 1e18, static_cast<long>(kSr * 5), 256, true, stopAt);
    check(!r.edges.empty() && r.edges.back() < stopAt + 2000, "MIDI clock: STOP para os pulsos");
    check(r.lastRun < 0.5f, "MIDI clock: RUN baixo depois do STOP");
    const double w = worstDev(r, 6000.0, static_cast<long>(kSr * 1));   // semicolcheia
    if (w > 4.0) std::cerr << "  DIV: desvio " << w << '\n';
    check(w <= 4.0, "MIDI clock: DIV semicolcheia");
}

void testMidiClockOffAndSilent() {
    SignalIn s; s.prepare(kSr, 256);                       // ON desligado
    const ClockRun r = runClock(s, 120.0, 120.0, 1e18, static_cast<long>(kSr * 2), 256);
    check(r.edges.empty() && r.rstEdges.empty(), "MIDI clock: desligado não deixa entrar");
    SignalIn q; q.setParameter("listen", 1.0f); q.prepare(kSr, 256);
    std::vector<AudioBlock> out(9, AudioBlock(kSr, 2, 256));
    bool quiet = true;
    for (int b = 0; b < 100; ++b) {
        q.process({}, out);
        for (int k = 6; k < 9; ++k) if (out[static_cast<std::size_t>(k)].at(0, 100) != 0.0f) quiet = false;
    }
    check(quiet, "MIDI clock: sem clock, CLK/RUN/RST em zero");
}

// CLK → EXT e RST → RST do CLOCK: o "um" do patch no "um" da DAW
void testMidiClockDrivesClock() {
    SignalIn s; s.setParameter("listen", 1.0f); s.prepare(kSr, 256);
    EuclidClock c; c.prepare(kSr, 256);
    std::vector<AudioBlock> so(9, AudioBlock(kSr, 2, 256));
    std::vector<AudioBlock> co(5, AudioBlock(kSr, 2, 256));
    double next = 0.0; bool started = false;
    float prevClk = 0.0f; int downbeats = 0, edges = 0; bool firstIsOne = false;
    for (long base = 0; base < static_cast<long>(kSr * 4); base += 256) {
        if (!started) { s.pushMidi(0xFA, 0, 0); started = true; }
        while (next < static_cast<double>(base)) { s.pushMidi(0xF8, 0, 0); next += 1000.0; }
        s.process({}, so);
        c.process({&so[6], &so[8], nullptr}, co);
        for (std::size_t f = 0; f < 256; ++f) {
            const float k = so[6].at(0, f);
            if (prevClk < 0.5f && k >= 0.5f) {
                ++edges;
                // a métrica do CLOCK duas amostras depois da borda
                const std::size_t g = std::min<std::size_t>(255, f + 2);
                if (co[4].at(0, g) >= 0.999f) { ++downbeats; if (edges == 1) firstIsOne = true; }
            }
            prevClk = k;
        }
    }
    check(edges > 10, "CLOCK seguiu o CLK do SIGNAL-IN");
    check(firstIsOne, "o primeiro pulso depois do START é o passo 0 do CLOCK");
    check(downbeats >= 1 && downbeats <= edges / 4 + 1, "só o passo 0 tem métrica 1");
}

int main() {
    testSilentWithoutFeed();
    testAudioRoundTrip();
    testGainApplied();
    testUnderrunSilentNotGarbage();
    testOverflowResyncs();
    testNoteOnGatePitchVel();
    testLastNotePriorityStack();
    testNoteOnZeroVelIsOff();
    testPitchBend();
    testCcFollowsSelectedNumber();
    testDeterminism();
    testType();
    testDesligadoNaoDeixaEntrar();
    testMidiClockSteady();
    testMidiClockTempoChange();
    testMidiClockStopAndDiv();
    testMidiClockOffAndSilent();
    testMidiClockDrivesClock();

    if (g_failures == 0) {
        std::cout << "RASGO Modular SIGNAL-IN tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
