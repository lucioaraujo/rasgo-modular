// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Lúcio de Araújo
//
// Cópia no Rasgo Modular: 15/11/2026, do monorepo privado
// rasgo-instruments (RASGO/PROJETOS_AUTORAIS/MAESTRUC/receptor), SHA-256 da
// origem 7cc9164840db…; ponte de som, mixer e senha. Ao atualizar, copiar de lá e anotar.
//
// Receptor de referência do protocolo maestruc.link/1 (PROTOCOLO_MAESTRUC_LINK.md)
// para apps JUCE. Fonte: RASGO/PROJETOS_AUTORAIS/MAESTRUC/receptor/MaestrucLink.h
// (14/11/2026). Cada instrumento leva uma cópia para o próprio repositório,
// registrando de onde veio; não há dependência de build entre projetos.
//
// Uso:
//     rasgo::maestruc::Link link {"Batruc", "0.1.2"};
//     link.handlers.play  = [this] (juce::int64 startUs) { ... };   // fio de mensagens
//     link.handlers.seed  = [this] (const juce::String& s) { ... };
//     link.start();                       // opcional: o app funciona igual sem Maestruc
//     link.pushEvent ("ataque", 0.8f);    // pode ser chamado do callback de áudio
//     link.setAudioLoad (0.31f, xruns);   // idem
//
// Os handlers rodam no fio de mensagens do JUCE (o mesmo da interface). Um
// handler vazio = capacidade ausente: o Maestruc recebe ack com ok = 0.
// Requer o módulo juce_osc (mesma licença do JUCE usado pela família) e
// PonteDeSom.h (na mesma pasta).
//
// 15/11/2026 — som pela ponte, mixer e senha de sessão:
//   - Som: no prepareToPlay, link.openSoundBridge (pares, taxa, nomes); no
//     callback de áudio, por um PortaoDeSom (ver abaixo):
//         if (portao.escrever (canais, n, quadros)) /* calar a própria saída */;
//     O Maestruc lê o som pela ponte e pede ao app que cale a própria saída;
//     se o Maestruc some, o app volta a tocar sozinho em ~0,5 s.
//   - Mixer: handlers.mixerList preenche os canais (índice -1 = master);
//     handlers.mixerSet aplica nível/mudo/solo vindos do Maestruc; quando
//     quem toca mexe no mixer do app, chamar link.mixerChanged().
//   - Senha: o Maestruc grava a senha da sessão num arquivo só do usuário
//     (pasta das pontes); o welcome só vale se trouxer essa senha, e todo
//     comando do Maestruc a traz como último argumento. Outro usuário da
//     máquina não consegue mandar comandos.

#pragma once

#include <juce_osc/juce_osc.h>

#include "PonteDeSom.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <thread>
#include <vector>

#if JUCE_WINDOWS
 #include <processthreadsapi.h>
#else
 #include <unistd.h>
#endif

namespace rasgo::maestruc
{

inline constexpr int maestrucPort = 47900;

// A senha da sessão do Maestruc: um arquivo na pasta das pontes (só do
// usuário). O Maestruc grava; o receptor só lê.
inline std::string caminhoDaSenha() { return caminhoDaSenhaMaestruc(); }

inline std::string lerSenha()
{
    const auto caminho = caminhoDaSenha();
    if (caminho.empty()) return {};
#if ! defined(_WIN32)
    struct stat st {};
    if (::lstat (caminho.c_str(), &st) != 0 || ! S_ISREG (st.st_mode) || st.st_uid != ::getuid()
        || (st.st_mode & 077) != 0)
        return {};
#endif
    std::ifstream f (caminho);
    std::string s;
    std::getline (f, s);
    return s.size() >= 16 && s.size() <= 128 ? s : std::string();
}

// Um canal do mixer de dentro do instrumento (índice -1 = o master dele).
// `objects`: os controles que o canal tem de verdade no instrumento, entre
// "nivel", "mudo", "solo" e "pan" (separados por vírgula); o Maestruc mostra só
// esses. `minDb`/`maxDb`: a faixa do fader do instrumento. 16/11/2026.
struct MixerChannel
{
    int index = 0;
    juce::String name;
    float levelDb = 0.0f;
    bool mute = false, solo = false;
    juce::String objects { "nivel,mudo,solo" };
    float minDb = -90.0f, maxDb = 12.0f;
};

struct Handlers
{
    std::function<void (juce::int64 startMicros)> play;   // 0 = agora
    std::function<void()> stop;
    std::function<void (float bpm)> tempo;
    std::function<void (int tonic, const juce::String& mode)> key;
    std::function<void (int index, const juce::String& name)> section;
    std::function<void (const juce::String& seed)> seed;
    // Abrir peça (arquivo do próprio instrumento). Devolve false e explica em
    // `note` se não abriu (formato, arquivo inválido…). 15/11/2026.
    std::function<bool (const juce::File& file, juce::String& note)> open;
    std::function<bool (const juce::String& name, float value)> macro;   // false = macro desconhecida
    std::function<void (const juce::File& folder, const juce::String& take)> recStart;
    std::function<void()> recStop;
    std::function<void (int level)> budget;   // 0 normal, 1 econômico, 2 mínimo
    std::function<void (bool& playing, juce::String& seed, float& bpm)> state;
    // Mixer de dentro do app (15/11/2026). mixerList: os canais e o master
    // (índice -1). mixerSet: what = "nivel" (dB), "mudo" ou "solo" (0/1);
    // false = índice ou valor recusado. Os dois juntos = capacidade "mixer".
    std::function<void (std::vector<MixerChannel>& channels)> mixerList;
    std::function<bool (int index, const juce::String& what, float value)> mixerSet;
};

class Link;

// O callback de áudio do instrumento escreve na ponte por aqui. O portão
// vive no instrumento (fora do Link): antes de desfazer o Link, o
// instrumento chama fecharEsperar(), e nenhum bloco de áudio fica no meio.
class PortaoDeSom
{
public:
    void abrir (Link* link) noexcept { link_.store (link); }
    // Fio de mensagens: depois desta chamada, o áudio não usa mais o Link.
    void fecharEsperar() noexcept
    {
        link_.store (nullptr);
        for (int i = 0; ocupado_.load() != 0 && i < 100000; ++i) std::this_thread::yield();
    }
    // Callback de áudio. true = o Maestruc pede que o app cale a própria saída.
    inline bool escrever (const float* const* canais, int numCanais, int quadros) noexcept;

private:
    std::atomic<Link*> link_ { nullptr };
    std::atomic<int> ocupado_ { 0 };
};

class Link final : private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
                   private juce::Timer
{
public:
    // `emits`: o que o instrumento de fato envia — "event", "obs", "load".
    // Só entra no anúncio o que for declarado aqui.
    Link (juce::String name, juce::String version,
          juce::StringArray macroNames = {}, juce::StringArray emits = {})
        : name_ (std::move (name)), version_ (std::move (version)),
          macros_ (std::move (macroNames)), emits_ (std::move (emits)) {}

    ~Link() override { stop(); }

    Handlers handlers;

    // ---- ponte de som (15/11/2026) ----
    // Fora do callback de áudio (prepareToPlay, com o áudio parado). `pares`:
    // 1 = só a mistura; mais = a mistura e os canais internos (nomes, um por
    // par; o 1º é a mistura). Devolve false se não abriu (o app segue igual).
    bool openSoundBridge (int pares, double taxa, const juce::StringArray& nomes = {})
    {
        std::vector<std::string> guardados;
        std::vector<const char*> ptrs;
        for (auto& n : nomes) guardados.push_back (n.toStdString());
        for (auto& n : guardados) ptrs.push_back (n.c_str());
        const auto caminho = emissor_.abrir (name_.toStdString(), pares, taxa, ptrs.empty() ? nullptr : ptrs.data());
        if (caminho.empty()) return false;
        bridgePairs_ = pares;
        bridgeRate_ = (float) taxa;
        bridgePath_ = juce::String (caminho);
        bridgeChanged_.store (true);
        return true;
    }
    // Callback de áudio (pelo PortaoDeSom). true = calar a própria saída.
    bool writeSound (const float* const* canais, int numCanais, int quadros) noexcept
    {
        return emissor_.escrever (canais, numCanais, quadros);
    }
    // Quem toca mexeu no mixer do app: manda a lista de novo ao Maestruc.
    void mixerChanged() { if (welcomed_) sendMixer(); }

    // Abre a porta local (escolhida pelo sistema) e começa a se anunciar.
    bool start (int maestrucPortNumber = maestrucPort)
    {
        stop();
        socket_ = std::make_unique<juce::DatagramSocket> (false);
        if (! socket_->bindToPort (0, "127.0.0.1"))
            return fail();
        port_ = socket_->getBoundPort();
        if (! receiver_.connectToSocket (*socket_)
            || ! sender_.connectToSocket (*socket_, "127.0.0.1", maestrucPortNumber))
            return fail();
        receiver_.addListener (this);
        welcomed_ = false;
        ticks_ = 0;
        startTimerHz (20);
        announce();
        return true;
    }

    void stop()
    {
        // a ponte só fecha no destrutor (o instrumento chama
        // PortaoDeSom::fecharEsperar() antes de desfazer o Link)
        if (socket_ == nullptr)
            return;
        stopTimer();
        out ("bye", port_);
        receiver_.removeListener (this);
        receiver_.disconnect();
        sender_.disconnect();
        socket_.reset();
        port_ = 0;
        welcomed_ = false;
    }

    bool isConnectedToMaestruc() const noexcept { return welcomed_; }
    int port() const noexcept { return port_; }

    // ---- seguros para o callback de áudio: sem alocação, sem lock ----
    void pushEvent (const char* type, float value) noexcept
    {
        int start1, size1, start2, size2;
        fifo_.prepareToWrite (1, start1, size1, start2, size2);
        if (size1 + size2 == 0)
            return;   // fila cheia: o acontecimento se perde, o áudio não espera
        auto& slot = events_[(size_t) (size1 > 0 ? start1 : start2)];
        std::strncpy (slot.type, type, sizeof (slot.type) - 1);
        slot.type[sizeof (slot.type) - 1] = '\0';
        slot.value = value;
        slot.micros = nowMicros();
        fifo_.finishedWrite (1);
    }

    void setAudioLoad (float fractionOfBlock, int xruns) noexcept
    {
        load_.store (fractionOfBlock, std::memory_order_relaxed);
        xruns_.store (xruns, std::memory_order_relaxed);
    }

    // ---- fio de mensagens ----
    void sendObservation (const juce::String& observation, float value) { out ("obs", port_, observation, value); }

private:
    struct Event { char type[24] {}; float value = 0.0f; juce::int64 micros = 0; };

    static juce::int64 nowMicros() noexcept
    {
        using namespace std::chrono;
        return duration_cast<microseconds> (system_clock::now().time_since_epoch()).count();
    }

    static int processId() noexcept
    {
       #if JUCE_WINDOWS
        return (int) ::GetCurrentProcessId();
       #else
        return (int) ::getpid();
       #endif
    }

    bool fail() { socket_.reset(); port_ = 0; return false; }

    template <typename... Args>
    void out (const char* address, Args&&... args)
    {
        if (socket_ != nullptr)
            sender_.send (juce::OSCAddressPattern (juce::String ("/rasgo/1/") + address),
                          std::forward<Args> (args)...);
    }

    juce::String capabilities() const
    {
        juce::StringArray caps;
        for (auto& e : emits_)
            if (e == "event" || e == "obs" || e == "load") caps.add (e);
        if (handlers.play && handlers.stop) caps.add ("transport");
        if (handlers.tempo)    caps.add ("tempo");
        if (handlers.key)      caps.add ("key");
        if (handlers.section)  caps.add ("section");
        if (handlers.seed)     caps.add ("seed");
        if (handlers.open)     caps.add ("open");
        if (handlers.recStart && handlers.recStop) caps.add ("rec");
        if (handlers.budget)   caps.add ("budget");
        if (handlers.mixerList && handlers.mixerSet) caps.add ("mixer");
        if (emissor_.aberto() || bridgeChanged_.load()) caps.add ("audio");
        if (handlers.macro)
            for (auto& m : macros_) caps.add ("macro:" + m);
        return caps.joinIntoString (",");
    }

    void announce()
    {
        out ("hello", name_, version_, port_, processId(), capabilities());
    }

    void timerCallback() override
    {
        // acontecimentos vindos do áudio: no máximo 64 por tique
        int start1, size1, start2, size2;
        fifo_.prepareToRead (juce::jmin (64, fifo_.getNumReady()), start1, size1, start2, size2);
        auto sendRange = [this] (int start, int size)
        {
            for (int i = start; i < start + size; ++i)
            {
                const auto& e = events_[(size_t) i];
                out ("event", port_, juce::String (e.type), e.value, juce::String (e.micros));
            }
        };
        sendRange (start1, size1);
        sendRange (start2, size2);
        fifo_.finishedRead (size1 + size2);

        if (welcomed_ && bridgeChanged_.exchange (false))
            sendBridge();
        if (++ticks_ % 20 != 0)
            return;   // o resto, uma vez por segundo
        if (welcomed_)
            out ("alive", port_);
        else
            announce();
        if (emits_.contains ("load"))
            out ("load", port_, load_.load (std::memory_order_relaxed), xruns_.load (std::memory_order_relaxed));
    }

    void ack (const juce::String& address, bool ok, const juce::String& note = {})
    {
        out ("ack", port_, address, ok ? 1 : 0, note);
    }

    void oscMessageReceived (const juce::OSCMessage& m) override
    {
        const auto address = m.getAddressPattern().toString();
        if (! address.startsWith ("/rasgo/1/"))
            return;
        const auto name = address.substring (9);
        auto isS = [&m] (int i) { return m.size() > i && m[i].isString(); };
        auto isI = [&m] (int i) { return m.size() > i && m[i].isInt32(); };
        auto isF = [&m] (int i) { return m.size() > i && m[i].isFloat32(); };

        if (name == "welcome")
        {
            // Só vale com a senha que o Maestruc gravou no arquivo do usuário.
            const auto senha = lerSenha();
            if (senha.empty() || ! isS (0) || m[0].getString().toStdString() != senha)
                return;
            token_ = senha;
            welcomed_ = true;
            if (emissor_.aberto()) sendBridge();
            if (handlers.mixerList && handlers.mixerSet) sendMixer();
            return;
        }
        if (name == "who")     { welcomed_ = false; announce(); return; }
        // Todo comando traz a senha como último argumento.
        if (token_.empty() || m.size() < 1 || ! m[m.size() - 1].isString()
            || m[m.size() - 1].getString().toStdString() != token_)
            return;
        if (name == "query")
        {
            bool playing = false; juce::String seed; float bpm = 0.0f;
            if (handlers.state) handlers.state (playing, seed, bpm);
            out ("state", port_, playing ? 1 : 0, seed, bpm);
            return;
        }

        bool ok = false;
        juce::String note = "capacidade ausente";
        if (name == "transport/play" && handlers.play)
        {
            ok = true;
            handlers.play (isS (0) ? m[0].getString().getLargeIntValue() : 0);
        }
        else if (name == "transport/stop" && handlers.stop) { ok = true; handlers.stop(); }
        else if (name == "tempo" && handlers.tempo && isF (0)) { ok = true; handlers.tempo (m[0].getFloat32()); }
        else if (name == "key" && handlers.key && isI (0) && isS (1))
        {
            ok = m[0].getInt32() >= 0 && m[0].getInt32() < 12;
            if (ok) handlers.key (m[0].getInt32(), m[1].getString()); else note = juce::String (juce::CharPointer_UTF8 ("t\xc3\xb4nica fora de 0..11"));
        }
        else if (name == "section" && handlers.section && isI (0) && isS (1))
        {
            ok = true; handlers.section (m[0].getInt32(), m[1].getString());
        }
        else if (name == "seed" && handlers.seed && isS (0)) { ok = true; handlers.seed (m[0].getString()); }
        else if (name == "open" && handlers.open && isS (0))
        {
            const juce::File file (m[0].getString());
            if (! juce::File::isAbsolutePath (m[0].getString()) || ! file.existsAsFile())
                note = "arquivo inexistente";
            else
                ok = handlers.open (file, note);
        }
        else if (name == "macro" && handlers.macro && isS (0) && isF (1))
        {
            ok = macros_.contains (m[0].getString()) && handlers.macro (m[0].getString(), juce::jlimit (0.0f, 1.0f, m[1].getFloat32()));
            if (! ok) note = "macro desconhecida";
        }
        else if (name == "rec/start" && handlers.recStart && isS (0) && isS (1))
        {
            const juce::File folder (m[0].getString());
            ok = juce::File::isAbsolutePath (m[0].getString()) && folder.isDirectory();
            if (ok) handlers.recStart (folder, m[1].getString()); else note = "pasta inexistente";
        }
        else if (name == "rec/stop" && handlers.recStop) { ok = true; handlers.recStop(); }
        else if (name == "budget" && handlers.budget && isI (0))
        {
            ok = true; handlers.budget (juce::jlimit (0, 2, m[0].getInt32()));
        }
        else if (name == "mixer/listar" && handlers.mixerList && handlers.mixerSet)
        {
            ok = true; sendMixer();
        }
        else if ((name == "mixer/nivel" || name == "mixer/mudo" || name == "mixer/solo")
                 && handlers.mixerList && handlers.mixerSet && isI (0) && (isF (1) || isI (1)))
        {
            const float v = isF (1) ? m[1].getFloat32() : (float) m[1].getInt32();
            const auto what = name.fromFirstOccurrenceOf ("/", false, false);
            const bool valido = what == "nivel" ? (v >= -90.0f && v <= 12.0f)
                                                : (std::fabs (v) < 1.0e-6f || std::fabs (v - 1.0f) < 1.0e-6f);   // 0 ou 1
            ok = valido && handlers.mixerSet (m[0].getInt32(), what, v);
            if (! ok) note = "canal ou valor recusado";
            else sendMixer();
        }
        if (ok) note = {};
        ack (address, ok, note);
    }

    void sendBridge()
    {
        out ("ponte", port_, bridgePath_, bridgePairs_, bridgeRate_, juce::String (token_));
    }

    void sendMixer()
    {
        std::vector<MixerChannel> ch;
        handlers.mixerList (ch);
        int n = 0;
        for (auto& c : ch)
        {
            if (n >= 64) break;   // um limite: a lista não inunda o Maestruc
            out ("mixer/canal", port_, c.index, c.name.substring (0, 48), c.levelDb, c.mute ? 1 : 0, c.solo ? 1 : 0,
                 c.objects.substring (0, 48), c.minDb, c.maxDb);
            ++n;
        }
        out ("mixer/fim", port_, n);
    }

    juce::String name_, version_;
    juce::StringArray macros_, emits_;
    std::string token_;
    EmissorDeSom emissor_;
    juce::String bridgePath_;
    int bridgePairs_ = 0;
    float bridgeRate_ = 0.0f;
    std::atomic<bool> bridgeChanged_ { false };
    std::unique_ptr<juce::DatagramSocket> socket_;
    juce::OSCReceiver receiver_;
    juce::OSCSender sender_;
    int port_ = 0;
    bool welcomed_ = false;
    int ticks_ = 0;

    juce::AbstractFifo fifo_ { 256 };
    std::array<Event, 256> events_ {};
    std::atomic<float> load_ { 0.0f };
    std::atomic<int> xruns_ { 0 };
};

inline bool PortaoDeSom::escrever (const float* const* canais, int numCanais, int quadros) noexcept
{
    ocupado_.fetch_add (1);
    auto* l = link_.load();
    const bool calar = l != nullptr && l->writeSound (canais, numCanais, quadros);
    ocupado_.fetch_sub (1);
    return calar;
}

} // namespace rasgo::maestruc
