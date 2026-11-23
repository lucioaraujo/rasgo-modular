// As notas do NOTE-OUT como .mid (23/11/2026, pendência do dossiê 38): um
// leitor mínimo de Standard MIDI File confere o que o exportador escreve.

#include "panel/ScoreMidi.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::panel;

namespace {
int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
struct Msg { std::uint32_t tick; int track; std::uint8_t st, d1, d2; std::string meta; std::uint8_t metaType; };
std::uint32_t rd32(const std::string& s, std::size_t p) {
    return (std::uint32_t(std::uint8_t(s[p])) << 24) | (std::uint32_t(std::uint8_t(s[p + 1])) << 16)
         | (std::uint32_t(std::uint8_t(s[p + 2])) << 8) | std::uint8_t(s[p + 3]);
}
std::uint32_t vlq(const std::string& s, std::size_t& p) {
    std::uint32_t v = 0;
    for (;;) { const std::uint8_t b = std::uint8_t(s[p++]); v = (v << 7) | (b & 0x7F); if (!(b & 0x80)) return v; }
}
bool parse(const std::string& f, int& ntracks, int& ppq, std::vector<Msg>& out) {
    if (f.size() < 14 || f.compare(0, 4, "MThd") != 0 || rd32(f, 4) != 6) return false;
    ntracks = (std::uint8_t(f[10]) << 8) | std::uint8_t(f[11]);
    ppq = (std::uint8_t(f[12]) << 8) | std::uint8_t(f[13]);
    std::size_t p = 14;
    for (int t = 0; t < ntracks; ++t) {
        if (f.compare(p, 4, "MTrk") != 0) return false;
        const std::uint32_t len = rd32(f, p + 4);
        std::size_t q = p + 8; const std::size_t end = q + len;
        std::uint32_t tick = 0; bool ended = false;
        while (q < end) {
            tick += vlq(f, q);
            const std::uint8_t st = std::uint8_t(f[q++]);
            if (st == 0xFF) {
                const std::uint8_t ty = std::uint8_t(f[q++]);
                const std::uint32_t l = vlq(f, q);
                out.push_back({tick, t, st, 0, 0, f.substr(q, l), ty});
                q += l;
                if (ty == 0x2F) ended = true;
            } else {
                out.push_back({tick, t, st, std::uint8_t(f[q]), std::uint8_t(f[q + 1]), {}, 0});
                q += 2;
            }
        }
        if (q != end || !ended) return false;
        p = end;
    }
    return p == f.size();
}
RasgoEvent note(double t, std::size_t node, float pitch, double dur, float vel = 0.8f) {
    RasgoEvent e; e.type = RasgoEvent::Type::Note; e.time = t; e.targetNode = node;
    e.notePitch = pitch; e.noteDuration = dur; e.noteVelocity = vel; return e;
}
}  // namespace

int main() {
    check(scoreToMidi({}, 120.0).empty(), "sem nota, sem arquivo");

    std::vector<RasgoEvent> ev;
    RasgoEvent other; other.type = RasgoEvent::Type::ParameterChange; ev.push_back(other);
    ev.push_back(note(0.0, 7, 0.0f, 0.5));                    // dó central, uma semínima a 120
    ev.push_back(note(0.5, 7, 340.0f / 1200.0f, 0.25));       // 340 cents: mi♭ + 40 cents
    ev.push_back(note(1.0, 3, -1.0f, 1.0, 1.0f));             // outro NOTE-OUT, uma oitava abaixo
    const std::string f = scoreToMidi(ev, 120.0);
    int nt = 0, ppq = 0; std::vector<Msg> m;
    check(parse(f, nt, ppq, m), "arquivo MIDI bem formado (cabeçalho, trilhas, fim de trilha)");
    check(nt == 3 && ppq == 480, "tipo 1: tempo + uma trilha por NOTE-OUT; 480 tiques");

    bool tempo = false;
    for (const auto& x : m) if (x.metaType == 0x51 && x.meta.size() == 3
        && ((std::uint8_t(x.meta[0]) << 16) | (std::uint8_t(x.meta[1]) << 8) | std::uint8_t(x.meta[2])) == 500000) tempo = true;
    check(tempo, "120 bpm = 500000 µs por semínima");

    // trilha 1 = nó 3 (ordem dos nós), canal 0; trilha 2 = nó 7, canal 1
    std::vector<Msg> t1, t2;
    for (const auto& x : m) if (x.st != 0xFF) (x.track == 1 ? t1 : t2).push_back(x);
    auto find = [](const std::vector<Msg>& v, std::uint8_t st, std::uint8_t d1) -> const Msg* {
        for (const auto& x : v)
            if (x.st == st && x.d1 == d1) return &x;
        return nullptr;
    };
    const Msg* c4 = find(t2, 0x91, 60);
    const Msg* c4off = find(t2, 0x81, 60);
    check(c4 && c4->tick == 0 && c4->d2 == 102, "dó central (60) no tique 0, velocidade 0,8 → 102");
    check(c4off && c4off->tick == 480, "0,5 s a 120 bpm = 480 tiques");
    const Msg* e4 = find(t2, 0x91, 63);   // 3,4 semitons → 63 com curva +0,4
    check(e4 && e4->tick == 480, "a segunda nota no tique 480");
    const Msg* bend = nullptr;
    for (const auto& x : t2) if (x.st == 0xE1 && x.tick == 480) bend = &x;
    const int bv = bend ? (bend->d1 | (bend->d2 << 7)) : -1;
    check(bend && std::abs(bv - 9830) <= 2, "340 cents = nota 63 com pitch bend de +0,4 semitom (faixa ±2)");
    check(find(t1, 0x90, 48) && find(t1, 0x90, 48)->d2 == 127, "−1 V = uma oitava abaixo (48), no canal 0");
    bool rpn = false;
    for (const auto& x : t2) if (x.st == 0xB1 && x.d1 == 6 && x.d2 == 2) rpn = true;
    check(rpn, "faixa do pitch bend declarada (RPN 0 = 2 semitons)");

    // canal 10 (índice 9) é pulado
    std::vector<RasgoEvent> many;
    for (std::size_t n = 0; n < 11; ++n) many.push_back(note(0.0, n, 0.0f, 0.1));
    std::vector<Msg> mm; int n2 = 0, p2 = 0;
    check(parse(scoreToMidi(many, 90.0), n2, p2, mm), "11 vozes bem formadas");
    bool ch9 = false;
    for (const auto& x : mm) if (x.st != 0xFF && (x.st & 0x0F) == 9) ch9 = true;
    check(!ch9, "o canal 10 (percussão GM) fica livre");

    if (g_failures == 0) { std::cout << "RASGO Modular score MIDI tests passed\n"; return 0; }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
