#pragma once

#include <string>

namespace rasgo::panel {

// Conteúdo pedagógico, separado do catálogo para testar sua apresentação
// sem compilar as tabelas de tradução inteiras.
struct LearnEntry {
    std::string quick;
    std::string understand;
    std::string explore;
};

} // namespace rasgo::panel
