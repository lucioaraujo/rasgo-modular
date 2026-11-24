// O Rasgo Modular no navegador (WebAssembly), primeiro passo: uma seed
// tocando dentro de um AudioWorklet (24/11/2026; dossies/
// ESTUDO_outras_plataformas.md, item 2). O mesmo motor do app e da linha de
// comando: o rack de fábrica, a seed (`seedPatch`) e o equilíbrio de volume
// (`balanceSeedLevel`), em blocos de 128 amostras, o quantum do Web Audio.
//
// Compilado com o Emscripten como WebAssembly autônomo (sem JavaScript de
// cola): o worklet chama estas funções direto.
//
//   rw_iniciar(taxa)        prepara o motor na taxa do navegador
//   rw_seed(alto, baixo)    monta a seed (64 bits em duas metades)
//   rw_processar()          um bloco de 128; devolve o endereço das
//                           amostras intercaladas (L, R, L, R…)
//   rw_cabos()              quantos cabos a seed ligou

#include "core/SignalGraph.hpp"
#include "escalas/BancoEscalas.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdint>
#include <memory>

using rasgo::modular::AudioBlock;
using rasgo::modular::Signal;
using rasgo::modular::SignalGraph;

namespace {

constexpr std::size_t kQuantum = 128;
float g_taxa = 48000.0f;
std::unique_ptr<SignalGraph> g_grafo;
std::size_t g_saida = 0;
std::unique_ptr<AudioBlock> g_bloco;
float g_pcm[kQuantum * 2] = {};

std::unique_ptr<Signal> fabrica(const std::string& t) {
    if (t == "OUT") return std::make_unique<rasgo::panel::SinkOut>();
    return rasgo::panel::makeModule(t);
}

}  // namespace

extern "C" {

__attribute__((used, visibility("default")))
void rw_iniciar(float taxa) {
    rasgo::escalas::BancoEscalas::ligarAoQuantizer();
    g_taxa = taxa;
    g_bloco = std::make_unique<AudioBlock>(taxa, 2, kQuantum);
}

__attribute__((used, visibility("default")))
void rw_seed(std::uint32_t alto, std::uint32_t baixo) {
    const std::uint64_t seed = (static_cast<std::uint64_t>(alto) << 32) | baixo;
    auto g = std::make_unique<SignalGraph>();
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g->add(std::move(n));
    g_saida = g->add(std::make_unique<rasgo::panel::SinkOut>());
    rasgo::panel::seedPatch(*g, seed, 0, fabrica);
    g->prepare(g_taxa, 2, kQuantum);
    g->setActiveOutput(g_saida);
    rasgo::panel::balanceSeedLevel(*g, seed, g_taxa, kQuantum);
    g_grafo = std::move(g);
}

__attribute__((used, visibility("default")))
float* rw_processar() {
    if (!g_grafo || !g_bloco) {
        for (auto& x : g_pcm) x = 0.0f;
        return g_pcm;
    }
    g_grafo->process(*g_bloco, g_saida, 0);
    for (std::size_t i = 0; i < kQuantum; ++i) {
        float l = g_bloco->at(0, i), r = g_bloco->at(1, i);
        g_pcm[2 * i] = std::isfinite(l) ? l : 0.0f;
        g_pcm[2 * i + 1] = std::isfinite(r) ? r : 0.0f;
    }
    return g_pcm;
}

__attribute__((used, visibility("default")))
int rw_cabos() { return g_grafo ? static_cast<int>(g_grafo->cableCount()) : 0; }

}  // extern "C"
