#pragma once

// Equilibra o VOLUME INICIAL entre seeds.
//
// ---- o achado que motivou isto -------------------------------------------
//
// Sessão de escuta do autor, 23 set. 2026, Estudo 1: "identifiquei seeds
// com sons bem mais fracos que outros, tive que aumentar o volume das
// caixas pra perceber que havia algum som". Medido depois em 40 seeds:
//
//     LUFS integrado:  mín −68,4 · mediana −26,1 · máx −15,6
//     FAIXA: 52,7 LU · 4 de 40 mais de 10 LU abaixo da mediana
//
// 52 LU é mais que a diferença entre um sussurro e um grito. Para um
// instrumento cuja identidade é SOAR AO ABRIR, um seed que abre quase
// inaudível é pior que um bug: parece que o instrumento não funciona.
//
// ---- por que isto NÃO é normalização automática -------------------------
//
// `SAIDA_AUDIO_COMUM.md §3` é explícito: "loudness informa a decisão, mas
// não deve normalizar uma performance ao vivo automaticamente". Esta
// função não normaliza performance nenhuma — ela roda **uma vez**, quando
// o patch nasce, e escolhe o ganho INICIAL do MASTER como qualquer outro
// valor inicial que o `seedPatch` já escolhe. Depois disso não toca em
// nada: não há detector, não há envelope, não há reação ao sinal. O
// músico mexe no GAIN e a função não interfere.
//
// É a diferença entre afinar o instrumento antes de tocar e ter um
// compressor escondido na saída.
//
// ---- por que o alvo é a MEDIANA, e não os −14 LUFS da publicação -------
//
// O MASTER abre em −24 dB por pedido explícito do autor (5 set. 2026), e
// isso dá uma mediana de ~−26 LUFS. Subir todos os seeds para os −14 LUFS
// do alvo de streaming reverteria aquela decisão por um efeito colateral.
// O objetivo aqui é **tirar a dispersão**, não mudar o nível que o autor
// escolheu — quem quiser mais volume tem o GAIN à mão.
//
// Framework-free: só o grafo e o medidor. Roda igual nos dois front-ends,
// e roda no `ctest`.

#include "core/SignalGraph.hpp"
#include "dsp/Loudness.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/SinkOut.hpp"
#include "panel/PatchSeed.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace rasgo::panel {

// Nível inicial de referência, em LUFS. É a mediana medida dos seeds com
// o MASTER no seu padrão de −24 dB: o alvo é fazer TODOS soarem como o
// seed mediano já soava, não mais alto que isso.
inline constexpr float kSeedTargetLufs = -26.0f;

// Teto do PICO depois da correção. É esta a régua que decide quanto se
// pode elevar, e não um número fixo de dB — a primeira versão usava
// +12 dB fixos e deixava a faixa em 43,5 LU, porque um seed medindo
// −68 LUFS precisa de +42 dB e o limite cego não deixava.
//
// A diferença entre "fraco" e "esparso" está no pico. Um seed
// uniformemente baixo tem pico baixo: elevar 40 dB é seguro e é o que
// resolve. Um seed esparso — silêncio com eventos raros — mede LUFS baixo
// mas tem pico ALTO, e elevá-lo pelo LUFS estouraria justamente nos
// eventos. Limitar pelo pico trata os dois casos com uma regra só, em vez
// de escolher entre atender um e estragar o outro.
//
// −6 dBFS deixa 6 dB de folga acima do maior pico medido: espaço para o
// material variar depois do trecho que foi medido, sem chegar ao teto de
// segurança da saída.
inline constexpr float kSeedPeakCeilingDb = -6.0f;

// Guarda final contra caso patológico (medição pegando um trecho mudo por
// acaso): mesmo com pico baixíssimo, não se eleva mais que isto.
inline constexpr float kSeedMaxLiftDb = 30.0f;
inline constexpr float kSeedMaxCutDb  =  9.0f;

// Quanto do patch medir antes de decidir o ganho — e a escolha é um
// compromisso MEDIDO, não um número redondo.
//
// A janela curta engana: medindo 1,5 s e tocando 20 s, um seed que começa
// quieto e cresce depois é elevado indevidamente, e o mais alto do lote
// saltava para −9,7 LUFS (acima de onde estava sem correção nenhuma).
// Janelas maiores corrigem isso, mas o pré-render custa tempo a cada
// SEED, e o protocolo de escuta pede apertar SEED dez vezes seguidas.
//
//     janela   dispersão   custo por SEED
//     1,5 s     36,0 LU        172 ms
//     4,0 s     33,8 LU        505 ms
//     8,0 s     31,7 LU       1088 ms
//
// 4 s é onde a curva vira: dobrar para 8 s compra 2,1 LU e cobra o dobro
// do tempo. Meio segundo num gesto que já reconstrói o grafo inteiro
// passa; um segundo inteiro, num botão feito para ser apertado em
// sequência, não.
//
// Melhoria possível, registrada e NÃO feita: medir em segundo plano
// enquanto o patch já toca, e aplicar pela rampa por amostra que o
// `gain` do MASTER já tem. Some com a espera, mas troca uma pausa
// previsível por um som que muda de nível depois de começar — e isso
// precisa ser ouvido antes de ser adotado.
inline constexpr double kSeedProbeSeconds = 4.0;

// Mede um seed e devolve a correção de ganho, em dB.
//
// Monta um rack PRÓPRIO, descartável, só para medir — e é por isso que
// tem esta forma. A primeira versão media rodando o grafo do músico e
// depois chamava `prepare()` para rebobiná-lo. O teste
// `test_seed_balance.cpp` provou que **não rebobina**: o primeiro bloco
// saía diferente do de um rack que nunca passou pela medição, ou seja, o
// patch começava adiantado em relação ao que o seed descreve, e dois
// racks com o mesmo seed soariam diferente conforme tivessem sido
// medidos. O comentário afirmava que rebobinava; o teste mostrou que era
// só a minha convicção.
//
// Medir num rack separado não tem esse problema por construção: o grafo
// do músico não é tocado, nem para ler. Custa montar um rack a mais —
// barato, e uma vez por seed.
inline float seedGainCorrection(const std::uint64_t seed,
                                const float sampleRate,
                                const std::size_t blockFrames,
                                const double seconds = kSeedProbeSeconds) {
    using namespace rasgo::modular;

    SignalGraph prova;
    for (const auto& grp : moduleCatalog())
        for (const char* t : grp.types)
            if (auto n = makeModule(t)) prova.add(std::move(n));
    // O sink do rack de prova precisa existir para o motor saber o que
    // processar. Usa a MESMA guarda de saída do instrumento, senão a
    // medição veria um sinal que o músico nunca ouviria.
    const std::size_t sink = prova.add(std::make_unique<SinkOut>());
    try { seedPatch(prova, seed); } catch (...) { return 0.0f; }
    prova.prepare(sampleRate, 2, blockFrames);
    prova.setActiveOutput(sink);

    std::size_t master = static_cast<std::size_t>(-1);
    for (std::size_t i = 0; i < prova.nodeCount(); ++i)
        if (prova.node(i).type() == "MASTER") { master = i; break; }
    if (master == static_cast<std::size_t>(-1)) return 0.0f;

    LoudnessMeter medidor;
    medidor.prepare(sampleRate);
    AudioBlock lixo(sampleRate, 2, blockFrames);
    const long blocos =
        static_cast<long>(seconds * sampleRate / static_cast<double>(blockFrames));
    float pico = 0.0f;
    for (long b = 0; b < blocos; ++b) {
        prova.process(lixo, sink, 0);
        for (std::size_t i = 0; i < blockFrames; ++i) {
            const float l = lixo.at(0, i), r = lixo.at(1, i);
            medidor.push(l, r);
            pico = std::max(pico, std::max(std::fabs(l), std::fabs(r)));
        }
    }

    const float medido = medidor.integrated();
    if (medido <= LoudnessMeter::kSilence || pico <= 1.0e-6f) return 0.0f;

    const float picoDb = 20.0f * std::log10(pico);
    float correcao = kSeedTargetLufs - medido;
    correcao = std::min(correcao, kSeedPeakCeilingDb - picoDb);
    correcao = std::min(correcao, kSeedMaxLiftDb);
    correcao = std::max(correcao, -kSeedMaxCutDb);
    return correcao;
}

// Aplica a correção, DISTRIBUÍDA pela folga que existe.
//
// Só o `gain` do MASTER não basta, e a medição mostrou por quê: um seed
// precisava de +42 dB e o parâmetro satura em +12. Mas a folga existia em
// outro lugar — o `out_gain` do MIXER vai de −24 a +12 dB e estava em 0.
//
// A ordem é do fim para o começo da cadeia: MASTER primeiro (é o controle
// de saída, o lugar natural), e o que não couber vai para o `out_gain` do
// MIXER. Não se mexe nos ganhos de CANAL do mixer: eles são a proporção
// entre as camadas, e alterá-los mudaria a mistura que o seed compôs, não
// só o volume dela.
//
// NÃO roda o grafo: a medição acontece no rack de prova de
// `seedGainCorrection`.
//
// CONTRATO: `seed` precisa ser o seed de que ESTE grafo nasceu. A medição
// é feita num rack de prova construído a partir dele, então passar outro
// número devolve uma correção sem relação com o que vai soar. É o preço
// de não tocar no grafo do músico, e é um preço barato — os dois
// front-ends chamam isto na linha seguinte ao `seedPatch`, com o mesmo
// seed. `test_seed_balance.cpp` fixa esse acoplamento por teste, para que
// quebrá-lo falhe no `ctest` e não numa sessão de escuta.
inline float balanceSeedLevel(rasgo::modular::SignalGraph& graph,
                              const std::uint64_t seed,
                              const float sampleRate,
                              const std::size_t blockFrames,
                              const double seconds = kSeedProbeSeconds) {
    float resta = seedGainCorrection(seed, sampleRate, blockFrames, seconds);
    if (resta == 0.0f) return 0.0f;
    const float pedido = resta;

    // Aplica o quanto o parâmetro aceitar e devolve o que sobrou. Sem
    // isto, pedir +42 num parâmetro que satura em +12 perde 30 dB em
    // silêncio — foi o caso que a medição de 40 seeds expôs.
    const auto aplicar = [&graph](const char* tipo, const char* par,
                                  float& falta) {
        if (falta == 0.0f) return;
        for (std::size_t i = 0; i < graph.nodeCount(); ++i) {
            if (graph.node(i).type() != tipo) continue;
            auto& nd = graph.node(i);
            for (const auto& p : nd.parameters()) {
                if (p.descriptor.id != par) continue;
                // `antes` é lido ANTES de mexer, e isto não é estilo: `p`
                // é referência para dentro do vetor de parâmetros, e
                // `setParameter` altera `p.value` no lugar. Lendo depois,
                // a diferença dava sempre zero — a função reportava "nada
                // aplicado" e o estágio seguinte recebia o pedido inteiro
                // outra vez. O teste pegou; a leitura do código, não.
                const float antes = p.value;
                const float lo = p.descriptor.minimum;
                const float hi = p.descriptor.maximum;
                const float alvo = antes + falta;
                const float posto = alvo > hi ? hi : (alvo < lo ? lo : alvo);
                nd.setParameter(par, posto);
                falta -= (posto - antes);
                return;
            }
            return;
        }
    };

    aplicar("MASTER", "gain", resta);
    aplicar("MIXER", "out_gain", resta);
    return pedido - resta;   // o que de fato foi aplicado
}

}  // namespace rasgo::panel
