#pragma once

#include "core/SignalGraph.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ============================================================================
// Patch Genetics — MUTATE / EVOLVE / FREEZE (protótipo)
// ============================================================================
//
// Ver o estudo: `RASGO_MODULAR/dossies/ESTUDO_seed_composicao_generativa.md`
// §4. `PatchSeed.hpp` só sabe GERAR um patch do zero (`seedPatch`); isto
// aqui sabe EDITAR um patch que já existe, preservando a topologia — a
// operação `MUTATE` da conversa de origem.
//
// Reaproveita a mesma ideia do passe genérico de `seedPatch()` (§2.2 do
// estudo: pra cada nó alcançado por cabo, ~fração dos parâmetros
// não-estruturais reamostrados na faixa TOTAL), só que como função
// independente, chamável a qualquer momento sobre o grafo como ele está
// AGORA — não redesenha cabo nenhum, não muda topologia.
//
// Escreve por `SignalGraph::setParameterBase()` (motor aditivo,
// 2026-09-04): se o parâmetro tem modulação por cabo (`connectToParameter`)
// já plugada, a mutação move a BASE por baixo dela — a modulação
// continua somando por cima, não é apagada.
//
// `rasgo_modular_core` continua sem saber o que é "mutação de patch" —
// isto vive em `apps/panel/`, como `PatchSeed.hpp` e `MotionEngine.hpp`.
//
// FREEZE: não é um estado guardado no motor — é só o conjunto de nós que
// quem chama passa em `frozen`. O chamador (painel, teste, uma peça)
// decide o que proteger; `mutatePatch`/`evolvePatch` só respeitam.
//
// CROSS (2026-09-05) — a peça que era "a mais arriscada da lista" (ver
// estudo §4) porque dois grafos de seeds diferentes quase nunca têm os
// mesmos nós nas mesmas posições. Alinhamento adotado: por TIPO de
// módulo, não por índice — a k-ésima ocorrência de um tipo no ALVO casa
// com a (k mod contagem-no-doador)-ésima ocorrência do MESMO tipo no
// DOADOR (um `FILTER` só no doador ainda cruza com os 3 `FILTER` do
// alvo, se for o caso). Um nó do alvo cujo tipo não existe no doador
// fica INTOCADO — CROSS nunca caça um substituto de outro tipo. Mesma
// simplificação de MUTATE: só parâmetro, nunca cabo/topologia — é
// crossover de TIMBRE/CARÁTER, não de forma.
//
// LIMITAÇÃO CONHECIDA (medida, não hipotética): ao contrário do
// `seedPatch()`, MUTATE não protege uma "espinha" — reamostra QUALQUER
// parâmetro não-estrutural de um nó tocado, sem noção de "isto é crítico
// pro som sair". Sonda: 30 seeds, 5 MUTATE em sequência (fração 0,3
// cada) sobre um patch já semeado, SEM FREEZE → **4/30 ficaram mudos**
// (algum parâmetro de nível/mix caiu perto de 0 por acaso, várias vezes
// seguidas). Não é bug — é o preço de não ter uma espinha em tempo de
// mutação (decisão de design, ver o estudo).
//
// Mitigação IMPLEMENTADA (2026-09-04): `[m]`/`[e]` no painel
// (`panel_main.cpp`) congelam `MIXER`/`MASTER` automaticamente antes de
// chamar `mutatePatch`/`evolvePatch` — a recomendação acima, sem exigir
// nada do usuário. Sonda com o freeze aplicado: **0/30 mudos** (30
// seeds, MUTATE fração 0,25 e EVOLVE 6×0,12, cada um medido
// separadamente). `fraction` pequena continua uma opção adicional, não
// mais a única defesa.

namespace rasgo::panel {

// parâmetros que MUTATE nunca toca — quebram a peça (duração/escala) ou
// emudecem fácil. Mesma lista do passe genérico do `PatchSeed.hpp`.
inline bool isMutationBlocked(const std::string& id) noexcept {
    static const std::unordered_set<std::string> blocked = {
        "bpm", "mult", "length", "scale", "root", "mode", "sub_2",
        "sync_enable", "dc_block", "limit", "voices", "freeze", "mono",
        "gain", "output", "out_gain", "fm_amount",
        // ganhos de nível — mutar isto joga o barramento pra fora e o
        // MASTER limita demais / distorce (o "clipe" reportado 2026-09-05)
        "gain1", "gain2", "gain3", "gain4", "level1", "level2",
    };
    return blocked.count(id) != 0;
}

namespace detail {
// xorshift64* — mesmo padrão usado nos módulos DSP e no PatchSeed
inline std::uint64_t mutNext(std::uint64_t& s) noexcept {
    s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
    return s * 0x2545F4914F6CDD1DULL;
}
inline float mutF01(std::uint64_t& s) noexcept {
    return static_cast<float>(static_cast<std::uint32_t>(mutNext(s) >> 32))
        / 4294967296.0f;
}
}  // namespace detail

// MUTATE: reamostra ~`fraction` dos parâmetros não-estruturais dos nós
// alcançados por cabo (exceto os em `frozen`), dentro da faixa total de
// cada parâmetro. `seed` determinístico — mesmo seed, mesmo grafo,
// mesma mutação.
inline void mutatePatch(rasgo::modular::SignalGraph& graph,
                        std::uint64_t seed, const float fraction,
                        const std::unordered_set<std::size_t>& frozen = {}) {
    std::uint64_t s = seed != 0 ? seed : 0x9E3779B97F4A7C15ULL;
    const std::size_t N = graph.nodeCount();

    std::vector<char> touched(N, 0);
    for (std::size_t i = 0; i < graph.cableCount(); ++i) {
        touched[graph.cable(i).source().node] = 1;
        touched[graph.cable(i).target().node] = 1;
    }

    for (std::size_t n = 0; n < N; ++n) {
        if (!touched[n] || frozen.count(n)) continue;
        auto& node = graph.node(n);
        for (const auto& pr : node.parameters()) {
            const auto& dsc = pr.descriptor;
            if (isMutationBlocked(dsc.id)) continue;
            if (detail::mutF01(s) > fraction) continue;

            const float lo = dsc.minimum, hi = dsc.maximum;
            float v;
            if (lo == 0.0f && hi == 1.0f
                && (dsc.id.find("enable") != std::string::npos
                    || dsc.id == "hold" || dsc.id == "accent_mode"))
                v = detail::mutF01(s) < 0.5f ? 1.0f : 0.0f;
            else
                v = lo + detail::mutF01(s) * (hi - lo);

            graph.setParameterBase(n, dsc.id, v);
        }
    }
}

// EVOLVE: várias MUTATE pequenas em sequência — o mesmo destino de uma
// MUTATE grande, percorrido em passos menores (cada passo com uma
// derivação própria do seed, então é reproduzível passo a passo).
inline void evolvePatch(rasgo::modular::SignalGraph& graph,
                        std::uint64_t seed, const int steps,
                        const float fractionPerStep,
                        const std::unordered_set<std::size_t>& frozen = {}) {
    for (int i = 0; i < steps; ++i)
        mutatePatch(graph,
                   seed + static_cast<std::uint64_t>(i) * 0x9E3779B97F4A7C15ULL,
                   fractionPerStep, frozen);
}

// CROSS: recombina valores de parâmetro do ALVO (editado no lugar) com
// os do DOADOR (só lido) — casando por TIPO de módulo (ver comentário
// do topo do arquivo). `fraction`=0,5 é o crossover "clássico" (cada
// parâmetro elegível tem ~metade de chance de virar o valor do doador);
// `fraction`=1 substitui TODO parâmetro elegível dos nós casados pelo
// valor do doador. Nunca mexe em cabo/topologia. `seed` determinístico.
// `donor` não é `const` só porque `SignalGraph::node()` não tem
// sobrecarga const (motor); esta função nunca escreve nele.
inline void crossPatch(rasgo::modular::SignalGraph& target,
                       rasgo::modular::SignalGraph& donor,
                       std::uint64_t seed, const float fraction,
                       const std::unordered_set<std::size_t>& frozen = {}) {
    std::uint64_t s = seed != 0 ? seed : 0x9E3779B97F4A7C15ULL;
    const std::size_t N = target.nodeCount();

    std::vector<char> touched(N, 0);
    for (std::size_t i = 0; i < target.cableCount(); ++i) {
        touched[target.cable(i).source().node] = 1;
        touched[target.cable(i).target().node] = 1;
    }

    // ocorrências de cada tipo no doador, em ordem de índice
    std::unordered_map<std::string, std::vector<std::size_t>> donorByType;
    for (std::size_t i = 0; i < donor.nodeCount(); ++i)
        donorByType[donor.node(i).type()].push_back(i);

    // conta, tipo a tipo, qual ocorrência do ALVO estamos vendo — pra
    // casar a k-ésima ocorrência de um tipo no alvo com a mesma
    // ocorrência (módulo o tamanho) no doador
    std::unordered_map<std::string, std::size_t> seenCount;

    for (std::size_t n = 0; n < N; ++n) {
        const std::string ty = target.node(n).type();
        const std::size_t occurrence = seenCount[ty]++;
        if (!touched[n] || frozen.count(n)) continue;

        const auto it = donorByType.find(ty);
        if (it == donorByType.end() || it->second.empty()) continue;
        const std::size_t donorNode = it->second[occurrence % it->second.size()];

        for (const auto& pr : target.node(n).parameters()) {
            const auto& dsc = pr.descriptor;
            if (isMutationBlocked(dsc.id)) continue;
            if (detail::mutF01(s) > fraction) continue;
            const float v = donor.parameterUserValue(donorNode, dsc.id);
            target.setParameterBase(n, dsc.id, v);
        }
    }
}

}  // namespace rasgo::panel
