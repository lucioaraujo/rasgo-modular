#pragma once

#include <string>
#include <vector>

// ============================================================================
// Panel — descrição DECLARATIVA do painel de um módulo
// ============================================================================
//
// Dado puro, sem framework. Um módulo diz "tenho estes controles ligados a
// estes parâmetros/portas, nestas posições" - NÃO diz como um knob se
// parece, nem se há knob, nem se há cabo. É o esqueleto sobre o qual
// qualquer renderizador (web/WASM, JUCE, ASCII de teste) desenha a
// linguagem visual própria do Rasgo (Atlas §26 interface não neutra, §27
// visualidade própria; `MODULE_DEVELOPMENT_STANDARD` motor ≠ interface).
//
// Coordenadas: MILÍMETROS REAIS, a partir do canto superior-esquerdo do
// painel (decisão "Eurorack proporcional", `apps/panel/design.md §3.2`).
// `Panel::hp` é a largura em HP; a largura do painel = `hp · 5,08 mm`
// (1 HP = 0,2 pol = 5,08 mm, o horizontal pitch da Eurocard). A ALTURA é
// implícita e comum a todo módulo: 128,5 mm (o painel 3U). `y` cresce pra
// baixo. `span` (Display/Slider) também em mm. O renderizador aplica um
// fator único `s` (px/mm) derivado do espaço de tela.
//
// `bind`: id de parâmetro (ex. "rate"), ou porta com prefixo ("in:rate_mod",
// "out:bi"), ou vazio para `Label`. O renderizador resolve contra o
// `manifest()` do módulo.
//
// CONTRATO DE PAINEL (decisões do Rasgo Modular, ver `apps/panel/design.md`):
//  - PROPORÇÃO EURORACK REAL: altura fixa 128,5 mm (3U); só a LARGURA
//    varia (`hp · 5,08 mm`). Módulo de verdade é alto e estreito. Módulo
//    com poucos controles tem espaço vazio - é autêntico e mantém o ritmo.
//  - GRADE INTERNA EM mm (guia, não obrigação): margem ~3 mm; nome no
//    topo (y~2); display y~7, span ~largura-5; knobs ø~10 mm, passo de
//    coluna ~11-13 mm, passo de linha ~17-18 mm; jacks ø~6 mm na faixa
//    inferior (y~102-122), passo ~9 mm. Empilhar na vertical: 4 HP ≈ 1
//    coluna de controles, 8 HP ≈ 2, 12 HP ≈ 3-4.
//  - SEM SOBREPOSIÇÃO ACIDENTAL: os widgets não devem se cruzar; a
//    sobreposição só vale quando o conceito/mecânica pedirem. O painel de
//    teste (`apps/panel/`) avisa no arranque se duas pegadas se cruzam.
//  - controles de performance junto das entradas/saídas que transformam.

namespace rasgo::modular {

struct Widget {
    enum class Kind {
        Knob,      // parâmetro contínuo
        Slider,    // parâmetro contínuo, orientação vertical
        Toggle,    // parâmetro 0/1
        Jack,      // porta de entrada ou saída
        Display,   // leitura (texto/gráfico) - não é controle
        Label,     // texto fixo
    };

    Kind kind = Kind::Label;
    std::string label;
    std::string bind;   // param id | "in:<porta>" | "out:<porta>" | ""
    float x = 0.0f;
    float y = 0.0f;
    float span = 1.0f;  // largura relativa (Display/Slider usam)
};

struct Panel {
    int hp = 8;
    std::vector<Widget> widgets;

    void add(const Widget::Kind kind, std::string label, std::string bind,
             const float x, const float y, const float span = 1.0f) {
        widgets.push_back({kind, std::move(label), std::move(bind), x, y, span});
    }
};

}  // namespace rasgo::modular
