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
//   rw_descrever()          o rack em JSON (módulos, painéis, parâmetros,
//                           portas), para a página desenhar; tamanho em
//                           rw_texto_tamanho()
//   rw_valores()            o valor atual de cada controle do JSON, na
//                           mesma ordem (Float32)
//   rw_cabos_estado()       os cabos agora: 14 números por cabo (Float32)
//   rw_cabo_*(d, pd, …)     o inspetor do cabo que chega em (d, pd):
//                           relação, condutância, ganho, romper/reconectar
//   rw_param(i, v)          o músico gira o controle i (base do parâmetro,
//                           como `setParameterBase` no app)
//   rw_param_usuario(i)     o valor do músico no controle i, sem modulação
//   rw_ligar(o, po, d, pd)  um cabo, como `tryPatch` no app (laço vira
//                           realimentação); devolve 1 se mudou o patch
//   rw_cortar_entrada(d, p) tira o cabo que chega numa entrada
//   rw_cortar_saida(o, p)   tira todos os cabos que saem de uma saída
//   rw_mao(ativa)           o músico está com a mão num controle: o VARIA
//                           não disputa (como `interacting()` no app)
//   rw_visor(id)            o visor (Display) do módulo agora; ids em
//                           rw_num_visores()/rw_visor_id(k)
//   rw_varia(ligado, int.)  o VARIA (a mão caótica, `MotionEngine`): ligado
//                           por padrão, intensidade 0–2 (1 = a do app)
//
// O VARIA segue o caminho do app JUCE: um passo a cada ~1/30 s, energia =
// RMS da saída × 2,2 ou a redução de ganho do MASTER × 0,18 (o que for
// maior), o LUFS de curto prazo da saída guiando chão e teto, e os cabos
// relidos a cada 30 passos. Aqui o passo roda dentro do worklet, entre dois
// blocos: no navegador há uma thread só, então não há o que disputar.

#include "core/SignalGraph.hpp"
#include "dsp/Cell.hpp"
#include "dsp/Coherence.hpp"
#include "dsp/Key.hpp"
#include "dsp/Loudness.hpp"
#include "dsp/Master.hpp"
#include "dsp/Mixer.hpp"
#include "escalas/BancoEscalas.hpp"
#include "juce/RasgoWordmark.h"   // só o vetor do wordmark, sem JUCE
#include "panel/ModuleCatalog.hpp"
#include "panel/Instances.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchGenetics.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"
#include "panel/UiLanguage.hpp"
#include "ui/ScopeTrace.hpp"

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_set>
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
bool g_musico = false;   // o músico mexendo: a mão do VARIA espera
int g_blocosPorPasso = 12;   // ~1/30 s em blocos de 128
int g_blocos = 0, g_passos = 0;

// a descrição do rack para a página (`rw_descrever`) e os controles que ela
// lê a cada quadro (`rw_valores`), na mesma ordem
std::string g_texto;
struct Controle { std::size_t no; std::string bind; };
std::vector<Controle> g_controles;
std::vector<float> g_valores, g_cabos;

// os visores (`Display`): a mesma telemetria do app (`Rack::feedScopes`),
// um `ScopeTrace` por módulo com visor. O app enche no bloco do
// dispositivo (512 amostras, o usual); aqui o quantum é 128, então o
// passo da onda segue contínuo entre quanta e o que o app empurra "uma vez
// por bloco" sai a cada 4 quanta, com o máximo dos 4 onde o app usa o
// máximo do bloco. Assim o visor cobre o mesmo tempo que no app.
constexpr std::size_t kBlocoApp = 512;
constexpr std::size_t kPassoOnda = kBlocoApp / 44;          // = o `stride` do app
constexpr int kQuantaPorBloco = static_cast<int>(kBlocoApp / kQuantum);
constexpr std::size_t kVisor = rasgo::ui::kScopeLen + 4 * rasgo::ui::kLaneLen + 16;
std::vector<std::size_t> g_visores;                // ids com visor, à vista
std::map<std::size_t, rasgo::ui::ScopeTrace> g_scopes;
std::map<std::size_t, float> g_maxBloco;           // gate/GR/colapso: o máximo do bloco
std::size_t g_faseOnda = 0;
int g_quantaBloco = 0;
float g_visor[kVisor] = {};
std::string g_nomeEscala;

// o patch como o app o guarda: a seed de origem (0 = feito à mão), a ordem
// dos módulos no rack (vazia = a de fábrica), a pilha do DESFAZ (fotos em
// texto, como `Rack::snapshot`), a ESPERA e o texto de SALVA/ABRIR
std::uint64_t g_seedAtual = 0;
std::vector<std::size_t> g_ordem;
std::vector<std::size_t> g_shown;                  // a última ordem mostrada
constexpr std::size_t kUndo = 24;                  // `kUndoDepth` do app
std::vector<std::string> g_undo;
bool g_espera = false;
bool g_todosRompidos = false;                      // [espaço]: todos rompidos
std::string g_salvo;
std::vector<char> g_entrada;

std::string familia(const std::string& tipo) {
    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types)
            if (tipo == t) return grp.family;
    return {};
}

void json(std::string& o, const std::string& s) {
    o += '"';
    for (const char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (static_cast<unsigned char>(c) < 0x20) o += ' ';
        else o += c;
    }
    o += '"';
}

void numero(std::string& o, float v) {
    if (!std::isfinite(v)) v = 0.0f;
    char b[32];
    std::snprintf(b, sizeof b, "%.6g", static_cast<double>(v));
    o += b;
}

// a porta de um jack ("in:<nome>" / "out:<nome>"), como `resolveJack` do app
bool porta(const Signal& n, const std::string& bind, bool& saida, int& idx, int& tipo) {
    saida = bind.rfind("out:", 0) == 0;
    if (!saida && bind.rfind("in:", 0) != 0) return false;
    const std::string nome = bind.substr(saida ? 4 : 3);
    const std::size_t k = saida ? n.outputCount() : n.inputCount();
    for (std::size_t i = 0; i < k; ++i) {
        const auto& d = saida ? n.outputDescriptor(i) : n.inputDescriptor(i);
        if (d.name == nome) { idx = static_cast<int>(i); tipo = static_cast<int>(d.kind); return true; }
    }
    return false;
}

void descrever() {
    using Kind = rasgo::modular::Widget::Kind;
    g_texto.clear();
    g_controles.clear();
    auto& g = *g_grafo;
    // os módulos à vista, na ordem do app (`Rack::build` +
    // `syncSeedCopies`): o rack de fábrica por família, alfabético dentro
    // dela, MIXER e MASTER no fim; das cópias que a seed criou, só as
    // cabeadas, junto da sua família
    std::vector<std::size_t> shown;
    if (!g_ordem.empty()) {
        // a ordem que veio do arquivo (ABRIR/DESFAZ), só ids válidos
        for (const auto id : g_ordem)
            if (id < g.nodeCount() && g.node(id).type() != "OUT") shown.push_back(id);
    }
    if (shown.empty()) {
        for (const auto& fam : rasgo::panel::moduleCatalog()) {
            std::vector<const char*> ord(fam.types.begin(), fam.types.end());
            rasgo::panel::sortFamilyForDisplay(ord);
            for (const char* t : ord)
                for (std::size_t i = 0; i < g.nodeCount(); ++i)
                    if (g.node(i).type() == t) { shown.push_back(i); break; }
        }
    }
    {
        std::vector<char> cabeado(g.nodeCount(), 0);
        for (std::size_t i = 0; i < g.cableCount(); ++i) {
            const auto& c = g.cable(i);
            if (c.source().node < cabeado.size()) cabeado[c.source().node] = 1;
            if (c.target().node < cabeado.size()) cabeado[c.target().node] = 1;
        }
        for (std::size_t id = 0; id < g.nodeCount(); ++id) {
            if (id == g_saida || !cabeado[id]
                || std::find(shown.begin(), shown.end(), id) != shown.end()) continue;
            const std::string t = g.node(id).type();
            bool antes = false;
            for (std::size_t j = 0; j < id && !antes; ++j) antes = g.node(j).type() == t;
            if (!antes) continue;
            const auto at = rasgo::panel::moduleInsertionIndex(
                shown, t, [&g](std::size_t i) { return g.node(i).type(); });
            shown.insert(shown.begin() + static_cast<std::ptrdiff_t>(at), id);
        }
    }
    g_shown = shown;
    const std::vector<char> chega = g.nodesFeeding(g_saida);
    g_visores.clear();
    for (const std::size_t id : shown)
        for (const auto& w : g.node(id).panel().widgets)
            if (w.kind == Kind::Display) { g_visores.push_back(id); g_scopes[id]; break; }
    std::string& o = g_texto;
    o += "{\"saida\":";
    o += std::to_string(g_saida);
    o += ",\"modulos\":[";
    bool primeiro = true;
    for (const std::size_t id : shown) {
        Signal& n = g.node(id);
        const rasgo::modular::Panel p = n.panel();
        if (!primeiro) o += ',';
        primeiro = false;
        o += "{\"id\":" + std::to_string(id) + ",\"tipo\":";
        json(o, n.type());
        o += ",\"familia\":";
        json(o, familia(n.type()));
        o += ",\"hp\":" + std::to_string(p.hp);
        o += ",\"inst\":" + std::to_string(rasgo::panel::instanceNumber(g, shown, id));
        o += ",\"chega\":" + std::string(id < chega.size() && chega[id] ? "1" : "0");
        o += ",\"w\":[";
        for (std::size_t k = 0; k < p.widgets.size(); ++k) {
            const auto& w = p.widgets[k];
            if (k) o += ',';
            o += "{\"k\":" + std::to_string(static_cast<int>(w.kind)) + ",\"l\":";
            json(o, w.label);
            o += ",\"b\":";
            json(o, w.bind);
            o += ",\"x\":"; numero(o, w.x);
            o += ",\"y\":"; numero(o, w.y);
            o += ",\"s\":"; numero(o, w.span);
            o += ",\"h\":"; numero(o, w.height);
            if ((w.kind == Kind::Knob || w.kind == Kind::Slider || w.kind == Kind::Toggle)
                && !w.bind.empty()) {
                for (const auto& par : n.parameters())
                    if (par.descriptor.id == w.bind) {
                        o += ",\"v\":" + std::to_string(g_controles.size());
                        o += ",\"min\":"; numero(o, par.descriptor.minimum);
                        o += ",\"max\":"; numero(o, par.descriptor.maximum);
                        o += ",\"def\":"; numero(o, par.descriptor.defaultValue);
                        o += ",\"u\":"; json(o, par.descriptor.unit);
                        g_controles.push_back({id, w.bind});
                        break;
                    }
            } else if (w.kind == Kind::Jack) {
                bool saida = false; int idx = -1, tipo = 0;
                if (porta(n, w.bind, saida, idx, tipo))
                    o += ",\"out\":" + std::string(saida ? "1" : "0")
                       + ",\"p\":" + std::to_string(idx)
                       + ",\"pk\":" + std::to_string(tipo);
            }
            o += '}';
        }
        o += "]}";
    }
    o += "]}";
    g_valores.assign(g_controles.size(), 0.0f);
}

// `Rack::feedScopes` do app, quantum a quantum (ver `kBlocoApp`)
void alimentarVisores() {
    auto& g = *g_grafo;
    const bool fimDoBloco = ++g_quantaBloco >= kQuantaPorBloco;
    if (fimDoBloco) g_quantaBloco = 0;
    const std::size_t inicio = (kPassoOnda - g_faseOnda % kPassoOnda) % kPassoOnda;
    g_faseOnda = (g_faseOnda + kQuantum) % kPassoOnda;
    for (const std::size_t id : g_visores) {
        Signal& n = g.node(id);
        if (n.outputCount() == 0) continue;
        const AudioBlock* ob = g.lastOutput(id, 0);
        if (!ob || ob->frames() == 0) continue;
        auto& sc = g_scopes[id];
        const std::size_t fr = ob->frames(), ult = fr - 1;
        for (std::size_t i = inicio; i < fr; i += kPassoOnda) sc.push(ob->at(0, i));
        const std::string t = n.type();
        float& mx = g_maxBloco[id];
        auto maxDe = [](const AudioBlock* b) {
            float r = 0.0f;
            for (std::size_t i = 0; i < b->frames(); ++i) r = std::max(r, b->at(0, i));
            return r;
        };
        if (t == "TRIGSEQ" && n.outputCount() >= 4) {
            const AudioBlock *o1 = g.lastOutput(id, 1), *o2 = g.lastOutput(id, 2), *o3 = g.lastOutput(id, 3);
            if (o1 && o2 && o3)
                for (std::size_t i = inicio; i < fr; i += kPassoOnda)
                    sc.pushLanes(ob->at(0, i), o1->at(0, i), o2->at(0, i), o3->at(0, i));
        } else if (t == "LISTEN") {
            const AudioBlock *lo = g.lastOutput(id, 1), *mi = g.lastOutput(id, 2), *hi = g.lastOutput(id, 3);
            const AudioBlock *lh = g.lastOutput(id, 4), *mh = g.lastOutput(id, 5), *hh = g.lastOutput(id, 6);
            if (lo && mi && hi && lh && mh && hh)
                for (std::size_t i = inicio; i < fr; i += kPassoOnda) {
                    const int mask = (lh->at(0, i) >= 0.5f ? 1 : 0) | (mh->at(0, i) >= 0.5f ? 2 : 0)
                                   | (hh->at(0, i) >= 0.5f ? 4 : 0);
                    sc.pushLanes(lo->at(0, i), mi->at(0, i), hi->at(0, i), static_cast<float>(mask));
                }
        } else if (t == "PHRASE") {
            const AudioBlock *ga = g.lastOutput(id, 1), *ac = g.lastOutput(id, 2), *ph = g.lastOutput(id, 4);
            if (ga && ac && ph)
                for (std::size_t i = inicio; i < fr; i += kPassoOnda)
                    sc.pushLanes(ob->at(0, i), ga->at(0, i), ac->at(0, i), ph->at(0, i));
        } else if (t == "DYNAMICS") {
            const AudioBlock *env = g.lastOutput(id, 1), *gr = g.lastOutput(id, 2);
            if (env && gr) {
                mx = std::max(mx, maxDe(gr));
                if (fimDoBloco) { sc.pushLanes(env->at(0, env->frames() - 1), mx, 0.0f, 0.0f); mx = 0.0f; }
            }
        } else if (t == "CELL") {
            if (fimDoBloco)
                if (const auto* c = dynamic_cast<const rasgo::modular::Cell*>(&n))
                    sc.pushLanes(c->rowAsFloat(), n.parameterValue("cells"), 0.0f, 0.0f);
        } else if (t == "MOUSSE") {
            if (fimDoBloco) sc.pushLanes(ob->at(0, ult), 0.0f, 0.0f, 0.0f);
        } else if (t == "COHERENCE") {
            const AudioBlock *ord = g.lastOutput(id, 0), *pres = g.lastOutput(id, 1), *col = g.lastOutput(id, 2);
            if (ord && pres && col) {
                mx = std::max(mx, maxDe(col));
                if (fimDoBloco) {
                    sc.pushLanes(ord->at(0, ord->frames() - 1), pres->at(0, pres->frames() - 1), mx, 0.0f);
                    mx = 0.0f;
                }
            }
        } else if (t == "MIXER") {
            if (fimDoBloco)
                if (const auto* m = dynamic_cast<const rasgo::modular::Mixer*>(&n))
                    sc.pushLanes(m->stagePosition(0), m->stagePosition(1), m->stagePosition(2), m->stagePosition(3));
        } else if (t == "PENDULUM") {
            const AudioBlock *x1 = g.lastOutput(id, 0), *y1 = g.lastOutput(id, 1);
            const AudioBlock *x2 = g.lastOutput(id, 2), *y2 = g.lastOutput(id, 3);
            if (fimDoBloco && x1 && y1 && x2 && y2 && x1->frames() > 0) {
                const std::size_t k = x1->frames() - 1;
                sc.pushLanes(x1->at(0, k), y1->at(0, k), x2->at(0, k), y2->at(0, k));
            }
        } else if (t == "BOUNCE") {
            const AudioBlock *h1 = g.lastOutput(id, 2), *h2 = g.lastOutput(id, 3), *h3 = g.lastOutput(id, 4);
            if (fimDoBloco && h1 && h2 && h3 && h1->frames() > 0) {
                const std::size_t k = h1->frames() - 1;
                sc.pushLanes(h1->at(0, k), h2->at(0, k), h3->at(0, k), n.parameterValue("balls"));
            }
        } else if (t == "ARP" || t == "MARKOV") {
            if (const AudioBlock* ga = g.lastOutput(id, 1)) {
                mx = std::max(mx, maxDe(ga));
                if (fimDoBloco) { sc.pushLanes(ob->at(0, ult), mx, 0.0f, 0.0f); mx = 0.0f; }
            }
        }
    }
}

float parametro(Signal& n, const char* id) {
    try { return n.parameterValue(id); } catch (...) { return 0.0f; }
}

std::unique_ptr<Signal> fabrica(const std::string& t) {
    if (t == "OUT") return std::make_unique<rasgo::panel::SinkOut>();
    return rasgo::panel::makeModule(t);
}

// um grafo novo passa a ser o patch (seed, ABRIR, DESFAZ): o MASTER, a
// saída, a mão do VARIA e os visores são refeitos em volta dele
void adotar(std::unique_ptr<SignalGraph> g, std::uint64_t seed) {
    g_saida = 0;
    for (std::size_t i = 0; i < g->nodeCount(); ++i)
        if (g->node(i).type() == "OUT") g_saida = i;
    g->prepare(g_taxa, 2, kQuantum);
    g->setActiveOutput(g_saida);
    // a mão habita o rack inteiro, como no app depois de uma seed
    std::vector<std::size_t> shown;
    g_masters.clear();
    for (std::size_t i = 0; i < g->nodeCount(); ++i) {
        if (g->node(i).type() == "OUT") continue;
        shown.push_back(i);
        if (auto* m = dynamic_cast<rasgo::modular::Master*>(&g->node(i))) {
            g_masters.push_back(m);
            // a ESPERA vale para o patch que chega
            if (g_espera) g->setParameterBase(i, "mute", 1.0f);
        }
    }
    g_seedAtual = seed;
    g_todosRompidos = false;
    g_mao.inhabit(*g, seed, shown);
    g_mao.setLoudnessTarget(rasgo::panel::kSeedTargetLufs);
    g_lufs.reset();
    g_blocos = 0; g_passos = 0;
    g_grafo = std::move(g);
    g_scopes.clear(); g_maxBloco.clear(); g_faseOnda = 0; g_quantaBloco = 0;
    descrever();
}

std::unique_ptr<SignalGraph> grafoDaSeed(std::uint64_t seed) {
    auto g = std::make_unique<SignalGraph>();
    for (const auto& f : rasgo::panel::seedModuleCatalog())
        for (const char* t : f.types)
            if (auto n = rasgo::panel::makeModule(t)) g->add(std::move(n));
    const auto saida = g->add(std::make_unique<rasgo::panel::SinkOut>());
    rasgo::panel::seedPatch(*g, seed, 0, fabrica);
    g->prepare(g_taxa, 2, kQuantum);
    g->setActiveOutput(saida);
    rasgo::panel::balanceSeedLevel(*g, seed, g_taxa, kQuantum);
    return g;
}

// o patch em texto, o mesmo `.rmp` do app (`patchText`/`snapshot`)
std::string textoDoPatch() {
    std::string t = g_grafo->serialize();
    if (g_seedAtual) t += "seed " + std::to_string(g_seedAtual) + "\n";
    t += "panel shown";
    for (const auto id : g_shown) t += ' ' + std::to_string(id);
    t += '\n';
    return t;
}

// `Rack::pushUndo`: a foto do patch antes de uma ação que o muda
void foto() {
    if (!g_grafo) return;
    g_undo.push_back(textoDoPatch());
    if (g_undo.size() > kUndo) g_undo.erase(g_undo.begin());
}

// `applyPatchText`: a ordem e a seed do arquivo, ids fora de faixa fora
bool aplicarTexto(const std::string& texto) {
    std::vector<std::size_t> ord;
    std::uint64_t seed = 0;
    std::istringstream is(texto);
    std::string linha;
    while (std::getline(is, linha)) {
        if (linha.rfind("panel shown", 0) == 0) {
            std::istringstream ls(linha.substr(11));
            std::size_t id = 0;
            while (ls >> id) ord.push_back(id);
        } else if (linha.rfind("seed ", 0) == 0) {
            seed = std::strtoull(linha.c_str() + 5, nullptr, 10);
        }
    }
    try {
        auto g = std::make_unique<SignalGraph>(SignalGraph::deserialize(texto, fabrica));
        bool temSaida = false;
        for (std::size_t i = 0; i < g->nodeCount(); ++i) temSaida |= g->node(i).type() == "OUT";
        if (!temSaida || g->nodeCount() < 2) return false;
        g_ordem = ord;
        adotar(std::move(g), seed);
        return true;
    } catch (...) {
        return false;
    }
}

// MIXER e MASTER ficam fora das operações genéticas (`freezeMixMaster`)
std::unordered_set<std::size_t> congelados() {
    std::unordered_set<std::size_t> f;
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i) {
        const std::string t = g_grafo->node(i).type();
        if (t == "MIXER" || t == "MASTER") f.insert(i);
    }
    return f;
}

std::uint64_t seed64(std::uint32_t alto, std::uint32_t baixo) {
    return (static_cast<std::uint64_t>(alto) << 32) | baixo;
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
    const std::uint64_t seed = seed64(alto, baixo);
    foto();
    g_ordem.clear();
    adotar(grafoDaSeed(seed), seed);
}

// REPOR: a seed atual de novo, sem a edição à mão (desfazível)
__attribute__((used, visibility("default")))
int rw_repor() {
    if (!g_grafo || g_seedAtual == 0) return 0;
    foto();
    g_ordem.clear();
    adotar(grafoDaSeed(g_seedAtual), g_seedAtual);
    return 1;
}

// a seed do patch que soa (0 = feito à mão), em duas metades
__attribute__((used, visibility("default")))
std::uint32_t rw_seed_alto() { return static_cast<std::uint32_t>(g_seedAtual >> 32); }
__attribute__((used, visibility("default")))
std::uint32_t rw_seed_baixo() { return static_cast<std::uint32_t>(g_seedAtual & 0xffffffffu); }

// uma foto para o DESFAZ (a página chama no começo de um gesto do inspetor)
__attribute__((used, visibility("default")))
void rw_foto() { foto(); }

__attribute__((used, visibility("default")))
int rw_pode_desfazer() { return g_undo.empty() ? 0 : 1; }

__attribute__((used, visibility("default")))
int rw_desfazer() {
    if (g_undo.empty()) return 0;
    const std::string t = std::move(g_undo.back());
    g_undo.pop_back();
    return aplicarTexto(t) ? 1 : 0;
}

// MUDA, EVOLUI, CRUZA (`PatchGenetics.hpp`), com as seeds sorteadas pela
// página; MIXER e MASTER congelados, a mão do VARIA rehabita o patch
void rehabitar() {
    std::vector<std::size_t> todos;
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i)
        if (i != g_saida) todos.push_back(i);
    g_mao.inhabit(*g_grafo, g_seedAtual, todos);
}

__attribute__((used, visibility("default")))
void rw_mudar(std::uint32_t alto, std::uint32_t baixo) {
    if (!g_grafo) return;
    foto();
    rasgo::panel::mutatePatch(*g_grafo, seed64(alto, baixo), 0.25f, congelados());
    rehabitar();
}

__attribute__((used, visibility("default")))
void rw_evoluir(std::uint32_t alto, std::uint32_t baixo) {
    if (!g_grafo) return;
    foto();
    rasgo::panel::evolvePatch(*g_grafo, seed64(alto, baixo), 6, 0.12f, congelados());
    rehabitar();
}

// CRUZA: recombina com um doador novo — o catálogo semeado com outra seed
__attribute__((used, visibility("default")))
void rw_cruzar(std::uint32_t alto, std::uint32_t baixo, std::uint32_t dAlto, std::uint32_t dBaixo) {
    if (!g_grafo) return;
    foto();
    SignalGraph doador;
    for (const auto& fam : rasgo::panel::seedModuleCatalog())
        for (const char* t : fam.types)
            if (auto n = rasgo::panel::makeModule(t)) doador.add(std::move(n));
    rasgo::panel::seedPatch(doador, seed64(dAlto, dBaixo));
    rasgo::panel::crossPatch(*g_grafo, doador, seed64(alto, baixo), 0.5f, congelados());
    rehabitar();
}

// DESCABEIA: tira todos os cabos menos o da saída de som (`clearCables`)
__attribute__((used, visibility("default")))
void rw_descabear() {
    if (!g_grafo) return;
    foto();
    for (std::size_t i = g_grafo->cableCount(); i-- > 0;) {
        const auto& c = g_grafo->cable(i);
        if (c.target().node == g_saida) continue;
        g_grafo->disconnect(c.target().node, c.target().port);
    }
    g_grafo->prepare(g_taxa, 2, kQuantum);
    g_grafo->setActiveOutput(g_saida);
    rehabitar();
    descrever();
}

// [espaço]: rompe e reata todos os cabos de uma vez (`toggleRupture`)
__attribute__((used, visibility("default")))
void rw_romper_todos() {
    if (!g_grafo) return;
    g_todosRompidos = !g_todosRompidos;
    for (std::size_t i = 0; i < g_grafo->cableCount(); ++i) {
        if (g_todosRompidos) g_grafo->cable(i).rupture();
        else g_grafo->cable(i).reconnect();
    }
}

// ESPERA: o MASTER mudo (com a rampa dele), o patch segue correndo
__attribute__((used, visibility("default")))
void rw_espera(int ligada) {
    g_espera = ligada != 0;
    if (!g_grafo) return;
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i)
        if (g_grafo->node(i).type() == "MASTER")
            g_grafo->setParameterBase(i, "mute", g_espera ? 1.0f : 0.0f);
}

// SALVA/BANCO: o patch em texto (.rmp); tamanho em rw_salvo_tamanho()
__attribute__((used, visibility("default")))
const char* rw_salvar() {
    g_salvo = g_grafo ? textoDoPatch() : std::string{};
    return g_salvo.c_str();
}
__attribute__((used, visibility("default")))
int rw_salvo_tamanho() { return static_cast<int>(g_salvo.size()); }

// ABRIR: a página escreve o texto em rw_entrada(n) e chama rw_abrir(n)
__attribute__((used, visibility("default")))
char* rw_entrada(int n) {
    g_entrada.assign(static_cast<std::size_t>(std::max(0, n)) + 1, 0);
    return g_entrada.data();
}
__attribute__((used, visibility("default")))
int rw_abrir(int n) {
    if (n <= 0 || static_cast<std::size_t>(n) >= g_entrada.size()) return 0;
    const std::string t(g_entrada.data(), static_cast<std::size_t>(n));
    foto();
    if (aplicarTexto(t)) return 1;
    g_undo.pop_back();   // não abriu: a foto não tem o que desfazer
    return 0;
}

// o VU do cabeçalho: o pico recente do MASTER (o mesmo anel do visor)
__attribute__((used, visibility("default")))
float rw_vu() {
    float p = 0.0f;
    for (auto* m : g_masters)
        for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i)
            if (&g_grafo->node(i) == m) {
                const auto it = g_scopes.find(i);
                if (it != g_scopes.end()) p = std::max(p, it->second.peak());
            }
    return p;
}

__attribute__((used, visibility("default")))
float* rw_processar() {
    if (!g_grafo || !g_bloco) {
        for (auto& x : g_pcm) x = 0.0f;
        return g_pcm;
    }
    g_grafo->process(*g_bloco, g_saida, 0);
    alimentarVisores();
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
    if (g_varia && !g_musico && ++g_blocos >= g_blocosPorPasso) {
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
const char* rw_descrever() { return g_texto.c_str(); }

__attribute__((used, visibility("default")))
int rw_texto_tamanho() { return static_cast<int>(g_texto.size()); }

// o wordmark RASGO (caminho SVG, caixa em rasgo::ui::kWordmark*)
__attribute__((used, visibility("default")))
const char* rw_wordmark() { return rasgo::ui::kWordmarkPath; }

__attribute__((used, visibility("default")))
int rw_num_controles() { return static_cast<int>(g_controles.size()); }

__attribute__((used, visibility("default")))
float* rw_valores() {
    if (!g_grafo) return g_valores.data();
    for (std::size_t i = 0; i < g_controles.size(); ++i) {
        try { g_valores[i] = g_grafo->node(g_controles[i].no).parameterValue(g_controles[i].bind); }
        catch (...) { g_valores[i] = 0.0f; }
    }
    return g_valores.data();
}

// por cabo (14 números): origem, porta, destino, porta, rompido, conduzindo
// agora, ganho, tem relação, condutância, relação (0 nenhuma, 1 ring, 2 fold,
// 3 diferença), quantidade, companion (nó, porta), realimentação
__attribute__((used, visibility("default")))
float* rw_cabos_estado() {
    g_cabos.clear();
    if (!g_grafo) return g_cabos.data();
    for (std::size_t i = 0; i < g_grafo->cableCount(); ++i) {
        const auto& c = g_grafo->cable(i);
        g_cabos.push_back(static_cast<float>(c.source().node));
        g_cabos.push_back(static_cast<float>(c.source().port));
        g_cabos.push_back(static_cast<float>(c.target().node));
        g_cabos.push_back(static_cast<float>(c.target().port));
        g_cabos.push_back(c.state() == rasgo::modular::CableState::Ruptured ? 1.0f : 0.0f);
        g_cabos.push_back(c.conductingNow() ? 1.0f : 0.0f);
        g_cabos.push_back(c.gain());
        g_cabos.push_back(c.hasRelation() ? 1.0f : 0.0f);
        g_cabos.push_back(c.conductance());
        g_cabos.push_back(static_cast<float>(static_cast<int>(c.relation())));
        g_cabos.push_back(c.relationAmount());
        g_cabos.push_back(static_cast<float>(c.companion().node));
        g_cabos.push_back(static_cast<float>(c.companion().port));
        g_cabos.push_back(c.isFeedback() ? 1.0f : 0.0f);
    }
    return g_cabos.data();
}

__attribute__((used, visibility("default")))
void rw_param(int i, float v) {
    if (!g_grafo || i < 0 || static_cast<std::size_t>(i) >= g_controles.size()) return;
    try { g_grafo->setParameterBase(g_controles[i].no, g_controles[i].bind, v); } catch (...) {}
}

__attribute__((used, visibility("default")))
float rw_param_usuario(int i) {
    if (!g_grafo || i < 0 || static_cast<std::size_t>(i) >= g_controles.size()) return 0.0f;
    try { return g_grafo->parameterUserValue(g_controles[i].no, g_controles[i].bind); }
    catch (...) { return 0.0f; }
}

// depois de mexer nos cabos: o grafo se prepara de novo (como `reprepare`
// no app), a mão do VARIA relê os cabos e a descrição do rack é refeita
// (quem chega à saída, cópias de voz que passaram a ter cabo)
void depoisDosCabos() {
    g_grafo->prepare(g_taxa, 2, kQuantum);
    g_grafo->setActiveOutput(g_saida);
    g_mao.refreshCables(*g_grafo);
    descrever();
}

__attribute__((used, visibility("default")))
int rw_ligar(int o, int po, int d, int pd) {
    if (!g_grafo || o < 0 || d < 0) return 0;
    const auto s = static_cast<std::size_t>(o), sp = static_cast<std::size_t>(po);
    const auto t = static_cast<std::size_t>(d), tp = static_cast<std::size_t>(pd);
    if (s >= g_grafo->nodeCount() || t >= g_grafo->nodeCount()) return 0;
    // como `tryPatch`: o laço só aparece no `prepare`, que por isso fica
    // dentro do mesmo try — um ciclo vira ligação de realimentação
    auto preparar = [] { g_grafo->prepare(g_taxa, 2, kQuantum); };
    foto();   // repatchear é desfazível
    try { g_grafo->disconnect(t, tp); } catch (...) {}
    try {
        g_grafo->connect(s, sp, t, tp);
        preparar();
    } catch (const std::logic_error&) {            // ciclo: realimentação
        try { g_grafo->disconnect(t, tp); } catch (...) {}
        try { g_grafo->connect(s, sp, t, tp, /*feedback=*/true); preparar(); }
        catch (...) { try { g_grafo->disconnect(t, tp); } catch (...) {} }
    } catch (...) {
        try { g_grafo->disconnect(t, tp); } catch (...) {}
    }
    depoisDosCabos();
    return 1;
}

__attribute__((used, visibility("default")))
int rw_cortar_entrada(int d, int pd) {
    if (!g_grafo || d < 0 || static_cast<std::size_t>(d) >= g_grafo->nodeCount()) return 0;
    bool mudou = false;
    foto();   // cortar é desfazível
    try { mudou = g_grafo->disconnect(static_cast<std::size_t>(d), static_cast<std::size_t>(pd)); } catch (...) {}
    if (mudou) depoisDosCabos();
    else g_undo.pop_back();
    return mudou ? 1 : 0;
}

__attribute__((used, visibility("default")))
int rw_cortar_saida(int o, int po) {
    if (!g_grafo || o < 0) return 0;
    bool mudou = false;
    foto();
    for (std::size_t i = g_grafo->cableCount(); i-- > 0;) {
        const auto& c = g_grafo->cable(i);
        if (c.target().node == g_saida) continue;   // a ligação ao sink fica
        if (c.source().node == static_cast<std::size_t>(o)
            && c.source().port == static_cast<std::size_t>(po)) {
            const auto t = c.target();
            g_grafo->disconnect(t.node, t.port);
            mudou = true;
        }
    }
    if (mudou) depoisDosCabos();
    else g_undo.pop_back();
    return mudou ? 1 : 0;
}

// o cabo que chega numa entrada (cada entrada aceita um só): é assim que o
// app identifica o cabo do inspetor, estável entre ligações e cortes
rasgo::modular::Cable* caboEm(int d, int pd) {
    if (!g_grafo) return nullptr;
    for (std::size_t i = 0; i < g_grafo->cableCount(); ++i) {
        auto& c = g_grafo->cable(i);
        if (static_cast<int>(c.target().node) == d && static_cast<int>(c.target().port) == pd) return &c;
    }
    return nullptr;
}

// relação: 0 nenhuma, 1 ring, 2 fold, 3 diferença; companion = uma saída
__attribute__((used, visibility("default")))
void rw_cabo_relacao(int d, int pd, int rel, int cn, int cp, float amt) {
    auto* c = caboEm(d, pd);
    if (!c || rel < 0 || rel > 3) return;
    try {
        if (rel == 0) c->setRelation(rasgo::modular::Relation{}, 0, 0, 0.0f);
        else c->setRelation(static_cast<rasgo::modular::Relation>(rel),
                            static_cast<std::size_t>(cn), static_cast<std::size_t>(cp),
                            std::clamp(amt, 0.0f, 1.0f));
    } catch (...) {}
}

__attribute__((used, visibility("default")))
void rw_cabo_condutancia(int d, int pd, float v) {
    if (auto* c = caboEm(d, pd)) c->setConductance(std::clamp(v, 0.0f, 1.0f));
}

__attribute__((used, visibility("default")))
void rw_cabo_ganho(int d, int pd, float v) {
    if (auto* c = caboEm(d, pd)) c->setGain(std::clamp(v, 0.0f, 2.0f));
}

__attribute__((used, visibility("default")))
void rw_cabo_romper(int d, int pd) {
    auto* c = caboEm(d, pd);
    if (!c) return;
    if (c->state() == rasgo::modular::CableState::Ruptured) c->reconnect();
    else c->rupture();
}

// os ids com visor, na ordem do rack (quantos em rw_num_visores)
__attribute__((used, visibility("default")))
int rw_num_visores() { return static_cast<int>(g_visores.size()); }

__attribute__((used, visibility("default")))
int rw_visor_id(int k) {
    return k >= 0 && static_cast<std::size_t>(k) < g_visores.size() ? static_cast<int>(g_visores[k]) : -1;
}

// o visor do módulo `id` agora, `kVisor` números: o anel da onda (220, em
// ordem cronológica), as 4 lanes (128 cada, cronológicas) e 16 extras por
// tipo — o que o desenho do app lê direto do módulo:
//   MASTER    redução de ganho (dB)
//   DYNAMICS  thresh, mode          CELL   rule
//   KEY       histograma (12), tônica, nota atual, menor, certeza
//   COHERENCE histograma (12)       PHRASE length, span
//   MIXER     peso dos 4 canais, centro, stage        QUANTIZER scale
__attribute__((used, visibility("default")))
float* rw_visor(int id) {
    std::fill(std::begin(g_visor), std::end(g_visor), 0.0f);
    if (!g_grafo || id < 0) return g_visor;
    const auto it = g_scopes.find(static_cast<std::size_t>(id));
    if (it == g_scopes.end()) return g_visor;
    const auto& sc = it->second;
    float* v = g_visor;
    for (std::size_t k = 0; k < rasgo::ui::kScopeLen; ++k) *v++ = sc.at(k);
    for (std::size_t L = 0; L < 4; ++L)
        for (std::size_t k = 0; k < rasgo::ui::kLaneLen; ++k) *v++ = sc.lane(L, k);
    Signal& n = g_grafo->node(static_cast<std::size_t>(id));
    const std::string t = n.type();
    if (auto* m = dynamic_cast<rasgo::modular::Master*>(&n)) v[0] = m->gainReductionDb();
    else if (t == "DYNAMICS") { v[0] = parametro(n, "thresh"); v[1] = parametro(n, "mode"); }
    else if (t == "CELL") v[0] = parametro(n, "rule");
    else if (const auto* k = dynamic_cast<const rasgo::modular::Key*>(&n)) {
        const auto h = k->histogram();
        for (int i = 0; i < 12; ++i) v[i] = h[static_cast<std::size_t>(i)];
        v[12] = static_cast<float>(k->tonic()); v[13] = static_cast<float>(k->currentNote());
        v[14] = k->minor() ? 1.0f : 0.0f; v[15] = k->certainty();
    } else if (const auto* c = dynamic_cast<const rasgo::modular::Coherence*>(&n)) {
        const auto h = c->histogram();
        for (int i = 0; i < 12; ++i) v[i] = static_cast<float>(h[static_cast<std::size_t>(i)]);
    } else if (const auto* m = dynamic_cast<const rasgo::modular::Mixer*>(&n)) {
        for (int i = 0; i < 4; ++i) v[i] = m->stageWeight(i);
        v[4] = m->stageCentre(); v[5] = parametro(n, "stage");
    } else if (t == "PHRASE") { v[0] = parametro(n, "length"); v[1] = parametro(n, "span"); }
    else if (t == "QUANTIZER") v[0] = parametro(n, "scale");
    return g_visor;
}

// QUANTIZER com o BANCO: o nome da escala, para o rótulo do visor
__attribute__((used, visibility("default")))
const char* rw_nome_escala(int id) {
    g_nomeEscala.clear();
    if (!g_grafo || id < 0 || static_cast<std::size_t>(id) >= g_grafo->nodeCount()) return g_nomeEscala.c_str();
    Signal& n = g_grafo->node(static_cast<std::size_t>(id));
    const auto* e = rasgo::escalas::BancoEscalas::instancia().procura(
        static_cast<std::uint32_t>(std::lround(parametro(n, "bank"))));
    g_nomeEscala = e ? e->nome : rasgo::panel::tr(rasgo::panel::strings::scaleBankEmpty, rasgo::panel::Lang::pt);
    return g_nomeEscala.c_str();
}

__attribute__((used, visibility("default")))
void rw_mao(int ativa) { g_musico = ativa != 0; }

__attribute__((used, visibility("default")))
void rw_varia(int ligado, float intensidade) {
    g_varia = ligado != 0;
    g_mao.setIntensity(intensidade);
}

}  // extern "C"
