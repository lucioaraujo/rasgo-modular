// Guarda de segurança do sink (`apps/panel/SinkOut.hpp`).
//
// O MASTER traz a proteção de saída de excelência, mas é um módulo como
// qualquer outro: o músico pode cabear direto no OUT e passar por fora
// dele. Este teste fixa o contrato da última barreira antes do
// dispositivo — `SAIDA_AUDIO_COMUM.md §3`: nenhum valor inválido pode
// atingir a saída, e existe um teto.
//
// Fixa também a NÃO-atuação: no caminho normal (sinal abaixo do teto) a
// guarda tem que ser transparente amostra a amostra. Uma guarda que
// colore o caminho normal seria pior que guarda nenhuma, porque mudaria o
// som de todo patch bem construído.

#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

using rasgo::modular::AudioBlock;
using rasgo::panel::SinkOut;

namespace {

int failures = 0;

void check(const bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FALHOU: %s\n", what); ++failures; }
}

constexpr float kSr = 48000.0f;

// roda um bloco pelo sink e devolve a saída
AudioBlock run(SinkOut& out, const AudioBlock& in) {
    std::vector<const AudioBlock*> ins{&in};
    std::vector<AudioBlock> outs(1, AudioBlock(kSr, 2, 64));
    out.process(ins, outs);
    return outs[0];
}

void testNonFiniteNeverReachesTheDevice() {
    SinkOut sink;
    AudioBlock in(kSr, 2, 64);
    in.at(0, 0) = std::numeric_limits<float>::quiet_NaN();
    in.at(0, 1) = std::numeric_limits<float>::infinity();
    in.at(1, 2) = -std::numeric_limits<float>::infinity();
    in.at(1, 3) = 0.25f;                       // vizinho válido, não pode sumir

    const AudioBlock o = run(sink, in);

    bool allFinite = true;
    for (std::size_t c = 0; c < o.channels(); ++c)
        for (std::size_t i = 0; i < o.frames(); ++i)
            if (!std::isfinite(o.at(c, i))) allFinite = false;

    check(allFinite, "nenhuma amostra não-finita chega à saída");
    check(o.at(0, 0) == 0.0f, "NaN vira silêncio, não lixo");
    check(o.at(0, 1) == 0.0f, "+Inf vira silêncio");
    check(o.at(1, 2) == 0.0f, "-Inf vira silêncio");
    check(o.at(1, 3) == 0.25f, "a amostra válida ao lado passa intacta");
    check(sink.nonFiniteCount() == 3, "telemetria conta os três consertos");
}

void testCeilingHolds() {
    SinkOut sink;
    AudioBlock in(kSr, 2, 64);
    for (std::size_t i = 0; i < in.frames(); ++i) {
        in.at(0, i) =  12.0f;     // muito acima do teto
        in.at(1, i) = -37.5f;
    }
    const AudioBlock o = run(sink, in);

    bool inRange = true;
    for (std::size_t c = 0; c < o.channels(); ++c)
        for (std::size_t i = 0; i < o.frames(); ++i)
            if (std::fabs(o.at(c, i)) > SinkOut::kCeiling + 1e-6f) inRange = false;

    check(inRange, "nada passa do teto de -1 dBFS");
    check(o.at(0, 0) ==  SinkOut::kCeiling, "positivo recorta no teto");
    check(o.at(1, 0) == -SinkOut::kCeiling, "negativo recorta no teto");
    check(sink.clippedCount() > 0, "telemetria registra o recorte");
}

// O teste que importa tanto quanto os outros: no caminho normal a guarda
// NÃO pode tocar em nada.
void testTransparentBelowCeiling() {
    SinkOut sink;
    AudioBlock in(kSr, 2, 64);
    for (std::size_t i = 0; i < in.frames(); ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(in.frames());
        in.at(0, i) = 0.88f * std::sin(6.2831853f * 3.0f * t);   // logo abaixo
        in.at(1, i) = -0.5f + t;
    }
    const AudioBlock o = run(sink, in);

    bool identical = true;
    for (std::size_t c = 0; c < o.channels(); ++c)
        for (std::size_t i = 0; i < o.frames(); ++i)
            if (o.at(c, i) != in.at(c, i)) identical = false;

    check(identical, "abaixo do teto a saída é IDÊNTICA amostra a amostra");
    check(sink.nonFiniteCount() == 0 && sink.clippedCount() == 0,
          "e a telemetria fica zerada — a guarda não atuou");
}

void testSilenceWithoutInput() {
    SinkOut sink;
    std::vector<const AudioBlock*> ins{nullptr};
    std::vector<AudioBlock> outs(1, AudioBlock(kSr, 2, 64));
    outs[0].at(0, 0) = 0.9f;               // lixo de um bloco anterior
    sink.process(ins, outs);
    check(outs[0].at(0, 0) == 0.0f, "sem entrada, o sink silencia");
}

}  // namespace

int main() {
    testNonFiniteNeverReachesTheDevice();
    testCeilingHolds();
    testTransparentBelowCeiling();
    testSilenceWithoutInput();
    if (failures == 0) std::puts("test_sink_guard: OK");
    return failures == 0 ? 0 : 1;
}
