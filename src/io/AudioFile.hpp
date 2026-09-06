#pragma once

// ============================================================================
// AudioFile — carga de áudio de disco (camada io/, FORA do core)
// ============================================================================
//
// O `rasgo_modular_core` continua SEM dependência e fazendo som sozinho.
// Isto vive em `io/` (como o `WavWriter`) e só é linkado por quem precisa
// (painel, testes). Usa `third_party/dr_wav` (domínio público / MIT-0 —
// ver `third_party/dr_wav/PROVENANCE.md`).
//
// Decisão em `dossies/ESTUDO_audio_sampling.md §3`. Carregar/decodificar é
// I/O + alloc → **nunca de `process()`**; é gesto de UI, e o buffer é
// trocado por ponteiro/handoff, não construído no callback.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rasgo::modular {

struct AudioFile {
    std::vector<float> samples;   // intercalado (channels)
    unsigned channels = 0;
    unsigned sampleRate = 0;

    [[nodiscard]] std::size_t frames() const noexcept {
        return channels ? samples.size() / channels : 0;
    }
    [[nodiscard]] bool valid() const noexcept {
        return channels > 0 && !samples.empty();
    }
};

// Carrega WAV (PCM 8/16/24/32-bit ou float) via dr_wav. `{}` em erro.
AudioFile loadAudioFile(const std::string& path);

// Soma os canais pra mono (÷ nº de canais). `{}` se o arquivo é inválido.
std::vector<float> toMono(const AudioFile& file);

}  // namespace rasgo::modular
