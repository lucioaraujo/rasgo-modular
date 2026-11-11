#pragma once

// ============================================================================
// Instâncias — o mesmo módulo mais de uma vez no rack (11/11/2026)
// ============================================================================
//
// Decisão do autor (11/11/2026, "sim, faça a sua recomendação"): permitir
// abrir o mesmo módulo várias vezes, COM LIMITE e por família — dois OSC
// desafinados, dois ENVELOPE (VCA e filtro), vários LFOs. A análise está em
// `dossies/AUDITORIA_modulos_e_cardinal.md` §1.
//
// O núcleo e o formato de patch já eram por instância (`node <id> <tipo>`,
// cabos e modulações por id); a partitura também (`modulo <id> <tipo>`). O
// que faltava era: (1) não deixar duplicar o que é do rack inteiro; (2) um
// limite; (3) distinguir as cópias na tela ("OSC 2"). Ordem combinada:
// app primeiro (aqui), depois partitura/VARIA, e só então a seed.
//
// Específico do app (não é regra RASGO comum). Sem JUCE: testável.

#include "core/SignalGraph.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace rasgo::panel {

// Uma cópia só: o que é do rack inteiro — a soma e a saída (MIXER, MASTER),
// o tempo comum (CLOCK), as pontes com o mundo de fora (SIGNAL-IN, NOTE-OUT)
// e a escuta da saída (LISTEN). Dois MASTER disputariam a saída de som.
inline bool isSingletonType(const std::string& type) {
    return type == "MIXER" || type == "MASTER" || type == "CLOCK"
        || type == "SIGNAL-IN" || type == "NOTE-OUT" || type == "LISTEN";
}

// Quantas cópias de um mesmo tipo, no máximo (os demais tipos).
inline constexpr int kMaxInstances = 4;

// Tudo aqui conta só os módulos que ESTÃO no rack (`shown`): remover um
// módulo no app o esconde e desliga, mas o nó continua no grafo.
inline int countOfType(rasgo::modular::SignalGraph& g,
                       const std::vector<std::size_t>& shown, const std::string& type) {
    int n = 0;
    for (const std::size_t id : shown)
        if (id < g.nodeCount() && g.node(id).type() == type) ++n;
    return n;
}

// Pode entrar mais um `type`? O motivo da recusa vai para o aviso da
// paleta, traduzido pelo app (`strings::instanceSingle` / `instanceMax`).
enum class AddRefusal { none, singleton, tooMany };
inline AddRefusal canAddInstance(rasgo::modular::SignalGraph& g,
                                 const std::vector<std::size_t>& shown, const std::string& type) {
    const int n = countOfType(g, shown, type);
    if (isSingletonType(type) && n >= 1) return AddRefusal::singleton;
    if (n >= kMaxInstances) return AddRefusal::tooMany;
    return AddRefusal::none;
}

// Número da cópia (1, 2, …) pela ordem de criação (id); 0 se é a única.
inline int instanceNumber(rasgo::modular::SignalGraph& g,
                          const std::vector<std::size_t>& shown, const std::size_t id) {
    if (id >= g.nodeCount()) return 0;
    const std::string t = g.node(id).type();
    int before = 0, total = 0;
    for (const std::size_t i : shown) {
        if (i >= g.nodeCount() || g.node(i).type() != t) continue;
        ++total;
        if (i < id) ++before;
    }
    return total > 1 ? before + 1 : 0;
}

// O nome que aparece: "OSC", ou "OSC 2" quando há mais de uma cópia.
inline std::string instanceLabel(rasgo::modular::SignalGraph& g,
                                 const std::vector<std::size_t>& shown, const std::size_t id) {
    if (id >= g.nodeCount()) return {};
    const int n = instanceNumber(g, shown, id);
    const std::string t = g.node(id).type();
    return n > 0 ? t + " " + std::to_string(n) : t;
}

}  // namespace rasgo::panel
