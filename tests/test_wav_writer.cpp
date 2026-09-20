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
#include <cmath>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

// lê o cabeçalho (bits por amostra) e os samples int24 do "data" de um WAV
std::vector<std::int32_t> readPcm24(const std::string& path,
                                    std::uint16_t* bitsOut = nullptr) {
    std::ifstream f(path, std::ios::binary);
    std::vector<char> buf((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());
    std::size_t p = 12, dataOff = 0, dataLen = 0;
    while (p + 8 <= buf.size()) {
        char id[5] = {buf[p], buf[p+1], buf[p+2], buf[p+3], 0};
        std::uint32_t sz; std::memcpy(&sz, &buf[p+4], 4);
        if (std::string(id) == "fmt " && bitsOut != nullptr)
            std::memcpy(bitsOut, &buf[p + 8 + 14], 2);
        if (std::string(id) == "data") { dataOff = p + 8; dataLen = sz; break; }
        p += 8 + sz + (sz & 1);
    }
    std::vector<std::int32_t> out(dataLen / 3);
    for (std::size_t i = 0; i < out.size(); ++i) {
        const auto b0 = static_cast<std::uint8_t>(buf[dataOff + i*3]);
        const auto b1 = static_cast<std::uint8_t>(buf[dataOff + i*3 + 1]);
        const auto b2 = static_cast<std::uint8_t>(buf[dataOff + i*3 + 2]);
        std::int32_t v = static_cast<std::int32_t>(b0 | (b1 << 8) | (b2 << 16));
        if (v & 0x800000) v -= 0x1000000;          // sinal de 24 bits
        out[i] = v;
    }
    return out;
}

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


// ---- PCM 24 bits (a GRAVAÇÃO desde 21 set. 2026) --------------------

// O cabeçalho tem que ANUNCIAR 24 bits. Um WAV que diz 16 e carrega 24
// toca como ruído — é o tipo de erro que só aparece no reprodutor de
// outra pessoa.
void test24HeaderDeclaresDepth() {
    const std::string path = "/tmp/rasgo_wav24_header.wav";
    EXPECT(writeWav24(path, {0.0f, 0.0f, 0.5f, -0.5f}, 48000, 2));
    std::uint16_t bits = 0;
    const auto pcm = readPcm24(path, &bits);
    check(bits == 24, "o cabeçalho declara 24 bits");
    check(pcm.size() == 4, "quatro amostras escritas");
    std::remove(path.c_str());
}

// Little-endian explícito: escrito byte a byte justamente para não
// depender da ordem da máquina. Se alguém "simplificar" para um fwrite de
// int32 truncado, isto quebra.
void test24IsLittleEndianAndSigned() {
    const std::string path = "/tmp/rasgo_wav24_le.wav";
    EXPECT(writeWav24(path, {1.0f, -1.0f}, 48000, 1));
    const auto pcm = readPcm24(path);
    check(pcm.size() == 2, "duas amostras");
    check(pcm[0] == 8388607, "fundo de escala positivo satura em +2^23-1");
    check(pcm[1] == -8388607 || pcm[1] == -8388608,
          "fundo de escala negativo satura no mínimo");
    std::remove(path.c_str());
}

// A MESMA guarda de finitude do writeWav16: NaN e Inf não podem chegar ao
// cast pra inteiro, porque as comparações de clamp são falsas pra NaN e o
// resultado seria indefinido.
void test24FinitenessGuard() {
    const std::string path = "/tmp/rasgo_wav24_nan.wav";
    const float inf = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT(writeWav24(path, {nan, inf, -inf, 0.25f}, 48000, 1));
    const auto pcm = readPcm24(path);
    // Não-finito vira ZERO, e não fundo de escala — mesma regra do
    // `writeWav16`. Um Inf é defeito, não uma amostra alta: saturá-lo
    // gravaria o bug como estouro audível no arquivo do músico, enquanto
    // zerar deixa um furo silencioso que se percebe e se investiga.
    check(pcm[0] == 0, "NaN vira zero, não lixo");
    check(pcm[1] == 0, "+Inf vira zero (é defeito, não sample alto)");
    check(pcm[2] == 0, "-Inf vira zero");
    check(pcm[3] > 2000000, "a amostra boa ao lado passa intacta");
    std::remove(path.c_str());
}

// 24 bits resolve o que 16 não resolve. Um sinal baixo — e o MASTER abre
// em −24 dB de propósito, então este é o caso REAL — tem que sobreviver
// com resolução de sobra.
void test24ResolvesQuietMaterial() {
    const std::string p16 = "/tmp/rasgo_wav24_cmp16.wav";
    const std::string p24 = "/tmp/rasgo_wav24_cmp24.wav";
    // ~-90 dBFS: abaixo de 1 LSB de 16 bits, bem acima do de 24
    const float tiny = 3.0e-5f;
    EXPECT(writeWav16(p16, {tiny, tiny, tiny, tiny}, 48000, 1, 0));
    EXPECT(writeWav24(p24, {tiny, tiny, tiny, tiny}, 48000, 1));
    const auto a = readPcm16(p16);
    const auto b = readPcm24(p24);
    check(a[0] == 0 || a[0] == 1, "em 16 bits o sinal baixo quase some");
    check(b[0] > 200, "em 24 bits ele ainda tem resolução de sobra");
    std::remove(p16.c_str());
    std::remove(p24.c_str());
}

// Sem dither, e isso é deliberado (ver WavWriter.hpp): duas escritas do
// mesmo material têm que sair byte a byte idênticas.
void test24IsDeterministic() {
    const std::string a = "/tmp/rasgo_wav24_det_a.wav";
    const std::string b = "/tmp/rasgo_wav24_det_b.wav";
    std::vector<float> v;
    for (int i = 0; i < 480; ++i) v.push_back(0.3f * std::sin(i * 0.05f));
    EXPECT(writeWav24(a, v, 48000, 1));
    EXPECT(writeWav24(b, v, 48000, 1));
    check(readPcm24(a) == readPcm24(b), "24 bits é determinístico (sem dither)");
    std::remove(a.c_str());
    std::remove(b.c_str());
}

}  // namespace

int main() {
    testFinitenessGuard();
    testRoundingNotTruncation();
    testDitherBypassIsDeterministic();
    testDitherChangesOutputButIsDeterministicPerSeed();
    testStereoChannelsDoNotShareDitherNoise();
    test24HeaderDeclaresDepth();
    test24IsLittleEndianAndSigned();
    test24FinitenessGuard();
    test24ResolvesQuietMaterial();
    test24IsDeterministic();
    if (g_failures == 0) {
        std::cout << "RASGO Modular WAV writer tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
