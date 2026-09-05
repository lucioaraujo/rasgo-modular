// Teste isolado do `src/io/WavWriter.hpp` — guarda de finitude, arredonda-
// mento correto (não truncamento) e dither TPDF opcional. Ver
// `AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md` §3.7/P1.1 da NAVALHA 2 (achado
// real que motivou esta correção: exportação PCM sem dither).

#include "io/WavWriter.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

// lê de volta os samples int16 do "data" chunk de um WAV recém-escrito
std::vector<std::int16_t> readPcm16(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    std::vector<char> buf((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());
    std::size_t p = 12, dataOff = 0, dataLen = 0;
    while (p + 8 <= buf.size()) {
        char id[5] = {buf[p], buf[p+1], buf[p+2], buf[p+3], 0};
        std::uint32_t sz; std::memcpy(&sz, &buf[p+4], 4);
        if (std::string(id) == "data") { dataOff = p + 8; dataLen = sz; break; }
        p += 8 + sz + (sz & 1);
    }
    std::vector<std::int16_t> out(dataLen / 2);
    std::memcpy(out.data(), &buf[dataOff], dataLen);
    return out;
}

void testFinitenessGuard() {
    const std::vector<float> in = {
        std::numeric_limits<float>::quiet_NaN(), 0.5f,
        std::numeric_limits<float>::infinity(), -0.5f,
        -std::numeric_limits<float>::infinity(), 0.0f,
    };
    const std::string path = "/tmp/rasgo_wavwriter_test_finite.wav";
    EXPECT(writeWav16(path, in, 48000, 1));
    const auto pcm = readPcm16(path);
    check(pcm[0] == 0, "NaN vira 0, nao lixo indefinido");
    check(pcm[2] == 0, "+Inf vira 0 (clamp de finitude, nao clamp de faixa)");
    check(pcm[4] == 0, "-Inf vira 0");
    std::remove(path.c_str());
}

void testRoundingNotTruncation() {
    // 0,00002 * 32767 ~ 0,655 -- arredonda pra 1, trunca pra 0. Escolhido
    // pra cair claramente de um lado da fronteira de 0,5.
    const std::vector<float> in = {0.00002f, -0.00002f};
    const std::string path = "/tmp/rasgo_wavwriter_test_round.wav";
    EXPECT(writeWav16(path, in, 48000, 1));
    const auto pcm = readPcm16(path);
    check(pcm[0] == 1, "arredonda pra cima (nao trunca pra 0)");
    check(pcm[1] == -1, "arredonda simetricamente pro lado negativo");
    std::remove(path.c_str());
}

void testDitherBypassIsDeterministic() {
    const std::vector<float> in = {0.1f, 0.2f, -0.3f, 0.4f, -0.5f};
    const std::string a = "/tmp/rasgo_wavwriter_test_nodither_a.wav";
    const std::string b = "/tmp/rasgo_wavwriter_test_nodither_b.wav";
    EXPECT(writeWav16(a, in, 48000, 1));           // ditherSeed padrao = 0
    EXPECT(writeWav16(b, in, 48000, 1, 0));        // explicito
    check(readPcm16(a) == readPcm16(b), "seed=0 e omitido dao o mesmo byte a byte");
    std::remove(a.c_str());
    std::remove(b.c_str());
}

void testDitherChangesOutputButIsDeterministicPerSeed() {
    // silencio puro: sem dither, tudo 0; com dither TPDF, uma dispersao de
    // +-1 LSB aparece (e nao e tudo zero)
    const std::vector<float> silence(2000, 0.0f);
    const std::string nodither = "/tmp/rasgo_wavwriter_test_dither_silence0.wav";
    const std::string withDither1 = "/tmp/rasgo_wavwriter_test_dither_silence1.wav";
    const std::string withDither1b = "/tmp/rasgo_wavwriter_test_dither_silence1b.wav";
    const std::string withDither2 = "/tmp/rasgo_wavwriter_test_dither_silence2.wav";
    EXPECT(writeWav16(nodither, silence, 48000, 1, 0));
    EXPECT(writeWav16(withDither1, silence, 48000, 1, 42));
    EXPECT(writeWav16(withDither1b, silence, 48000, 1, 42));
    EXPECT(writeWav16(withDither2, silence, 48000, 1, 43));

    const auto p0 = readPcm16(nodither);
    bool allZero = true;
    for (const auto s : p0) if (s != 0) allZero = false;
    check(allZero, "sem dither, silencio digital continua exatamente 0");

    const auto p1 = readPcm16(withDither1);
    bool anyNonZero = false, withinOneLsb = true;
    for (const auto s : p1) { if (s != 0) anyNonZero = true; if (s < -1 || s > 1) withinOneLsb = false; }
    check(anyNonZero, "com dither, silencio ganha ruido de +-1 LSB (nao fica cravado em 0)");
    check(withinOneLsb, "o dither TPDF fica dentro de +-1 LSB, nao explode a amplitude");

    check(p1 == readPcm16(withDither1b), "mesma seed -> mesmo ruido de dither, byte a byte");
    check(p1 != readPcm16(withDither2), "seed diferente -> ruido de dither diferente");

    for (const auto& f : {nodither, withDither1, withDither1b, withDither2})
        std::remove(f.c_str());
}

void testStereoChannelsDoNotShareDitherNoise() {
    // silencio estereo: se L e R compartilhassem o mesmo par de uniformes
    // do TPDF, os dois canais sairiam sempre com o MESMO valor em cada
    // frame -- confere que isso nao acontece (alguma diferenca aparece).
    const std::vector<float> silence(4000, 0.0f);  // 2000 frames estereo
    const std::string path = "/tmp/rasgo_wavwriter_test_dither_stereo.wav";
    EXPECT(writeWav16(path, silence, 48000, 2, 7));
    const auto pcm = readPcm16(path);
    bool anyChannelDiff = false;
    for (std::size_t i = 0; i + 1 < pcm.size(); i += 2)
        if (pcm[i] != pcm[i + 1]) anyChannelDiff = true;
    check(anyChannelDiff, "L e R nao compartilham o mesmo ruido de dither por frame");
    std::remove(path.c_str());
}

}  // namespace

int main() {
    testFinitenessGuard();
    testRoundingNotTruncation();
    testDitherBypassIsDeterministic();
    testDitherChangesOutputButIsDeterministicPerSeed();
    testStereoChannelsDoNotShareDitherNoise();
    if (g_failures == 0) {
        std::cout << "RASGO Modular WAV writer tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
