#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

// WAV PCM 16 bits, sem dependência.
//
// `ditherSeed`: estudado de `NAVALHA2_JUCE/AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md`
// §3.7/P1.1 ("exportação sem dither" — arredondamento direto pra PCM16 sem
// TPDF, achado real, corrigido lá) — código do próprio autor, reescrito no
// idioma zero-dep deste projeto (desvio: sem PCM24, sem política de app; só
// o TPDF em si). `0` (padrão) = SEM dither — bypass explícito, preserva os
// renders de auditoria/goldens byte-idênticos entre execuções, exatamente
// como os "WAVs dourados" da NAVALHA pedem `none`. Qualquer outro valor liga
// TPDF de ±1 LSB determinístico por esse seed — usado pela gravação ao vivo
// do painel ([Ctrl+R]), nunca pelos renders de exemplo/CI.

namespace rasgo::modular {

namespace detail {
// TPDF = diferença de duas uniformes independentes (mesmo xorshift64* do
// resto do projeto) — triangular em [-1,+1] LSB, sem correlação entre
// amostras/canais consecutivos (L e R avançam o mesmo gerador, nunca
// compartilham o mesmo par).
inline float tpdfDitherLsb(std::uint64_t& s) noexcept {
    const auto next01 = [&]() -> float {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::uint32_t>(x >> 32))
            / 4294967296.0f;
    };
    return next01() - next01();
}
}  // namespace detail

inline bool writeWav16(const std::string& path, const std::vector<float>& interleaved,
                       const std::uint32_t sampleRate, const std::uint16_t channels,
                       const std::uint64_t ditherSeed = 0) {
    FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr)
        return false;

    const std::uint32_t frames =
        channels > 0 ? static_cast<std::uint32_t>(interleaved.size()) / channels : 0;
    const std::uint16_t bitsPerSample = 16;
    const std::uint16_t blockAlign = channels * bitsPerSample / 8;
    const std::uint32_t byteRate = sampleRate * blockAlign;
    const std::uint32_t dataBytes = frames * blockAlign;

    auto put32 = [&](const std::uint32_t v) { std::fwrite(&v, 4, 1, file); };
    auto put16 = [&](const std::uint16_t v) { std::fwrite(&v, 2, 1, file); };

    std::fwrite("RIFF", 1, 4, file);
    put32(36 + dataBytes);
    std::fwrite("WAVE", 1, 4, file);
    std::fwrite("fmt ", 1, 4, file);
    put32(16);
    put16(1);  // PCM
    put16(channels);
    put32(sampleRate);
    put32(byteRate);
    put16(blockAlign);
    put16(bitsPerSample);
    std::fwrite("data", 1, 4, file);
    put32(dataBytes);

    std::uint64_t rng = ditherSeed != 0 ? ditherSeed : 0x9E3779B97F4A7C15ULL;
    for (const float sample : interleaved) {
        // guarda de finitude: NaN/Inf nunca chegam no cast pra inteiro
        // (antes passava direto — as comparações de clamp abaixo são falsas
        // pra NaN, então um NaN cru virava lixo indefinido no cast)
        float clamped = std::isfinite(sample) ? sample : 0.0f;
        if (clamped > 1.0f) clamped = 1.0f;
        if (clamped < -1.0f) clamped = -1.0f;
        float scaled = clamped * 32767.0f;
        if (ditherSeed != 0) scaled += detail::tpdfDitherLsb(rng);
        if (scaled > 32767.0f) scaled = 32767.0f;
        if (scaled < -32768.0f) scaled = -32768.0f;
        const auto pcm = static_cast<std::int16_t>(std::lround(scaled));
        put16(static_cast<std::uint16_t>(pcm));
    }

    std::fclose(file);
    return true;
}

}  // namespace rasgo::modular
