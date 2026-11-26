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

// Margem lateral e centro (26/11/2026; autor: "estipulou margem para as
// laterais dos módulos?", "muitos módulos tem as fileiras de knobs
// descentralizadas"). Cada módulo declarou as posições à mão e nada as
// centrava: 46 dos 105 tinham uma lateral mais de 1,5 mm maior que a outra.
// Aqui o conteúdo (knobs, jacks, sliders, toggles, com os rótulos de texto
// livres) anda junto, num passo só, até as duas sobras ficarem iguais; as
// colunas entre fileiras se mantêm. O visor de largura cheia não anda; um
// painel com visor estreito ou de lado fica como está (geometria própria).
inline constexpr float kMargemLateral = 3.0f;   // mm do corpo do objeto à borda
inline constexpr float kBottomRowMM = 110.0f;   // a última fileira de jacks (G4)

inline void centerPanel(rasgo::modular::Panel& p, const std::string& type) {
    using Kind = rasgo::modular::Widget::Kind;
    if (type == "MATRIX") return;
    const float W = static_cast<float>(p.hp) * 5.08f;
    // o visor de largura cheia sempre no mesmo lugar (autor, 26/11/2026:
    // "verifique os alinhamentos dos displays"): 2,5 mm de cada lado, a
    // 6 mm do topo — cinco módulos tinham 0,6 a 1,3 mm a menos ou 1 mm abaixo
    for (auto& w : p.widgets)
        if (w.kind == Kind::Display && (w.span > 1.0f ? w.span : 16.0f) >= W - 8.0f) {
            w.x = 2.5f; w.span = W - 5.0f; w.y = 6.0f;
        }
    // a margem de BAIXO (autor, 26/11/2026: "não há um padrão de margem em
    // relação à logo"): o bloco de jacks/toggles abaixo do último knob ou
    // slider anda junto, na vertical, até a última fileira ficar em
    // `kBottomRowMM` — até onde houver espaço acima dele (3 mm do knob).
    {
        float limite = 0.0f, ultimo = 0.0f, topo = 1e9f;
        for (const auto& w : p.widgets) {
            if (w.kind == Kind::Knob) limite = std::max(limite, w.y + compacto::diametro(w));
            else if (w.kind == Kind::Slider) limite = std::max(limite, w.y + 34.0f);
            else if (w.kind == Kind::Display) limite = std::max(limite, w.y + w.height);
        }
        for (const auto& w : p.widgets)
            if ((w.kind == Kind::Jack && w.y - 7.0f >= limite) || (w.kind == Kind::Toggle && w.y >= limite)) {
                ultimo = std::max(ultimo, w.kind == Kind::Jack ? w.y : w.y + 4.0f);
                topo = std::min(topo, w.kind == Kind::Jack ? w.y - 7.0f : w.y);
            }
        if (ultimo > 0.0f) {
            float dy = kBottomRowMM - ultimo;
            if (dy < 0.0f) dy = std::max(dy, (limite + 3.0f) - topo);
            if (std::fabs(dy) > 0.25f)
                for (auto& w : p.widgets)
                    if (((w.kind == Kind::Jack && w.y - 7.0f >= limite) || (w.kind == Kind::Toggle && w.y >= limite))
                        || (w.kind == Kind::Label && w.y >= topo - 1.0f && w.y <= ultimo + 4.0f))
                        w.y += dy;
        }
    }
    float a = 1e9f, b = -1e9f;
    for (const auto& w : p.widgets) {
        float x0, x1;
        if (w.kind == Kind::Knob) { x0 = w.x; x1 = w.x + compacto::diametro(w); }
        else if (w.kind == Kind::Jack) { x0 = w.x - 3.2f; x1 = w.x + 3.2f; }
        else if (w.kind == Kind::Slider) { x0 = w.x; x1 = w.x + 6.0f; }
        else if (w.kind == Kind::Toggle) { x0 = w.x; x1 = w.x + 4.0f; }
        else if (w.kind == Kind::Display) {
            if ((w.span > 1.0f ? w.span : 16.0f) < W - 8.0f) return;
            continue;
        } else continue;
        a = std::min(a, x0); b = std::max(b, x1);
    }
    if (!(a < b)) return;
    const float dx = 0.5f * ((W - b) - a);
    if (std::fabs(dx) < 0.25f) return;
    for (auto& w : p.widgets) if (w.kind != Kind::Display) w.x += dx;
}

// Knobs MÉDIOS (10 mm) também na vista NORMAL (26/11/2026; autor: "na versão
// compacta os knobs tem tamanhos diferentes, verifique porque não acontece as
// variações de tamanho no modo normal"; escolheu a regra própria do normal).
// Mede o espaço livre real: uma fileira de knobs cresce INTEIRA para 10 mm
// (em volta dos mesmos centros, rótulos juntos) quando nenhum knob dela,
// já crescido, chega a menos de 1 mm de outro objeto, do rótulo dele, do
// visor ou da borda. A fileira decide junta, para a variação ser regular.
namespace normal {

struct Caixa { float x0, x1, y0, y1; };

inline Caixa caixa(const rasgo::modular::Widget& w, float dKnob = 0.0f) {
    using Kind = rasgo::modular::Widget::Kind;
    const float rot = 0.5f * (1.55f * static_cast<float>(w.label.size()) + 1.0f);
    switch (w.kind) {
    case Kind::Knob: {
        const float d0 = compacto::diametro(w);
        const float cx = w.x + d0 * 0.5f, cy = w.y + d0 * 0.5f;
        const float d = dKnob > 0.0f ? dKnob : d0, m = std::max(d * 0.5f, rot);
        return {cx - m, cx + m, cy - d * 0.5f - 4.0f, cy + d * 0.5f};
    }
    case Kind::Jack: { const float m = std::max(3.2f, rot); return {w.x - m, w.x + m, w.y - 7.0f, w.y + 3.2f}; }
    case Kind::Slider: return {w.x, w.x + 6.0f, w.y, w.y + 34.0f};
    case Kind::Toggle: { const float m = std::max(3.0f, rot); return {w.x + 2.0f - m, w.x + 2.0f + m, w.y, w.y + 8.0f}; }
    case Kind::Display: return {w.x, w.x + (w.span > 1.0f ? w.span : 16.0f), w.y, w.y + w.height};
    default: return {0, 0, 0, 0};
    }
}

inline bool batem(const Caixa& a, const Caixa& b, float folga) {
    return a.x0 < b.x1 + folga && b.x0 < a.x1 + folga && a.y0 < b.y1 + folga && b.y0 < a.y1 + folga;
}

}  // namespace normal

inline void knobSizes(rasgo::modular::Panel& p, const std::string& type) {
    using namespace compacto;
    if (type == "MATRIX") return;
    const float W = static_cast<float>(p.hp) * 5.08f;
    std::vector<std::size_t> knobs;
    for (std::size_t k = 0; k < p.widgets.size(); ++k)
        if (p.widgets[k].kind == Kind::Knob && !(p.widgets[k].span > 1.0f)) knobs.push_back(k);
    std::vector<char> feito(p.widgets.size(), 0);
    for (const std::size_t a : knobs) {
        if (feito[a]) continue;
        const float ya = p.widgets[a].y;
        std::vector<std::size_t> fila;
        for (const std::size_t b : knobs)
            if (!feito[b] && std::fabs(p.widgets[b].y - ya) < 4.0f) { fila.push_back(b); feito[b] = 1; }
        bool cabe = true;
        for (const std::size_t k : fila) {
            const auto nova = normal::caixa(p.widgets[k], 10.0f);
            const auto velha = normal::caixa(p.widgets[k]);
            // longe das laterais (autor: "só cuide para não haver botões e
            // jacks muito nas extremidades"): corpo a 3 mm, rótulo a 2 mm
            const float cx = 0.5f * (velha.x0 + velha.x1);
            if (cx - 5.0f < kMargemLateral || cx + 5.0f > W - kMargemLateral
                || nova.x0 < 2.0f || nova.x1 > W - 2.0f || nova.y0 < 1.0f) { cabe = false; break; }
            for (std::size_t o = 0; o < p.widgets.size() && cabe; ++o) {
                if (o == k || p.widgets[o].kind == Kind::Label) continue;
                const bool irma = std::find(fila.begin(), fila.end(), o) != fila.end();
                const auto outra = irma ? normal::caixa(p.widgets[o], 10.0f) : normal::caixa(p.widgets[o]);
                const auto outraVelha = normal::caixa(p.widgets[o]);
                if (normal::batem(velha, outraVelha, 1.0f)) continue;   // já encostavam: não é o crescer
                if (normal::batem(nova, outra, 1.0f)) cabe = false;
            }
            if (!cabe) break;
        }
        if (!cabe) continue;
        for (const std::size_t k : fila) { auto& w = p.widgets[k]; w.span = 10.0f; w.x -= 1.0f; w.y -= 1.0f; }
    }
}

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
