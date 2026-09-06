#pragma once

#include "core/SignalGraph.hpp"

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// ============================================================================
// SIGNAL-IN — adaptador de entrada: áudio + MIDI + CV (Módulo 49)
// ============================================================================
//
// Evolução do `AUDIO-IN` (#35) — decisão do autor 2026-09-06: em vez de
// módulos `MIDI-IN` e `CV-IN` separados, UM adaptador. Contraparte de
// ENTRADA do `NOTE-OUT`. Identidade RASGO: nó adaptador OPCIONAL, nunca
// dependência — o painel soa sozinho sem ele; `rasgo_modular_core` não
// sabe o que é ALSA nem ALSA-seq.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/49_signal_in.md`.
//
// Dois anéis SPSC lock-free, alimentados de threads externas
// (`apps/panel/`):
//   - ÁUDIO: `pushSamples(interleavedLR, frames)` — igual ao `AUDIO-IN`.
//   - MIDI:  `pushMidi(status, d1, d2)` — eventos empacotados; `process()`
//            drena e resolve uma voz MONOFÔNICA (last-note, com pilha).
//
// Saídas: `out` (áudio L — nome preservado pra compat de patch salvo),
// `r` (áudio R), `pitch` (1 V/oct, nota 60 = 0 V, + pitch-bend), `gate`,
// `vel`, `cc` (o CC nº `cc_num`). Sem nada alimentando → tudo em silêncio,
// determinístico (testável sem hardware).
//
// COMPAT: `makeModule("AUDIO-IN")` também constrói este módulo (alias);
// `type()` devolve "SIGNAL-IN", então re-salvar um `.rmp` antigo migra.

namespace rasgo::modular {

class SignalIn final : public Signal {
public:
    SignalIn()
        : Signal({},
                 {{"out", PortKind::Audio, ""},      // áudio L (compat: era "out")
                  {"r", PortKind::Audio, ""},        // áudio R
                  {"pitch", PortKind::Control, "v/oct"},
                  {"gate", PortKind::Control, "gate"},
                  {"vel", PortKind::Control, ""},
                  {"cc", PortKind::Control, ""}},
                 {{"gain", 0.0f, 2.0f, 1.0f, ""},
                  {"bend", 0.0f, 24.0f, 2.0f, "st"},
                  {"cc_num", 0.0f, 127.0f, 1.0f, ""}}) {}

    std::string type() const override { return "SIGNAL-IN"; }

    Panel panel() const override {
        Panel p;
        p.hp = 6;
        p.add(Widget::Kind::Label, "SIGNAL-IN", "", 2.0f, 2.0f);
        p.add(Widget::Kind::Display, "in", "", 2.5f, 6.0f, 25.0f);
        p.add(Widget::Kind::Knob, "GAIN", "gain", 4.0f, 30.0f);
        p.add(Widget::Kind::Knob, "BEND", "bend", 17.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CC#", "cc_num", 10.5f, 50.0f);
        p.add(Widget::Kind::Jack, "L", "out:out", 5.0f, 78.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 16.0f, 78.0f);
        p.add(Widget::Kind::Jack, "1V/O", "out:pitch", 5.0f, 96.0f);
        p.add(Widget::Kind::Jack, "GATE", "out:gate", 18.0f, 96.0f);
        p.add(Widget::Kind::Jack, "VEL", "out:vel", 5.0f, 114.0f);
        p.add(Widget::Kind::Jack, "CC", "out:cc", 18.0f, 114.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        std::size_t cap = 1;
        while (cap < static_cast<std::size_t>(sampleRate) + 1) cap <<= 1;
        ring_.assign(cap * 2, 0.0f);
        mask_ = cap - 1;
        writeIdx_.store(0, std::memory_order_relaxed);
        readIdx_ = 0;

        midiWrite_.store(0, std::memory_order_relaxed);
        midiRead_ = 0;
        for (auto& e : midi_) e = 0;

        noteCount_ = 0;
        curNote_ = 60;
        gateHigh_ = false;
        curVel_ = 0.0f;
        ccVal_ = 0.0f;
        bendNorm_ = 0.0f;
        gateRamp_ = 0.0f;
        rampCoef_ = 1.0f - std::exp(-1.0f / (0.001f * std::max(1.0f, sampleRate)));
    }

    // ---- PRODUTOR ÁUDIO (thread de captura ALSA) ----
    void pushSamples(const float* interleavedLR, const std::size_t frames) noexcept {
        if (ring_.empty()) return;
        const std::size_t cap = mask_ + 1;
        const std::size_t w = writeIdx_.load(std::memory_order_relaxed);
        const std::size_t n = frames > cap ? cap : frames;
        const std::size_t start = frames > cap ? frames - cap : 0;
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t slot = (w + i) & mask_;
            ring_[slot * 2] = interleavedLR[(start + i) * 2];
            ring_[slot * 2 + 1] = interleavedLR[(start + i) * 2 + 1];
        }
        writeIdx_.store(w + n, std::memory_order_release);
    }

    // ---- PRODUTOR MIDI (thread ALSA-seq) ----
    // status/d1/d2 = bytes MIDI crus (canal já mascarado ou não — só o
    // nibble alto importa aqui). Nunca bloqueia; anel de 1024 eventos.
    void pushMidi(const std::uint8_t status, const std::uint8_t d1,
                  const std::uint8_t d2) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_relaxed);
        midi_[w & kMidiMask] = (static_cast<std::uint32_t>(status) << 16)
                             | (static_cast<std::uint32_t>(d1) << 8)
                             | static_cast<std::uint32_t>(d2);
        midiWrite_.store(w + 1, std::memory_order_release);
    }

    // ---- CONSUMIDOR (thread de áudio do grafo) ----
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& oL = outputs[0];
        AudioBlock& oR = outputs[1];
        AudioBlock& oPitch = outputs[2];
        AudioBlock& oGate = outputs[3];
        AudioBlock& oVel = outputs[4];
        AudioBlock& oCc = outputs[5];
        const std::size_t frames = oL.frames();
        const std::size_t channels = oL.channels();

        const float gain = parameterValue("gain");
        const float bendSt = parameterValue("bend");
        const int ccNum = static_cast<int>(parameterValue("cc_num") + 0.5f);

        // drena todos os eventos MIDI pendentes (uma vez por bloco — a
        // granularidade de bloco é suficiente pra controle)
        drainMidi(ccNum);

        const float pitchV = static_cast<float>(curNote_ - 60) / 12.0f
                           + bendNorm_ * bendSt / 12.0f;
        const float gateTarget = gateHigh_ ? 1.0f : 0.0f;

        // ---- áudio ----
        std::size_t n = 0;
        std::size_t base = 0;
        if (!ring_.empty()) {
            const std::size_t cap = mask_ + 1;
            const std::size_t w = writeIdx_.load(std::memory_order_acquire);
            if (w - readIdx_ > cap) readIdx_ = w - cap;
            const std::size_t avail = w - readIdx_;
            n = avail < frames ? avail : frames;
            base = readIdx_;
            readIdx_ += n;
        }

        for (std::size_t f = 0; f < frames; ++f) {
            float l = 0.0f, r = 0.0f;
            if (f < n && !ring_.empty()) {
                const std::size_t slot = (base + f) & mask_;
                l = ring_[slot * 2] * gain;
                r = ring_[slot * 2 + 1] * gain;
            }
            gateRamp_ += (gateTarget - gateRamp_) * rampCoef_;
            for (std::size_t c = 0; c < channels; ++c) {
                oL.at(c, f) = l;
                oR.at(c, f) = r;
                oPitch.at(c, f) = pitchV;
                oGate.at(c, f) = gateRamp_;
                oVel.at(c, f) = curVel_;
                oCc.at(c, f) = ccVal_;
            }
        }
    }

private:
    void drainMidi(const int ccNum) noexcept {
        const std::size_t w = midiWrite_.load(std::memory_order_acquire);
        while (midiRead_ != w) {
            const std::uint32_t ev = midi_[midiRead_ & kMidiMask];
            ++midiRead_;
            const std::uint8_t st = static_cast<std::uint8_t>((ev >> 16) & 0xF0);
            const std::uint8_t d1 = static_cast<std::uint8_t>((ev >> 8) & 0x7F);
            const std::uint8_t d2 = static_cast<std::uint8_t>(ev & 0x7F);
            switch (st) {
            case 0x90:
                if (d2 > 0) { noteOn(d1, d2); break; }
                [[fallthrough]];   // note-on vel 0 = note-off
            case 0x80:
                noteOff(d1);
                break;
            case 0xB0:
                if (d1 == ccNum) ccVal_ = static_cast<float>(d2) / 127.0f;
                break;
            case 0xE0:
                bendNorm_ = (static_cast<float>((d2 << 7) | d1) - 8192.0f)
                          / 8192.0f;
                break;
            default:
                break;
            }
        }
    }

    void noteOn(const std::uint8_t note, const std::uint8_t vel) noexcept {
        if (noteCount_ < static_cast<int>(stack_.size()))
            stack_[static_cast<std::size_t>(noteCount_++)] = note;
        curNote_ = note;
        curVel_ = static_cast<float>(vel) / 127.0f;
        gateHigh_ = true;
    }

    void noteOff(const std::uint8_t note) noexcept {
        int w = 0;
        for (int i = 0; i < noteCount_; ++i)
            if (stack_[static_cast<std::size_t>(i)] != note)
                stack_[static_cast<std::size_t>(w++)] = stack_[static_cast<std::size_t>(i)];
        noteCount_ = w;
        if (noteCount_ > 0) {
            curNote_ = stack_[static_cast<std::size_t>(noteCount_ - 1)];
            gateHigh_ = true;
        } else {
            gateHigh_ = false;
        }
    }

    static constexpr std::size_t kMidiCap = 1024;
    static constexpr std::size_t kMidiMask = kMidiCap - 1;

    // áudio
    std::vector<float> ring_;
    std::size_t mask_ = 0;
    std::atomic<std::size_t> writeIdx_{0};
    std::size_t readIdx_ = 0;

    // MIDI
    std::array<std::uint32_t, kMidiCap> midi_{};
    std::atomic<std::size_t> midiWrite_{0};
    std::size_t midiRead_ = 0;

    // voz monofônica
    std::array<std::uint8_t, 16> stack_{};
    int noteCount_ = 0;
    int curNote_ = 60;
    bool gateHigh_ = false;
    float curVel_ = 0.0f;
    float ccVal_ = 0.0f;
    float bendNorm_ = 0.0f;
    float gateRamp_ = 0.0f;
    float rampCoef_ = 1.0f;
};

}  // namespace rasgo::modular
