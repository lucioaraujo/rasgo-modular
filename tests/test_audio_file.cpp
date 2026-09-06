// Round-trip da camada io/: `writeWav16` (WavWriter) → `loadAudioFile`
// (AudioFile + dr_wav). Confere que o que a gente grava, a gente lê de
// volta dentro da quantização de 16 bits.

#include "io/AudioFile.hpp"
#include "io/WavWriter.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

std::string tmpPath(const char* name) {
    const char* dir = std::getenv("CLAUDE_JOB_DIR");
    std::string base = dir ? (std::string(dir) + "/tmp/") : std::string("/tmp/");
    return base + "rasgo_af_" + name + ".wav";
}

void testRoundTripMono() {
    const std::string p = tmpPath("mono");
    std::vector<float> src(4000);
    for (std::size_t i = 0; i < src.size(); ++i)
        src[i] = 0.6f * std::sin(2.0 * M_PI * 220.0 * (double)i / 44100.0);
    EXPECT(writeWav16(p, src, 44100, 1));

    const AudioFile f = loadAudioFile(p);
    EXPECT(f.valid());
    EXPECT(f.channels == 1);
    EXPECT(f.sampleRate == 44100);
    EXPECT(f.frames() == src.size());
    double maxErr = 0.0;
    for (std::size_t i = 0; i < src.size() && i < f.samples.size(); ++i)
        maxErr = std::max(maxErr, (double)std::fabs(src[i] - f.samples[i]));
    EXPECT(maxErr < 2.0 / 32767.0);   // ± 1 LSB de 16 bits
    std::remove(p.c_str());
}

void testRoundTripStereo() {
    const std::string p = tmpPath("stereo");
    std::vector<float> il(2000 * 2);
    for (std::size_t i = 0; i < 2000; ++i) {
        il[i * 2] = 0.5f * std::sin(0.05 * (double)i);       // L
        il[i * 2 + 1] = -0.3f * std::sin(0.08 * (double)i);  // R
    }
    EXPECT(writeWav16(p, il, 48000, 2));

    const AudioFile f = loadAudioFile(p);
    EXPECT(f.valid() && f.channels == 2 && f.sampleRate == 48000);
    EXPECT(f.frames() == 2000);

    const std::vector<float> m = toMono(f);
    EXPECT(m.size() == 2000);
    // mono = (L + R) / 2
    double err = 0.0;
    for (std::size_t i = 0; i < 2000; ++i) {
        const float want = 0.5f * ((float)(0.5 * std::sin(0.05 * (double)i))
                                   + (float)(-0.3 * std::sin(0.08 * (double)i)));
        err = std::max(err, (double)std::fabs(want - m[i]));
    }
    EXPECT(err < 3.0 / 32767.0);
    std::remove(p.c_str());
}

void testMissingFile() {
    const AudioFile f = loadAudioFile("/nao/existe/mesmo_12345.wav");
    EXPECT(!f.valid());
    EXPECT(f.channels == 0 && f.samples.empty());
    EXPECT(toMono(f).empty());
}

}  // namespace

int main() {
    testRoundTripMono();
    testRoundTripStereo();
    testMissingFile();

    if (g_failures == 0) {
        std::cout << "RASGO Modular audio file tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
