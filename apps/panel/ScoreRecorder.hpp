#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <map>
#include <locale>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
// ScoreRecorder — registro de eventos do patch (SYSTEM + MUSICAL SCORE)
// ============================================================================
//
// Ver o estudo: `RASGO_MODULAR/dossies/ESTUDO_seed_composicao_generativa.md`
// §5. Nível `SYSTEM SCORE` — conexões, modulações por cabo, e mudanças
// de parâmetro (do `MotionEngine`, do `PatchGenetics`, ou de um giro de
// knob no painel) — junto do começo do nível `MUSICAL SCORE`: eventos
// de `Note` (nota-liga/nota-desliga, pitch, velocity, acento).
//
// A "noção de nota" que faltava (§5 original: "precisaria que
// ENVELOPE/SEQUENCE/etc. anunciassem seus próprios eventos de forma
// genérica, mudando a interface deles") foi resolvida SEM mudar
// nenhum dos outros módulos: `NOTE-OUT` (`src/dsp/NoteOut.hpp`,
// Módulo 38) é um observador que você cabeia em GATE+PITCH onde
// quiser — o `ScoreRecorder` continua sem saber o que é um oscilador
// ou um envelope, só recebe o evento já pronto de quem chama
// (`panel_main.cpp`, que faz a ponte lendo `NoteOut::takeCompletedNote()`
// a cada bloco). Ainda não é MusicXML/MIDI de verdade — só a captura,
// visível no texto da partitura (próxima camada, não inventada aqui
// sem revisão).
//
// Regra de determinismo: o `ScoreRecorder` NUNCA lê relógio de parede.
// Quem chama passa `t` (sempre `amostra_atual / sample_rate`, derivado
// da contagem de blocos processados) — então dois renders do mesmo
// patch com o mesmo seed produzem o mesmo texto de partitura, não só o
// mesmo áudio.
//
// `rasgo_modular_core` continua sem saber o que é "partitura" — isto
// vive em `apps/panel/`, como `MotionEngine.hpp` e `PatchGenetics.hpp`.
// Sem alocação DENTRO de `process()`: `ScoreRecorder` só é chamado fora
// do áudio (mesma regra do `MotionEngine`/`PatchGenetics`).

namespace rasgo::panel {

struct RasgoEvent {
    enum class Type { Connection, Modulation, ParameterChange, Note };

    double time = 0.0;   // segundos — SEMPRE amostra/sr, nunca wall clock
    Type type = Type::ParameterChange;

    // Connection (cabo áudio/controle) e Modulation (connectToParameter)
    std::size_t sourceNode = 0;
    std::size_t sourcePort = 0;
    std::size_t targetNode = 0;
    std::size_t targetPort = 0;      // Connection
    std::string targetParamId;       // Modulation / ParameterChange
    float depth = 0.0f, offset = 0.0f;   // Modulation

    // ParameterChange
    float fromValue = 0.0f, toValue = 0.0f;

    // Note (MUSICAL SCORE, via NOTE-OUT — ver `src/dsp/NoteOut.hpp`):
    // `time` acima é o início (nota-liga); os campos abaixo completam.
    // `notePitch` é 1V/oct CRU — conversão pra nome de nota/MIDI fica
    // pra hora de exportar, não presa aqui.
    float notePitch = 0.0f;
    float noteVelocity = 1.0f;
    double noteDuration = 0.0;
    bool noteAccent = false;
};

// Descrição de um módulo para o texto da partitura.
//
// O `ScoreRecorder` não conhece `SignalGraph` — é decisão de projeto, e
// continua valendo. Mas escrever `33:1 -> 38:2` e chamar aquilo de
// partitura era inútil: na sessão de escuta de 23 set. 2026 o autor disse
// "não dá pra entender que cabo está conectado onde ou quais módulos
// estão acionados, nem qual a regulagem empregada" — sobre o documento
// que o próprio protocolo chama de principal do Estudo 3.
//
// A saída é quem chama passar o dicionário. O gravador continua sem saber
// o que é um oscilador; só sabe imprimir o nome que recebeu.
struct ScoreNodeInfo {
    std::string type;                                  // "OSC", "MIXER"…
    std::vector<std::string> inPorts, outPorts;        // nomes das portas
    std::vector<std::pair<std::string, float>> params; // a regulagem
};

class ScoreRecorder {
public:
    void connection(const double t, const std::size_t sourceNode,
                    const std::size_t sourcePort, const std::size_t targetNode,
                    const std::size_t targetPort) {
        RasgoEvent e;
        e.time = t; e.type = RasgoEvent::Type::Connection;
        e.sourceNode = sourceNode; e.sourcePort = sourcePort;
        e.targetNode = targetNode; e.targetPort = targetPort;
        events_.push_back(std::move(e));
    }

    void modulation(const double t, const std::size_t sourceNode,
                    const std::size_t sourcePort, const std::size_t targetNode,
                    const std::string& targetParamId, const float depth,
                    const float offset) {
        RasgoEvent e;
        e.time = t; e.type = RasgoEvent::Type::Modulation;
        e.sourceNode = sourceNode; e.sourcePort = sourcePort;
        e.targetNode = targetNode; e.targetParamId = targetParamId;
        e.depth = depth; e.offset = offset;
        events_.push_back(std::move(e));
    }

    void parameterChange(const double t, const std::size_t node,
                         const std::string& paramId, const float from,
                         const float to) {
        RasgoEvent e;
        e.time = t; e.type = RasgoEvent::Type::ParameterChange;
        e.targetNode = node; e.targetParamId = paramId;
        e.fromValue = from; e.toValue = to;
        events_.push_back(std::move(e));
    }

    // MUSICAL SCORE — 1 nota completa (nota-liga já detectada e
    // nota-desliga já chegou). `t` = início da nota (nota-liga).
    void note(const double t, const std::size_t node, const float pitch,
             const float velocity, const double duration, const bool accent) {
        RasgoEvent e;
        e.time = t; e.type = RasgoEvent::Type::Note;
        e.targetNode = node;
        e.notePitch = pitch; e.noteVelocity = velocity;
        e.noteDuration = duration; e.noteAccent = accent;
        events_.push_back(std::move(e));
    }

    std::size_t eventCount() const noexcept { return events_.size(); }
    const std::vector<RasgoEvent>& events() const noexcept { return events_; }
    void clear() noexcept { events_.clear(); nodes_.clear(); seed_ = 0;
                            sampleRate_ = 0.0; }

    // ---- dicionário, preenchido por quem chama ------------------------
    void describe(const std::size_t id, ScoreNodeInfo info) {
        nodes_[id] = std::move(info);
    }
    void setSeed(const std::uint64_t s) noexcept { seed_ = s; }
    void setSampleRate(const double sr) noexcept { sampleRate_ = sr; }

    // Texto da partitura, para ser LIDO por uma pessoa.
    //
    // Antes era uma lista de `33:1 -> 38:2`: cronologicamente correta,
    // determinística, e ilegível. Agora traz, nesta ordem:
    //
    //   1. o SEED — sem ele a tomada não é reproduzível, e ele faltava;
    //   2. os MÓDULOS que participam, com a regulagem de cada um;
    //   3. as LIGAÇÕES com nome de módulo e de porta;
    //   4. os eventos ao longo do tempo.
    //
    // Quando não há dicionário (chamador antigo, ou um módulo que não foi
    // descrito), cai no número cru — degrada, não quebra.
    std::string toText() const {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::fixed;

        out << "rasgo-system-score 2\n";
        if (seed_ != 0)       out << "seed " << seed_ << '\n';
        if (sampleRate_ > 0.0)
            out << "taxa " << std::setprecision(0) << sampleRate_ << " Hz\n";

        // ---- módulos e regulagem -------------------------------------
        // Só os que aparecem em algum evento: listar os 59 do rack
        // afogaria o que importa. Partitura é o que TOCOU.
        std::vector<std::size_t> usados;
        for (const auto& e : events_) {
            usados.push_back(e.targetNode);
            if (e.type == RasgoEvent::Type::Connection
                || e.type == RasgoEvent::Type::Modulation)
                usados.push_back(e.sourceNode);
        }
        std::sort(usados.begin(), usados.end());
        usados.erase(std::unique(usados.begin(), usados.end()), usados.end());

        if (!nodes_.empty()) {
            out << "\n# módulos\n";
            for (const std::size_t id : usados) {
                const auto it = nodes_.find(id);
                if (it == nodes_.end()) continue;
                out << "modulo " << id << ' ' << it->second.type;
                for (const auto& [nome, v] : it->second.params)
                    out << ' ' << nome << '=' << std::setprecision(4) << v;
                out << '\n';
            }
        }

        out << "\n# eventos\n";
        for (const auto& e : events_) {
            out << "t=" << std::setprecision(6) << e.time << ' ';
            switch (e.type) {
            case RasgoEvent::Type::Connection:
                out << "cabo " << saida(e.sourceNode, e.sourcePort)
                    << " -> " << entrada(e.targetNode, e.targetPort) << '\n';
                break;
            case RasgoEvent::Type::Modulation:
                out << "modula " << saida(e.sourceNode, e.sourcePort)
                    << " -> " << nome(e.targetNode) << '.' << e.targetParamId
                    << " profundidade=" << std::setprecision(4) << e.depth
                    << " deslocamento=" << e.offset << '\n';
                break;
            case RasgoEvent::Type::ParameterChange:
                out << "ajuste " << nome(e.targetNode) << '.' << e.targetParamId
                    << ' ' << std::setprecision(4) << e.fromValue << " -> "
                    << e.toValue << '\n';
                break;
            case RasgoEvent::Type::Note:
                out << "nota " << nome(e.targetNode)
                    << " altura=" << std::setprecision(4) << e.notePitch
                    << " intensidade=" << e.noteVelocity
                    << " duração=" << e.noteDuration
                    << (e.noteAccent ? " acento" : "") << '\n';
                break;
            }
        }
        return out.str();
    }

private:
    // "OSC[12]" quando há dicionário, "12" quando não há: o texto degrada
    // para o formato antigo em vez de mentir ou falhar.
    std::string nome(const std::size_t id) const {
        const auto it = nodes_.find(id);
        std::ostringstream o;
        if (it == nodes_.end()) { o << id; return o.str(); }
        o << it->second.type << '[' << id << ']';
        return o.str();
    }
    std::string porta(const std::vector<std::string>& v,
                      const std::size_t i) const {
        std::ostringstream o;
        if (i < v.size() && !v[i].empty()) o << v[i]; else o << i;
        return o.str();
    }
    std::string saida(const std::size_t id, const std::size_t p) const {
        const auto it = nodes_.find(id);
        return nome(id) + '.'
             + (it == nodes_.end() ? std::to_string(p)
                                   : porta(it->second.outPorts, p));
    }
    std::string entrada(const std::size_t id, const std::size_t p) const {
        const auto it = nodes_.find(id);
        return nome(id) + '.'
             + (it == nodes_.end() ? std::to_string(p)
                                   : porta(it->second.inPorts, p));
    }

    std::vector<RasgoEvent> events_;
    std::map<std::size_t, ScoreNodeInfo> nodes_;
    std::uint64_t seed_ = 0;
    double sampleRate_ = 0.0;
};

}  // namespace rasgo::panel
