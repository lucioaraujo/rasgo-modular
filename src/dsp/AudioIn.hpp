#pragma once

#include "core/SignalGraph.hpp"

#include <atomic>
#include <cstddef>
#include <vector>

// ============================================================================
// AUDIO-IN — entrada de áudio ao vivo (Módulo 35)
// ============================================================================
//
// O adaptador de entrada de áudio: "MIDI/audio/instrument coupling são nós
// adaptadores OPCIONAIS, nunca dependência" (identidade do RASGO Modular —
// o painel soa sozinho ao carregar, com ou sem este módulo). `AUDIO-IN`
// deixa o instrumento OUVIR — qualquer fonte externa (outro instrumento
// RASGO, uma entrada de linha, um microfone) rodando no mesmo sistema vira
// matéria-prima dentro do grafo, pra passar por `FILTER`, `SHAPE`,
// `MATRIX`, qualquer coisa.
//
// `rasgo_modular_core` é zero-dep — não sabe o que é ALSA. A CAPTURA de
// verdade vive em `apps/panel/AlsaSource.hpp` (contraparte de
// `AlsaSink.hpp`, mesmo padrão). Este módulo só é o lado RECEPTOR: um anel
// circular sem alocação em `process()`, alimentado por `pushSamples()` a
// partir de uma thread de captura externa (produtor único) e lido por
// `process()` no thread de áudio do grafo (consumidor único) — SPSC
// lock-free clássico, sem trava.
//
// Sem nenhuma captura alimentando (painel sem `AUDIO-IN` ligado a nada, ou
// rodando fora do painel — ex.: os testes/exemplos): o anel nunca recebe
// `pushSamples()`, então a saída fica em silêncio — nunca trava, nunca lê
// lixo. Determinístico nesse caso (sempre 0), o que deixa este módulo
// testável sem hardware de áudio nenhum.

namespace rasgo::modular {

class AudioIn final : public Signal {
public:
    AudioIn()
        : Signal({}, {{"out", PortKind::Audio, ""}},
                 {{"gain", 0.0f, 2.0f, 1.0f, ""}}) {}

    std::string type() const override { return "AUDIO-IN"; }

    Panel panel() const override {
        // módulo minúsculo — 1 knob, 1 jack. 4 HP basta (era 6, "muito
        // largo", pedido do autor 2026-09-05).
        // W = 20,32 mm; centro em 10,16. Knob (ø 9) centrado -> x 5,7;
        // jack (desenhado no próprio x) -> x 10,2.
        Panel p;
        p.hp = 4;
        p.add(Widget::Kind::Label, "AUDIO-IN", "", 2.0f, 2.0f);
        p.add(Widget::Kind::Display, "in", "", 2.5f, 6.0f, 15.3f);
        p.add(Widget::Kind::Knob, "GAIN", "gain", 5.7f, 46.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 10.2f, 104.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        // ~1 s de fôlego, potência de 2 (a máscara de anel exige) — absorve
        // uma pausa razoável do lado do consumidor (ex.: o thread de UI
        // segurando `gmx` por uma edição de patch) sem estourar.
        std::size_t cap = 1;
        while (cap < static_cast<std::size_t>(sampleRate) + 1) cap <<= 1;
        ring_.assign(cap * 2, 0.0f);  // intercalado L/R
        mask_ = cap - 1;
        writeIdx_.store(0, std::memory_order_relaxed);
        readIdx_ = 0;
    }

    // ---- lado PRODUTOR (thread de captura, `apps/panel/AlsaSource.hpp`) --
    // `interleavedLR` tem `frames*2` amostras em [-1,1] (L0,R0,L1,R1,...).
    // Nunca bloqueia; se o anel está cheio (consumidor muito atrasado),
    // sobrescreve o mais antigo — `process()` percebe e resincroniza (ver
    // abaixo). Chamado só de FORA do thread de áudio do grafo.
    void pushSamples(const float* interleavedLR, const std::size_t frames) noexcept {
        if (ring_.empty()) return;
        const std::size_t cap = mask_ + 1;
        std::size_t w = writeIdx_.load(std::memory_order_relaxed);
        const std::size_t n = frames > cap ? cap : frames;
        const std::size_t start = frames > cap ? frames - cap : 0;
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t slot = (w + i) & mask_;
            ring_[slot * 2] = interleavedLR[(start + i) * 2];
            ring_[slot * 2 + 1] = interleavedLR[(start + i) * 2 + 1];
        }
        writeIdx_.store(w + n, std::memory_order_release);
    }

    // ---- lado CONSUMIDOR (thread de áudio do grafo) ----------------------
    void process(const std::vector<const AudioBlock*>&,
                 std::vector<AudioBlock>& outputs) noexcept override {
        AudioBlock& out = outputs[0];
        const std::size_t frames = out.frames();
        const std::size_t channels = out.channels();
        const float gain = parameterValue("gain");

        if (ring_.empty()) {
            for (std::size_t f = 0; f < frames; ++f)
                for (std::size_t c = 0; c < channels; ++c) out.at(c, f) = 0.0f;
            return;
        }

        const std::size_t cap = mask_ + 1;
        const std::size_t w = writeIdx_.load(std::memory_order_acquire);
        // produtor pode ter sobrescrito dado não lido (consumidor ficou pra
        // trás mais que a capacidade inteira) -- resincroniza pro início da
        // janela ainda válida em vez de ler lixo já sobrescrito.
        if (w - readIdx_ > cap) readIdx_ = w - cap;
        const std::size_t available = w - readIdx_;
        const std::size_t n = available < frames ? available : frames;

        for (std::size_t f = 0; f < frames; ++f) {
            float l = 0.0f, r = 0.0f;
            if (f < n) {
                const std::size_t slot = (readIdx_ + f) & mask_;
                l = ring_[slot * 2] * gain;
                r = ring_[slot * 2 + 1] * gain;
            }
            // canal 0 = esquerdo, 1 = direito; canal >=2 (se algum dia
            // houver) repete o direito, mesma convenção do `MASTER`.
            for (std::size_t c = 0; c < channels; ++c)
                out.at(c, f) = c == 0 ? l : r;
        }
        readIdx_ += n;  // < frames em underrun -- o resto ficou em silêncio,
                        // sem travar; o próximo bloco pode alcançar.
    }

private:
    std::vector<float> ring_;         // intercalado L/R, tamanho = (mask_+1)*2
    std::size_t mask_ = 0;
    std::atomic<std::size_t> writeIdx_{0};  // só o produtor escreve
    std::size_t readIdx_ = 0;               // só o consumidor mexe
};

}  // namespace rasgo::modular
