#pragma once

#include "core/SignalGraph.hpp"

#include <cstdint>

// ============================================================================
// NOTE-OUT — detector de nota, gate+pitch → evento (Módulo 38)
// ============================================================================
//
// O adaptador que deixa o RASGO Score registrar uma "nota" (`MUSICAL
// SCORE`, ver `dossies/ESTUDO_seed_composicao_generativa.md §5`) sem
// mudar o contrato de NENHUM dos outros 37 módulos. Em vez de fazer
// `ENVELOPE`/`SEQUENCE`/etc. "anunciarem" eventos de forma genérica
// (mudaria a interface deles — a razão original de `CROSS` do `NOTE`
// ficar em aberto), este módulo é OBSERVADOR: você cabeia GATE e PITCH
// nele (os mesmos cabos que já existem no patch, o grafo digital deixa
// fan-out livre) e ele detecta borda de nota-liga/nota-desliga.
//
// `rasgo_modular_core` continua sem saber o que é "partitura" — este
// módulo só EXPÕE a nota completa mais recente por `takeCompletedNote()`
// (chamado de fora de `process()`, nunca dentro); quem grava de verdade
// no `ScoreRecorder` é `apps/panel/panel_main.cpp` (que já inclui
// `ScoreRecorder.hpp`), lendo essa saída a cada bloco.
//
// **Precisa ser cabeado EM LINHA** pra rodar: o motor só processa nós
// que chegam ao `sink` ativo (`setActiveOutput`, "órfãos não custam DSP
// por bloco") — por isso `gate`/`pitch` saem de novo em `gate_thru`/
// `pitch_thru` (cópia exata, sem mudar o sinal), pra encadear
// `ALGO.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate`
// sem perder o caminho original. Mesma exigência de qualquer utilidade
// deste catálogo que precisa estar no caminho de verdade (`CONTROL`,
// `MULT`, etc.), não uma limitação nova.
//
// **v1 é MONOFÔNICO** — 1 `NOTE-OUT` capta 1 voz por vez. Polifonia
// (várias notas ao mesmo tempo, tipo o `CHORD`) fica de fora de
// propósito, documentado, não escondida.
//
// Determinístico (sem RNG). Sem alocação em `process()`.

namespace rasgo::modular {

class NoteOut final : public Signal {
public:
    struct CompletedNote {
        float pitch = 0.0f;         // 1V/oct CRU — conversão pra nota/MIDI
                                     // fica pra hora de exportar, não aqui
        float velocity = 1.0f;      // 0..1 (1.0 se `velocity` não conectada)
        double durationSeconds = 0.0;
        bool accent = false;
    };

    NoteOut()
        : Signal(
              {{"gate", PortKind::Control, "trig"},
               {"pitch", PortKind::Control, "v/oct"},
               {"velocity", PortKind::Control, ""},
               {"accent", PortKind::Control, ""}},
              {{"gate_thru", PortKind::Control, "trig"},
               {"pitch_thru", PortKind::Control, "v/oct"}},
              {}) {}

    std::string type() const override { return "NOTE-OUT"; }

    Panel panel() const override {
        // coordenadas em mm; painel 3U (128,5 mm) x hp*5,08 mm
        // adaptador (sem knobs) — 8 -> 7 HP no passe de ergonomia
        // 2026-09-06; "PITCH" (5 letras) agora com respiro; display cheio.
        Panel p;
        p.hp = 7;
        p.add(Widget::Kind::Label, "NOTE-OUT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "note", "", 2.5f, 6.0f, 30.6f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 8.0f, 62.0f);
        p.add(Widget::Kind::Jack, "PITCH", "in:pitch", 22.0f, 62.0f);
        p.add(Widget::Kind::Jack, "VEL", "in:velocity", 8.0f, 82.0f);
        p.add(Widget::Kind::Jack, "ACC", "in:accent", 22.0f, 82.0f);
        p.add(Widget::Kind::Jack, "GTHR", "out:gate_thru", 8.0f, 106.0f);
        p.add(Widget::Kind::Jack, "PTHR", "out:pitch_thru", 22.0f, 106.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        prevGate_ = 0.0f;
        noteActive_ = false;
        elapsed_ = 0;
        onSample_ = 0;
        onPitch_ = 0.0f;
        onVelocity_ = 1.0f;
        onAccent_ = false;
        pendingReady_ = false;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& gateThru = outputs[0];
        AudioBlock& pitchThru = outputs[1];
        const std::size_t frames = gateThru.frames();
        const std::size_t channels = gateThru.channels();

        const AudioBlock* gateIn = inputs[0];
        const AudioBlock* pitchIn = inputs[1];
        const AudioBlock* velIn = inputs[2];
        const AudioBlock* accIn = inputs[3];

        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float gate = gateIn ? gateIn->at(0, frame) : 0.0f;
            const float pitch = pitchIn ? pitchIn->at(0, frame) : 0.0f;

            if (prevGate_ < 0.5f && gate >= 0.5f) {
                // nota-liga
                onSample_ = elapsed_;
                onPitch_ = pitch;
                onVelocity_ = velIn ? clampf(velIn->at(0, frame), 0.0f, 1.0f) : 1.0f;
                onAccent_ = accIn ? accIn->at(0, frame) >= 0.5f : false;
                noteActive_ = true;
            } else if (prevGate_ >= 0.5f && gate < 0.5f && noteActive_) {
                // nota-desliga -- fecha a nota completa
                pending_.pitch = onPitch_;
                pending_.velocity = onVelocity_;
                pending_.durationSeconds =
                    static_cast<double>(elapsed_ - onSample_) / sampleRate_;
                pending_.accent = onAccent_;
                pendingReady_ = true;
                noteActive_ = false;
            }
            prevGate_ = gate;
            ++elapsed_;

            for (std::size_t c = 0; c < channels; ++c) {
                gateThru.at(c, frame) = gate;
                pitchThru.at(c, frame) = pitch;
            }
        }
    }

    // chamado de FORA de `process()` (thread de áudio, entre blocos —
    // nunca dentro do laço de amostra) — devolve a nota completa mais
    // recente e limpa o estado; `false` se não há nenhuma nova.
    bool takeCompletedNote(CompletedNote& out) noexcept {
        if (!pendingReady_) return false;
        out = pending_;
        pendingReady_ = false;
        return true;
    }

private:
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    float prevGate_ = 0.0f;
    bool noteActive_ = false;
    std::uint64_t elapsed_ = 0;
    std::uint64_t onSample_ = 0;
    float onPitch_ = 0.0f;
    float onVelocity_ = 1.0f;
    bool onAccent_ = false;

    CompletedNote pending_;
    bool pendingReady_ = false;
};

}  // namespace rasgo::modular
