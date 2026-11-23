#pragma once

// ============================================================================
// ScoreMidi — as notas do NOTE-OUT numa tomada, como arquivo MIDI (.mid)
// ============================================================================
//
// A pendência do dossiê 38 ("exportação real pra .mid", 23/11/2026): o REC
// já guarda cada nota concluída na partitura (`RasgoEvent::Type::Note`, altura
// em 1V/oct CRU). Aqui elas viram um Standard MIDI File tipo 1, para abrir a
// tomada numa DAW ou num editor de partitura.
//
// Convenções (escritas também no dossiê 38 §Exportação):
//   - altura: 0 V = dó central (MIDI 60), a convenção do VCV Rack; cada
//     1 V = uma oitava. A nota é a mais próxima; o que sobra (microtom: o
//     banco de escalas, AFIN, MRPH) vai num pitch bend antes da nota, na
//     faixa padrão de ±2 semitons (RPN 0 posto no começo de cada canal);
//   - uma trilha por NOTE-OUT (o NOTE-OUT é monofônico), um canal por
//     trilha, pulando o canal 10 (percussão no General MIDI);
//   - tempo: o `bpm` dado (o do CLOCK no fim da tomada, ou 120); 480
//     tiques por semínima. Os tempos vêm da partitura (amostra/sr), não
//     do relógio de parede;
//   - velocidade: `noteVelocity` (0–1) → 1–127.
//
// Sem dependência de plataforma (testado em `tests/test_score_midi.cpp`).

#include "panel/ScoreRecorder.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace rasgo::panel {

namespace detail_midi {
inline void be32(std::string& s, const std::uint32_t v) {
    for (int k = 3; k >= 0; --k) s += static_cast<char>((v >> (8 * k)) & 0xFF);
}
inline void be16(std::string& s, const std::uint16_t v) {
    s += static_cast<char>((v >> 8) & 0xFF);
    s += static_cast<char>(v & 0xFF);
}
inline void vlq(std::string& s, std::uint32_t v) {
    std::uint8_t b[5];
    int n = 0;
    b[n++] = static_cast<std::uint8_t>(v & 0x7F);
    while ((v >>= 7) != 0) b[n++] = static_cast<std::uint8_t>(0x80 | (v & 0x7F));
    while (n > 0) s += static_cast<char>(b[--n]);
}
struct Ev {
    std::uint32_t tick;
    int order;              // no mesmo tique: desliga (0) antes de curva (1) antes de liga (2)
    std::string bytes;
};
inline std::string track(std::vector<Ev> evs) {
    std::stable_sort(evs.begin(), evs.end(), [](const Ev& a, const Ev& b) {
        return a.tick != b.tick ? a.tick < b.tick : a.order < b.order;
    });
    std::string body;
    std::uint32_t last = 0;
    for (const auto& e : evs) {
        vlq(body, e.tick - last);
        last = e.tick;
        body += e.bytes;
    }
    body += '\0';
    body += static_cast<char>(0xFF);
    body += static_cast<char>(0x2F);
    body += '\0';   // fim da trilha
    std::string t = "MTrk";
    be32(t, static_cast<std::uint32_t>(body.size()));
    return t + body;
}
inline std::string meta(const std::uint8_t type, const std::string& data) {
    std::string s;
    s += static_cast<char>(0xFF);
    s += static_cast<char>(type);
    vlq(s, static_cast<std::uint32_t>(data.size()));
    return s + data;
}
}  // namespace detail_midi

// Quantas notas a tomada tem (sem nota, o app não escreve o .mid).
inline std::size_t countNotes(const std::vector<RasgoEvent>& events) {
    return static_cast<std::size_t>(std::count_if(events.begin(), events.end(),
        [](const RasgoEvent& e) { return e.type == RasgoEvent::Type::Note; }));
}

// O arquivo .mid inteiro (bytes). Vazio se não houver nota.
inline std::string scoreToMidi(const std::vector<RasgoEvent>& events, double bpm) {
    using namespace detail_midi;
    if (countNotes(events) == 0) return {};
    if (!(bpm > 1.0) || !(bpm < 1000.0)) bpm = 120.0;
    constexpr std::uint16_t kPpq = 480;
    const auto tickOf = [&](const double sec) {
        const double t = std::max(0.0, sec) * bpm / 60.0 * kPpq;
        return static_cast<std::uint32_t>(std::llround(t));
    };

    // trilha 0: tempo e nome
    std::vector<Ev> head;
    {
        const auto usPerQuarter = static_cast<std::uint32_t>(std::llround(60000000.0 / bpm));
        std::string d;
        d += static_cast<char>((usPerQuarter >> 16) & 0xFF);
        d += static_cast<char>((usPerQuarter >> 8) & 0xFF);
        d += static_cast<char>(usPerQuarter & 0xFF);
        head.push_back({0, 0, meta(0x03, "Rasgo Modular")});
        head.push_back({0, 1, meta(0x51, d)});
    }

    // uma trilha por NOTE-OUT (pelo nó), na ordem dos nós
    std::map<std::size_t, std::vector<const RasgoEvent*>> byNode;
    for (const auto& e : events)
        if (e.type == RasgoEvent::Type::Note) byNode[e.targetNode].push_back(&e);

    std::vector<std::string> tracks{track(head)};
    int chIdx = 0;
    for (const auto& [node, notes] : byNode) {
        const int ch = chIdx + (chIdx >= 9 ? 1 : 0);   // pula o canal 10
        if (ch > 15) break;                             // 15 vozes bastam
        ++chIdx;
        const auto st = [&](const std::uint8_t s) { return static_cast<char>(s | ch); };
        std::vector<Ev> evs;
        evs.push_back({0, 0, meta(0x03, "NOTE-OUT " + std::to_string(node))});
        // faixa do pitch bend = ±2 semitons (RPN 0), dita explicitamente
        // (uma mensagem por evento: cada uma leva o seu tempo delta)
        for (const auto& cc : {std::pair<int, int>{101, 0}, {100, 0}, {6, 2}, {38, 0}}) {
            std::string m;
            m += st(0xB0);
            m += static_cast<char>(cc.first);
            m += static_cast<char>(cc.second);
            evs.push_back({0, 0, m});
        }
        int lastBend = 8192;
        for (const RasgoEvent* n : notes) {
            const double semi = 60.0 + static_cast<double>(n->notePitch) * 12.0;
            const int key = std::clamp(static_cast<int>(std::lround(semi)), 0, 127);
            const double rest = std::clamp(semi - key, -2.0, 2.0);
            const int bend = std::clamp(static_cast<int>(std::lround(8192.0 + rest / 2.0 * 8192.0)), 0, 16383);
            const int vel = std::clamp(static_cast<int>(std::lround(n->noteVelocity * 127.0f)), 1, 127);
            const std::uint32_t on = tickOf(n->time);
            const std::uint32_t off = std::max(on + 1, tickOf(n->time + n->noteDuration));
            if (bend != lastBend) {
                std::string b;
                b += st(0xE0);
                b += static_cast<char>(bend & 0x7F);
                b += static_cast<char>((bend >> 7) & 0x7F);
                evs.push_back({on, 1, b});
                lastBend = bend;
            }
            std::string a;
            a += st(0x90); a += static_cast<char>(key); a += static_cast<char>(vel);
            evs.push_back({on, 2, a});
            std::string z;
            z += st(0x80); z += static_cast<char>(key); z += static_cast<char>(64);
            evs.push_back({off, 0, z});
        }
        tracks.push_back(track(std::move(evs)));
    }

    std::string out = "MThd";
    be32(out, 6);
    be16(out, 1);                                            // tipo 1
    be16(out, static_cast<std::uint16_t>(tracks.size()));
    be16(out, kPpq);
    for (const auto& t : tracks) out += t;
    return out;
}

}  // namespace rasgo::panel
