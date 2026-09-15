#pragma once

// Geometria de painel em MILÍMETROS, compartilhada entre os front-ends.
//
// O contrato de `Panel`/`Widget` (`core/Panel.hpp`) é declarativo e em mm:
// origem no canto superior esquerdo do painel, painel de 3U (`kMM3U`) e
// largura `hp * kMMHP`. Cada front-end escolhe sua própria escala px/mm —
// por isso aqui só mora o domínio em mm; a conversão pra pixel fica em
// quem desenha (o painel X11 tem `g_s`/`mmpx`; o app JUCE tem a sua).
//
// Existe pra que o painel de teste X11 (`apps/panel/`) e o front-end de
// produção JUCE (`apps/juce/`) usem a MESMA área de widget — se cada um
// tivesse a sua, o clique de um acertaria onde o outro não desenha.
//
// NÃO confundir com o modelo de `tests/test_panel_layout.cpp`: aquele é
// deliberadamente diferente e mais fino — separa controle e rótulo em
// caixas distintas, com tolerância de 0,3 mm, pra ser um gate de
// ergonomia TIPOGRÁFICA (rótulo colidindo com rótulo do vizinho). Este
// aqui é a área de PEGA (hit-test/pick), grossa de propósito. Unificar os
// dois enfraqueceria o gate; são propósitos diferentes.
//
// Sem dependência de X11/JUCE — só `core/Panel.hpp`.

#include "core/Panel.hpp"

#include <algorithm>
#include <cstddef>

namespace rasgo::ui {

// --- "Eurorack proporcional" (apps/panel/design.md §3.2) ----------------
constexpr float kMMHP = 5.08f;    // mm por HP (0,2 pol — horizontal pitch)
constexpr float kMM3U = 128.5f;   // mm de altura do painel (3U)

inline float panelWidthMM(const int hp) {
    return static_cast<float>(hp) * kMMHP;
}

struct RectMM { float x, y, w, h; };

// Pegada do widget em mm (canto sup-esq do painel na origem). É a área
// que responde ao clique — mais generosa que o desenho, de propósito.
inline RectMM footprintMM(const rasgo::modular::Widget& w) {
    using Kind = rasgo::modular::Widget::Kind;
    const float lbl = static_cast<float>(w.label.size());
    switch (w.kind) {
    case Kind::Knob:   return {w.x - 0.5f, w.y - 1.0f, 10.0f, 15.0f};
    case Kind::Slider: return {w.x - 1.0f, w.y - 1.0f, 10.0f, 36.0f};
    case Kind::Toggle: return {w.x - 0.5f, w.y - 0.5f, 5.0f + lbl * 1.7f, 6.0f};
    // o rótulo do jack (fonte cap ~9px, NÃO escala com o zoom) fica
    // centrado em `w.x`, acima do furo — modelamos a largura dele, senão
    // rótulos de 3+ letras a ~9 mm de distância se tocam
    case Kind::Jack: {
        const float half = std::max(3.0f, lbl * 1.6f);
        return {w.x - half, w.y - 7.0f, 2.0f * half, 13.0f};
    }
    case Kind::Display: return {w.x - 0.5f, w.y - 0.5f,
                                (w.span > 1.0f ? w.span : 16.0f) + 1.0f, 17.5f};
    case Kind::Label:  return {w.x - 0.5f, w.y - 0.5f, 2.0f + lbl * 1.9f, 3.5f};
    }
    return {w.x, w.y, 4.0f, 4.0f};
}

inline bool overlapMM(const RectMM& a, const RectMM& b) {
    return a.x < b.x + b.w && b.x < a.x + a.w
        && a.y < b.y + b.h && b.y < a.y + a.h;
}

} // namespace rasgo::ui
