// O REC do Rasgo Modular no navegador (25/11/2026, F4c): os arquivos da
// tomada feitos com o MESMO código do app, para saírem iguais byte a byte:
//   - o WAV estéreo de 24 bits, com a conversão de `writeWav24`
//     (src/io/WavWriter.hpp: limita a ±1, ×8388607, arredonda, sem dither);
//   - a versão de entrega a -14 LUFS / <= -1 dBTP (`DeliveryMaster`,
//     src/saida/, como `writeStreamingVersion` em apps/juce/StreamingVersion.h),
//     com o mesmo .txt de medidas.
// Roda no motor da PÁGINA (não no worklet que toca): a página copia a tomada
// para `rw_rec_buffer`, pede os arquivos e os baixa. Nada sai do navegador.

#include "saida/DeliveryMaster.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
std::vector<float> g_tomada;     // a tomada, intercalada L R L R…
std::string g_arquivo, g_texto;

void put32(std::string& o, std::uint32_t v) {
    for (int k = 0; k < 4; ++k) o += static_cast<char>((v >> (8 * k)) & 0xFF);
}
void put16(std::string& o, std::uint16_t v) {
    o += static_cast<char>(v & 0xFF);
    o += static_cast<char>((v >> 8) & 0xFF);
}

// `writeWav24`, em memória
void wav24(std::string& o, const std::vector<float>& v, std::uint32_t taxa) {
    const std::uint16_t canais = 2, bits = 24, bloco = canais * bits / 8;
    const std::uint32_t quadros = static_cast<std::uint32_t>(v.size()) / canais;
    const std::uint32_t dados = quadros * bloco;
    o.clear();
    o.reserve(44 + dados);
    o += "RIFF"; put32(o, 36 + dados); o += "WAVE";
    o += "fmt "; put32(o, 16); put16(o, 1); put16(o, canais); put32(o, taxa);
    put32(o, taxa * bloco); put16(o, bloco); put16(o, bits);
    o += "data"; put32(o, dados);
    for (std::size_t i = 0; i < static_cast<std::size_t>(quadros) * canais; ++i) {
        float c = std::isfinite(v[i]) ? v[i] : 0.0f;
        if (c > 1.0f) c = 1.0f;
        if (c < -1.0f) c = -1.0f;
        double s = static_cast<double>(c) * 8388607.0;
        if (s > 8388607.0) s = 8388607.0;
        if (s < -8388608.0) s = -8388608.0;
        const auto u = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::llround(s)));
        o += static_cast<char>(u & 0xFF);
        o += static_cast<char>((u >> 8) & 0xFF);
        o += static_cast<char>((u >> 16) & 0xFF);
    }
}
}  // namespace

extern "C" {

// espaço para a tomada: `floats` números (L R intercalados)
__attribute__((used, visibility("default")))
float* rw_rec_buffer(int floats) {
    g_tomada.assign(static_cast<std::size_t>(std::max(0, floats)), 0.0f);
    return g_tomada.data();
}

// o WAV da tomada; tamanho em rw_rec_bytes()
__attribute__((used, visibility("default")))
const char* rw_rec_wav(int taxa) {
    wav24(g_arquivo, g_tomada, static_cast<std::uint32_t>(taxa));
    return g_arquivo.data();
}

__attribute__((used, visibility("default")))
int rw_rec_bytes() { return static_cast<int>(g_arquivo.size()); }

// o último arquivo montado (o WAV da tomada ou o de entrega)
__attribute__((used, visibility("default")))
const char* rw_rec_arquivo() { return g_arquivo.data(); }

// a versão de entrega (<nome>.streaming.wav) e o texto das medidas
// (rw_rec_texto); `nome` = o arquivo da tomada, para o texto. 0 se falhou.
__attribute__((used, visibility("default")))
int rw_rec_entrega(int taxa, const char* nome) {
    const std::size_t n = g_tomada.size() / 2;
    std::vector<float> l(n), r(n), ol, orr;
    for (std::size_t i = 0; i < n; ++i) { l[i] = g_tomada[2 * i]; r[i] = g_tomada[2 * i + 1]; }
    const auto rep = rasgo::modular::saida::DeliveryMaster::process(l, r, static_cast<double>(taxa), ol, orr);
    if (rep.failed || ol.empty()) return 0;
    std::vector<float> saida(ol.size() * 2);
    for (std::size_t i = 0; i < ol.size(); ++i) { saida[2 * i] = ol[i]; saida[2 * i + 1] = orr[i]; }
    wav24(g_arquivo, saida, static_cast<std::uint32_t>(taxa));
    char b[2048];
    std::snprintf(b, sizeof b,
        "Rasgo Modular - versao de entrega (streaming), saida-ref 1.2\n"
        "tomada: %s (post-safety)\n"
        "alvo: -14 LUFS, true peak <= -1 dBTP; limitador no maximo 6 dB (guarda de dinamica)\n"
        "entrada: %g LUFS, %g dBTP\n"
        "ganho: %g dB; limitador: %g dB\n"
        "saida: %g LUFS, %g dBTP\n%s",
        nome ? nome : "", static_cast<double>(rep.inputLufs), static_cast<double>(rep.inputTruePeakDb),
        static_cast<double>(rep.gainDb), static_cast<double>(rep.limiterReductionDb),
        static_cast<double>(rep.outputLufs), static_cast<double>(rep.outputTruePeakDb),
        rep.silent ? "silencio: copiada como veio\n"
        : rep.reachedTarget ? "alvo atingido\n"
        : "alvo nao atingido sem esmagar a dinamica: entregue no mais alto possivel (os servicos sobem faixas baixas no modo normal)\n");
    g_texto = b;
    return 1;
}

__attribute__((used, visibility("default")))
const char* rw_rec_texto() { return g_texto.c_str(); }

// devolve a memória da tomada e dos arquivos
__attribute__((used, visibility("default")))
void rw_rec_liberar() {
    std::vector<float>().swap(g_tomada);
    std::string().swap(g_arquivo);
}

}  // extern "C"
