#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// POLY-IN — o MIDI polifônico em até 4 vozes (Módulo 103)
// ============================================================================
//
// Pedido do autor (20/11/2026): "também havia comentado sobre as
// polifonias" → a proposta de um adaptador que reparte o MIDI de um teclado
// ou de uma DAW em vozes, cada uma com ALTURA (1 V/oct), GATE e VEL — e o
// rack, que já aceita até 4 cópias de cada módulo, toca acordes: 4 OSC, 4
// ENVELOPE. "ok" do autor no mesmo dia. Sem cabo polifônico no núcleo: cada
// voz é um cabo comum (o passo 2, cabos de várias vozes, só se fizer falta).
//
// Como o SIGNAL-IN: adaptador opcional, começa DESLIGADO (ON), recebe o
// MIDI por um anel SPSC (`pushMidi`, mesmo formato), não sabe o que é ALSA.
//
// Repartição (MODE):
//   ROTA   — a próxima voz livre, em rodízio (cada nota numa voz nova:
//            as caudas dos envelopes não se cortam);
//   MESMA  — a nota repetida volta à voz em que tocou (o timbre da nota
//            fica o mesmo); senão, a voz livre há mais tempo;
//   ALTURA — as vozes em ordem de altura: a voz 1 é sempre a nota mais
//            grave que está soando (o baixo do acorde), a 2 a seguinte…
// Sem voz livre, a nota rouba a voz mais antiga. Toda voz roubada ou
// reaproveitada fecha o gate por 2 ms antes da nota nova, para o envelope
// disparar de novo. Pedal de sustentação (CC 64) segura os gates; CC 123
// (all notes off) solta tudo. Pitch bend (BEND semitons) vale para todas.
//
// Referências: a alocação de vozes dos sintetizadores polifônicos (o
// "rotate / reset / unison" dos poli analógicos; o MIDI-CV polifônico do
// VCV Rack, GPL — conceito, nenhum código).

namespace rasgo::modular {

class PolyIn final : public Signal {
public:
    static constexpr int kVoices = 4;

    PolyIn()
        : Signal({},
                 {{"p1", PortKind::Control, "v/oct"}, {"g1", PortKind::Control, "gate"}, {"v1", PortKind::Control, ""},
                  {"p2", PortKind::Control, "v/oct"}, {"g2", PortKind::Control, "gate"}, {"v2", PortKind::Control, ""},
                  {"p3", PortKind::Control, "v/oct"}, {"g3", PortKind::Control, "gate"}, {"v3", PortKind::Control, ""},
                  {"p4", PortKind::Control, "v/oct"}, {"g4", PortKind::Control, "gate"}, {"v4", PortKind::Control, ""}},
                 {{"voices", 1.0f, 4.0f, 4.0f, ""},
                  {"mode", 0.0f, 2.0f, 0.0f, ""},
                  {"bend", 0.0f, 24.0f, 2.0f, "st"},
                  // ON: como no SIGNAL-IN, o MIDI só entra com ele ligado, e
                  // fica fora de todo sorteio (seed, VARIA, MUDA)
                  {"listen", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "POLY-IN"; }

    Panel panel() const override {
        Panel p;
        p.hp = 8;
        p.add(Widget::Kind::Label, "POLY-IN", "", 2.0f, 2.0f);
        p.add(Widget::Kind::Display, "voices", "", 2.5f, 6.0f, 35.0f);
        p.add(Widget::Kind::Knob, "VOZES", "voices", 4.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MODE", "mode", 17.0f, 30.0f);
        p.add(Widget::Kind::Knob, "BEND", "bend", 30.0f, 30.0f);
        p.add(Widget::Kind::Toggle, "ON", "listen", 3.0f, 46.0f);
        for (int v = 0; v < kVoices; ++v) {
            const float y = 62.0f + static_cast<float>(v) * 17.0f;
            const std::string n = std::to_string(v + 1);
            p.add(Widget::Kind::Jack, "P" + n, "out:p" + n, 5.0f, y);
            p.add(Widget::Kind::Jack, "G" + n, "out:g" + n, 18.0f, y);
            p.add(Widget::Kind::Jack, "V" + n, "out:v" + n, 31.0f, y);
        }
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate > 0 ? sampleRate : 48000.0f;
        rampCoef_ = 1.0f - std::exp(-1.0f / (0.001f * sr_));
        midiWrite_.store(0, std::memory_order_relaxed);
        midiRead_ = 0;
        allOff(true);
    }

    // ---- PRODUTOR MIDI (mesmo formato do SIGNAL-IN; o instante é ignorado)
    void pushMidi(const std::uint8_t status, const std::uint8_t d1,
                  const std::uint8_t d2, const float = 0.0f) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_relaxed);
        midi_[w & kMidiMask] = (static_cast<std::uint32_t>(status) << 16)
                             | (static_cast<std::uint32_t>(d1) << 8)
                             | static_cast<std::uint32_t>(d2);
        midiWrite_.store(w + 1, std::memory_order_release);
    }

    // para o visor e os testes
    int voiceNote(const int v) const noexcept { return voice_[static_cast<std::size_t>(v)].note; }
    bool voiceGate(const int v) const noexcept { return voice_[static_cast<std::size_t>(v)].on; }

    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        if (parameterValue("listen") < 0.5f) {
            drain(true);                  // MIDI pendente descartado
            allOff(true);
            for (auto& o : outputs) o.clear();
            return;
        }
        nVoices_ = std::min(kVoices, std::max(1, static_cast<int>(parameterValue("voices") + 0.5f)));
        mode_ = std::min(2, std::max(0, static_cast<int>(parameterValue("mode") + 0.5f)));
        for (int v = nVoices_; v < kVoices; ++v) voice_[static_cast<std::size_t>(v)].on = false;
        drain(false);
        const float bendV = bend_ * parameterValue("bend") / 12.0f;
        const int gap = static_cast<int>(sr_ * 0.002f);

        for (std::size_t f = 0; f < frames; ++f) {
            for (int v = 0; v < kVoices; ++v) {
                auto& vo = voice_[static_cast<std::size_t>(v)];
                if (vo.retrig > 0) { vo.retrig = std::min(vo.retrig, gap); --vo.retrig; }
                const float target = (vo.on && vo.retrig == 0) ? 1.0f : 0.0f;
                vo.ramp += (target - vo.ramp) * rampCoef_;
                const float pitch = static_cast<float>(vo.note - 60) / 12.0f + bendV;
                const std::size_t base = static_cast<std::size_t>(v) * 3;
                for (std::size_t c = 0; c < channels; ++c) {
                    outputs[base].at(c, f) = pitch;
                    outputs[base + 1].at(c, f) = vo.ramp;
                    outputs[base + 2].at(c, f) = vo.vel;
                }
            }
        }
    }

private:
    struct Voice {
        int note = 60;
        float vel = 0.0f;
        bool on = false;          // gate (tecla ou pedal)
        bool held = false;        // a tecla ainda está apertada
        long age = 0;             // quando começou (para roubar a mais antiga)
        long freed = 0;           // quando ficou livre (MESMA: a livre há mais tempo)
        int retrig = 0;           // amostras de gate fechado antes da nota nova
        float ramp = 0.0f;
    };

    void allOff(const bool reset) noexcept {
        for (auto& v : voice_) {
            v.on = false; v.held = false;
            if (reset) { v = Voice{}; }
        }
        heldCount_ = 0;
        sustain_ = false;
    }

    void drain(const bool discard) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_acquire);
        while (midiRead_ != w) {
            const std::uint32_t ev = midi_[midiRead_ & kMidiMask];
            ++midiRead_;
            if (discard) continue;
            const std::uint8_t raw = static_cast<std::uint8_t>((ev >> 16) & 0xFF);
            if (raw >= 0xF0) continue;                   // clock etc.: é do SIGNAL-IN
            const std::uint8_t st = raw & 0xF0;
            const int d1 = static_cast<int>((ev >> 8) & 0x7F);
            const int d2 = static_cast<int>(ev & 0x7F);
            if (st == 0x90 && d2 > 0) noteOn(d1, d2);
            else if (st == 0x80 || st == 0x90) noteOff(d1);
            else if (st == 0xB0 && d1 == 64) pedal(d2 >= 64);
            else if (st == 0xB0 && d1 == 123) { for (auto& v : voice_) { v.on = false; v.held = false; } heldCount_ = 0; }
            else if (st == 0xE0) bend_ = (static_cast<float>((d2 << 7) | d1) - 8192.0f) / 8192.0f;
        }
    }

    void noteOn(const int note, const int vel) noexcept {
        ++clock_;
        if (mode_ == 2) { addHeld(note, vel); revoiceByPitch(); return; }
        int pick = -1;
        for (int v = 0; v < nVoices_; ++v)               // a mesma nota já soando
            if (voice_[static_cast<std::size_t>(v)].on && voice_[static_cast<std::size_t>(v)].note == note) { pick = v; break; }
        if (pick < 0 && mode_ == 1) {                    // MESMA: a voz em que ela tocou
            for (int v = 0; v < nVoices_; ++v)
                if (!voice_[static_cast<std::size_t>(v)].on && voice_[static_cast<std::size_t>(v)].note == note) { pick = v; break; }
            if (pick < 0) {                              // senão a livre há mais tempo
                long best = 0;
                for (int v = 0; v < nVoices_; ++v) {
                    const auto& vo = voice_[static_cast<std::size_t>(v)];
                    if (!vo.on && (pick < 0 || vo.freed < best)) { pick = v; best = vo.freed; }
                }
            }
        }
        if (pick < 0 && mode_ == 0) {                    // ROTA: a próxima livre
            for (int k = 0; k < nVoices_; ++k) {
                const int v = (next_ + k) % nVoices_;
                if (!voice_[static_cast<std::size_t>(v)].on) { pick = v; break; }
            }
        }
        if (pick < 0) {                                  // sem livre: rouba a mais antiga
            long best = 0;
            for (int v = 0; v < nVoices_; ++v) {
                const auto& vo = voice_[static_cast<std::size_t>(v)];
                if (pick < 0 || vo.age < best) { pick = v; best = vo.age; }
            }
        }
        auto& vo = voice_[static_cast<std::size_t>(pick)];
        if (vo.on || vo.ramp > 0.01f) vo.retrig = static_cast<int>(sr_ * 0.002f);
        vo.note = note; vo.vel = static_cast<float>(vel) / 127.0f;
        vo.on = true; vo.held = true; vo.age = clock_;
        next_ = (pick + 1) % nVoices_;
    }

    void noteOff(const int note) noexcept {
        ++clock_;
        if (mode_ == 2) {
            removeHeld(note);
            if (!sustain_) revoiceByPitch();
            return;
        }
        for (int v = 0; v < kVoices; ++v) {
            auto& vo = voice_[static_cast<std::size_t>(v)];
            if (vo.held && vo.note == note) {
                vo.held = false;
                if (!sustain_) { vo.on = false; vo.freed = clock_; }
            }
        }
    }

    void pedal(const bool down) noexcept {
        sustain_ = down;
        if (down) return;
        ++clock_;
        if (mode_ == 2) { pruneReleased(); revoiceByPitch(); return; }
        for (auto& vo : voice_)
            if (vo.on && !vo.held) { vo.on = false; vo.freed = clock_; }
    }

    // ALTURA: a lista das notas soando, em ordem de altura, nas vozes 1..N
    void addHeld(const int note, const int vel) noexcept {
        for (int i = 0; i < heldCount_; ++i)
            if (held_[static_cast<std::size_t>(i)].note == note) {
                held_[static_cast<std::size_t>(i)] = {note, vel, true};
                return;
            }
        if (heldCount_ >= static_cast<int>(held_.size())) return;
        held_[static_cast<std::size_t>(heldCount_++)] = {note, vel, true};
        std::sort(held_.begin(), held_.begin() + heldCount_,
                  [](const Held& a, const Held& b) { return a.note < b.note; });
    }
    void removeHeld(const int note) noexcept {
        for (int i = 0; i < heldCount_; ++i)
            if (held_[static_cast<std::size_t>(i)].note == note) held_[static_cast<std::size_t>(i)].down = false;
        if (!sustain_) pruneReleased();
    }
    void pruneReleased() noexcept {
        int w = 0;
        for (int i = 0; i < heldCount_; ++i)
            if (held_[static_cast<std::size_t>(i)].down) held_[static_cast<std::size_t>(w++)] = held_[static_cast<std::size_t>(i)];
        heldCount_ = w;
    }
    void revoiceByPitch() noexcept {
        for (int v = 0; v < nVoices_; ++v) {
            auto& vo = voice_[static_cast<std::size_t>(v)];
            if (v < heldCount_) {
                const auto& h = held_[static_cast<std::size_t>(v)];
                if (!vo.on) vo.retrig = 0;
                else if (vo.note != h.note) vo.retrig = static_cast<int>(sr_ * 0.002f);
                vo.note = h.note; vo.vel = static_cast<float>(h.vel) / 127.0f;
                vo.on = true; vo.held = true;
            } else {
                vo.on = false; vo.held = false;
            }
        }
    }

    struct Held { int note = 0; int vel = 0; bool down = false; };

    static constexpr std::size_t kMidiCap = 1024;
    static constexpr std::size_t kMidiMask = kMidiCap - 1;
    std::array<std::uint32_t, kMidiCap> midi_{};
    std::atomic<std::size_t> midiWrite_{0};
    std::size_t midiRead_ = 0;

    std::array<Voice, kVoices> voice_{};
    std::array<Held, 32> held_{};
    int heldCount_ = 0;
    int nVoices_ = kVoices, mode_ = 0, next_ = 0;
    long clock_ = 0;
    bool sustain_ = false;
    float bend_ = 0.0f, sr_ = 48000.0f, rampCoef_ = 1.0f;
};

}  // namespace rasgo::modular
