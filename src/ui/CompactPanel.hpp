#pragma once

// A vista COMPACTA dos painéis (25/11/2026) — as regras que o autor aprovou
// para o plugin VCV em 24/11 ("opte por módulos mais estreitos, sem encolher
// os objetos"; "cada módulo pode ter seu estilo pessoal, mas com certa
// padronização no conjunto"), agora numa fonte só para os front-ends: o app
// JUCE (e o plugin de DAW, que é o mesmo código) e a página do navegador.
// Origem: `RASGO_MODULAR_VCV/src/Casca.hpp` (`tamanhoKnob`,
// `aproximarFileiras`, `estreitar`); lá ela ainda é uma cópia, a trocar por
// esta quando o plugin VCV for regerado.
//
// É uma VISTA, à escolha de quem toca (o botão COMPACTO no cabeçalho; autor:
// "o modo estreito poderia ser opcional"): o módulo continua declarando o
// seu painel em `Signal::panel()`, e isto só o transforma na hora de
// desenhar.
//   1. knobs em três tamanhos: o grande onde o módulo declara (span > 11
//      mm), o MÉDIO (10 mm, em volta do mesmo centro) onde há 15 mm livres
//      em volta, o pequeno (8 mm) nos outros;
//   2. fileiras muito espaçadas (todos os vizinhos a mais de 18 mm) se
//      juntam em volta do próprio centro, no passo padrão (12,7 mm knobs,
//      10,16 mm jacks) ou no que os rótulos pedem;
//   3. o painel estreita por um fator único, até dois vizinhos de uma
//      fileira (objeto ou rótulo) ficarem a 1,5 mm; nunca abaixo de 6 HP nem
//      do que o título pede.
// A MATRIX fica como está: a grade de células tem geometria própria
// (`matrixCellMM`).
//
// Framework-free, como o resto de `src/ui/`.

#include "core/Panel.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace rasgo::ui {

namespace compacto {

using Kind = rasgo::modular::Widget::Kind;

inline float diametro(const rasgo::modular::Widget& w) { return w.span > 1.0f ? w.span : 8.0f; }

inline void centro(const rasgo::modular::Widget& w, float& cx, float& cy) {
    cx = w.x; cy = w.y;
    if (w.kind == Kind::Knob) { const float d = diametro(w); cx += d * 0.5f; cy += d * 0.5f; }
    else if (w.kind == Kind::Slider) { cx += 3.0f; cy += 15.0f; }
    else if (w.kind == Kind::Toggle) { cx += 2.0f; cy += 2.0f; }
}

// 0 pequeno, 1 médio, 2 grande (`tamanhoKnob` do plugin VCV)
inline int tamanhoKnob(const rasgo::modular::Panel& p, const rasgo::modular::Widget& w) {
    if (w.span > 11.0f) return 2;
    float cx, cy;
    centro(w, cx, cy);
    const float W = static_cast<float>(p.hp) * 5.08f;
    if (cx < 7.0f || cx > W - 7.0f) return 0;
    for (const auto& o : p.widgets) {
        if (&o == &w || o.kind == Kind::Label) continue;
        float ox, oy;
        centro(o, ox, oy);
        if (o.kind == Kind::Display) {   // a distância ao retângulo do visor
            const float dw = o.span > 1.0f ? o.span : 16.0f;
            ox = std::clamp(cx, o.x, o.x + dw);
            oy = std::clamp(cy, o.y, o.y + o.height);
        }
        if (std::hypot(ox - cx, oy - cy) < 15.0f) return 0;
    }
    return 1;
}

struct Item { std::size_t i; float cx, cy, meia; bool jack; };

}  // namespace compacto

inline void compactPanel(rasgo::modular::Panel& p, const std::string& type) {
    using namespace compacto;
    if (type == "MATRIX" || p.widgets.empty()) return;
    // 1. os tamanhos saem do layout original
    std::vector<float> larg(p.widgets.size(), 0.0f);
    std::vector<int> tam(p.widgets.size(), 0);
    for (std::size_t k = 0; k < p.widgets.size(); ++k) {
        const auto& w = p.widgets[k];
        if (w.kind == Kind::Knob) {
            tam[k] = tamanhoKnob(p, w);
            larg[k] = tam[k] == 2 ? 12.7f : (tam[k] == 1 ? 10.0f : 8.0f);
        } else if (w.kind == Kind::Jack) larg[k] = 8.4f;
        else if (w.kind == Kind::Slider || w.kind == Kind::Toggle) larg[k] = 6.0f;
    }
    for (std::size_t k = 0; k < p.widgets.size(); ++k) {   // o médio cresce em volta do centro
        auto& w = p.widgets[k];
        if (tam[k] == 1 && !(w.span > 1.0f)) { w.span = 10.0f; w.x -= 1.0f; w.y -= 1.0f; }
    }
    std::vector<Item> it;
    for (std::size_t k = 0; k < p.widgets.size(); ++k) {
        const auto& w = p.widgets[k];
        if (w.kind == Kind::Label || w.kind == Kind::Display) continue;
        float cx, cy;
        centro(w, cx, cy);
        const float rot = 1.55f * static_cast<float>(w.label.size()) + 1.0f;   // ~1,55 mm por letra
        it.push_back({k, cx, cy, 0.5f * std::max(larg[k], rot), w.kind == Kind::Jack});
    }
    const auto mover = [&](Item& e, float ncx) { p.widgets[e.i].x += ncx - e.cx; e.cx = ncx; };
    // 2. fileiras muito espaçadas
    std::vector<char> feito(it.size(), 0);
    for (std::size_t a = 0; a < it.size(); ++a) {
        if (feito[a]) continue;
        std::vector<std::size_t> fila;
        for (std::size_t b = 0; b < it.size(); ++b)
            if (!feito[b] && std::fabs(it[b].cy - it[a].cy) < 4.0f) { fila.push_back(b); feito[b] = 1; }
        if (fila.size() < 2) continue;
        std::sort(fila.begin(), fila.end(), [&](std::size_t x, std::size_t y) { return it[x].cx < it[y].cx; });
        float menor = 1e9f;
        for (std::size_t k = 1; k < fila.size(); ++k) menor = std::min(menor, it[fila[k]].cx - it[fila[k - 1]].cx);
        if (menor <= 18.0f) continue;
        const float c = 0.5f * (it[fila.front()].cx + it[fila.back()].cx);
        std::vector<float> pos(fila.size(), 0.0f);
        for (std::size_t k = 1; k < fila.size(); ++k) {
            const auto& A = it[fila[k - 1]];
            const auto& B = it[fila[k]];
            pos[k] = pos[k - 1] + std::max((A.jack && B.jack) ? 10.16f : 12.7f, A.meia + B.meia + 1.5f);
        }
        const float meio = 0.5f * pos.back();
        for (std::size_t k = 0; k < fila.size(); ++k) mover(it[fila[k]], c - meio + pos[k]);
    }
    // 3. estreitar
    if (it.size() < 2) return;
    float s = 0.0f;
    for (std::size_t a = 0; a < it.size(); ++a)
        for (std::size_t b = a + 1; b < it.size(); ++b) {
            if (std::fabs(it[a].cy - it[b].cy) > 6.0f) continue;
            const float dx = std::fabs(it[a].cx - it[b].cx);
            if (dx >= 0.5f) s = std::max(s, (it[a].meia + it[b].meia + 1.5f) / dx);
        }
    s = std::min(1.0f, std::max(s, 0.3f));
    float xmin = 1e9f, xmax = -1e9f;
    for (const auto& e : it) { xmin = std::min(xmin, e.cx - e.meia); xmax = std::max(xmax, e.cx + e.meia); }
    const float conteudo = (xmax - xmin) * s;
    const int hp = std::max({6, static_cast<int>(std::ceil((conteudo + 6.0f) / 5.08f)), std::min(p.hp, 7)});
    if (hp >= p.hp) return;
    const float W = static_cast<float>(hp) * 5.08f, desloc = (W - conteudo) * 0.5f;
    for (auto& e : it) mover(e, desloc + (e.cx - xmin) * s);
    for (auto& w : p.widgets)
        if (w.kind == Kind::Display) { w.x = 2.5f; w.span = W - 5.0f; }
        else if (w.kind == Kind::Label) w.x = std::min(w.x, W - 4.0f);
    p.hp = hp;
}

}  // namespace rasgo::ui
