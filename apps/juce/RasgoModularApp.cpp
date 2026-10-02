// Rasgo Modular — front-end de PRODUÇÃO (JUCE), multiplataforma.
//
// Por que existe: `apps/panel/` é o painel de TESTE — X11 + ALSA, só Linux.
// Publicar exige um caminho de build/execução verificável por plataforma
// (`RASGO_DOCUMENTATION/ESTRATEGIA_DE_PUBLICACAO.md`), e o front-end de
// produção JUCE/web-WASM já estava decidido em `RASGO_MODULAR.md §36.7`
// desde 2026-09-01. Este é o JUCE.
//
// O que ele compartilha com o painel X11 (e por quê):
//  - o motor inteiro (`src/core`, `src/dsp`) — framework-free por projeto;
//  - `src/ui/PanelGeometry.hpp` — a MESMA pegada de widget em mm, pra o
//    clique de um front-end não acertar onde o outro não desenha;
//  - `apps/panel/ModuleCatalog.hpp` e `PatchSeed.hpp` — headers sem
//    dependência de plataforma (o nome `panel/` é histórico; são lógica de
//    aplicação compartilhada, não do painel X11).
//
// O que é dele: o desenho e a entrada. O painel X11 é immediate-mode (um
// `redraw()` que repinta tudo); o `paint()` do JUCE também é — então isto
// é uma transliteração daquele desenho, não um segundo desenho.

#include <juce_audio_utils/juce_audio_utils.h>

#include "panel/assets/rasgo_logo_gray.h"

#include "core/SignalGraph.hpp"
#include "dsp/Loudness.hpp"       // medição BS.1770/EBU R128
#include "dsp/Master.hpp"          // gainReductionDb() — clip-latch do VU
#include "dsp/NoteOut.hpp"        // notas concluídas -> SYSTEM SCORE
#include "dsp/SignalIn.hpp"       // entrada de áudio/MIDI (módulo adaptador)
#include "io/WavWriter.hpp"       // REC: o mesmo .wav do painel X11
#include "panel/ScoreRecorder.hpp"  // SYSTEM SCORE da tomada
#include "panel/SinkOut.hpp"      // sink + guarda de segurança
#include "panel/LearnCatalog.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/MotionEngine.hpp"    // VARIA
#include "panel/PatchGenetics.hpp"   // MUTA / EVOLUI / CRUZA
#include "panel/PatchSeed.hpp"
#include "panel/SeedBalance.hpp"  // equilibra o volume entre seeds
#include "panel/UiLanguage.hpp"
#include "panel/WindowPolicy.hpp"   // mesma política de abertura do painel X11
#include "ui/CableGeometry.hpp"
#include "ui/PanelGeometry.hpp"
#include "ui/ScopeTrace.hpp"
#include "ui/Shortcuts.hpp"        // tabela ÚNICA de atalhos (testada)

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

using namespace rasgo::modular;
namespace ui = rasgo::ui;

// Nó terminal do grafo (o que o áudio lê). Mesmo `Out` do painel X11 e das
// peças de `examples/` — trivial de propósito.
// O sink `OUT` mora em `panel/SinkOut.hpp` — os dois front-ends usam o
// MESMO nó, e é ele que carrega a guarda de segurança da saída.
using Out = rasgo::panel::SinkOut;

// Tokens de cor — os mesmos valores do painel X11 (IDENTIDADE_VISUAL §4:
// cor nomeada por função), pra os dois front-ends serem reconhecivelmente
// o mesmo instrumento.
// ---- texto: SEMPRE por aqui -------------------------------------------
// `juce::String(const char*)` NÃO é UTF-8 — o construtor documenta que
// aceita só ASCII de 7 bits e converte byte a byte por `CharPointer_ASCII`.
// Passar UTF-8 por ele dá mojibake de dupla codificação (verificado nesta
// versão do JUCE: "SAÍDA · relação" vira "SAÃDA Â· relaÃ§Ã£o"). Como
// quase todo texto deste projeto é português com acento — rótulos, LEARN,
// tutorial, créditos —, qualquer construção descuidada aparece na tela.
//
// `u8()` é o único caminho permitido para levar texto do projeto à tela.
// Aceita `const char*` e `std::string`; nunca use `juce::String(...)`
// direto sobre texto que possa ter acento.
inline juce::String u8(const char* s) { return juce::String::fromUTF8(s); }
inline juce::String u8(const std::string& s) {
    return juce::String::fromUTF8(s.c_str(), static_cast<int>(s.size()));
}

struct Tokens {
    juce::Colour bg{0xff131a1a}, surface{0xff262b36}, recessed{0xff0e1015},
        line{0xff485060}, textPrimary{0xffe0e4ec}, textSecondary{0xff8890a0},
        accent{0xffff9d4c}, warning{0xffff6b5b},
        // REC aceso: vermelho de gravação, não o laranja de "ligado" dos
        // outros botões — pedido do autor, 2 out. 2026. O `warning` (coral)
        // ao lado do laranja não chegava a ler como vermelho.
        recording{0xffe0362c};
};
const Tokens T;

// ---- o instrumento: grafo, sink e o patch de partida -------------------
// Espelha a montagem do painel X11 (`panel_main.cpp`, seção "rack cheio"):
// instancia o catálogo inteiro, cria o `Out`, liga a voz mínima e deixa o
// `setActiveOutput` podar o que não chega à saída (os módulos órfãos do
// catálogo não custam DSP por bloco).
struct Rack {
    SignalGraph graph;
    std::mutex gmx;                       // ver a disciplina de lock abaixo
    std::size_t sink = 0;
    std::vector<std::size_t> shown;       // ordem de exibição no rack
    std::map<std::string, std::size_t> byType;

    // Telemetria dos `Display`. `scopes` é escrito pelo THREAD DE ÁUDIO
    // sob `gmx`; `scopeSnap` é a cópia que a UI desenha, tirada sem
    // bloquear (`try_lock`) — se o áudio estiver com o lock, a UI redesenha
    // o quadro anterior e segue. Mesma disciplina do painel X11: a imagem
    // pode atrasar um quadro, o áudio nunca espera a imagem.
    std::map<std::size_t, rasgo::ui::ScopeTrace> scopes;
    std::map<std::size_t, rasgo::ui::ScopeTrace> scopeSnap;

    // Instantâneo dos cabos PARA DESENHO. A UI desenha a partir daqui e
    // NÃO trava o grafo no `paint` — travar 30×/s fazia o thread de áudio
    // perder a corrida pelo `try_lock` o tempo todo, e cada bloco perdido
    // é um bloco reemitido com fade: o resultado eram cliques audíveis.
    // O painel X11 nunca travou o grafo pra desenhar cabo; aqui foi
    // regressão minha.
    struct CableView {
        std::size_t source = 0, sourcePort = 0, target = 0, targetPort = 0;
        bool ruptured = false;
        bool hasRelation = false;
        Relation relation{};
        float amount = 0.0f, conductance = 0.0f;
        std::size_t companionNode = 0, companionPort = 0;
        float gain = 1.0f;
        bool conducting = true;   // o sorteio de condução, neste quadro
        bool feedback = false;    // fecha um laço (atrasado um bloco)
    };
    std::vector<CableView> cableSnap;

    void snapshotCables() {            // chamado sob o try_lock do timer
        cableSnap.clear();
        cableSnap.reserve(graph.cableCount());
        for (std::size_t i = 0; i < graph.cableCount(); ++i) {
            const Cable& c = graph.cable(i);
            CableView v;
            v.source = c.source().node;   v.sourcePort = c.source().port;
            v.target = c.target().node;   v.targetPort = c.target().port;
            v.ruptured = c.state() == CableState::Ruptured;
            v.hasRelation = c.hasRelation();
            v.relation = c.relation();
            v.amount = c.relationAmount();
            v.conductance = c.conductance();
            v.companionNode = c.companion().node;
            v.companionPort = c.companion().port;
            v.gain = c.gain();
            v.conducting = c.conductingNow();
            v.feedback = c.isFeedback();
            cableSnap.push_back(v);
        }
    }

    // Cache de `Panel` por nó. `Signal::panel()` CONSTRÓI a descrição do
    // painel a cada chamada; `relayout()` a pedia para todos os módulos, e
    // `relayout()` roda a cada evento de arrasto de módulo — `mouseDrag`
    // no JUCE dispara por evento do mouse (centenas por segundo, não uma
    // vez por quadro como o laço do X11). Eram dezenas de milhares de
    // construções por segundo: a interface parava de responder.
    std::map<std::size_t, Panel> panelCache;
    const Panel& panelOf(const std::size_t id) {
        const auto it = panelCache.find(id);
        if (it != panelCache.end()) return it->second;
        return panelCache.emplace(id, graph.node(id).panel()).first->second;
    }
    void invalidatePanels() { panelCache.clear(); }

    void allocScopes() {                  // pré-aloca FORA do thread de áudio
        for (const auto id : shown) scopes[id];
    }

    // VARIA — variação ao vivo dos parâmetros. O motor é o mesmo header
    // framework-free do painel X11 (`panel/MotionEngine.hpp`): aqui só
    // mora a fiação.
    rasgo::panel::MotionEngine motion;
    bool motionOn = true;
    std::uint64_t curSeed = 0;            // 0 = patch editado à mão
    std::atomic<float> outRms{0.0f};      // energia p/ o duck do VARIA

    // Últimos parâmetros de `prepare`. Toda mudança de TOPOLOGIA
    // (connect/disconnect) precisa ser seguida de `prepare` — é ele que
    // recalcula a ordem de processamento e os buffers. Sem isso o cabo
    // entra no grafo mas não passa a valer: foi exatamente o bug de "não
    // consigo cabear nem descabear" no front-end JUCE.
    float sampleRate = 48000.0f;
    std::size_t blockFrames = 256;

    // REC — acumula em memória (reserva de ~4 min estéreo) e escreve o
    // .wav ao parar, como o painel X11. O `insert` no thread de áudio é
    // memcpy dentro de capacidade já reservada; auto-para quando a
    // reserva enche, pra nunca realocar em tempo real. Mover pra uma
    // thread escritora continua pendente nos DOIS front-ends.
    // Loudness da SAÍDA (BS.1770-4). Alimentado pelo thread de áudio com
    // o mesmo par que vai pro dispositivo, então mede o que se ouve — não
    // uma estimativa de algum ponto interno. É medidor: não toca no sinal
    // em lugar nenhum, e nada aqui normaliza nada automaticamente
    // (`SAIDA_AUDIO_COMUM.md §3`: loudness informa a decisão).
    LoudnessMeter loudness;
    std::atomic<float> lufsM{LoudnessMeter::kSilence};
    std::atomic<float> lufsS{LoudnessMeter::kSilence};
    std::atomic<float> lufsI{LoudnessMeter::kSilence};
    std::atomic<float> lufsTP{-120.0f};   // true-peak, dBTP

    // BLOCOS FAMINTOS: quantas vezes o thread de áudio não conseguiu o
    // grafo e teve de reemitir o bloco anterior.
    //
    // Existe porque isso acontecia em SILÊNCIO. O autor relatou estalos
    // em 23 set. 2026; o render sem janela não os reproduziu (zero
    // descontinuidades em 60 s, com e sem VARIA), então não são do DSP.
    // Sobrou a suspeita de disputa pelo lock — a interface consome 47% de
    // um núcleo repintando tudo a 30 Hz e toma o `gmx` para desenhar.
    // Reemitir bloco é descontinuidade, e descontinuidade é estalo.
    //
    // Sem contador isso é hipótese; com contador é fato ou não é. O
    // número aparece no cartão SOBRE, ao lado das leituras de loudness,
    // para poder ser comparado com o que o ouvido escuta.
    std::atomic<unsigned> starvedBlocks{0};

    // ---- taps de gravação (`SAIDA_AUDIO_COMUM.md §3/§4`) --------------
    // `post-safety` é o que se ouviu — depois do limitador, do teto, de
    // tudo. `pre-safety` é o mesmo sinal ANTES da proteção de saída.
    // Gravar só o primeiro faz o limitador esconder justamente a dinâmica
    // que se queria examinar; gravar só o segundo mente sobre o que saiu
    // pelos alto-falantes. Por isso os dois são nomeados e escolhíveis, e
    // por isso o padrão é `post` — o que se ouviu é o que se grava, salvo
    // pedido explícito.
    enum class RecTap { post, pre, both };
    RecTap recTap = RecTap::post;

    std::atomic<bool> recording{false};
    std::vector<float> recBuf;       // post-safety
    std::vector<float> recPreBuf;    // pre-safety
    void reserveRec() {
        const auto n = static_cast<std::size_t>(sampleRate) * 2 * 240;
        if (recTap != RecTap::pre)  recBuf.reserve(n);
        if (recTap != RecTap::post) recPreBuf.reserve(n);
    }

    // nó MASTER de onde sai o tap `pre-safety`, achado uma vez só —
    // varrer o grafo comparando strings a cada bloco seria trabalho de
    // thread de áudio pra uma resposta que não muda
    std::size_t masterTapNode = static_cast<std::size_t>(-1);
    void findMasterTap() {
        masterTapNode = static_cast<std::size_t>(-1);
        for (const auto id : shown)
            if (graph.node(id).type() == "MASTER") { masterTapNode = id; return; }
    }

    // SYSTEM SCORE da tomada (§5 do estudo): a topologia no instante em
    // que a gravação começa, mais o que acontece enquanto ela corre. O
    // tempo é SEMPRE amostras já gravadas / taxa — nunca relógio de
    // parede: assim o score é relativo à TOMADA, não ao patch, e um
    // `.wav` e o seu `.score.txt` continuam alinhados mesmo se a interface
    // engasgar. Mesma regra do painel X11.
    rasgo::panel::ScoreRecorder score;
    double recElapsed() const {
        return static_cast<double>(recBuf.size()) / 2.0
             / static_cast<double>(sampleRate);
    }

    // Lê as notas que os módulos NoteOut fecharam neste bloco. Chamado do
    // thread de áudio sob `gmx`, como no painel X11 — `takeCompletedNote`
    // é feito pra ser chamado de FORA do `process()`.
    void collectNotes() {
        if (!recording.load(std::memory_order_relaxed)) return;
        const double now = recElapsed();
        for (const auto id : shown) {
            auto* no = dynamic_cast<rasgo::modular::NoteOut*>(&graph.node(id));
            if (no == nullptr) continue;
            rasgo::modular::NoteOut::CompletedNote cn;
            while (no->takeCompletedNote(cn))
                score.note(now - cn.durationSeconds, id, cn.pitch,
                           cn.velocity, cn.durationSeconds, cn.accent);
        }
    }
    void reprepare() { graph.prepare(sampleRate, 2, blockFrames); }

    void populateMotion() { motion.inhabit(graph, curSeed, shown); }

    // ---- desfazer -----------------------------------------------------
    // Um anel de fotografias do patch. Como o motor já serializa o grafo
    // inteiro em texto (`serialize`/`deserialize`), não é preciso escrever
    // a inversa de cada ação — o que seria a parte cara e a parte que
    // envelhece mal, porque toda ação nova teria que lembrar de escrever a
    // sua. Aqui, qualquer ação destrutiva chama `pushUndo()` antes e
    // pronto.
    //
    // Por que isto importa mais que um botão de limpar: MUTA, EVOLUI,
    // CRUZA e SEED reescrevem o patch inteiro de uma vez. Sem volta, são
    // porta de mão única — e o efeito prático é que o músico deixa de
    // apertá-los, que é o oposto do que este instrumento quer.
    static constexpr std::size_t kUndoDepth = 24;
    std::vector<std::string> undoStack;

    std::string snapshot() const {
        std::string t = graph.serialize();
        t += "seed " + std::to_string(curSeed) + "\n";
        t += "panel shown";
        for (const auto id : shown) t += ' ' + std::to_string(id);
        t += '\n';
        return t;
    }
    // chamar SEM o lock (ele é tomado aqui)
    void pushUndo() {
        std::lock_guard<std::mutex> lk(gmx);
        undoStack.push_back(snapshot());
        if (undoStack.size() > kUndoDepth)
            undoStack.erase(undoStack.begin());
    }
    bool canUndo() const { return !undoStack.empty(); }

    // DESCABEAR — começar o patch do zero, à mão.
    //
    // Tira os CABOS, não os módulos: neste instrumento o rack é o conjunto
    // de módulos disponíveis e o PATCH é o cabeamento. Depois disto o
    // músico continua com um de cada módulo na case, todos mudos, e
    // constrói a peça ligação por ligação — que é o estado que o tutorial
    // sempre descreveu como ponto de partida manual.
    //
    // O Rasgo Modular abre TOCANDO, e isso é identidade: não existe folha
    // em branco por omissão. Mas "não por omissão" é diferente de "não
    // existe" — sem isto, descabear à mão eram dezenas de cliques. Aqui a
    // folha em branco é um ato DELIBERADO, não o estado inicial.
    //
    // Nada é removido do grafo, e `setActiveOutput` poda o que não chega à
    // saída: sem cabos, o rack não custa DSP.
    void clearCables() {
        std::lock_guard<std::mutex> lk(gmx);
        for (std::size_t i = graph.cableCount(); i-- > 0;) {
            const auto& c = graph.cable(i);
            graph.disconnect(c.target().node, c.target().port);
        }
        cableSnap.clear();
        // O seed NÃO é jogado fora. Descabear é uma EDIÇÃO como qualquer
        // outra, e o seed que gerou o patch continua sendo um ponto de
        // retorno válido — aliás o mais útil justamente aqui. Zerá-lo
        // fazia o botão REPOR sumir no instante em que ele mais serviria,
        // e era incoerente com o resto: ligar e cortar cabos à mão nunca
        // zerou o seed. (Achado do autor, 20 set. 2026: pressionou `n` e
        // depois `r`, e o `r` corretamente não fez nada.)
        allRuptured = false;
        reprepare();
        graph.setActiveOutput(sink);
    }

    // [espaço] — rompe/reata TODOS os cabos de uma vez (gesto grande). É
    // ISTO que muda a cor dos cabos (todos viram tracejado de `warning`);
    // o STANDBY nunca fez isso, nem aqui nem no painel X11 — ele silencia
    // a saída com rampa e deixa o patch correndo por baixo.
    bool allRuptured = false;
    void toggleRupture() {
        std::lock_guard<std::mutex> lk(gmx);
        allRuptured = !allRuptured;
        for (std::size_t i = 0; i < graph.cableCount(); ++i) {
            if (allRuptured) graph.cable(i).rupture();
            else             graph.cable(i).reconnect();
        }
    }

    // FREEZE automático de MIXER/MASTER em toda operação genética —
    // mitigação documentada em `PatchGenetics.hpp`: sem ela, uma mutação
    // no ganho de saída troca a variação musical por um susto.
    void freezeMixMaster(std::unordered_set<std::size_t>& frozen) {
        for (std::size_t i = 0; i < graph.nodeCount(); ++i) {
            const std::string t = graph.node(i).type();
            if (t == "MIXER" || t == "MASTER") frozen.insert(i);
        }
    }
    void feedScopes(const std::size_t block) {   // chamado sob `gmx`, no áudio
        const std::size_t stride = std::max<std::size_t>(1, block / 44);
        for (const auto id : shown) {
            if (graph.node(id).outputCount() == 0) continue;
            const AudioBlock* ob = graph.lastOutput(id, 0);
            if (!ob) continue;
            const auto it = scopes.find(id);
            if (it == scopes.end()) continue;
            for (std::size_t i = 0; i < ob->frames(); i += stride)
                it->second.push(ob->at(0, i));
            if (graph.node(id).outputCount() >= 4
                && graph.node(id).type() == "TRIGSEQ") {
                const AudioBlock* o1 = graph.lastOutput(id, 1);
                const AudioBlock* o2 = graph.lastOutput(id, 2);
                const AudioBlock* o3 = graph.lastOutput(id, 3);
                if (o1 && o2 && o3)
                    for (std::size_t i = 0; i < ob->frames(); i += stride)
                        it->second.pushLanes(ob->at(0, i), o1->at(0, i),
                                             o2->at(0, i), o3->at(0, i));
            }
        }
    }

    void build() {
        for (const auto& fam : rasgo::panel::moduleCatalog())
            for (const char* t : fam.types) {
                auto n = rasgo::panel::makeModule(t);
                if (!n) continue;
                byType[t] = graph.add(std::move(n));
            }
        for (const auto& fam : rasgo::panel::moduleCatalog()) {
            std::vector<const char*> ordered(fam.types.begin(), fam.types.end());
            rasgo::panel::sortFamilyForDisplay(ordered);
            for (const char* t : ordered) {
                const auto it = byType.find(t);
                if (it != byType.end()) shown.push_back(it->second);
            }
        }
        sink = graph.add(std::make_unique<Out>());

        // voz mínima ligada — o instrumento SOA ao abrir (identidade do
        // Rasgo Modular: não existe folha em branco silenciosa)
        auto at = [&](const char* t) { return byType.at(t); };
        graph.node(at("CLOCK")).setParameter("bpm", 96.0f);
        graph.node(at("CLOCK")).setParameter("mult", 2.0f);
        graph.node(at("CLOCK")).setParameter("fill", 7.0f);
        graph.node(at("CLOCK")).setParameter("drift", 0.25f);
        graph.node(at("OSC")).setParameter("freq", 110.0f);
        graph.node(at("OSC")).setParameter("drift", 0.12f);
        graph.node(at("FILTER")).setParameter("cutoff", 420.0f);
        graph.node(at("FILTER")).setParameter("resonance", 0.35f);
        graph.node(at("ENVELOPE")).setParameter("mode", 1.0f);
        graph.node(at("ENVELOPE")).setParameter("attack", 0.006f);
        graph.node(at("ENVELOPE")).setParameter("decay", 0.30f);
        graph.connect(at("CLOCK"), 1, at("ENVELOPE"), 1);
        graph.connect(at("OSC"), 2, at("FILTER"), 0);
        graph.connect(at("FILTER"), 3, at("ENVELOPE"), 0);
        graph.connect(at("ENVELOPE"), 0, at("MIXER"), 0);
        graph.connect(at("MIXER"), 0, at("MASTER"), 0);
        graph.connect(at("MASTER"), 0, sink, 0);
        graph.connect(at("ENVELOPE"), 1, at("FILTER"), 1, /*feedback=*/true);
        graph.setActiveOutput(sink);
    }

    // Mesma sequência do painel X11 (`applySeed`): semeia, reprepara e
    // reancora a saída ativa. `seedPatch` refaz o patch inteiro sobre o
    // grafo já montado — é seguro chamar de novo.
    // Os parâmetros NÃO sombreiam mais os membros `sampleRate`/
    // `blockFrames`: preparar com um valor e deixar o membro com outro
    // faria o próximo `reprepare()` (todo connect/disconnect passa por
    // ele) reconfigurar o grafo com uma taxa diferente da do dispositivo.
    void applySeed(std::uint64_t s, double sr, std::size_t block) {
        std::lock_guard<std::mutex> lk(gmx);
        sampleRate = static_cast<float>(sr);
        blockFrames = block;
        try {
            rasgo::panel::seedPatch(graph, s);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[seed %llu] %s — voltando ao seed 1\n",
                         static_cast<unsigned long long>(s), e.what());
            rasgo::panel::seedPatch(graph, 1);
        }
        invalidatePanels();
        reprepare();
        graph.setActiveOutput(sink);

        // EQUILÍBRIO DE VOLUME entre seeds. Achado da sessão de escuta do
        // autor (23 set. 2026): seeds abrindo tão fracos que era preciso
        // subir as caixas para perceber que havia som. Medido em 40
        // seeds: 52,7 LU de dispersão. Um instrumento cuja identidade é
        // SOAR AO ABRIR não pode abrir inaudível.
        //
        // Roda UMA VEZ, aqui, quando o patch nasce — não é normalização
        // ao vivo (ver a explicação longa em `SeedBalance.hpp`), e é
        // determinística: o mesmo seed dá o mesmo ganho.
        rasgo::panel::balanceSeedLevel(graph, s, sampleRate, blockFrames);
    }
};

// ---- coluna da esquerda: paleta por família + caixa LEARN --------------
// Mesmas proporções do painel X11 (`kPaletteW`/`kLearnH`): a lista de
// módulos agrupada por família em cima, e a caixa LEARN sempre presente
// no rodapé — passar o mouse num módulo escreve ali o que ele é. A caixa
// é silenciosa por decisão de projeto (não pula, não pisca).
// ---- cor de família -------------------------------------------------
// Sugestão de um usuário do r/modular (1 out. 2026): dentro do rack não
// dava para saber de relance a que família um módulo pertence. Uma FAIXA
// fina na cor da família no topo de cada módulo, e o mesmo tom num
// quadrado ao lado do nome da família na paleta, que vira a legenda.
// Escolha do autor em 2 out. 2026 entre faixa e fundo tingido, pelas
// capturas do mesmo patch (o tingido está no branch
// `experimento-cor-familia`).
// Tons APAGADOS (saturação baixa) e espaçados no círculo cromático: a cor
// forte da tela é dos cabos (quentes = áudio, frios = controle), e a
// família não pode competir com ela nem ser confundida com tipo de sinal.
inline std::string familyOfType(const std::string& type) {
    for (const auto& grp : rasgo::panel::moduleCatalog())
        for (const char* t : grp.types)
            if (type == t) return grp.family;
    return {};
}
inline juce::Colour familyColour(const std::string& family) {
    static const std::map<std::string, float> hue = {
        {"SOURCE", 90.0f},   {"TRANSFORM", 285.0f}, {"MODULATE", 325.0f},
        {"TIME", 150.0f},    {"DECISION", 52.0f},   {"ROUTE", 195.0f},
        {"SPACE", 245.0f},   {"OUT", 0.0f}};
    const auto it = hue.find(family);
    if (it == hue.end()) return T.line;
    // OUT quase neutro: é a família que fecha o patch, e o MASTER já
    // carrega a cor mais forte do rack (o VU)
    const float sat = family == "OUT" ? 0.12f : 0.34f;
    return juce::Colour::fromHSL(it->second / 360.0f, sat, 0.56f, 1.0f);
}

class PaletteColumn : public juce::Component {
public:
    // A coluna cresceu de 158 pra 186 px e a caixa LEARN de 172 pra 232:
    // o LEARN é TEXTO DE LEITURA, não rótulo, e a 10 px numa coluna
    // estreita ficava ilegível (relato do autor). Com corpo em 12 px, a
    // largura antiga daria ~4 palavras por linha — aumentar a fonte sem
    // aumentar a coluna só trocaria letra pequena por texto picotado.
    static constexpr int kWidth = 186;
    static constexpr int kLearnH = 232;
    static constexpr float kListPt  = 11.0f;   // lista de módulos
    static constexpr float kLearnPt = 12.0f;   // corpo do LEARN

    PaletteColumn() { setOpaque(true); }

    void setLanguage(rasgo::panel::Lang l) { lang_ = l; repaint(); }

    // Layout da lista em coordenadas de CONTEÚDO (sem rolagem). Separado
    // da pintura de propósito: quando eu media a altura dentro do laço de
    // desenho, o `break` da borda visível interrompia a contagem — a
    // altura do conteúdo nunca passava da viewport e a roda não tinha
    // para onde rolar. Medida não pode depender do recorte que ela mesma
    // determina (é o mesmo erro que travou a rolagem do rack).
    void layoutRows() {
        rows_.clear();
        int y = 10;
        for (const auto& fam : rasgo::panel::moduleCatalog()) {
            heads_.push_back({fam.family, y});
            y += 18;
            // MESMA ordem do painel X11: alfabética dentro da família, com
            // MIXER/MASTER no fim (`sortFamilyForDisplay`). A lista estava
            // saindo na ordem crua do catálogo — que é a ordem de
            // instanciação dos nós, não de leitura.
            std::vector<const char*> ordered(fam.types.begin(), fam.types.end());
            rasgo::panel::sortFamilyForDisplay(ordered);
            for (const char* t : ordered) {
                rows_.push_back({t, {0, y, kWidth, 14}});
                y += 15;
            }
            y += 4;
        }
        contentH_ = y;
    }

    void resized() override { clampScroll(); }

    void paint(juce::Graphics& g) override {
        g.fillAll(T.bg);
        const int learnTop = getHeight() - kLearnH;
        if (rows_.empty()) { heads_.clear(); layoutRows(); }

        g.setFont(juce::FontOptions(kListPt));
        for (const auto& h : heads_) {
            const int y = h.second - scroll_;
            if (y < -14 || y > learnTop) continue;
            g.setColour(familyColour(h.first));   // legenda da faixa
            g.fillRect(10, y + 3, 8, 8);
            g.setColour(T.accent);
            g.drawText(u8(h.first), 22, y, kWidth - 32, 14,
                       juce::Justification::centredLeft, false);
        }
        for (const auto& r : rows_) {
            const int y = r.bounds.getY() - scroll_;
            if (y < -14 || y > learnTop) continue;
            g.setColour(hover_ == r.type ? T.accent : T.textSecondary);
            g.drawText(u8(r.type), 18, y, kWidth - 26, 14,
                       juce::Justification::centredLeft, false);
        }

        // barra de rolagem: só existe quando há o que rolar, e é
        // arrastável — a roda sozinha não se anuncia, e o autor não tinha
        // como adivinhar que a lista continuava
        if (const int over = maxScroll(); over > 0) {
            const int trackH = learnTop;
            const int thumbH = std::max(24, trackH * trackH / (trackH + over));
            const int thumbY = (trackH - thumbH) * scroll_ / over;
            g.setColour(T.recessed);
            g.fillRect(kWidth - 7, 0, 7, trackH);
            g.setColour(T.line);
            g.fillRect(kWidth - 6, thumbY, 5, thumbH);
        }

        // caixa LEARN
        g.setColour(T.recessed);
        g.fillRect(0, learnTop, kWidth, kLearnH);
        g.setColour(T.line);
        g.drawHorizontalLine(learnTop, 0.0f, static_cast<float>(kWidth));
        g.setColour(T.accent);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText("LEARN", 8, learnTop + 6, kWidth - 16, 12,
                   juce::Justification::centredLeft, false);

        // O conteúdo é o MESMO do painel X11: título, depois `quick`
        // (destaque), `understand` (secundário) e `explore` (acento, com
        // seta). Antes eu mostrava só o `quick` e deixava a caixa vazia
        // sem hover, alegando que ela era "silenciosa por decisão de
        // projeto" — estava errado: o painel X11 tem uma dica explícita
        // ali, e mostra os três segmentos.
        const int tx = 8;
        int ty = learnTop + 22;
        const int wrapW = kWidth - 20;
        const int bottom = getHeight() - 6;
        g.setFont(juce::FontOptions(kLearnPt));

        if (noticeActive()) {
            g.setColour(T.accent);
            g.drawFittedText(notice_, tx, ty, wrapW, kLearnH - 40,
                             juce::Justification::topLeft, 8);
            return;
        }

        if (learn_ == nullptr) {
            g.setColour(T.textSecondary);
            g.drawFittedText(str(rasgo::panel::strings::learnIdle),
                             tx, ty, wrapW, 56,
                             juce::Justification::topLeft, 5);
            return;
        }

        g.setColour(T.accent);
        g.setFont(juce::FontOptions(kLearnPt, juce::Font::bold));
        g.drawText(learnTitle_, tx, ty, wrapW, 15,
                   juce::Justification::topLeft, false);
        g.setFont(juce::FontOptions(kLearnPt));
        ty += 19;

        const struct { const std::string& s; juce::Colour c; const char* pre; }
        segs[] = {
            {learn_->quick,      T.textPrimary,   ""},
            {learn_->understand, T.textSecondary, ""},
            {learn_->explore,    T.accent,        "\xe2\x86\x92 "},
        };
        for (const auto& seg : segs) {
            if (seg.s.empty() || ty > bottom) continue;
            const juce::String txt = u8(seg.pre) + u8(seg.s);
            juce::AttributedString as;
            as.append(txt, juce::FontOptions(kLearnPt), seg.c);
            juce::TextLayout tl;
            tl.createLayout(as, static_cast<float>(wrapW));
            const int h = std::min(static_cast<int>(tl.getHeight()) + 2,
                                   bottom - ty);
            if (h <= 0) break;
            g.setColour(seg.c);
            g.drawFittedText(txt, tx, ty, wrapW, h,
                             juce::Justification::topLeft, 12);
            ty += h + 7;
        }
    }

    // dwell de 1 s antes de trocar o conteúdo (senão pisca a cada
    // movimento do mouse), e fora de qualquer objeto MANTÉM o último —
    // exatamente a regra do painel X11.
    // Aviso momentâneo, por cima do LEARN. Some sozinho — é retorno de
    // uma ação, não conteúdo; ficar fixo competiria com o LEARN, que é
    // quem mora aqui.
    void setNotice(const juce::String& text) {
        notice_ = text;
        noticeUntil_ = juce::Time::getMillisecondCounter() + 6000;
        repaint();
    }
    bool noticeActive() const {
        return notice_.isNotEmpty()
            && juce::Time::getMillisecondCounter() < noticeUntil_;
    }

    void setLearn(const rasgo::panel::LearnEntry* e, const juce::String& title) {
        if (e == learn_) return;
        learn_ = e;
        learnTitle_ = title;
        repaint();
    }

    void mouseMove(const juce::MouseEvent& e) override {
        std::string hit;
        for (const auto& r : rows_)
            if (r.bounds.translated(0, -scroll_).contains(e.getPosition())) {
                hit = r.type;
                break;
            }
        if (hit != hover_) { hover_ = hit; repaint(); }
    }
    void mouseExit(const juce::MouseEvent&) override {
        if (!hover_.empty()) { hover_.clear(); repaint(); }
    }
    // arrastar uma linha da paleta pra a case adiciona o módulo — quem
    // sabe onde é "a case" é o MainComponent (a paleta é irmã do rack)
    std::function<void(const std::string&, juce::Point<int>)> onSpawnDrag;
    std::function<void(const std::string&, juce::Point<int>)> onSpawnDrop;

    void mouseDown(const juce::MouseEvent& e) override {
        barDrag_ = maxScroll() > 0 && e.getPosition().x >= kWidth - 8
                   && e.getPosition().y < getHeight() - kLearnH;
        if (barDrag_) { dragFrom_ = {e.getPosition().y, scroll_}; return; }
        spawn_.clear();
        for (const auto& r : rows_)
            if (r.bounds.translated(0, -scroll_).contains(e.getPosition())) {
                spawn_ = r.type;
                break;
            }
    }
    void mouseUp(const juce::MouseEvent& e) override {
        barDrag_ = false;
        if (!spawn_.empty() && onSpawnDrop) onSpawnDrop(spawn_, e.getScreenPosition());
        spawn_.clear();
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (!barDrag_) {
            if (!spawn_.empty() && onSpawnDrag)
                onSpawnDrag(spawn_, e.getScreenPosition());
            return;
        }
        const int trackH = getHeight() - kLearnH;
        const int over = maxScroll();
        if (trackH <= 0 || over <= 0) return;
        scroll_ = juce::jlimit(0, over, dragFrom_.second
            + (e.getPosition().y - dragFrom_.first) * (trackH + over) / trackH);
        repaint();
    }
    void mouseWheelMove(const juce::MouseEvent&,
                        const juce::MouseWheelDetails& w) override {
        scroll_ = juce::jlimit(0, maxScroll(),
                               scroll_ - juce::roundToInt(w.deltaY * 60.0f));
        repaint();
    }

    const std::string& hoveredType() const { return hover_; }

private:
    struct Row { std::string type; juce::Rectangle<int> bounds; };

    juce::String str(const rasgo::panel::L4& s) const {
        return u8(rasgo::panel::tr(s, lang_));
    }

    int maxScroll() const {
        return std::max(0, contentH_ - (getHeight() - kLearnH) + 10);
    }
    void clampScroll() { scroll_ = juce::jlimit(0, maxScroll(), scroll_); }

    std::vector<Row> rows_;
    std::vector<std::pair<std::string, int>> heads_;   // família, y
    std::string hover_;
    const rasgo::panel::LearnEntry* learn_ = nullptr;
    juce::String learnTitle_;
    juce::String notice_;
    std::uint32_t noticeUntil_ = 0;
    int scroll_ = 0, contentH_ = 0;
    bool barDrag_ = false;
    std::string spawn_;
    std::pair<int, int> dragFrom_{0, 0};               // mouseY, scroll
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
};

// ---- cabeçalho de linha única ------------------------------------------
// Mesmo idioma do painel X11: retângulo + rótulo, empilhado numa lista de
// hits, despachado no clique. Só entram botões cujo recurso EXISTE aqui —
// botão morto (REC, SALVAR, MUTA/EVOLUI/CRUZA, TUTORIAL) é pior que botão
// ausente; eles voltam junto com o recurso.
class HeaderBar : public juce::Component {
public:
    static constexpr int kHeight = 46;        // uma fileira
    static constexpr int kRow2 = 28;         // acréscimo da segunda

    // Altura necessária pra NENHUM botão sumir. A barra de comandos
    // cortava o excedente em silêncio (`break` no laço) — e quem sumia
    // primeiro era o último da fila, que por acaso era o DESCABEIA. O
    // autor apertou `n`, nada acendeu, e concluiu que o atalho estava
    // quebrado: ele funcionava, o botão é que não estava lá.
    //
    // Botão que some sem avisar já tinha me mordido antes (o VU e a
    // leitura, que ganharam guarda). Em vez de recuperar folga — que a
    // próxima palavra longa ou o próximo botão consome de novo —, o
    // cabeçalho passa a QUEBRAR EM DUAS FILEIRAS quando não cabe.
    int preferredHeight(const int width) const {
        return commandsFit(width) ? kHeight : kHeight + kRow2;
    }

    std::function<void()> onSeed, onLang, onRackView, onStandby,
        onVary, onMutate, onEvolve, onCross, onBank, onSave,
        onTutorial, onAbout, onRec, onOpen, onUndo, onUncable, onRestore;
    std::function<void(int)> onZoom;

    // INTENSIDADE da mão do VARIA (0..2; 1,0 é o comportamento histórico).
    // Pedido do autor na escuta de 23 set. 2026, e ele mesmo sugeriu que
    // fosse um slider em vez de um knob escondido — sugestão melhor, pela
    // razão que este projeto acabou de aprender caro: controle que não se
    // vê é controle que ninguém usa.
    std::function<void(float)> onVaryAmount;
    void setVaryAmount(const float v) { varyAmount_ = v; repaint(); }
    float varyAmount() const noexcept { return varyAmount_; }

    std::function<void(std::uint64_t)> onSeedTyped;

    HeaderBar() {
        setOpaque(true);
        addAndMakeVisible(seedBox_);
        seedBox_.setJustification(juce::Justification::centredLeft);
        // 20 dígitos: é o comprimento de `uint64` máximo
        // (18446744073709551615)
        seedBox_.setInputRestrictions(20, "0123456789");
        seedBox_.setColour(juce::TextEditor::backgroundColourId, T.recessed);
        seedBox_.setColour(juce::TextEditor::outlineColourId, T.line);
        seedBox_.setColour(juce::TextEditor::focusedOutlineColourId, T.accent);
        seedBox_.setColour(juce::TextEditor::textColourId, T.textPrimary);
        seedBox_.setColour(juce::TextEditor::highlightColourId, T.accent);
        seedBox_.setColour(juce::TextEditor::highlightedTextColourId, T.bg);
        seedBox_.setColour(juce::CaretComponent::caretColourId, T.accent);
        seedBox_.setFont(juce::FontOptions(11.0f));
        // Enter aplica; perder o foco sem confirmar volta ao seed atual —
        // digitar um número e clicar fora não deve trocar o patch sem
        // querer.
        seedBox_.onReturnKey = [this] {
            // `getLargeIntValue()` devolve int64: não representa seed
            // acima de 2^63−1, que é metade da faixa. `strtoull` lê os 20
            // dígitos inteiros.
            const auto txt = seedBox_.getText().trim();
            if (txt.isEmpty()) return;
            errno = 0;
            const unsigned long long v =
                std::strtoull(txt.toRawUTF8(), nullptr, 10);
            if (errno == ERANGE || v == 0) { syncSeedText(); return; }
            if (onSeedTyped) onSeedTyped(static_cast<std::uint64_t>(v));
        };
        seedBox_.onFocusLost = [this] { syncSeedText(); };
        seedBox_.onEscapeKey = [this] {
            syncSeedText();
            if (auto* p = getParentComponent()) p->grabKeyboardFocus();
        };
        // A marca vem do MESMO mapa de cobertura que o painel X11 usa
        // (`assets/rasgo_logo_gray.h`, 118×15, gerado por `regen_logo.sh`
        // e commitado): 0 = fundo, 255 = marca cheia, com anti-aliasing.
        // Misturado de `bg` até `accent` — a mesma rampa do X11 — pra a
        // marca sair IDÊNTICA nos dois front-ends. Renderizar o SVG cru
        // aqui dava uma marca visivelmente diferente da do painel.
        logo_ = juce::Image(juce::Image::ARGB, rasgo_logo_w, rasgo_logo_h, true);
        juce::Image::BitmapData px(logo_, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < rasgo_logo_h; ++y)
            for (int x = 0; x < rasgo_logo_w; ++x) {
                const float t = static_cast<float>(
                    rasgo_logo_gray[y * rasgo_logo_w + x]) / 255.0f;
                px.setPixelColour(x, y, T.bg.interpolatedWith(T.accent, t));
            }
    }

    void setState(std::uint64_t seed, rasgo::panel::Lang lang, bool rackOutputOnly,
                  bool standby) {
        seed_ = seed; lang_ = lang; rackOut_ = rackOutputOnly; standby_ = standby;
        syncSeedText();
        repaint();
    }
    // Quem está com o teclado: o MainComponent precisa saber pra não
    // roubar o foco de quem está digitando um seed.
    bool seedBoxFocused() const { return seedBox_.hasKeyboardFocus(true); }

    // leitura viva: contagem de módulos/cabos, VU do MASTER e VARIA
    void setReadout(std::size_t visMods, std::size_t totalMods,
                    std::size_t cables, float vu, bool vary, bool rec) {
        visMods_ = visMods; totalMods_ = totalMods; cables_ = cables;
        vu_ = vu; vary_ = vary; recording_ = rec;
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(T.bg);
        g.setColour(T.line);
        g.drawHorizontalLine(getHeight() - 1, 0.0f,
                             static_cast<float>(getWidth()));
        hits_.clear();

        // marca + "MODULAR", como no X11: logo em tamanho natural (118×15)
        // centrado na faixa de botões
        int x = 14;
        g.drawImageAt(logo_, x, 10 + (22 - rasgo_logo_h) / 2);
        x += rasgo_logo_w + 7;
        g.setColour(T.textSecondary);
        g.setFont(juce::FontOptions(11.0f));
        g.drawText("MODULAR", x, 12, 70, 22, juce::Justification::centredLeft, false);
        x += 82;

        g.setColour(T.line);
        g.drawVerticalLine(x - 10, 10.0f, static_cast<float>(kHeight - 10));

        // A caixa do seed é um `juce::TextEditor` de verdade (filho deste
        // componente), não texto desenhado: assim vêm de graça cursor,
        // seleção, teclado e área de transferência — que no painel X11
        // custaram ~17 blocos de código escritos na mão. Ele é
        // reposicionado aqui porque o resto do cabeçalho é immediate-mode
        // e a posição depende da largura da marca.
        seedBox_.setBounds(x, 12, seedBoxW(), 22);
        x += seedBoxW() + 6;

        x += button(g, x, u8("\xE2\x9A\x84 ")
                    + str(rasgo::panel::strings::hdrSeed),
                    flashing(Act::seed), Act::seed);
        // REPOR só existe quando há um seed pra onde voltar. Num patch
        // construído à mão (seed 0) ele seria um botão morto — e botão
        // morto é pior que botão ausente.
        if (seed_ != 0)
            x += button(g, x, str(rasgo::panel::strings::hdrRestore),
                        flashing(Act::restore), Act::restore);

        // Largura que a barra de comandos VAI precisar, medida antes de
        // qualquer coisa opcional ser desenhada. Sem isto o VU e a leitura
        // comiam o vão primeiro (eles são desenhados antes) e a barra era
        // cortada depois, em silêncio — que foi exatamente o que escondeu
        // o botão DESCABEIA e fez o `n` parecer quebrado. A guarda que eu
        // tinha posto media o espaço no ponto errado do desenho.
        {
            namespace S2 = rasgo::panel::strings;
            cmdsW_ = 0;
            for (const auto* l : {&S2::hdrChange, &S2::hdrEvolve,
                                  &S2::hdrCross, &S2::hdrBank, &S2::hdrSave,
                                  &S2::hdrOpen, &S2::hdrUndo, &S2::hdrUncable})
                cmdsW_ += labelW(str(*l)) + 6;
            cmdsW_ += 38 + kVaryW + 6;   // rótulo VARIA + slider
            cmdsW_ += 50 + labelW(u8("\xe2\x88\x92")) + labelW("+") + 6;
            cmdsW_ += labelW(str(S2::hdrStandby)) + 6;
            cmdsW_ += labelW(rackLabel()) + 6;      // RACK, junto do ZOOM
            cmdsW_ += 4 * kGroupSepW;               // réguas entre grupos
            // A fileira é decidida AQUI, antes de qualquer coisa que a
            // consulte. Decidir depois deixava as guardas do VU e da
            // leitura lendo o valor do quadro ANTERIOR — um atraso de um
            // quadro que pisca na troca de tamanho da janela.
            cmdY_ = commandsFit(getWidth()) ? 12 : kHeight - 6;
        }

        // ---- cluster da direita, montado da borda pra dentro -----------
        btnY_ = 12;
        int rx = getWidth() - 12;
        rx -= buttonR(g, rx, str(rasgo::panel::strings::hdrAbout),
                      overlay_ == 2, Act::about);
        rx -= buttonR(g, rx, str(rasgo::panel::strings::hdrTutorial),
                      overlay_ == 1, Act::tutorial);
        rx -= buttonR(g, rx, langLabel(), false, Act::lang);
        // REC — ponto cheio quando gravando, como no painel X11
        rx -= buttonR(g, rx, u8("\xe2\x97\x8f ")
                      + str(rasgo::panel::strings::hdrRec),
                      recording_ || flashing(Act::rec), Act::rec,
                      recording_ ? T.recording : T.accent);
        // (o RACK saiu daqui em 2 out. 2026: foi para junto do ZOOM, no
        // grupo de visualização — ver o fim de `paint`)

        // VU do MASTER — a mesma leitura que o painel X11 põe no
        // cabeçalho: dá pra ver que o instrumento está soando sem ter que
        // achar o módulo MASTER no meio do rack.
        // O VU e a leitura são EXTRAS: só entram se sobrar espaço. Os
        // botões de comando (VARIA/MUTA/…) vêm depois no desenho, e sem
        // esta guarda uma janela estreita os empurrava pra fora em
        // silêncio — botão que some não é botão discreto, é botão que o
        // músico procura e não acha.
        if (rx - x - (cmdY_ == 12 ? cmdsW_ : 0) > 54 + 24) {
            const int mw = 54, mh = 8;
            const int my = 19;
            rx -= mw;
            g.setColour(T.recessed);
            g.fillRect(rx, my, mw, mh);
            const float frac = juce::jlimit(0.0f, 1.0f, vu_ / 0.891f);
            if (frac > 0.001f) {
                g.setColour(frac < 0.85f ? T.accent : T.warning);
                g.fillRect(rx, my, static_cast<int>(frac * mw), mh);
            }
            rx -= 12;
        }

        // leitura N mód · M cabos (na vista filtrada, visíveis/total)
        {
            const juce::String mods = rackOut_
                ? juce::String(static_cast<int>(visMods_)) + "/"
                  + juce::String(static_cast<int>(totalMods_))
                : juce::String(static_cast<int>(visMods_));
            const juce::String rd = mods + " "
                + str(rasgo::panel::strings::rdModules)
                + u8("  \xc2\xb7  ")
                + juce::String(static_cast<int>(cables_)) + " "
                + str(rasgo::panel::strings::rdCables);
            g.setFont(juce::FontOptions(11.0f));
            const int w = textW(g, rd);
            if (rx - x - (cmdY_ == 12 ? cmdsW_ : 0) > w + 24) {
                rx -= w;
                g.setColour(T.textSecondary);
                g.drawText(rd, rx, 12, w, 22,
                           juce::Justification::centredLeft, false);
                rx -= 12;
            }
        }

        // ---- barra de comandos ao centro (enche o vão; corta à direita)
        // Os recursos por trás destes botões JÁ EXISTIAM em headers
        // framework-free (`MotionEngine.hpp`, `PatchGenetics.hpp`,
        // `serialize()` do motor) — eram fiação, não porte. Por isso
        // entram todos de uma vez.
        namespace S = rasgo::panel::strings;
        // Segunda fileira (decidida acima): recomeça da margem esquerda,
        // com a largura toda disponível. Nada é cortado.
        if (cmdY_ != 12) { x = 14; rx = getWidth() - 12; }
        // VARIA não é botão: é o SLIDER desenhado logo abaixo, e ZERO
        // significa desligado.
        //
        // Sugestão do autor em 25 set. 2026, apontando o precedente do
        // ANTITOTEM: "há sliders que se desligam quando estão zerados,
        // isso elimina a necessidade de botão + slider". O precedente está
        // documentado lá como vocabulário DELE — "0 = off entirely", usado
        // em `excitationAmount`, `grooveAmount` e `metaSequencerAmount`.
        //
        // Ganha três coisas: um controle em vez de dois para um conceito
        // só; consistência com o irmão da família; e espaço no cabeçalho,
        // que já estourou quatro vezes nesta semana. O estado ligado/
        // desligado deixa de ser um dado separado que podia divergir do
        // valor — 0 é a ÚNICA fonte da verdade.
        const struct { const rasgo::panel::L4* label; bool on; Act act; } cmds[] = {
            {&S::hdrChange, false, Act::mutate},
            {&S::hdrEvolve, false, Act::evolve},
            {&S::hdrCross,  false, Act::cross},
            {&S::hdrBank,   false, Act::bank},
            {&S::hdrSave,   false, Act::save},
            {&S::hdrOpen,   false, Act::open},
            // DESFAZ e DESCABEIA existiam só no teclado — e o autor não os
            // encontrou, o que é o mesmo que não existirem. Recurso sem
            // porta de entrada visível é recurso que ninguém usa.
            {&S::hdrUndo,    false, Act::undo},
            {&S::hdrUncable, false, Act::uncable},
        };
        g.setFont(juce::FontOptions(11.0f));
        btnY_ = cmdY_;

        // VARIA: rótulo + slider, no lugar onde o botão ficava. Zero à
        // esquerda é desligado; a marca do meio é o 1,0, o comportamento
        // histórico. Sem isto não há como saber onde era o "normal".
        // GRUPOS separados por uma régua fina (pedido do autor, 2 out.
        // 2026): [seed · SEED · REPOR] | [VARIA · MUDA · EVOLUI · CRUZA] |
        // [BANCO · SALVA · ABRIR] | [DESFAZ · DESCABEIA · ESPERA] |
        // [ZOOM − + · RACK]. Na segunda fileira (janela estreita) o VARIA
        // abre a fileira e não leva régua antes.
        if (cmdY_ == 12) x += groupSeparator(g, x);
        if (x + 38 + kVaryW + 6 < rx - 8) {
            g.setColour(varyAmount_ > 0.0f ? T.textPrimary : T.textSecondary);
            g.drawText(str(rasgo::panel::strings::hdrVary), x, cmdY_, 36, 22,
                       juce::Justification::centredLeft, false);
            x += 38;
            varySlider_ = {x, cmdY_ + 4, kVaryW, 14};
            g.setColour(T.recessed);
            g.fillRoundedRectangle(varySlider_.toFloat(), 3.0f);
            const int fw = juce::roundToInt(
                (varyAmount_ * 0.5f) * static_cast<float>(kVaryW - 2));
            if (fw > 0) {
                g.setColour(flashing(Act::vary) ? T.textPrimary : T.accent);
                g.fillRoundedRectangle(
                    juce::Rectangle<int>(varySlider_.getX() + 1,
                                         varySlider_.getY() + 1,
                                         fw, varySlider_.getHeight() - 2)
                        .toFloat(), 2.0f);
            }
            g.setColour(T.textSecondary.withAlpha(0.7f));
            const int mx = varySlider_.getX() + 1 + (kVaryW - 2) / 2;
            g.drawVerticalLine(mx, static_cast<float>(varySlider_.getY() + 2),
                               static_cast<float>(varySlider_.getBottom() - 2));
            x += kVaryW + 6;
        }

        for (const auto& c : cmds) {
            const juce::String L = str(*c.label);
            if (c.act == Act::bank || c.act == Act::undo)   // início de grupo
                x += groupSeparator(g, x);
            // com duas fileiras o corte nunca acontece; a guarda fica só
            // como rede pra janela absurdamente estreita
            if (x + textW(g, L) + 14 > rx - 8) break;
            x += button(g, x, L, c.on || flashing(c.act), c.act);

        }
        // STANDBY logo à direita do DESCABEIA (pedido do autor, 2 out.
        // 2026): os dois gestos que calam o som ficam juntos, como dois
        // botões comuns. A régua que o separava dos outros comandos (herdada
        // do painel X11, contra clique acidental) saiu no mesmo dia, também
        // a pedido: ao lado do DESCABEIA ela só cortava a fileira.
        {
            const juce::String L = str(rasgo::panel::strings::hdrStandby);
            g.setFont(juce::FontOptions(11.0f));
            if (x + textW(g, L) + 14 < rx - 8)
                x += button(g, x, L, standby_, Act::standby);
        }
        // (o realce momentâneo de SEED, ZOOM, REC e afins é aplicado nos
        // próprios `button`/`buttonR` acima, via `flashing`)
        // ZOOM e RACK juntos: os dois dizem respeito a COMO se vê o rack
        // (pedido do autor, 2 out. 2026). O RACK vinha do canto direito.
        if (x + kGroupSepW + 24 + 40 + labelW(rackLabel()) < rx - 8) {
            x += groupSeparator(g, x);
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(11.0f));
            // `cmdY_`, não 12: na segunda fileira (janela estreita) o
            // rótulo ficava sozinho lá em cima, separado dos seus botões.
            g.drawText("ZOOM", x, cmdY_, 38, 22,
                       juce::Justification::centredLeft, false);
            x += 40;
            x += button(g, x, u8("\xe2\x88\x92"), flashing(Act::zoomOut), Act::zoomOut);
            x += button(g, x, "+", flashing(Act::zoomIn), Act::zoomIn);
            // As duas palavras vinham FIXAS em português — o botão dizia
            // "RACK · SAÍDA" mesmo com a interface em inglês (achado do
            // autor, 18 set. 2026); hoje vêm de `UiLanguage.hpp`.
            x += button(g, x, rackLabel(), rackOut_, Act::rackView);
        }
    }

    // Régua fina entre grupos de botões. Devolve a largura que ocupa, a
    // mesma `kGroupSepW` que as duas contas de largura somam.
    int groupSeparator(juce::Graphics& g, int x) const {
        g.setColour(T.line);
        g.drawVerticalLine(x + kGroupSepW / 2 - 3,
                           static_cast<float>(btnY_ + 3),
                           static_cast<float>(btnY_ + 19));
        return kGroupSepW;
    }
    juce::String rackLabel() const {
        return u8("RACK \xc2\xb7 ")
            + str(rackOut_ ? rasgo::panel::strings::hdrRackOut
                           : rasgo::panel::strings::hdrRackAll);
    }


    void mouseDown(const juce::MouseEvent& e) override {
        // sair da caixa de seed: sem isto o campo fica com o foco e os
        // atalhos de teclado continuam mudos depois de digitar um número
        if (seedBox_.hasKeyboardFocus(true))
            if (auto* p = getParentComponent()) p->grabKeyboardFocus();

        // O slider vem ANTES dos botões no teste de acerto: ele é
        // desenhado entre eles e é o alvo mais estreito dos dois.
        // Alcance generoso na vertical (uma faixa de 22 px em torno),
        // porque 14 px de altura é pouco para mirar — foi a reclamação
        // "preciso clicar várias vezes até achar o ponto certo" do
        // inspector de cabo, e não vou repetir.
        if (!varySlider_.isEmpty()
            && varySlider_.expanded(3, 6).contains(e.getPosition())) {
            varyDragging_ = true;
            setVaryFromX(e.getPosition().x);
            return;
        }
        for (const auto& h : hits_)
            if (h.bounds.contains(e.getPosition())) { trigger(h.act); return; }
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        if (varyDragging_) setVaryFromX(e.getPosition().x);
    }
    void mouseUp(const juce::MouseEvent&) override { varyDragging_ = false; }

    // Valor a partir da posição: clicar em qualquer ponto do slider salta
    // para lá, como em qualquer slider — não exige agarrar um punho.
    void setVaryFromX(const int px) {
        const float f = juce::jlimit(0.0f, 1.0f,
            static_cast<float>(px - varySlider_.getX() - 1)
                / static_cast<float>(kVaryW - 2));
        varyAmount_ = f * 2.0f;
        if (onVaryAmount) onVaryAmount(varyAmount_);
        repaint();
    }


    enum class Act { seed, lang, rackView, standby, zoomIn, zoomOut,
                     vary, mutate, evolve, cross, bank, save,
                     tutorial, about, rec, open, undo, uncable, restore };

    // Despacho ÚNICO de ação: o clique e o atalho de teclado entram os
    // dois por aqui. Antes o teclado chamava os callbacks direto e pulava
    // o flash — o atalho funcionava "no escuro", sem nada na tela dizer
    // que algo aconteceu, e era por isso que ficava difícil saber o que
    // estava ou não funcionando. Um despacho só também garante que uma
    // ação nova não fique ligada ao mouse e esquecida no teclado.
    void trigger(const Act a) {
        flash_[static_cast<int>(a)] = std::chrono::steady_clock::now();
        switch (a) {
        case Act::seed:      if (onSeed) onSeed(); break;
        case Act::lang:      if (onLang) onLang(); break;
        case Act::rackView:  if (onRackView) onRackView(); break;
        case Act::standby:   if (onStandby) onStandby(); break;
        case Act::zoomIn:    if (onZoom) onZoom(+1); break;
        case Act::zoomOut:   if (onZoom) onZoom(-1); break;
        case Act::vary:      if (onVary) onVary(); break;
        case Act::mutate:    if (onMutate) onMutate(); break;
        case Act::evolve:    if (onEvolve) onEvolve(); break;
        case Act::cross:     if (onCross) onCross(); break;
        case Act::bank:      if (onBank) onBank(); break;
        case Act::save:      if (onSave) onSave(); break;
        case Act::tutorial:  if (onTutorial) onTutorial(); break;
        case Act::about:     if (onAbout) onAbout(); break;
        case Act::rec:       if (onRec) onRec(); break;
        case Act::open:      if (onOpen) onOpen(); break;
        case Act::undo:      if (onUndo) onUndo(); break;
        case Act::uncable:   if (onUncable) onUncable(); break;
        case Act::restore:   if (onRestore) onRestore(); break;
        }
        repaint();
    }

private:
    struct Hit { juce::Rectangle<int> bounds; Act act; };

    static juce::String str(const rasgo::panel::L4& s, rasgo::panel::Lang l) {
        return u8(rasgo::panel::tr(s, l));
    }
    juce::String str(const rasgo::panel::L4& s) const { return str(s, lang_); }

    static int textW(juce::Graphics& g, const juce::String& s) {
        return juce::roundToInt(
            juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), s));
    }

    // ~160 ms aceso depois do clique. Sem isto um MUTA não dá retorno
    // nenhum: o patch muda, mas o gesto não deixa marca na tela.
    bool flashing(Act a) const {
        const auto it = flash_.find(static_cast<int>(a));
        return it != flash_.end()
            && std::chrono::steady_clock::now() - it->second
               < std::chrono::milliseconds(160);
    }

    // Largura de um rótulo de botão, medível FORA do `paint` — é o que
    // permite decidir a altura antes de desenhar.
    static int labelW(const juce::String& t) {
        const juce::Font f(juce::FontOptions(11.0f));
        return juce::roundToInt(juce::GlyphArrangement::getStringWidth(f, t)) + 14;
    }

    bool commandsFit(const int width) const {
        namespace S = rasgo::panel::strings;
        int need = kBrandW + 6 + seedBoxW() + 6
                 + labelW(u8("\xE2\x9A\x84 ") + str(S::hdrSeed)) + 6;
        if (seed_ != 0) need += labelW(str(S::hdrRestore)) + 6;
        for (const auto* l : {&S::hdrChange, &S::hdrEvolve,
                              &S::hdrCross, &S::hdrBank, &S::hdrSave,
                              &S::hdrOpen, &S::hdrUndo, &S::hdrUncable})
            need += labelW(str(*l)) + 6;
        // o slider do VARIA. Esquecer esta linha é o erro que este
        // cabeçalho já cometeu TRÊS vezes de formas diferentes: a conta de
        // largura (`cmdsW_`) e a DECISÃO de quebrar em duas fileiras
        // (`commandsFit`) medindo coisas diferentes. Quando divergem, algum
        // botão desaparece em silêncio — foi assim que o DESCABEIA sumiu e
        // o `n` pareceu quebrado por dois dias.
        need += 38 + kVaryW + 6;   // rótulo VARIA + slider (não mais botão)
        need += 50 + labelW(u8("\xe2\x88\x92")) + labelW("+") + 6;   // ZOOM
        need += labelW(str(S::hdrStandby)) + 6;                        // ESPERA
        need += labelW(rackLabel()) + 6;                               // RACK
        need += 4 * kGroupSepW;                                 // réguas
        // cluster da direita (sem o VU e a leitura, que já têm guarda)
        need += labelW(str(S::hdrAbout)) + labelW(str(S::hdrTutorial))
              + labelW(langLabel()) + labelW(u8("\xe2\x97\x8f ") + str(S::hdrRec))
              + 4 * 6 + 24;
        return need <= width;
    }

    static constexpr int kBrandW = 240;   // marca + "MODULAR" + régua
    static constexpr int kGroupSepW = 14;  // régua entre grupos de botões
    // Largura da caixa do seed, MEDIDA: 20 dígitos (o máximo de um seed de
    // 64 bits, `setInputRestrictions(20, …)`) na fonte do campo, mais as
    // margens internas do `TextEditor`. Eram 108 px fixos, e um seed de 19
    // ou 20 algarismos aparecia com o primeiro cortado (achado do autor, 2
    // out. 2026). Uma função só, usada pelo desenho E pela conta de
    // `commandsFit` — antes os dois tinham cada um o seu número.
    int seedBoxW() const {
        return juce::roundToInt(juce::GlyphArrangement::getStringWidth(
                   seedBox_.getFont(), "00000000000000000000"))
               + seedBox_.getLeftIndent() * 2 + 8;
    }

    juce::String langLabel() const {
        switch (lang_) {
        case rasgo::panel::Lang::en: return "EN";
        case rasgo::panel::Lang::pt: return "PT";
        case rasgo::panel::Lang::fr: return "FR";
        case rasgo::panel::Lang::es: return "ES";
        }
        return "PT";
    }

    int drawButton(juce::Graphics& g, int bx, const juce::String& label,
                   bool active, Act act, juce::Colour on = T.accent) {
        g.setFont(juce::FontOptions(11.0f));
        const int w = juce::roundToInt(
            juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), label)) + 14;
        const juce::Rectangle<int> r(bx, btnY_, w, 22);
        if (active) { g.setColour(on); g.fillRect(r); }
        g.setColour(active ? on : T.line);
        g.drawRect(r, 1);
        if (active) g.drawRect(r.expanded(2), 1);   // anel de "ligado"
        g.setColour(active ? T.bg : T.textSecondary);
        g.drawText(label, r, juce::Justification::centred, false);
        hits_.push_back({r, act});
        return w + 6;
    }
    int button(juce::Graphics& g, int bx, const juce::String& l, bool a, Act c) {
        return drawButton(g, bx, l, a, c);
    }
    int buttonR(juce::Graphics& g, int rightEdge, const juce::String& label,
                bool active, Act act, juce::Colour on = T.accent) {
        g.setFont(juce::FontOptions(11.0f));
        const int w = juce::roundToInt(
            juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), label)) + 14;
        drawButton(g, rightEdge - w, label, active, act, on);
        return w + 6;
    }

    void syncSeedText() {
        // Enquanto a pessoa digita, o campo manda — não atropelamos o que
        // está sendo escrito. MAS se o seed mudou por OUTRO caminho (o
        // botão SEED, [g], carregar patch), essa mudança é autoritativa e
        // tem que aparecer: era o bug de "editei o número, cliquei em
        // SEED, o patch mudou e o número ficou o mesmo" — o campo seguia
        // com o foco e a sincronização era sempre pulada.
        const bool changedElsewhere = seed_ != shownSeed_;
        if (seedBox_.hasKeyboardFocus(true) && !changedElsewhere) return;
        shownSeed_ = seed_;
        // SEM SINAL. O seed é `uint64_t` e o sorteio usa os 64 bits, então
        // exibi-lo como inteiro com sinal punha um "−" em metade dos
        // sorteios — e, como o campo só aceita dígitos, o número na tela
        // não podia ser digitado de volta: o seed exibido não reproduzia
        // o patch, que é a única razão de ele estar na tela. O painel X11
        // sempre formatou como `unsigned long long`; o erro era só aqui.
        const juce::String t(static_cast<juce::uint64>(seed_));
        if (seedBox_.getText() != t) seedBox_.setText(t, juce::dontSendNotification);
    }

    juce::TextEditor seedBox_;
    // slider da intensidade do VARIA: retângulo em tela e valor 0..2
    // 54 → 84 px (pedido do autor, 2 out. 2026): a 54 o ajuste fino de
    // VARIA ficava curto demais para a mão — cada pixel valia ~3,8% do
    // alcance; a 84, ~2,4%.
    static constexpr int kVaryW = 84;
    juce::Rectangle<int> varySlider_;
    float varyAmount_ = 1.0f;
    bool varyDragging_ = false;
    int cmdY_ = 12;     // y da barra de comandos (12 = 1ª fileira)
    int cmdsW_ = 0;     // largura que a barra de comandos exige
    int btnY_ = 12;     // y do botão sendo desenhado agora
    std::uint64_t shownSeed_ = 0;
    juce::Image logo_;
    std::vector<Hit> hits_;
    std::map<int, std::chrono::steady_clock::time_point> flash_;
    std::uint64_t seed_ = 0;
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
    bool rackOut_ = false, standby_ = false, vary_ = true, recording_ = false;
    std::size_t visMods_ = 0, totalMods_ = 0, cables_ = 0;
    float vu_ = 0.0f;

public:
    void setOverlay(int o) { overlay_ = o; repaint(); }
private:
    int overlay_ = 0;   // 0 nenhum · 1 tutorial · 2 sobre
};

// ---- o rack desenhado --------------------------------------------------
// Um único Component pinta tudo (immediate-mode), como o `redraw()` do
// painel X11 — em vez de milhares de juce::Slider, que seria a forma
// "JUCE idiomática" e a errada aqui: são ~60 módulos × N widgets, todos
// descritos por dados (`Panel`/`Widget`), e a interação já é por hit-test
// de pegada em mm.
class RackView : public juce::Component {
public:
    explicit RackView(Rack& r) : rack_(r) { setOpaque(true); }

    // px por mm — recalculado pra caber `kTargetRows` linhas na altura
    static constexpr float kSMin = 1.6f, kSMax = 2.6f;
    static constexpr int kTargetRows = 3;
    static constexpr float kGapMM = 3.0f;
    static constexpr int kPadPx = 14;
    static constexpr int kTopPadPx = 2;   // ver `relayout`

    int mmpx(float mm) const { return juce::roundToInt(mm * scale_); }


    void paint(juce::Graphics& g) override {
        g.fillAll(T.bg);
        // Só o que está à vista. O painel X11 já descartava módulo fora da
        // janela (`if (by + modH < kCaseTop || by > winH) continue;`); sem
        // isso aqui, o rack inteiro era repintado 30×/s mesmo com duas
        // fileiras visíveis.
        const juce::Rectangle<int> vis = g.getClipBounds();
        for (auto& m : mods_)
            if (m.bounds.intersects(vis)) paintModule(g, m);
        paintCables(g);
        paintInspector(g);

        // Vista filtrada sem nada pra mostrar: uma dica no lugar do rack
        // vazio. Ficou essencial quando o [n] (descabear) entrou — em
        // RACK·SAÍDA, tirar todos os cabos esvazia a tela por completo, e
        // sem explicação isso parece o app ter quebrado.
        if (mods_.empty() && outputOnly_) {
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(13.0f));
            g.drawText(u8(rasgo::panel::tr(
                           rasgo::panel::strings::rackViewEmpty, lang_)),
                       0, viewportH_ / 3, getWidth(), 24,
                       juce::Justification::centred, false);
        }

        // fantasma do módulo vindo da paleta
        if (!spawnType_.empty()) {
            g.setColour(T.accent);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText("+ " + u8(spawnType_), spawnAt_.x + 8, spawnAt_.y - 8,
                       160, 16, juce::Justification::centredLeft, false);
        }
    }

    // Cabos POR CIMA dos módulos, como no painel X11: curva com barriga
    // (`ui::cablePoints`, compartilhada com o painel pra os dois
    // desenharem a mesma curva), cor por hash determinístico do cabo —
    // paleta quente pra áudio, fria pra controle —, rompido = tracejado
    // na cor de aviso.
    // SEM lock: desenha do `cableSnap`, tirado uma vez por quadro pelo
    // timer com `try_lock`. Travar o grafo aqui roubava a corrida do
    // thread de áudio 30×/s — ver o comentário em `Rack::cableSnap`.
    void paintCables(juce::Graphics& g) {
        cableHits_.clear();
        // Destaque: o cabo sob o mouse e o do inspector aberto. Desenhados
        // DEPOIS dos outros (por cima), mais grossos e mais claros — e o do
        // inspector continua aceso enquanto a caixa está aberta, que agora
        // fica num canto fixo da tela, longe do cabo: o destaque é o que
        // liga uma coisa à outra.
        const int inspected = insp_.open ? snapIndexOfInspected() : -1;
        struct Hot { int x0, y0, x1, y1; juce::Colour col; bool cut; };
        std::vector<Hot> hot;
        for (std::size_t i = 0; i < rack_.cableSnap.size(); ++i) {
            const auto& c = rack_.cableSnap[i];
            const JackScreen* s = findJack(c.source,
                                           static_cast<int>(c.sourcePort), true);
            const JackScreen* t = findJack(c.target,
                                           static_cast<int>(c.targetPort), false);
            if (!s || !t) continue;   // uma das pontas é o sink (não exibido)
            const bool cut = c.ruptured;
            const std::size_t h = c.source * 7 + c.sourcePort * 3
                + c.target * 5 + c.targetPort;
            juce::Colour col = cut ? T.warning
                : (s->kind == PortKind::Control ? kCableCtrl[h & 3]
                                                : kCableAudio[h & 3]);
            // STANDBY: nada está chegando à saída, e o cabeamento mostra
            // isso — esmaecido, mas inteiro (o patch continua rodando por
            // baixo; quem ROMPE tudo é o [espaço], e aí eles ficam
            // tracejados de `warning`).
            const int ii = static_cast<int>(i);
            if (ii == hoverCable_ || ii == inspected)
                hot.push_back({s->x, s->y, t->x, t->y, col.brighter(0.45f), cut});
            if (silenced_) col = col.withMultipliedAlpha(0.32f);
            strokeCable(g, s->x, s->y, t->x, t->y, col, cut);
            cableHits_.push_back({i, s->x, s->y, t->x, t->y});
        }
        for (const auto& c : hot)
            strokeCable(g, c.x0, c.y0, c.x1, c.y1, c.col, c.cut, 4.0f);
        if (cdrag_.active) {
            // Fonte sem sinal agora: o cabo sai acinzentado. Explica o
            // "liguei e não aconteceu nada" ANTES de ligar — e aponta o
            // culpado certo, que nesse caso é a origem e não o destino.
            juce::Colour col = cdrag_.kind == PortKind::Control
                ? kCableCtrl[0] : kCableAudio[0];
            if (dragSourceSilent_) col = col.withAlpha(0.30f);
            strokeCable(g, cdrag_.ax, cdrag_.ay, mouse_.x, mouse_.y, col, true);
        }
    }

    // ---- inspector de cabo ---------------------------------------------
    // Um cabo do Rasgo não é um fio: é um objeto com estado (ganho,
    // condutância, relação com um companion, ruptura com cicatriz). Esta
    // caixa é onde esse estado fica editável — ver `guia/RELACAO_DE_CABO.md`.
    // Diferente do painel X11, ela é ancorada em coordenadas de CONTEÚDO e
    // não de tela: rola junto com o rack, ficando sempre ao lado do cabo a
    // que pertence (no X11 não havia viewport, o ponto era o mesmo).
    void paintInspector(juce::Graphics& g) {
        inspHits_.clear();
        // `RASGO_INSPECIONAR=n` abre a caixa do n-ésimo cabo uma vez, no
        // primeiro quadro que tiver cabos — para capturas de tela e
        // documentação sem precisar mirar e clicar. Sem a variável, nada.
        if (!inspectEnvDone_ && !rack_.cableSnap.empty()) {
            inspectEnvDone_ = true;
            if (const char* v = std::getenv("RASGO_INSPECIONAR")) {
                const auto n = static_cast<std::size_t>(std::strtoul(v, nullptr, 10));
                if (n < rack_.cableSnap.size()) {
                    const auto& cv = rack_.cableSnap[n];
                    insp_ = {true, cv.target, cv.targetPort, 0, 0, 0, 0, 0, 0};
                }
            }
        }
        if (!insp_.open) return;
        const int si = snapIndexOfInspected();
        if (si < 0) { insp_.open = false; return; }     // o cabo sumiu

        const auto& c = rack_.cableSnap[static_cast<std::size_t>(si)];
        const bool hasRel = c.hasRelation;
        // Linhas: título · relação · [companion · AMT] · GANHO · COND ·
        // romper/remover. Grupo 1 da caixa do cabo (2 out. 2026): GANHO e
        // REMOVER entraram, COND deixou de depender de haver relação (a
        // condutância sempre funcionou sozinha, e a caixa a escondia), o
        // companion aparece pelo nome, e o título traz a luz de condução e
        // a marca de realimentação.
        const int pad = 8, rowH = 20, bw = 210;
        const int rows = hasRel ? 8 : 6;   // o título ocupa duas
        const int bh = pad * 2 + rowH * rows;
        // Canto INFERIOR DIREITO da área visível (sugestão do autor, 2 out.
        // 2026), não mais ao lado do clique: ancorada no clique, a caixa
        // caía em cima de módulos e cabos vizinhos e mudava de lugar a cada
        // abertura. Fixa, ela está sempre onde o olho a espera; o cabo
        // inspecionado fica destacado (ver `paintCables`). Coordenadas da
        // view: acompanha a rolagem, sempre no canto da janela.
        juce::Rectangle<int> vis = getLocalBounds();
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
            vis = vp->getViewArea();
        insp_.bx = std::max(vis.getX() + 4, vis.getRight() - bw - 12);
        insp_.by = std::max(vis.getY() + 4, vis.getBottom() - bh - 12);
        insp_.bw = bw; insp_.bh = bh;

        g.setColour(T.surface);
        g.fillRect(insp_.bx, insp_.by, bw, bh);
        g.setColour(T.accent);
        g.drawRect(insp_.bx, insp_.by, bw, bh, 1);

        int ry = insp_.by + pad;
        // TÍTULO: as duas pontas com módulo E porta (sugestão do autor, 2
        // out. 2026). Com a caixa fixa no canto, longe do cabo, só os
        // módulos não bastavam: dois cabos entre os mesmos módulos eram
        // indistinguíveis. À direita: a luz de condução (acesa quando o
        // sorteio deixa passar NESTE quadro — com COND < 1 ela pisca) e a
        // marca de realimentação.
        {
            const auto portName = [&](std::size_t node, std::size_t port, bool out) {
                try {
                    const auto& n = rack_.graph.node(node);
                    return u8(out ? n.outputDescriptor(port).name
                                  : n.inputDescriptor(port).name);
                } catch (...) { return juce::String(); }
            };
            const juce::String dot = u8("  \xc2\xb7  ");
            const juce::String from = moduleLabel(c.source) + dot
                + portName(c.source, c.sourcePort, true);
            const juce::String to = u8("\xe2\x86\x92 ") + moduleLabel(c.target) + dot
                + portName(c.target, c.targetPort, false);
            const int tw = bw - pad * 2 - 16;
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.setColour(T.textPrimary);
            g.drawText(from, insp_.bx + pad, ry, tw, 14,
                       juce::Justification::centredLeft, true);
            g.drawText(to, insp_.bx + pad, ry + rowH - 4, tw, 14,
                       juce::Justification::centredLeft, true);
            const float lx = static_cast<float>(insp_.bx + bw - pad - 9);
            g.setColour(c.conducting && !c.ruptured ? T.accent : T.recessed);
            g.fillEllipse(lx, static_cast<float>(ry + 3), 8.0f, 8.0f);
            g.setColour(T.line);
            g.drawEllipse(lx, static_cast<float>(ry + 3), 8.0f, 8.0f, 1.0f);
            if (c.feedback) {
                g.setFont(juce::FontOptions(8.0f));
                g.setColour(T.warning);
                g.drawText(u8("\xe2\x86\xba ") + u8(rasgo::panel::tr(
                               rasgo::panel::strings::inspFeedback, lang_)),
                           insp_.bx + pad, ry + 2 * rowH - 8, bw - pad * 2, 10,
                           juce::Justification::centredRight, false);
            }
        }
        ry += 2 * rowH;

        const auto boxBtn = [&](int bx, int by, int bwp, int bhp,
                                const char* label, bool active, InspAct act) {
            if (active) { g.setColour(T.accent); g.fillRect(bx, by, bwp, bhp); }
            g.setColour(active ? T.accent : T.line);
            g.drawRect(bx, by, bwp, bhp, 1);
            g.setColour(active ? T.bg : T.textSecondary);
            g.setFont(juce::FontOptions(9.0f));
            g.drawText(u8(label), bx + 2, by, bwp - 4, bhp,
                       juce::Justification::centred, false);
            inspHits_.push_back({{bx, by, bwp, bhp}, act});
        };

        int cx = insp_.bx + pad;
        const int relW = (bw - pad * 2) / 4;
        // `Relation{}` é o enumerador zero (None). No painel X11 isso era
        // obrigatório porque `Xlib.h` define a macro `None`; aqui não é,
        // mas mantenho a forma pra os dois lerem igual.
        boxBtn(cx, ry, relW, rowH - 2, "NONE",
               c.relation == Relation{}, InspAct::RelNone);
        cx += relW;
        boxBtn(cx, ry, relW, rowH - 2, "RING",
               c.relation == Relation::RingMod, InspAct::RelRing);
        cx += relW;
        boxBtn(cx, ry, relW, rowH - 2, "FOLD",
               c.relation == Relation::Fold, InspAct::RelFold);
        cx += relW;
        boxBtn(cx, ry, bw - pad - cx + insp_.bx, rowH - 2, "DIFF",
               c.relation == Relation::Difference, InspAct::RelDiff);
        ry += rowH;

        const auto sliderRow = [&](const char* label, float v, InspAct act,
                                   bool centreMark) {
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(9.0f));
            g.drawText(u8(label), insp_.bx + pad, ry, 40, rowH - 2,
                       juce::Justification::centredLeft, false);
            const int trackX = insp_.bx + pad + 42;
            const int trackW = bw - pad * 2 - 42;
            g.setColour(T.line);
            g.drawRect(trackX, ry + 3, trackW, rowH - 8, 1);
            const int fillW = static_cast<int>(
                static_cast<float>(trackW) * juce::jlimit(0.0f, 1.0f, v));
            if (fillW > 0) {
                g.setColour(T.accent);
                g.fillRect(trackX, ry + 3, fillW, rowH - 8);
            }
            if (centreMark) {   // ganho neutro (1,0) no meio da trilha
                g.setColour(T.textSecondary.withAlpha(0.8f));
                g.drawVerticalLine(trackX + trackW / 2,
                                   static_cast<float>(ry + 4),
                                   static_cast<float>(ry + rowH - 6));
            }
            inspHits_.push_back({{trackX, ry, trackW, rowH - 2}, act});
            ry += rowH;
        };

        if (hasRel) {
            // o segundo sinal, pelo nome; clicar volta ao modo de escolha
            juce::String comp = moduleLabel(c.companionNode);
            try {
                comp += u8("  \xc2\xb7  ") + u8(rack_.graph.node(c.companionNode)
                            .outputDescriptor(c.companionPort).name);
            } catch (...) {}
            const juce::Rectangle<int> cr(insp_.bx + pad, ry, bw - pad * 2, rowH - 2);
            g.setColour(T.line);
            g.drawRect(cr, 1);
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(9.0f));
            g.drawText(u8("COMP  ") + comp, cr.reduced(4, 0),
                       juce::Justification::centredLeft, false);
            inspHits_.push_back({cr, InspAct::Companion});
            ry += rowH;
            sliderRow("AMT", c.amount, InspAct::Amount, false);
        }
        sliderRow("GAIN", c.gain * 0.5f, InspAct::Gain, true);   // 0..2
        sliderRow("COND", c.conductance, InspAct::Conductance, false);

        const bool ruptured = c.ruptured;
        const int half = (bw - pad * 2 - 4) / 2;
        boxBtn(insp_.bx + pad, ry, half, rowH - 2,
               rasgo::panel::tr(ruptured ? rasgo::panel::strings::inspReconnect
                                         : rasgo::panel::strings::inspRupture,
                                lang_).c_str(),
               ruptured, InspAct::Rupture);
        boxBtn(insp_.bx + pad + half + 4, ry, half, rowH - 2,
               rasgo::panel::tr(rasgo::panel::strings::inspRemove, lang_).c_str(),
               false, InspAct::Remove);
    }

    // Prepara a afordância do cabeamento: quem alcança a saída, e se a
    // fonte de onde se está puxando tem sinal agora.
    void beginDragAffordance(std::size_t sourceNode) {
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            dragFeeds_ = rack_.graph.nodesFeeding(rack_.sink);
        }
        const auto it = rack_.scopeSnap.find(sourceNode);
        dragSourceSilent_ = it == rack_.scopeSnap.end()
                            || it->second.peak() < 1.0e-4f;
    }

    // `true` se ligar NESTE nó produz som agora — isto é, se ele alcança a
    // saída. Puxando de uma ENTRADA, quem precisa alcançar é o dono dela,
    // e a resposta é a mesma pra todos os destinos.
    bool audibleTarget(std::size_t node) const {
        const std::size_t n = cdrag_.fromOutput ? node : cdrag_.node;
        return n < dragFeeds_.size() && dragFeeds_[n] != 0;
    }

    // índice do cabo inspecionado no SNAPSHOT (-1 se já não existe)
    int snapIndexOfInspected() const {
        for (std::size_t i = 0; i < rack_.cableSnap.size(); ++i)
            if (rack_.cableSnap[i].target == insp_.tnode
                && rack_.cableSnap[i].targetPort == insp_.tport)
                return static_cast<int>(i);
        return -1;
    }
    // …e no grafo VIVO (chamar sob `gmx`)
    int liveIndexOfInspected() const {
        for (std::size_t i = 0; i < rack_.graph.cableCount(); ++i) {
            const auto& c = rack_.graph.cable(i);
            if (c.target().node == insp_.tnode && c.target().port == insp_.tport)
                return static_cast<int>(i);
        }
        return -1;
    }

    // "OSC", "OSC 2"… — desambigua quando há vários do mesmo tipo, como no
    // painel X11
    juce::String moduleLabel(std::size_t nodeId) const {
        const std::string ty = rack_.graph.node(nodeId).type();
        int count = 0, mine = -1;
        for (const auto& m : mods_) {
            if (rack_.graph.node(m.id).type() != ty) continue;
            if (m.id == nodeId) mine = count;
            ++count;
        }
        if (count <= 1 || mine < 0) return u8(ty);
        return u8(ty) + " " + juce::String(mine + 1);
    }

    void strokeCable(juce::Graphics& g, int x0, int y0, int x1, int y1,
                     juce::Colour col, bool dashed, float width = 2.0f) const {
        const auto pts = ui::cablePoints(
            static_cast<float>(x0), static_cast<float>(y0),
            static_cast<float>(x1), static_cast<float>(y1));
        juce::Path path;
        path.startNewSubPath(pts[0].x, pts[0].y);
        for (std::size_t k = 1; k < pts.size(); ++k)
            path.lineTo(pts[k].x, pts[k].y);
        g.setColour(col);
        if (dashed) {
            const float dash[] = {5.0f, 4.0f};
            juce::Path dashedPath;
            juce::PathStrokeType(width * 0.5f).createDashedStroke(dashedPath, path,
                                                                  dash, 2);
            g.fillPath(dashedPath);
        } else {
            g.strokePath(path, juce::PathStrokeType(width));
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        mouse_ = e.getPosition();
        if (onFocusWanted) onFocusWanted();

        // Botão do MEIO paneia o rack, como no painel X11 — inclusive
        // durante o cabeamento, pra alcançar um jack fora da tela sem
        // largar o cabo.
        if (e.mods.isMiddleButtonDown()) {
            if (auto* vp = findParentComponentOfClass<juce::Viewport>())
                pan_ = {true, e.getPosition().y, vp->getViewPositionY()};
            return;
        }

        // ---- escolhendo o companion de uma relação de cabo ------------
        // qualquer SAÍDA serve, inclusive a própria origem do cabo (a
        // auto-relação é legítima — ver `guia/RELACAO_DE_CABO.md` §1)
        if (picking_) {
            const int ji = jackAt(e.getPosition());
            if (ji >= 0 && jacks_[static_cast<std::size_t>(ji)].isOut) {
                const JackScreen j = jacks_[static_cast<std::size_t>(ji)];
                std::lock_guard<std::mutex> lk(rack_.gmx);
                const int li = liveIndexOfInspected();
                if (li >= 0) {
                    Cable& c = rack_.graph.cable(static_cast<std::size_t>(li));
                    c.setRelation(c.relation(), j.node,
                                  static_cast<std::size_t>(j.port),
                                  c.relationAmount());
                }
            }
            picking_ = false;
            repaint();
            return;
        }

        // ---- inspector aberto: seus botões, ou fecha ------------------
        if (insp_.open && handleInspectorClick(e)) { repaint(); return; }

        // ---- jack: começa a cabear, ou (botão direito) desliga --------
        const int ji = jackAt(e.getPosition());
        if (ji >= 0) {
            const JackScreen j = jacks_[static_cast<std::size_t>(ji)];
            if (e.mods.isRightButtonDown()) {
                bool any = false;
                rack_.pushUndo();          // cortar cabo é desfazível
                {
                    std::lock_guard<std::mutex> lk(rack_.gmx);
                    const auto before = rack_.graph.nodesFeeding(rack_.sink);
                    if (!j.isOut) {
                        any = rack_.graph.disconnect(
                            j.node, static_cast<std::size_t>(j.port));
                    } else {
                        // tira todos os cabos que SAEM deste jack
                        for (std::size_t i = rack_.graph.cableCount(); i-- > 0;) {
                            const auto& c = rack_.graph.cable(i);
                            if (c.source().node == j.node
                                && static_cast<int>(c.source().port) == j.port) {
                                rack_.graph.disconnect(c.target().node,
                                                       c.target().port);
                                any = true;
                            }
                        }
                    }
                    if (any) {
                        rack_.reprepare();   // sem isto o corte não vale
                        keepVisibleAfterCut(before);
                    }
                }
                if (any) layoutFor(viewportW_, viewportH_);  // vista filtrada
                repaint();
                return;
            }

            // botão esquerdo numa ENTRADA já cabeada: "pega" a ponta —
            // desliga e ancora o arrasto na saída de origem, que é o gesto
            // de repatchear sem ter que ir até a outra ponta
            if (!j.isOut) {
                int from = -1;
                std::size_t srcNode = 0;
                rack_.pushUndo();          // repatchear é desfazível
                {
                    std::lock_guard<std::mutex> lk(rack_.gmx);
                    for (std::size_t i = 0; i < rack_.graph.cableCount(); ++i) {
                        const auto& c = rack_.graph.cable(i);
                        if (c.target().node == j.node
                            && static_cast<int>(c.target().port) == j.port) {
                            from = static_cast<int>(i); break;
                        }
                    }
                    if (from >= 0) {
                        const auto src = rack_.graph.cable(
                            static_cast<std::size_t>(from)).source();
                        const auto before = rack_.graph.nodesFeeding(rack_.sink);
                        rack_.graph.disconnect(
                            j.node, static_cast<std::size_t>(j.port));
                        rack_.reprepare();
                        keepVisibleAfterCut(before);
                        rebuildJacks();
                        const JackScreen* sj = findJack(
                            src.node, static_cast<int>(src.port), true);
                        cdrag_ = {true, src.node, static_cast<int>(src.port),
                                  true, sj ? sj->kind : PortKind::Audio,
                                  sj ? sj->x : e.getPosition().x,
                                  sj ? sj->y : e.getPosition().y};
                        srcNode = src.node;
                    }
                }
                // FORA do lock: `beginDragAffordance` trava o `gmx` por
                // conta própria, e `std::mutex` não é reentrante. Chamada
                // aqui dentro (como foi de ab316bf até a v0.1.0), travar
                // duas vezes na mesma thread é comportamento indefinido. No
                // Linux a thread de mensagens congela esperando a si mesma
                // (comprovado com um teste mínimo em 1 out. 2026); a STL
                // da MSVC detecta a reentrada e lança exceção, o que é
                // compatível com o relato de um usuário do Windows 10 no
                // mesmo dia: "crash ao tirar um cabo de um módulo". O
                // gesto era o mais comum de repatchear — clicar numa
                // entrada já cabeada para pegar a ponta do cabo.
                if (from >= 0) {
                    beginDragAffordance(srcNode);
                    repaint();
                    return;
                }
            }

            cdrag_ = {true, j.node, j.port, j.isOut, j.kind, j.x, j.y};
            beginDragAffordance(j.node);
            juce::Desktop::getInstance().beginDragAutoRepeat(40);
            repaint();
            return;
        }

        for (const auto& m : mods_) {
            if (!m.bounds.contains(e.getPosition())) continue;
            // [x] de remover
            if (juce::Rectangle<int>(m.bounds.getRight() - 15, m.bounds.getY() + 2,
                                     13, 13).contains(e.getPosition())) {
                if (onRemoveModule) onRemoveModule(m.id);
                return;
            }
            Signal& node = rack_.graph.node(m.id);

            // MATRIX: arrasto vertical numa célula ajusta `g<jk>` — mesma
            // via do knob genérico, só a área de pega é outra
            if (node.type() == "MATRIX") {
                for (int j = 0; j < 4; ++j)
                    for (int k = 0; k < 4; ++k) {
                        const ui::RectMM rm = ui::matrixCellMM(j, k);
                        const juce::Rectangle<int> cell(
                            m.bounds.getX() + mmpx(rm.x),
                            m.bounds.getY() + mmpx(rm.y),
                            mmpx(rm.w), mmpx(rm.h));
                        if (!cell.contains(e.getPosition())) continue;
                        const char id[4] = {'g', static_cast<char>('1' + j),
                                            static_cast<char>('1' + k), 0};
                        drag_ = {true, m.id, std::string(id),
                                 rack_.graph.parameterUserValue(m.id, id),
                                 -1.0f, 1.0f, e.getPosition().y};
                        return;
                    }
            }

            for (const auto& w : m.panel.widgets) {
                if (node.type() == "MATRIX" && w.kind == Widget::Kind::Knob
                    && ui::isMatrixCellBind(w.bind)) continue;
                if (w.kind != Widget::Kind::Knob
                    && w.kind != Widget::Kind::Slider
                    && w.kind != Widget::Kind::Toggle) continue;
                if (!widgetBounds(m, w).contains(e.getPosition())) continue;
                if (w.kind == Widget::Kind::Toggle) {
                    // toggle vira na hora (não arrasta)
                    float from = 0.0f, to = 0.0f;
                    {
                        std::lock_guard<std::mutex> lk(rack_.gmx);
                        from = rack_.graph.parameterUserValue(m.id, w.bind);
                        to = from >= 0.5f ? 0.0f : 1.0f;
                        rack_.graph.setParameterBase(m.id, w.bind, to);
                    }
                    if (onParamGesture) onParamGesture(m.id, w.bind, from, to);
                    repaint();
                } else {
                    const ParameterDescriptor* d = paramDesc(node, w.bind);
                    drag_ = {true, m.id, w.bind,
                             rack_.graph.parameterUserValue(m.id, w.bind),
                             d ? d->minimum : 0.0f, d ? d->maximum : 1.0f,
                             e.getPosition().y};
                }
                return;
            }
            // SCOPE: clicar no Display alterna onda ↔ espectro
            if (node.type() == "SCOPE")
                for (const auto& w : m.panel.widgets)
                    if (w.kind == Widget::Kind::Display
                        && widgetBounds(m, w).contains(e.getPosition())) {
                        scopeView_[m.id] = (scopeView_[m.id] + 1) % 2;
                        repaint();
                        return;
                    }
            // Corpo do módulo COM um cabo passando por cima: as duas
            // ações querem o mesmo pixel — abrir o inspector do cabo e
            // arrastar o módulo. Dar prioridade a uma delas quebra a
            // outra (foi o que aconteceu: priorizar o cabo matou o
            // arrasto de módulo). Quem decide é o GESTO, não a posição:
            // soltar sem mover = clique = inspector; mover além do limiar
            // = arrasto de módulo. Só decidimos no `mouseDrag`/`mouseUp`.
            //
            // Regra do autor (2 out. 2026), que substitui a decisão pelo
            // gesto descrita acima: clique SOBRE UM CABO é sempre do cabo —
            // abre o inspector ao soltar, e arrastar dali não move nada. O
            // módulo só se desloca se o clique começar numa área SEM cabo e
            // o arrasto passar de `kDragSlopPx`. Antes, um clique que errava
            // o cabo por um pixel virava arrasto, e a menor tremida da mão
            // tirava o módulo do lugar — era preciso DESFAZ e recomeçar.
            downAt_ = e.getPosition();
            if (const int ci = cableUnder(e.getPosition()); ci >= 0) {
                pendingInspect_ = ci;
                return;
            }
            mdrag_ = {true, m.id, false};
            juce::Desktop::getInstance().beginDragAutoRepeat(40);
            return;
        }

        // fora de qualquer módulo: ainda pode ser o corpo de um cabo
        return (void)tryOpenInspector(e);
    }

    // Clique no CORPO de um cabo → abre o inspector daquele cabo.
    //
    // Ordem de prioridade: jack e CONTROLE ganham sempre (são alvos
    // pequenos e intencionais). Mas o cabo ganha do CORPO do módulo, e
    // isso é deliberado: os cabos são desenhados POR CIMA dos módulos, e
    // como os módulos ocupam quase toda a case, deixar o corpo do módulo
    // ganhar fazia quase todo o comprimento do cabo virar zona morta — só
    // dava pra abrir o inspector nas frestas entre fileiras. Era o relato
    // "preciso clicar várias vezes até achar o ponto certo". Arrastar o
    // módulo continua funcionando em toda parte onde não passa cabo.
    //
    // Alcance proporcional ao zoom: fixo em pixel ficava minúsculo em
    // zoom alto, e mirar numa curva fina já é mais difícil que num jack.
    // índice do cabo sob o ponto (-1 se nenhum)
    //
    // De 15 set. a 2 out. 2026 esta função passava o PONTO por último a
    // `pointNearCable`, que o espera primeiro: testava se o jack de origem
    // estava perto de uma curva do destino até o mouse. O clique no cabo
    // acertava por acaso — o "fico tentando várias vezes" do autor. Agora
    // usa `nearestCable`, testada no ctest, que também escolhe o MAIS
    // PRÓXIMO quando dois cabos passam perto.
    int cableUnder(juce::Point<int> p) const {
        const float tol = static_cast<float>(mmpx(5.0f) + 3);
        std::vector<ui::CableEnds> ends;
        ends.reserve(cableHits_.size());
        for (const auto& ch : cableHits_)
            ends.push_back({static_cast<float>(ch.x0), static_cast<float>(ch.y0),
                            static_cast<float>(ch.x1), static_cast<float>(ch.y1)});
        const int k = ui::nearestCable(static_cast<float>(p.x),
                                       static_cast<float>(p.y), ends, tol);
        return k < 0 ? -1 : static_cast<int>(cableHits_[static_cast<std::size_t>(k)].cable);
    }

    // Há um controle (jack, knob, slider, toggle, célula da MATRIX, [x])
    // sob o ponto? Esses ganham do cabo no clique, então o destaque de
    // cabo não acende sobre eles — o destaque tem que mostrar exatamente
    // o que o clique vai pegar.
    bool controlAt(juce::Point<int> p) const {
        if (jackAt(p) >= 0) return true;
        for (const auto& m : mods_) {
            if (!m.bounds.contains(p)) continue;
            if (juce::Rectangle<int>(m.bounds.getRight() - 15, m.bounds.getY() + 2,
                                     13, 13).contains(p))
                return true;
            const std::string ty = rack_.graph.node(m.id).type();
            if (ty == "MATRIX")
                for (int j = 0; j < 4; ++j)
                    for (int k = 0; k < 4; ++k) {
                        const ui::RectMM rm = ui::matrixCellMM(j, k);
                        if (juce::Rectangle<int>(m.bounds.getX() + mmpx(rm.x),
                                                 m.bounds.getY() + mmpx(rm.y),
                                                 mmpx(rm.w), mmpx(rm.h)).contains(p))
                            return true;
                    }
            for (const auto& w : m.panel.widgets) {
                const bool interactive = w.kind == Widget::Kind::Knob
                    || w.kind == Widget::Kind::Slider
                    || w.kind == Widget::Kind::Toggle
                    || (w.kind == Widget::Kind::Display && ty == "SCOPE");
                if (interactive && widgetBounds(m, w).contains(p)) return true;
            }
            return false;
        }
        return false;
    }

    // Destaque do cabo sob o mouse (sugestão do autor, 2 out. 2026): sem
    // ele só se sabia se o ponteiro estava no cabo DEPOIS de clicar.
    void mouseMove(const juce::MouseEvent& e) override {
        setHoverCable(controlAt(e.getPosition()) ? -1 : cableUnder(e.getPosition()));
    }
    void mouseExit(const juce::MouseEvent&) override { setHoverCable(-1); }
    void setHoverCable(int h) {
        if (h == hoverCable_) return;
        hoverCable_ = h;
        setMouseCursor(h >= 0 ? juce::MouseCursor::PointingHandCursor
                              : juce::MouseCursor::NormalCursor);
        repaint();
    }

    void openInspectorFor(int snapIdx, juce::Point<int> at) {
        if (snapIdx < 0
            || static_cast<std::size_t>(snapIdx) >= rack_.cableSnap.size()) return;
        const auto& cv = rack_.cableSnap[static_cast<std::size_t>(snapIdx)];
        insp_ = {true, cv.target, cv.targetPort, at.x, at.y, 0, 0, 0, 0};
        repaint();
    }

    bool tryOpenInspector(const juce::MouseEvent& e) {
        if (!e.mods.isLeftButtonDown()) return false;
        const int ci = cableUnder(e.getPosition());
        if (ci < 0) return false;
        openInspectorFor(ci, e.getPosition());
        return true;
    }

    // Levar o cabo (ou o módulo) para perto da borda rola o rack, como no
    // painel X11. Duas peças são necessárias:
    //  - `Viewport::autoScroll`, que faz a rolagem em si;
    //  - `beginDragAutoRepeat`, sem o qual o JUCE só entrega `mouseDrag`
    //    quando o ponteiro SE MOVE — parado na borda, a rolagem parava
    //    depois de um passo só.
    // Devolve true se rolou (aí o repinte tem que ser do rack inteiro: o
    // conteúdo andou sob o ponteiro e o retângulo sujo do cabo não vale).
    bool autoScrollAtEdge(const juce::MouseEvent& e) {
        auto* vp = findParentComponentOfClass<juce::Viewport>();
        if (vp == nullptr) return false;
        const auto p = vp->getLocalPoint(this, e.getPosition());
        return vp->autoScroll(p.x, p.y, 48, 14);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        if (pan_.active) {
            if (auto* vp = findParentComponentOfClass<juce::Viewport>())
                vp->setViewPosition(0, std::max(0,
                    pan_.startView + (pan_.startY - e.getPosition().y)));
            return;
        }
        // slider AMT/COND do inspector: barra HORIZONTAL, o valor segue a
        // posição X do mouse na trilha (não um delta vertical)
        if (cslide_.active) {
            // sem `pushUndo` aqui: o clique que iniciou o arrasto já
            // empilhou. Um snapshot por pixel encheria a pilha e faria o
            // desfazer andar um pixel de cada vez.
            std::lock_guard<std::mutex> lk(rack_.gmx);
            const int li = liveIndexOfInspected();
            if (li >= 0)
                applyCableSlider(rack_.graph.cable(static_cast<std::size_t>(li)),
                                 e.getPosition().x);
            repaint();
            return;
        }
        // clique que começou num cabo: arrastar além do limiar desiste do
        // clique, e não move nada (regra do autor — ver `mouseDown`)
        if (pendingInspect_ >= 0 && !mdrag_.active) {
            if (e.getPosition().getDistanceFrom(downAt_) > kDragSlopPx)
                pendingInspect_ = -1;
            return;
        }
        if (mdrag_.active) {
            if (!mdrag_.moved) {          // ainda é clique: zona morta
                if (e.getPosition().getDistanceFrom(downAt_) <= kDragSlopPx)
                    return;
                mdrag_.moved = true;
            }
            autoScrollAtEdge(e);
            mouse_ = e.getPosition();
            // reordenar ao vivo — só na vista TODOS: nas filtradas a ordem
            // visível é parcial e a conta do índice de destino não fecha.
            // Arrastar pra a paleta pra remover continua valendo nas duas.
            reorderTo(mdrag_.id, e.getPosition());
            repaint();
            return;
        }
        if (cdrag_.active) {
            if (autoScrollAtEdge(e)) {
                mouse_ = e.getPosition();
                repaint();
                return;
            }
            // Repinta só o que o cabo elástico tocou: a união do traçado
            // anterior com o novo, com folga. Repintar o rack inteiro a
            // cada evento de mouse fazia o cabo ficar atrás do ponteiro —
            // era o "puxo o cabo e não me obedece".
            const juce::Point<int> a{cdrag_.ax, cdrag_.ay};
            juce::Rectangle<int> dirty =
                juce::Rectangle<int>(a, mouse_)
                    .getUnion(juce::Rectangle<int>(a, e.getPosition()));
            mouse_ = e.getPosition();
            // a BARRIGA do cabo desce abaixo dos dois extremos, e a folga
            // cresce com a distância horizontal (`cablePoints`: 18 + dx/6)
            // — sem somar isso, o retângulo sujo corta a curva e deixa
            // rastro
            const int sag = 18 + std::max(std::abs(a.x - mouse_.x),
                                          std::abs(a.x - e.getPosition().x)) / 6;
            dirty.setBottom(dirty.getBottom() + sag);
            repaint(dirty.expanded(mmpx(6.0f) + 6));
            return;
        }
        if (!drag_.active) return;
        // MESMA matemática do painel X11: 220 px = curso inteiro; faixa
        // larga (razão > 30, tipo frequência) é exponencial, senão linear.
        const float dy = static_cast<float>(drag_.startY - e.getPosition().y) / 220.0f;
        float v;
        if (drag_.lo > 0.0f && drag_.hi / drag_.lo > 30.0f) {
            const float k = std::log(drag_.hi / drag_.lo);
            float t = std::log(drag_.startVal / drag_.lo) / k + dy;
            t = juce::jlimit(0.0f, 1.0f, t);
            v = drag_.lo * std::exp(k * t);
        } else {
            v = drag_.startVal + dy * (drag_.hi - drag_.lo);
        }
        v = juce::jlimit(drag_.lo, drag_.hi, v);
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            rack_.graph.setParameterBase(drag_.node, drag_.bind, v);
        }
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override {
        juce::Desktop::getInstance().beginDragAutoRepeat(0);
        if (pan_.active) { pan_.active = false; return; }
        if (cslide_.active) { cslide_.active = false; return; }
        if (pendingInspect_ >= 0 && !mdrag_.active) {   // clique num cabo
            openInspectorFor(pendingInspect_, downAt_);
            pendingInspect_ = -1;
            return;
        }
        if (mdrag_.active) {
            const std::size_t id = mdrag_.id;
            const bool moved = mdrag_.moved;
            mdrag_.active = false;
            // soltou sobre a paleta = tira o módulo da case — só se houve
            // arrasto de verdade, nunca num clique parado
            if (moved && overPalette && overPalette(e.getScreenPosition())
                && onRemoveModule)
                onRemoveModule(id);
            repaint();
            return;
        }
        if (drag_.active) {
            // Registra no SYSTEM SCORE a mudança feita à mão, do valor em
            // que o controle estava ao soltar — um evento por gesto, não
            // um por pixel de arrasto.
            const float to = rack_.graph.parameterUserValue(drag_.node, drag_.bind);
            if (to != drag_.startVal && onParamGesture)
                onParamGesture(drag_.node, drag_.bind, drag_.startVal, to);
            drag_.active = false;
        }
        if (!cdrag_.active) return;
        const int ji = jackAt(e.getPosition());
        if (ji >= 0) {
            const JackScreen j = jacks_[static_cast<std::size_t>(ji)];
            if (j.isOut != cdrag_.fromOutput) {   // polaridade oposta
                // O caso que mais precisa de desfazer: o músico liga um
                // cabo, o resultado não é o esperado, e ele já não sabe
                // ao certo qual ligação fez.
                rack_.pushUndo();
                const std::size_t s = cdrag_.fromOutput ? cdrag_.node : j.node;
                const int sp = cdrag_.fromOutput ? cdrag_.port : j.port;
                const std::size_t d = cdrag_.fromOutput ? j.node : cdrag_.node;
                const int dp = cdrag_.fromOutput ? j.port : cdrag_.port;
                {
                    std::lock_guard<std::mutex> lk(rack_.gmx);
                    tryPatch(s, sp, d, dp);
                }
                // Retorno DEPOIS do ato, em vez de restrição antes: se o
                // caminho recém-ligado ainda não chega ao som, a caixa
                // LEARN diz isso numa linha. Não impede nada — ensina a
                // topologia enquanto a pessoa explora.
                if (!audibleTarget(d) && onNotice)
                    onNotice(rasgo::panel::tr(
                        rasgo::panel::strings::cableNotAudible, lang_));
            }
        }
        cdrag_.active = false;
        // `layoutFor`, não `relayout`: a vista SAÍDA pode passar a mostrar
        // um módulo novo, e só o `layoutFor` reajusta o tamanho do
        // componente — sem ele o Viewport fica com a faixa de rolagem
        // antiga e o último módulo não alcança.
        layoutFor(viewportW_, viewportH_);
        repaint();
    }

    // Liga, e se o motor recusar por ciclo, tenta de novo como feedback —
    // mesma política do painel X11 (`tryPatch`): ciclo vira conexão de
    // realimentação automaticamente, em vez de recusar o gesto.
    // devolve true se o clique foi consumido pelo inspector
    bool handleInspectorClick(const juce::MouseEvent& e) {
        const auto p = e.getPosition();
        const InspHit* hit = nullptr;
        for (const auto& h : inspHits_)
            if (h.bounds.contains(p)) { hit = &h; break; }

        if (hit == nullptr) {
            const juce::Rectangle<int> box(insp_.bx, insp_.by, insp_.bw, insp_.bh);
            if (box.contains(p)) return true;   // dentro da caixa, sem alvo
            insp_.open = false;                 // fora: fecha
            return false;                       // e o clique segue seu curso
        }

        if (hit->act == InspAct::Companion) {   // volta a escolher o companion
            picking_ = true;
            return true;
        }
        if (hit->act == InspAct::Remove) {
            // tirar o cabo pela caixa (antes só pelo clique direito no
            // jack). Desfazível como o clique direito.
            rack_.pushUndo();
            {
                std::lock_guard<std::mutex> lk(rack_.gmx);
                if (const int li = liveIndexOfInspected(); li >= 0) {
                    const auto before = rack_.graph.nodesFeeding(rack_.sink);
                    rack_.graph.disconnect(insp_.tnode, insp_.tport);
                    rack_.reprepare();
                    keepVisibleAfterCut(before);
                }
            }
            insp_.open = false;
            hoverCable_ = -1;
            // FORA do lock: `layoutFor` → `relayout` trava o `gmx` por
            // conta própria (a vista SAÍDA pode mudar sem o cabo)
            layoutFor(viewportW_, viewportH_);
            return true;
        }

        rack_.pushUndo();   // relação, condutância, ganho e ruptura são desfazíveis
        std::lock_guard<std::mutex> lk(rack_.gmx);
        const int li = liveIndexOfInspected();
        if (li < 0) { insp_.open = false; return true; }
        Cable& c = rack_.graph.cable(static_cast<std::size_t>(li));

        switch (hit->act) {
        case InspAct::RelNone:
            c.setRelation(Relation{}, 0, 0, 0.0f);
            break;
        case InspAct::RelRing:
        case InspAct::RelFold:
        case InspAct::RelDiff: {
            const Relation r = hit->act == InspAct::RelRing ? Relation::RingMod
                : (hit->act == InspAct::RelFold ? Relation::Fold
                                                : Relation::Difference);
            if (!c.hasRelation()) {
                // relação nova: começa AUTO-relacionada (companion = a
                // própria origem) e abre o modo de escolha pra trocar
                c.setRelation(r, c.source().node, c.source().port, 0.5f);
                picking_ = true;
            } else {
                // só troca o tipo; mantém companion e amount
                c.setRelation(r, c.companion().node, c.companion().port,
                              c.relationAmount());
            }
            break;
        }
        case InspAct::Amount:
        case InspAct::Conductance:
        case InspAct::Gain: {
            cslide_ = {true, hit->act == InspAct::Amount ? 0
                             : (hit->act == InspAct::Conductance ? 1 : 2),
                       hit->bounds.getX(), hit->bounds.getWidth()};
            applyCableSlider(c, p.x);
            break;
        }
        case InspAct::Rupture:
            if (c.state() == CableState::Ruptured) c.reconnect();
            else                                   c.rupture();
            break;
        case InspAct::Companion:   // tratados antes do lock
        case InspAct::Remove:
            break;
        }
        return true;
    }

    void applyCableSlider(Cable& c, int mouseX) {
        const float v = juce::jlimit(0.0f, 1.0f,
            static_cast<float>(mouseX - cslide_.trackX)
                / static_cast<float>(std::max(1, cslide_.trackW)));
        if (cslide_.which == 0)
            c.setRelation(c.relation(), c.companion().node, c.companion().port, v);
        else if (cslide_.which == 1)
            c.setConductance(v);
        else
            c.setGain(v * 2.0f);   // GAIN: 0..2, neutro (1,0) no meio
    }

    // Liga src→dst ao vivo; em ciclo, tenta de novo como feedback; deixa o
    // grafo consistente sempre. Deve rodar sob `gmx`.
    //
    // Cada caminho termina em `reprepare()`: é o `prepare` que recalcula a
    // ordem de processamento. Sem ele o cabo é registrado e não passa a
    // valer — o sintoma é exatamente "não consigo cabear".
    void tryPatch(std::size_t s, int sp, std::size_t d, int dp) {
        const auto sP = static_cast<std::size_t>(sp);
        const auto dP = static_cast<std::size_t>(dp);
        rack_.graph.disconnect(d, dP);
        try {
            rack_.graph.connect(s, sP, d, dP);
            rack_.reprepare();
            // recabear DURANTE a tomada também é gesto de performance e
            // entra no score (o painel X11 só guardava a topologia do
            // instante zero — mesma adição lá)
            if (rack_.recording.load(std::memory_order_relaxed))
                rack_.score.connection(rack_.recElapsed(), s, sP, d, dP);
        } catch (const std::logic_error&) {          // ciclo
            rack_.graph.disconnect(d, dP);
            try {
                rack_.graph.connect(s, sP, d, dP, /*feedback=*/true);
                rack_.reprepare();
            } catch (...) {
                rack_.graph.disconnect(d, dP);
                rack_.reprepare();
            }
        } catch (...) {
            rack_.graph.disconnect(d, dP);
            rack_.reprepare();
        }
    }

private:
    // O `Panel` fica CACHEADO aqui. `Signal::panel()` devolve por valor e
    // CONSTRÓI a lista de widgets a cada chamada — chamá-lo por módulo a
    // cada quadro (60 módulos × 30 fps) era boa parte da lentidão que o
    // autor sentiu. O painel de um módulo é estático; só muda quando o
    // patch é refeito, e é aí que o `relayout()` o reconstrói.
    struct ModBox { std::size_t id; int hp; juce::Rectangle<int> bounds;
                    Panel panel;
                    // Camada fixa do módulo, rasterizada uma vez (ver
                    // `paintChrome`). Nasce vazia a cada `relayout()`,
                    // que é exatamente quando a geometria muda.
                    juce::Image chrome; };
    enum class Pass { Static, Dynamic };
    struct Drag { bool active = false; std::size_t node = 0; std::string bind;
        float startVal = 0, lo = 0, hi = 1; int startY = 0; };
    struct JackScreen { std::size_t node; int port; bool isOut;
        PortKind kind; int x, y; };
    // `moved`: o módulo só se desloca depois de `kDragSlopPx` de arrasto
    // (ver `mouseDown`) — antes disso o gesto ainda é um clique.
    struct ModDrag { bool active = false; std::size_t id = 0; bool moved = false; };
    static constexpr int kDragSlopPx = 6;
    struct CableHit { std::size_t cable; int x0, y0, x1, y1; };
    enum class InspAct { RelNone, RelRing, RelFold, RelDiff,
                         Amount, Conductance, Gain, Companion, Rupture, Remove };
    struct InspHit { juce::Rectangle<int> bounds; InspAct act; };
    // O inspector guarda a PONTA DE DESTINO do cabo, não o índice dele.
    // Cada porta de entrada aceita um cabo só, então (nó, porta) identifica
    // o cabo de forma estável. Guardar o índice era um bug silencioso e
    // destrutivo: qualquer conexão ou desconexão feita com a caixa aberta
    // desloca os índices, e o clique seguinte editaria a relação — ou
    // romperia — um cabo DIFERENTE do que está na tela.
    struct Inspector { bool open = false;
                       std::size_t tnode = 0, tport = 0;
                       int ax = 0, ay = 0;              // âncora (clique)
                       int bx = 0, by = 0, bw = 0, bh = 0; };
    // A barra AMT/COND é HORIZONTAL: o valor segue a posição X do mouse na
    // trilha desde o primeiro clique — não um delta vertical como o knob
    // genérico de módulo. `relationAmount()`/`conductance()` não são
    // `ParameterDescriptor`, não passam por `setParameterBase`, daí o
    // estado à parte.
    struct CableSlider { bool active = false; int which = 0;
                         int trackX = 0, trackW = 1; };

    // Calculado UMA vez, ao começar o arrasto: a topologia não muda no
    // meio do gesto. Diz quais nós alcançam a saída — é a diferença entre
    // "dá pra ligar aqui" e "aqui você vai OUVIR".
    std::vector<char> dragFeeds_;
    bool dragSourceSilent_ = false;

    struct CableDrag { bool active = false; std::size_t node = 0; int port = 0;
        bool fromOutput = true; PortKind kind = PortKind::Audio;
        int ax = 0, ay = 0; };

    // saco de cabos — os mesmos tons do painel X11 (quentes = áudio,
    // frios = controle), escolhidos por hash determinístico do cabo
    static inline const juce::Colour kCableAudio[4] = {
        juce::Colour(0xffd97b4a), juce::Colour(0xffc96f6f),
        juce::Colour(0xffd9a24a), juce::Colour(0xffb5734f)};
    static inline const juce::Colour kCableCtrl[4] = {
        juce::Colour(0xff5cb0d4), juce::Colour(0xff6f96c9),
        juce::Colour(0xff4aa9b5), juce::Colour(0xff7f8fd9)};

    static const ParameterDescriptor* paramDesc(const Signal& n,
                                                const std::string& b) {
        for (const auto& p : n.parameters())
            if (p.descriptor.id == b) return &p.descriptor;
        return nullptr;
    }

    juce::Rectangle<int> widgetBounds(const ModBox& m, const Widget& w) const {
        const ui::RectMM r = ui::footprintMM(w);
        return {m.bounds.getX() + mmpx(r.x), m.bounds.getY() + mmpx(r.y),
                mmpx(r.w), mmpx(r.h)};
    }

    // quebra o rack em linhas, como o `relayout()` do painel X11.
    //
    // A escala vem da altura do VIEWPORT, não da altura deste componente:
    // a altura do componente É o conteúdo, e o conteúdo depende da escala
    // — tirar a escala dela mesma fecharia um laço (foi o bug que deixou
    // o rack sem rolagem: a altura do conteúdo era calculada tarde demais
    // pro `Viewport` saber que havia o que rolar).
    // Onde o módulo arrastado cai na ordem de `shown`: conta quantos
    // módulos ficam ANTES do cursor (linha acima, ou mesma linha e centro
    // à esquerda). Transliterado do `moduleReorderTo` do painel X11.
    // Onde o módulo arrastado cai na ordem de `shown`.
    //
    // O índice é calculado entre os módulos VISÍVEIS e depois TRADUZIDO
    // pra a lista completa. É isso que faz o gesto funcionar também na
    // vista SAÍDA: o painel X11 desistia dela ("a ordem visível é parcial
    // e a conta não fecha") e simplesmente ignorava o arrasto — o que
    // deixava o músico sem conseguir posicionar um módulo que ele acabou
    // de adicionar ali. A conta fecha ancorando no vizinho visível: o
    // módulo entra imediatamente antes (ou depois) de um módulo que se
    // está vendo, e os invisíveis ficam onde estão, com a ordem relativa
    // intacta.
    void reorderTo(std::size_t id, juce::Point<int> p) {
        // no máximo ~25 reordenações por segundo: `mouseDrag` dispara por
        // evento do mouse, e cada reordenação refaz o layout inteiro
        const auto now = std::chrono::steady_clock::now();
        if (now - lastReorder_ < std::chrono::milliseconds(40)) return;
        lastReorder_ = now;

        const int rowH = modH_ + kPadPx;
        const int cursorRow = std::max(0, (p.y - kPadPx) / std::max(1, rowH));
        int dropIdx = 0;
        for (const auto& m : mods_) {
            if (m.id == id) continue;
            const int mrow = (m.bounds.getY() - kPadPx) / std::max(1, rowH);
            const int mcx = m.bounds.getCentreX();
            if (mrow < cursorRow || (mrow == cursorRow && mcx < p.x)) ++dropIdx;
        }
        // ordem dos VISÍVEIS (sem o arrastado), pra ancorar a tradução
        std::vector<std::size_t> vis;
        vis.reserve(mods_.size());
        for (const auto& m : mods_) if (m.id != id) vis.push_back(m.id);

        std::vector<std::size_t> next;
        next.reserve(rack_.shown.size());
        for (const auto sid : rack_.shown) if (sid != id) next.push_back(sid);

        const auto posOf = [&](std::size_t what) {
            return std::find(next.begin(), next.end(), what) - next.begin();
        };
        std::ptrdiff_t at;
        if (vis.empty())            at = static_cast<std::ptrdiff_t>(next.size());
        else if (dropIdx <= 0)      at = posOf(vis.front());
        else if (dropIdx >= static_cast<int>(vis.size()))
                                    at = posOf(vis.back()) + 1;
        else                        at = posOf(vis[static_cast<std::size_t>(dropIdx)]);
        at = std::max<std::ptrdiff_t>(0, std::min<std::ptrdiff_t>(at,
                 static_cast<std::ptrdiff_t>(next.size())));
        next.insert(next.begin() + at, id);
        if (next != rack_.shown) {
            rack_.shown = next;
            relayout();
        }
    }

    void relayout() {
        const int availH = std::max(120, viewportH_);
        // A escala tem que caber `kTargetRows` fileiras COM o padding —
        // e era isso que faltava. O cálculo antigo dividia a altura pelas
        // fileiras e só descontava um `kPadPx`, ignorando o respiro do
        // topo e o arredondamento de `mmpx`. O conteúdo saía de 1 a 3 px
        // MAIS ALTO que a viewport: o bastante pra barra de rolagem
        // aparecer mostrando exatamente as três fileiras que deviam
        // caber — um pixel de conta errada virando um elemento de
        // interface.
        //
        // `- kTopPadPx` desconta o respiro do topo; `- 0.5f` garante que o
        // arredondamento de `mmpx` caia pra baixo e não devolva o pixel.
        const float rowBudget =
            static_cast<float>(availH - kTopPadPx) / kTargetRows;
        scale_ = juce::jlimit(kSMin, kSMax,
            (rowBudget - kPadPx - 0.5f) / ui::kMM3U) * zoom_;
        modH_ = mmpx(ui::kMM3U);
        mods_.clear();

        // vista SAÍDA: só os módulos que chegam ao sink (o mesmo
        // `nodesFeeding` do painel X11 — é só uma VISTA, não remove nada
        // do patch)
        std::vector<char> feeds;
        if (outputOnly_) {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            feeds = rack_.graph.nodesFeeding(rack_.sink);
            // Um módulo recém-adicionado ainda NÃO chega à saída — ele
            // acabou de nascer sem cabo nenhum. Filtrado pela regra
            // normal, ele simplesmente não aparecia: o músico pedia um
            // módulo e nada acontecia na tela, sendo obrigado a trocar
            // pra vista TODOS pra encontrá-lo. O filtro estava CERTO e
            // ainda assim escondia justamente a coisa que a pessoa acabou
            // de pedir pra existir.
            //
            // Exceção: quem entrou agora fica à vista até chegar à saída.
            // Ela se limpa sozinha — no instante em que o módulo é cabeado
            // até o som, a regra normal passa a mostrá-lo e a exceção sai.
            for (auto it = pending_.begin(); it != pending_.end();) {
                if (*it < feeds.size() && feeds[*it]) it = pending_.erase(it);
                else ++it;
            }
        } else {
            pending_.clear();   // na vista TODOS a exceção não tem sentido
        }

        // Topo SEM o padding cheio: a faixa de crédito já é um componente
        // próprio acima do rack, então somar `kPadPx` aqui empilhava dois
        // espaços e afastava a primeira fileira. No painel X11 o crédito é
        // desenhado DENTRO da faixa de respiro da case (em `kCaseTop+11`,
        // com os módulos começando em `kCasePad`), não acima dela — por
        // isso lá o vão é menor. `kPadPx` segue valendo pras laterais e
        // pro intervalo entre fileiras.
        int x = kPadPx, y = kTopPadPx;
        for (const auto id : rack_.shown) {
            if (outputOnly_ && (id >= feeds.size() || !feeds[id])
                && pending_.count(id) == 0) continue;
            const Panel& p = rack_.panelOf(id);
            const int w = mmpx(ui::panelWidthMM(p.hp));
            if (x + w > viewportW_ - kPadPx && x > kPadPx) {
                x = kPadPx;
                y += modH_ + kPadPx;
            }
            // `{}` explícito pra `chrome`: ela nasce vazia de propósito
            // (é rasterizada no 1º `paintModule`), e deixar implícito
            // fazia o compilador avisar com razão
            mods_.push_back({id, p.hp, {x, y, w, modH_}, p, {}});
            x += w + mmpx(kGapMM);
        }
        contentH_ = y + modH_ + kPadPx;
        rebuildJacks();
    }

    // `bind` de jack ("out:<porta>" / "in:<porta>") → índice de porta e
    // tipo de sinal, a mesma resolução que o painel X11 faz. Fica num só
    // lugar porque dois consumidores precisam CONCORDAR: `rebuildJacks`
    // (onde o jack está) e `paintWidget` (se ele acende como destino
    // válido). Concordar por cópia é como se desalinha.
    static bool resolveJack(const Signal& node, const std::string& bind,
                            bool& isOut, int& port, PortKind& kind) {
        isOut = bind.rfind("out:", 0) == 0;
        if (!isOut && bind.rfind("in:", 0) != 0) return false;
        const std::string name = bind.substr(isOut ? 4 : 3);
        const std::size_t n = isOut ? node.outputCount() : node.inputCount();
        for (std::size_t i = 0; i < n; ++i) {
            const auto& d = isOut ? node.outputDescriptor(i)
                                  : node.inputDescriptor(i);
            if (d.name == name) {
                port = static_cast<int>(i);
                kind = d.kind;
                return true;
            }
        }
        return false;
    }

    // posições de tela de todo jack visível
    void rebuildJacks() {
        jacks_.clear();
        for (const auto& m : mods_) {
            Signal& node = rack_.graph.node(m.id);
            for (const auto& w : m.panel.widgets) {
                if (w.kind != Widget::Kind::Jack) continue;
                bool isOut = false;
                int pidx = -1;
                PortKind kind = PortKind::Audio;
                if (!resolveJack(node, w.bind, isOut, pidx, kind)) continue;
                jacks_.push_back({m.id, pidx, isOut, kind,
                                  m.bounds.getX() + mmpx(w.x),
                                  m.bounds.getY() + mmpx(w.y)});
            }
        }
    }

    int jackAt(juce::Point<int> p) const {
        const int r = mmpx(3.2f) + 2;
        for (std::size_t i = 0; i < jacks_.size(); ++i) {
            const int dx = p.x - jacks_[i].x, dy = p.y - jacks_[i].y;
            if (dx * dx + dy * dy <= r * r) return static_cast<int>(i);
        }
        return -1;
    }
    const JackScreen* findJack(std::size_t n, int port, bool out) const {
        for (const auto& j : jacks_)
            if (j.node == n && j.port == port && j.isOut == out) return &j;
        return nullptr;
    }

public:
    // O dono passa o tamanho VISÍVEL (do viewport); daí sai a escala, e a
    // altura própria deste componente passa a ser a do conteúdo — que é o
    // que faz o `Viewport` saber que há o que rolar.
    void layoutFor(int viewportW, int viewportH) {
        viewportW_ = std::max(200, viewportW);
        viewportH_ = std::max(120, viewportH);
        relayout();
        setSize(viewportW_, std::max(viewportH_, contentH_));
        repaint();
    }
    int contentHeight() const { return contentH_; }
    void setOutputOnly(bool v) { outputOnly_ = v; layoutFor(viewportW_, viewportH_); }
    bool outputOnly() const { return outputOnly_; }
    std::size_t visibleModuleCount() const { return mods_.size(); }

    // Um módulo que acabou de entrar: fica visível mesmo na vista SAÍDA
    // até ser cabeado até o som. Ver `relayout`.
    void markPending(std::size_t id) { pending_.insert(id); }

    // Desplugar na vista SAÍDA: quem chegava ao som antes do corte e deixou
    // de chegar fica à vista POR EXCEÇÃO (borda tracejada), como o módulo
    // recém-adicionado. Sem isto o módulo — e tudo o que o alimentava —
    // sumia da tela no instante do corte: continuava no patch, mas para
    // quem toca era igual a ter sido apagado (relato do autor, 2 out.
    // 2026: "o módulo está sendo removido"). Chamar sob `gmx`, com o
    // `nodesFeeding` tirado ANTES do corte.
    void keepVisibleAfterCut(const std::vector<char>& before) {
        if (!outputOnly_) return;
        const auto after = rack_.graph.nodesFeeding(rack_.sink);
        for (std::size_t i = 0; i < before.size(); ++i)
            if (before[i] && !(i < after.size() && after[i]))
                pending_.insert(i);
    }

    // O rack desenha o inspector de cabo, que tem PROSA (romper /
    // reconectar) — então ele precisa saber o idioma. Não sabia: o
    // inspector inteiro estava fora do sistema de tradução, e os dois
    // botões saíam sempre em português.
    void setLanguage(rasgo::panel::Lang l) { lang_ = l; repaint(); }

    // [Esc] — desiste do gesto em curso: solta o cabo que está sendo
    // puxado, fecha o inspector, cancela a escolha de companion. Devolve
    // true se havia algo pra cancelar. Sem isto, começar a puxar um cabo
    // e mudar de ideia obrigava a soltar em algum lugar inofensivo e
    // torcer — desistir é um gesto legítimo e precisa de tecla.
    bool cancelInteraction() {
        const bool had = cdrag_.active || picking_ || insp_.open || mdrag_.active;
        cdrag_.active = false;
        picking_ = false;
        insp_.open = false;
        mdrag_.active = false;
        pendingInspect_ = -1;
        if (had) repaint();
        return had;
    }

    // Ligações com o resto da janela: a paleta é componente irmão, então
    // quem sabe se o ponteiro está sobre ela (pra soltar = remover) é o
    // MainComponent. `spawn*` desenha o fantasma do módulo que vem da
    // paleta enquanto ele ainda não foi solto.
    std::function<bool(juce::Point<int>)> overPalette;   // pos. de TELA
    std::function<void(std::size_t)> onRemoveModule;
    // clicar no rack devolve o teclado aos atalhos (a caixa de seed é um
    // TextEditor e retém o foco enquanto ninguém o tira dela)
    std::function<void()> onFocusWanted;
    std::function<void(const std::string&)> onNotice;
    // gesto de parâmetro concluído (pra o SYSTEM SCORE da gravação)
    std::function<void(std::size_t, const std::string&, float, float)>
        onParamGesture;

    // Tipo sob o mouse na paleta: o módulo correspondente no rack ganha
    // borda de acento. É a ponte entre a lista e a case — sem ela, achar
    // no rack o módulo que você está lendo na paleta é caça ao tesouro.
    void setPaletteHover(const std::string& t) {
        if (t == palHover_) return;
        palHover_ = t;
        repaint();
    }
    // saída silenciada (STANDBY): os cabos ficam esmaecidos
    void setSilenced(bool v) { if (v != silenced_) { silenced_ = v; repaint(); } }

    void setSpawnGhost(const std::string& type, juce::Point<int> at) {
        spawnType_ = type; spawnAt_ = at; repaint();
    }
    void clearSpawnGhost() { spawnType_.clear(); repaint(); }

    // LEARN sobre o rack: widget sob o mouse → o que AQUELE controle faz;
    // corpo do módulo → o que o MÓDULO é. `key` identifica o objeto pra o
    // dwell saber que o mouse continua no mesmo lugar.
    struct LearnHit { const rasgo::panel::LearnEntry* entry = nullptr;
                      juce::String title; std::string key; };
    LearnHit learnAt(juce::Point<int> p) const {
        // PRIORIDADE, e ela é a MESMA do clique do mouse — não por
        // simetria estética, mas porque foi corrigida aqui depois de eu
        // quebrá-la:
        //
        //   1. widget (knob, slider, toggle, jack) — alvo pequeno e
        //      intencional, ganha sempre;
        //   2. cabo — ganha do CORPO do módulo;
        //   3. corpo/nome do módulo.
        //
        // Em 25 set. 2026 eu pus o cabo em PRIMEIRO lugar, para que a
        // ideia do cabo-objeto deixasse de ficar escondida. Quebrou o
        // LEARN de vários controles de uma vez: os cabos são desenhados
        // POR CIMA dos módulos e a tolerância de acerto é folgada (5 mm +
        // 3 px), então todo knob com um cabo passando perto passou a
        // explicar o cabo em vez de si mesmo. O autor reportou no BODY e
        // disse que "outros itens também".
        //
        // O cabo continua alcançável: sobra todo o comprimento dele que
        // não cruza um widget, que é a maior parte.

        // 1. widget
        for (const auto& m : mods_) {
            if (!m.bounds.contains(p)) continue;
            const std::string mt = rack_.graph.node(m.id).type();
            for (const auto& w : m.panel.widgets) {
                if (w.bind.empty()) continue;
                if (!widgetBounds(m, w).contains(p)) continue;
                // com o IDIOMA, pelo mesmo motivo do verbete de módulo
                // adiante: sem a língua, os 803 verbetes de widget
                // traduzidos ficariam escritos e invisíveis.
                if (const auto* e =
                        rasgo::panel::lookupLearn(mt, w.bind, lang_))
                    return {e, u8(mt) + u8("  \xc2\xb7  ") + u8(w.label),
                            std::to_string(m.id) + "|" + w.bind};
                break;   // widget sem verbete: cai pro módulo, adiante
            }
        }

        // 2. cabo
        if (const int ci = cableUnder(p); ci >= 0)
            return {&rasgo::panel::learnCable(),
                    u8("CABO  \xc2\xb7  clique para abrir"),
                    "cabo|" + std::to_string(ci)};

        // 3. corpo do módulo
        for (const auto& m : mods_) {
            if (!m.bounds.contains(p)) continue;
            const std::string mt = rack_.graph.node(m.id).type();
            // passa o IDIOMA: os verbetes de módulo estão traduzidos
            // desde 27 set. 2026, e chamar sem língua devolveria português
            // a quem escolheu outra
            if (const auto* e = rasgo::panel::lookupLearnModule(mt, lang_))
                return {e, u8(mt), std::to_string(m.id) + "|\x01mod"};
            return {};
        }
        return {};
    }
    // mão do músico num controle ou num cabo — o VARIA não disputa o knob
    // com quem está mexendo nele
    bool interacting() const { return drag_.active || cdrag_.active; }
    void nudgeZoom(int dir) {
        zoom_ = juce::jlimit(0.55f, 1.40f, zoom_ + 0.10f * static_cast<float>(dir));
        layoutFor(viewportW_, viewportH_);
    }
    void refresh() { layoutFor(viewportW_, viewportH_); }

private:
    void paintModule(juce::Graphics& g, ModBox& m) {
        Signal& node = rack_.graph.node(m.id);

        // ---- camada FIXA: rasterizada uma vez, depois só copiada ------
        if (!m.chrome.isValid()
            || m.chrome.getWidth() != m.bounds.getWidth()
            || m.chrome.getHeight() != m.bounds.getHeight()) {
            m.chrome = juce::Image(juce::Image::ARGB, m.bounds.getWidth(),
                                   m.bounds.getHeight(), true);
            juce::Graphics ig(m.chrome);
            // desloca a origem pra o código de desenho seguir usando as
            // MESMAS coordenadas absolutas nas duas passadas
            ig.setOrigin(-m.bounds.getX(), -m.bounds.getY());
            paintChrome(ig, m, node);
        }
        g.drawImageAt(m.chrome, m.bounds.getX(), m.bounds.getY());

        // ---- borda: fora da camada fixa porque muda com hover/arrasto -
        const bool dragging = mdrag_.active && mdrag_.moved && mdrag_.id == m.id;
        const bool palHit = !palHover_.empty() && node.type() == palHover_;
        g.setColour((dragging || palHit) ? T.accent : T.line);
        g.drawRect(m.bounds, 1);
        if (dragging || palHit) g.drawRect(m.bounds.expanded(1), 1);

        // Módulo à vista por EXCEÇÃO (entrou agora e ainda não chega à
        // saída): borda tracejada, pra ficar claro que ele está ali porque
        // é novo e não porque está soando. Sem essa marca a vista SAÍDA
        // passaria a mentir — mostraria algo que não chega ao som sem
        // dizer que é diferente.
        if (outputOnly_ && pending_.count(m.id)) {
            g.setColour(T.accent);
            const float dash[] = {4.0f, 3.0f};
            const auto r = m.bounds.toFloat().reduced(0.5f);
            juce::Path p;
            p.addRectangle(r);
            juce::Path dashed;
            juce::PathStrokeType(1.0f).createDashedStroke(dashed, p, dash, 2);
            g.fillPath(dashed);
        }

        const juce::Graphics::ScopedSaveState clip(g);
        g.reduceClipRegion(m.bounds.reduced(1));
        const bool matrix = node.type() == "MATRIX";
        for (const auto& w : m.panel.widgets) {
            if (matrix && w.kind == Widget::Kind::Knob
                && ui::isMatrixCellBind(w.bind)) continue;
            paintWidget(g, m, node, w, Pass::Dynamic);
        }
        if (matrix) paintMatrix(g, m, node);
    }

    // MATRIX (#33): a grade 4×4 como células clicáveis, não 16 knobs
    // minúsculos — o mesmo desenho do painel X11, agora a partir da
    // célula compartilhada em `ui::matrixCellMM`. Barra a partir do
    // CENTRO: pra cima é ganho positivo, pra baixo negativo, que é o que
    // faz a matriz ser lida de relance.
    void paintMatrix(juce::Graphics& g, const ModBox& m, Signal& node) {
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k) {
                const ui::RectMM rm = ui::matrixCellMM(j, k);
                const int cx = m.bounds.getX() + mmpx(rm.x);
                const int cy = m.bounds.getY() + mmpx(rm.y);
                const int cw = mmpx(rm.w), chh = mmpx(rm.h);
                const char id[4] = {'g', static_cast<char>('1' + j),
                                    static_cast<char>('1' + k), 0};
                float v = 0.0f;
                try { v = node.parameterValue(id); } catch (...) {}

                g.setColour(T.recessed);
                g.fillRect(cx, cy, cw, chh);
                const int mid = cy + chh / 2;
                const int bar = static_cast<int>(v * (chh / 2 - 2));
                if (bar != 0) {
                    g.setColour(v >= 0.0f ? T.accent : kCableCtrl[0]);
                    g.fillRect(cx + 2, bar > 0 ? mid - bar : mid,
                               cw - 4, bar > 0 ? bar : -bar);
                }
                g.setColour(T.line);
                g.drawHorizontalLine(mid, static_cast<float>(cx + 1),
                                     static_cast<float>(cx + cw - 2));
                const bool hot = drag_.active && drag_.node == m.id
                    && drag_.bind.size() == 3 && drag_.bind[1] == id[1]
                    && drag_.bind[2] == id[2];
                g.setColour(hot ? T.accent : T.line);
                g.drawRect(cx, cy, cw, chh, 1);
            }
    }

    // Tudo que NÃO depende de valor nem de interação: fundo, trilhos de
    // parafuso, [x] e a parte fixa de cada widget — inclusive TODOS os
    // rótulos. É esta passada que sai do quadro: no renderizador do JUCE
    // cada `drawText` refaz layout de glifos, e a 30 fps, com centenas de
    // rótulos visíveis, isso dominava o tempo de pintura (era a lentidão
    // que o autor sentia, e o que fazia o cabo arrastado atrasar).
    //
    // O NOME do módulo não é desenhado à parte: cada módulo já o declara
    // no próprio `Panel` como um `Widget::Kind::Label` (ex.: `MIXER` em
    // `src/dsp/Mixer.hpp`). O recorte à área útil garante que rótulo
    // comprido de módulo estreito não invada o vizinho — mesma proteção
    // do `clipTo` do painel X11.
    void paintChrome(juce::Graphics& g, const ModBox& m, Signal& node) {
        g.setColour(T.surface);
        g.fillRect(m.bounds);
        // faixa da família: na camada fixa, então custa zero por quadro
        if (const std::string fam = familyOfType(node.type()); !fam.empty()) {
            g.setColour(familyColour(fam));
            g.fillRect(m.bounds.getX() + 1, m.bounds.getY() + 1,
                       m.bounds.getWidth() - 2, 3);
        }
        g.setColour(T.line);
        // trilhos de parafuso (a "cara" do painel Eurorack), como no X11
        g.drawHorizontalLine(m.bounds.getY() + 3,
                             static_cast<float>(m.bounds.getX() + 2),
                             static_cast<float>(m.bounds.getRight() - 2));
        g.drawHorizontalLine(m.bounds.getBottom() - 3,
                             static_cast<float>(m.bounds.getX() + 2),
                             static_cast<float>(m.bounds.getRight() - 2));
        {   // [x] de remover, canto superior direito — como no painel X11
            const int xx = m.bounds.getRight() - 12, xy = m.bounds.getY() + 5;
            g.setColour(T.textSecondary);
            g.drawLine(static_cast<float>(xx), static_cast<float>(xy),
                       static_cast<float>(xx + 6), static_cast<float>(xy + 6));
            g.drawLine(static_cast<float>(xx + 6), static_cast<float>(xy),
                       static_cast<float>(xx), static_cast<float>(xy + 6));
        }
        const juce::Graphics::ScopedSaveState clip(g);
        g.reduceClipRegion(m.bounds.reduced(1));
        const bool matrix = node.type() == "MATRIX";
        for (const auto& w : m.panel.widgets) {
            // as 16 células da matriz não são knobs: ver `paintMatrix`
            if (matrix && w.kind == Widget::Kind::Knob
                && ui::isMatrixCellBind(w.bind)) continue;
            paintWidget(g, m, node, w, Pass::Static);
        }
        if (matrix) {
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(9.0f));
            g.drawText(u8("IN \xe2\x86\x93   OUT \xe2\x86\x92"),
                       m.bounds.getX() + mmpx(50.0f),
                       m.bounds.getY() + mmpx(16.0f), mmpx(40.0f), mmpx(5.0f),
                       juce::Justification::centredLeft, false);
        }
    }

    void paintWidget(juce::Graphics& g, const ModBox& m, Signal& node,
                     const Widget& w, Pass pass) {
        const bool st = pass == Pass::Static;
        const int wx = m.bounds.getX() + mmpx(w.x);
        const int wy = m.bounds.getY() + mmpx(w.y);
        const float value = [&] {
            if (st || w.bind.empty()) return 0.0f;
            try { return node.parameterValue(w.bind); } catch (...) { return 0.0f; }
        }();
        const ParameterDescriptor* d = (st || w.bind.empty())
            ? nullptr : paramDesc(node, w.bind);
        const float norm = (d && d->maximum > d->minimum)
            ? juce::jlimit(0.0f, 1.0f,
                           (value - d->minimum) / (d->maximum - d->minimum))
            : 0.0f;

        g.setFont(juce::FontOptions(9.0f));
        switch (w.kind) {
        case Widget::Kind::Knob: {
            const int r = mmpx(4.0f);
            const juce::Rectangle<int> box(wx, wy, 2 * r, 2 * r);
            if (st) {
                g.setColour(T.recessed);
                g.fillEllipse(box.toFloat());
                g.setColour(T.line);
                g.drawEllipse(box.toFloat(), 1.0f);
                g.setColour(T.textSecondary);
                g.drawText(w.label, wx - mmpx(2.0f), wy - mmpx(4.0f),
                           2 * r + mmpx(4.0f), mmpx(3.5f),
                           juce::Justification::centred, false);
                break;
            }
            // ponteiro: -135° a +135°, como o painel X11
            const float a = juce::degreesToRadians(-135.0f + 270.0f * norm);
            const auto c = box.getCentre().toFloat();
            g.setColour(T.accent);
            g.drawLine(c.x, c.y, c.x + std::sin(a) * static_cast<float>(r) * 0.8f,
                       c.y - std::cos(a) * static_cast<float>(r) * 0.8f, 2.0f);
            break;
        }
        case Widget::Kind::Slider: {
            const juce::Rectangle<int> box(wx, wy, mmpx(6.0f), mmpx(30.0f));
            if (st) {
                g.setColour(T.recessed);
                g.fillRect(box);
                g.setColour(T.line);
                g.drawRect(box, 1);
                g.setColour(T.textSecondary);
                g.drawText(w.label, box.getX() - mmpx(2.0f), box.getBottom(),
                           box.getWidth() + mmpx(4.0f), mmpx(3.5f),
                           juce::Justification::centred, false);
                break;
            }
            const int fill = juce::roundToInt(box.getHeight() * norm);
            if (fill > 0) {
                g.setColour(T.accent);
                g.fillRect(box.getX() + 1, box.getBottom() - fill,
                           box.getWidth() - 2, fill - 1);
            }
            break;
        }
        case Widget::Kind::Toggle: {
            const int s = mmpx(4.0f);
            const juce::Rectangle<int> box(wx, wy, s, s);
            if (st) {
                g.setColour(T.textSecondary);
                g.drawText(w.label, box.getRight() + 2, wy, mmpx(12.0f), s,
                           juce::Justification::centredLeft, false);
                break;
            }
            g.setColour(value >= 0.5f ? T.accent : T.recessed);
            g.fillRect(box);
            g.setColour(T.line);
            g.drawRect(box, 1);
            break;
        }
        case Widget::Kind::Jack: {
            // (não há mais `isOut` aqui: o anel base passou a depender só
            // do halo quando alinhei a cor ao painel X11 — saída e entrada
            // em repouso usam o mesmo `T.line`)
            const int r = mmpx(2.4f);
            const juce::Rectangle<float> box(static_cast<float>(wx - r),
                                             static_cast<float>(wy - r),
                                             static_cast<float>(2 * r),
                                             static_cast<float>(2 * r));

            // Afordância de cabeamento (a mesma do painel X11): ao puxar
            // um cabo, os destinos de polaridade OPOSTA acendem — halo
            // duplo quando o tipo de sinal casa (áudio→áudio,
            // controle→controle), simples quando cruza domínio; os
            // inválidos apagam. É o que ensina onde dá pra ligar.
            if (st) {
                g.setColour(T.recessed);
                g.fillEllipse(box);
                g.setColour(T.textSecondary);
                g.drawText(w.label, wx - mmpx(5.0f), wy - mmpx(6.5f),
                           mmpx(10.0f), mmpx(3.5f),
                           juce::Justification::centred, false);
                break;
            }

            int halo = 0;   // 0 nada · 1 forte · 2 válido · 3 apagado
            if (cdrag_.active) {
                bool jo = false;
                int jp = -1;
                PortKind jk = PortKind::Audio;
                if (!resolveJack(node, w.bind, jo, jp, jk))
                    halo = 3;
                else if (jo == cdrag_.fromOutput)
                    halo = 3;   // mesma polaridade: saída→saída não existe
                else
                    halo = (jk == cdrag_.kind) ? 1 : 2;
            }
            // DOIS canais visuais independentes, e a distinção entre eles
            // é o ponto: o halo diz "dá pra ligar", o brilho diz "você vai
            // OUVIR".
            //
            //  · quantidade de anéis = TIPO de sinal casa (duplo) ou cruza
            //    domínio (simples) — como sempre foi;
            //  · brilho = o destino alcança a saída. Cheio: ligar aqui soa
            //    agora. Apagado: a ligação é válida e legítima, mas este
            //    caminho ainda não chega ao som.
            //
            // O apagado NÃO é aviso de erro. Construir uma cadeia inteira
            // longe da saída e ligá-la ao som por último é um jeito
            // legítimo de trabalhar — o que faltava era distinguir isso de
            // um engano, que até agora eram indistinguíveis.
            if (halo == 1 || halo == 2) {
                const bool soa = audibleTarget(m.id);
                const float hr = static_cast<float>(r + mmpx(halo == 1 ? 2.4f : 1.6f));
                const auto c = box.getCentre();

                // A distinção é de FORMA, não só de brilho: contínuo = vai
                // soar, tracejado = ainda não chega ao som. Só a diferença
                // de alfa era fraca demais pra ler como duas categorias —
                // o autor olhou e não distinguiu. Forma também sobrevive a
                // monitor ruim e a quem enxerga cor de outro jeito.
                const auto ring = [&](float rad, float w) {
                    if (soa) {
                        g.setColour(T.accent);
                        g.drawEllipse(c.x - rad, c.y - rad, 2 * rad, 2 * rad, w);
                        return;
                    }
                    juce::Path p;
                    p.addEllipse(c.x - rad, c.y - rad, 2 * rad, 2 * rad);
                    const float dash[] = {3.0f, 3.0f};
                    juce::Path d;
                    juce::PathStrokeType(w).createDashedStroke(d, p, dash, 2);
                    g.setColour(T.accent.withAlpha(0.55f));
                    g.fillPath(d);
                };
                ring(hr, soa ? 1.8f : 1.2f);
                if (halo == 1) ring(hr + 2.0f, soa ? 1.8f : 1.2f);

                // destino que SOA ganha ainda um miolo aceso — a leitura
                // "vai dar som" tem que ser instantânea
                if (soa) {
                    g.setColour(T.accent.withAlpha(0.28f));
                    g.fillEllipse(box);
                }
            }

            // Jack INVÁLIDO durante o cabeamento continua com o anel
            // visível, só recuado. Apagá-lo de todo (era `T.recessed`, o
            // mesmo tom do miolo — e é o que o painel X11 fazia) sumia com
            // o jack: você perdia o mapa do painel justo no momento em que
            // está mirando. Guiar é destacar o válido, não cegar o resto.
            // Recuado em direção à SUPERFÍCIE DO MÓDULO, que é o que
            // está atrás do jack — não em direção ao fundo da janela.
            // Misturar com `T.bg` dava #2b323a, a cinco unidades da
            // superfície (#262b36): o anel sumia por completo. É preciso
            // mirar no que está atrás, não no fundo geral.
            g.setColour(halo == 3 ? T.line.interpolatedWith(T.surface, 0.45f)
                                  : (halo ? T.accent : T.line));
            g.drawEllipse(box, 1.5f);
            break;
        }
        case Widget::Kind::Display:
            paintDisplay(g, m, node, w, wx, wy, st);
            break;
        case Widget::Kind::Label: {
            if (!st) break;
            // O NOME do módulo é o primeiro Label do painel e vale como
            // título: entra maior e em negrito, no tom primário. Os demais
            // Labels (rótulos de seção) seguem discretos. Sem isso o nome
            // se perdia no meio dos rótulos de controle — pedido do autor.
            const bool isTitle = !m.panel.widgets.empty()
                && &w == &m.panel.widgets.front();
            g.setColour(isTitle ? T.textPrimary : T.textSecondary);
            g.setFont(juce::FontOptions(isTitle ? 12.0f : 10.0f,
                                        isTitle ? juce::Font::bold
                                                : juce::Font::plain));
            g.drawText(w.label, wx, wy, mmpx(24.0f),
                       mmpx(isTitle ? 4.2f : 3.5f),
                       juce::Justification::centredLeft, false);
            g.setFont(juce::FontOptions(10.0f));
            break;
        }
        }
    }

    // ---- os `Display` -------------------------------------------------
    // Quatro gráficos diferentes no mesmo retângulo, conforme o módulo —
    // exatamente os do painel X11. Os dados vêm de `rack_.scopeSnap`,
    // cópia sem bloqueio do anel que o thread de áudio enche.
    void paintDisplay(juce::Graphics& g, const ModBox& m, Signal& node,
                      const Widget& w, const int wx, const int wy,
                      const bool staticPass) {
        int dw = mmpx(w.span > 1.0f ? w.span : 16.0f);
        dw = std::min(dw, m.bounds.getWidth() - mmpx(w.x) - mmpx(2.0f));
        const int dh = mmpx(16.0f);
        const juce::Rectangle<int> box(wx, wy, dw, dh);
        if (staticPass) {      // só a moldura; o conteúdo anima
            g.setColour(T.recessed);
            g.fillRect(box);
            g.setColour(T.line);
            g.drawRect(box, 1);
            return;
        }
        g.setColour(T.recessed);
        g.fillRect(box.reduced(1));

        const std::string mtype = node.type();
        const auto si = rack_.scopeSnap.find(m.id);
        juce::String dlabel = w.label;

        if (si != rack_.scopeSnap.end() && dw > 6) {
            const rasgo::ui::ScopeTrace& sc = si->second;

            if (mtype == "MASTER") {
                // VU com clip-latch — "o medidor esconde estouros" (achado
                // §3.3 da auditoria de saída, NAVALHA 2). A barra é o pico
                // da janela recente contra o teto de −1 dBFS (0,891); o
                // indicador vermelho acende quando o LIMITADOR de verdade
                // teve que segurar algo (`gainReductionDb()`), não quando o
                // pico bruto passa de um número — e decai após ~2 s.
                const float peak = sc.peak();
                const float frac = std::min(1.0f, peak / 0.891f);
                const int barW = static_cast<int>(frac * (dw - 2));
                if (barW > 0) {
                    g.setColour(frac < 0.7f ? T.accent : T.warning);
                    g.fillRect(wx + 1, wy + 1, barW, dh - 2);
                }
                if (auto* mst = dynamic_cast<rasgo::modular::Master*>(&node))
                    if (mst->gainReductionDb() > 0.05f)
                        clipSeen_[m.id] = std::chrono::steady_clock::now();
                const auto ic = clipSeen_.find(m.id);
                if (ic != clipSeen_.end()
                    && std::chrono::steady_clock::now() - ic->second
                       < std::chrono::seconds(2)) {
                    g.setColour(T.warning);
                    g.fillRect(wx + dw - 7, wy + 1, 6, dh - 2);
                }
                dlabel = juce::String(20.0f * std::log10(std::max(peak, 1.0e-4f)),
                                      0) + "dB";

            } else if (mtype == "TRIGSEQ") {
                // 4 lanes de gate (t1–t4) rolando — piano-roll
                dlabel = "t1-t4";
                const int nL = static_cast<int>(rasgo::ui::kLaneLen);
                const int lh = std::max(2, (dh - 2) / 4);
                const int cellw = std::max(1, (dw - 2) / nL);
                for (int L = 0; L < 4; ++L) {
                    const int ly = wy + 1 + L * lh;
                    g.setColour(T.line);
                    g.drawHorizontalLine(ly + lh - 1, static_cast<float>(wx + 1),
                                         static_cast<float>(wx + dw - 2));
                    g.setColour(T.accent);
                    for (int k = 0; k < nL; ++k) {
                        if (sc.lane(static_cast<std::size_t>(L),
                                    static_cast<std::size_t>(k)) < 0.5f) continue;
                        g.fillRect(wx + 1 + k * (dw - 2) / nL, ly + 1,
                                   cellw, lh - 2);
                    }
                }

            } else if (scopeView_[m.id] == 1) {
                // espectro: banco Goertzel log de 24 bandas
                dlabel = "spec";
                constexpr int nb = 24;
                float mag[nb];
                rasgo::ui::scopeSpectrum(sc, mag, nb,
                                         rasgo::ui::kLegacySpectrumRateHz);
                float mmax = 1.0e-6f;
                for (int b = 0; b < nb; ++b) mmax = std::max(mmax, mag[b]);
                const int bw = std::max(1, (dw - 2) / nb);
                g.setColour(T.accent);
                for (int b = 0; b < nb; ++b) {
                    const float db = 20.0f * std::log10(mag[b] / mmax + 1.0e-6f);
                    const float t = juce::jlimit(0.0f, 1.0f, 1.0f + db / 54.0f);
                    const int hh = static_cast<int>(t * (dh - 3));
                    if (hh > 0)
                        g.fillRect(wx + 1 + b * bw, wy + dh - 1 - hh,
                                   std::max(1, bw - 1), hh);
                }

            } else {
                // onda (padrão) — osciloscópio da saída 0, normalizado
                const float norm = 0.92f / std::max(sc.peak(), 1.0e-4f);
                const int n = static_cast<int>(sc.buf.size());
                g.setColour(T.line);
                g.drawHorizontalLine(wy + dh / 2, static_cast<float>(wx + 1),
                                     static_cast<float>(wx + dw - 1));
                juce::Path p;
                for (int k = 0; k < n; ++k) {
                    const float v = sc.at(static_cast<std::size_t>(k)) * norm;
                    const float px = static_cast<float>(
                        wx + 1 + k * (dw - 2) / std::max(1, n - 1));
                    const float py = static_cast<float>(wy + dh / 2)
                        - v * static_cast<float>(dh / 2 - 1);
                    if (k == 0) p.startNewSubPath(px, py);
                    else        p.lineTo(px, py);
                }
                g.setColour(T.accent);
                g.strokePath(p, juce::PathStrokeType(1.0f));
            }
        }

        g.setColour(T.textSecondary);
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(dlabel, wx + 3, wy + dh - 12, dw - 6, 11,
                   juce::Justification::bottomLeft, false);
    }

    Rack& rack_;
    std::vector<ModBox> mods_;
    std::vector<JackScreen> jacks_;
    struct Pan { bool active = false; int startY = 0, startView = 0; };
    Pan pan_;
    ModDrag mdrag_;
    juce::Point<int> downAt_;
    int pendingInspect_ = -1;   // clique-ou-arrasto ainda indeciso
    int hoverCable_ = -1;       // cabo sob o mouse (índice no `cableSnap`)
    bool inspectEnvDone_ = false;   // ver `RASGO_INSPECIONAR` em `paintInspector`
    std::chrono::steady_clock::time_point lastReorder_{};
    std::string palHover_;
    bool silenced_ = false;
    std::string spawnType_;
    juce::Point<int> spawnAt_;
    std::vector<CableHit> cableHits_;
    std::vector<InspHit> inspHits_;
    Inspector insp_;
    CableSlider cslide_;
    bool picking_ = false;                   // escolhendo companion
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
    std::set<std::size_t> pending_;   // à vista por exceção
    std::map<std::size_t, int> scopeView_;   // SCOPE: 0 = onda, 1 = espectro
    std::map<std::size_t, std::chrono::steady_clock::time_point> clipSeen_;
    Drag drag_;
    CableDrag cdrag_;
    juce::Point<int> mouse_;
    float scale_ = 2.0f;
    float zoom_ = 1.0f;
    bool outputOnly_ = false;
    int modH_ = 0;
    int contentH_ = 0;
    int viewportW_ = 1000, viewportH_ = 600;
};

// ---- faixa de crédito --------------------------------------------------
// Padrão da família RASGO (cf. Antitotem, Rasgo Synth). No Modular ela vai
// na faixa acima da 1ª fileira de módulos, alinhada à direita, com o ano na
// frase e o carimbo de build ao fim — não no rodapé.
class CreditsStrip : public juce::Component {
public:
    static constexpr int kHeight = 16;
    CreditsStrip() { setOpaque(true); setInterceptsMouseClicks(false, false); }
    void setLanguage(rasgo::panel::Lang l) { lang_ = l; repaint(); }
    void paint(juce::Graphics& g) override {
        g.fillAll(T.bg);
        g.setColour(T.textSecondary);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(u8(rasgo::panel::tr(rasgo::panel::strings::footerCredit,
                                       lang_))
                       + u8(RASGO_MODULAR_BUILD),
                   0, 0, getWidth() - 10, kHeight,
                   juce::Justification::centredRight, false);
    }
private:
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
};

// ---- overlays TUTORIAL e SOBRE -----------------------------------------
// Véu escuro sobre o rack (os módulos continuam à vista) e um cartão
// centrado. Qualquer clique ou [Esc] fecha — igual ao painel X11. O
// tutorial é rolável; o SOBRE cabe num cartão baixo.
class OverlayView : public juce::Component {
public:
    enum class Mode { none, tutorial, about };

    // Tamanhos de texto dos overlays. 10 px servia pra rótulo de painel,
    // não pra LEITURA CORRIDA — tutorial e SOBRE são textos que a pessoa
    // lê de fato, e ficavam pequenos demais (relato do autor).
    static constexpr float kBodyPt  = 13.0f;
    static constexpr float kCardPt  = 14.0f;
    static constexpr float kTitlePt = 16.0f;

    OverlayView() { setWantsKeyboardFocus(false); }

    void show(Mode m, rasgo::panel::Lang l) {
        mode_ = m; lang_ = l; scroll_ = 0;
        setVisible(m != Mode::none);
        repaint();
    }
    void close() { mode_ = Mode::none; setVisible(false); }
    Mode mode() const { return mode_; }
    void scrollBy(int dy) {
        scroll_ = juce::jlimit(0, std::max(0, contentH_ - viewH_), scroll_ + dy);
        repaint();
    }
    void scrollTo(int y) { scroll_ = y; scrollBy(0); }

    void paint(juce::Graphics& g) override {
        g.fillAll(T.bg.withAlpha(0.82f));       // véu
        const int cw = std::min(820, getWidth() - 60);
        const int chh = std::min(getHeight() - 60,
                                 mode_ == Mode::tutorial ? getHeight() - 60 : 330);
        const int cx = (getWidth() - cw) / 2, cy = (getHeight() - chh) / 2;
        const juce::Rectangle<int> card(cx, cy, cw, chh);
        g.setColour(T.surface);
        g.fillRect(card);
        g.setColour(T.accent);
        g.drawRect(card, 1);

        namespace S = rasgo::panel::strings;
        // FECHAR é decorativo — o clique fecha em qualquer lugar
        {
            g.setFont(juce::FontOptions(11.0f));
            const juce::String cl = str(S::close);
            const int w = juce::roundToInt(juce::GlyphArrangement::getStringWidth(
                              g.getCurrentFont(), cl)) + 14;
            g.setColour(T.line);
            g.drawRect(cx + cw - w - 12, cy + 10, w, 20, 1);
            g.setColour(T.textSecondary);
            g.drawText(cl, cx + cw - w - 12, cy + 10, w, 20,
                       juce::Justification::centred, false);
        }

        const int px = cx + 24, wrapW = cw - 56;
        const int top = cy + 30;

        if (mode_ == Mode::tutorial) {
            g.setColour(T.accent);
            g.setFont(juce::FontOptions(kTitlePt));
            g.drawText(str(S::tutTitle), px, top, wrapW, 18,
                       juce::Justification::topLeft, false);
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(kBodyPt));
            g.drawText(str(S::tutSubtitle), px, top + 20, wrapW, 17,
                       juce::Justification::topLeft, false);

            const int viewTop = top + 42;
            viewH_ = cy + chh - 16 - viewTop;
            scroll_ = juce::jlimit(0, std::max(0, contentH_ - viewH_), scroll_);
            g.saveState();
            g.reduceClipRegion({cx + 1, viewTop, cw - 2, viewH_});
            int py = viewTop + 4 - scroll_;
            static const rasgo::panel::L4* const cards[][2] = {
                {&S::tutWhatTitle,    &S::tutWhatBody},
                {&S::tutSeedTitle,    &S::tutSeedBody},
                {&S::tutSeedBoxTitle, &S::tutSeedBoxBody},
                {&S::tutVaryTitle,    &S::tutVaryBody},
                {&S::tutStoreTitle,   &S::tutStoreBody},
                {&S::tutRecTitle,     &S::tutRecBody},
                {&S::tutHdrTitle,     &S::tutHdrBody},
                {&S::tutCableTitle,   &S::tutCableBody},
                {&S::tutNavTitle,     &S::tutNavBody},
                {&S::tutKeysTitle,    &S::tutKeysBody},
                {&S::tutModTitle,     &S::tutModBody},
                {&S::tutScratchTitle, &S::tutScratchBody},
                {&S::tutFamTitle,     &S::tutFamBody},
                {&S::tutLearnTitle,   &S::tutLearnBody},
            };
            for (const auto& c : cards) {
                g.setColour(T.accent);
                g.setFont(juce::FontOptions(kCardPt));
                g.drawText(str(*c[0]), px, py, wrapW, 18,
                           juce::Justification::topLeft, false);
                py += 21;
                py += drawWrapped(g, str(*c[1]), px, py, wrapW, T.textSecondary);
                py += 12;
            }
            contentH_ = (py + scroll_) - (viewTop + 4);
            g.restoreState();

            if (contentH_ > viewH_) {
                const int thH = std::max(24, viewH_ * viewH_ / contentH_);
                const int thY = viewTop
                    + (viewH_ - thH) * scroll_ / (contentH_ - viewH_);
                g.setColour(T.line);
                g.fillRect(cx + cw - 7, thY, 4, thH);
            }
        } else {
            g.saveState();
            g.reduceClipRegion({cx + 1, cy + 1, cw - 2, chh - 2});
            int py = top;
            g.setColour(T.accent);
            g.setFont(juce::FontOptions(kTitlePt));
            g.drawText("RASGO MODULAR", px, py, wrapW, 18,
                       juce::Justification::topLeft, false);
            py += 21;
            g.setColour(T.textSecondary);
            g.setFont(juce::FontOptions(kBodyPt));
            g.drawText(u8(RASGO_MODULAR_BUILD)
                           + u8("  \xc2\xb7  ")
                           + str(rasgo::panel::strings::builtOn) + u8(" ")
                           + u8(__DATE__ " " __TIME__),
                       px, py, wrapW, 17, juce::Justification::topLeft, false);
            py += 24;
            if (onLoudness) {
                const auto lu = onLoudness();
                g.setColour(T.textSecondary);
                g.setFont(juce::FontOptions(kBodyPt));
                g.drawText(u8("LUFS   M ") + lufsText(lu.m)
                               + u8("   S ") + lufsText(lu.s)
                               + u8("   I ") + lufsText(lu.i)
                               + u8("   (BS.1770-4)"),
                           px, py, wrapW, 17,
                           juce::Justification::topLeft, false);
                py += 19;

                // FORMATO REAL da saída, não o presumido. O app adota a
                // taxa do dispositivo em vez de impor uma; sem esta linha
                // a única forma de saber em que taxa se está tocando era
                // ler o código.
                // "blocos perdidos" em cor de aviso quando há algum: é o
                // sinal de que a interface está atropelando o áudio, e a
                // causa provável de estalo. Zero é o esperado.
                if (lu.starved > 0) g.setColour(T.warning);
                g.drawText(juce::String(juce::roundToInt(lu.sr)) + u8(" Hz")
                               + u8("   \xc2\xb7   REC 24 bits PCM")
                               + u8("   \xc2\xb7   blocos perdidos ")
                               + juce::String((int)lu.starved),
                           px, py, wrapW, 17,
                           juce::Justification::topLeft, false);
                g.setColour(T.textSecondary);
                py += 19;

                // Distância até o alvo declarado (streaming). Os dois
                // números que importam estão aqui: o quanto falta em LU
                // para −14, e se o true-peak já passou de −1 dBTP — que é
                // o que estoura na recodificação e NÃO aparece no pico de
                // amostra. Nada disto normaliza nada; é leitura para
                // quem está ouvindo decidir.
                using LM = rasgo::modular::LoudnessMeter;
                const bool overTp = lu.tp > LM::kTargetDbtp;
                g.setColour(overTp ? T.warning : T.textSecondary);
                juce::String alvo = u8("alvo  ")
                    + juce::String(LM::kTargetLufs, 0) + u8(" LUFS / ")
                    + juce::String(LM::kTargetDbtp, 0) + u8(" dBTP   \xc2\xb7   TP ")
                    + (lu.tp <= -119.0f ? u8("--")
                                        : juce::String(lu.tp, 1) + u8(" dBTP"));
                if (lu.i > LM::kSilence) {
                    const float d = LM::kTargetLufs - lu.i;
                    alvo += u8("   \xc2\xb7   ")
                          + juce::String(d >= 0.0f ? "+" : "")
                          + juce::String(d, 1) + u8(" LU");
                }
                g.drawText(alvo, px, py, wrapW, 17,
                           juce::Justification::topLeft, false);
                g.setColour(T.textSecondary);
                py += 26;
            }
            drawWrapped(g, str(S::aboutBody), px, py, wrapW, T.textSecondary);
            g.restoreState();
        }
    }

    void mouseDown(const juce::MouseEvent&) override {
        close();
        if (onClose) onClose();
    }
    void mouseWheelMove(const juce::MouseEvent&,
                        const juce::MouseWheelDetails& w) override {
        scrollBy(-juce::roundToInt(w.deltaY * 60.0f));
    }

    std::function<void()> onClose;
    // `tp` = true-peak em dBTP; `sr` = a taxa REAL do dispositivo, que o
    // app adota em vez de impor (foi a pergunta do autor em 21 set. 2026:
    // "a saída é 48 kHz?" — e a resposta só podia vir da tela, porque
    // depende do dispositivo).
    struct Lufs { float m, s, i, tp; double sr; unsigned starved; };
    std::function<Lufs()> onLoudness;

private:
    static juce::String lufsText(const float v) {
        return v <= rasgo::modular::LoudnessMeter::kSilence
            ? juce::String("--") : juce::String(v, 1);
    }
    juce::String str(const rasgo::panel::L4& s) const {
        return u8(rasgo::panel::tr(s, lang_));
    }
    // devolve a altura consumida — os corpos têm quebras de parágrafo
    static int drawWrapped(juce::Graphics& g, const juce::String& text,
                           int x, int y, int w, juce::Colour c) {
        juce::AttributedString as;
        as.append(text, juce::FontOptions(kBodyPt), c);
        juce::TextLayout tl;
        tl.createLayout(as, static_cast<float>(w));
        tl.draw(g, {static_cast<float>(x), static_cast<float>(y),
                    static_cast<float>(w), tl.getHeight()});
        return static_cast<int>(tl.getHeight());
    }

    Mode mode_ = Mode::none;
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
    int scroll_ = 0, contentH_ = 0, viewH_ = 1;
};

// ---- áudio + janela ----------------------------------------------------
class MainComponent : public juce::AudioAppComponent,
                      private juce::Timer,
                      private juce::MidiInputCallback {
public:
    MainComponent() {
        loadPrefs();
        rack_.build();
        rack_.allocScopes();
        rack_.populateMotion();
        view_ = std::make_unique<RackView>(rack_);
        viewport_.setViewedComponent(view_.get(), false);
        viewport_.setScrollBarsShown(true, false);
        addAndMakeVisible(viewport_);
        addAndMakeVisible(palette_);
        addAndMakeVisible(header_);

        header_.onSeed = [this] {
            rack_.pushUndo();
            seed_ = nextRandomSeed();
            rack_.curSeed = seed_;
            rack_.applySeed(seed_, sampleRate_, blockSize_);
            rack_.populateMotion();
            view_->refresh();
            syncHeader();
        };
        header_.onLang = [this] {
            lang_ = rasgo::panel::nextLang(lang_);
            saveLangPref();
            palette_.setLanguage(lang_);
            credits_.setLanguage(lang_);
            view_->setLanguage(lang_);
            if (overlay_.mode() != OverlayView::Mode::none)
                overlay_.show(overlay_.mode(), lang_);
            syncHeader();
        };
        header_.onRackView = [this] {
            view_->setOutputOnly(!view_->outputOnly());
            saveRackViewPref();
            syncHeader();
        };
        header_.onStandby = [this] {
            standby_ = !standby_;
            view_->setSilenced(standby_);
            std::lock_guard<std::mutex> lk(rack_.gmx);
            for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                if (rack_.graph.node(i).type() == "MASTER")
                    rack_.graph.setParameterBase(i, "mute", standby_ ? 1.0f : 0.0f);
            syncHeader();
        };
        header_.onZoom = [this](int dir) {
            view_->nudgeZoom(dir);
            layoutRack();
        };

        // ---- VARIA e as operações genéticas ----------------------------
        // O código já existia, framework-free, em `panel/MotionEngine.hpp` e
        // `panel/PatchGenetics.hpp` — o que faltava aqui era só a fiação.
        header_.onVary = [this] {
            rack_.motionOn = !rack_.motionOn;
            syncHeader();
        };
        header_.onMutate = [this] {
            rack_.pushUndo();
            // reamostra ~25% dos parâmetros não-estruturais dos nós
            // alcançados por cabo, uma vez
            std::unordered_set<std::size_t> frozen;
            {
                std::lock_guard<std::mutex> lk(rack_.gmx);
                rack_.freezeMixMaster(frozen);
                rasgo::panel::mutatePatch(rack_.graph, nextRandomSeed(),
                                          0.25f, frozen);
            }
            rack_.populateMotion();
        };
        header_.onEvolve = [this] {
            rack_.pushUndo();
            // mesmo destino de um MUTA grande, em 6 passos de 12% —
            // transição gradual em vez de salto
            std::unordered_set<std::size_t> frozen;
            {
                std::lock_guard<std::mutex> lk(rack_.gmx);
                rack_.freezeMixMaster(frozen);
                rasgo::panel::evolvePatch(rack_.graph, nextRandomSeed(),
                                          6, 0.12f, frozen);
            }
            rack_.populateMotion();
        };
        header_.onCross = [this] {
            rack_.pushUndo();
            // recombina com um DOADOR novo: o catálogo inteiro semeado
            // com um seed fresco. O doador é um grafo solto — nunca
            // processa áudio, só é lido por `crossPatch`.
            SignalGraph donor;
            for (const auto& fam : rasgo::panel::moduleCatalog())
                for (const char* t : fam.types)
                    if (auto n = rasgo::panel::makeModule(t))
                        donor.add(std::move(n));
            rasgo::panel::seedPatch(donor, nextRandomSeed());
            std::unordered_set<std::size_t> frozen;
            {
                std::lock_guard<std::mutex> lk(rack_.gmx);
                rack_.freezeMixMaster(frozen);
                rasgo::panel::crossPatch(rack_.graph, donor, nextRandomSeed(),
                                         0.5f, frozen);
            }
            rack_.populateMotion();
        };
        header_.onSeedTyped = [this](std::uint64_t v) {
            rack_.pushUndo();
            seed_ = v;
            rack_.curSeed = v;
            rack_.applySeed(v, sampleRate_, blockSize_);
            rack_.populateMotion();
            view_->refresh();
            syncHeader();
            grabKeyboardFocus();   // devolve o teclado aos atalhos
        };
        header_.onRec = [this] { toggleRec(); };
        header_.onOpen = [this] { openPatchDialog(); };
        header_.onUndo = [this] { undoLast(); grabKeyboardFocus(); };
        // REPOR — volta o patch ao estado ORIGINAL do seed atual, jogando
        // fora toda a edição manual de uma vez. Diferente do desfazer, que
        // anda um passo: aqui o destino é conhecido e não depende de
        // quantas alterações houve pelo caminho. Entra no desfazer, então
        // repor não é irreversível.
        header_.onRestore = [this] {
            const std::uint64_t s = rack_.curSeed;
            if (s == 0) return;
            rack_.pushUndo();
            rack_.applySeed(s, sampleRate_, blockSize_);
            rack_.curSeed = s;
            rack_.populateMotion();
            syncSignalIn();
            view_->refresh();
            syncHeader();
            grabKeyboardFocus();
        };
        header_.onUncable = [this] { clearCablesAndRefresh(); grabKeyboardFocus(); };
        header_.onSave = [this] { savePatch(dataDir().getChildFile("session.rmp")); };
        header_.onBank = [this] {
            // se o músico gostou de um seed, BANCO registra o patch num
            // arquivo próprio — não sobrescreve a sessão
            auto dir = dataDir().getChildFile("patches");
            dir.createDirectory();
            const juce::String name = rack_.curSeed
                ? "seed-" + juce::String(static_cast<juce::int64>(rack_.curSeed))
                : "patch-" + juce::String(juce::Time::currentTimeMillis() / 1000);
            savePatch(dir.getChildFile(name + ".rmp"));
        };

        header_.onTutorial = [this] { toggleOverlay(OverlayView::Mode::tutorial); };
        header_.onAbout    = [this] { toggleOverlay(OverlayView::Mode::about); };

        // ---- arrastar módulo: remover, e adicionar da paleta -----------
        view_->overPalette = [this](juce::Point<int> screenPos) {
            return palette_.getScreenBounds().contains(screenPos);
        };
        view_->onRemoveModule = [this](std::size_t id) { removeModule(id); };
        view_->onFocusWanted = [this] { grabKeyboardFocus(); };
        view_->onNotice = [this](const std::string& t) {
            palette_.setNotice(u8(t));
        };
        view_->onParamGesture = [this](std::size_t node, const std::string& bind,
                                       float from, float to) {
            if (!rack_.recording.load()) return;
            std::lock_guard<std::mutex> lk(rack_.gmx);
            rack_.score.parameterChange(rack_.recElapsed(), node, bind, from, to);
        };

        palette_.onSpawnDrag = [this](const std::string& t,
                                      juce::Point<int> screenPos) {
            if (viewport_.getScreenBounds().contains(screenPos))
                view_->setSpawnGhost(t, view_->getLocalPoint(nullptr, screenPos));
            else
                view_->clearSpawnGhost();
        };
        palette_.onSpawnDrop = [this](const std::string& t,
                                      juce::Point<int> screenPos) {
            view_->clearSpawnGhost();
            if (viewport_.getScreenBounds().contains(screenPos)) addModule(t);
        };

        addAndMakeVisible(credits_);
        addChildComponent(overlay_);
        overlay_.onClose = [this] { header_.setOverlay(0); grabKeyboardFocus(); };
        header_.onVaryAmount = [this](const float v) {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            rack_.motion.setIntensity(v);
        };
        header_.setVaryAmount(rack_.motion.intensity());

        overlay_.onLoudness = [this] {
            return OverlayView::Lufs{
                rack_.lufsM.load(std::memory_order_relaxed),
                rack_.lufsS.load(std::memory_order_relaxed),
                rack_.lufsI.load(std::memory_order_relaxed),
                rack_.lufsTP.load(std::memory_order_relaxed),
                sampleRate_,
                rack_.starvedBlocks.load(std::memory_order_relaxed)};
        };

        if (rackOutputPref_) view_->setOutputOnly(true);
        palette_.setLanguage(lang_);
        credits_.setLanguage(lang_);
        view_->setLanguage(lang_);
        syncHeader();
        setWantsKeyboardFocus(true);
        // Abertura pela MESMA política do painel X11
        // (`panel/WindowPolicy.hpp`, geometria pura — o comentário do
        // próprio header diz "sem X11 nem ALSA"): ~88% do monitor
        // primário, com o canvas de referência como mínimo desejável.
        //
        // Abrir fixo em 1280×760 num monitor grande dava uma janela BAIXA
        // demais — e era parte do "antes não tinha barra de rolagem com 3
        // fileiras": o painel X11 abria alto o bastante pras três caberem,
        // o app JUCE não.
        {
            rasgo::panel::MonitorRect mon;
            if (const auto* d = juce::Desktop::getInstance().getDisplays()
                                    .getPrimaryDisplay()) {
                // `userBounds` (não o `userArea`, obsoleto): a área útil,
                // já sem barra de tarefas e menu. É `Rectangle<float>`,
                // então arredonda — pixel fracionário de monitor não
                // existe, e truncar perderia uma linha em telas com
                // escala fracionária.
                const auto b = d->userBounds.toNearestInt();
                mon.x = b.getX();      mon.y = b.getY();
                mon.w = b.getWidth();  mon.h = b.getHeight();
            }
            const auto b = rasgo::panel::firstOpen(mon, 1280, 760, 0.88f);
            setSize(b.w, b.h);
        }
        setAudioChannels(0, 2);

        // ---- que patch abrir -------------------------------------------
        // Mesma política do painel X11, e ela é uma posição de projeto: o
        // Rasgo Modular abre TOCANDO um patch novo, não retomando um
        // documento congelado. `RASGO_SEED=N` reproduz um seed específico
        // (render determinístico); `RASGO_RESUME=1` é o único caminho que
        // volta à sessão salva — pra quem estava no meio de um patch feito
        // à mão. Ctrl+S e o BANCO continuam sendo como se guarda de
        // propósito.
        if (const char* sv = std::getenv("RASGO_SEED")) {
            seed_ = std::strtoull(sv, nullptr, 10);
            rack_.curSeed = seed_;
            rack_.applySeed(seed_, sampleRate_, blockSize_);
            rack_.populateMotion();
        } else if (std::getenv("RASGO_RESUME") != nullptr
                   && loadPatch(dataDir().getChildFile("session.rmp"))) {
            seed_ = rack_.curSeed;
        } else {
            seed_ = nextRandomSeed();
            rack_.curSeed = seed_;
            rack_.applySeed(seed_, sampleRate_, blockSize_);
            rack_.populateMotion();
        }
        view_->refresh();
        syncHeader();
        syncSignalIn();

        // O QUADRO. Sem isto o app só repinta quando o mouse se mexe, e
        // tudo que anima sozinho fica congelado: osciloscópio, espectro,
        // VU, lanes do TRIGSEQ, o anel do knob quando o parâmetro está
        // MODULADO (o valor já é lido do motor, faltava repintar) e o
        // flash do botão acionado. 30 Hz é a cadência do painel X11
        // (33 ms, `panel_main.cpp`) — o instrumento é o mesmo, o
        // movimento na tela também tem que ser.
        startTimerHz(30);
    }

    ~MainComponent() override {
        stopTimer();
        for (auto* in : midiIns_) in->stop();
        midiIns_.clear();
        shutdownAudio();
        // continua daqui na próxima sessão (com `RASGO_RESUME=1`)
        savePatch(dataDir().getChildFile("session.rmp"));
    }

    void resized() override {
        overlay_.setBounds(getLocalBounds());
        auto area = getLocalBounds();
        header_.setBounds(area.removeFromTop(
            header_.preferredHeight(getWidth())));
        palette_.setBounds(area.removeFromLeft(PaletteColumn::kWidth));
        credits_.setBounds(area.removeFromTop(CreditsStrip::kHeight));
        viewport_.setBounds(area);
        layoutRack();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        // `AudioBlock` do motor tem teto de 256 amostras — o bloco do host
        // pode ser maior, então o grafo roda em sub-blocos e o resto fica
        // no `carry_` até a próxima chamada.
        blockSize_ = static_cast<std::size_t>(
            std::min(256, std::max(1, samplesPerBlockExpected)));
        sampleRate_ = sampleRate;
        std::lock_guard<std::mutex> lk(rack_.gmx);
        rack_.sampleRate = static_cast<float>(sampleRate);
        rack_.blockFrames = blockSize_;
        rack_.loudness.prepare(static_cast<float>(sampleRate));
        rack_.graph.prepare(static_cast<float>(sampleRate), 2, blockSize_);
        st_ = std::make_unique<AudioBlock>(static_cast<float>(sampleRate), 2,
                                           blockSize_);
        carryL_.assign(blockSize_, 0.0f);
        carryR_.assign(blockSize_, 0.0f);
        carryUsed_ = blockSize_;   // vazio
        lastL_.assign(blockSize_, 0.0f);
        lastR_.assign(blockSize_, 0.0f);
        lastOutL_ = 0.0f;
        lastOutR_ = 0.0f;
        starve_ = 1.0f;
        // reserva da fila de entrada: quatro blocos do host, ou oito
        // sub-blocos, o que for maior — pra `push_back` nunca alocar
        // dentro do callback de áudio
        inFifo_.clear();
        inFifo_.reserve(std::max<std::size_t>(
            static_cast<std::size_t>(std::max(1, samplesPerBlockExpected)) * 8,
            blockSize_ * 16));
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override {
        // A entrada TEM que ser copiada antes de qualquer coisa: o JUCE
        // entrega o mesmo buffer pra ler e escrever, e a primeira coisa
        // que fazemos é escrever a saída por cima.
        if (info.buffer == nullptr || info.buffer->getNumChannels() == 0)
            return;   // sem canal de saída não há o que escrever

        // A entrada TEM que ser copiada antes de qualquer escrita, e vai
        // pra uma FILA — não pra um buffer do tamanho do bloco do host.
        // O grafo consome em sub-blocos de 256 e o que sobra de um callback
        // fica no `carry_`; com bloco de host que não é múltiplo de 256, a
        // conta não fecha e a versão anterior DESCARTAVA a sobra — entrada
        // picotada. A fila também nunca realoca em tempo real: a reserva é
        // feita no `prepareToPlay`.
        if (signalIn_.load(std::memory_order_relaxed)) {
            const float* iL = info.buffer->getReadPointer(0, info.startSample);
            const float* iR = info.buffer->getNumChannels() > 1
                ? info.buffer->getReadPointer(1, info.startSample) : iL;
            const std::size_t want = static_cast<std::size_t>(info.numSamples) * 2;
            if (inFifo_.size() + want > inFifo_.capacity())
                inFifo_.clear();          // atrasou demais: recomeça limpo
            for (int i = 0; i < info.numSamples; ++i) {
                inFifo_.push_back(iL[i]);
                inFifo_.push_back(iR[i]);
            }
        } else if (!inFifo_.empty()) {
            inFifo_.clear();
        }

        auto* L = info.buffer->getWritePointer(0, info.startSample);
        auto* R = info.buffer->getNumChannels() > 1
            ? info.buffer->getWritePointer(1, info.startSample) : nullptr;

        int done = 0;
        while (done < info.numSamples) {
            if (carryUsed_ >= blockSize_) renderBlock();
            const int n = std::min<int>(info.numSamples - done,
                static_cast<int>(blockSize_ - carryUsed_));
            for (int i = 0; i < n; ++i) {
                L[done + i] = carryL_[carryUsed_ + static_cast<std::size_t>(i)];
                if (R) R[done + i] = carryR_[carryUsed_ + static_cast<std::size_t>(i)];
            }
            carryUsed_ += static_cast<std::size_t>(n);
            done += n;
        }
    }

    void releaseResources() override {}

    // ---- teclado -------------------------------------------------------
    // Os atalhos do painel X11. Faltavam TODOS: o app JUCE não tinha
    // `keyPressed`, então nenhuma tecla fazia nada — incluindo [espaço],
    // que é o gesto que rompe/reata todos os cabos (e portanto o que muda
    // a cor deles; o STANDBY nunca fez isso).
    //
    // Este método NÃO decide mais quais teclas existem: quem decide é a
    // tabela de `src/ui/Shortcuts.hpp`, que é testada. Aqui ficam só as
    // teclas especiais (que têm constante própria no JUCE) e a tradução
    // de `Shortcut` para a ação. Ter a decisão em um lugar só foi a
    // correção de raiz das três quebras seguidas do teclado — cada
    // remendo anterior consertava um ramo e deixava os outros com a
    // regra antiga.
    bool keyPressed(const juce::KeyPress& k) override {
        const int c = k.getKeyCode();
        const bool ctrl = k.getModifiers().isCommandDown();

        if (overlay_.mode() != OverlayView::Mode::none) {
            if (c == juce::KeyPress::escapeKey) { closeOverlay(); return true; }
            if (c == juce::KeyPress::downKey)     { overlay_.scrollBy(+40); return true; }
            if (c == juce::KeyPress::upKey)       { overlay_.scrollBy(-40); return true; }
            if (c == juce::KeyPress::pageDownKey) { overlay_.scrollBy(+320); return true; }
            if (c == juce::KeyPress::pageUpKey)   { overlay_.scrollBy(-320); return true; }
            if (c == juce::KeyPress::homeKey)     { overlay_.scrollTo(0); return true; }
            if (c == juce::KeyPress::endKey)      { overlay_.scrollTo(1 << 20); return true; }
            closeOverlay();
            return true;
        }

        // Teclas especiais primeiro — elas têm constante própria no JUCE
        // e não passam pela tabela. if/else e não switch: essas constantes
        // são `static const int` de runtime, não expressões constantes.
        if (c == juce::KeyPress::escapeKey) {
            view_->cancelInteraction();   // desistir é um gesto legítimo
            return true;
        }
        if (c == juce::KeyPress::spaceKey) {
            rack_.toggleRupture();      // rompe/reata TODOS os cabos
            view_->repaint();
            return true;
        }
        if (c == juce::KeyPress::downKey) {
            viewport_.setViewPosition(0, viewport_.getViewPositionY() + 40);
            return true;
        }
        if (c == juce::KeyPress::upKey) {
            viewport_.setViewPosition(0,
                std::max(0, viewport_.getViewPositionY() - 40));
            return true;
        }

        // E o resto pela tabela testada. Todo atalho que TEM botão passa
        // por `header_.trigger`, que acende o botão correspondente por
        // ~160 ms além de executar a ação — sem isso o atalho agia sem
        // sinal nenhum na tela, e era impossível saber o que funcionava.
        using S = rasgo::ui::Shortcut;
        using A = HeaderBar::Act;
        switch (rasgo::ui::lookupShortcut(
                    c, static_cast<int>(k.getTextCharacter()), ctrl)) {
        case S::seed:    header_.trigger(A::seed);    return true;
        case S::vary:    header_.trigger(A::vary);    return true;
        case S::mutate:  header_.trigger(A::mutate);  return true;
        case S::evolve:  header_.trigger(A::evolve);  return true;
        case S::cross:   header_.trigger(A::cross);   return true;
        case S::uncable: header_.trigger(A::uncable); return true;
        case S::restore: header_.trigger(A::restore); return true;
        case S::save:    header_.trigger(A::save);    return true;
        case S::bank:    header_.trigger(A::bank);    return true;
        case S::rec:     header_.trigger(A::rec);     return true;
        case S::undo:    header_.trigger(A::undo);    return true;
        case S::open:    header_.trigger(A::open);    return true;
        case S::zoomIn:  header_.trigger(A::zoomIn);  return true;
        case S::zoomOut: header_.trigger(A::zoomOut); return true;
        case S::zoomReset: view_->nudgeZoom(0); layoutRack(); return true;
        case S::quit:
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            return true;
        case S::none: return false;
        }
        return false;
    }

private:
    // DESFAZER. Cobre toda ação que muda o patch — inclusive CADA CABO
    // ligado ou cortado, que é o caso mais comum: o músico cabeia, o
    // resultado não é o esperado, e ele já não sabe ao certo qual ligação
    // fez. Sem isso a única saída era caçar o cabo na tela.
    void undoLast() {
        std::string prev;
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            if (rack_.undoStack.empty()) return;
            prev = rack_.undoStack.back();
            rack_.undoStack.pop_back();
        }
        // `applyPatchText` empilharia de novo se passasse pelo caminho
        // normal — aqui a pilha já foi consumida de propósito
        applyPatchText(prev, "desfeito");
        seed_ = rack_.curSeed;
        syncHeader();
    }

    void clearCablesAndRefresh() {
        rack_.pushUndo();
        rack_.clearCables();
        rack_.populateMotion();
        syncSignalIn();
        // O seed NÃO é zerado aqui. Ontem tirei o zeramento de
        // `Rack::curSeed` e deixei ESTE passar — o seed vive em dois
        // lugares (o do rack e o do cabeçalho), e corrigir um só deixou o
        // REPOR sumindo do mesmo jeito. Foi por isso que o autor
        // continuou sem ver o `r` funcionar depois da "correção".
        view_->refresh();
        syncHeader();
    }

    void toggleOverlay(OverlayView::Mode m) {
        const bool same = overlay_.mode() == m;
        overlay_.show(same ? OverlayView::Mode::none : m, lang_);
        header_.setOverlay(same ? 0
            : (m == OverlayView::Mode::tutorial ? 1 : 2));
        grabKeyboardFocus();
    }
    void closeOverlay() {
        overlay_.close();
        header_.setOverlay(0);
        grabKeyboardFocus();
    }

    // Snapshot dos osciloscópios SEM bloquear: se o thread de áudio está
    // com o `gmx`, a UI redesenha o snapshot anterior e segue — nunca
    // congela à espera do áudio (`redraw()` do painel X11 faz igual).
    void timerCallback() override {
        // O TECLADO só chega aqui se este componente tiver o foco. O único
        // filho focável é a caixa de seed (um TextEditor), e o JUCE dá o
        // foco inicial ao primeiro filho que o queira — então, ao abrir, o
        // campo ficava com ele e engolia tudo: `n` não é dígito e era
        // ignorado, `Ctrl+Z` virava desfazer DO CAMPO. Os atalhos pareciam
        // não existir até o músico clicar no rack por acaso.
        // Reaver o foco a cada quadro é barato e se auto-corrige — menos
        // isso quando a pessoa está de fato digitando um seed.
        // GUARDAS, e a primeira delas é a que importa mais:
        //
        // `isForegroundProcess()` — sem isto o app reavia o teclado mesmo
        // com a janela em SEGUNDO PLANO, 30 vezes por segundo, e roubava o
        // foco de qualquer outra janela: o autor não conseguia digitar no
        // terminal enquanto o app estivesse aberto. Uma correção que fazia
        // os atalhos funcionarem tornou o resto do computador inutilizável
        // — reaver foco só faz sentido quando a janela já é a ativa.
        //
        // `hasKeyboardFocus(true)` (com filhos) em vez de `false`: se
        // QUALQUER filho já tem o foco, não há nada a reaver.
        //
        // As outras duas: não disputar com um diálogo modal nem com o
        // seletor de arquivo nativo do ABRIR, que é assíncrono.
        if (juce::Process::isForegroundProcess()
            && isShowing() && !chooserOpen_
            && juce::Component::getCurrentlyModalComponent() == nullptr
            && !hasKeyboardFocus(true) && !header_.seedBoxFocused())
            grabKeyboardFocus();

        if (rack_.gmx.try_lock()) {
            // atribui entrada a entrada em vez de `scopeSnap = scopes`:
            // reaproveita os vetores já alocados. A troca do mapa inteiro
            // realocava ~175 KB por quadro, 30×/s, à toa.
            for (const auto& [id, sc] : rack_.scopes) rack_.scopeSnap[id] = sc;
            rack_.snapshotCables();
            rack_.gmx.unlock();
        }

        // VARIA: um passo de variação por quadro (dt = 1/30 s, a mesma
        // cadência do painel X11). Sob o `gmx` porque `tick` escreve as
        // bases de parâmetro que o thread de áudio lê no mesmo instante —
        // sem o lock era uma corrida (leitura rasgada de float →
        // coeficiente absurdo por um bloco = estalo). Não roda enquanto o
        // músico está com a mão num controle: a máquina não disputa o
        // knob com quem está mexendo nele.
        float vu = 0.0f;
        if (rack_.motionOn && !view_->interacting()) {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            if (++motionThrottle_ >= 30) {
                motionThrottle_ = 0;
                rack_.motion.refreshCables(rack_.graph);
            }
            // se o limitador do MASTER já está segurando, o patch está
            // QUENTE mesmo que o RMS não pareça — dobra a redução de ganho
            // na energia pra o duck puxar de volta (~5 dB = duck cheio)
            float grDb = 0.0f;
            for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                if (auto* ms = dynamic_cast<rasgo::modular::Master*>(
                        &rack_.graph.node(i)))
                    grDb = std::max(grDb, ms->gainReductionDb());
            const float energy = std::min(1.0f, std::max(
                rack_.outRms.load(std::memory_order_relaxed) * 2.2f,
                grDb * 0.18f));
            rack_.motion.tick(rack_.graph, 1.0f / 30.0f, energy);
        }
        for (const auto& [id, sc] : rack_.scopeSnap)
            if (rack_.graph.node(id).type() == "MASTER")
                vu = std::max(vu, sc.peak());

        header_.setReadout(view_->visibleModuleCount(), rack_.shown.size(),
                           rack_.graph.cableCount(), vu, rack_.motionOn,
                           rack_.recording.load(std::memory_order_relaxed));
        tickLearn();
        pollRecAutoStop();
        view_->setPaletteHover(palette_.hoveredType());

        view_->repaint();
        header_.repaint();
    }

    // Caixa LEARN: o que está sob o mouse — um widget do rack, o corpo de
    // um módulo, ou uma linha da paleta. O conteúdo só troca depois de 1 s
    // parado sobre o MESMO objeto (senão pisca a cada movimento), e fora
    // de qualquer objeto mantém o último. Regra do painel X11.
    void tickLearn() {
        RackView::LearnHit hit;
        const auto mp = juce::Desktop::getInstance().getMainMouseSource()
                            .getScreenPosition().roundToInt();
        const auto inView = view_->getLocalPoint(nullptr, mp);
        if (viewport_.getScreenBounds().contains(mp))
            hit = view_->learnAt(inView);
        if (hit.entry == nullptr) {
            const std::string& t = palette_.hoveredType();
            if (!t.empty())
                if (const auto* e = rasgo::panel::lookupLearnModule(t, lang_))
                    hit = {e, u8(t), "pal|" + t};
        }

        // Dwell de 1 s: o texto troca depois de UM SEGUNDO parado sobre o
        // MESMO objeto (jack, controle ou corpo/nome do módulo). Sem isso
        // a caixa pisca a cada movimento do mouse.
        //
        // A contagem NÃO reinicia quando o ponteiro fica sobre nada: as
        // pegadas dos widgets têm folga entre si, e atravessar um vão de
        // um pixel zerava o relógio — na prática o segundo quase nunca
        // fechava. Só um objeto DIFERENTE reinicia.
        const auto now = std::chrono::steady_clock::now();
        if (hit.key.empty()) return;              // nada sob o mouse: espera
        if (hit.key != learnKey_) {
            learnKey_ = hit.key;
            learnSince_ = now;
            return;
        }
        if (now - learnSince_ >= kLearnDwell && !palette_.noticeActive())
            palette_.setLearn(hit.entry, hit.title);
    }

    static constexpr auto kLearnDwell = std::chrono::milliseconds(1000);

    // Entropia de várias fontes independentes + splitmix64, como no painel
    // X11: o relógio sozinho podia repetir entre acionamentos rápidos.
    std::uint64_t nextRandomSeed() {
        static std::random_device rd;
        std::uint64_t s = (static_cast<std::uint64_t>(rd()) << 32) ^ rd();
        s ^= static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        s ^= lastSeed_ * 0x9E3779B97F4A7C15ULL;
        s += 0x9E3779B97F4A7C15ULL;
        s = (s ^ (s >> 30)) * 0xBF58476D1CE4E5B9ULL;
        s = (s ^ (s >> 27)) * 0x94D049BB133111EBULL;
        lastSeed_ = s ^ (s >> 31);
        return lastSeed_;
    }

    // `userApplicationDataDirectory` em vez do XDG cru do painel X11: no
    // Linux dá o MESMO `~/.local/share` (patches intercambiáveis entre os
    // dois front-ends), e no macOS/Windows dá o lugar nativo em vez de
    // espalhar um diretório de convenção Linux por lá.
    static juce::File dataDir() {
        auto d = juce::File::getSpecialLocation(
                     juce::File::userApplicationDataDirectory)
                 .getChildFile("rasgo-modular");
        d.createDirectory();
        return d;
    }

    // Tira um módulo da CASE sem tocar nos outros cabos: desliga só os que
    // encostam nele; o nó fica órfão no grafo (silencioso, sem custo de
    // DSP, porque `setActiveOutput` poda o que não chega à saída). Mesma
    // semântica do painel X11 — remover da vista não é apagar do patch.
    void removeModule(std::size_t id) {
        rack_.pushUndo();
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            for (std::size_t i = rack_.graph.cableCount(); i-- > 0;) {
                const auto& c = rack_.graph.cable(i);
                if (c.source().node == id || c.target().node == id)
                    rack_.graph.disconnect(c.target().node, c.target().port);
            }
            rack_.reprepare();
            rack_.shown.erase(
                std::remove(rack_.shown.begin(), rack_.shown.end(), id),
                rack_.shown.end());
            rack_.scopes.erase(id);
            rack_.scopeSnap.erase(id);
            rack_.panelCache.erase(id);
        }
        rack_.populateMotion();
        syncSignalIn();
        view_->refresh();
    }

    void addModule(const std::string& type) {
        rack_.pushUndo();
        std::size_t newId = static_cast<std::size_t>(-1);
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            auto n = rasgo::panel::makeModule(type);
            if (!n) return;
            const std::size_t id = rack_.graph.add(std::move(n));
            rack_.shown.push_back(id);
            rack_.scopes[id];
            rack_.byType[type] = id;
            rack_.reprepare();
            newId = id;
        }
        if (newId != static_cast<std::size_t>(-1)) view_->markPending(newId);
        rack_.populateMotion();
        syncSignalIn();
        view_->refresh();
    }

    // Carregar um `.rmp`. O formato é o do motor (`serialize`), então um
    // patch salvo no painel X11 abre aqui e vice-versa — no Linux os dois
    // front-ends inclusive gravam no mesmo diretório.
    bool loadPatch(const juce::File& path) {
        if (!path.existsAsFile()) return false;
        return applyPatchText(path.loadFileAsString().toStdString(),
                              path.getFullPathName());
    }

    // Instala um patch a partir do texto serializado. Serve pro `.rmp` e
    // pro DESFAZER — que é a mesma operação: uma fotografia do grafo
    // voltando a ser o grafo.
    bool applyPatchText(const std::string& text, const juce::String& origem) {

        // ordem de exibição e SEED, se o arquivo trouxer
        std::vector<std::size_t> ord;
        std::uint64_t seedInFile = 0;
        {
            std::istringstream is(text);
            std::string line;
            while (std::getline(is, line)) {
                if (line.rfind("panel shown", 0) == 0) {
                    std::istringstream ls(line);
                    std::string a, b;
                    std::size_t id = 0;
                    ls >> a >> b;
                    while (ls >> id) ord.push_back(id);
                } else if (line.rfind("seed ", 0) == 0) {
                    // `savePatch`/`snapshot` SEMPRE escreveram esta linha e
                    // o carregador nunca a lia. Efeito: desfazer uma troca
                    // de seed restaurava o patch certo e deixava o NÚMERO
                    // do anterior na tela — a caixa passava a mentir sobre
                    // qual patch está soando, que é justamente a única
                    // coisa que ela existe pra dizer.
                    seedInFile = std::strtoull(line.c_str() + 5, nullptr, 10);
                }
            }
        }
        try {
            SignalGraph g2 = SignalGraph::deserialize(text, patchFactory);
            {
                std::lock_guard<std::mutex> lk(rack_.gmx);
                rack_.graph = std::move(g2);
                rack_.invalidatePanels();   // ids passam a significar outra coisa
                rack_.sink = 0;
                for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                    if (rack_.graph.node(i).type() == "OUT") rack_.sink = i;
                rack_.shown.clear();
                // Validar os ids do arquivo NÃO é paranoia: `shown` é
                // percorrido pelo thread de áudio (`feedScopes`,
                // `collectNotes`) e `SignalGraph::node()` é `nodes_.at()`,
                // que LANÇA. Um `.rmp` de um patch maior — ou corrompido —
                // derrubaria o áudio com uma exceção escapando do
                // callback. Ids fora de faixa são descartados.
                if (!ord.empty()) {
                    for (const auto id : ord)
                        if (id < rack_.graph.nodeCount()
                            && rack_.graph.node(id).type() != "OUT")
                            rack_.shown.push_back(id);
                }
                if (rack_.shown.empty())
                    for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                        if (rack_.graph.node(i).type() != "OUT")
                            rack_.shown.push_back(i);
                rack_.byType.clear();
                for (const auto id : rack_.shown)
                    rack_.byType[rack_.graph.node(id).type()] = id;
                rack_.scopes.clear();
                rack_.scopeSnap.clear();
                rack_.graph.prepare(static_cast<float>(sampleRate_), 2, blockSize_);
                // o move zerou o alvo ativo — reancorar, senão não sai som
                rack_.graph.setActiveOutput(rack_.sink);
                rack_.curSeed = seedInFile;   // 0 = patch editado à mão
            }
            rack_.allocScopes();
            rack_.populateMotion();
            syncSignalIn();
            view_->refresh();
            std::fprintf(stderr, "[patch] %s\n", origem.toRawUTF8());
            return true;
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[patch] falhou ao carregar: %s\n", e.what());
            return false;
        }
    }

    // `OUT` é o sink deste app, não vem do catálogo de módulos
    static std::unique_ptr<Signal> patchFactory(const std::string& t) {
        if (t == "OUT") return std::make_unique<Out>();
        return rasgo::panel::makeModule(t);
    }

    // ---- preferências -------------------------------------------------
    // Idioma e vista do rack em arquivos próprios de UMA linha, separados
    // do patch: sobrevivem a abrir num seed novo, não só ao `--resume`.
    // Mesma divisão do painel X11 — preferência de quem usa não é estado
    // do documento.
    void loadPrefs() {
        const auto lf = dataDir().getChildFile("ui-lang");
        if (lf.existsAsFile())
            lang_ = rasgo::panel::langFromCode(
                lf.loadFileAsString().trim().toStdString());
        const auto rf = dataDir().getChildFile("rack-view");
        if (rf.existsAsFile())
            rackOutputPref_ = rf.loadFileAsString().trim() == "output";
    }
    void saveLangPref() const {
        dataDir().getChildFile("ui-lang")
            .replaceWithText(u8(rasgo::panel::langCode(lang_)));
    }
    void saveRackViewPref() const {
        dataDir().getChildFile("rack-view")
            .replaceWithText(view_->outputOnly() ? "output" : "all");
    }

    // ---- SIGNAL-IN: áudio e MIDI de fora ------------------------------
    // Abrir entrada de áudio e MIDI só quando o patch TEM um SIGNAL-IN é
    // decisão de projeto, não economia: o Rasgo Modular soa sozinho, e
    // MIDI/áudio são adaptadores opcionais. Pedir microfone a quem nunca
    // vai usar é ruído — e no macOS é um diálogo de permissão do sistema
    // aparecendo sem motivo. Mesma regra do `syncSignalIn` do painel X11.
    void syncSignalIn() {
        bool present = false;
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                if (rack_.graph.node(i).type() == "SIGNAL-IN") { present = true; break; }
        }
        if (present == signalIn_.load()) return;
        signalIn_.store(present);

        // canais de entrada: trocar reabre o dispositivo, por isso só é
        // feito quando o estado de fato muda
        setAudioChannels(present ? 2 : 0, 2);

        if (present) {
            for (const auto& d : juce::MidiInput::getAvailableDevices()) {
                if (auto in = juce::MidiInput::openDevice(d.identifier, this)) {
                    in->start();
                    midiIns_.add(std::move(in));
                }
            }
            if (midiIns_.isEmpty())
                std::fprintf(stderr, "[signal-in] nenhuma entrada MIDI encontrada\n");
        } else {
            for (auto* in : midiIns_) in->stop();
            midiIns_.clear();
        }
    }

    // Chamado pelo thread de MIDI do JUCE, não pelo de áudio nem pelo de
    // interface. `pushMidi` é SPSC e não bloqueia, mas o grafo pode estar
    // sendo editado — `try_lock` e descarta: perder um CC é melhor que
    // travar a entrada MIDI (e o painel X11 faz igual).
    void handleIncomingMidiMessage(juce::MidiInput*,
                                   const juce::MidiMessage& msg) override {
        if (!signalIn_.load(std::memory_order_relaxed)) return;
        const auto* raw = msg.getRawData();
        if (msg.getRawDataSize() < 2) return;
        const std::uint8_t st = raw[0];
        const std::uint8_t d1 = raw[1];
        const std::uint8_t d2 = msg.getRawDataSize() > 2 ? raw[2] : 0;
        std::unique_lock<std::mutex> lk(rack_.gmx, std::try_to_lock);
        if (!lk) return;
        for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
            if (auto* si = dynamic_cast<rasgo::modular::SignalIn*>(
                    &rack_.graph.node(i)))
                si->pushMidi(st, d1, d2);
    }

    // ABRIR um `.rmp`. Até 2026-09-15 o BANCO era um botão que só
    // escrevia: os patches iam pra `patches/` e NÃO havia como reabri-los
    // por dentro do app — o único caminho de carga era a variável
    // `RASGO_RESUME=1` com o `session.rmp`. Uma gaveta sem puxador.
    //
    // O seletor começa na pasta do banco, mas aceita qualquer `.rmp`:
    // trocar patch com outra pessoa é o mesmo gesto que reabrir o próprio.
    void openPatchDialog() {
        auto dir = dataDir().getChildFile("patches");
        dir.createDirectory();
        chooserOpen_ = true;
        chooser_ = std::make_unique<juce::FileChooser>(
            u8(rasgo::panel::tr(rasgo::panel::strings::openPatch, lang_)),
            dir, "*.rmp");
        chooser_->launchAsync(
            juce::FileBrowserComponent::openMode
                | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                chooserOpen_ = false;
                const juce::File f = fc.getResult();
                if (f == juce::File{}) return;      // cancelou
                rack_.pushUndo();                   // abrir é desfazível
                if (!loadPatch(f)) {
                    std::fprintf(stderr, "[patch] não abriu: %s\n",
                                 f.getFullPathName().toRawUTF8());
                    return;
                }
                seed_ = rack_.curSeed;
                syncHeader();
                grabKeyboardFocus();
            });
    }

    // REC. As GRAVAÇÕES não vão pro diretório de dados: vão pra a pasta de
    // música do usuário, como no painel X11 (`RASGO_REC_DIR` sobrepõe) —
    // são obra, não estado interno do app.
    // `RASGO_REC_TAP` = post (padrão) · pre · both
    static Rack::RecTap tapFromEnv() {
        const char* v = std::getenv("RASGO_REC_TAP");
        if (v == nullptr) return Rack::RecTap::post;
        const std::string t = v;
        if (t == "pre")  return Rack::RecTap::pre;
        if (t == "both") return Rack::RecTap::both;
        return Rack::RecTap::post;
    }

    static juce::File recDir() {
        if (const char* over = std::getenv("RASGO_REC_DIR"); over && *over) {
            juce::File d(juce::String::fromUTF8(over));
            d.createDirectory();
            return d;
        }
        auto d = juce::File::getSpecialLocation(juce::File::userMusicDirectory)
                     .getChildFile("RasgoModular");
        d.createDirectory();
        return d;
    }

    void toggleRec() {
        if (rack_.recording.load()) { finishRec(); return; }
        startRec();
    }

    // A gravação pode parar SOZINHA quando a reserva enche (o thread de
    // áudio baixa o atômico pra nunca realocar em tempo real). Sem
    // perceber isso, o próximo clique em REC caía no ramo de INÍCIO e
    // limpava o buffer: a tomada inteira ia pro lixo em silêncio. O timer
    // vigia a transição e finaliza o arquivo.
    void pollRecAutoStop() {
        const bool on = rack_.recording.load(std::memory_order_relaxed);
        if (recWasOn_ && !on) {
            std::fprintf(stderr, "[rec] reserva cheia — tomada encerrada\n");
            finishRec();
        }
        recWasOn_ = rack_.recording.load(std::memory_order_relaxed);
    }

    void startRec() {
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            rack_.recBuf.clear();
            rack_.recPreBuf.clear();
            rack_.recTap = tapFromEnv();
            rack_.reserveRec();
            rack_.findMasterTap();
            // t = 0 da tomada: a fiação inteira, pra o score dizer de onde
            // o som partiu e não só o que mudou depois
            rack_.score.clear();
            for (std::size_t i = 0; i < rack_.graph.cableCount(); ++i) {
                const auto& c = rack_.graph.cable(i);
                rack_.score.connection(0.0, c.source().node, c.source().port,
                                       c.target().node, c.target().port);
            }
            // DICIONÁRIO da partitura: nomes de módulo, de porta e a
            // regulagem. Sem isto o `.score.txt` sai como `33:1 -> 38:2`
            // — cronologicamente correto e ilegível, que foi o relato do
            // autor na sessão de escuta de 23 set. 2026.
            rack_.score.setSeed(rack_.curSeed);
            rack_.score.setSampleRate(sampleRate_);
            for (const auto id : rack_.shown) {
                auto& nd = rack_.graph.node(id);
                rasgo::panel::ScoreNodeInfo info;
                info.type = nd.type();
                for (std::size_t p = 0; p < nd.inputCount(); ++p)
                    info.inPorts.push_back(nd.inputDescriptor(p).name);
                for (std::size_t p = 0; p < nd.outputCount(); ++p)
                    info.outPorts.push_back(nd.outputDescriptor(p).name);
                for (const auto& pr : nd.parameters())
                    info.params.emplace_back(pr.descriptor.id, pr.value);
                rack_.score.describe(id, std::move(info));
            }
            {   // o sink também aparece nas ligações, então precisa de nome
                auto& so = rack_.graph.node(rack_.sink);
                rasgo::panel::ScoreNodeInfo info;
                info.type = so.type();
                for (std::size_t p = 0; p < so.inputCount(); ++p)
                    info.inPorts.push_back(so.inputDescriptor(p).name);
                rack_.score.describe(rack_.sink, std::move(info));
            }

            rack_.loudness.reset();   // o integrado é DA TOMADA
            rack_.recording.store(true);
        }
        recWasOn_ = true;
    }

    void finishRec() {
        rack_.recording.store(false);
        recWasOn_ = false;
        std::vector<float> take, takePre;
        std::string scoreText;
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            take.swap(rack_.recBuf);
            takePre.swap(rack_.recPreBuf);
            scoreText = rack_.score.toText();
        }
        if (take.empty() && takePre.empty()) return;

        ++recCount_;
        // nome por data/hora; se já existir (2ª tomada no mesmo segundo),
        // acrescenta -2, -3…
        const juce::String stamp =
            juce::Time::getCurrentTime().formatted("rec-%Y%m%d-%H%M%S");
        juce::File out = recDir().getChildFile(stamp + ".wav");
        for (int n = 2; out.existsAsFile(); ++n)
            out = recDir().getChildFile(stamp + "-" + juce::String(n) + ".wav");

        // PCM 24 bits desde 21 set. 2026, quando o alvo de publicação foi
        // declarado (streaming). A tomada do músico não é o arquivo final
        // — vai ser comparada com o tap `pre-safety` e possivelmente
        // masterizada depois, e 16 bits jogariam fora resolução que não
        // volta. Sem dither: em 24 bits o degrau de quantização está bem
        // abaixo do ruído do material, então TPDF só somaria ruído sem
        // corrigir defeito nenhum (a justificativa longa está no
        // `WavWriter.hpp`).
        const auto write = [&](const juce::File& f, const std::vector<float>& v) {
            if (v.empty()) return;
            rasgo::modular::writeWav24(f.getFullPathName().toStdString(), v,
                                       static_cast<std::uint32_t>(sampleRate_), 2);
        };
        // O nome DIZ o tap. Um `.wav` sem essa marca seria uma armadilha:
        // dois arquivos da mesma tomada soando diferente sem explicação.
        if (!take.empty())
            write(rack_.recTap == Rack::RecTap::post
                      ? out
                      : out.getSiblingFile(out.getFileNameWithoutExtension()
                                           + ".post-safety.wav"), take);
        if (!takePre.empty())
            write(out.getSiblingFile(out.getFileNameWithoutExtension()
                                     + ".pre-safety.wav"), takePre);
        // o .score.txt vai ao lado, com o MESMO nome — é o par do áudio
        const juce::File scoreFile =
            out.getSiblingFile(out.getFileNameWithoutExtension() + ".score.txt");
        scoreFile.replaceWithText(u8(scoreText));
        std::fprintf(stderr, "[rec] %.1f s -> %s (+ %s)\n",
                     static_cast<double>(take.size()) / 2.0 / sampleRate_,
                     out.getFullPathName().toRawUTF8(),
                     scoreFile.getFileName().toRawUTF8());
    }

    void savePatch(const juce::File& path) {
        std::string text;
        {
            std::lock_guard<std::mutex> lk(rack_.gmx);
            text = rack_.graph.serialize();
            if (rack_.curSeed)
                text += "seed " + std::to_string(rack_.curSeed) + "\n";
            text += "panel shown";
            for (const auto id : rack_.shown) text += ' ' + std::to_string(id);
            text += '\n';
        }
        if (path.replaceWithText(u8(text)))
            std::fprintf(stderr, "[patch] salvo em %s\n",
                         path.getFullPathName().toRawUTF8());
    }

    // Um bloco do grafo. Disciplina de lock IDÊNTICA à do painel X11
    // (`panel_main.cpp:512`): o áudio NÃO espera a UI. Se a interface está
    // com o mutex (editando o grafo), reemite o último bloco com um fade —
    // zerar duro seria um degrau na onda, ou seja, um clique audível a
    // cada mexida na interface.
    void renderBlock() {
        std::unique_lock<std::mutex> lk(rack_.gmx, std::try_to_lock);
        if (!lk) {
            rack_.starvedBlocks.fetch_add(1, std::memory_order_relaxed);

            // FOME: a interface está com o grafo e não há bloco novo.
            //
            // Antes isto REPRODUZIA o bloco anterior com ganho menor, e
            // era essa a causa do estalo que o autor relatou em 23 set.
            // 2026: a primeira amostra do bloco repetido não tem relação
            // com a última que saiu, então há um SALTO na onda. Salto na
            // onda é clique, por definição — o fade de 0,86 por bloco
            // reduzia a amplitude do problema sem tocar na sua natureza.
            //
            // Agora o preenchimento CONTINUA de onde a onda parou e desce
            // até zero dentro do bloco. Não há descontinuidade no início
            // (começa exatamente no último valor emitido) nem no fim
            // (chega a zero), então não há o que estalar. O custo é uma
            // queda curta de volume em vez de um clique — que é o defeito
            // certo a ter quando falta dado: audível se acontecer muito,
            // inofensivo se acontecer pouco, e nunca confundível com som
            // do instrumento.
            //
            // O contador acima existe justamente para dizer se acontece
            // muito: ele aparece no cartão SOBRE.
            const float l0 = lastOutL_, r0 = lastOutR_;
            const float n = static_cast<float>(blockSize_);
            for (std::size_t i = 0; i < blockSize_; ++i) {
                const float k = 1.0f - static_cast<float>(i + 1) / n;
                carryL_[i] = l0 * k;
                carryR_[i] = r0 * k;
            }
            lastOutL_ = 0.0f;
            lastOutR_ = 0.0f;
            starve_ = 0.0f;   // a volta é por amostra, no bloco seguinte
            carryUsed_ = 0;
            return;
        }
        // entrega a fatia de entrada deste sub-bloco aos SIGNAL-IN antes
        // de processar — já estamos sob o `gmx`
        if (inFifo_.size() >= blockSize_ * 2) {
            for (std::size_t i = 0; i < rack_.graph.nodeCount(); ++i)
                if (auto* si = dynamic_cast<rasgo::modular::SignalIn*>(
                        &rack_.graph.node(i)))
                    si->pushSamples(inFifo_.data(), blockSize_);
            inFifo_.erase(inFifo_.begin(),
                          inFifo_.begin()
                              + static_cast<std::ptrdiff_t>(blockSize_ * 2));
        }
        rack_.graph.process(*st_, rack_.sink, 0);
        rack_.feedScopes(blockSize_);   // ainda sob `gmx`, como no X11
        rack_.collectNotes();
        // Volta do fade POR AMOSTRA. Saltar `starve_` de volta pra 1.0 de
        // uma vez era um degrau na onda — ou seja, um clique — toda vez
        // que a UI soltava o lock. Agora o ganho sobe ao longo do bloco.
        const float g0 = starve_;
        const float step = (1.0f - g0) / static_cast<float>(blockSize_);
        double sum = 0.0;
        for (std::size_t i = 0; i < blockSize_; ++i) {
            const float gain = g0 + step * static_cast<float>(i);
            carryL_[i] = lastL_[i] = st_->at(0, i) * gain;
            carryR_[i] = lastR_[i] = st_->at(1, i) * gain;
            // a última amostra emitida é o ponto de partida do
            // preenchimento, se o bloco seguinte faltar
            lastOutL_ = carryL_[i];
            lastOutR_ = carryR_[i];
            sum += static_cast<double>(carryL_[i]) * carryL_[i];
        }
        starve_ = 1.0f;
        // energia do bloco — o duck do VARIA lê isto pra recuar quando o
        // patch já está cheio
        rack_.outRms.store(static_cast<float>(
            std::sqrt(sum / static_cast<double>(blockSize_))),
            std::memory_order_relaxed);

        // loudness do que de fato sai
        for (std::size_t i = 0; i < blockSize_; ++i)
            rack_.loudness.push(carryL_[i], carryR_[i]);
        rack_.lufsM.store(rack_.loudness.momentary(), std::memory_order_relaxed);
        rack_.lufsS.store(rack_.loudness.shortTerm(), std::memory_order_relaxed);
        rack_.lufsI.store(rack_.loudness.integrated(), std::memory_order_relaxed);
        rack_.lufsTP.store(rack_.loudness.truePeakDbtp(), std::memory_order_relaxed);

        // REC: intercala L/R no buffer reservado. Para sozinho se a
        // reserva encher — realocar aqui seria alocação em tempo real.
        if (rack_.recording.load(std::memory_order_relaxed)) {
            const bool wantPost = rack_.recTap != Rack::RecTap::pre;
            const bool wantPre  = rack_.recTap != Rack::RecTap::post;
            const bool roomPost = !wantPost
                || rack_.recBuf.size() + blockSize_ * 2 <= rack_.recBuf.capacity();
            const bool roomPre = !wantPre
                || rack_.recPreBuf.size() + blockSize_ * 2 <= rack_.recPreBuf.capacity();
            if (!roomPost || !roomPre) {
                rack_.recording.store(false, std::memory_order_relaxed);
            } else {
                if (wantPost)
                    for (std::size_t i = 0; i < blockSize_; ++i) {
                        rack_.recBuf.push_back(carryL_[i]);
                        rack_.recBuf.push_back(carryR_[i]);
                    }
                if (wantPre
                    && rack_.masterTapNode != static_cast<std::size_t>(-1)) {
                    if (auto* ms = dynamic_cast<rasgo::modular::Master*>(
                            &rack_.graph.node(rack_.masterTapNode))) {
                        const float* pl = ms->preSafety(0);
                        const float* pr = ms->preSafety(1);
                        const std::size_t n =
                            std::min(blockSize_, ms->preSafetyFrames());
                        for (std::size_t i = 0; i < n; ++i) {
                            rack_.recPreBuf.push_back(pl[i]);
                            rack_.recPreBuf.push_back(pr[i]);
                        }
                    }
                }
            }
        }
        carryUsed_ = 0;
    }

    void layoutRack() {
        view_->layoutFor(viewport_.getMaximumVisibleWidth(),
                         viewport_.getMaximumVisibleHeight());
    }
    // Fonte ÚNICA do seed: `rack_.curSeed`. Manter uma cópia em `seed_`
    // foi o que permitiu a correção pela metade — dois lugares pra
    // esquecer um. `seed_` segue existindo só como o que o usuário
    // digitou/sorteou nesta sessão, mas quem manda na tela é o rack.
    void syncHeader() {
        seed_ = rack_.curSeed;
        header_.setState(seed_, lang_, view_->outputOnly(), standby_);
    }

    Rack rack_;
    std::unique_ptr<RackView> view_;
    juce::Viewport viewport_;
    PaletteColumn palette_;
    HeaderBar header_;
    CreditsStrip credits_;
    OverlayView overlay_;
    std::uint64_t seed_ = 0;
    rasgo::panel::Lang lang_ = rasgo::panel::Lang::pt;
    bool standby_ = false;
    std::unique_ptr<AudioBlock> st_;
    std::vector<float> carryL_, carryR_, lastL_, lastR_;
    // Última amostra que de fato saiu pelos alto-falantes. É a âncora do
    // preenchimento quando falta bloco: começar dali é o que impede a
    // descontinuidade — ou seja, o estalo.
    float lastOutL_ = 0.0f, lastOutR_ = 0.0f;
    std::size_t blockSize_ = 256, carryUsed_ = 256;
    double sampleRate_ = 48000.0;
    float starve_ = 1.0f;
    int motionThrottle_ = 0;
    std::uint64_t lastSeed_ = 0;
    bool rackOutputPref_ = false;
    std::atomic<bool> signalIn_{false};   // há SIGNAL-IN no patch?
    std::vector<float> inFifo_;           // entrada intercalada L/R
    juce::OwnedArray<juce::MidiInput> midiIns_;
    int recCount_ = 0;
    bool recWasOn_ = false;
    std::unique_ptr<juce::FileChooser> chooser_;
    bool chooserOpen_ = false;
    std::string learnKey_;
    std::chrono::steady_clock::time_point learnSince_{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class RasgoModularApplication : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "Rasgo Modular"; }
    const juce::String getApplicationVersion() override
    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override {
        window_ = std::make_unique<Window>(getApplicationName());
    }
    void shutdown() override { window_ = nullptr; }
    void systemRequestedQuit() override { quit(); }

private:
    class Window : public juce::DocumentWindow {
    public:
        explicit Window(const juce::String& name)
            : DocumentWindow(name, T.bg, DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            // Abre sempre MAXIMIZADO no monitor PRINCIPAL (pedido do autor,
            // 1 out. 2026). Maximizar age sobre o monitor onde a janela
            // está, então primeiro ela é posta dentro do principal. Isso é
            // feito DUAS vezes: antes de mostrar e de novo depois, porque o
            // gerenciador de janelas pode reposicioná-la ao mapear — o
            // Cinnamon abre janela nova no monitor do MOUSE, e com só a
            // primeira colocação ela maximizava no monitor secundário
            // (conferido com `xprop`/`xwininfo`). O mapeamento é
            // assíncrono, então a segunda colocação espera a janela
            // aparecer (150 ms) e a maximização espera o gerenciador
            // aplicar a posição (mais 150 ms).
            // Com barra de título nativa, o `setFullScreen` do JUCE pede ao
            // gerenciador o estado MAXIMIZADO, não tela cheia: barra de
            // título e painel do sistema continuam à vista, e restaurar
            // volta ao tamanho de antes (~88% do monitor, calculado em
            // `MainComponent`).
            placeOnPrimaryDisplay();
            setVisible(true);
            const juce::Component::SafePointer<Window> self(this);
            juce::Timer::callAfterDelay(150, [self] {
                if (self == nullptr) return;
                self->placeOnPrimaryDisplay();
                juce::Timer::callAfterDelay(150, [self] {
                    if (self != nullptr) self->setFullScreen(true);
                });
            });
        }
        void placeOnPrimaryDisplay() {
            if (const auto* d = juce::Desktop::getInstance().getDisplays()
                                    .getPrimaryDisplay())
                setBounds(d->userBounds.toNearestInt()
                              .withSizeKeepingCentre(getWidth(), getHeight()));
            else
                centreWithSize(getWidth(), getHeight());
        }
        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };

    std::unique_ptr<Window> window_;
};

} // namespace

START_JUCE_APPLICATION(RasgoModularApplication)
