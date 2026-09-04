#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <sstream>
#include <string>

// Renderizador de painel em TEXTO. Não é uma linguagem visual - é só uma
// prova de que a descrição de `Signal::panel()` fecha (todo widget cai
// numa posição, todo `bind` aponta pra parâmetro/porta real). Serve pra
// teste e pra inspeção rápida no terminal, nada mais.

namespace rasgo::modular {

// Confere que todo `bind` de widget resolve contra o módulo. Devolve ""
// se ok, ou a descrição do primeiro problema.
inline std::string validatePanel(const Signal& node) {
    const Panel p = node.panel();
    for (const Widget& w : p.widgets) {
        if (w.bind.empty()) {
            if (w.kind != Widget::Kind::Label && w.kind != Widget::Kind::Display)
                return "widget sem bind não é Label/Display: " + w.label;
            continue;
        }
        if (w.bind.rfind("in:", 0) == 0) {
            const std::string port = w.bind.substr(3);
            bool found = false;
            for (std::size_t i = 0; i < node.inputCount(); ++i)
                if (node.inputDescriptor(i).name == port) found = true;
            if (!found) return "bind de entrada não existe: " + w.bind;
        } else if (w.bind.rfind("out:", 0) == 0) {
            const std::string port = w.bind.substr(4);
            bool found = false;
            for (std::size_t i = 0; i < node.outputCount(); ++i)
                if (node.outputDescriptor(i).name == port) found = true;
            if (!found) return "bind de saída não existe: " + w.bind;
        } else {
            bool found = false;
            for (const auto& parameter : node.parameters())
                if (parameter.descriptor.id == w.bind) found = true;
            if (!found) return "bind de parâmetro não existe: " + w.bind;
        }
    }
    return "";
}

inline std::string renderAscii(const Signal& node) {
    // Coordenadas do painel em mm (painel 3U = 128,5 mm x hp*5,08 mm).
    // Aqui comprimimos pra caber no terminal: ~2,3 mm/char em x, ~4,5 em y.
    const Panel p = node.panel();
    const double kX = 2.3, kY = 4.5;
    const int width = std::max(20, static_cast<int>(p.hp * 5.08 / kX) + 2);
    int height = 4;
    for (const Widget& w : p.widgets)
        height = std::max(height, static_cast<int>(w.y / kY) + 3);

    std::vector<std::string> canvas(static_cast<std::size_t>(height),
                                    std::string(static_cast<std::size_t>(width), ' '));
    auto place = [&](const int x, const int y, const std::string& text) {
        if (y < 0 || y >= height) return;
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int cx = x + static_cast<int>(i);
            if (cx >= 0 && cx < width)
                canvas[static_cast<std::size_t>(y)][static_cast<std::size_t>(cx)] = text[i];
        }
    };

    for (const Widget& w : p.widgets) {
        const int x = static_cast<int>(w.x / kX);
        const int y = static_cast<int>(w.y / kY);
        switch (w.kind) {
        case Widget::Kind::Knob:   place(x, y, "(" + w.label + ")"); break;
        case Widget::Kind::Slider: place(x, y, "[" + w.label + "]"); break;
        case Widget::Kind::Toggle: place(x, y, "<" + w.label + ">"); break;
        case Widget::Kind::Jack:   place(x, y, "o " + w.label); break;
        case Widget::Kind::Display: place(x, y, "|" + w.label + "...|"); break;
        case Widget::Kind::Label:  place(x, y, w.label); break;
        }
    }

    std::ostringstream out;
    out << '+' << std::string(static_cast<std::size_t>(width), '-') << "+\n";
    for (const auto& row : canvas)
        out << '|' << row << "|\n";
    out << '+' << std::string(static_cast<std::size_t>(width), '-') << "+  ("
        << p.hp << " HP)\n";
    return out.str();
}

}  // namespace rasgo::modular
