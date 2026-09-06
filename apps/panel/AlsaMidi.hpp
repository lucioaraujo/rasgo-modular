#pragma once

// Entrada MIDI mínima via ALSA **sequencer** — contraparte MIDI do
// `AlsaSource` (que é PCM). NÃO faz parte do `rasgo_modular_core`.
// Cria uma porta virtual de ENTRADA "RASGO Modular : IN" que o usuário
// conecta a um teclado/controlador por `aconnect` ou um patchbay gráfico
// (qpwgraph, Helvum…). Alimenta o `SIGNAL-IN` (`src/dsp/SignalIn.hpp`)
// por `pushMidi(status, d1, d2)`, chamado de uma thread de polling
// dedicada em `panel_main.cpp` — nunca do thread de áudio do grafo
// (`SignalIn::pushMidi` é lock-free por dentro, SPSC).
//
// Sem nada conectado à porta: os eventos simplesmente não chegam — a
// thread de polling roda um `poll()` que não faz nada. Nunca trava.

#include <alsa/asoundlib.h>

#include <cstdint>
#include <stdexcept>

namespace rasgo::panel {

class AlsaMidi {
public:
    AlsaMidi() {
        if (snd_seq_open(&seq_, "default", SND_SEQ_OPEN_INPUT,
                         SND_SEQ_NONBLOCK) < 0)
            throw std::runtime_error("ALSA seq: não abriu a entrada MIDI");
        snd_seq_set_client_name(seq_, "RASGO Modular");
        port_ = snd_seq_create_simple_port(
            seq_, "IN",
            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
        if (port_ < 0) {
            snd_seq_close(seq_);
            seq_ = nullptr;
            throw std::runtime_error("ALSA seq: não criou a porta de entrada");
        }
    }

    ~AlsaMidi() {
        if (seq_ != nullptr) snd_seq_close(seq_);
    }

    AlsaMidi(const AlsaMidi&) = delete;
    AlsaMidi& operator=(const AlsaMidi&) = delete;

    // Não bloqueia (porta em NONBLOCK). Chama `fn(status, d1, d2)` — bytes
    // MIDI de canal (só o nibble alto de `status` importa pro `SignalIn`)
    // — pra cada evento pendente.
    template <class Fn>
    void poll(Fn&& fn) {
        if (seq_ == nullptr) return;
        snd_seq_event_t* ev = nullptr;
        while (snd_seq_event_input(seq_, &ev) >= 0 && ev != nullptr) {
            const std::uint8_t ch =
                static_cast<std::uint8_t>(ev->data.note.channel & 0x0F);
            switch (ev->type) {
            case SND_SEQ_EVENT_NOTEON:
                fn(static_cast<std::uint8_t>(0x90 | ch),
                   static_cast<std::uint8_t>(ev->data.note.note),
                   static_cast<std::uint8_t>(ev->data.note.velocity));
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                fn(static_cast<std::uint8_t>(0x80 | ch),
                   static_cast<std::uint8_t>(ev->data.note.note),
                   static_cast<std::uint8_t>(ev->data.note.velocity));
                break;
            case SND_SEQ_EVENT_KEYPRESS:  // aftertouch polifônico → como CC
                break;
            case SND_SEQ_EVENT_CONTROLLER: {
                const std::uint8_t cc =
                    static_cast<std::uint8_t>(ev->data.control.channel & 0x0F);
                fn(static_cast<std::uint8_t>(0xB0 | cc),
                   static_cast<std::uint8_t>(ev->data.control.param & 0x7F),
                   static_cast<std::uint8_t>(ev->data.control.value & 0x7F));
                break;
            }
            case SND_SEQ_EVENT_PITCHBEND: {
                const std::uint8_t pc =
                    static_cast<std::uint8_t>(ev->data.control.channel & 0x0F);
                const int v14 = ev->data.control.value + 8192;   // −8192..8191 → 0..16383
                fn(static_cast<std::uint8_t>(0xE0 | pc),
                   static_cast<std::uint8_t>(v14 & 0x7F),
                   static_cast<std::uint8_t>((v14 >> 7) & 0x7F));
                break;
            }
            default:
                break;
            }
        }
    }

private:
    snd_seq_t* seq_ = nullptr;
    int port_ = -1;
};

}  // namespace rasgo::panel
