#pragma once

#include "Graph.hpp"  // AudioBlock, PortDescriptor, PortKind, Parameter, ParameterDescriptor
#include "Panel.hpp"

#include <array>
#include <cmath>
#include <functional>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// SignalGraph — grafo de sinal unificado do Rasgo Modular
// ============================================================================
//
// Este é o caminho de áudio que faltava (INVENTARIO_GLOBAL_MODULOS,
// `CORE-GRAPH-CONNECTION`, marcado "prioridade arquitetural"). Ele NÃO é
// só encanamento: o grafo é a principal fonte de variedade sonora
// (autor, 2026-09-01: "os fluxos proporcionam variedades sonoras").
//
// Quatro decisões que vêm do Atlas de Referência:
//
//   1. A CONEXÃO É UM OBJETO, não um fio transparente (Atlas seções 9-11).
//      `Connection` processa o bloco que atravessa. Começa com passagem +
//      ruptura/cicatriz (seção 37, Clouds -> "romper não significa
//      apagar"); a arquitetura já prevê ganho, atraso, filtro, saturação,
//      probabilidade e degradação na própria conexão.
//
//   2. FEEDBACK com atraso explícito de um bloco (Atlas seção 9.1, Hexen
//      seção 119.4). Feedback é uma das maiores fontes de variedade
//      generativa. Uma conexão marcada `feedback` lê a saída do bloco
//      ANTERIOR e fica fora da verificação de ciclo.
//
//   3. QUALQUER saída -> qualquer entrada, inclusive saída -> PARÂMETRO
//      (modulação). `connectToParameter` amostra o bloco de origem e
//      escreve num parâmetro do módulo de destino, com profundidade e
//      offset. Metade da variedade nasce da modulação.
//
//   4. FAN-IN EXPLÍCITO com orçamento de ganho
//      (ORQUESTRACAO_INSTRUMENTOS.md, "fan-in com orçamento de ganho e
//      mixer explícito"). Cada porta de entrada recebe UMA conexão; a
//      soma é feita por um módulo `Sum` explícito. Cada conexão tem seu
//      próprio ganho.
//
// Preparação vs execução: `prepare()` calcula ordem topológica e aloca os
// buffers por porta FORA do caminho de áudio. `process()` só percorre
// armazenamento pré-alocado; sem alocação, lock ou I/O (contrato RASGO
// realtime, AGENTS.md seção 4).

namespace rasgo::modular {

// ---------------------------------------------------------------------------
// Signal — base de um módulo que processa áudio por bloco
// ---------------------------------------------------------------------------
class Signal {
public:
    virtual ~Signal() = default;

    virtual std::string type() const = 0;

    // Chamado fora do caminho de áudio, antes de `process`.
    virtual void prepare(float sampleRate, std::size_t blockSize) {
        sampleRate_ = sampleRate;
        blockSize_ = blockSize;
    }

    // `inputs[i]` pode ser nullptr (porta sem conexão) - trate como silêncio.
    // `outputs` já vem dimensionado (uma entrada por porta de saída).
    virtual void process(const std::vector<const AudioBlock*>& inputs,
                         std::vector<AudioBlock>& outputs) noexcept = 0;

    std::size_t inputCount() const noexcept { return inputPorts_.size(); }
    std::size_t outputCount() const noexcept { return outputPorts_.size(); }
    const PortDescriptor& inputDescriptor(std::size_t port) const {
        return inputPorts_.at(port);
    }
    const PortDescriptor& outputDescriptor(std::size_t port) const {
        return outputPorts_.at(port);
    }

    const std::vector<Parameter>& parameters() const noexcept { return parameters_; }

    // Descrição de painel. O padrão é um auto-layout simples (parâmetros
    // numa fileira de knobs, portas numa fileira de jacks embaixo) - o
    // módulo sobrescreve pra um layout próprio. É só estrutura; a
    // linguagem visual fica no renderizador.
    virtual Panel panel() const {
        Panel p;
        const std::size_t slots = std::max(parameters_.size(),
                                           inputPorts_.size() + outputPorts_.size());
        p.hp = std::max(4, static_cast<int>((slots * 10 + 8) / 4));  // ~10 unid/slot
        float x = 3.0f;
        for (const auto& parameter : parameters_) {
            const bool toggle = parameter.descriptor.minimum >= 0.0f
                && parameter.descriptor.maximum <= 1.0f
                && parameter.descriptor.maximum - parameter.descriptor.minimum <= 1.0f;
            p.add(toggle ? Widget::Kind::Toggle : Widget::Kind::Knob,
                  parameter.descriptor.id, parameter.descriptor.id, x, 4.0f);
            x += 10.0f;
        }
        x = 3.0f;
        for (const auto& port : inputPorts_) {
            p.add(Widget::Kind::Jack, port.name, "in:" + port.name, x, 10.0f);
            x += 8.0f;
        }
        for (const auto& port : outputPorts_) {
            p.add(Widget::Kind::Jack, port.name, "out:" + port.name, x, 10.0f);
            x += 8.0f;
        }
        return p;
    }

    void setParameter(const std::string& id, const float value) {
        for (auto& parameter : parameters_) {
            if (parameter.descriptor.id == id) {
                parameter.value = std::clamp(value, parameter.descriptor.minimum,
                                             parameter.descriptor.maximum);
                return;
            }
        }
        throw std::invalid_argument("unknown parameter: " + id);
    }

    float parameterValue(const std::string& id) const {
        for (const auto& parameter : parameters_)
            if (parameter.descriptor.id == id)
                return parameter.value;
        throw std::invalid_argument("unknown parameter: " + id);
    }

    std::string manifest() const {
        std::ostringstream stream;
        stream << "{\"version\":1,\"type\":\"" << type() << "\",\"inputs\":[";
        for (std::size_t index = 0; index < inputPorts_.size(); ++index) {
            if (index != 0) stream << ',';
            stream << "{\"name\":\"" << inputPorts_[index].name << "\",\"kind\":\""
                   << portKindName(inputPorts_[index].kind) << "\"}";
        }
        stream << "],\"outputs\":[";
        for (std::size_t index = 0; index < outputPorts_.size(); ++index) {
            if (index != 0) stream << ',';
            stream << "{\"name\":\"" << outputPorts_[index].name << "\",\"kind\":\""
                   << portKindName(outputPorts_[index].kind) << "\"}";
        }
        stream << "],\"parameters\":[";
        for (std::size_t index = 0; index < parameters_.size(); ++index) {
            if (index != 0) stream << ',';
            const auto& parameter = parameters_[index];
            stream << "{\"id\":\"" << parameter.descriptor.id << "\",\"min\":"
                   << parameter.descriptor.minimum << ",\"max\":"
                   << parameter.descriptor.maximum << ",\"default\":"
                   << parameter.descriptor.defaultValue << ",\"unit\":\""
                   << parameter.descriptor.unit << "\"}";
        }
        stream << "]}";
        return stream.str();
    }

protected:
    Signal(std::vector<PortDescriptor> inputPorts,
           std::vector<PortDescriptor> outputPorts,
           std::vector<ParameterDescriptor> parameterDescriptors = {})
        : inputPorts_(std::move(inputPorts)), outputPorts_(std::move(outputPorts)) {
        for (auto& descriptor : parameterDescriptors)
            parameters_.push_back({descriptor, descriptor.defaultValue});
    }

    float sampleRate_ = 48000.0f;
    std::size_t blockSize_ = 64;

private:
    std::vector<PortDescriptor> inputPorts_;
    std::vector<PortDescriptor> outputPorts_;
    std::vector<Parameter> parameters_;
};

// ---------------------------------------------------------------------------
// Connection — a conexão como objeto que processa (Atlas seções 9-11, 37)
// ---------------------------------------------------------------------------
enum class CableState {
    Intact,
    Ruptured,
};

// A RELAÇÃO entre dois sinais pode ser o próprio DSP (Warps, Atlas §39):
// a conexão lê um segundo sinal (companion) e o combina com o que
// atravessa. Não é um módulo - é comportamento da conexão.
enum class Relation {
    None,        // passa direto
    RingMod,     // x * companion (blend por `amount`)
    Fold,        // wavefolder de x dirigido pelo companion
    Difference,  // x - amount * companion (retificador de diferença)
};

class Cable {
public:
    struct Endpoint {
        std::size_t node = 0;
        std::size_t port = 0;
    };

    Cable(const Endpoint source, const Endpoint target, const bool feedback,
          const float gain)
        : source_(source), target_(target), feedback_(feedback), gain_(gain) {}

    const Endpoint& source() const noexcept { return source_; }
    const Endpoint& target() const noexcept { return target_; }
    bool isFeedback() const noexcept { return feedback_; }
    CableState state() const noexcept { return state_; }
    float gain() const noexcept { return gain_; }
    float conductance() const noexcept { return conductance_; }
    bool conductingNow() const noexcept { return conducting_; }

    void setGain(const float gain) noexcept { gain_ = gain; }

    // Ganho de CONSTELAÇÃO: a superfície de performance põe os nós num
    // campo; a distância entre dois nós vira a intensidade da conexão entre
    // eles (perto = forte, longe = quase mudo). `SignalGraph` calcula e
    // aplica isto a partir das posições; 1.0 = neutro (sem constelação).
    void setConstellationGain(const float value) noexcept {
        constellationGain_ = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    }
    float constellationGain() const noexcept { return constellationGain_; }

    // Probabilidade de conduzir (Marbles/Branches embutido em TODA conexão -
    // decisão do autor, 2026-09-01: "toda conexão nasce com probabilidade de
    // condução"). 1.0 = sempre conduz. A decisão é re-sorteada em intervalos
    // (~20 Hz), não por amostra - senão soa como ruído de AM. Quando não
    // conduz, o sinal cai rápido a silêncio e fica (estado "intermitente"
    // do Atlas §24) até o próximo sorteio dar "conduz".
    void setConductance(const float value) noexcept {
        conductance_ = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    }

    // Faz a conexão PROCESSAR: combina o sinal que atravessa com um
    // segundo sinal (`companion`). `amount` mistura seco/molhado.
    void setRelation(const Relation relation, const std::size_t companionNode,
                     const std::size_t companionPort, const float amount) noexcept {
        relation_ = relation;
        companion_ = {companionNode, companionPort};
        relationAmount_ = amount < 0.0f ? 0.0f : (amount > 1.0f ? 1.0f : amount);
    }
    Relation relation() const noexcept { return relation_; }
    const Endpoint& companion() const noexcept { return companion_; }
    float relationAmount() const noexcept { return relationAmount_; }
    bool hasRelation() const noexcept { return relation_ != Relation::None; }

    // Gesto de performance: romper a conexão. O sinal NÃO some - a
    // cicatriz segura o último bloco e o repete decaindo (Clouds -> freeze,
    // Atlas seção 37: "romper não significa apagar").
    void rupture() noexcept {
        if (state_ == CableState::Ruptured)
            return;
        state_ = CableState::Ruptured;
        scarGain_ = 1.0f;
        scarPhase_ = 0;
    }

    void reconnect() noexcept {
        state_ = CableState::Intact;
        scarGain_ = 0.0f;
    }

    void prepare(const AudioBlock& shape, const float sampleRate) {
        held_ = shape;
        held_.clear();
        heldValid_ = false;
        // Cicatriz decai ~350 ms.
        scarDecayPerSample_ =
            std::exp(-1.0f / (0.350f * std::max(1.0f, sampleRate)));
        // dropout de condução cai ~30 ms.
        dropDecayPerSample_ = std::exp(-1.0f / (0.030f * std::max(1.0f, sampleRate)));
        gateInterval_ = static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 20.0f));
        gateCounter_ = 0;
        conducting_ = true;
        dropGain_ = 1.0f;
        rngState_ = 0x9E3779B97F4A7C15ULL
            ^ (static_cast<std::uint64_t>(source_.node) << 32)
            ^ (static_cast<std::uint64_t>(target_.node) << 16)
            ^ static_cast<std::uint64_t>(target_.port);
    }

    // Escreve em `destination` o resultado de `source` atravessando a
    // conexão. `source`/`companion` podem estar vazios (nullptr).
    void process(const AudioBlock* sourceBlock, const AudioBlock* companionBlock,
                 AudioBlock& destination) noexcept {
        const std::size_t channels = destination.channels();
        const std::size_t frames = destination.frames();

        if (state_ == CableState::Intact) {
            if (sourceBlock != nullptr && sourceBlock->matches(destination)) {
                for (std::size_t frame = 0; frame < frames; ++frame) {
                    // sorteio de condução em intervalos (~20 Hz), por frame
                    if (gateCounter_ == 0)
                        conducting_ = conductance_ >= 1.0f || uniform01() < conductance_;
                    if (++gateCounter_ >= gateInterval_)
                        gateCounter_ = 0;
                    const float goal = conducting_ ? 1.0f : 0.0f;
                    dropGain_ = goal + dropDecayPerSample_ * (dropGain_ - goal);
                    if (goal == 0.0f && dropGain_ < 1.0e-4f)
                        dropGain_ = 0.0f;
                    for (std::size_t channel = 0; channel < channels; ++channel) {
                        float x = sourceBlock->at(channel, frame) * gain_;
                        if (relation_ != Relation::None) {
                            const float y = (companionBlock != nullptr
                                             && companionBlock->matches(destination))
                                ? companionBlock->at(channel, frame)
                                : 0.0f;
                            x = applyRelation(x, y);
                        }
                        destination.at(channel, frame) =
                            x * dropGain_ * constellationGain_;
                    }
                }
                held_.copyFrom(destination);
                heldValid_ = true;
            } else {
                destination.clear();
            }
            return;
        }

        // Ruptura: repete o último bloco guardado, decaindo. Abaixo de um
        // piso a cicatriz é silêncio absoluto (não fica um rastro infinito
        // de nível numérico ínfimo).
        if (!heldValid_ || scarGain_ <= 1.0e-3f) {
            scarGain_ = 0.0f;
            destination.clear();
            return;
        }
        const std::size_t heldFrames = held_.frames();
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float envelope = scarGain_;
            const std::size_t sourceFrame = scarPhase_ % heldFrames;
            for (std::size_t channel = 0; channel < channels; ++channel)
                destination.at(channel, frame) =
                    held_.at(channel, sourceFrame) * envelope;
            ++scarPhase_;
            scarGain_ *= scarDecayPerSample_;
        }
    }

private:
    float uniform01() noexcept {
        rngState_ ^= rngState_ >> 12;
        rngState_ ^= rngState_ << 25;
        rngState_ ^= rngState_ >> 27;
        const std::uint64_t x = rngState_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF) / static_cast<float>(0x1000000);
    }

    float applyRelation(const float x, const float y) const noexcept {
        const float a = relationAmount_;
        switch (relation_) {
        case Relation::RingMod:
            return x * (1.0f - a) + (x * y) * a;
        case Relation::Fold: {
            float f = x * (1.0f + a * 3.0f * std::fabs(y));
            for (int i = 0; i < 4; ++i) {
                if (f > 1.0f) f = 2.0f - f;
                else if (f < -1.0f) f = -2.0f - f;
                else break;
            }
            return f;
        }
        case Relation::Difference:
            return x - a * y;
        case Relation::None:
        default:
            return x;
        }
    }

    Endpoint source_;
    Endpoint target_;
    bool feedback_ = false;
    float gain_ = 1.0f;
    Relation relation_ = Relation::None;
    Endpoint companion_;
    float relationAmount_ = 0.0f;

    float constellationGain_ = 1.0f;

    float conductance_ = 1.0f;
    bool conducting_ = true;
    float dropGain_ = 1.0f;
    float dropDecayPerSample_ = 0.99f;
    std::uint32_t gateInterval_ = 2400;
    std::uint32_t gateCounter_ = 0;
    std::uint64_t rngState_ = 0x9E3779B97F4A7C15ULL;

    CableState state_ = CableState::Intact;
    AudioBlock held_{48000.0f, 2, 64};
    bool heldValid_ = false;
    float scarGain_ = 0.0f;
    float scarDecayPerSample_ = 0.999f;
    std::uint64_t scarPhase_ = 0;
};

inline const char* relationName(const Relation relation) noexcept {
    switch (relation) {
    case Relation::RingMod: return "ring";
    case Relation::Fold: return "fold";
    case Relation::Difference: return "diff";
    case Relation::None: return "none";
    }
    return "none";
}
inline Relation relationFromName(const std::string& name) noexcept {
    if (name == "ring") return Relation::RingMod;
    if (name == "fold") return Relation::Fold;
    if (name == "diff") return Relation::Difference;
    return Relation::None;
}

// ---------------------------------------------------------------------------
// ParameterLink — modulação: uma saída de áudio escreve num parâmetro
// ---------------------------------------------------------------------------
struct ParameterLink {
    std::size_t sourceNode = 0;
    std::size_t sourcePort = 0;
    std::size_t targetNode = 0;
    std::string parameterId;
    float depth = 1.0f;
    float offset = 0.0f;
    bool feedback = false;
    // valor de intenção do usuário (o knob), capturado no connect: a
    // modulação é ADITIVA sobre ele -> `param = base + offset + depth·fonte`.
    // Um front-end atualiza via `SignalGraph::setParameterBase`.
    float base = 0.0f;
};

// ---------------------------------------------------------------------------
// SignalGraph
// ---------------------------------------------------------------------------
class SignalGraph {
public:
    std::size_t add(std::unique_ptr<Signal> node) {
        if (!node)
            throw std::invalid_argument("cannot add a null signal node");
        nodes_.push_back(std::move(node));
        prepared_ = false;
        return nodes_.size() - 1;
    }

    Signal& node(const std::size_t id) { return *nodes_.at(id); }

    // Conecta (source.node, source.port) -> (target.node, target.port).
    // Cada porta de ENTRADA aceita apenas uma conexão (fan-in é explícito,
    // via um módulo Sum). `feedback = true` usa a saída do bloco anterior e
    // não entra na verificação de ciclo.
    Cable& connect(const std::size_t sourceNode, const std::size_t sourcePort,
                        const std::size_t targetNode, const std::size_t targetPort,
                        const bool feedback = false, const float gain = 1.0f) {
        checkNode(sourceNode);
        checkNode(targetNode);
        if (sourcePort >= nodes_[sourceNode]->outputCount())
            throw std::out_of_range("source port out of range");
        if (targetPort >= nodes_[targetNode]->inputCount())
            throw std::out_of_range("target port out of range");
        for (const auto& connection : connections_)
            if (connection->target().node == targetNode
                && connection->target().port == targetPort)
                throw std::invalid_argument(
                    "input port already has a connection - use an explicit Sum");
        connections_.push_back(std::make_unique<Cable>(
            Cable::Endpoint{sourceNode, sourcePort},
            Cable::Endpoint{targetNode, targetPort}, feedback, gain));
        prepared_ = false;
        return *connections_.back();
    }

    void connectToParameter(const std::size_t sourceNode, const std::size_t sourcePort,
                            const std::size_t targetNode, const std::string& parameterId,
                            const float depth = 1.0f, const float offset = 0.0f,
                            const bool feedback = false) {
        checkNode(sourceNode);
        checkNode(targetNode);
        if (sourcePort >= nodes_[sourceNode]->outputCount())
            throw std::out_of_range("source port out of range");
        const float base = nodes_[targetNode]->parameterValue(parameterId);  // valida id
        parameterLinks_.push_back(
            {sourceNode, sourcePort, targetNode, parameterId, depth, offset,
             feedback, base});
        prepared_ = false;
    }

    // Atualiza o VALOR DE INTENÇÃO (o knob) de um parâmetro: se ele tem
    // modulação (link de parâmetro ou follower de qualidade), a `base` da
    // modulação passa a ser `value` — a modulação continua ADITIVA sobre o
    // novo knob. Se não tem modulação, é um `setParameter` comum. Um
    // front-end chama isto quando o usuário mexe num knob.
    void setParameterBase(const std::size_t node, const std::string& id,
                          const float value) {
        checkNode(node);
        nodes_[node]->setParameter(id, value);
        const float applied = nodes_[node]->parameterValue(id);  // já clampado
        for (auto& link : parameterLinks_)
            if (link.targetNode == node && link.parameterId == id)
                link.base = applied;
        for (auto& f : followers_)
            if (f.targetNode == node && f.parameterId == id)
                f.base = applied;
    }

    // Valor de intenção do usuário de um parâmetro: a `base` se ele tem
    // modulação, senão o valor atual do parâmetro. (Usado por `serialize`:
    // depois de `process()` o `parameterValue` guarda o valor MODULADO.)
    float parameterUserValue(const std::size_t node, const std::string& id) const {
        for (const auto& link : parameterLinks_)
            if (link.targetNode == node && link.parameterId == id)
                return link.base;
        for (const auto& f : followers_)
            if (f.targetNode == node && f.parameterId == id)
                return f.base;
        return nodes_[node]->parameterValue(id);
    }

    // Um parâmetro tem modulação entrando (link de parâmetro ou follower
    // de qualidade)? Consumidor: a Motion Engine (`apps/panel/`), pra não
    // brigar com a fiação do músico / do seed.
    bool parameterIsModulated(const std::size_t node,
                              const std::string& id) const noexcept {
        for (const auto& link : parameterLinks_)
            if (link.targetNode == node && link.parameterId == id) return true;
        for (const auto& f : followers_)
            if (f.targetNode == node && f.parameterId == id) return true;
        return false;
    }

    std::size_t cableCount() const noexcept { return connections_.size(); }
    Cable& cable(const std::size_t index) { return *connections_.at(index); }
    std::size_t nodeCount() const noexcept { return nodes_.size(); }

    // Último bloco calculado na saída (node, port) — pra medidores /
    // osciloscópios de UI. Válido só depois de um `process()`; devolve
    // nullptr se ainda não preparado ou índices fora de faixa.
    const AudioBlock* lastOutput(const std::size_t node,
                                 const std::size_t port) const noexcept {
        if (node >= previousOutputs_.size()
            || port >= previousOutputs_[node].size())
            return nullptr;
        return &previousOutputs_[node][port];
    }

    // Remove a conexão que chega em (targetNode, targetPort), se houver.
    // Devolve true se removeu. A porta de entrada volta a estar livre
    // (fan-in continua explícito). Requer novo prepare() antes de process().
    bool disconnect(const std::size_t targetNode,
                    const std::size_t targetPort) noexcept {
        for (auto it = connections_.begin(); it != connections_.end(); ++it)
            if ((*it)->target().node == targetNode
                && (*it)->target().port == targetPort) {
                connections_.erase(it);
                prepared_ = false;
                return true;
            }
        return false;
    }

    // ---------------------------------------------------------------------
    // Modelo de conexão 1 — MATRIZ de roteamento (superfície de edição).
    // Linhas = saídas de todos os nós; colunas = destinos roteáveis (toda
    // porta de entrada + todo parâmetro modulável). Cada célula é vazia,
    // um `Cable` (áudio) ou um `ParameterLink` (modulação). É uma VISÃO
    // sobre o mesmo grafo - não duplica estado.
    // ---------------------------------------------------------------------
    struct MatrixSource {
        std::size_t node = 0;
        std::size_t port = 0;
    };
    struct MatrixSlot {
        enum class Kind { Input, Param };
        std::size_t node = 0;
        Kind kind = Kind::Input;
        std::size_t port = 0;      // se Input
        std::string parameterId;   // se Param
    };
    struct MatrixCell {
        enum class Kind { Empty, Cable, Modulation };
        Kind kind = Kind::Empty;
        const Cable* cable = nullptr;
        const ParameterLink* link = nullptr;
    };

    std::vector<MatrixSource> matrixSources() const {
        std::vector<MatrixSource> rows;
        for (std::size_t id = 0; id < nodes_.size(); ++id)
            for (std::size_t p = 0; p < nodes_[id]->outputCount(); ++p)
                rows.push_back({id, p});
        return rows;
    }

    std::vector<MatrixSlot> matrixSlots() const {
        std::vector<MatrixSlot> cols;
        for (std::size_t id = 0; id < nodes_.size(); ++id) {
            for (std::size_t p = 0; p < nodes_[id]->inputCount(); ++p)
                cols.push_back({id, MatrixSlot::Kind::Input, p, {}});
            for (const auto& parameter : nodes_[id]->parameters())
                cols.push_back({id, MatrixSlot::Kind::Param, 0,
                                parameter.descriptor.id});
        }
        return cols;
    }

    MatrixCell matrixCell(const MatrixSource& src, const MatrixSlot& slot) const {
        if (slot.kind == MatrixSlot::Kind::Input) {
            for (const auto& c : connections_)
                if (c->source().node == src.node && c->source().port == src.port
                    && c->target().node == slot.node
                    && c->target().port == slot.port)
                    return {MatrixCell::Kind::Cable, c.get(), nullptr};
        } else {
            for (const auto& l : parameterLinks_)
                if (l.sourceNode == src.node && l.sourcePort == src.port
                    && l.targetNode == slot.node
                    && l.parameterId == slot.parameterId)
                    return {MatrixCell::Kind::Modulation, nullptr, &l};
        }
        return {};
    }

    // Renderiza a matriz como texto (superfície de edição em modo terminal).
    // '.' vazio  'X' cabo  '~' cabo com relação  'c' cabo com condução<1
    // 'm' modulação de parâmetro
    std::string matrixToText() const {
        const auto rows = matrixSources();
        const auto cols = matrixSlots();
        std::ostringstream out;
        out << "matriz " << rows.size() << "x" << cols.size() << "  (linhas=saídas, colunas=destinos)\n";
        for (std::size_t r = 0; r < rows.size(); ++r) {
            out << nodes_[rows[r].node]->type() << ':' << rows[r].port << "  ";
            for (const auto& slot : cols) {
                const auto cell = matrixCell(rows[r], slot);
                char ch = '.';
                if (cell.kind == MatrixCell::Kind::Cable) {
                    ch = 'X';
                    if (cell.cable->hasRelation()) ch = '~';
                    else if (cell.cable->conductance() < 1.0f) ch = 'c';
                } else if (cell.kind == MatrixCell::Kind::Modulation) {
                    ch = 'm';
                }
                out << ch;
            }
            out << '\n';
        }
        return out.str();
    }

    // ---------------------------------------------------------------------
    // Modelo de conexão 2 — CONSTELAÇÃO (superfície de performance). Cada nó
    // tem uma posição num campo; a distância entre dois nós conectados vira
    // a intensidade da conexão. Mesma ideia de
    // `PER-RS-CONSTELLATION-GEOMETRY-V1`. Não muda a topologia - só o ganho.
    // ---------------------------------------------------------------------
    void setNodePosition(const std::size_t id, const float x, const float y) {
        checkNode(id);
        if (positions_.size() < nodes_.size())
            positions_.resize(nodes_.size(), {0.0f, 0.0f});
        positions_[id] = {x, y};
    }
    std::pair<float, float> nodePosition(const std::size_t id) const {
        checkNode(id);
        if (id < positions_.size())
            return {positions_[id].x, positions_[id].y};
        return {0.0f, 0.0f};
    }

    // Acoplamento por distância: 1 quando coincidem, caindo suavemente a 0
    // em ~`radius`. Gaussiana (sem descontinuidade, sempre >0 mas some
    // rápido). `radius <= 0` desativa (retorna 1).
    static float couplingFromDistance(const float distance,
                                      const float radius) noexcept {
        if (radius <= 0.0f)
            return 1.0f;
        const float t = distance / radius;
        return std::exp(-t * t);
    }

    // Aplica o campo: para cada cabo, ganho de constelação =
    // couplingFromDistance(distância entre os nós de origem e destino).
    void applyConstellation(const float radius) {
        if (positions_.size() < nodes_.size())
            positions_.resize(nodes_.size(), {0.0f, 0.0f});
        for (auto& c : connections_) {
            const auto& a = positions_[c->source().node];
            const auto& b = positions_[c->target().node];
            const float dx = a.x - b.x;
            const float dy = a.y - b.y;
            c->setConstellationGain(
                couplingFromDistance(std::sqrt(dx * dx + dy * dy), radius));
        }
    }

    // Volta todos os cabos ao ganho de constelação neutro (1.0).
    void clearConstellation() {
        for (auto& c : connections_)
            c->setConstellationGain(1.0f);
    }

    // ---------------------------------------------------------------------
    // Modelo de conexão 3 — BARRAMENTO SEMÂNTICO. Conectar por SIGNIFICADO,
    // não por porta: um nó CONTRIBUI para uma qualidade (energia, brilho,
    // densidade, tensão, movimento) e um parâmetro de outro nó SEGUE essa
    // qualidade. Muitas fontes somam numa qualidade, muitos parâmetros a
    // seguem, sem cabo explícito. Padrão `EnergyControlBus` (TRIOIO),
    // generalizado. As qualidades são resolvidas do bloco ANTERIOR.
    // ---------------------------------------------------------------------
    enum class Quality { Energy, Brightness, Density, Tension, Motion, Count };

    static const char* qualityName(const Quality q) noexcept {
        switch (q) {
        case Quality::Energy: return "energy";
        case Quality::Brightness: return "brightness";
        case Quality::Density: return "density";
        case Quality::Tension: return "tension";
        case Quality::Motion: return "motion";
        default: return "?";
        }
    }

    static Quality qualityFromName(const std::string& name) noexcept {
        if (name == "brightness") return Quality::Brightness;
        if (name == "density") return Quality::Density;
        if (name == "tension") return Quality::Tension;
        if (name == "motion") return Quality::Motion;
        return Quality::Energy;
    }

    // O nó contribui pra uma qualidade: valor = clamp(bias + weight·|média
    // do bloco|·escala), somado (média) com as outras contribuições.
    void contributeQuality(const std::size_t sourceNode, const std::size_t sourcePort,
                           const Quality quality, const float weight = 1.0f,
                           const float bias = 0.0f) {
        checkNode(sourceNode);
        if (sourcePort >= nodes_[sourceNode]->outputCount())
            throw std::out_of_range("source port out of range");
        contributions_.push_back({sourceNode, sourcePort, quality, weight, bias});
        prepared_ = false;
    }

    // Um parâmetro segue uma qualidade: param = offset + depth·qualidade.
    void followQuality(const Quality quality, const std::size_t targetNode,
                       const std::string& parameterId, const float depth = 1.0f,
                       const float offset = 0.0f) {
        checkNode(targetNode);
        const float base = nodes_[targetNode]->parameterValue(parameterId);  // valida id
        followers_.push_back({quality, targetNode, parameterId, depth, offset, base});
        prepared_ = false;
    }

    float qualityValue(const Quality q) const noexcept {
        return qualities_[static_cast<std::size_t>(q)];
    }

    // ---------------------------------------------------------------------
    // Patch como PARTITURA legível (Atlas §23, "cabo como partitura"). O
    // texto É a composição: salvável, lível, versionável. Uma linha por nó,
    // por cabo e por link de parâmetro.
    //
    // O texto usa SEMPRE ponto decimal (locale "C"), independente do locale
    // do processo — um front-end que chama setlocale(LC_ALL, "") num sistema
    // pt_BR/de_DE não pode fazer "0.3" virar 0 na releitura (era o bug que
    // zerava todos os parâmetros fracionários a cada salvar/carregar).
    // ---------------------------------------------------------------------
    static float parseNum(const std::string& s) {
        std::istringstream in(s);
        in.imbue(std::locale::classic());
        double v = 0.0;
        in >> v;
        return static_cast<float>(v);
    }
    static std::size_t parseIndex(const std::string& s) {
        std::istringstream in(s);
        in.imbue(std::locale::classic());
        unsigned long long v = 0;
        in >> v;
        return static_cast<std::size_t>(v);
    }

    std::string serialize() const {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << "rasgo-modular-patch 1\n";
        for (std::size_t id = 0; id < nodes_.size(); ++id) {
            out << "node " << id << ' ' << nodes_[id]->type();
            for (const auto& parameter : nodes_[id]->parameters())
                // valor de INTENÇÃO (a base do knob): depois de process() o
                // `parameter.value` guarda o valor modulado, mas o patch
                // grava o que o usuário setou
                out << ' ' << parameter.descriptor.id << '='
                    << std::setprecision(9)
                    << parameterUserValue(id, parameter.descriptor.id);
            out << '\n';
        }
        for (const auto& c : connections_) {
            out << "cable " << c->source().node << ':' << c->source().port
                << " -> " << c->target().node << ':' << c->target().port
                << " gain=" << std::setprecision(9) << c->gain()
                << " conductance=" << c->conductance()
                << " feedback=" << (c->isFeedback() ? 1 : 0)
                << " state=" << (c->state() == CableState::Ruptured ? "ruptured" : "intact");
            if (c->hasRelation())
                out << " relation=" << relationName(c->relation())
                    << " companion=" << c->companion().node << ':' << c->companion().port
                    << " amount=" << c->relationAmount();
            out << '\n';
        }
        for (const auto& link : parameterLinks_) {
            out << "mod " << link.sourceNode << ':' << link.sourcePort
                << " -> " << link.targetNode << ':' << link.parameterId
                << " depth=" << std::setprecision(9) << link.depth
                << " offset=" << link.offset
                << " feedback=" << (link.feedback ? 1 : 0) << '\n';
        }
        // camada 2 - constelação: posições de nó (só as não-nulas)
        for (std::size_t id = 0; id < positions_.size(); ++id) {
            if (positions_[id].x == 0.0f && positions_[id].y == 0.0f)
                continue;
            out << "pos " << id << ' ' << std::setprecision(9)
                << positions_[id].x << ' ' << positions_[id].y << '\n';
        }
        // camada 3 - barramento semântico
        for (const auto& c : contributions_)
            out << "qin " << c.node << ':' << c.port << ' '
                << qualityName(c.quality) << " weight=" << std::setprecision(9)
                << c.weight << " bias=" << c.bias << '\n';
        for (const auto& f : followers_)
            out << "qout " << qualityName(f.quality) << " -> " << f.targetNode
                << ':' << f.parameterId << " depth=" << std::setprecision(9)
                << f.depth << " offset=" << f.offset << '\n';
        return out.str();
    }

    // `factory(type)` devolve um `Signal` novo pra cada linha `node`.
    static SignalGraph deserialize(
        const std::string& text,
        const std::function<std::unique_ptr<Signal>(const std::string&)>& factory) {
        SignalGraph graph;
        std::istringstream in(text);
        std::string line;
        bool header = false;
        while (std::getline(in, line)) {
            if (line.empty())
                continue;
            std::istringstream ls(line);
            ls.imbue(std::locale::classic());  // ponto decimal, sempre
            std::string kind;
            ls >> kind;
            if (kind == "rasgo-modular-patch") {
                header = true;
                continue;
            }
            if (!header)
                throw std::invalid_argument("patch sem cabeçalho rasgo-modular-patch");
            if (kind == "node") {
                std::size_t id = 0;
                std::string type;
                ls >> id >> type;
                auto node = factory(type);
                if (!node)
                    throw std::invalid_argument("factory não conhece o tipo: " + type);
                const std::size_t added = graph.add(std::move(node));
                if (added != id)
                    throw std::invalid_argument("nós fora de ordem no patch");
                std::string token;
                while (ls >> token) {
                    const auto eq = token.find('=');
                    if (eq == std::string::npos)
                        continue;
                    graph.node(id).setParameter(token.substr(0, eq),
                                                parseNum(token.substr(eq + 1)));
                }
            } else if (kind == "cable" || kind == "mod") {
                std::string src, arrow, dst;
                ls >> src >> arrow >> dst;
                const auto sColon = src.find(':');
                const auto dColon = dst.find(':');
                const std::size_t sNode = parseIndex(src.substr(0, sColon));
                const std::size_t sPort = parseIndex(src.substr(sColon + 1));
                const std::string dLeft = dst.substr(0, dColon);
                const std::string dRight = dst.substr(dColon + 1);
                float gain = 1.0f, conductance = 1.0f, depth = 1.0f, offset = 0.0f;
                float relAmount = 0.0f;
                bool feedback = false;
                std::string ruptured, relation, companion;
                std::string token;
                while (ls >> token) {
                    const auto eq = token.find('=');
                    if (eq == std::string::npos)
                        continue;
                    const std::string key = token.substr(0, eq);
                    const std::string val = token.substr(eq + 1);
                    if (key == "gain") gain = parseNum(val);
                    else if (key == "conductance") conductance = parseNum(val);
                    else if (key == "depth") depth = parseNum(val);
                    else if (key == "offset") offset = parseNum(val);
                    else if (key == "feedback") feedback = (val == "1");
                    else if (key == "state") ruptured = val;
                    else if (key == "relation") relation = val;
                    else if (key == "companion") companion = val;
                    else if (key == "amount") relAmount = parseNum(val);
                }
                if (kind == "cable") {
                    Cable& c = graph.connect(sNode, sPort, parseIndex(dLeft),
                                             parseIndex(dRight), feedback, gain);
                    c.setConductance(conductance);
                    if (ruptured == "ruptured")
                        c.rupture();
                    if (!relation.empty() && !companion.empty()) {
                        const auto cColon = companion.find(':');
                        c.setRelation(relationFromName(relation),
                                      parseIndex(companion.substr(0, cColon)),
                                      parseIndex(companion.substr(cColon + 1)), relAmount);
                    }
                } else {
                    graph.connectToParameter(sNode, sPort, parseIndex(dLeft), dRight,
                                             depth, offset, feedback);
                }
            } else if (kind == "pos") {
                std::size_t id = 0;
                float x = 0.0f, y = 0.0f;
                ls >> id >> x >> y;
                graph.setNodePosition(id, x, y);
            } else if (kind == "qin") {
                std::string src, q;
                ls >> src >> q;
                const auto colon = src.find(':');
                float weight = 1.0f, bias = 0.0f;
                std::string token;
                while (ls >> token) {
                    const auto eq = token.find('=');
                    if (eq == std::string::npos) continue;
                    if (token.substr(0, eq) == "weight")
                        weight = parseNum(token.substr(eq + 1));
                    else if (token.substr(0, eq) == "bias")
                        bias = parseNum(token.substr(eq + 1));
                }
                graph.contributeQuality(parseIndex(src.substr(0, colon)),
                                        parseIndex(src.substr(colon + 1)),
                                        qualityFromName(q), weight, bias);
            } else if (kind == "qout") {
                std::string q, arrow, dst;
                ls >> q >> arrow >> dst;
                const auto colon = dst.find(':');
                float depth = 1.0f, offset = 0.0f;
                std::string token;
                while (ls >> token) {
                    const auto eq = token.find('=');
                    if (eq == std::string::npos) continue;
                    if (token.substr(0, eq) == "depth")
                        depth = parseNum(token.substr(eq + 1));
                    else if (token.substr(0, eq) == "offset")
                        offset = parseNum(token.substr(eq + 1));
                }
                graph.followQuality(qualityFromName(q),
                                    parseIndex(dst.substr(0, colon)),
                                    dst.substr(colon + 1), depth, offset);
            }
        }
        return graph;
    }

    void prepare(const float sampleRate, const std::size_t channels,
                 const std::size_t blockSize) {
        if (nodes_.empty())
            throw std::invalid_argument("graph has no nodes");
        sampleRate_ = sampleRate;
        blockSize_ = blockSize;
        channels_ = channels;

        const AudioBlock shape(sampleRate, channels, blockSize);
        currentOutputs_.assign(nodes_.size(), {});
        previousOutputs_.assign(nodes_.size(), {});
        gathered_.assign(nodes_.size(), {});
        inputPtrs_.assign(nodes_.size(), {});
        for (std::size_t id = 0; id < nodes_.size(); ++id) {
            currentOutputs_[id].assign(nodes_[id]->outputCount(), shape);
            previousOutputs_[id].assign(nodes_[id]->outputCount(), shape);
            gathered_[id].assign(nodes_[id]->inputCount(), shape);
            inputPtrs_[id].assign(nodes_[id]->inputCount(), nullptr);
            nodes_[id]->prepare(sampleRate, blockSize);
        }
        for (auto& connection : connections_)
            connection->prepare(shape, sampleRate);

        order_ = evaluationOrder();
        prepared_ = true;
    }

    // Opcional (front-ends ao vivo): restringe process() aos nós que
    // alimentam `outputNode`. Num painel com todos os módulos do catálogo
    // no rack, os órfãos deixam de custar DSP por bloco. Padrão =
    // kEvaluateAll (processa tudo, como sempre).
    static constexpr std::size_t kEvaluateAll = static_cast<std::size_t>(-1);
    void setActiveOutput(const std::size_t outputNode) {
        activeOutput_ = outputNode;
        if (prepared_) order_ = evaluationOrder();
    }
    void evaluateAllNodes() { setActiveOutput(kEvaluateAll); }

    // Renderiza um bloco na saída do nó `outputNode`, porta `outputPort`.
    void process(AudioBlock& destination, const std::size_t outputNode,
                 const std::size_t outputPort = 0) {
        if (!prepared_)
            throw std::logic_error("prepare() must run before process()");
        checkNode(outputNode);

        // Modelo de conexão 3 — BARRAMENTO SEMÂNTICO. Resolve as
        // QUALIDADES (energia/brilho/densidade/tensão/movimento) a partir
        // das contribuições (bloco ANTERIOR, evita depender da ordem), pra
        // os `followers` aplicarem antes de cada nó processar.
        if (!contributions_.empty() || !followers_.empty())
            resolveSemanticBus();

        for (const auto nodeId : order_) {
            auto& node = *nodes_[nodeId];

            // Barramento semântico: parâmetros que SEGUEM uma qualidade.
            // ADITIVO sobre a base (o knob): base + offset + depth·qualidade.
            for (const auto& f : followers_) {
                if (f.targetNode != nodeId)
                    continue;
                node.setParameter(
                    f.parameterId,
                    f.base + f.offset + f.depth
                        * qualities_[static_cast<std::size_t>(f.quality)]);
            }

            // Modulação: links de parâmetro que apontam pra este nó.
            // ADITIVO sobre a base: base + offset + depth·média(fonte).
            for (const auto& link : parameterLinks_) {
                if (link.targetNode != nodeId)
                    continue;
                const auto& sourceBlock = link.feedback
                    ? previousOutputs_[link.sourceNode][link.sourcePort]
                    : currentOutputs_[link.sourceNode][link.sourcePort];
                node.setParameter(
                    link.parameterId,
                    link.base + link.offset + link.depth * blockMean(sourceBlock));
            }

            // Reúne as entradas atravessando as conexões (a conexão processa).
            // `inputPtrs_` é pré-alocado em prepare() - nada aloca aqui.
            auto& inputs = inputPtrs_[nodeId];
            std::fill(inputs.begin(), inputs.end(), nullptr);
            for (auto& connection : connections_) {
                if (connection->target().node != nodeId)
                    continue;
                const auto& source = connection->isFeedback()
                    ? previousOutputs_[connection->source().node][connection->source().port]
                    : currentOutputs_[connection->source().node][connection->source().port];
                const AudioBlock* companion = nullptr;
                if (connection->hasRelation()) {
                    const auto& comp = connection->companion();
                    // o companion sempre vem do bloco ANTERIOR - evita
                    // depender da ordem topológica e permite auto-relação.
                    if (comp.node < previousOutputs_.size()
                        && comp.port < previousOutputs_[comp.node].size())
                        companion = &previousOutputs_[comp.node][comp.port];
                }
                AudioBlock& slot = gathered_[nodeId][connection->target().port];
                connection->process(&source, companion, slot);
                inputs[connection->target().port] = &slot;
            }

            node.process(inputs, currentOutputs_[nodeId]);
        }

        destination.copyFrom(currentOutputs_[outputNode].at(outputPort));
        std::swap(currentOutputs_, previousOutputs_);
    }

private:
    void checkNode(const std::size_t id) const {
        if (id >= nodes_.size())
            throw std::out_of_range("signal node out of range");
    }

    struct QualityContribution {
        std::size_t node = 0;
        std::size_t port = 0;
        Quality quality = Quality::Energy;
        float weight = 1.0f;
        float bias = 0.0f;
    };
    struct QualityFollower {
        Quality quality = Quality::Energy;
        std::size_t targetNode = 0;
        std::string parameterId;
        float depth = 1.0f;
        float offset = 0.0f;
        float base = 0.0f;   // valor de intenção do usuário; modulação aditiva
    };

    // Resolve `qualities_` das contribuições, usando o bloco ANTERIOR.
    void resolveSemanticBus() noexcept {
        constexpr std::size_t n = static_cast<std::size_t>(Quality::Count);
        float acc[n] = {};
        int cnt[n] = {};
        for (const auto& c : contributions_) {
            if (c.node >= previousOutputs_.size()
                || c.port >= previousOutputs_[c.node].size())
                continue;
            const auto& blk = previousOutputs_[c.node][c.port];
            float mag = 0.0f;
            const std::size_t total = blk.channels() * blk.frames();
            for (std::size_t ch = 0; ch < blk.channels(); ++ch)
                for (std::size_t fr = 0; fr < blk.frames(); ++fr)
                    mag += std::fabs(blk.at(ch, fr));
            mag = total > 0 ? mag / static_cast<float>(total) : 0.0f;
            float v = c.bias + c.weight * mag;
            v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
            const auto q = static_cast<std::size_t>(c.quality);
            acc[q] += v;
            ++cnt[q];
        }
        for (std::size_t q = 0; q < n; ++q)
            qualities_[q] = cnt[q] > 0 ? acc[q] / static_cast<float>(cnt[q]) : 0.0f;
    }

    static float blockMean(const AudioBlock& block) noexcept {
        float sum = 0.0f;
        for (std::size_t channel = 0; channel < block.channels(); ++channel)
            for (std::size_t frame = 0; frame < block.frames(); ++frame)
                sum += block.at(channel, frame);
        const float count =
            static_cast<float>(block.channels() * block.frames());
        return count > 0.0f ? sum / count : 0.0f;
    }

    // Ordem topológica só sobre as conexões NÃO-feedback; feedback usa o
    // bloco anterior e por isso não fecha ciclo.
    std::vector<std::size_t> evaluationOrder() const {
        std::vector<std::vector<std::size_t>> outgoing(nodes_.size());
        for (const auto& connection : connections_)
            if (!connection->isFeedback())
                outgoing[connection->source().node].push_back(connection->target().node);
        for (const auto& link : parameterLinks_)
            if (!link.feedback)
                outgoing[link.sourceNode].push_back(link.targetNode);

        std::vector<int> mark(nodes_.size(), 0);
        std::vector<std::size_t> order;
        std::function<void(std::size_t)> visit = [&](const std::size_t id) {
            if (mark[id] == 1)
                throw std::logic_error(
                    "signal graph has a cycle without an explicit feedback connection");
            if (mark[id] == 2)
                return;
            mark[id] = 1;
            for (const auto next : outgoing[id])
                visit(next);
            mark[id] = 2;
            order.push_back(id);
        };
        for (std::size_t id = 0; id < nodes_.size(); ++id)
            visit(id);
        std::reverse(order.begin(), order.end());

        // Poda opcional: só os nós que ALIMENTAM `activeOutput_` (inclui
        // arestas de feedback — a fonte de um cabo de feedback ainda pesa
        // na saída). Sentinela kEvaluateAll => sem poda (renders de
        // exemplo e testes byte-idênticos).
        if (activeOutput_ != kEvaluateAll && activeOutput_ < nodes_.size()) {
            std::vector<std::vector<std::size_t>> incoming(nodes_.size());
            for (const auto& connection : connections_)
                incoming[connection->target().node]
                    .push_back(connection->source().node);
            for (const auto& link : parameterLinks_)
                incoming[link.targetNode].push_back(link.sourceNode);
            std::vector<char> keep(nodes_.size(), 0);
            std::vector<std::size_t> stack{activeOutput_};
            keep[activeOutput_] = 1;
            while (!stack.empty()) {
                const std::size_t n = stack.back();
                stack.pop_back();
                for (const auto p : incoming[n])
                    if (!keep[p]) { keep[p] = 1; stack.push_back(p); }
            }
            order.erase(std::remove_if(order.begin(), order.end(),
                        [&](const std::size_t id) { return !keep[id]; }),
                        order.end());
        }
        return order;
    }

    struct NodePos {
        float x = 0.0f;
        float y = 0.0f;
    };

    std::vector<std::unique_ptr<Signal>> nodes_;
    std::vector<std::unique_ptr<Cable>> connections_;
    std::vector<ParameterLink> parameterLinks_;
    std::vector<NodePos> positions_;
    std::vector<QualityContribution> contributions_;
    std::vector<QualityFollower> followers_;
    std::array<float, static_cast<std::size_t>(Quality::Count)> qualities_{};

    std::vector<std::vector<AudioBlock>> currentOutputs_;
    std::vector<std::vector<AudioBlock>> previousOutputs_;
    std::vector<std::vector<AudioBlock>> gathered_;
    std::vector<std::vector<const AudioBlock*>> inputPtrs_;
    std::vector<std::size_t> order_;
    std::size_t activeOutput_ = kEvaluateAll;

    float sampleRate_ = 48000.0f;
    std::size_t blockSize_ = 64;
    std::size_t channels_ = 2;
    bool prepared_ = false;
};

}  // namespace rasgo::modular
