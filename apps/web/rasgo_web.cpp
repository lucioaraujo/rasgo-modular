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
//   rw_cabos_estado()       os cabos agora: 9 números por cabo (Float32)
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
#include "juce/RasgoWordmark.h"   // só o vetor do wordmark, sem JUCE
#include "panel/ModuleCatalog.hpp"
#include "panel/Instances.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"
#include "panel/SinkOut.hpp"

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <string>
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

// a descrição do rack para a página (`rw_descrever`) e os controles que ela
// lê a cada quadro (`rw_valores`), na mesma ordem
std::string g_texto;
struct Controle { std::size_t no; std::string bind; };
std::vector<Controle> g_controles;
std::vector<float> g_valores, g_cabos;

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
    {
        std::vector<std::size_t> primeiro;   // 1ª instância de cada tipo
        for (const auto& fam : rasgo::panel::moduleCatalog()) {
            std::vector<const char*> ord(fam.types.begin(), fam.types.end());
            rasgo::panel::sortFamilyForDisplay(ord);
            for (const char* t : ord)
                for (std::size_t i = 0; i < g.nodeCount(); ++i)
                    if (g.node(i).type() == t) { shown.push_back(i); break; }
        }
        std::vector<char> cabeado(g.nodeCount(), 0);
        for (std::size_t i = 0; i < g.cableCount(); ++i) {
            const auto& c = g.cable(i);
            if (c.source().node < cabeado.size()) cabeado[c.source().node] = 1;
            if (c.target().node < cabeado.size()) cabeado[c.target().node] = 1;
        }
        for (std::size_t id = 0; id < g.nodeCount(); ++id) {
            if (id == g_saida || !cabeado[id]) continue;
            const std::string t = g.node(id).type();
            bool antes = false;
            for (std::size_t j = 0; j < id && !antes; ++j) antes = g.node(j).type() == t;
            if (!antes) continue;
            const auto at = rasgo::panel::moduleInsertionIndex(
                shown, t, [&g](std::size_t i) { return g.node(i).type(); });
            shown.insert(shown.begin() + static_cast<std::ptrdiff_t>(at), id);
        }
    }
    const std::vector<char> chega = g.nodesFeeding(g_saida);
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
    descrever();
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

// por cabo (9 números): origem, porta, destino, porta, rompido, conduzindo
// agora, ganho, tem relação, condutância
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
    }
    return g_cabos.data();
}

__attribute__((used, visibility("default")))
void rw_varia(int ligado, float intensidade) {
    g_varia = ligado != 0;
    g_mao.setIntensity(intensidade);
}

}  // extern "C"
