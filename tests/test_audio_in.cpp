// Teste isolado do `AUDIO-IN` (src/dsp/AudioIn.hpp) — o anel circular
// SPSC entre `pushSamples()` (produtor, thread de captura) e `process()`
// (consumidor, thread de áudio do grafo). Testado single-threaded aqui
// (chamando os dois lados na ordem certa) — a segurança entre threads de
// verdade vem de `std::atomic` com acquire/release, não é o que este teste
// prova; o que este teste prova é a LÓGICA do anel (índices, resync,
// silêncio em underrun).

#include "dsp/AudioIn.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace rasgo::modular;

namespace {

int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
#define EXPECT(x) check((x), #x)

constexpr float kSr = 48000.0f;

void testSilentWithoutFeed() {
    AudioIn a;
    a.prepare(kSr, 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    for (int b = 0; b < 20; ++b)
        a.process({}, out);
    for (std::size_t i = 0; i < 64; ++i) {
        check(out[0].at(0, i) == 0.0f, "sem pushSamples, saida fica em silencio (L)");
        check(out[0].at(1, i) == 0.0f, "sem pushSamples, saida fica em silencio (R)");
    }
}

void testRoundTrip() {
    AudioIn a;
    a.prepare(kSr, 64);
    std::vector<float> feed(64 * 2);
    for (std::size_t i = 0; i < 64; ++i) {
        feed[i * 2] = static_cast<float>(i) / 64.0f;          // L: rampa 0..1
        feed[i * 2 + 1] = -static_cast<float>(i) / 64.0f;     // R: rampa 0..-1
    }
    a.pushSamples(feed.data(), 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    a.process({}, out);
    bool same = true;
    for (std::size_t i = 0; i < 64; ++i) {
        if (out[0].at(0, i) != feed[i * 2]) same = false;
        if (out[0].at(1, i) != feed[i * 2 + 1]) same = false;
    }
    check(same, "o que entra por pushSamples sai identico em process (gain=1 default)");
}

void testGainApplied() {
    AudioIn a;
    a.prepare(kSr, 64);
    a.setParameter("gain", 0.5f);
    std::vector<float> feed(64 * 2, 0.0f);
    for (std::size_t i = 0; i < 64; ++i) feed[i * 2] = 0.8f;  // L constante
    a.pushSamples(feed.data(), 64);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    a.process({}, out);
    check(std::fabs(out[0].at(0, 0) - 0.4f) < 1.0e-6f, "gain 0.5 aplicado (0,8 -> 0,4)");
}

void testUnderrunStaysSilentNotGarbage() {
    // empurra só 30 amostras, mas pede um bloco de 64 -- as 34 que faltam
    // devem ficar em 0, nao lixo, e nao travar.
    AudioIn a;
    a.prepare(kSr, 64);
    std::vector<float> feed(30 * 2, 0.0f);
    for (std::size_t i = 0; i < 30; ++i) feed[i * 2] = 1.0f;
    a.pushSamples(feed.data(), 30);
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    a.process({}, out);
    bool first30ok = true, rest0 = true;
    for (std::size_t i = 0; i < 30; ++i) if (out[0].at(0, i) != 1.0f) first30ok = false;
    for (std::size_t i = 30; i < 64; ++i) if (out[0].at(0, i) != 0.0f) rest0 = false;
    check(first30ok, "underrun: as amostras que chegaram saem certas");
    check(rest0, "underrun: o resto do bloco fica em silencio, nao lixo");

    // bloco seguinte: mais 40 chegam -- as 4 que faltavam do bloco anterior
    // ficaram pra tras (perdidas, esperado -- e' captura ao vivo, nao fila
    // garantida), mas o consumidor continua andando sem travar
    std::vector<float> feed2(40 * 2, 0.0f);
    for (std::size_t i = 0; i < 40; ++i) feed2[i * 2] = 0.5f;
    a.pushSamples(feed2.data(), 40);
    a.process({}, out);
    EXPECT(std::isfinite(out[0].at(0, 0)));
}

void testOverflowResyncsInsteadOfReadingStale() {
    // empurra em MUITOS pedacos pequenos, sem nunca chamar process() no
    // meio -- assim writeIdx_ acumula alem da capacidade entre chamadas
    // (o clip interno de um pushSamples() gigante sozinho nao alcanca
    // esse caminho; precisa de varias chamadas reais, como a captura ao
    // vivo faria). A capacidade e' a proxima potencia de 2 >= 48001 =
    // 65536; empurra 3x isso em blocos de 1000 pra estourar de verdade.
    AudioIn a;
    a.prepare(kSr, 64);
    const std::size_t total = 65536 * 3;
    std::vector<float> chunk(1000 * 2);
    std::size_t pushed = 0;
    while (pushed < total) {
        const std::size_t n = std::min<std::size_t>(1000, total - pushed);
        for (std::size_t i = 0; i < n; ++i) {
            chunk[i * 2] = static_cast<float>(pushed + i);  // valor crescente identificavel
            chunk[i * 2 + 1] = 0.0f;
        }
        a.pushSamples(chunk.data(), n);
        pushed += n;
    }
    std::vector<AudioBlock> out(1, AudioBlock(kSr, 2, 64));
    a.process({}, out);
    bool allFinite = true;
    for (std::size_t i = 0; i < 64; ++i)
        if (!std::isfinite(out[0].at(0, i))) allFinite = false;
    EXPECT(allFinite);
    // o valor lido deve vir de dentro da JANELA MAIS RECENTE ainda valida
    // (as ultimas ~65536 amostras empurradas), nunca de antes dela (dado
    // ha muito sobrescrito por voltas anteriores do anel)
    const float lo = static_cast<float>(total) - 65536.0f;
    check(out[0].at(0, 0) >= lo - 1.0f,
          "apos estouro por acumulo, resincroniza pra janela mais recente, nao le dado sobrescrito");
    check(out[0].at(0, 0) < static_cast<float>(total),
          "o valor lido nao passa do que foi realmente empurrado");
}

void testDeterminism() {
    AudioIn a, b;
    a.prepare(kSr, 64);
    b.prepare(kSr, 64);
    std::vector<float> feed(64 * 2);
    for (std::size_t i = 0; i < feed.size(); ++i) feed[i] = (i % 7) * 0.1f - 0.3f;
    a.pushSamples(feed.data(), 64);
    b.pushSamples(feed.data(), 64);
    std::vector<AudioBlock> oa(1, AudioBlock(kSr, 2, 64)), ob(1, AudioBlock(kSr, 2, 64));
    a.process({}, oa);
    b.process({}, ob);
    bool same = true;
    for (std::size_t i = 0; i < 64; ++i)
        if (oa[0].at(0, i) != ob[0].at(0, i) || oa[0].at(1, i) != ob[0].at(1, i))
            same = false;
    EXPECT(same);
}

}  // namespace

int main() {
    testSilentWithoutFeed();
    testRoundTrip();
    testGainApplied();
    testUnderrunStaysSilentNotGarbage();
    testOverflowResyncsInsteadOfReadingStale();
    testDeterminism();
    if (g_failures == 0) {
        std::cout << "RASGO Modular AUDIO-IN tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
