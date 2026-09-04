#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

// WAV PCM 16 bits, sem dependência. Só pra render offline de auditoria -
// nunca no caminho de áudio.

namespace rasgo::modular {

inline bool writeWav16(const std::string& path, const std::vector<float>& interleaved,
                       const std::uint32_t sampleRate, const std::uint16_t channels) {
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

    for (const float sample : interleaved) {
        float clamped = sample;
        if (clamped > 1.0f) clamped = 1.0f;
        if (clamped < -1.0f) clamped = -1.0f;
        const auto pcm = static_cast<std::int16_t>(clamped * 32767.0f);
        put16(static_cast<std::uint16_t>(pcm));
    }

    std::fclose(file);
    return true;
}

}  // namespace rasgo::modular
