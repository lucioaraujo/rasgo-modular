// A seed é a identidade do patch — invariantes (9/11/2026).
//
// Origem: o autor clicou várias vezes em SEED e "percebi que o som estava
// alto". A medição do patch salvo mostrou −14 LUFS, 12 LU acima do alvo:
// o `out_gain` do MIXER, posto pelo equilíbrio de volume, nunca voltava a
// 0 e subia de clique em clique. A investigação achou o problema maior: a
// seed só redefinia o que ela mesma sorteava, e as modulações da DRIFT se
// somavam às da seed anterior. Em 40 cliques seguidos, 39 seeds saíam
// diferentes da mesma seed num rack novo.
//
// Este arquivo é o mecanismo para que isso não se repita:
//   1. a mesma seed dá o MESMO patch num rack novo e num rack usado
//      (parâmetros, cabos e modulações — a serialização inteira);
//   2. vale também depois de o músico mexer em knobs e ligar modulações;
//   3. o volume de saída não carrega de uma seed para a outra, e as
//      escolhas do músico que nenhuma seed sorteia ficam (MUTE do MASTER,
//      ganho e monitor da entrada);
//   4. todo módulo do instrumento faz parte do rack da seed (decisão do
//      autor: "preciso que todos os módulos façam parte das escolhas da
//      seed") — um módulo novo fora do sorteio faz este teste falhar.

#include "core/SignalGraph.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <cstdint>
#include <cstdio>
#include <set>
#include <string>

using namespace rasgo::modular;

namespace {

int falhas = 0;
void check(const bool ok, const char* o_que) {
    if (!ok) { std::printf("  FALHOU: %s\n", o_que); ++falhas; }
}

SignalGraph rack() {
    SignalGraph g;
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g.add(std::move(n));
    g.add(std::make_unique<rasgo::panel::SinkOut>());
    return g;
}

std::uint64_t seedN(int i) { return static_cast<std::uint64_t>(i) * 0x9E3779B97F4A7C15ULL; }

// 1. cliques seguidos: cada seed igual à mesma seed num rack novo
void testCliquesSeguidos() {
    SignalGraph usado = rack();
    int diferentes = 0;
    for (int i = 1; i <= 30; ++i) {
        rasgo::panel::seedPatch(usado, seedN(i));
        SignalGraph novo = rack();
        rasgo::panel::seedPatch(novo, seedN(i));
        if (novo.serialize() != usado.serialize()) ++diferentes;
    }
    if (diferentes) std::printf("  %d de 30 seeds diferentes num rack usado\n", diferentes);
    check(diferentes == 0, "a mesma seed dá o mesmo patch depois de outras seeds");
}

// 2. depois do músico: knobs no extremo e modulações à mão
void testDepoisDoMusico() {
    SignalGraph usado = rack();
    rasgo::panel::seedPatch(usado, seedN(3));
    for (std::size_t n = 0; n < usado.nodeCount(); ++n)
        for (const auto& p : usado.node(n).parameters()) {
            const std::string id = p.descriptor.id;
            // escolhas do músico que a seed preserva (testadas à parte)
            if (usado.node(n).type() == "MASTER" && id == "mute") continue;
            if (usado.node(n).type() == "SIGNAL-IN" && (id == "gain" || id == "listen")) continue;
            usado.node(n).setParameter(id, p.descriptor.maximum);
        }
    for (std::size_t n = 0; n + 1 < usado.nodeCount(); ++n)
        if (usado.node(n).outputCount() > 0 && !usado.node(n + 1).parameters().empty())
            usado.connectToParameter(n, 0, n + 1,
                                     usado.node(n + 1).parameters().front().descriptor.id,
                                     0.5f, 0.0f, true);
    const std::uint64_t s = seedN(7);
    rasgo::panel::seedPatch(usado, s);
    SignalGraph novo = rack();
    rasgo::panel::seedPatch(novo, s);
    check(novo.serialize() == usado.serialize(),
          "knobs e modulações do patch anterior não passam para a seed nova");
}

// 3. o volume de saída não carrega (o caso do autor)
void testVolumeNaoCarrega() {
    const float sr = 48000.0f;
    const std::size_t bloco = 256;
    SignalGraph usado = rack();
    for (std::size_t n = 0; n < usado.nodeCount(); ++n)
        if (usado.node(n).type() == "MIXER") usado.node(n).setParameter("out_gain", 12.0f);
    const std::uint64_t s = 12345;
    rasgo::panel::seedPatch(usado, s);
    rasgo::panel::balanceSeedLevel(usado, s, sr, bloco);
    SignalGraph novo = rack();
    rasgo::panel::seedPatch(novo, s);
    rasgo::panel::balanceSeedLevel(novo, s, sr, bloco);
    for (std::size_t n = 0; n < novo.nodeCount(); ++n) {
        const auto t = novo.node(n).type();
        if (t == "MIXER")
            check(usado.node(n).parameterValue("out_gain") == novo.node(n).parameterValue("out_gain"),
                  "o out_gain do MIXER não carrega de uma seed para a outra");
        if (t == "MASTER")
            check(usado.node(n).parameterValue("gain") == novo.node(n).parameterValue("gain"),
                  "o gain do MASTER não carrega de uma seed para a outra");
    }
}

// 3b. escolhas do músico que nenhuma seed sorteia: o MUTE do MASTER
// (proteção dos ouvidos) e o ganho/monitor da entrada de áudio
void testEscolhasDoMusico() {
    SignalGraph g = rack();
    for (std::size_t n = 0; n < g.nodeCount(); ++n) {
        if (g.node(n).type() == "MASTER") g.node(n).setParameter("mute", 1.0f);
        if (g.node(n).type() == "SIGNAL-IN") {
            g.node(n).setParameter("gain", 1.5f);
            g.node(n).setParameter("listen", 1.0f);
        }
    }
    rasgo::panel::seedPatch(g, seedN(5));
    for (std::size_t n = 0; n < g.nodeCount(); ++n) {
        if (g.node(n).type() == "MASTER")
            check(g.node(n).parameterValue("mute") >= 0.5f, "a seed não desliga o MUTE do músico");
        if (g.node(n).type() == "SIGNAL-IN") {
            check(g.node(n).parameterValue("gain") == 1.5f, "a seed preserva o ganho da entrada");
            check(g.node(n).parameterValue("listen") >= 0.5f, "a seed preserva o monitor da entrada");
        }
    }
}

// 4. todo módulo está no rack da seed
void testTodoModuloNaSeed() {
    std::set<std::string> naSeed;
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types) naSeed.insert(t);
    for (const auto& f : rasgo::panel::moduleCatalog())
        for (const char* t : f.types) {
            if (!naSeed.count(t)) std::printf("  fora da seed: %s\n", t);
            check(naSeed.count(t) == 1, "todo módulo da biblioteca está no rack da seed");
        }
    // e todo tipo que a fábrica constrói está no catálogo (exceto o sink)
    std::size_t construidos = 0;
    for (const auto& t : naSeed) if (rasgo::panel::makeModule(t)) ++construidos;
    check(construidos == naSeed.size(), "todo módulo do rack da seed é construído pela fábrica");
}

}  // namespace

int main() {
    testCliquesSeguidos();
    testDepoisDoMusico();
    testVolumeNaoCarrega();
    testEscolhasDoMusico();
    testTodoModuloNaSeed();
    if (falhas == 0) std::printf("seed identity: todos os testes passaram\n");
    return falhas == 0 ? 0 : 1;
}
