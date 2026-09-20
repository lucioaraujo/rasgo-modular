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

// WAV PCM 24 bits — o formato da GRAVAÇÃO desde 21 set. 2026.
//
// Por que trocar. A tomada do músico não é o arquivo final: ela vai ser
// ouvida, comparada com o tap `pre-safety`, talvez editada e masterizada
// depois. Entregar isso em 16 bits joga fora resolução que não volta, e o
// caso é justamente o que dói — o MASTER abre em −24 dB de propósito, ou
// seja, o material costuma estar longe do fundo de escala, e é aí que os
// bits que faltam aparecem como ruído.
//
// **Sem dither, e isto é deliberado.** Em 16 bits o TPDF é necessário: o
// degrau de quantização fica acima do ruído do próprio material, e sem
// dither o erro vira distorção correlacionada com o sinal. Em 24 bits o
// degrau está ~48 dB abaixo disso — somar dither aqui só acrescentaria
// ruído audível-em-tese sem corrigir defeito nenhum. `writeWav16`
// continua existindo com o TPDF para quem precise de 16 bits.
//
// Little-endian explícito byte a byte: o formato WAV é LE por definição, e
// escrever um `int32` truncado assumiria a ordem da máquina — quebraria
// silenciosamente em big-endian, que é o pior modo de quebrar.
inline bool writeWav24(const std::string& path, const std::vector<float>& interleaved,
                       const std::uint32_t sampleRate, const std::uint16_t channels) {
    FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr)
        return false;

    const std::uint32_t frames =
        channels > 0 ? static_cast<std::uint32_t>(interleaved.size()) / channels : 0;
    const std::uint16_t bitsPerSample = 24;
    const std::uint16_t blockAlign = channels * bitsPerSample / 8;
    const std::uint32_t byteRate = sampleRate * blockAlign;
    const std::uint32_t dataBytes = frames * blockAlign;

    auto put32 = [&](const std::uint32_t v) {
        const unsigned char b[4] = {
            static_cast<unsigned char>(v & 0xFF),
            static_cast<unsigned char>((v >> 8) & 0xFF),
            static_cast<unsigned char>((v >> 16) & 0xFF),
            static_cast<unsigned char>((v >> 24) & 0xFF)};
        std::fwrite(b, 1, 4, file);
    };
    auto put16 = [&](const std::uint16_t v) {
        const unsigned char b[2] = {
            static_cast<unsigned char>(v & 0xFF),
            static_cast<unsigned char>((v >> 8) & 0xFF)};
        std::fwrite(b, 1, 2, file);
    };

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

    for (const float sample : interleaved) {
        // mesma guarda de finitude do writeWav16: NaN/Inf nunca chegam ao
        // cast pra inteiro, porque as comparações de clamp são falsas pra
        // NaN e o cast seria indefinido
        float clamped = std::isfinite(sample) ? sample : 0.0f;
        if (clamped > 1.0f) clamped = 1.0f;
        if (clamped < -1.0f) clamped = -1.0f;
        double scaled = static_cast<double>(clamped) * 8388607.0;
        if (scaled >  8388607.0) scaled =  8388607.0;
        if (scaled < -8388608.0) scaled = -8388608.0;
        const auto pcm = static_cast<std::int32_t>(std::llround(scaled));
        const auto u = static_cast<std::uint32_t>(pcm);
        const unsigned char b[3] = {
            static_cast<unsigned char>(u & 0xFF),
            static_cast<unsigned char>((u >> 8) & 0xFF),
            static_cast<unsigned char>((u >> 16) & 0xFF)};
        std::fwrite(b, 1, 3, file);
    }

    std::fclose(file);
    return true;
}

}  // namespace rasgo::modular
