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
#include "dsp/NoteOut.hpp"
#include "dsp/PolyIn.hpp"
#include "dsp/SignalIn.hpp"
#include "escalas/BancoEscalas.hpp"
#include "juce/RasgoWordmark.h"   // só o vetor do wordmark, sem JUCE
#include "panel/ModuleCatalog.hpp"
#include "panel/Instances.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchGenetics.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/ScoreMidi.hpp"
#include "panel/ScoreRecorder.hpp"
#include "panel/SinkOut.hpp"
#include "panel/UiLanguage.hpp"
#include "ui/CompactPanel.hpp"
#include "ui/OutputLights.hpp"
#include "ui/PanelGeometry.hpp"
#include "ui/PrimaryControl.hpp"
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
// as luzes das saídas (`ui/OutputLights.hpp`): uma por jack de saída do JSON,
// na ordem de "li"; o pico com sinal desde a última leitura (`rw_luzes`)
struct Luz { std::size_t no, porta; };
std::vector<Luz> g_luzes;
std::vector<float> g_luzPico, g_luzLida;
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
bool g_compacto = false;   // a vista COMPACTA (src/ui/CompactPanel.hpp), como no app
std::vector<std::size_t> g_ordem;
std::vector<std::size_t> g_shown;                  // a última ordem mostrada
constexpr std::size_t kUndo = 24;                  // `kUndoDepth` do app
std::vector<std::string> g_undo;
bool g_espera = false;
bool g_todosRompidos = false;                      // [espaço]: todos rompidos

// REC: a partitura da tomada (`ScoreRecorder`, como no app) — o áudio fica
// no worklet; aqui o tempo é amostras gravadas / taxa, nunca relógio
constexpr double kRecMax = 240.0;                  // 4 min (decisão do autor)
bool g_gravando = false;
std::size_t g_recFrames = 0;
rasgo::panel::ScoreRecorder g_score;
std::uint64_t g_recSeed = 0;
std::vector<float> g_variaLog;
std::uint64_t g_variaGen = 0;
int g_variaTick = 0;
float g_variaAmtLog = -1.0f;
bool g_variaLigadoLog = true;
std::string g_midi;
double recTempo() { return static_cast<double>(g_recFrames) / static_cast<double>(g_taxa); }
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
        else if (c == '\n') o += "\\n";   // as quebras de linha dos textos ficam
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
    g_luzes.clear();
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
        rasgo::modular::Panel p = n.panel();
        rasgo::ui::centerPanel(p, n.type());
        if (g_compacto) rasgo::ui::compactPanel(p, n.type());
        else rasgo::ui::knobSizes(p, n.type());
        if (!primeiro) o += ',';
        primeiro = false;
        o += "{\"id\":" + std::to_string(id) + ",\"tipo\":";
        json(o, n.type());
        o += ",\"familia\":";
        json(o, familia(n.type()));
        o += ",\"hp\":" + std::to_string(p.hp);
        o += ",\"inst\":" + std::to_string(rasgo::panel::instanceNumber(g, shown, id));
        o += ",\"chega\":" + std::string(id < chega.size() && chega[id] ? "1" : "0");
        o += ",\"wm\":"; numero(o, rasgo::ui::wordmarkXMM(p, 30.75f / 258.74f));
        o += ",\"pl\":[";
        {
            bool pr = true;
            for (const auto& r : rasgo::ui::outputPlates(p)) {
                if (!pr) o += ',';
                pr = false;
                numero(o, r.x); o += ','; numero(o, r.y); o += ','; numero(o, r.w); o += ','; numero(o, r.h);
            }
        }
        o += ']';
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
            if (w.kind == Kind::Display && rasgo::ui::roundDisplay(n.type())) o += ",\"rd\":1";
            if (w.kind == Kind::Knob && rasgo::ui::isPrimaryControl(n.type(), w.bind)) o += ",\"pr\":1";
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
                if (porta(n, w.bind, saida, idx, tipo)) {
                    o += ",\"out\":" + std::string(saida ? "1" : "0")
                       + ",\"p\":" + std::to_string(idx)
                       + ",\"pk\":" + std::to_string(tipo);
                    if (saida) {
                        o += ",\"li\":" + std::to_string(g_luzes.size());
                        g_luzes.push_back({id, static_cast<std::size_t>(idx)});
                    }
                }
            }
            o += '}';
        }
        o += "]}";
    }
    o += "]}";
    g_valores.assign(g_controles.size(), 0.0f);
    g_luzPico.assign(g_luzes.size(), 0.0f);
    g_luzLida.assign(g_luzes.size(), 0.0f);
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
    // seed trocada DURANTE a tomada: a partitura registra a nova e os cabos
    if (g_gravando) {
        const double t = recTempo();
        g_score.seedChange(t, seed);
        for (std::size_t i = 0; i < g_grafo->cableCount(); ++i) {
            const auto& c = g_grafo->cable(i);
            g_score.connection(t, c.source().node, c.source().port, c.target().node, c.target().port);
        }
    }
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

// ---- ENTRADAS: microfone e MIDI (`syncSignalIn` do app) ----------------

// O que o patch pede agora: 1 = áudio (um SIGNAL-IN com o ON ligado e alguma
// saída cabeada), 2 = MIDI (o mesmo, ou um POLY-IN). A página só abre o
// microfone e o MIDI quando pedidos — o Rasgo soa sozinho; entrada é
// adaptador opcional, e pedir microfone a quem não vai usar é ruído.
__attribute__((used, visibility("default")))
int rw_entrada_necessaria() {
    if (!g_grafo) return 0;
    int r = 0;
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i) {
        const std::string ty = g_grafo->node(i).type();
        if (ty != "SIGNAL-IN" && ty != "POLY-IN") continue;
        if (parametro(g_grafo->node(i), "listen") < 0.5f) continue;
        bool cabeado = false;
        for (std::size_t c = 0; c < g_grafo->cableCount() && !cabeado; ++c)
            cabeado = g_grafo->cable(c).source().node == i;
        if (!cabeado) continue;
        r |= 2;
        if (ty == "SIGNAL-IN") r |= 1;
    }
    return r;
}

// o microfone: 128 quadros L R intercalados em rw_audio_buffer(), entregues
// a cada SIGNAL-IN antes do bloco (`pushSamples`, como a placa no app)
float g_audioIn[kQuantum * 2] = {};
__attribute__((used, visibility("default")))
float* rw_audio_buffer() { return g_audioIn; }
__attribute__((used, visibility("default")))
void rw_audio_in(int quadros) {
    if (!g_grafo) return;
    const auto n = static_cast<std::size_t>(std::clamp(quadros, 0, static_cast<int>(kQuantum)));
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i)
        if (auto* si = dynamic_cast<rasgo::modular::SignalIn*>(&g_grafo->node(i))) si->pushSamples(g_audioIn, n);
}

// um evento MIDI (bytes crus) para os SIGNAL-IN e POLY-IN (`deliverMidi`)
__attribute__((used, visibility("default")))
void rw_midi(int st, int d1, int d2) {
    if (!g_grafo) return;
    for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i) {
        auto& nd = g_grafo->node(i);
        if (auto* si = dynamic_cast<rasgo::modular::SignalIn*>(&nd))
            si->pushMidi(static_cast<std::uint8_t>(st), static_cast<std::uint8_t>(d1), static_cast<std::uint8_t>(d2), 0.0f);
        else if (auto* pi = dynamic_cast<rasgo::modular::PolyIn*>(&nd))
            pi->pushMidi(static_cast<std::uint8_t>(st), static_cast<std::uint8_t>(d1), static_cast<std::uint8_t>(d2));
    }
}

// ---- REC (`startRec`/`finishRec` do app) -------------------------------

// começa a tomada: a fiação inteira em t = 0, o dicionário dos módulos, a
// seed e o estado do VARIA, como no app
__attribute__((used, visibility("default")))
void rw_rec_iniciar() {
    if (!g_grafo) return;
    g_score.clear();
    g_recFrames = 0;
    for (std::size_t i = 0; i < g_grafo->cableCount(); ++i) {
        const auto& c = g_grafo->cable(i);
        g_score.connection(0.0, c.source().node, c.source().port, c.target().node, c.target().port);
    }
    g_score.setSeed(g_seedAtual);
    g_recSeed = g_seedAtual;
    g_variaLog.clear();
    g_mao.forEachFiber([&](std::size_t, const std::string&, float v, float, float) { g_variaLog.push_back(v); });
    g_variaTick = 0;
    g_variaGen = g_mao.generation();
    g_variaAmtLog = g_mao.intensity();
    g_variaLigadoLog = g_varia;
    {
        char b[64];
        std::snprintf(b, sizeof b, "varia %s intensidade=%.2f", g_varia ? "ligado" : "desligado",
                      static_cast<double>(g_mao.intensity()));
        g_score.instrument(0.0, b);
    }
    g_score.setSampleRate(g_taxa);
    auto descreve = [](std::size_t id, bool comParametros) {
        auto& nd = g_grafo->node(id);
        rasgo::panel::ScoreNodeInfo info;
        info.type = nd.type();
        for (std::size_t p = 0; p < nd.inputCount(); ++p) info.inPorts.push_back(nd.inputDescriptor(p).name);
        for (std::size_t p = 0; p < nd.outputCount(); ++p) info.outPorts.push_back(nd.outputDescriptor(p).name);
        if (comParametros)
            for (const auto& pr : nd.parameters()) info.params.emplace_back(pr.descriptor.id, pr.value);
        g_score.describe(id, std::move(info));
    };
    for (const auto id : g_shown) descreve(id, true);
    descreve(g_saida, false);   // o sink também aparece nas ligações
    g_gravando = true;
}

// a tomada ainda corre? (para sozinha aos 4 min)
__attribute__((used, visibility("default")))
int rw_rec_ativo() { return g_gravando ? 1 : 0; }

// a seed em que a tomada começou (alto/baixo) e se houve troca de seed
// durante ela (o `-montage` do nome do arquivo)
__attribute__((used, visibility("default")))
std::uint32_t rw_rec_seed_alto() { return static_cast<std::uint32_t>(g_recSeed >> 32); }
__attribute__((used, visibility("default")))
std::uint32_t rw_rec_seed_baixo() { return static_cast<std::uint32_t>(g_recSeed & 0xffffffffu); }
__attribute__((used, visibility("default")))
int rw_rec_montagem() {
    for (const auto& e : g_score.events())
        if (e.type == rasgo::panel::RasgoEvent::Type::Seed && e.seed != g_recSeed) return 1;
    return 0;
}

// um gesto do músico num controle (de → para), como `onParamGesture`
__attribute__((used, visibility("default")))
void rw_rec_param(int i, float de, float para) {
    if (!g_gravando || i < 0 || static_cast<std::size_t>(i) >= g_controles.size()) return;
    g_score.parameterChange(recTempo(), g_controles[i].no, g_controles[i].bind, de, para);
}

// para e devolve a partitura (.score.txt); o cabeçalho vem da página em
// rw_entrada: "data\taudio\tinstrumento". As notas do NOTE-OUT saem em
// rw_rec_midi()/rw_rec_midi_tamanho(), no andamento do primeiro CLOCK.
__attribute__((used, visibility("default")))
const char* rw_rec_parar(int tamanho) {
    g_gravando = false;
    std::string cab(g_entrada.data(), static_cast<std::size_t>(std::max(0, std::min<int>(tamanho, static_cast<int>(g_entrada.size())))));
    std::vector<std::string> campos;
    for (std::size_t a = 0, b; ; a = b + 1) {
        b = cab.find('\t', a);
        campos.push_back(cab.substr(a, b == std::string::npos ? std::string::npos : b - a));
        if (b == std::string::npos) break;
    }
    while (campos.size() < 3) campos.emplace_back();
    g_score.setHeader({
        {"data", campos[0]},
        {"audio", campos[1]},
        {"instrumento", campos[2]},
        {"autoria", "instrumento criado por L\xc3\xba" "cio Ara\xc3\xba" "jo "
                    "\xe2\x80\x94 fam\xc3\xad" "lia RASGO, rasgoinstruments.arquiviagem.net"},
        {"licenca", "instrumento sob GNU AGPL-3.0-or-later"},
        {"contato", "rasgo.instruments@gmail.com"},
    });
    g_salvo = g_score.toText();
    double bpm = 120.0;
    if (g_grafo)
        for (std::size_t i = 0; i < g_grafo->nodeCount(); ++i)
            if (g_grafo->node(i).type() == "CLOCK") {
                try { bpm = g_grafo->parameterUserValue(i, "bpm"); } catch (...) {}
                break;
            }
    g_midi = rasgo::panel::scoreToMidi(g_score.events(), bpm);
    return g_salvo.c_str();
}
__attribute__((used, visibility("default")))
const char* rw_rec_midi() { return g_midi.data(); }
__attribute__((used, visibility("default")))
int rw_rec_midi_tamanho() { return static_cast<int>(g_midi.size()); }

// COMPACTO: a vista compacta dos painéis (a mesma regra do app)
__attribute__((used, visibility("default")))
void rw_compacto(int ligado) {
    g_compacto = ligado != 0;
    if (g_grafo) descrever();
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

// ---- a paleta: acrescentar, tirar e reordenar módulos -----------------

void depoisDosCabos();   // mais abaixo, junto dos cabos

// Acrescenta um módulo do tipo (`addModule` do app): respeita as instâncias
// (`canAddInstance`), entra junto da família, um MASTER novo vai direto à
// saída se ela estiver vazia. Devolve o id novo, −1 se o tipo é único no
// rack e já está, −2 se já há o máximo de cópias, −3 se o tipo não existe.
__attribute__((used, visibility("default")))
int rw_adicionar(int tamanho) {
    if (!g_grafo || tamanho <= 0 || static_cast<std::size_t>(tamanho) >= g_entrada.size()) return -3;
    const std::string tipo(g_entrada.data(), static_cast<std::size_t>(tamanho));
    const auto why = rasgo::panel::canAddInstance(*g_grafo, g_shown, tipo);
    if (why == rasgo::panel::AddRefusal::singleton) return -1;
    if (why != rasgo::panel::AddRefusal::none) return -2;
    auto n = rasgo::panel::makeModule(tipo);
    if (!n) return -3;
    foto();
    std::vector<std::size_t> ordem = g_shown;
    const std::size_t id = g_grafo->add(std::move(n));
    const auto at = rasgo::panel::moduleInsertionIndex(
        ordem, tipo, [](std::size_t i) { return g_grafo->node(i).type(); });
    ordem.insert(ordem.begin() + static_cast<std::ptrdiff_t>(at), id);
    if (tipo == "MASTER") {
        bool ligada = false;
        for (std::size_t i = 0; i < g_grafo->cableCount(); ++i)
            ligada |= g_grafo->cable(i).target().node == g_saida;
        if (!ligada) try { g_grafo->connect(id, 0, g_saida, 0); } catch (...) {}
        if (auto* m = dynamic_cast<rasgo::modular::Master*>(&g_grafo->node(id))) {
            g_masters.push_back(m);
            if (g_espera) g_grafo->setParameterBase(id, "mute", 1.0f);
        }
    }
    g_ordem = ordem;
    depoisDosCabos();
    return static_cast<int>(id);
}

// Tira o módulo da case (`removeModule`): desliga só os cabos que encostam
// nele; o nó fica órfão no grafo, sem custo (a saída poda o que não chega)
__attribute__((used, visibility("default")))
int rw_remover(int id, int comFoto) {
    if (!g_grafo || id < 0 || static_cast<std::size_t>(id) >= g_grafo->nodeCount()
        || static_cast<std::size_t>(id) == g_saida) return 0;
    if (comFoto) foto();   // arrastado à paleta: a foto já veio no começo do arrasto
    const auto n = static_cast<std::size_t>(id);
    for (std::size_t i = g_grafo->cableCount(); i-- > 0;) {
        const auto& c = g_grafo->cable(i);
        if (c.source().node == n || c.target().node == n)
            g_grafo->disconnect(c.target().node, c.target().port);
    }
    g_ordem = g_shown;
    g_ordem.erase(std::remove(g_ordem.begin(), g_ordem.end(), n), g_ordem.end());
    g_scopes.erase(n);
    depoisDosCabos();
    return 1;
}

// A ordem nova do rack depois de arrastar um módulo pelo título: os ids
// em texto ("3 0 7 …") em rw_entrada. Os que faltarem continuam no fim.
__attribute__((used, visibility("default")))
int rw_ordem(int tamanho) {
    if (!g_grafo || tamanho <= 0 || static_cast<std::size_t>(tamanho) >= g_entrada.size()) return 0;
    std::istringstream is(std::string(g_entrada.data(), static_cast<std::size_t>(tamanho)));
    std::vector<std::size_t> ordem;
    std::size_t id = 0;
    while (is >> id)
        if (std::find(g_shown.begin(), g_shown.end(), id) != g_shown.end()
            && std::find(ordem.begin(), ordem.end(), id) == ordem.end()) ordem.push_back(id);
    for (const auto s : g_shown)
        if (std::find(ordem.begin(), ordem.end(), s) == ordem.end()) ordem.push_back(s);
    g_ordem = ordem;
    descrever();
    return 1;
}

// o catálogo da paleta (`PaletteColumn::layoutRows`): as famílias na ordem
// do catálogo e, dentro de cada uma, a ordem do rack (alfabética, MIXER e
// MASTER no fim) — [{"familia":…, "tipos":[…]}, …]
__attribute__((used, visibility("default")))
const char* rw_catalogo() {
    g_salvo = "[";
    bool pf = true;
    for (const auto& fam : rasgo::panel::moduleCatalog()) {
        if (!pf) g_salvo += ',';
        pf = false;
        g_salvo += "{\"familia\":";
        json(g_salvo, fam.family);
        g_salvo += ",\"tipos\":[";
        std::vector<const char*> ord(fam.types.begin(), fam.types.end());
        rasgo::panel::sortFamilyForDisplay(ord);
        for (std::size_t i = 0; i < ord.size(); ++i) {
            if (i) g_salvo += ',';
            json(g_salvo, ord[i]);
        }
        g_salvo += "]}";
    }
    g_salvo += ']';
    return g_salvo.c_str();
}

// ---- o banco de escalas (o navegador do QUANTIZER) ---------------------

// o banco inteiro em JSON, para a janela de escalas: famílias na ordem do
// banco e cada escala {id, n(ome), f(amília: índice), p(eríodo), g(raus), s(fonte)}
__attribute__((used, visibility("default")))
const char* rw_banco() {
    const auto& b = rasgo::escalas::BancoEscalas::instancia();
    g_salvo = "{\"familias\":[";
    for (std::size_t i = 0; i < b.familias().size(); ++i) {
        if (i) g_salvo += ',';
        json(g_salvo, b.familias()[i]);
    }
    g_salvo += "],\"escalas\":[";
    bool primeiro = true;
    for (const auto& e : b.escalas()) {
        if (!primeiro) g_salvo += ',';
        primeiro = false;
        std::size_t f = 0;
        while (f < b.familias().size() && b.familias()[f] != e.familia) ++f;
        g_salvo += "{\"id\":" + std::to_string(e.id) + ",\"n\":";
        json(g_salvo, e.nome);
        g_salvo += ",\"f\":" + std::to_string(f) + ",\"p\":";
        numero(g_salvo, e.periodo);
        g_salvo += ",\"g\":[";
        for (std::size_t k = 0; k < e.graus.size(); ++k) {
            if (k) g_salvo += ',';
            numero(g_salvo, e.graus[k]);
        }
        g_salvo += "],\"s\":";
        json(g_salvo, e.fonte);
        g_salvo += '}';
    }
    g_salvo += "]}";
    return g_salvo.c_str();
}

// o QUANTIZER `id` passa a tocar a escala `banco` do banco (SCALE = BANCO)
__attribute__((used, visibility("default")))
void rw_escolher_escala(int id, std::uint32_t banco) {
    if (!g_grafo || id < 0 || static_cast<std::size_t>(id) >= g_grafo->nodeCount()) return;
    auto& n = g_grafo->node(static_cast<std::size_t>(id));
    if (n.type() != "QUANTIZER") return;
    try {
        g_grafo->setParameterBase(static_cast<std::size_t>(id), "bank", static_cast<float>(banco));
        g_grafo->setParameterBase(static_cast<std::size_t>(id), "scale", 13.0f);
    } catch (...) {}
}

// o valor do músico num parâmetro qualquer (o banco atual do QUANTIZER)
__attribute__((used, visibility("default")))
float rw_parametro(int id, int tamanho) {
    if (!g_grafo || id < 0 || static_cast<std::size_t>(id) >= g_grafo->nodeCount()
        || tamanho <= 0 || static_cast<std::size_t>(tamanho) >= g_entrada.size()) return 0.0f;
    try { return g_grafo->parameterUserValue(static_cast<std::size_t>(id),
                                             std::string(g_entrada.data(), static_cast<std::size_t>(tamanho))); }
    catch (...) { return 0.0f; }
}

// a entrada de texto e o patch que toca, para o LEARN (rasgo_learn.cpp,
// compilado à parte)
const std::vector<char>& rw_web_entrada() { return g_entrada; }
SignalGraph* rw_web_grafo() { return g_grafo.get(); }

// ---- os textos da interface (`UiLanguage.hpp`), para os 4 idiomas -------

#define RW_TEXTOS \
    X(hdrVary) \
    X(hdrStandby) \
    X(hdrChange) \
    X(hdrEvolve) \
    X(hdrCross) \
    X(hdrBank) \
    X(hdrSave) \
    X(hdrOpen) \
    X(hdrUndo) \
    X(hdrUncable) \
    X(hdrRestore) \
    X(cableNotAudible) \
    X(hdrRec) \
    X(instanceSingle) \
    X(instanceMax) \
    X(hdrSeed) \
    X(hdrOpens) \
    X(startSeed) \
    X(startUncabled) \
    X(startInit) \
    X(startLast) \
    X(startNoteSeed) \
    X(startNoteUncabled) \
    X(startNoteInit) \
    X(scaleBankTitle) \
    X(scaleBankSearch) \
    X(scaleBankDegrees) \
    X(scaleBankPeriod) \
    X(scaleBankSource) \
    X(scaleBankHint) \
    X(scaleBankAll) \
    X(scaleBankEmpty) \
    X(startNoteLast) \
    X(startNoteSafe) \
    X(hdrRackAll) \
    X(hdrRackOut) \
    X(rackViewEmpty) \
    X(hdrTutorial) \
    X(hdrAbout) \
    X(close) \
    X(inspRupture) \
    X(inspReconnect) \
    X(learnCableHead) \
    X(inspRemove) \
    X(inspFeedback) \
    X(openPatch) \
    X(builtOn) \
    X(rdModules) \
    X(rdCables) \
    X(rdPeak) \
    X(tutTitle) \
    X(tutSubtitle) \
    X(tutWhatTitle) \
    X(tutWhatBody) \
    X(tutSeedTitle) \
    X(tutSeedBody) \
    X(tutSeedBoxTitle) \
    X(tutSeedBoxBody) \
    X(tutVaryTitle) \
    X(tutVaryBody) \
    X(tutStoreTitle) \
    X(tutStoreBody) \
    X(tutRecTitle) \
    X(tutRecBody) \
    X(tutHdrTitle) \
    X(tutHdrBody) \
    X(tutCableTitle) \
    X(tutCableBody) \
    X(tutNavTitle) \
    X(tutNavBody) \
    X(tutModTitle) \
    X(tutModBody) \
    X(tutFamTitle) \
    X(tutFamBody) \
    X(tutScratchTitle) \
    X(tutScratchBody) \
    X(tutKeysTitle) \
    X(tutKeysBody) \
    X(tutLearnTitle) \
    X(tutLearnBody) \
    X(guideUrl) \
    X(aboutTarget) \
    X(learnIdle) \
    X(aboutBody) \
    X(footerCredit)

// Todos os textos da interface do app que a página usa, na língua pedida
// (0 en, 1 pt, 2 fr, 3 es), em JSON {"chave":"texto",…}
__attribute__((used, visibility("default")))
const char* rw_textos(int l) {
    const auto lg = static_cast<rasgo::panel::Lang>(std::clamp(l, 0, 3));
    g_salvo = "{";
    bool pr = true;
#define X(k) \
    if (!pr) g_salvo += ','; \
    pr = false; \
    json(g_salvo, #k); \
    g_salvo += ':'; \
    json(g_salvo, rasgo::panel::tr(rasgo::panel::strings::k, lg));
    RW_TEXTOS
#undef X
    g_salvo += '}';
    return g_salvo.c_str();
}

// a língua dos textos que o motor escreve (o nome da escala vazia do BANCO)
int g_lingua = 1;
__attribute__((used, visibility("default")))
void rw_lingua(int l) { g_lingua = std::clamp(l, 0, 3); }

// a versão do motor (o `git describe` do build, como no SOBRE do app)
#ifndef RASGO_MODULAR_BUILD
#define RASGO_MODULAR_BUILD "dev"
#endif
__attribute__((used, visibility("default")))
const char* rw_versao() { return RASGO_MODULAR_BUILD; }

// o loudness da saída (BS.1770-4) para o SOBRE: momentâneo, curto,
// integrado (LUFS) e o pico verdadeiro (dBTP)
float g_lufsOut[4] = {};
__attribute__((used, visibility("default")))
float* rw_lufs() {
    g_lufsOut[0] = g_lufs.momentary(); g_lufsOut[1] = g_lufs.shortTerm();
    g_lufsOut[2] = g_lufs.integrated(); g_lufsOut[3] = g_lufs.truePeakDbtp();
    return g_lufsOut;
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

// as luzes das saídas: o pico com sinal de cada uma desde a última
// leitura (Float32, na ordem de "li"; quantas em rw_num_luzes), e zera
__attribute__((used, visibility("default")))
int rw_num_luzes() { return static_cast<int>(g_luzes.size()); }

__attribute__((used, visibility("default")))
float* rw_luzes() {
    g_luzLida = g_luzPico;
    std::fill(g_luzPico.begin(), g_luzPico.end(), 0.0f);
    return g_luzLida.data();
}

__attribute__((used, visibility("default")))
float* rw_processar() {
    if (!g_grafo || !g_bloco) {
        for (auto& x : g_pcm) x = 0.0f;
        return g_pcm;
    }
    g_grafo->process(*g_bloco, g_saida, 0);
    alimentarVisores();
    for (std::size_t k = 0; k < g_luzes.size(); ++k)
        if (const AudioBlock* b = g_grafo->lastOutput(g_luzes[k].no, g_luzes[k].porta))
            g_luzPico[k] = rasgo::ui::blockSignedPeak(*b, g_luzPico[k]);
    if (g_gravando) {
        g_recFrames += kQuantum;
        // as notas que os NOTE-OUT fecharam (`collectNotes`)
        const double agora = recTempo();
        for (const auto id : g_shown)
            if (auto* no = dynamic_cast<rasgo::modular::NoteOut*>(&g_grafo->node(id))) {
                rasgo::modular::NoteOut::CompletedNote cn;
                while (no->takeCompletedNote(cn))
                    g_score.note(agora - cn.durationSeconds, id, cn.pitch, cn.velocity,
                                 cn.durationSeconds, cn.accent);
            }
        if (g_recFrames >= static_cast<std::size_t>(kRecMax * g_taxa)) g_gravando = false;   // 4 min
    }
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
        // a mão do VARIA na partitura, 1×/s: cada fibra que andou mais de 2 %
        // do curso desde a última anotação (como no app)
        if (g_gravando && ++g_variaTick >= 30) {
            g_variaTick = 0;
            const double t = recTempo();
            if (g_mao.generation() != g_variaGen) { g_variaGen = g_mao.generation(); g_variaLog.clear(); }
            std::size_t i = 0;
            g_mao.forEachFiber([&](std::size_t node, const std::string& pid, float v, float lo, float hi) {
                if (i >= g_variaLog.size()) { g_variaLog.push_back(v); ++i; return; }
                if (std::fabs(v - g_variaLog[i]) > 0.02f * std::max(1e-4f, hi - lo)) {
                    g_score.parameterChange(t, node, pid, g_variaLog[i], v, /*byVaria=*/true);
                    g_variaLog[i] = v;
                }
                ++i;
            });
        }
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
    if (g_gravando) g_score.connection(recTempo(), s, sp, t, tp);
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
    g_nomeEscala = e ? e->nome : rasgo::panel::tr(rasgo::panel::strings::scaleBankEmpty, static_cast<rasgo::panel::Lang>(g_lingua));
    return g_nomeEscala.c_str();
}

__attribute__((used, visibility("default")))
void rw_mao(int ativa) { g_musico = ativa != 0; }

__attribute__((used, visibility("default")))
void rw_varia(int ligado, float intensidade) {
    if (g_gravando && (ligado != 0) != g_variaLigadoLog) {
        g_variaLigadoLog = ligado != 0;
        g_score.instrument(recTempo(), g_variaLigadoLog ? "varia ligado" : "varia desligado");
    }
    if (g_gravando && std::fabs(intensidade - g_variaAmtLog) >= 0.05f) {
        g_variaAmtLog = intensidade;
        char b[48];
        std::snprintf(b, sizeof b, "varia intensidade=%.2f", static_cast<double>(intensidade));
        g_score.instrument(recTempo(), b);
    }
    g_varia = ligado != 0;
    g_mao.setIntensity(intensidade);
}

}  // extern "C"
