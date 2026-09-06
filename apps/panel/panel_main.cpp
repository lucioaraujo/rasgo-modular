// ============================================================================
// rasgo_modular_panel — painel gráfico de teste (X11 + ALSA)
// ============================================================================
//
// O PRIMEIRO front-end: uma "case Eurorack" onde os painéis dos módulos
// aparecem como knobs/sliders/toggles (a partir da `Panel` declarativa de
// cada módulo — nada hard-coded), o instrumento TOCA em tempo real por
// ALSA (autônomo: soa ao abrir), e a saída passa por MIXER + MASTER
// (estéreo, largura, mono, limitador, VU).
//
// NÃO faz parte do `rasgo_modular_core` (que continua sem dependência).
// Bases de design: `RASGO_DOCUMENTATION/design/` (ver `apps/panel/design.md`).
//   - "EURORACK PROPORCIONAL" (design.md §3.2): coordenadas em mm reais,
//     altura de módulo fixa 128,5 mm (3U), largura = HP*5,08 mm, tudo
//     escalado por um fator único `s` (px/mm) derivado do espaço de tela;
//   - fluxo de sinal legível: módulos numa "case" que ENCHE a largura
//     (colada na paleta, sem margem vazia) e quebra em LINHAS;
//   - janela abre no MONITOR PRIMÁRIO, ~88% da área, centrada;
//   - TEXTO SEMPRE UTF-8: dentro do painel via `setlocale` +
//     `Xutf8DrawString` (com fallback); o título da janela via
//     `_NET_WM_NAME`/`UTF8_STRING` (`setTitle()`) — `XStoreName` sozinho
//     grava STRING/Latin-1 e o WM mostra "—"/"●" como lixo;
//   - SEM SOBREPOSIÇÃO ACIDENTAL (checagem no arranque, em mm);
//   - tokens de cor nomeados por função.
//
// Controles:
//   puxar cabo entre jacks (halo duplo = mesmo tipo de sinal); botão
//     direito num jack tira o cabo
//   arrastar o corpo do módulo -> reposiciona; [x] / soltar na paleta -> remove
//   arrastar knob/slider (vertical) -> muda o parâmetro
//   roda do mouse / setas ↑↓ -> rola a case
//   [Ctrl+S] salva o patch   [Ctrl+R] grava/para a gravação (.wav)
//   [espaço] rompe tudo   [r] re-semeia   [q]/Esc sai
//
// O trabalho do músico fica em ~/.local/share/rasgo-modular/ : a sessão é
// auto-carregada no arranque e auto-salva na saída (inclusive em SIGINT/
// SIGTERM); as gravações vão pra a mesma pasta.

#include "core/SignalGraph.hpp"
#include "dsp/Envelope.hpp"
#include "dsp/EuclidClock.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FunctionGenerator.hpp"
#include "dsp/Master.hpp"
#include "dsp/Mixer.hpp"
#include "io/WavWriter.hpp"
#include "panel/AlsaSink.hpp"
#include "panel/AlsaSource.hpp"
#include "panel/AlsaMidi.hpp"
#include "panel/LearnCatalog.hpp"
#include "panel/ModuleCatalog.hpp"
#include "panel/MotionEngine.hpp"
#include "panel/PatchGenetics.hpp"
#include "panel/PatchSeed.hpp"
#include "panel/ScoreRecorder.hpp"
#include "panel/UiLanguage.hpp"
#include "panel/WindowPolicy.hpp"

// carimbo de build (git hash + data) — o CMake passa via -D; fallback pra
// um build fora do CMake.
#ifndef RASGO_MODULAR_BUILD
#define RASGO_MODULAR_BUILD "dev"
#endif

// marca RASGO — mapa de cobertura (tons de cinza, anti-aliased) gerado do
// SVG da família (`assets/regen_logo.sh`); commitado, o build não precisa
// de inkscape/ImageMagick.
#include "panel/assets/rasgo_logo_gray.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <clocale>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace rasgo::modular;

namespace {

// SIGINT/SIGTERM -> pede saída limpa (o loop então salva a sessão)
std::atomic<bool> g_quit{false};
void onSignal(int) { g_quit.store(true); }

struct Out final : Signal {
    Out() : Signal({{"in", PortKind::Audio, ""}}, {{"out", PortKind::Audio, ""}}) {}
    std::string type() const override { return "OUT"; }
    void process(const std::vector<const AudioBlock*>& in,
                 std::vector<AudioBlock>& out) noexcept override {
        if (in[0]) out[0].copyFrom(*in[0]);
        else out[0].clear();
    }
};

// osciloscópio por módulo — anel de amostras da saída, desenhado no
// retângulo `Display` do painel. Enche pelo thread de áudio.
constexpr std::size_t kScopeLen = 220;
constexpr std::size_t kLaneLen = 128;
struct ScopeTrace {
    std::vector<float> buf = std::vector<float>(kScopeLen, 0.0f);
    std::size_t w = 0;
    // TRIGSEQ: histórico das 4 saídas de gate (t1-t4) — pré-alocado sempre
    // pra o thread de áudio nunca alocar; ~2 KB por módulo
    std::array<std::vector<float>, 4> lanes{
        std::vector<float>(kLaneLen, 0.0f), std::vector<float>(kLaneLen, 0.0f),
        std::vector<float>(kLaneLen, 0.0f), std::vector<float>(kLaneLen, 0.0f)};
    std::size_t lw = 0;
    void push(const float v) noexcept {
        buf[w] = v;
        w = (w + 1) % kScopeLen;
    }
    void pushLanes(const float a, const float b, const float c,
                   const float d) noexcept {
        lanes[0][lw] = a; lanes[1][lw] = b; lanes[2][lw] = c; lanes[3][lw] = d;
        lw = (lw + 1) % kLaneLen;
    }
};

// ---- tokens (IDENTIDADE_VISUAL §4: cor nomeada por função) --------------
struct Tokens {
    unsigned long bg, surface, recessed, line, textPrimary, textSecondary,
        accent, warning;
};
// --- "Eurorack proporcional" (design.md §3.2): coordenadas em mm --------
constexpr float kMMHP = 5.08f;      // mm por HP (0,2 pol - horizontal pitch)
constexpr float kMM3U = 128.5f;     // mm de altura do painel (3U)
constexpr float kSMin = 1.6f;       // px/mm mínimo (knob legível)
constexpr float kSMax = 2.6f;       // px/mm máximo (não vira outdoor)
constexpr int kTargetRows = 3;      // linhas de módulos que se quer ver
constexpr int kRackHP = 104;        // largura virtual da case, centrada
constexpr float kModGapMM = 3.0f;   // folga entre módulos vizinhos
constexpr int kCaseTop = 46;        // faixa de status no topo (px)
constexpr int kCasePad = 14;        // px
constexpr int kPaletteW = 158;      // coluna de módulos disponíveis (px)
constexpr int kLearnH = 172;        // caixa LEARN no rodapé da coluna esq.
                                    // (sempre presente — a paleta encurta pra caber)

// zoom de conteúdo do rack, estilo navegador (Ctrl+= amplia, Ctrl+- reduz,
// Ctrl+0 volta a 100%). Multiplica `g_s` DEPOIS do ajuste-por-altura, então
// zoom 1.0 desenha idêntico ao de antes. Reduzir é o caso de uso: ver a 1ª
// e a última linha juntas pra cabear entre elas (pedido do autor 2026-09-05,
// precedente ANTITOTEM ZoomableViewport).
constexpr float kZoomMin  = 0.55f;
constexpr float kZoomMax  = 1.40f;
constexpr float kZoomStep = 0.10f;

float g_s = 2.0f;                   // px/mm - recalculado no relayout
int mmpx(float mm) { return static_cast<int>(std::lround(mm * g_s)); }

const ParameterDescriptor* paramDesc(const Signal& n, const std::string& b) {
    for (const auto& p : n.parameters())
        if (p.descriptor.id == b) return &p.descriptor;
    return nullptr;
}
float paramVal(Signal& n, const std::string& b) {
    try { return n.parameterValue(b); } catch (...) { return 0.0f; }
}

struct Rect { int x, y, w, h; };

// Ordem de EXIBIÇÃO dos tipos de uma família (paleta + rack inicial):
// alfabética, MAS `MIXER` e `MASTER` — o par de saída — sempre grudados
// e no fim da família, nessa ordem (pedido do autor 2026-09-05: "par
// grudado sem unir os módulos"). Só exibição — não mexe na ordem de
// `moduleCatalog()`.
inline void sortFamilyForDisplay(std::vector<const char*>& types) {
    auto key = [](const char* t) -> std::string {
        const std::string s = t;
        if (s == "MIXER") return "\x7e" "1";   // '~' vem depois de A..Z
        if (s == "MASTER") return "\x7e" "2";
        return s;
    };
    std::sort(types.begin(), types.end(),
              [&](const char* a, const char* b) { return key(a) < key(b); });
}

// pegada do widget em MILÍMETROS (canto sup-esq do painel na origem)
struct RectMM { float x, y, w, h; };
RectMM footprintMM(const Widget& w) {
    const float lbl = static_cast<float>(w.label.size());
    switch (w.kind) {
    case Widget::Kind::Knob:   return {w.x - 0.5f, w.y - 1.0f, 10.0f, 15.0f};
    case Widget::Kind::Slider: return {w.x - 1.0f, w.y - 1.0f, 10.0f, 36.0f};
    case Widget::Kind::Toggle: return {w.x - 0.5f, w.y - 0.5f, 5.0f + lbl * 1.7f, 6.0f};
    // o rótulo do jack (fonte cap ~9px, NÃO escala com o zoom) fica
    // centrado em `w.x`, acima do furo — modelamos a largura dele, senão
    // rótulos de 3+ letras a ~9 mm de distância se tocam
    case Widget::Kind::Jack: {
        const float half = std::max(3.0f, lbl * 1.6f);
        return {w.x - half, w.y - 7.0f, 2.0f * half, 13.0f};
    }
    case Widget::Kind::Display: return {w.x - 0.5f, w.y - 0.5f,
                                       (w.span > 1.0f ? w.span : 16.0f) + 1.0f, 17.5f};
    case Widget::Kind::Label:  return {w.x - 0.5f, w.y - 0.5f, 2.0f + lbl * 1.9f, 3.5f};
    }
    return {w.x, w.y, 4.0f, 4.0f};
}
Rect footprintPx(const Widget& w) {
    const RectMM r = footprintMM(w);
    return {mmpx(r.x), mmpx(r.y), mmpx(r.w), mmpx(r.h)};
}
bool overlapMM(const RectMM& a, const RectMM& b) {
    return a.x < b.x + b.w && b.x < a.x + a.w
        && a.y < b.y + b.h && b.y < a.y + a.h;
}

// MATRIX: a grade 4×4 de ganhos (`g<jk>`) é desenhada como uma matriz de
// pontos clicável, não como 16 knobs minúsculos. Célula (linha j = entrada,
// coluna k = saída), em mm, centrada nas posições antigas dos knobs.
RectMM matrixCellMM(const int j, const int k) {
    const float cx = 27.0f + static_cast<float>(k) * 17.0f;
    const float cy = 35.0f + static_cast<float>(j) * 18.0f;
    return {cx - 8.0f, cy - 8.5f, 16.0f, 17.0f};
}

rasgo::panel::MonitorRect primaryMonitor(Display* dpy) {
    rasgo::panel::MonitorRect m;
    m.w = DisplayWidth(dpy, DefaultScreen(dpy));
    m.h = DisplayHeight(dpy, DefaultScreen(dpy));
    int n = 0;
    XRRMonitorInfo* mons =
        XRRGetMonitors(dpy, DefaultRootWindow(dpy), True, &n);
    if (mons && n > 0) {
        int pick = 0;
        for (int i = 0; i < n; ++i)
            if (mons[i].primary) { pick = i; break; }
        m.x = mons[pick].x; m.y = mons[pick].y;
        m.w = mons[pick].width; m.h = mons[pick].height;
    }
    if (mons) XRRFreeMonitors(mons);
    return m;
}

}  // namespace

int main() {
    std::setlocale(LC_ALL, "");
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    // ---- grafo (autônomo: toca ao abrir) --------------------------------
    // O instrumento nasce com UM DE CADA MÓDULO no rack (decisão do autor
    // 2026-09-02) — todos silenciosos até serem cabeados, exceto uma voz
    // mínima ligada pra soar ao abrir.
    //
    // Duas ordens DIFERENTES de propósito:
    //  1. instanciação dos nós = ordem de `moduleCatalog()` -> índice de
    //     nó. A taxonomia foi consolidada e a ordem reordenada em
    //     2026-09-06; `tests/test_seed_patch.cpp` confirmou que
    //     `RASGO_SEED=N` e o CROSS saem BYTE-IDÊNTICOS antes/depois — o
    //     `seedPatch` cabeia a espinha por NOME de tipo (`firstT`), não
    //     por posição do passeio. Ainda assim: mexer aqui pede re-rodar
    //     esse teste.
    //  2. ordem de EXIBIÇÃO no rack (`shown`) = ordem da paleta: famílias
    //     na ordem do catálogo, módulos alfabéticos dentro da família,
    //     mas `MIXER`+`MASTER` grudados no fim (`sortFamilyForDisplay`).
    //     Só layout — `shown` já é reordenável arrastando e é salvo no
    //     `.panel`.
    SignalGraph graph;
    std::vector<std::size_t> shown;
    std::map<std::string, std::size_t> byType;
    for (const auto& g : rasgo::panel::moduleCatalog())
        for (const char* t : g.types) {
            auto n = rasgo::panel::makeModule(t);
            if (!n) continue;
            byType[t] = graph.add(std::move(n));
        }
    for (const auto& g : rasgo::panel::moduleCatalog()) {
        std::vector<const char*> ordered(g.types.begin(), g.types.end());
        sortFamilyForDisplay(ordered);
        for (const char* t : ordered) {
            const auto it = byType.find(t);
            if (it != byType.end()) shown.push_back(it->second);
        }
    }
    std::size_t sink = graph.add(std::make_unique<Out>());  // não exibido; o
                                     // load de patch pode reencontrar o OUT
    auto at = [&](const char* t) { return byType.at(t); };

    // voz mínima ligada: CLOCK -> ENVELOPE.gate ; OSC -> FILTER -> ENVELOPE
    // -> MIXER -> MASTER -> saída ; ENVELOPE.env -> FILTER.FC (cabo real)
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
    graph.node(at("MIXER")).setParameter("pan1", -0.35f);
    graph.connect(at("CLOCK"), 1, at("ENVELOPE"), 1);   // euclid -> gate
    graph.connect(at("OSC"), 2, at("FILTER"), 0);       // saw -> filtro
    graph.connect(at("FILTER"), 3, at("ENVELOPE"), 0);  // all -> VCA
    graph.connect(at("ENVELOPE"), 0, at("MIXER"), 0);
    graph.connect(at("MIXER"), 0, at("MASTER"), 0);
    graph.connect(at("MASTER"), 0, sink, 0);
    graph.connect(at("ENVELOPE"), 1, at("FILTER"), 1, /*feedback=*/true);

    // ---- áudio (estéreo) ----------------------------------------------
    // período de 256 (limite do AudioBlock) mas com um BUFFER FUNDO (8
    // períodos) = folga contra xrun (o "estalo" que soa como clip)
    // enquanto a UI e o re-prepare do grafo disputam a CPU
    rasgo::panel::AlsaSink alsa(48000, 2, 256);
    // o grafo processa em blocos de <=256 (limite do AudioBlock); um
    // período do ALSA pode ser maior -> processa em N sub-blocos por escrita
    const std::size_t period = static_cast<std::size_t>(alsa.period());
    const std::size_t block = std::min<std::size_t>(256, period);
    const std::size_t chunks = (period + block - 1) / block;
    graph.prepare(static_cast<float>(alsa.rate()), 2, block);
    // rack cheio: só processa os módulos que chegam ao `sink` — os órfãos
    // do catálogo não custam DSP por bloco (era a causa do "glitch")
    graph.setActiveOutput(sink);

    std::atomic<bool> running{true};
    std::atomic<bool> reprepare{false};
    std::mutex gmx;

    // ---- AUDIO-IN — captura ao vivo (adaptador OPCIONAL, nunca
    // dependência) --------------------------------------------------------
    // Só abre o dispositivo de captura ALSA quando o patch tem de fato um
    // nó `AUDIO-IN` -- o painel nunca pega o microfone/entrada de linha à
    // toa. Thread PRÓPRIA, separada da `audio` (que já toca o grafo) — de
    // propósito: a pacing da captura de verdade não deve arriscar a
    // temporização já delicada da reprodução (comentários de `pace()` no
    // `AlsaSink`). O único acoplamento é `AudioIn::pushSamples()`, que é
    // lock-free por dentro (SPSC).
    std::unique_ptr<rasgo::panel::AlsaSource> audioInDev;
    std::thread audioInThread;
    std::atomic<bool> audioInRunning{false};
    auto stopAudioIn = [&] {
        if (audioInRunning.exchange(false)) {
            // corta um `read()` bloqueado ANTES do join — senão a thread de
            // captura só sai quando o PipeWire entregar o próximo período, e
            // se ele travou o `join()` pendura a janela toda no encerramento.
            if (audioInDev) audioInDev->abort();
            if (audioInThread.joinable()) audioInThread.join();
        }
        audioInDev.reset();
    };

    // ---- SIGNAL-IN — entrada MIDI (ALSA sequencer) ----------------------
    // Abre uma porta virtual "RASGO Modular : IN" quando o patch tem um
    // `SIGNAL-IN`; o usuário conecta um teclado por `aconnect`/patchbay.
    // Thread de polling própria (não bloqueia; `SignalIn::pushMidi` é SPSC
    // lock-free). Falha em abrir → log e segue sem MIDI, como a captura.
    std::unique_ptr<rasgo::panel::AlsaMidi> midiIn;
    std::thread midiInThread;
    std::atomic<bool> midiInRunning{false};
    auto stopMidiIn = [&] {
        if (midiInRunning.exchange(false)) {
            if (midiInThread.joinable()) midiInThread.join();
        }
        midiIn.reset();
    };

    // chamada em todo ponto que já chama `buildMods()` — mesmo padrão do
    // `populateMotion()` — pra abrir/fechar a captura sempre que o
    // conjunto de nós do patch muda.
    auto syncSignalIn = [&] {
        bool any = false;
        for (std::size_t i = 0; i < graph.nodeCount(); ++i)
            if (graph.node(i).type() == "SIGNAL-IN") { any = true; break; }

        // ---- MIDI ----
        if (any && !midiInRunning.load()) {
            try {
                midiIn = std::make_unique<rasgo::panel::AlsaMidi>();
            } catch (const std::exception& e) {
                std::fprintf(stderr, "[signal-in] sem MIDI: %s\n", e.what());
                midiIn.reset();
            }
            if (midiIn) {
                midiInRunning.store(true);
                midiInThread = std::thread([&] {
                    while (midiInRunning.load(std::memory_order_relaxed)) {
                        if (gmx.try_lock()) {
                            midiIn->poll([&](std::uint8_t st, std::uint8_t d1,
                                             std::uint8_t d2) {
                                for (std::size_t i = 0; i < graph.nodeCount(); ++i) {
                                    if (graph.node(i).type() != "SIGNAL-IN") continue;
                                    auto* n = dynamic_cast<rasgo::modular::SignalIn*>(
                                        &graph.node(i));
                                    if (n) n->pushMidi(st, d1, d2);
                                }
                            });
                            gmx.unlock();
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                });
            }
        } else if (!any && midiInRunning.load()) {
            stopMidiIn();
        }

        // ---- ÁUDIO ----
        if (any && !audioInRunning.load()) {
            try {
                const char* dev = std::getenv("RASGO_AUDIO_IN_DEVICE");
                audioInDev = std::make_unique<rasgo::panel::AlsaSource>(
                    static_cast<unsigned>(alsa.rate()), 2, alsa.period(),
                    dev ? std::string(dev) : std::string("default"));
            } catch (const std::exception& e) {
                std::fprintf(stderr, "[audio-in] não abriu captura: %s\n", e.what());
                audioInDev.reset();
                return;
            }
            audioInRunning.store(true);
            audioInThread = std::thread([&] {
                std::vector<float> buf(audioInDev->period() * 2);
                while (audioInRunning.load(std::memory_order_relaxed)) {
                    if (!audioInDev->read(buf.data())) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        continue;
                    }
                    // só precisa do `gmx` pra enumerar os nós com segurança
                    // (a estrutura do grafo pode mudar na UI); o
                    // `pushSamples` em si não trava. Se a UI está com o
                    // lock (editando o patch), pula este período em vez de
                    // esperar — perde um pedaço de captura, nunca trava.
                    if (gmx.try_lock()) {
                        for (std::size_t i = 0; i < graph.nodeCount(); ++i) {
                            if (graph.node(i).type() != "SIGNAL-IN") continue;
                            auto* node = dynamic_cast<rasgo::modular::SignalIn*>(&graph.node(i));
                            if (node) node->pushSamples(buf.data(), audioInDev->period());
                        }
                        gmx.unlock();
                    }
                }
            });
        } else if (!any && audioInRunning.load()) {
            stopAudioIn();
        }
    };

    // gravação: acumula em memória (reservada, ~4 min estéreo) enquanto
    // grava; escreve o .wav ao parar. Cap: auto-para quando a reserva
    // enche. Não é RT-perfeito (o `insert` é memcpy limitado) — pra o
    // painel de teste basta; mover pra thread escritora é pendência.
    std::atomic<bool> recording{false};
    std::vector<float> recBuf;
    recBuf.reserve(static_cast<std::size_t>(alsa.rate()) * 2 * 240);
    // SYSTEM SCORE ao vivo (§5 do estudo) — acompanha a gravação de áudio
    // ([Ctrl+R]): topologia (cabos) no início da tomada + toda mudança de
    // parâmetro feita à mão enquanto grava. `t` = amostras já gravadas/sr
    // (nunca relógio de parede — mesma regra do `ScoreRecorder`), então é
    // relativo à TOMADA, não ao patch inteiro. Não captura ainda mudanças
    // de `MUTATE`/`EVOLVE`/Motion Engine durante a gravação (pendência
    // registrada) nem os links de modulação por cabo (`connectToParameter`
    // não tem um enumerador público no motor hoje).
    rasgo::panel::ScoreRecorder score;

    std::map<std::size_t, ScopeTrace> scopes;
    for (const auto id : shown) scopes[id];  // pré-aloca (fora do RT)

    // VU do MASTER com clip-latch: "o medidor esconde estouros" (achado
    // §3.3 da auditoria de saída, NAVALHA 2 — nunca corrigido até agora).
    // `Master::gainReductionDb()` já é telemetria pública acumulada (pico
    // de redução desde o último `clearTelemetry()`, nunca chamado hoje) —
    // então só ler > 0 já diz "o limitador teve que segurar algo", sem
    // precisar de estado novo no motor. Aqui só guardamos QUANDO foi visto
    // pela última vez, pra decair o indicador depois de alguns segundos em
    // vez de ficar aceso pra sempre.
    std::map<std::size_t, std::chrono::steady_clock::time_point> clipLastSeen;

    std::thread audio([&] {
        AudioBlock st(static_cast<float>(alsa.rate()), 2, block);
        std::vector<float> inter(chunks * block * 2);
        while (running.load(std::memory_order_relaxed)) {
            // PRIORIDADE À UI: se o thread de desenho / evento está com o
            // `gmx` (editando o grafo, salvando, etc.), o áudio NÃO espera —
            // emite um período de silêncio e tenta de novo. Uma edição de
            // patch custa alguns ms de silêncio; nunca custa a janela travar.
            std::unique_lock<std::mutex> lk(gmx, std::try_to_lock);
            if (!lk) {
                std::fill(inter.begin(), inter.end(), 0.0f);
                if (!alsa.write(inter.data()))
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
            {
                if (reprepare.exchange(false))
                    graph.prepare(static_cast<float>(alsa.rate()), 2, block);
                for (std::size_t c = 0; c < chunks; ++c) {
                    graph.process(st, sink, 0);
                    for (std::size_t i = 0; i < block; ++i) {
                        inter[2 * (c * block + i)] = st.at(0, i);
                        inter[2 * (c * block + i) + 1] = st.at(1, i);
                    }
                }
                if (recording.load(std::memory_order_relaxed)) {
                    if (recBuf.size() + period * 2 <= recBuf.capacity())
                        recBuf.insert(recBuf.end(), inter.begin(),
                                      inter.begin() + period * 2);
                    else
                        recording.store(false);   // reserva cheia -> auto-para
                    // MUSICAL SCORE — qualquer NOTE-OUT no patch pode ter
                    // fechado uma nota neste bloco; `t` é o início da
                    // nota (agora menos a duração), relativo à TOMADA
                    // (mesma unidade das mudanças de parâmetro já
                    // gravadas no Ctrl+R).
                    const double nowElapsed =
                        static_cast<double>(recBuf.size()) / 2.0 / alsa.rate();
                    for (const auto id : shown) {
                        auto* no = dynamic_cast<NoteOut*>(&graph.node(id));
                        if (!no) continue;
                        NoteOut::CompletedNote cn;
                        while (no->takeCompletedNote(cn))
                            score.note(nowElapsed - cn.durationSeconds, id,
                                      cn.pitch, cn.velocity, cn.durationSeconds,
                                      cn.accent);
                    }
                }
                // alimenta os osciloscópios dos módulos exibidos (saída 0)
                const std::size_t stride =
                    std::max<std::size_t>(1, block / 44);
                for (const auto id : shown) {
                    if (graph.node(id).outputCount() == 0) continue;
                    const AudioBlock* ob = graph.lastOutput(id, 0);
                    if (!ob) continue;
                    auto it = scopes.find(id);
                    if (it == scopes.end()) continue;
                    for (std::size_t i = 0; i < ob->frames(); i += stride)
                        it->second.push(ob->at(0, i));
                    // TRIGSEQ: histórico das 4 lanes de gate (t1-t4)
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
            lk.unlock();   // a escrita ALSA (bloqueante) fica FORA do lock
            if (!alsa.write(inter.data()))
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // ---- janela: monitor primário, ~88% da área, centrada -------------
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) { fprintf(stderr, "sem display X\n"); running = false; audio.join(); stopAudioIn(); stopMidiIn(); return 1; }
    const int scr = DefaultScreen(dpy);
    const rasgo::panel::MonitorRect mon = primaryMonitor(dpy);

    Colormap cmap = DefaultColormap(dpy, scr);
    auto C = [&](unsigned short r, unsigned short g, unsigned short b) {
        XColor c; c.red = r; c.green = g; c.blue = b;
        c.flags = DoRed | DoGreen | DoBlue;
        XAllocColor(dpy, cmap, &c);
        return c.pixel;
    };
    const Tokens T{
        C(0x1300, 0x1500, 0x1a00),  // background
        C(0x2600, 0x2b00, 0x3600),  // surface (módulo)
        C(0x0e00, 0x1000, 0x1500),  // recessed (display / poço)
        C(0x4800, 0x5000, 0x6000),  // line
        C(0xe000, 0xe400, 0xec00),  // text.primary
        C(0x8800, 0x9000, 0xa000),  // text.secondary
        C(0xff00, 0x9d00, 0x4c00),  // accent
        C(0xff00, 0x6b00, 0x5b00),  // warning
    };

    // cores de cabo — como um saco de cabos de patch: variam pra a case
    // ficar viva, com um leve viés por tipo de sinal (áudio = mais quente,
    // controle = mais frio). Escolhidas por hash determinístico do cabo.
    const unsigned long cableAudio[4] = {
        C(0xe600, 0x9a00, 0x4c00),  // âmbar
        C(0xd400, 0x6600, 0x5000),  // telha
        C(0xc200, 0x7a00, 0x9e00),  // rosa-poeira
        C(0xd8d8, 0xb000, 0x5c00),  // ouro
    };
    const unsigned long cableCtrl[4] = {
        C(0x5c00, 0xb0d0, 0xaa00),  // verde-água
        C(0x7a00, 0x9e00, 0xd400),  // azul-poeira
        C(0x9a00, 0xc200, 0x7a00),  // verde suave
        C(0x9a00, 0x8ad0, 0xc800),  // violeta claro
    };
    // escala divergente da grade da MATRIX: ganho negativo (frio) ↔ 0 ↔
    // positivo (quente); usada também pelo espectro do SCOPE
    const unsigned long cellPos = C(0xff00, 0x9d00, 0x4c00);   // âmbar (=accent)
    const unsigned long cellNeg = C(0x5c00, 0xb0d0, 0xd400);   // ciano frio

    // fonte UTF-8 (locale fontset) com fallback pra fonte de núcleo
    char** miss = nullptr; int nmiss = 0; char* defstr = nullptr;
    XFontSet fs = XCreateFontSet(
        dpy, "-*-*-medium-r-normal--12-*-*-*-*-*-*-*,-*-*-*-*-*--12-*",
        &miss, &nmiss, &defstr);
    if (miss) XFreeStringList(miss);
    // fonte MENOR pros rótulos de widget (knob/toggle/jack) — a fonte
    // não escala com o zoom, então uma menor evita o texto invadir o
    // widget vizinho no zoom mínimo.
    char** miss2 = nullptr; int nmiss2 = 0; char* defstr2 = nullptr;
    XFontSet fsCap = XCreateFontSet(
        dpy, "-*-*-medium-r-normal--9-*-*-*-*-*-*-*,-*-*-*-*-*--9-*,"
             "-*-*-medium-r-normal--10-*-*-*-*-*-*-*",
        &miss2, &nmiss2, &defstr2);
    if (miss2) XFreeStringList(miss2);
    if (!fsCap) fsCap = fs;
    XFontStruct* coreFont = fs ? nullptr : XLoadQueryFont(dpy, "fixed");
    GC gc = XCreateGC(dpy, DefaultRootWindow(dpy), 0, nullptr);
    if (coreFont) XSetFont(dpy, gc, coreFont->fid);

    // ---- layout de "case": módulos numa fileira que quebra em LINHAS --
    // Mod::w em px é recalculado no relayout (depende de `g_s`).
    struct Mod { std::size_t id; int col; int w; int hp; };
    std::vector<Mod> mods;
    rasgo::panel::MotionEngine motion;
    bool motionOn = true;     // VARIA / [v] -- variação ao vivo dos knobs (±20%), ligada
    std::uint64_t curSeed = 0;   // seed do patch atual (0 = editado à mão)
    auto buildMods = [&] {
        mods.clear();
        for (const auto id : shown) {
            const Panel pn = graph.node(id).panel();
            mods.push_back({id, 0, 0, pn.hp});
            for (std::size_t i = 0; i < pn.widgets.size(); ++i)
                for (std::size_t j = i + 1; j < pn.widgets.size(); ++j)
                    if (overlapMM(footprintMM(pn.widgets[i]),
                                  footprintMM(pn.widgets[j])))
                        fprintf(stderr, "[sobreposicao] %s: '%s' x '%s'\n",
                                graph.node(id).type().c_str(),
                                pn.widgets[i].label.c_str(),
                                pn.widgets[j].label.c_str());
        }
    };

    // ---- variação ao vivo (Motion Engine, [v]) -------------------------
    // "instrumento de composição, não de regras prontas" (autor,
    // 2026-09-07). A engine v3 (`ESTUDO §3.7`) é uma MÃO CAÓTICA: um campo
    // de Thomas move TODAS as fibras (knob/slider/toggle de todo módulo,
    // menos MIXER/MASTER) em relação; a velocidade do campo segue a
    // energia do som. "Gosto" = só a amplitude por fibra, de 4 pistas de
    // palavra. Ver `MotionEngine::inhabit()` / `MotionField.hpp`.
    auto populateMotion = [&] {
        motion.inhabit(graph, curSeed, shown);
    };

    buildMods();
    populateMotion();
    syncSignalIn();

    // auditoria: checa a pegada (em mm) de TODO módulo do catálogo, não só
    // os exibidos - assim o `timeout` de smoke-test cobre os 16 painéis.
    {
        int probs = 0;
        for (const auto& g : rasgo::panel::moduleCatalog())
            for (const char* t : g.types) {
                auto n = rasgo::panel::makeModule(t);
                if (!n) continue;
                const Panel pn = n->panel();
                for (std::size_t i = 0; i < pn.widgets.size(); ++i)
                    for (std::size_t j = i + 1; j < pn.widgets.size(); ++j)
                        if (overlapMM(footprintMM(pn.widgets[i]),
                                      footprintMM(pn.widgets[j]))) {
                            ++probs;
                            fprintf(stderr, "[sobreposicao] %s: '%s' x '%s'\n",
                                    t, pn.widgets[i].label.c_str(),
                                    pn.widgets[j].label.c_str());
                        }
            }
        fprintf(stderr, "[painel] auditoria de pegada: %d sobreposicao(oes)\n",
                probs);
    }

    // ---- paleta: módulos disponíveis, por família --------------------
    // Mesma ordem do rack inicial (`shown`, ver acima):
    // `sortFamilyForDisplay` — alfabético dentro da família, `MIXER`+
    // `MASTER` grudados no fim. Só EXIBIÇÃO — a ordem de
    // `ModuleCatalog.hpp::moduleCatalog()` (instanciação de nós, doador do
    // CROSS) segue sua própria ordem — ver o comentário do bloco do grafo.
    struct PaletteRow { int y; bool header; std::string label; std::string type; };
    std::vector<PaletteRow> palette;
    {
        int py = kCaseTop + 6;
        for (const auto& g : rasgo::panel::moduleCatalog()) {
            palette.push_back({py, true, g.family, ""});
            py += 17;
            std::vector<const char*> sorted = g.types;
            sortFamilyForDisplay(sorted);
            for (const char* t : sorted) {
                palette.push_back({py, false, std::string("  ") + t, t});
                py += 15;
            }
            py += 6;
        }
    }
    int paletteScroll = 0, paletteH = palette.empty() ? 0
        : palette.back().y + 20 - kCaseTop;

    // largura de referência do conteúdo = paleta + case de 104 HP a s=2,0
    const int refW = kPaletteW + kCasePad
        + static_cast<int>(std::lround(kRackHP * kMMHP * 2.0f)) + kCasePad;
    // altura de referência: 3 linhas de 3U a s=2,0
    const int refH = kCaseTop + kCasePad
        + 3 * (static_cast<int>(std::lround(kMM3U * 2.0f)) + kCasePad);

    rasgo::panel::WindowBounds wb =
        rasgo::panel::firstOpen(mon, refW, refH, 0.88f);
    wb.h = std::min(std::max(wb.h, kCaseTop + kCasePad + 260),
                    static_cast<int>(mon.h * 0.9f));

    Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr),
                                     wb.x, wb.y, wb.w, wb.h, 0, T.line, T.bg);

    // Título da janela em UTF-8 (metodologia RASGO: todo texto é UTF-8).
    // `XStoreName` sozinho grava `WM_NAME` como STRING (Latin-1) — um "—"
    // ou "●" vira "â€"" na barra de título. O gerenciador de janelas
    // moderno lê `_NET_WM_NAME` como `UTF8_STRING`; setamos os dois (o
    // `XStoreName` fica só de fallback pra WM antigo).
    const Atom aNetWmName = XInternAtom(dpy, "_NET_WM_NAME", False);
    const Atom aNetWmIconName = XInternAtom(dpy, "_NET_WM_ICON_NAME", False);
    const Atom aUtf8 = XInternAtom(dpy, "UTF8_STRING", False);
    auto setTitle = [&](const std::string& s) {
        const auto* d = reinterpret_cast<const unsigned char*>(s.data());
        const int n = static_cast<int>(s.size());
        XChangeProperty(dpy, win, aNetWmName, aUtf8, 8, PropModeReplace, d, n);
        XChangeProperty(dpy, win, aNetWmIconName, aUtf8, 8, PropModeReplace, d, n);
        XStoreName(dpy, win, s.c_str());
    };
    setTitle("RASGO Modular — painel de teste");
    XSizeHints* sh = XAllocSizeHints();
    sh->flags = PMinSize; sh->min_width = 480; sh->min_height = 320;
    XSetWMNormalHints(dpy, win, sh); XFree(sh);
    XSelectInput(dpy, win, ExposureMask | ButtonPressMask | ButtonReleaseMask
                 | PointerMotionMask | KeyPressMask | StructureNotifyMask);
    Atom wmDel = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDel, 1);
    XMapWindow(dpy, win);
    XMoveWindow(dpy, win, wb.x, wb.y);

    int winW = wb.w, winH = wb.h, scrollY = 0, caseH = 260, modH = 260;
    int rackX0 = kPaletteW + kCasePad;   // origem da case (recalc no relayout)
    float uiZoom = 1.0f;                 // Ctrl+= / Ctrl+- / Ctrl+0

    // idioma da UI (cabeçalho/tutorial/créditos) — inglês por padrão, como
    // os RASGO Synth; botão IDIOMA cicla EN→PT→FR→ES. Carregado do pref
    // `ui-lang` logo abaixo. `overlay`: 0 nenhum · 1 tutorial · 2 sobre.
    rasgo::panel::Lang uiLang = rasgo::panel::Lang::en;
    int overlay = 0;

    // recalcula `g_s` pela altura, as larguras px, e distribui os módulos
    // numa case de largura de rack (104 HP), centrada, que quebra em linhas
    const int caseLeft = kPaletteW + kCasePad;
    auto relayout = [&] {
        const int caseAvailH = std::max(120, winH - kCaseTop - kCasePad);
        g_s = (static_cast<float>(caseAvailH) / kTargetRows - kCasePad) / kMM3U;
        g_s = std::min(kSMax, std::max(kSMin, g_s));
        // zoom de conteúdo (Ctrl+=/-): no-op em uiZoom==1.0; deixa `g_s` cair
        // abaixo de kSMin ao reduzir (é o objetivo — panorama pra cabear).
        g_s = std::min(kSMax * 1.3f, std::max(kSMin * 0.5f, g_s * uiZoom));
        modH = mmpx(kMM3U);
        const int gapPx = mmpx(kModGapMM);
        for (auto& m : mods) m.w = mmpx(static_cast<float>(m.hp) * kMMHP);

        // a case ENCHE a largura disponível — começa colada na paleta,
        // sem margem vazia (o autor pediu aproveitamento máximo da tela).
        // O rack de 104 HP fica só como largura de referência da 1ª abertura.
        const int availW = std::max(200, winW - caseLeft - kCasePad);
        const int rackW = availW;
        rackX0 = caseLeft;

        int x = rackX0, row = 0;
        for (auto& m : mods) {
            if (x + m.w > rackX0 + rackW && x > rackX0) { x = rackX0; ++row; }
            m.col = (row << 20) | x;
            x += m.w + gapPx;
        }
        caseH = (row + 1) * (modH + kCasePad);
        const int maxScroll = std::max(0, caseH - (winH - kCaseTop));
        scrollY = std::min(std::max(0, scrollY), maxScroll);
    };
    relayout();

    // buffer fora da tela (evita flicker: desenha tudo aqui, depois copia
    // pra a janela numa passada só)
    const int depth = DefaultDepth(dpy, scr);
    Pixmap bb = XCreatePixmap(dpy, win, static_cast<unsigned>(std::max(1, winW)),
                              static_cast<unsigned>(std::max(1, winH)),
                              static_cast<unsigned>(depth));
    auto resizeBuf = [&] {
        XFreePixmap(dpy, bb);
        bb = XCreatePixmap(dpy, win, static_cast<unsigned>(std::max(1, winW)),
                           static_cast<unsigned>(std::max(1, winH)),
                           static_cast<unsigned>(depth));
    };

    // véu semi-transparente pros overlays (tutorial / sobre) — um stipple
    // de ~62% deixa os módulos VISÍVEIS por trás, só escurecidos, em vez
    // de sumirem (pedido do autor 2026-09-07). X11 puro não tem alfa.
    static const char kDimBits[8] = { '\x6D', '\xB6', '\xDB', '\x6D',
                                      '\xB6', '\xDB', '\x6D', '\xB6' };
    const Pixmap dimStipple =
        XCreateBitmapFromData(dpy, win, kDimBits, 8, 8);

    // marca RASGO — pré-composta UMA vez num Pixmap: a cobertura em tons
    // de cinza do SVG (com anti-aliasing) é misturada do fundo pra a cor
    // de acento, então o desenho a cada quadro é só um XCopyArea. Sem
    // dependência de imagem — a cobertura vem de `assets/rasgo_logo_gray.h`
    // (gerado por `regen_logo.sh`).
    const Pixmap logoPix = XCreatePixmap(
        dpy, win, static_cast<unsigned>(rasgo_logo_w),
        static_cast<unsigned>(rasgo_logo_h), static_cast<unsigned>(depth));
    {
        // rampa fundo→acento (33 níveis) via o mesmo alocador de cor
        unsigned long ramp[33];
        for (int k = 0; k <= 32; ++k) {
            const float t = static_cast<float>(k) / 32.0f;
            auto mix = [&](int bg8, int ac8) {
                return static_cast<unsigned short>(
                    (bg8 + (ac8 - bg8) * t) * 256.0f);
            };
            ramp[k] = C(mix(0x13, 0xff), mix(0x15, 0x9d), mix(0x1a, 0x4c));
        }
        GC tmp = XCreateGC(dpy, logoPix, 0, nullptr);
        XImage* img = XCreateImage(dpy, DefaultVisual(dpy, scr),
                                   static_cast<unsigned>(depth), ZPixmap, 0,
                                   nullptr, static_cast<unsigned>(rasgo_logo_w),
                                   static_cast<unsigned>(rasgo_logo_h), 32, 0);
        char* buf = img ? static_cast<char*>(std::malloc(
            static_cast<std::size_t>(img->bytes_per_line)
            * static_cast<std::size_t>(rasgo_logo_h))) : nullptr;
        if (img && buf) {
            img->data = buf;
            for (int y = 0; y < rasgo_logo_h; ++y)
                for (int x = 0; x < rasgo_logo_w; ++x) {
                    const int cov = rasgo_logo_gray[y * rasgo_logo_w + x];
                    XPutPixel(img, x, y, ramp[(cov * 32) / 255]);
                }
            XPutImage(dpy, logoPix, tmp, img, 0, 0, 0, 0,
                      static_cast<unsigned>(rasgo_logo_w),
                      static_cast<unsigned>(rasgo_logo_h));
            XDestroyImage(img);   // libera img->data também
        } else {
            // fallback improvável (XCreateImage/malloc falhou): só o fundo
            if (img) XDestroyImage(img);
            XSetForeground(dpy, gc, T.bg);
            XFillRectangle(dpy, logoPix, gc, 0, 0,
                           static_cast<unsigned>(rasgo_logo_w),
                           static_cast<unsigned>(rasgo_logo_h));
        }
        XFreeGC(dpy, tmp);
    }

    auto text = [&](int x, int y, const std::string& s, unsigned long c) {
        XSetForeground(dpy, gc, c);
        if (fs) Xutf8DrawString(dpy, bb, fs, gc, x, y, s.c_str(),
                                static_cast<int>(s.size()));
        else XDrawString(dpy, bb, gc, x, y, s.c_str(),
                         static_cast<int>(s.size()));
    };
    // rótulo de widget: fonte menor, centrado em `cx` (baseline em `y`)
    auto capText = [&](int cx, int y, const std::string& s, unsigned long c) {
        int tw = static_cast<int>(s.size()) * 6;
        if (fsCap) {
            XRectangle ink, log;
            Xutf8TextExtents(fsCap, s.c_str(), static_cast<int>(s.size()),
                             &ink, &log);
            tw = log.width;
        }
        const int x = cx - tw / 2;
        XSetForeground(dpy, gc, c);
        if (fsCap) Xutf8DrawString(dpy, bb, fsCap, gc, x, y, s.c_str(),
                                   static_cast<int>(s.size()));
        else XDrawString(dpy, bb, gc, x, y, s.c_str(),
                         static_cast<int>(s.size()));
    };

    struct Drag { bool active = false; std::size_t node = 0; std::string bind;
        float startVal = 0, lo = 0, hi = 1; int startY = 0; } drag;
    std::map<std::size_t, int> scopeView;   // SCOPE: 0 = onda, 1 = espectro
    std::string spawnType;   // arrastando um módulo da paleta pra a case
    int spawnX = 0, spawnY = 0;
    int mouseX = 0, mouseY = 0;

    // ---- cabeamento jack-a-jack (decisão do autor 2026-09-02) -----------
    struct JackScreen {
        std::size_t node = 0;
        int port = 0;
        bool isOut = false;
        PortKind kind = PortKind::Audio;
        int x = 0, y = 0;
    };
    std::vector<JackScreen> jacks;
    struct CableDrag {
        bool active = false;
        std::size_t node = 0;   // ponta ANCORADA
        int port = 0;
        bool fromOutput = true; // a ponta ancorada é uma saída?
        PortKind kind = PortKind::Audio;
        int ax = 0, ay = 0;
    } cdrag;
    bool allRuptured = false;
    struct ModDrag { bool active = false; std::size_t id = 0; } mdrag;
    struct { bool active = false; int grabOff = 0; } palBarDrag;
    // geometria da barra de scroll da paleta (nulo se a lista cabe toda)
    auto palBar = [&](int& trkY, int& trkH, int& thY, int& thH) -> bool {
        const int viewH = winH - kLearnH - kCaseTop;
        const int contentH = paletteH + 30;   // +30 = folga do clamp do wheel
        if (contentH <= viewH + 4) return false;
        trkY = kCaseTop + 3;
        trkH = viewH - 6;
        const int palMax = std::max(1, contentH - viewH);
        thH = std::max(18, trkH * viewH / contentH);
        if (thH > trkH) thH = trkH;
        thY = trkY + (trkH - thH)
            * std::min(std::max(0, paletteScroll), palMax) / palMax;
        return true;
    };
    auto palMaxScroll = [&] {
        return std::max(0, paletteH - (winH - kLearnH - kCaseTop) + 30);
    };

    auto modOrigin = [&](const Mod& m) {
        const int rowIdx = m.col >> 20;
        const int x = m.col & 0xFFFFF;
        const int y = kCaseTop + kCasePad + rowIdx * (modH + kCasePad)
            - scrollY;
        return std::pair<int, int>(x, y);
    };

    // posições de tela de todos os jacks dos módulos exibidos (dependem do
    // scroll e do layout — recalculado a cada quadro / hit-test)
    auto rebuildJacks = [&] {
        jacks.clear();
        for (const auto& m : mods) {
            const auto [bx, by] = modOrigin(m);
            Signal& node = graph.node(m.id);
            for (const auto& w : node.panel().widgets) {
                if (w.kind != Widget::Kind::Jack) continue;
                const bool isOut = w.bind.rfind("out:", 0) == 0;
                const std::string port = w.bind.substr(isOut ? 4 : 3);
                int pidx = -1;
                PortKind kind = PortKind::Audio;
                if (isOut) {
                    for (std::size_t i = 0; i < node.outputCount(); ++i)
                        if (node.outputDescriptor(i).name == port) {
                            pidx = static_cast<int>(i);
                            kind = node.outputDescriptor(i).kind;
                        }
                } else {
                    for (std::size_t i = 0; i < node.inputCount(); ++i)
                        if (node.inputDescriptor(i).name == port) {
                            pidx = static_cast<int>(i);
                            kind = node.inputDescriptor(i).kind;
                        }
                }
                if (pidx < 0) continue;
                jacks.push_back({m.id, pidx, isOut, kind,
                                 bx + mmpx(w.x), by + mmpx(w.y)});
            }
        }
    };
    auto jackAt = [&](const int mx, const int my) -> int {
        const int r = mmpx(3.2f) + 2;
        for (std::size_t i = 0; i < jacks.size(); ++i) {
            const int dx = mx - jacks[i].x, dy = my - jacks[i].y;
            if (dx * dx + dy * dy <= r * r) return static_cast<int>(i);
        }
        return -1;
    };
    // curva do cabo (bezier quadrática com barriga pra baixo)
    auto drawCable = [&](int x0, int y0, int x1, int y1, unsigned long col,
                         bool dashed) {
        const int sag = 18 + std::abs(x1 - x0) / 6;
        const int cx = (x0 + x1) / 2, cy = std::max(y0, y1) + sag;
        XPoint pts[15];
        for (int k = 0; k < 15; ++k) {
            const float t = static_cast<float>(k) / 14.0f;
            const float u = 1.0f - t;
            pts[k].x = static_cast<short>(u * u * x0 + 2 * u * t * cx + t * t * x1);
            pts[k].y = static_cast<short>(u * u * y0 + 2 * u * t * cy + t * t * y1);
        }
        XSetForeground(dpy, gc, col);
        XSetLineAttributes(dpy, gc, dashed ? 1 : 2,
                           dashed ? LineOnOffDash : LineSolid, CapButt, JoinRound);
        XDrawLines(dpy, bb, gc, pts, 15, CoordModeOrigin);
        XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinRound);
    };

    auto clipTo = [&](int x, int y, int w, int h) {
        XRectangle r{static_cast<short>(x), static_cast<short>(y),
                     static_cast<unsigned short>(std::max(0, w)),
                     static_cast<unsigned short>(std::max(0, h))};
        XSetClipRectangles(dpy, gc, 0, 0, &r, 1, Unsorted);
    };
    auto clipOff = [&] { XSetClipMask(dpy, gc, None); };

    std::map<std::size_t, ScopeTrace> scopeSnap;
    std::uint64_t seedNum = 0;

    // ---- cabeçalho de linha única (modelo RASGO Synth) ----------------
    // `redraw` monta `headerHits` a cada quadro; o laço de evento lê a
    // lista pra rotear o clique pro `act*` certo. `hdrFlash` acende um
    // botão momentâneo por ~160 ms depois de acionado.
    enum HdrAct { HA_NONE, HA_SEED, HA_REC, HA_LANG, HA_TUTORIAL, HA_ABOUT,
                  HA_VARY, HA_STANDBY, HA_MUTATE, HA_EVOLVE, HA_CROSS,
                  HA_BANK, HA_SAVE, HA_ZOUT, HA_ZIN };
    struct HdrHit { int x, y, w, h; HdrAct act; };
    std::vector<HdrHit> headerHits;
    std::map<int, std::chrono::steady_clock::time_point> hdrFlash;

    // dwell da caixa LEARN — ver o bloco no `redraw`
    std::string learnHoverKey;
    std::chrono::steady_clock::time_point learnHoverSince{};
    const rasgo::panel::LearnEntry* learnShown = nullptr;
    std::string learnShownTitle;
    // rótulo do botão SEED (o número pode ter até 9 dígitos no [g], e um
    // `RASGO_SEED=N` pode ter mais — abrevia com "…" acima de 10 dígitos)
    auto seedLabel = [&](char* out, const std::size_t n) {
        if (seedNum == 0) { std::snprintf(out, n, "\xE2\x9A\x84 SEED"); return; }
        char num[24];
        std::snprintf(num, sizeof num, "%llu",
                      static_cast<unsigned long long>(seedNum));
        if (std::strlen(num) > 10) { num[9] = '\0'; std::strcat(num, "\xE2\x80\xA6"); }
        std::snprintf(out, n, "\xE2\x9A\x84 SEED %s", num);
    };
    // quebra `s` (com '\n' respeitados) em linhas que cabem em `maxPx`.
    auto textW = [&](const std::string& t) -> int {
        if (t.empty()) return 0;
        if (fs) {
            XRectangle ink, logical;
            return Xutf8TextExtents(fs, t.c_str(),
                                   static_cast<int>(t.size()), &ink, &logical);
        }
        return static_cast<int>(t.size()) * 6;
    };
    auto wrapText = [&](const std::string& s, int maxPx) -> std::vector<std::string> {
        std::vector<std::string> out;
        std::string para;
        auto emitPara = [&] {
            std::string line, word;
            auto pushWord = [&] {
                if (word.empty()) return;
                const std::string cand = line.empty() ? word : line + " " + word;
                if (!line.empty() && textW(cand) > maxPx) {
                    out.push_back(line);
                    line = word;
                } else {
                    line = cand;
                }
                word.clear();
            };
            for (const char c : para) {
                if (c == ' ') pushWord();
                else word += c;
            }
            pushWord();
            out.push_back(line);
        };
        for (const char c : s) {
            if (c == '\n') { emitPara(); para.clear(); }
            else para += c;
        }
        emitPara();
        return out;
    };

    auto redraw = [&] {
        rebuildJacks();
        // snapshot dos osciloscópios SEM bloquear: se o thread de áudio
        // estiver com o `gmx`, a UI usa o snapshot anterior e segue — nunca
        // congela à espera do áudio.
        if (gmx.try_lock()) { scopeSnap = scopes; gmx.unlock(); }
        XSetForeground(dpy, gc, T.bg);
        XFillRectangle(dpy, bb, gc, 0, 0, winW, winH);

        // ---- LEARN: acha o widget sob o mouse (o texto vai numa caixa
        // fixa no rodapé da coluna esquerda, estilo terminal — NÃO flutua
        // sobre o painel; modelo do ANTITOTEM, pedido do autor 2026-09-05)
        const rasgo::panel::LearnEntry* rawHit = nullptr;
        std::string rawTitle, rawKey;
        {
            for (const auto& m : mods) {
                const auto [bx, by] = modOrigin(m);
                if (mouseX < bx || mouseX > bx + m.w
                    || mouseY < by || mouseY > by + modH) continue;
                Signal& node = graph.node(m.id);
                const std::string mt = node.type();
                bool widgetHit = false;
                for (const auto& w : node.panel().widgets) {
                    if (w.bind.empty()) continue;
                    Rect fp = footprintPx(w);
                    fp.x += bx; fp.y += by;
                    if (mouseX < fp.x || mouseX > fp.x + fp.w
                        || mouseY < fp.y || mouseY > fp.y + fp.h) continue;
                    if (const auto* e = rasgo::panel::lookupLearn(mt, w.bind)) {
                        rawHit = e;
                        rawTitle = mt + "  \xC2\xB7  " + w.label;
                        rawKey = std::to_string(m.id) + "|" + w.bind;
                        widgetHit = true;
                    }
                    break;
                }
                // sobre o CORPO do módulo (não num controle) — inclui o
                // título: o LEARN mostra o que o MÓDULO é (pedido do autor
                // 2026-09-07).
                if (!widgetHit) {
                    if (const auto* e = rasgo::panel::lookupLearnModule(mt)) {
                        rawHit = e;
                        rawTitle = mt;
                        rawKey = std::to_string(m.id) + "|\x01mod";
                    }
                }
                break;
            }
        }
        // dwell: o conteúdo da caixa LEARN só troca depois de ~1 s parado
        // sobre o MESMO objeto — senão pisca a cada movimento do mouse
        // (autor 2026-09-05; encurtado de 2 s pra 1 s em 2026-09-07). Fora
        // de qualquer objeto, mantém o último.
        {
            const auto now = std::chrono::steady_clock::now();
            if (rawKey != learnHoverKey) {
                learnHoverKey = rawKey;
                learnHoverSince = now;
            } else if (!rawKey.empty() && rawHit != learnShown
                       && now - learnHoverSince >= std::chrono::milliseconds(1000)) {
                learnShown = rawHit;
                learnShownTitle = rawTitle;
            }
        }
        const rasgo::panel::LearnEntry* learnHit = learnShown;
        const std::string& learnHitTitle = learnShownTitle;

        // ==== CABEÇALHO (linha única, altura kCaseTop) =================
        // modelo dos RASGO Synth: wordmark · barra de comandos ·
        // leitura de estado · pico · SEED · REC · IDIOMA/TUTORIAL/SOBRE.
        // Tudo via `tr()` (idioma corrente); rótulos de módulo NÃO.
        namespace S = rasgo::panel::strings;
        using rasgo::panel::tr;
        headerHits.clear();
        const auto hdrNow = std::chrono::steady_clock::now();
        auto flashing = [&](int a) {
            auto it = hdrFlash.find(a);
            return it != hdrFlash.end()
                && hdrNow - it->second < std::chrono::milliseconds(160);
        };
        const int hbY = 10, hbH = 22, hbBase = hbY + 15;
        // Um botão do cabeçalho. `active` = TOGGLE ligado (estado que
        // fica) — ganha ANEL externo, destaque persistente. `flashing` =
        // clique momentâneo (MUTA/EVOLUI/…) — só um preenchimento de
        // ~160 ms, SEM anel. `onCol` distingue REC (vermelho) dos demais.
        auto hdrBtnC = [&](int x, const std::string& label, bool active,
                           HdrAct act, unsigned long onCol) -> int {
            const int pad = 7, w = textW(label) + pad * 2;
            const bool lit = active || flashing(act);
            if (lit) { XSetForeground(dpy, gc, onCol);
                       XFillRectangle(dpy, bb, gc, x, hbY, w, hbH); }
            XSetForeground(dpy, gc, lit ? onCol : T.line);
            XDrawRectangle(dpy, bb, gc, x, hbY, w, hbH);
            if (active)   // toggle LIGADO: anel de destaque (idioma do
                          // realce de módulo arrastado, §case)
                XDrawRectangle(dpy, bb, gc, x - 2, hbY - 2, w + 3, hbH + 3);
            text(x + pad, hbBase, label, lit ? T.bg : T.textSecondary);
            headerHits.push_back({x, hbY, w, hbH, act});
            return w;
        };
        auto hdrBtn = [&](int x, const std::string& label, bool active,
                          HdrAct act) {
            return hdrBtnC(x, label, active, act, T.accent);
        };

        // wordmark: marca RASGO (pré-composta, anti-aliased) + "MODULAR"
        int hx = kPaletteW + kCasePad;
        XCopyArea(dpy, logoPix, bb, gc, 0, 0,
                  static_cast<unsigned>(rasgo_logo_w),
                  static_cast<unsigned>(rasgo_logo_h),
                  hx, hbY + (hbH - rasgo_logo_h) / 2);
        hx += rasgo_logo_w + 7;
        text(hx, hbBase, "MODULAR", T.textSecondary);
        hx += textW("MODULAR") + 12;
        XSetForeground(dpy, gc, T.line);
        XDrawLine(dpy, bb, gc, hx - 6, hbY, hx - 6, hbY + hbH);

        // ---- cluster da direita, montado da borda pra dentro ----------
        int rx = winW - 12;
        auto hdrBtnR = [&](const std::string& label, bool active, HdrAct act,
                           unsigned long onCol = 0) {
            if (onCol == 0) onCol = T.accent;
            const int padL = 7, w = textW(label) + padL * 2;
            rx -= w;
            const bool lit = active || flashing(act);
            if (lit) { XSetForeground(dpy, gc, onCol);
                       XFillRectangle(dpy, bb, gc, rx, hbY, w, hbH); }
            XSetForeground(dpy, gc, lit ? onCol : T.line);
            XDrawRectangle(dpy, bb, gc, rx, hbY, w, hbH);
            if (active)   // toggle LIGADO: anel de destaque
                XDrawRectangle(dpy, bb, gc, rx - 2, hbY - 2, w + 3, hbH + 3);
            text(rx + padL, hbBase, label, lit ? T.bg : T.textSecondary);
            headerHits.push_back({rx, hbY, w, hbH, act});
            rx -= 6;
        };
        hdrBtnR(tr(S::hdrAbout, uiLang), overlay == 2, HA_ABOUT);
        hdrBtnR(tr(S::hdrTutorial, uiLang), overlay == 1, HA_TUTORIAL);
        hdrBtnR(rasgo::panel::langLabel(uiLang), false, HA_LANG);
        rx -= 8;
        // REC — toggle vermelho (T.warning), com o mesmo anel de destaque
        hdrBtnR(std::string("\xE2\x97\x8F ") + tr(S::hdrRec, uiLang),
                recording.load(), HA_REC, T.warning);
        rx -= 2;
        {   // SEED — sempre cheio (cor de acento), como o botão antigo
            char lab[40]; seedLabel(lab, sizeof lab);
            const int w = textW(lab) + 16;
            rx -= w;
            XSetForeground(dpy, gc, T.accent);
            XFillRectangle(dpy, bb, gc, rx, hbY, w, hbH);
            text(rx + 8, hbBase, lab, T.bg);
            headerHits.push_back({rx, hbY, w, hbH, HA_SEED});
            rx -= 10;
        }

        // pico da saída do MASTER (lê o snapshot dos osciloscópios) e o
        // estado de STANDBY (= `mute` de algum MASTER — o botão do
        // cabeçalho e o toggle do módulo compartilham o mesmo estado)
        float mPeak = 0.0f;
        bool masterMuted = false;
        for (const auto& m : mods)
            if (graph.node(m.id).type() == "MASTER") {
                if (graph.parameterUserValue(m.id, "mute") >= 0.5f)
                    masterMuted = true;
                auto it = scopeSnap.find(m.id);
                if (it != scopeSnap.end())
                    for (const float v : it->second.buf)
                        mPeak = std::max(mPeak, std::fabs(v));
            }
        const float mDb = mPeak > 1.0e-4f ? 20.0f * std::log10(mPeak) : -60.0f;
        {
            const std::string peakTxt = tr(S::rdPeak, uiLang);
            char db[16]; std::snprintf(db, sizeof db, "%.0f", mDb);
            const int mw = 44, mh = 8;
            const int grpW = textW(peakTxt) + 6 + mw + 6 + textW(db);
            if (rx - hx > grpW + 44) {
                rx -= grpW;
                int gx = rx;
                text(gx, hbBase, peakTxt, T.textSecondary);
                gx += textW(peakTxt) + 6;
                const int my = hbY + (hbH - mh) / 2;
                const float frac = std::min(1.0f, mPeak / 0.891f);
                XSetForeground(dpy, gc, T.recessed);
                XFillRectangle(dpy, bb, gc, gx, my, mw, mh);
                XSetForeground(dpy, gc, frac < 0.85f ? T.accent : T.warning);
                if (frac > 0.001f)
                    XFillRectangle(dpy, bb, gc, gx, my,
                                   static_cast<int>(frac * mw), mh);
                gx += mw + 6;
                text(gx, hbBase, db,
                     frac < 0.85f ? T.textSecondary : T.warning);
                rx -= 12;
            }
        }
        {   // leitura N mód · M cabos
            const std::string rd = std::to_string(mods.size()) + " "
                + tr(S::rdModules, uiLang) + "  \xC2\xB7  "
                + std::to_string(graph.cableCount()) + " "
                + tr(S::rdCables, uiLang);
            if (rx - hx > textW(rd) + 24) {
                rx -= textW(rd);
                text(rx, hbBase, rd, T.textSecondary);
                rx -= 12;
            }
        }

        // ---- barra de comandos ao centro (enche o vão; corta à direita)
        // `MUTE` fica FORA do bloco — isolado no fim, depois do ZOOM e de
        // um vão largo, pra não clicar nele sem querer (pedido do autor).
        struct CmdB { const rasgo::panel::L4* label; bool active; HdrAct act; };
        const CmdB cmds[] = {
            {&S::hdrVary,   motionOn,    HA_VARY},
            {&S::hdrChange, false,       HA_MUTATE},
            {&S::hdrEvolve, false,       HA_EVOLVE},
            {&S::hdrCross,  false,       HA_CROSS},
            {&S::hdrBank,   false,       HA_BANK},
            {&S::hdrSave,   false,       HA_SAVE},
        };
        for (const auto& cb : cmds) {
            const std::string L = tr(*cb.label, uiLang);
            if (hx + textW(L) + 14 > rx - 8) break;
            hx += hdrBtn(hx, L, cb.active, cb.act) + 5;
        }
        if (hx + 24 + 40 < rx - 8) {   // ZOOM − / +
            text(hx, hbBase, "ZOOM", T.textSecondary);
            hx += textW("ZOOM") + 5;
            hx += hdrBtn(hx, "\xE2\x88\x92", false, HA_ZOUT) + 3;  // −
            hx += hdrBtn(hx, "+", false, HA_ZIN) + 5;
        }
        {   // STANDBY — isolado: vão largo + régua vertical antes dele.
            // Aciona o `mute` do MASTER (silêncio limpo, com rampa) — NÃO
            // rompe cabos; o rompe-tudo continua só na tecla [espaço].
            const std::string L = tr(S::hdrStandby, uiLang);
            if (hx + 22 + textW(L) + 14 < rx - 8) {
                hx += 16;
                XSetForeground(dpy, gc, T.line);
                XDrawLine(dpy, bb, gc, hx, hbY + 2, hx, hbY + hbH - 2);
                hx += 12;
                hdrBtn(hx, L, masterMuted, HA_STANDBY);
            }
        }

        XSetForeground(dpy, gc, T.line);
        XDrawLine(dpy, bb, gc, 0, kCaseTop - 4, winW, kCaseTop - 4);

        // ---- paleta (coluna esquerda) --------------------------------
        // com o modo Learn ligado, a caixa LEARN ocupa o rodapé da coluna
        // -> a lista de módulos encurta (não sobrepõe).
        const int palBottom = winH - kLearnH;
        XSetForeground(dpy, gc, T.recessed);
        XFillRectangle(dpy, bb, gc, 0, kCaseTop, kPaletteW, palBottom - kCaseTop);
        XSetForeground(dpy, gc, T.line);
        XDrawLine(dpy, bb, gc, kPaletteW, kCaseTop, kPaletteW, winH);
        clipTo(0, kCaseTop, kPaletteW - 2, palBottom - kCaseTop);
        text(10, kCaseTop + 2 - paletteScroll + 12, "MÓDULOS", T.textSecondary);
        // passar o mouse num tipo da paleta destaca onde ele está na case
        // (mesma faixa de acerto do clique-pra-adicionar, logo abaixo) —
        // "facilitar identificar onde está o módulo no painel" (feedback
        // do autor, 2026-09-05).
        std::string paletteHoverType;
        if (mouseX < kPaletteW && mouseY < palBottom) {
            for (const auto& pr : palette) {
                if (pr.header) continue;
                const int y = pr.y - paletteScroll + 20;
                if (mouseY >= y - 12 && mouseY <= y + 3) { paletteHoverType = pr.type; break; }
            }
        }
        for (const auto& pr : palette) {
            const int y = pr.y - paletteScroll + 20;
            if (y < kCaseTop + 8 || y > palBottom) continue;
            if (!pr.header && pr.type == paletteHoverType) {
                XSetForeground(dpy, gc, T.recessed);
                XFillRectangle(dpy, bb, gc, 0, y - 12, kPaletteW - 2, 16);
            }
            text(pr.header ? 8 : 12, y,
                 pr.header ? pr.label : ("· " + pr.type),
                 pr.header ? T.accent
                 : (pr.type == paletteHoverType ? T.accent : T.textPrimary));
        }
        if (mdrag.active && mouseX < kPaletteW) {   // soltar aqui = remover
            XSetForeground(dpy, gc, T.warning);
            XDrawRectangle(dpy, bb, gc, 2, kCaseTop + 2, kPaletteW - 4,
                           palBottom - kCaseTop - 4);
            text(10, (kCaseTop + palBottom) / 2, "soltar = remover", T.warning);
        }
        clipOff();

        // ---- barra de scroll discreta (só quando a lista transborda) --
        {
            int trkY, trkH, thY, thH;
            if (palBar(trkY, trkH, thY, thH)) {
                const int trkX = kPaletteW - 4;
                const bool over = mouseX >= trkX - 3 && mouseX <= kPaletteW
                    && mouseY >= kCaseTop && mouseY <= palBottom;
                XSetForeground(dpy, gc, T.line);
                XFillRectangle(dpy, bb, gc, trkX, trkY, 2, trkH);
                XSetForeground(dpy, gc, (over || palBarDrag.active)
                                            ? T.accent : T.textSecondary);
                XFillRectangle(dpy, bb, gc, trkX - 1, thY, 4, thH);
            }
        }

        // ---- caixa LEARN (rodapé da coluna esquerda, estilo terminal) --
        {
            const int lx = 3, ly = winH - kLearnH + 2;
            const int lw = kPaletteW - 6, lh = kLearnH - 5;
            XSetForeground(dpy, gc, T.bg);
            XFillRectangle(dpy, bb, gc, lx, ly, lw, lh);
            XSetForeground(dpy, gc, learnHit ? T.accent : T.line);
            XDrawRectangle(dpy, bb, gc, lx, ly, lw, lh);
            clipTo(lx + 1, ly + 1, lw - 2, lh - 2);
            const int tx = lx + 8;
            int ty = ly + 16;
            text(tx, ty, "LEARN", T.textSecondary);
            ty += 6;
            XSetForeground(dpy, gc, T.line);
            XDrawLine(dpy, bb, gc, tx, ty, lx + lw - 8, ty);
            ty += 15;
            const int wrapPx = lw - 16;
            if (!learnHit) {
                for (const auto& ln : wrapText(
                         "passe o mouse sobre um knob ou jack\n"
                         "de um módulo do rack.", wrapPx)) {
                    text(tx, ty, ln, T.textSecondary);
                    ty += 14;
                }
            } else {
                text(tx, ty, learnHitTitle, T.accent);
                ty += 17;
                struct Seg { const std::string& s; unsigned long c; const char* pre; };
                const Seg segs[] = {
                    {learnHit->quick, T.textPrimary, ""},
                    {learnHit->understand, T.textSecondary, ""},
                    {learnHit->explore, T.accent, "\xE2\x86\x92 "},
                };
                for (const auto& seg : segs) {
                    if (seg.s.empty()) continue;
                    for (const auto& ln :
                         wrapText(std::string(seg.pre) + seg.s, wrapPx)) {
                        if (ty > ly + lh - 6) break;
                        text(tx, ty, ln, seg.c);
                        ty += 14;
                    }
                    ty += 7;
                }
            }
            clipOff();
        }

        // ---- case: os módulos (cada um recortado à sua caixa) --------
        clipTo(kPaletteW + 1, kCaseTop, winW - kPaletteW, winH - kCaseTop);
        // frase de crédito da família, na faixa vazia acima da 1ª fileira
        // (pedido do autor 2026-09-07 — os módulos ficam onde estão) —
        // com ano (na frase) + versão (o carimbo de build)
        capText(kPaletteW + (winW - kPaletteW) / 2, kCaseTop + 11,
                tr(S::footerCredit, uiLang)
                    + std::string(RASGO_MODULAR_BUILD),
                T.textSecondary);
        for (const auto& m : mods) {
            const auto [bx, by] = modOrigin(m);
            if (by + modH < kCaseTop || by > winH) continue;
            Signal& node = graph.node(m.id);
            const Panel pn = node.panel();
            const std::string mtype = node.type();
            const bool dragged = mdrag.active && mdrag.id == m.id;
            const bool paletteHit = !paletteHoverType.empty() && mtype == paletteHoverType;
            XSetForeground(dpy, gc, T.surface);
            XFillRectangle(dpy, bb, gc, bx, by, m.w, modH);
            XSetForeground(dpy, gc, (dragged || paletteHit) ? T.accent : T.line);
            XDrawRectangle(dpy, bb, gc, bx, by, m.w, modH);
            if (dragged || paletteHit)
                XDrawRectangle(dpy, bb, gc, bx - 1, by - 1, m.w + 2, modH + 2);
            XSetForeground(dpy, gc, T.line);
            XDrawLine(dpy, bb, gc, bx + 2, by + 3, bx + m.w - 2, by + 3);
            XDrawLine(dpy, bb, gc, bx + 2, by + modH - 3, bx + m.w - 2,
                      by + modH - 3);
            // [x] de remover, no canto superior direito
            XSetForeground(dpy, gc, T.textSecondary);
            const int xx = bx + m.w - 12, xy = by + 5;
            XDrawLine(dpy, bb, gc, xx, xy, xx + 6, xy + 6);
            XDrawLine(dpy, bb, gc, xx + 6, xy, xx, xy + 6);

            // recorta o conteúdo à área ÚTIL do módulo (dentro da borda)
            clipTo(bx + 1, std::max(kCaseTop, by + 1), m.w - 2, modH - 2);
            const int kn = mmpx(9.0f);
            const int jr = mmpx(2.6f);
            const int tg = mmpx(4.4f);
            for (const auto& w : pn.widgets) {
                const int wx = bx + mmpx(w.x), wy = by + mmpx(w.y);
                // MATRIX: as 16 células `g<jk>` viram uma grade clicável,
                // desenhada depois do laço — pula os knobs aqui
                if (mtype == "MATRIX" && w.kind == Widget::Kind::Knob
                    && w.bind.size() == 3 && w.bind[0] == 'g')
                    continue;
                switch (w.kind) {
                case Widget::Kind::Label:
                    text(wx, wy + mmpx(3.6f), w.label, T.textPrimary);
                    break;
                case Widget::Kind::Display: {
                    int dw = mmpx(w.span > 1.0f ? w.span : 16.0f);
                    dw = std::min(dw, m.w - mmpx(w.x) - mmpx(2.0f));
                    // altura 16 mm (era 13) — osciloscópios/espectros com
                    // mais respiro (pedido do autor 2026-09-06)
                    const int dh = mmpx(16.0f);
                    XSetForeground(dpy, gc, T.recessed);
                    XFillRectangle(dpy, bb, gc, wx, wy, dw, dh);
                    XSetForeground(dpy, gc, T.line);
                    XDrawRectangle(dpy, bb, gc, wx, wy, dw, dh);
                    const auto si = scopeSnap.find(m.id);
                    const int view = mtype == "SCOPE" ? scopeView[m.id] : 0;
                    const char* dlabel = w.label.c_str();
                    char dbTxt[16] = {};  // vida até o text() no fim do case
                    if (si != scopeSnap.end() && dw > 6) {
                        const ScopeTrace& sc = si->second;
                        if (mtype == "MASTER") {
                            // VU com clip-latch — "o medidor esconde
                            // estouros" (achado §3.3 da auditoria de saída,
                            // NAVALHA 2). Barra = pico da janela recente
                            // relativo ao teto (−1 dBFS = 0,891); o
                            // indicador vermelho acende quando o
                            // LIMITADOR de verdade teve que segurar algo
                            // (`gainReductionDb()`, telemetria pública já
                            // existente — não é só o pico bruto passar de
                            // um número) e decai depois de ~2 s sem novo
                            // estouro.
                            dlabel = "vu";
                            float peak = 0.0f;
                            for (const float v : sc.buf)
                                peak = std::max(peak, std::fabs(v));
                            const float frac = std::min(1.0f, peak / 0.891f);
                            const int barW = static_cast<int>(frac * (dw - 2));
                            XSetForeground(dpy, gc, frac < 0.7f ? T.accent : T.warning);
                            if (barW > 0)
                                XFillRectangle(dpy, bb, gc, wx + 1, wy + 1,
                                              barW, dh - 2);
                            if (auto* mst = dynamic_cast<Master*>(&node))
                                if (mst->gainReductionDb() > 0.05f)
                                    clipLastSeen[m.id] = std::chrono::steady_clock::now();
                            const auto itClip = clipLastSeen.find(m.id);
                            const bool clipping = itClip != clipLastSeen.end()
                                && std::chrono::steady_clock::now() - itClip->second
                                   < std::chrono::seconds(2);
                            if (clipping) {
                                XSetForeground(dpy, gc, T.warning);
                                XFillRectangle(dpy, bb, gc, wx + dw - 7, wy + 1,
                                              6, dh - 2);
                            }
                            std::snprintf(dbTxt, sizeof dbTxt, "%.0fdB",
                                         20.0f * std::log10(std::max(peak, 1.0e-4f)));
                            dlabel = dbTxt;
                        } else if (mtype == "TRIGSEQ") {
                            // 4 lanes de gate (t1-t4) rolando — piano-roll
                            dlabel = "t1-t4";
                            const int nL = static_cast<int>(kLaneLen);
                            const int lh = std::max(2, (dh - 2) / 4);
                            const int cellw = std::max(1, (dw - 2) / nL);
                            for (int L = 0; L < 4; ++L) {
                                const int ly = wy + 1 + L * lh;
                                XSetForeground(dpy, gc, T.line);
                                XDrawLine(dpy, bb, gc, wx + 1, ly + lh - 1,
                                          wx + dw - 2, ly + lh - 1);
                                XSetForeground(dpy, gc, cellPos);
                                const auto& lane = sc.lanes[static_cast<std::size_t>(L)];
                                for (int k = 0; k < nL; ++k) {
                                    const float g = lane[(sc.lw
                                        + static_cast<std::size_t>(k)) % kLaneLen];
                                    if (g < 0.5f) continue;
                                    XFillRectangle(dpy, bb, gc,
                                        wx + 1 + k * (dw - 2) / nL, ly + 1,
                                        cellw, lh - 2);
                                }
                            }
                        } else if (view == 1) {
                            // espectro: banco Goertzel log de ~24 bandas
                            dlabel = "spec";
                            constexpr int nb = 24;
                            const int n = static_cast<int>(sc.buf.size());
                            float mag[nb];
                            float mmax = 1e-6f;
                            for (int b = 0; b < nb; ++b) {
                                const float f = 60.0f * std::pow(
                                    200.0f, static_cast<float>(b) / (nb - 1));
                                const float wn = 6.2831853f * f / 24000.0f;
                                const float cr = std::cos(wn);
                                float s1 = 0.0f, s2 = 0.0f;
                                for (int k = 0; k < n; ++k) {
                                    const float x = sc.buf[(sc.w
                                        + static_cast<std::size_t>(k))
                                        % sc.buf.size()];
                                    const float s0 = x + 2.0f * cr * s1 - s2;
                                    s2 = s1; s1 = s0;
                                }
                                mag[b] = std::sqrt(std::fabs(
                                    s1 * s1 + s2 * s2 - 2.0f * cr * s1 * s2))
                                    / static_cast<float>(n);
                                mmax = std::max(mmax, mag[b]);
                            }
                            const int bw2 = std::max(1, (dw - 2) / nb);
                            XSetForeground(dpy, gc, T.accent);
                            for (int b = 0; b < nb; ++b) {
                                const float db = 20.0f * std::log10(
                                    mag[b] / mmax + 1e-6f);
                                const float t = std::max(0.0f,
                                    std::min(1.0f, 1.0f + db / 54.0f));
                                const int hh = static_cast<int>(t * (dh - 3));
                                if (hh > 0)
                                    XFillRectangle(dpy, bb, gc,
                                        wx + 1 + b * bw2, wy + dh - 1 - hh,
                                        std::max(1, bw2 - 1), hh);
                            }
                        } else {
                            // onda (padrão) — osciloscópio da saída 0
                            float peak = 1e-4f;
                            for (const float v : sc.buf)
                                peak = std::max(peak, std::fabs(v));
                            const float norm = 0.92f / peak;
                            const int n = static_cast<int>(sc.buf.size());
                            std::vector<XPoint> pts(static_cast<std::size_t>(n));
                            for (int k = 0; k < n; ++k) {
                                const float v = sc.buf[(sc.w
                                    + static_cast<std::size_t>(k))
                                    % sc.buf.size()] * norm;
                                pts[static_cast<std::size_t>(k)].x =
                                    static_cast<short>(wx + 1
                                        + k * (dw - 2) / std::max(1, n - 1));
                                pts[static_cast<std::size_t>(k)].y =
                                    static_cast<short>(wy + dh / 2
                                        - static_cast<int>(v * (dh / 2 - 1)));
                            }
                            XSetForeground(dpy, gc, T.line);
                            XDrawLine(dpy, bb, gc, wx + 1, wy + dh / 2,
                                      wx + dw - 1, wy + dh / 2);
                            XSetForeground(dpy, gc, T.accent);
                            XDrawLines(dpy, bb, gc, pts.data(), n,
                                       CoordModeOrigin);
                        }
                    }
                    text(wx + 3, wy + dh - 3, dlabel, T.textSecondary);
                    break;
                }
                case Widget::Kind::Jack: {
                    const bool isOut = w.bind.rfind("out:", 0) == 0;
                    // afordância de cabeamento: ao arrastar um cabo, os
                    // destinos VÁLIDOS (polaridade oposta) ganham um halo;
                    // os inadequados ficam apagados
                    int state = 0;  // 0 normal · 1 válido-forte · 2 válido · 3 apagado
                    if (cdrag.active) {
                        if (isOut != cdrag.fromOutput) {
                            // resolve o PortKind deste jack
                            const std::string pn2 = w.bind.substr(isOut ? 4 : 3);
                            PortKind k = PortKind::Audio;
                            if (isOut) {
                                for (std::size_t i = 0; i < node.outputCount(); ++i)
                                    if (node.outputDescriptor(i).name == pn2)
                                        k = node.outputDescriptor(i).kind;
                            } else {
                                for (std::size_t i = 0; i < node.inputCount(); ++i)
                                    if (node.inputDescriptor(i).name == pn2)
                                        k = node.inputDescriptor(i).kind;
                            }
                            state = (k == cdrag.kind) ? 1 : 2;
                        } else {
                            state = 3;
                        }
                    }
                    if (state == 1 || state == 2) {
                        XSetForeground(dpy, gc, T.accent);
                        const int hr = jr + mmpx(state == 1 ? 2.4f : 1.6f);
                        XDrawArc(dpy, bb, gc, wx - hr, wy - hr, 2 * hr, 2 * hr,
                                 0, 360 * 64);
                        if (state == 1)
                            XDrawArc(dpy, bb, gc, wx - hr - 2, wy - hr - 2,
                                     2 * hr + 4, 2 * hr + 4, 0, 360 * 64);
                    }
                    const unsigned long ring = state == 3 ? T.recessed
                        : (state ? T.accent : T.line);
                    XSetForeground(dpy, gc, ring);
                    XDrawArc(dpy, bb, gc, wx - jr, wy - jr, 2 * jr, 2 * jr, 0,
                             360 * 64);
                    XSetForeground(dpy, gc, T.recessed);
                    XFillArc(dpy, bb, gc, wx - jr / 2, wy - jr / 2, jr, jr, 0,
                             360 * 64);
                    // rótulo ACIMA do jack — o cabo (que sai por baixo, com
                    // barriga) esconderia um rótulo desenhado embaixo
                    if (state != 3)
                        capText(wx, wy - jr - mmpx(1.8f), w.label,
                                state ? T.textPrimary : T.textSecondary);
                    break;
                }
                case Widget::Kind::Toggle: {
                    const float v = paramVal(node, w.bind);
                    XSetForeground(dpy, gc, v >= 0.5f ? T.accent : T.line);
                    if (v >= 0.5f) XFillRectangle(dpy, bb, gc, wx, wy, tg, tg);
                    else XDrawRectangle(dpy, bb, gc, wx, wy, tg, tg);
                    // rótulo EMBAIXO da caixa, centrado (não à direita — invadia
                    // o widget vizinho)
                    capText(wx + tg / 2, wy + tg + mmpx(3.8f), w.label,
                            T.textPrimary);
                    break;
                }
                case Widget::Kind::Slider:
                case Widget::Kind::Knob: {
                    const ParameterDescriptor* d = paramDesc(node, w.bind);
                    const float lo = d ? d->minimum : 0.0f;
                    const float hi = d ? d->maximum : 1.0f;
                    const float n = hi > lo
                        ? (paramVal(node, w.bind) - lo) / (hi - lo) : 0.0f;
                    if (w.kind == Widget::Kind::Knob) {
                        const int r = kn / 2;
                        const int cx = wx + r, cy = wy + r;
                        XSetForeground(dpy, gc, T.recessed);
                        XFillArc(dpy, bb, gc, wx, wy, kn, kn, 0, 360 * 64);
                        XSetForeground(dpy, gc, T.line);
                        XDrawArc(dpy, bb, gc, wx, wy, kn, kn, 0, 360 * 64);
                        const float a = (0.75f - 1.5f * n) * 3.14159265f + 1.5708f;
                        XSetForeground(dpy, gc, T.accent);
                        XDrawLine(dpy, bb, gc, cx, cy,
                                  cx + static_cast<int>(std::cos(a) * r),
                                  cy - static_cast<int>(std::sin(a) * r));
                        // rótulo ACIMA do knob, centrado (igual aos jacks) —
                        // não empurra pra a fileira de baixo
                        capText(wx + kn / 2, wy - mmpx(1.6f), w.label,
                                T.textSecondary);
                    } else {
                        const int h = mmpx(32.0f), sw = mmpx(8.0f);
                        XSetForeground(dpy, gc, T.line);
                        XDrawLine(dpy, bb, gc, wx + sw / 2, wy, wx + sw / 2,
                                  wy + h);
                        const int hy = wy + h - static_cast<int>(n * h);
                        XSetForeground(dpy, gc, T.accent);
                        XFillRectangle(dpy, bb, gc, wx, hy - 2, sw, 4);
                        capText(wx + sw / 2, wy + h + mmpx(4.0f), w.label,
                                T.textSecondary);
                    }
                    break;
                }
                }
            }

            // ---- MATRIX: grade 4×4 de ganhos, clicável -------------------
            if (mtype == "MATRIX") {
                capText(bx + mmpx(50.0f), by + mmpx(20.0f),
                        "IN \xE2\x86\x93   OUT \xE2\x86\x92", T.textSecondary);
                for (int j = 0; j < 4; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        const RectMM rm = matrixCellMM(j, k);
                        const int cx = bx + mmpx(rm.x), cy = by + mmpx(rm.y);
                        const int cw = mmpx(rm.w), chh = mmpx(rm.h);
                        char id[4] = {'g', static_cast<char>('1' + j),
                                      static_cast<char>('1' + k), 0};
                        const float g = paramVal(node, id);
                        XSetForeground(dpy, gc, T.recessed);
                        XFillRectangle(dpy, bb, gc, cx, cy, cw, chh);
                        // barra do valor a partir do centro (cima = +, baixo = −)
                        const int mid = cy + chh / 2;
                        const int bar = static_cast<int>(
                            (g / 1.0f) * static_cast<float>(chh / 2 - 2));
                        XSetForeground(dpy, gc, g >= 0.0f ? cellPos : cellNeg);
                        if (bar != 0)
                            XFillRectangle(dpy, bb, gc, cx + 2,
                                           bar > 0 ? mid - bar : mid,
                                           cw - 4, bar > 0 ? bar : -bar);
                        XSetForeground(dpy, gc, T.line);
                        XDrawLine(dpy, bb, gc, cx + 1, mid, cx + cw - 2, mid);
                        const bool hot = drag.active && drag.node == m.id
                            && drag.bind.size() == 3 && drag.bind[1] == id[1]
                            && drag.bind[2] == id[2];
                        XSetForeground(dpy, gc, hot ? T.accent : T.line);
                        XDrawRectangle(dpy, bb, gc, cx, cy, cw, chh);
                    }
                }
            }

            clipTo(kPaletteW + 1, kCaseTop, winW - kPaletteW, winH - kCaseTop);
        }
        clipOff();

        // ---- cabos por cima dos módulos (recortados à case) ----------
        clipTo(kPaletteW + 1, kCaseTop, winW - kPaletteW, winH - kCaseTop);
        auto findJack = [&](std::size_t n, int p, bool out) -> const JackScreen* {
            for (const auto& j : jacks)
                if (j.node == n && j.port == p && j.isOut == out) return &j;
            return nullptr;
        };
        for (std::size_t i = 0; i < graph.cableCount(); ++i) {
            const Cable& c = graph.cable(i);
            const JackScreen* s = findJack(c.source().node,
                                           static_cast<int>(c.source().port), true);
            const JackScreen* t = findJack(c.target().node,
                                           static_cast<int>(c.target().port), false);
            if (!s || !t) continue;  // uma das pontas é o sink (não exibido)
            const bool cut = c.state() == CableState::Ruptured;
            // hash determinístico do cabo -> uma cor do "saco de cabos";
            // paleta quente pra áudio, fria pra controle
            const std::size_t h = c.source().node * 7 + c.source().port * 3
                + c.target().node * 5 + c.target().port;
            const unsigned long* pal = s->kind == PortKind::Control
                ? cableCtrl : cableAudio;
            drawCable(s->x, s->y, t->x, t->y,
                      cut ? T.warning : pal[h & 3], cut);
        }
        // cabo elástico sendo puxado — na cor da paleta da ponta ancorada
        if (cdrag.active) {
            const unsigned long* pal = cdrag.kind == PortKind::Control
                ? cableCtrl : cableAudio;
            drawCable(cdrag.ax, cdrag.ay, mouseX, mouseY, pal[0], true);
        }
        clipOff();

        // (o hover-learn agora vai na caixa LEARN fixa do rodapé da
        // coluna esquerda — desenhada junto da paleta, acima — pra não
        // flutuar sobre o painel; `learnHit`/`learnHitTitle` no topo do
        // `redraw`. `dossies/ESTUDO_seed_composicao_generativa.md §6`.)

        // fantasma do módulo sendo arrastado do catálogo
        if (!spawnType.empty()) {
            XSetForeground(dpy, gc, T.accent);
            text(spawnX + 8, spawnY, "+ " + spawnType, T.accent);
        }

        // ==== OVERLAY: tutorial / sobre ================================
        // qualquer clique (ou [Esc]) fecha — ver o laço de evento.
        if (overlay) {
            // véu escuro semi-transparente — os módulos continuam à vista
            XSetForeground(dpy, gc, T.bg);
            XSetStipple(dpy, gc, dimStipple);
            XSetFillStyle(dpy, gc, FillStippled);
            XFillRectangle(dpy, bb, gc, 0, 0, winW, winH);
            XSetFillStyle(dpy, gc, FillSolid);
            const int cw = std::min(760, winW - 60);
            const int chh = std::min(winH - 60, overlay == 1 ? 600 : 285);
            const int cxx = (winW - cw) / 2, cyy = (winH - chh) / 2;
            XSetForeground(dpy, gc, T.surface);
            XFillRectangle(dpy, bb, gc, cxx, cyy, cw, chh);
            XSetForeground(dpy, gc, T.accent);
            XDrawRectangle(dpy, bb, gc, cxx, cyy, cw, chh);
            clipTo(cxx + 1, cyy + 1, cw - 2, chh - 2);
            const int px = cxx + 24, wrapPx = cw - 48;
            int py = cyy + 30;
            {   // CLOSE (canto sup. direito do card) — decorativo: o clique
                // fecha em qualquer lugar
                const std::string cl = tr(S::close, uiLang);
                const int w = textW(cl) + 14, bxc = cxx + cw - w - 12,
                          byc = cyy + 10;
                XSetForeground(dpy, gc, T.line);
                XDrawRectangle(dpy, bb, gc, bxc, byc, w, 20);
                text(bxc + 7, byc + 14, cl, T.textSecondary);
            }
            if (overlay == 1) {
                text(px, py, tr(S::tutTitle, uiLang), T.accent);
                py += 18;
                text(px, py, tr(S::tutSubtitle, uiLang), T.textSecondary);
                py += 26;
                const rasgo::panel::L4* cards[][2] = {
                    {&S::tutCableTitle, &S::tutCableBody},
                    {&S::tutSeedTitle,  &S::tutSeedBody},
                    {&S::tutVaryTitle,  &S::tutVaryBody},
                    {&S::tutMoveTitle,  &S::tutMoveBody},
                    {&S::tutZoomTitle,  &S::tutZoomBody},
                    {&S::tutLearnTitle, &S::tutLearnBody},
                };
                for (auto& c : cards) {
                    text(px, py, tr(*c[0], uiLang), T.textPrimary);
                    py += 16;
                    for (const auto& ln : wrapText(tr(*c[1], uiLang), wrapPx)) {
                        text(px, py, ln, T.textSecondary);
                        py += 14;
                    }
                    py += 10;
                }
            } else {
                text(px, py, "RASGO MODULAR", T.accent);
                py += 16;
                text(px, py,
                     std::string("build ") + RASGO_MODULAR_BUILD
                         + "  ·  compilado " __DATE__ " " __TIME__,
                     T.textSecondary);
                py += 20;
                for (const auto& ln : wrapText(tr(S::aboutBody, uiLang), wrapPx)) {
                    text(px, py, ln, T.textSecondary);
                    py += 15;
                }
            }
            clipOff();
        }

        XCopyArea(dpy, bb, win, gc, 0, 0,
                  static_cast<unsigned>(std::max(1, winW)),
                  static_cast<unsigned>(std::max(1, winH)), 0, 0);
        XFlush(dpy);
    };

    bool alive = true;
    const float sr = static_cast<float>(alsa.rate());
    // liga src->dst ao vivo; em ciclo, tenta de novo como feedback; guarda
    // o grafo consistente sempre. Deve rodar sob `gmx`.
    auto tryPatch = [&](std::size_t s, int sp, std::size_t d, int dp) {
        graph.disconnect(d, dp);
        try {
            graph.connect(s, static_cast<std::size_t>(sp), d,
                          static_cast<std::size_t>(dp));
            graph.prepare(sr, 2, block);
        } catch (const std::logic_error&) {          // ciclo
            graph.disconnect(d, dp);
            try {
                graph.connect(s, static_cast<std::size_t>(sp), d,
                              static_cast<std::size_t>(dp), true);
                graph.prepare(sr, 2, block);
            } catch (...) { graph.disconnect(d, dp); graph.prepare(sr, 2, block); }
        } catch (...) { graph.disconnect(d, dp); graph.prepare(sr, 2, block); }
    };

    // tira um módulo da case SEM tocar nos outros cabos: desliga só os
    // cabos que tocam nele; o nó fica órfão no grafo (silencioso) — v1.
    auto removeModule = [&](const std::size_t id) {
        {
            std::lock_guard<std::mutex> lk(gmx);
            for (std::size_t i = graph.cableCount(); i-- > 0;) {
                const auto& c = graph.cable(i);
                if (c.source().node == id || c.target().node == id)
                    graph.disconnect(c.target().node, c.target().port);
            }
            graph.prepare(sr, 2, block);
        }
        shown.erase(std::remove(shown.begin(), shown.end(), id), shown.end());
        buildMods();
        populateMotion();
        syncSignalIn();
        relayout();
    };

    // reordena `shown` pra que `id` caia na posição de leitura do cursor
    // (os cabos acompanham — nada desconecta)
    auto moduleReorderTo = [&](const std::size_t id, const int cx, const int cy) {
        const int rowH = modH + kCasePad;
        const int cursorRow = std::max(0,
            (cy - kCaseTop - kCasePad + scrollY) / std::max(1, rowH));
        int dropIdx = 0;
        for (const auto& m : mods) {
            if (m.id == id) continue;
            const int mrow = m.col >> 20;
            const int mcx = (m.col & 0xFFFFF) + m.w / 2;
            if (mrow < cursorRow || (mrow == cursorRow && mcx < cx)) ++dropIdx;
        }
        std::vector<std::size_t> next;
        next.reserve(shown.size());
        for (const auto s : shown) if (s != id) next.push_back(s);
        if (dropIdx > static_cast<int>(next.size()))
            dropIdx = static_cast<int>(next.size());
        next.insert(next.begin() + dropIdx, id);
        if (next != shown) { shown = next; buildMods(); relayout(); }
    };

    // ---- salvar / carregar o patch ------------------------------------
    // o trabalho do músico fica em ~/.local/share/rasgo-modular/. A sessão
    // é auto-carregada no arranque e auto-salva na saída; [Ctrl+S] salva
    // na hora; as gravações vão pra a mesma pasta.
    auto dataDir = [] {
        const char* xdg = std::getenv("XDG_DATA_HOME");
        const char* home = std::getenv("HOME");
        std::filesystem::path base = (xdg && *xdg)
            ? std::filesystem::path(xdg)
            : std::filesystem::path(home ? home : ".") / ".local" / "share";
        std::filesystem::path d = base / "rasgo-modular";
        std::error_code ec;
        std::filesystem::create_directories(d, ec);
        return d;
    };
    const std::filesystem::path sessionFile = dataDir() / "session.rmp";

    // pref de idioma — arquivo próprio (uma linha, o código), independente
    // do patch: sobrevive a abrir num seed novo, não só no `--resume`.
    const std::filesystem::path langPrefFile = dataDir() / "ui-lang";
    {
        std::ifstream lf(langPrefFile);
        std::string code;
        if (lf && (lf >> code)) uiLang = rasgo::panel::langFromCode(code);
    }
    auto saveLangPref = [&] {
        std::ofstream lf(langPrefFile);
        if (lf) lf << rasgo::panel::langCode(uiLang) << '\n';
    };

    auto panelFactory = [](const std::string& t) -> std::unique_ptr<Signal> {
        if (t == "OUT") return std::make_unique<Out>();
        return rasgo::panel::makeModule(t);
    };
    auto savePatch = [&](const std::filesystem::path& path) {
        std::lock_guard<std::mutex> lk(gmx);
        std::ofstream f(path);
        if (!f) return;
        f << graph.serialize();
        if (curSeed) f << "seed " << curSeed << '\n';
        f << "panel shown";
        for (const auto id : shown) f << ' ' << id;
        f << '\n';
        std::fprintf(stderr, "[patch] salvo em %s\n", path.string().c_str());
    };
    // banco de patches: se o músico gostou de um seed, [Ctrl+B] registra o
    // patch atual num arquivo próprio (não sobrescreve a sessão)
    auto bankPatch = [&] {
        const auto dir = dataDir() / "patches";
        std::error_code ec; std::filesystem::create_directories(dir, ec);
        char name[64];
        if (curSeed)
            std::snprintf(name, sizeof name, "seed-%llu.rmp",
                          static_cast<unsigned long long>(curSeed));
        else
            std::snprintf(name, sizeof name, "patch-%ld.rmp",
                          static_cast<long>(std::time(nullptr)));
        savePatch(dir / name);
        return dir / name;
    };
    auto loadPatch = [&](const std::filesystem::path& path) -> bool {
        std::ifstream f(path);
        if (!f) return false;
        const std::string text((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
        std::vector<std::size_t> ord;
        { std::istringstream is(text); std::string line;
          while (std::getline(is, line))
            if (line.rfind("panel shown", 0) == 0) {
                std::istringstream ls(line); std::string a, b; std::size_t id;
                ls >> a >> b;
                while (ls >> id) ord.push_back(id);
            } }
        try {
            SignalGraph g2 = SignalGraph::deserialize(text, panelFactory);
            std::lock_guard<std::mutex> lk(gmx);
            graph = std::move(g2);
            sink = 0;
            for (std::size_t i = 0; i < graph.nodeCount(); ++i)
                if (graph.node(i).type() == "OUT") sink = i;
            shown.clear();
            if (!ord.empty()) shown = ord;
            else for (std::size_t i = 0; i < graph.nodeCount(); ++i)
                if (graph.node(i).type() != "OUT") shown.push_back(i);
            scopes.clear();
            for (const auto id : shown) scopes[id];
            graph.prepare(sr, 2, block);
            graph.setActiveOutput(sink);  // o move zerou o alvo ativo
            buildMods();
            populateMotion();
            syncSignalIn();
            relayout();
            std::fprintf(stderr, "[patch] carregado de %s\n",
                         path.string().c_str());
            return true;
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[patch] falhou ao carregar: %s\n", e.what());
            return false;
        }
    };
    int recCount = 0;
    auto stopRec = [&] {
        if (recBuf.empty()) { recording.store(false); return; }
        recording.store(false);
        std::vector<float> copy;
        { std::lock_guard<std::mutex> lk(gmx); copy.swap(recBuf); }
        char name[64];
        std::snprintf(name, sizeof name, "rec-%02d.wav", ++recCount);
        const auto p = dataDir() / name;
        // dither TPDF ligado (seed != 0) -- gravação real do usuário, não
        // um render de auditoria/exemplo (ver `src/io/WavWriter.hpp`)
        rasgo::modular::writeWav16(p.string(), copy,
                                   static_cast<std::uint32_t>(alsa.rate()), 2,
                                   static_cast<std::uint64_t>(recCount));
        char scoreName[64];
        std::snprintf(scoreName, sizeof scoreName, "rec-%02d.score.txt", recCount);
        std::ofstream sf(dataDir() / scoreName);
        std::string scoreText;
        { std::lock_guard<std::mutex> lk(gmx); scoreText = score.toText(); }
        sf << scoreText;
        std::fprintf(stderr, "[rec] %.1f s -> %s (+ %s)\n",
                     static_cast<double>(copy.size()) / 2.0 / alsa.rate(),
                     p.string().c_str(), scoreName);
        { std::lock_guard<std::mutex> lk(gmx); recBuf.reserve(copy.capacity()); }
    };

    // ---- ponto de partida por SEED (como o seed do RASGO Synth Studio, mas
    // de CABEAMENTO): limpa os cabos, monta um patch generativo coerente e
    // já toca. `[g]` avança pro próximo seed; o botão "SEED" na faixa de
    // status faz o mesmo. Mesmo número -> mesma música (tudo determinístico).
    // próximo seed ALEATÓRIO (o espaço é ~10^19; `[g]` sorteia de verdade,
    // não incrementa). `RASGO_SEED=N` / `--seed N` reproduzem um específico.
    auto nextRandomSeed = [&]() -> std::uint64_t {
        std::uint64_t x = static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        x ^= 0x9E3779B97F4A7C15ULL * (seedNum + 1);
        x ^= x << 13; x ^= x >> 7; x ^= x << 17;
        return (x % 999999999ULL) + 1;
    };
    auto applySeed = [&](const std::uint64_t s) {
        curSeed = s;
        {
            std::lock_guard<std::mutex> lk(gmx);
            try {
                rasgo::panel::seedPatch(graph, s);
                graph.prepare(sr, 2, block);
            } catch (const std::exception& e) {
                std::fprintf(stderr, "[seed %llu] %s — voltando ao seed 1\n",
                             static_cast<unsigned long long>(s), e.what());
                rasgo::panel::seedPatch(graph, 1);
                graph.prepare(sr, 2, block);
            }
            graph.setActiveOutput(sink);
        }
        allRuptured = false;
        buildMods();
        populateMotion();
        syncSignalIn();
        relayout();
        char t[64];
        std::snprintf(t, sizeof t, "RASGO Modular — seed %llu",
                      static_cast<unsigned long long>(s));
        setTitle(t);
        redraw();
    };

    // ---- ações do cabeçalho -------------------------------------------
    // Cada comando do cabeçalho tem UMA implementação, chamada tanto pela
    // tecla quanto pelo clique no botão da barra. (Antes viviam soltas no
    // switch de KeyPress.)
    auto actSeed = [&] { seedNum = nextRandomSeed(); applySeed(seedNum); };

    auto actMotion = [&] {
        motionOn = !motionOn;
        setTitle(motionOn ? "RASGO Modular — variação ao vivo: ligada"
                          : "RASGO Modular — variação ao vivo: pausada");
        redraw();
    };

    // [espaço] — rompe/reata TODOS os cabos de uma vez (gesto grande)
    auto actRupture = [&] {
        std::lock_guard<std::mutex> lk(gmx);
        allRuptured = !allRuptured;
        for (std::size_t i = 0; i < graph.cableCount(); ++i) {
            if (allRuptured) graph.cable(i).rupture();
            else graph.cable(i).reconnect();
        }
        redraw();
    };

    // STANDBY (botão do cabeçalho) — liga/desliga o `mute` de todo MASTER:
    // silêncio limpo com rampa, o patch continua rodando por baixo. Mesmo
    // estado do toggle MUTE no painel do módulo.
    auto actStandby = [&] {
        bool anyOn = false;
        for (std::size_t i = 0; i < graph.nodeCount(); ++i)
            if (graph.node(i).type() == "MASTER"
                && graph.parameterUserValue(i, "mute") >= 0.5f) anyOn = true;
        const float v = anyOn ? 0.0f : 1.0f;
        for (std::size_t i = 0; i < graph.nodeCount(); ++i)
            if (graph.node(i).type() == "MASTER")
                graph.setParameterBase(i, "mute", v);
        setTitle(v >= 0.5f ? "RASGO Modular — STANDBY (saída em silêncio)"
                           : "RASGO Modular — saída ativa");
        redraw();
    };

    auto freezeMixMaster = [&](std::unordered_set<std::size_t>& frozen) {
        for (std::size_t i = 0; i < graph.nodeCount(); ++i)
            if (graph.node(i).type() == "MIXER"
                || graph.node(i).type() == "MASTER")
                frozen.insert(i);
    };

    auto actMutate = [&] {
        // reamostra ~25% dos parâmetros não-estruturais dos nós alcançados
        // por cabo, AGORA, uma vez. FREEZE automático em MIXER/MASTER
        // (mitigação documentada em `PatchGenetics.hpp`).
        std::unordered_set<std::size_t> frozen;
        {
            std::lock_guard<std::mutex> lk(gmx);
            freezeMixMaster(frozen);
            rasgo::panel::mutatePatch(graph, nextRandomSeed(), 0.25f, frozen);
        }
        populateMotion();  // alvos do Motion Engine reiniciam coerentes
        setTitle("RASGO Modular — MUTATE");
        redraw();
    };

    auto actEvolve = [&] {
        // o mesmo destino de um MUTATE grande, em 6 passos pequenos (12%
        // cada) — transição mais gradual que um MUTATE único de 25%.
        std::unordered_set<std::size_t> frozen;
        {
            std::lock_guard<std::mutex> lk(gmx);
            freezeMixMaster(frozen);
            rasgo::panel::evolvePatch(graph, nextRandomSeed(), 6, 0.12f, frozen);
        }
        populateMotion();
        setTitle("RASGO Modular — EVOLVE (6 passos)");
        redraw();
    };

    auto actCross = [&] {
        // recombina o patch atual com um DOADOR novo: o catálogo inteiro
        // semeado com um seed fresco (igual a um patch de verdade), fração
        // 0,5. Mesmo FREEZE de MIXER/MASTER. O doador é um grafo solto —
        // nunca processa áudio, só é lido por `crossPatch`.
        SignalGraph donor;
        for (const auto& grp : rasgo::panel::moduleCatalog())
            for (const char* t : grp.types)
                donor.add(rasgo::panel::makeModule(t));
        rasgo::panel::seedPatch(donor, nextRandomSeed());
        std::unordered_set<std::size_t> frozen;
        {
            std::lock_guard<std::mutex> lk(gmx);
            freezeMixMaster(frozen);
            rasgo::panel::crossPatch(graph, donor, nextRandomSeed(), 0.5f, frozen);
        }
        populateMotion();
        setTitle("RASGO Modular — CROSS");
        redraw();
    };

    auto actBank = [&] {
        const auto bp = bankPatch();
        setTitle("RASGO Modular — no banco: " + bp.filename().string());
    };

    auto actSave = [&] {
        savePatch(sessionFile);
        setTitle("RASGO Modular — patch salvo");
    };

    auto actRec = [&] {
        if (recording.load()) { stopRec(); redraw(); return; }
        {
            std::lock_guard<std::mutex> lk(gmx);
            recBuf.clear();
            score.clear();
            for (std::size_t i = 0; i < graph.cableCount(); ++i) {
                const auto& c = graph.cable(i);
                score.connection(0.0, c.source().node, c.source().port,
                                 c.target().node, c.target().port);
            }
        }
        recording.store(true);
        setTitle("RASGO Modular — ● GRAVANDO");
        redraw();
    };

    // dir: -1 reduz · +1 amplia · 0 volta a 100%
    auto actZoom = [&](const int dir) {
        const float prev = g_s;
        if (dir == 0) uiZoom = 1.0f;
        else if (dir < 0) uiZoom = std::max(kZoomMin, uiZoom - kZoomStep);
        else uiZoom = std::min(kZoomMax, uiZoom + kZoomStep);
        relayout();
        if (prev > 0.0f) {  // âncora: mantém o conteúdo no topo
            scrollY = static_cast<int>(std::lround(scrollY * (g_s / prev)));
            relayout();
        }
        setTitle("RASGO Modular — zoom "
            + std::to_string(static_cast<int>(std::lround(uiZoom * 100.0f))) + "%");
        redraw();
    };

    auto cycleLang = [&] {
        uiLang = rasgo::panel::nextLang(uiLang);
        saveLangPref();
        redraw();
    };

    // "sempre quando abro o instrumento está com a mesma configuração"
    // (feedback do autor, 2026-09-04) — abrir o RASGO Modular é abrir um
    // INSTRUMENTO GENERATIVO (`project_rasgo_modular_identity`: soa
    // sozinho, sem entrada, desde o load), não retomar um documento
    // congelado. Por padrão, todo lançamento sorteia um seed novo — timbre,
    // cabeamento e forma diferentes cada vez. `RASGO_SEED=N`/`--seed N`
    // reproduz um específico (som/render determinístico). `RASGO_RESUME=1`/
    // `--resume` é o único caminho que carrega a sessão salva de propósito
    // — pra quem estava no meio de um patch feito à mão e quer voltar
    // exatamente onde parou. `Ctrl+S`/o banco (`Ctrl+B`) continuam sendo
    // como se guarda um patch de propósito, sem mudar aqui.
    if (const char* sv = std::getenv("RASGO_SEED")) {
        seedNum = std::strtoull(sv, nullptr, 10);
        applySeed(seedNum);
    } else if (std::getenv("RASGO_RESUME") && std::filesystem::exists(sessionFile)) {
        loadPatch(sessionFile);
    } else {
        applySeed(nextRandomSeed());
    }

    redraw();  // primeira pintura (não depende só do Expose)
    while (alive) {
        if (g_quit.load()) alive = false;
        while (XPending(dpy)) {
            XEvent ev; XNextEvent(dpy, &ev);
            if (ev.type == MotionNotify) {
                mouseX = ev.xmotion.x; mouseY = ev.xmotion.y;
            }
            if (ev.type == ClientMessage
                && static_cast<Atom>(ev.xclient.data.l[0]) == wmDel) {
                alive = false;
            } else if (ev.type == ConfigureNotify) {
                if (ev.xconfigure.width != winW || ev.xconfigure.height != winH) {
                    winW = ev.xconfigure.width; winH = ev.xconfigure.height;
                    resizeBuf();
                    relayout(); redraw();
                }
            } else if (ev.type == Expose) {
                if (ev.xexpose.count == 0) redraw();
            } else if (ev.type == KeyPress) {
                const KeySym k = XLookupKeysym(&ev.xkey, 0);
                const bool ctrl = (ev.xkey.state & ControlMask) != 0;
                if (k == XK_Escape) {
                    if (overlay) { overlay = 0; redraw(); }
                    else alive = false;
                }
                else if (k == XK_q) alive = false;
                else if (ctrl && (k == XK_plus || k == XK_equal || k == XK_KP_Add))
                    actZoom(+1);
                else if (ctrl && (k == XK_minus || k == XK_KP_Subtract))
                    actZoom(-1);
                else if (ctrl && (k == XK_0 || k == XK_KP_0))
                    actZoom(0);
                else if (ctrl && (k == XK_s || k == XK_S)) actSave();
                else if (ctrl && (k == XK_b || k == XK_B)) actBank();
                else if (ctrl && (k == XK_r || k == XK_R)) actRec();
                else if (k == XK_space) actRupture();
                else if (k == XK_r) reprepare = true;
                else if (k == XK_s) {
                    // "sugestão": adiciona um módulo do catálogo (rotação
                    // determinística). O ponto-de-partida por seed (como o
                    // seed do RASGO Synth) é feature de design registrada -
                    // ver apps/panel/design.md §Sugestão.
                    static const char* seq[] = {"TURING", "MATTER", "STRING",
                                                "SPACE", "PARAMETRIC", "SEQUENCE",
                                                "QUANTIZER", "HARMONY"};
                    static int si = 0;
                    std::lock_guard<std::mutex> lk(gmx);
                    auto nn = rasgo::panel::makeModule(seq[si % 8]);
                    ++si;
                    if (nn) {
                        const std::size_t nid = graph.add(std::move(nn));
                        shown.push_back(nid);
                        scopes[nid];
                        graph.prepare(static_cast<float>(alsa.rate()), 2, block);
                        buildMods(); populateMotion(); syncSignalIn(); relayout();
                    }
                    redraw();
                }
                else if (k == XK_g || k == XK_G) actSeed();
                else if (k == XK_v || k == XK_V) actMotion();
                else if (k == XK_m || k == XK_M) actMutate();
                else if (k == XK_e || k == XK_E) actEvolve();
                else if (k == XK_c || k == XK_C) actCross();
                else if (k == XK_Down) { scrollY += 40; relayout(); redraw(); }
                else if (k == XK_Up) { scrollY = std::max(0, scrollY - 40); redraw(); }
            } else if (ev.type == ButtonPress) {
                const int mx = ev.xbutton.x, my = ev.xbutton.y;

                // ---- overlay (tutorial / sobre): clique fecha ----------
                if (overlay) {
                    if (ev.xbutton.button == 1) { overlay = 0; redraw(); }
                    continue;
                }

                // ---- cabeçalho: botão da barra? -----------------------
                if (ev.xbutton.button == 1 && my < kCaseTop) {
                    HdrAct hit = HA_NONE;
                    for (const auto& h : headerHits)
                        if (mx >= h.x && mx <= h.x + h.w
                            && my >= h.y && my <= h.y + h.h) { hit = h.act; break; }
                    switch (hit) {
                    case HA_SEED:   actSeed(); continue;
                    case HA_REC:    actRec(); continue;
                    case HA_LANG:   cycleLang(); continue;
                    case HA_TUTORIAL: overlay = (overlay == 1 ? 0 : 1); redraw(); continue;
                    case HA_ABOUT:  overlay = (overlay == 2 ? 0 : 2); redraw(); continue;
                    case HA_VARY:   actMotion(); continue;
                    case HA_STANDBY: actStandby(); continue;
                    case HA_MUTATE: hdrFlash[HA_MUTATE] = std::chrono::steady_clock::now(); actMutate(); continue;
                    case HA_EVOLVE: hdrFlash[HA_EVOLVE] = std::chrono::steady_clock::now(); actEvolve(); continue;
                    case HA_CROSS:  hdrFlash[HA_CROSS] = std::chrono::steady_clock::now(); actCross(); continue;
                    case HA_BANK:   hdrFlash[HA_BANK] = std::chrono::steady_clock::now(); actBank(); redraw(); continue;
                    case HA_SAVE:   hdrFlash[HA_SAVE] = std::chrono::steady_clock::now(); actSave(); redraw(); continue;
                    case HA_ZOUT:   actZoom(-1); continue;
                    case HA_ZIN:    actZoom(+1); continue;
                    case HA_NONE:   break;
                    }
                }

                const int palBot = winH - kLearnH;
                if (mx < kPaletteW && my < palBot) {
                    if (ev.xbutton.button == 4) { paletteScroll = std::max(0, paletteScroll - 40); redraw(); continue; }
                    if (ev.xbutton.button == 5) { paletteScroll = std::min(palMaxScroll(), paletteScroll + 40); redraw(); continue; }
                    // barra de scroll: pega o cursor, ou pagina até o clique
                    if (ev.xbutton.button == 1 && mx >= kPaletteW - 8) {
                        int trkY, trkH, thY, thH;
                        if (palBar(trkY, trkH, thY, thH)) {
                            if (my >= thY && my <= thY + thH) {
                                palBarDrag.active = true;
                                palBarDrag.grabOff = my - thY;
                            } else {
                                const int page = std::max(60, trkH - thH);
                                paletteScroll = std::min(palMaxScroll(),
                                    std::max(0, paletteScroll
                                        + (my < thY ? -page : page)));
                            }
                            redraw(); continue;
                        }
                    }
                    for (const auto& pr : palette) {
                        if (pr.header) continue;
                        const int y = pr.y - paletteScroll + 20;
                        if (my >= y - 12 && my <= y + 3) {
                            spawnType = pr.type; spawnX = mx; spawnY = my;
                        }
                    }
                    redraw(); continue;
                }
                if (ev.xbutton.button == 4) { scrollY = std::max(0, scrollY - 48); relayout(); redraw(); continue; }
                if (ev.xbutton.button == 5) { scrollY += 48; relayout(); redraw(); continue; }

                // ---- jack: começa a cabear (ou remove um cabo) ----------
                rebuildJacks();
                const int ji = jackAt(mx, my);
                if (ji >= 0) {
                    const JackScreen j = jacks[static_cast<std::size_t>(ji)];
                    if (ev.xbutton.button == 3) {   // botão direito: desliga
                        std::lock_guard<std::mutex> lk(gmx);
                        if (!j.isOut) {
                            if (graph.disconnect(j.node, j.port))
                                graph.prepare(sr, 2, block);
                        } else {
                            // remove todos os cabos que saem deste jack
                            bool any = false;
                            for (std::size_t i = graph.cableCount(); i-- > 0;) {
                                const auto& c = graph.cable(i);
                                if (c.source().node == j.node
                                    && static_cast<int>(c.source().port) == j.port) {
                                    graph.disconnect(c.target().node,
                                                     c.target().port);
                                    any = true;
                                }
                            }
                            if (any) graph.prepare(sr, 2, block);
                        }
                        redraw(); continue;
                    }
                    // botão esquerdo: puxa um cabo
                    if (!j.isOut) {
                        // se já tem cabo, "pega" a ponta: desliga e ancora
                        // na SAÍDA de origem
                        int fromCable = -1;
                        for (std::size_t i = 0; i < graph.cableCount(); ++i) {
                            const auto& c = graph.cable(i);
                            if (c.target().node == j.node
                                && static_cast<int>(c.target().port) == j.port) {
                                fromCable = static_cast<int>(i);
                                break;
                            }
                        }
                        if (fromCable >= 0) {
                            const auto src = graph.cable(
                                static_cast<std::size_t>(fromCable)).source();
                            { std::lock_guard<std::mutex> lk(gmx);
                              graph.disconnect(j.node, j.port);
                              graph.prepare(sr, 2, block); }
                            rebuildJacks();
                            const JackScreen* sj = nullptr;
                            for (const auto& q : jacks)
                                if (q.node == src.node
                                    && q.port == static_cast<int>(src.port)
                                    && q.isOut) sj = &q;
                            cdrag = {true, src.node, static_cast<int>(src.port),
                                     true,
                                     sj ? sj->kind : PortKind::Audio,
                                     sj ? sj->x : mx, sj ? sj->y : my};
                        } else {
                            cdrag = {true, j.node, j.port, false, j.kind, j.x, j.y};
                        }
                    } else {
                        cdrag = {true, j.node, j.port, true, j.kind, j.x, j.y};
                    }
                    redraw(); continue;
                }

                for (const auto& m : mods) {
                    const auto [bx, by] = modOrigin(m);
                    if (mx < bx || mx > bx + m.w || my < by || my > by + modH)
                        continue;
                    Signal& node = graph.node(m.id);
                    // [x] no canto superior direito -> remove o módulo
                    if (mx >= bx + m.w - 15 && my <= by + 15) {
                        removeModule(m.id);
                        break;
                    }
                    bool hitCtl = false;
                    // MATRIX: grade 4×4 — arrasto vertical numa célula ajusta
                    // o ganho `g<jk>` (mesma via dos knobs: `drag` + motion
                    // aplica `setParameterBase`)
                    if (node.type() == "MATRIX") {
                        for (int j = 0; j < 4 && !hitCtl; ++j)
                            for (int k = 0; k < 4 && !hitCtl; ++k) {
                                const RectMM rm = matrixCellMM(j, k);
                                const int cx = bx + mmpx(rm.x);
                                const int cy = by + mmpx(rm.y);
                                if (mx < cx || mx > cx + mmpx(rm.w)
                                    || my < cy || my > cy + mmpx(rm.h)) continue;
                                char id[4] = {'g', static_cast<char>('1' + j),
                                              static_cast<char>('1' + k), 0};
                                hitCtl = true;
                                drag = {true, m.id, std::string(id),
                                        graph.parameterUserValue(m.id, id),
                                        -1.0f, 1.0f, my};
                            }
                    }
                    for (const auto& w : node.panel().widgets) {
                        if (hitCtl) break;
                        if (w.kind != Widget::Kind::Knob
                            && w.kind != Widget::Kind::Slider
                            && w.kind != Widget::Kind::Toggle) continue;
                        if (node.type() == "MATRIX" && w.kind == Widget::Kind::Knob
                            && w.bind.size() == 3 && w.bind[0] == 'g') continue;
                        Rect fp = footprintPx(w);
                        fp.x += bx; fp.y += by;
                        if (mx < fp.x || mx > fp.x + fp.w
                            || my < fp.y || my > fp.y + fp.h) continue;
                        hitCtl = true;
                        if (w.kind == Widget::Kind::Toggle) {
                            // base do knob (não o valor já modulado)
                            const float v =
                                graph.parameterUserValue(m.id, w.bind);
                            // setParameterBase: se o parâmetro tem modulação,
                            // atualiza a BASE (o knob) — a modulação é aditiva
                            graph.setParameterBase(m.id, w.bind,
                                                   v >= 0.5f ? 0.0f : 1.0f);
                        } else {
                            const ParameterDescriptor* d = paramDesc(node, w.bind);
                            drag = {true, m.id, w.bind,
                                    graph.parameterUserValue(m.id, w.bind),
                                    d ? d->minimum : 0.0f, d ? d->maximum : 1.0f,
                                    my};
                        }
                        break;
                    }
                    // SCOPE: clicar no Display alterna onda <-> espectro
                    if (!hitCtl && node.type() == "SCOPE") {
                        for (const auto& w : node.panel().widgets) {
                            if (w.kind != Widget::Kind::Display) continue;
                            Rect fp = footprintPx(w);
                            fp.x += bx; fp.y += by;
                            if (mx >= fp.x && mx <= fp.x + fp.w
                                && my >= fp.y && my <= fp.y + fp.h) {
                                scopeView[m.id] = (scopeView[m.id] + 1) % 2;
                                hitCtl = true;
                            }
                        }
                    }
                    // corpo do módulo (nem controle nem [x]) -> arrasta pra
                    // reposicionar (os cabos acompanham; nada desconecta)
                    if (!hitCtl) mdrag = {true, m.id};
                    break;
                }
                redraw();
            } else if (ev.type == ButtonRelease) {
                palBarDrag.active = false;
                if (drag.active && recording.load()) {
                    const float toVal = graph.parameterUserValue(drag.node, drag.bind);
                    if (toVal != drag.startVal) {
                        std::lock_guard<std::mutex> lk(gmx);
                        const double t = static_cast<double>(recBuf.size())
                            / 2.0 / alsa.rate();
                        score.parameterChange(t, drag.node, drag.bind,
                                              drag.startVal, toVal);
                    }
                }
                drag.active = false;
                if (cdrag.active) {
                    rebuildJacks();
                    const int ji = jackAt(ev.xbutton.x, ev.xbutton.y);
                    if (ji >= 0) {
                        const JackScreen j = jacks[static_cast<std::size_t>(ji)];
                        if (j.isOut != cdrag.fromOutput) {  // polaridade oposta
                            std::size_t s, d; int sp, dp;
                            if (cdrag.fromOutput) {
                                s = cdrag.node; sp = cdrag.port;
                                d = j.node;     dp = j.port;
                            } else {
                                s = j.node;     sp = j.port;
                                d = cdrag.node; dp = cdrag.port;
                            }
                            std::lock_guard<std::mutex> lk(gmx);
                            tryPatch(s, sp, d, dp);
                        }
                    }
                    cdrag.active = false;
                    redraw();
                    continue;
                }
                if (mdrag.active) {
                    if (ev.xbutton.x < kPaletteW)   // solto na paleta = remove
                        removeModule(mdrag.id);
                    mdrag.active = false;
                    redraw();
                    continue;
                }
                if (!spawnType.empty()) {
                    if (ev.xbutton.x > kPaletteW) {
                        std::lock_guard<std::mutex> lk(gmx);
                        auto nn = rasgo::panel::makeModule(spawnType);
                        if (nn) {
                            const std::size_t id = graph.add(std::move(nn));
                            shown.push_back(id);
                            scopes[id];
                            graph.prepare(static_cast<float>(alsa.rate()), 2, block);
                            buildMods();
                            populateMotion();
                            syncSignalIn();
                            relayout();
                        }
                    }
                    spawnType.clear();
                }
                setTitle("RASGO Modular — painel de teste");
                redraw();
            } else if (ev.type == MotionNotify && palBarDrag.active) {
                mouseX = ev.xmotion.x; mouseY = ev.xmotion.y;
                int trkY, trkH, thY, thH;
                if (palBar(trkY, trkH, thY, thH) && trkH > thH) {
                    const int rel = mouseY - palBarDrag.grabOff - trkY;
                    paletteScroll = std::min(palMaxScroll(), std::max(0,
                        rel * palMaxScroll() / (trkH - thH)));
                }
                redraw();
            } else if (ev.type == MotionNotify && cdrag.active) {
                mouseX = ev.xmotion.x; mouseY = ev.xmotion.y;
                redraw();
            } else if (ev.type == MotionNotify && mdrag.active) {
                mouseX = ev.xmotion.x; mouseY = ev.xmotion.y;
                if (mouseX >= kPaletteW)   // reordena ao vivo (cabos seguem)
                    moduleReorderTo(mdrag.id, mouseX, mouseY);
                redraw();
            } else if (ev.type == MotionNotify && !spawnType.empty()) {
                spawnX = ev.xmotion.x; spawnY = ev.xmotion.y;
                mouseX = spawnX; mouseY = spawnY;
                redraw();
            } else if (ev.type == MotionNotify && drag.active) {
                const float dy =
                    static_cast<float>(drag.startY - ev.xmotion.y) / 220.0f;
                float v;
                if (drag.lo > 0.0f && drag.hi / drag.lo > 30.0f) {
                    // faixa larga tipo frequência: arrasto EXPONENCIAL
                    // (oitavas por pixel) — linear seria inútil
                    const float k = std::log(drag.hi / drag.lo);
                    float t = std::log(drag.startVal / drag.lo) / k + dy;
                    t = std::max(0.0f, std::min(1.0f, t));
                    v = drag.lo * std::exp(k * t);
                } else {
                    v = drag.startVal + dy * (drag.hi - drag.lo);
                }
                v = std::max(drag.lo, std::min(drag.hi, v));
                // setParameterBase: atualiza a BASE do knob; se o parâmetro
                // tem modulação (cabo/qualidade), ela soma-se aditivamente
                graph.setParameterBase(drag.node, drag.bind, v);
                char title[128];
                std::snprintf(title, sizeof title, "%s : %s = %.4g",
                              graph.node(drag.node).type().c_str(),
                              drag.bind.c_str(), v);
                setTitle(title);
                redraw();
            }
        }
        // Motion Engine: a mão caótica move as fibras ([v] liga/desliga).
        // Pausa enquanto a mão do usuário está num controle (`drag.active`).
        // A energia do som (RMS do MASTER) acelera a mão e, perto do teto,
        // puxa tudo pro centro (duck protetor — evita o clip com [v] on).
        if (motionOn && !drag.active) {
            float eRms = 0.0f;
            for (const auto& kv : scopeSnap) {
                if (graph.node(kv.first).type() != "MASTER") continue;
                double sq = 0.0; std::size_t n = 0;
                for (const float v : kv.second.buf) { sq += (double)v * v; ++n; }
                if (n) eRms = std::max(eRms,
                                       (float)std::sqrt(sq / (double)n));
            }
            const float energy = std::min(1.0f, eRms * 2.2f);
            motion.tick(graph, 0.033f, energy);
        }
        // repinta a ~30 fps pra os osciloscópios dos módulos animarem — o
        // buffer fora da tela mantém sem flicker; o áudio é outro thread.
        redraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // tira a janela da tela JÁ — antes de qualquer desmonte de áudio. Se o
    // ALSA/PipeWire estiver travado, o teardown abaixo pode demorar; o
    // usuário não deve ficar olhando pra uma janela que "não fecha".
    XUnmapWindow(dpy, win);
    XFlush(dpy);

    if (recording.load()) stopRec();
    savePatch(sessionFile);   // continua daqui na próxima sessão
    running = false;
    audio.join();
    stopAudioIn();   // fecha a captura e devolve o microfone/entrada
    stopMidiIn();     // fecha a porta MIDI virtual
    XFreePixmap(dpy, bb);
    XFreePixmap(dpy, logoPix);
    if (fsCap && fsCap != fs) XFreeFontSet(dpy, fsCap);
    if (fs) XFreeFontSet(dpy, fs);
    if (coreFont) XFreeFont(dpy, coreFont);
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
