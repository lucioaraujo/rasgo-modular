// Teste isolado do POLY-IN (src/dsp/PolyIn.hpp): o MIDI polifônico em até
// 4 vozes — ROTA, MESMA, ALTURA, o roubo da mais antiga, o redisparo de
// 2 ms, o pedal, o bend e o ON desligado.

#include "dsp/PolyIn.hpp"

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

constexpr float kSr = 48000.0f;

struct Rig {
    PolyIn p;
    std::vector<AudioBlock> out = std::vector<AudioBlock>(12, AudioBlock(kSr, 2, 256));
    explicit Rig(float mode, float voices = 4.0f) {
        p.setParameter("listen", 1.0f);
        p.setParameter("mode", mode);
        p.setParameter("voices", voices);
        p.prepare(kSr, 256);
    }
    void on(int n, int v = 100) { p.pushMidi(0x90, static_cast<std::uint8_t>(n), static_cast<std::uint8_t>(v)); }
    void off(int n) { p.pushMidi(0x80, static_cast<std::uint8_t>(n), 0); }
    void run(int blocks = 4) { for (int b = 0; b < blocks; ++b) p.process({}, out); }
    float pitch(int v) const { return out[static_cast<std::size_t>(v) * 3].at(0, 255); }
    float gate(int v) const { return out[static_cast<std::size_t>(v) * 3 + 1].at(0, 255); }
    float vel(int v) const { return out[static_cast<std::size_t>(v) * 3 + 2].at(0, 255); }
};

void testChordRotate() {
    Rig r(0.0f);
    r.on(60); r.on(64); r.on(67); r.run();
    check(std::abs(r.pitch(0) - 0.0f) < 1e-5f, "ROTA: voz 1 = dó");
    check(std::abs(r.pitch(1) - 4.0f / 12.0f) < 1e-5f, "ROTA: voz 2 = mi");
    check(std::abs(r.pitch(2) - 7.0f / 12.0f) < 1e-5f, "ROTA: voz 3 = sol");
    check(r.gate(0) > 0.9f && r.gate(1) > 0.9f && r.gate(2) > 0.9f, "ROTA: três gates abertos");
    check(r.gate(3) < 0.01f, "ROTA: a quarta voz livre");
    check(std::abs(r.vel(0) - 100.0f / 127.0f) < 1e-5f, "velocidade");
    r.off(64); r.run();
    check(r.gate(1) < 0.01f && r.gate(0) > 0.9f, "soltar o mi fecha só a voz 2");
    r.on(65); r.run();   // rodízio: a próxima livre depois da 3 é a 4
    check(std::abs(r.pitch(3) - 5.0f / 12.0f) < 1e-5f, "ROTA: a nota nova vai à próxima voz");
}

void testStealOldest() {
    Rig r(0.0f, 2.0f);
    r.on(60); r.run(1); r.on(62); r.run(1); r.on(64); r.run();
    check(std::abs(r.pitch(0) - 4.0f / 12.0f) < 1e-5f, "sem voz livre: rouba a mais antiga");
    check(std::abs(r.pitch(1) - 2.0f / 12.0f) < 1e-5f, "a mais nova continua");
    // o redisparo: logo depois do roubo o gate fecha por 2 ms
    Rig s(0.0f, 1.0f);
    s.on(60); s.run();
    s.on(62);
    s.p.process({}, s.out);
    float lowest = 1.0f;   // a rampa anti-clique de 1 ms desce dentro dos 2 ms
    for (std::size_t i = 0; i < 120; ++i) lowest = std::min(lowest, s.out[1].at(0, i));
    check(lowest < 0.3f, "a voz roubada fecha o gate (redisparo)");
    s.run();
    check(s.gate(0) > 0.9f, "e abre de novo para a nota nova");
}

void testSameVoice() {
    Rig r(1.0f);
    r.on(60); r.run(); r.off(60); r.run();
    r.on(67); r.run(); r.off(67); r.run();
    r.on(60); r.run();
    check(std::abs(r.pitch(0) - 0.0f) < 1e-5f && r.gate(0) > 0.9f, "MESMA: o dó volta à voz 1");
}

void testByPitch() {
    Rig r(2.0f);
    r.on(67); r.run(1); r.on(60); r.run(1); r.on(64); r.run();
    check(std::abs(r.pitch(0) - 0.0f) < 1e-5f, "ALTURA: voz 1 = a mais grave");
    check(std::abs(r.pitch(1) - 4.0f / 12.0f) < 1e-5f, "ALTURA: voz 2 = a seguinte");
    check(std::abs(r.pitch(2) - 7.0f / 12.0f) < 1e-5f, "ALTURA: voz 3 = a mais aguda");
    r.off(60); r.run();
    check(std::abs(r.pitch(0) - 4.0f / 12.0f) < 1e-5f, "ALTURA: soltou o baixo, o mi vira o baixo");
    check(r.gate(2) < 0.01f, "ALTURA: a voz de cima fica livre");
}

void testSustainAndBend() {
    Rig r(0.0f);
    r.p.pushMidi(0xB0, 64, 127);
    r.on(60); r.run(); r.off(60); r.run();
    check(r.gate(0) > 0.9f, "pedal segura o gate");
    r.p.pushMidi(0xB0, 64, 0); r.run();
    check(r.gate(0) < 0.01f, "soltar o pedal fecha");
    r.on(60); r.p.pushMidi(0xE0, 0x7F, 0x7F); r.run();   // bend máximo, BEND 2
    check(std::abs(r.pitch(0) - (2.0f / 12.0f) * (8191.0f / 8192.0f)) < 1e-4f, "pitch bend em todas");
}

void testOffAndQuiet() {
    PolyIn p; p.prepare(kSr, 256);
    std::vector<AudioBlock> out(12, AudioBlock(kSr, 2, 256));
    p.pushMidi(0x90, 60, 100);
    p.process({}, out);
    bool quiet = true;
    for (auto& b : out) if (b.at(0, 100) != 0.0f) quiet = false;
    check(quiet, "ON desligado: nada entra");
    check(p.type() == "POLY-IN", "tipo");
}

}  // namespace

int main() {
    testChordRotate();
    testStealOldest();
    testSameVoice();
    testByPitch();
    testSustainAndBend();
    testOffAndQuiet();
    if (g_failures == 0) {
        std::cout << "RASGO Modular POLY-IN tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
