#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>

// ============================================================================
// ControlSnapshot — barramento de controle por SNAPSHOT COERENTE
// ============================================================================
//
// Um conjunto de N valores escalares publicado de forma atômica e coerente:
// a thread que decide (análise, estratégia, plano generativo, GUI) escreve
// FORA do callback de áudio; a thread de áudio lê o snapshot INTEIRO sem lock,
// com número de tentativas limitado.
//
// Papel no Rasgo Modular:
//   - trocar o "plano compilado" do grafo na fronteira de bloco
//     (ver RASGO_DOCUMENTATION/architecture/ORQUESTRACAO_INSTRUMENTOS.md,
//     "o callback recebe somente um snapshot imutável e pré-alocado");
//   - modulação SAÍDA -> PARÂMETRO com coerência entre todos os campos de
//     uma decisão (nunca meia decisão de um lote, meia de outro).
//
// Proveniência (pesquisa, seção 35.5 de RASGO_MODULAR.md):
//   Adaptado do padrão `EnergyControlBus` de
//   `TRIOIO/src/core/TrioioBrain.{h,cpp}` — seqlock com retry limitado de 16,
//   autoria Lúcio de Araújo, código próprio do workspace RASGO. Ali os campos
//   são fixos e semânticos (energy, density, brightness, harmonicity...);
//   aqui a estrutura é generalizada para N slots, mantendo o mesmo contrato
//   de coerência e de segurança em tempo real. Registrado no inventário
//   global como derivação de `EnergyControlBus`.
//
// Conceito de fundo (Mutable Frames -> "memória de estados", Atlas seção 40):
//   um snapshot é um ESTADO inteiro do controle, não uma curva de parâmetro
//   isolada. Uma performance pode navegar entre estados coerentes.
//
// Contrato:
//   - UM único escritor (`publish`). Vários leitores.
//   - `read()` é seguro em tempo real: retry limitado, sem alocação, sem lock.
//     Se um `publish` ocupar toda a janela de retry, devolve o snapshot zero
//     (baseline conhecido) em vez de campos meio-copiados.
//   - valores não finitos entram como 0 (saneamento; a faixa/clamp por
//     parâmetro é responsabilidade de quem consome, ex.: Signal::setParameter).

namespace rasgo::modular {

template <std::size_t SlotCount>
class ControlSnapshot {
public:
    static constexpr std::size_t slots = SlotCount;

    struct Frame {
        std::array<float, SlotCount> value{};
        std::uint64_t revision = 0;
    };

    // Seqlock (variante portátil de Rigtorp: stores da sequência com
    // `release`, loads com `acquire`, e barreiras de thread entre a
    // sequência e os dados). Sequência ímpar = escrita em andamento; par
    // = estável. Sem as barreiras um leitor pode casar `seq0 == seq1`
    // (par) e ainda assim ler slots meio-sobrescritos.
    void publish(const Frame& frame) noexcept {
        const auto sequence = sequence_.load(std::memory_order_relaxed) + 1;
        sequence_.store(sequence, std::memory_order_release);  // -> ímpar
        std::atomic_thread_fence(std::memory_order_release);
        for (std::size_t index = 0; index < SlotCount; ++index)
            slot_[index].store(finite(frame.value[index]), std::memory_order_relaxed);
        revision_.store(frame.revision, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
        sequence_.store(sequence + 1, std::memory_order_release);  // -> par
    }

    // RT-safe: retry limitado, sem alocação, sem lock. Se um `publish`
    // ocupar toda a janela de retry, devolve o snapshot zero.
    [[nodiscard]] Frame read() const noexcept {
        for (int attempt = 0; attempt < 16; ++attempt) {
            const auto seq0 = sequence_.load(std::memory_order_acquire);
            if ((seq0 & 1u) != 0u)
                continue;
            std::atomic_thread_fence(std::memory_order_acquire);

            Frame frame;
            for (std::size_t index = 0; index < SlotCount; ++index)
                frame.value[index] = slot_[index].load(std::memory_order_relaxed);
            frame.revision = revision_.load(std::memory_order_relaxed);

            std::atomic_thread_fence(std::memory_order_acquire);
            const auto seq1 = sequence_.load(std::memory_order_acquire);
            if (seq0 == seq1)
                return frame;
        }
        return Frame{};
    }

private:
    static float finite(const float value) noexcept {
        return std::isfinite(value) ? value : 0.0f;
    }

    std::atomic<std::uint64_t> sequence_{0};
    std::atomic<std::uint64_t> revision_{0};
    std::array<std::atomic<float>, SlotCount> slot_{};
};

}  // namespace rasgo::modular
