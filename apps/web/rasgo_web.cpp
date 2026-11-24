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
//   rw_varia(ligado, int.)  o VARIA (a mão caótica, `MotionEngine`): ligado
//                           por padrão, intensidade 0–2 (1 = a do app)
//
// O VARIA segue o caminho do app JUCE: um passo a cada ~1/30 s, energia =
// RMS da saída × 2,2 ou a redução de ganho do MASTER × 0,18 (o que for
// maior), o LUFS de curto prazo da saída guiando chão e teto, e os cabos
// relidos a cada 30 passos. Aqui o passo roda dentro do worklet, entre dois
// blocos: no navegador há uma thread só, então não há o que disputar.

#include "core/SignalGraph.hpp"
#include "dsp/Loudness.hpp"
#include "dsp/Master.hpp"
#include "escalas/BancoEscalas.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <vector>

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

// VARIA
rasgo::panel::MotionEngine g_mao;
rasgo::modular::LoudnessMeter g_lufs;
std::vector<rasgo::modular::Master*> g_masters;
bool g_varia = true;
int g_blocosPorPasso = 12;   // ~1/30 s em blocos de 128
int g_blocos = 0, g_passos = 0;

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
    g_lufs.prepare(taxa);
    g_blocosPorPasso = std::max(1, static_cast<int>(
        std::lround(taxa / 30.0f / static_cast<float>(kQuantum))));
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
    // a mão habita o rack inteiro, como no app depois de uma seed
    std::vector<std::size_t> shown;
    g_masters.clear();
    for (std::size_t i = 0; i < g->nodeCount(); ++i) {
        if (g->node(i).type() == "OUT") continue;
        shown.push_back(i);
        if (auto* m = dynamic_cast<rasgo::modular::Master*>(&g->node(i)))
            g_masters.push_back(m);
    }
    g_mao.inhabit(*g, seed, shown);
    g_mao.setLoudnessTarget(rasgo::panel::kSeedTargetLufs);
    g_lufs.reset();
    g_blocos = 0; g_passos = 0;
    g_grafo = std::move(g);
}

__attribute__((used, visibility("default")))
float* rw_processar() {
    if (!g_grafo || !g_bloco) {
        for (auto& x : g_pcm) x = 0.0f;
        return g_pcm;
    }
    g_grafo->process(*g_bloco, g_saida, 0);
    double s2 = 0.0;
    for (std::size_t i = 0; i < kQuantum; ++i) {
        float l = g_bloco->at(0, i), r = g_bloco->at(1, i);
        l = std::isfinite(l) ? l : 0.0f;
        r = std::isfinite(r) ? r : 0.0f;
        g_pcm[2 * i] = l;
        g_pcm[2 * i + 1] = r;
        g_lufs.push(l, r);
        s2 += static_cast<double>(l) * l + static_cast<double>(r) * r;
    }
    if (g_varia && ++g_blocos >= g_blocosPorPasso) {
        g_blocos = 0;
        if (++g_passos >= 30) { g_passos = 0; g_mao.refreshCables(*g_grafo); }
        const float rms = static_cast<float>(std::sqrt(s2 / (2.0 * kQuantum)));
        float grDb = 0.0f;
        for (auto* m : g_masters) grDb = std::max(grDb, m->gainReductionDb());
        const float energia = std::min(1.0f, std::max(rms * 2.2f, grDb * 0.18f));
        g_mao.tick(*g_grafo,
                   static_cast<float>(g_blocosPorPasso * kQuantum) / g_taxa,
                   energia, g_lufs.shortTerm());
    }
    return g_pcm;
}

__attribute__((used, visibility("default")))
int rw_cabos() { return g_grafo ? static_cast<int>(g_grafo->cableCount()) : 0; }

__attribute__((used, visibility("default")))
void rw_varia(int ligado, float intensidade) {
    g_varia = ligado != 0;
    g_mao.setIntensity(intensidade);
}

}  // extern "C"
