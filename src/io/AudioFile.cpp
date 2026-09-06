// Implementação de `AudioFile.hpp` — o ÚNICO TU que compila o dr_wav.
// (single-header: `DR_WAV_IMPLEMENTATION` só pode aparecer uma vez.)

#include "io/AudioFile.hpp"

// dr_wav mexe com string.h/stdio.h e tem seus próprios warnings; isola.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#define DR_WAV_IMPLEMENTATION
#include "dr_wav/dr_wav.h"

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace rasgo::modular {

AudioFile loadAudioFile(const std::string& path) {
    AudioFile out;
    unsigned int ch = 0;
    unsigned int sr = 0;
    drwav_uint64 frameCount = 0;
    float* data = drwav_open_file_and_read_pcm_frames_f32(
        path.c_str(), &ch, &sr, &frameCount, nullptr);
    if (data == nullptr || ch == 0 || frameCount == 0) {
        if (data != nullptr) drwav_free(data, nullptr);
        return out;
    }
    out.channels = ch;
    out.sampleRate = sr;
    out.samples.assign(data, data + frameCount * ch);
    drwav_free(data, nullptr);
    return out;
}

std::vector<float> toMono(const AudioFile& file) {
    std::vector<float> mono;
    if (!file.valid()) return mono;
    const std::size_t fr = file.frames();
    mono.resize(fr);
    const float inv = 1.0f / static_cast<float>(file.channels);
    for (std::size_t i = 0; i < fr; ++i) {
        float acc = 0.0f;
        for (unsigned c = 0; c < file.channels; ++c)
            acc += file.samples[i * file.channels + c];
        mono[i] = acc * inv;
    }
    return mono;
}

}  // namespace rasgo::modular
