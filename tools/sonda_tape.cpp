// Sonda (diagnóstico, 23/11/2026): o chiado do TAPE dá RMS 0,0216 no macOS
// e 0,0051 no Linux. Mede o caminho do chiado em partes, sem JUCE.
#include "dsp/Tape.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>
using namespace rasgo::modular;

static double rmsTape(float hiss, float drive, int& naoZero) {
    Tape m;
    m.setParameter("drive", drive); m.setParameter("speed", 1.0f); m.setParameter("wow", 0.0f);
    m.setParameter("flutter", 0.0f); m.setParameter("hiss", hiss); m.setParameter("wear", 0.0f);
    m.prepare(48000.0f, 256);
    AudioBlock in(48000.0f, 1, 256);
    std::vector<AudioBlock> out(2, AudioBlock(48000.0f, 1, 256));
    std::vector<const AudioBlock*> ins{&in, nullptr, nullptr};
    double e = 0; std::size_t n = 0; naoZero = 0;
    for (int b = 0; b < 187; ++b) {
        for (std::size_t i = 0; i < 256; ++i) in.at(0, i) = 0.0f;
        m.process(ins, out);
        for (std::size_t i = 0; i < 256; ++i) {
            const float v = out[0].at(0, i);
            if (v != 0.0f) ++naoZero;
            if (b * 256 + int(i) >= 4800) { e += double(v) * v; ++n; }
        }
    }
    return std::sqrt(e / double(n));
}

int main() {
    std::uint64_t r = 0x7A9E0FF1CEULL;
    auto unit = [&] { r ^= r >> 12; r ^= r << 25; r ^= r >> 27;
                      return static_cast<float>((r * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f; };
    std::printf("primeiros unit():");
    for (int i = 0; i < 6; ++i) std::printf(" %.6f", unit());
    std::printf("\n");
    // o filtro do chiado fora do módulo (um unit por amostra, como se só ele existisse)
    r = 0x7A9E0FF1CEULL; float lp = 0, hp = 0; double e = 0;
    for (int i = 0; i < 48000; ++i) { const float n = 2.0f * unit() - 1.0f; lp += (n - lp) * 0.35f; hp += (lp - hp) * 0.02f; if (i >= 4800) e += double(lp - hp) * (lp - hp); }
    std::printf("filtro do chiado isolado: RMS %.5f (x0,02 = %.5f)\n", std::sqrt(e / 43200.0), 0.02 * std::sqrt(e / 43200.0));
    int nz = 0;
    std::printf("TAPE HISS 0 DRIVE 0,4: RMS %.6f, amostras nao zero %d\n", rmsTape(0.0f, 0.4f, nz), nz);
    std::printf("TAPE HISS 0 DRIVE 0: RMS %.6f, amostras nao zero %d\n", rmsTape(0.0f, 0.0f, nz), nz);
    std::printf("TAPE HISS 1 DRIVE 0,4: RMS %.6f\n", rmsTape(1.0f, 0.4f, nz));
    std::printf("TAPE HISS 1 DRIVE 0: RMS %.6f\n", rmsTape(1.0f, 0.0f, nz));
    std::printf("tanh(0.85f)=%.9f  tanh(0)=%.9f\n", std::tanh(0.85f), std::tanh(0.0f));
    return 0;
}
