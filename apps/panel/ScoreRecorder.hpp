#pragma once

#include <cstddef>
#include <iomanip>
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
    void clear() noexcept { events_.clear(); }

    // formato de texto próprio — uma linha por evento, em ordem de
    // registro (não reordena por tempo: a ordem de chamada já É a
    // ordem cronológica, porque quem chama processa o grafo em blocos
    // crescentes). Não é MusicXML/MIDI — ver o comentário do topo.
    std::string toText() const {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << "rasgo-system-score 1\n";
        for (const auto& e : events_) {
            out << "t=" << std::fixed << std::setprecision(6) << e.time << ' ';
            switch (e.type) {
            case RasgoEvent::Type::Connection:
                out << "connection " << e.sourceNode << ':' << e.sourcePort
                    << " -> " << e.targetNode << ':' << e.targetPort << '\n';
                break;
            case RasgoEvent::Type::Modulation:
                out << "modulation " << e.sourceNode << ':' << e.sourcePort
                    << " -> " << e.targetNode << '.' << e.targetParamId
                    << " depth=" << std::setprecision(6) << e.depth
                    << " offset=" << e.offset << '\n';
                break;
            case RasgoEvent::Type::ParameterChange:
                out << "param " << e.targetNode << '.' << e.targetParamId
                    << ' ' << std::setprecision(6) << e.fromValue << " -> "
                    << e.toValue << '\n';
                break;
            case RasgoEvent::Type::Note:
                out << "note " << e.targetNode
                    << " pitch=" << std::setprecision(6) << e.notePitch
                    << " velocity=" << e.noteVelocity
                    << " duration=" << e.noteDuration
                    << " accent=" << (e.noteAccent ? 1 : 0) << '\n';
                break;
            }
        }
        return out.str();
    }

private:
    std::vector<RasgoEvent> events_;
};

}  // namespace rasgo::panel
