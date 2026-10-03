// Teste isolado do `SIGNAL-IN` (src/dsp/SignalIn.hpp) — evolução do
// `AUDIO-IN`: dois anéis SPSC (áudio via `pushSamples`, MIDI via
// `pushMidi`) + voz monofônica. Single-threaded (chama os dois lados na
// ordem certa) — prova a LÓGICA (índices, resync, pilha de notas), não a
// segurança entre threads (essa vem do `std::atomic` acquire/release).

#include "dsp/SignalIn.hpp"

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

    if (g_failures == 0) {
        std::cout << "RASGO Modular SIGNAL-IN tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
