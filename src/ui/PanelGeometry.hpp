#pragma once

#include <string>

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
#include <cmath>
#include <vector>

namespace rasgo::ui {

// --- "Eurorack proporcional" (apps/panel/design.md §3.2) ----------------
constexpr float kMMHP = 5.08f;    // mm por HP (0,2 pol — horizontal pitch)
constexpr float kMM3U = 128.5f;   // mm de altura do painel (3U)

inline float panelWidthMM(const int hp) {
    return static_cast<float>(hp) * kMMHP;
}

struct RectMM { float x, y, w, h; };

// Visores redondos (autor, 26/11/2026: "gostaria que o visor do pendulum
// fosse um círculo"): o círculo inscrito no retângulo do visor, centrado.
// Um lugar só para todos os front-ends (o app lê aqui; o navegador recebe
// pelo motor, `"rd"` no painel).
inline bool roundDisplay(const std::string& type) { return type == "PENDULUM"; }

// A área das SAÍDAS (26/11/2026, frente de human polish; convenção do VCV:
// as saídas numa placa de cor contrastante, para achar de relance onde tirar
// o sinal). Uma placa por trecho contínuo de saídas numa fileira (jacks
// `out:` lado a lado, sem entrada no meio), cobrindo o jack e o rótulo de
// cima. Cor: `kOutputPlate` (RGB), canto `kOutputPlateRadius` mm.
inline constexpr unsigned kOutputPlate = 0x3a4252;
inline constexpr float kOutputPlateRadius = 1.5f;

// meia largura mínima da placa em volta de cada saída: a luz fica a 3,4 mm
// do centro do jack com raio 0,8 (`ui/OutputLights.hpp`), ou seja, até 4,2;
// com 4,4 o ponto encostava na borda e o da última saída saía para fora
// (autor, 27/11/2026). 5,4 deixa 1,2 mm de placa em volta da luz.
inline constexpr float kOutputPlateMinHalf = 5.4f;

inline std::vector<RectMM> outputPlates(const rasgo::modular::Panel& p) {
    using Kind = rasgo::modular::Widget::Kind;
    struct J { float x, y, meia; bool out; };
    std::vector<J> js;
    for (const auto& w : p.widgets)
        if (w.kind == Kind::Jack)
            js.push_back({w.x, w.y, std::max(kOutputPlateMinHalf, 0.5f * (1.55f * static_cast<float>(w.label.size()) + 1.0f) + 0.6f),
                          w.bind.rfind("out:", 0) == 0});
    std::vector<RectMM> r;
    std::vector<char> feito(js.size(), 0);
    for (std::size_t a = 0; a < js.size(); ++a) {
        if (feito[a]) continue;
        std::vector<J> fila;
        for (std::size_t b = 0; b < js.size(); ++b)
            if (!feito[b] && std::fabs(js[b].y - js[a].y) < 3.0f) { fila.push_back(js[b]); feito[b] = 1; }
        std::sort(fila.begin(), fila.end(), [](const J& x, const J& y) { return x.x < y.x; });
        for (std::size_t k = 0; k < fila.size();) {
            if (!fila[k].out) { ++k; continue; }
            std::size_t f = k;
            while (f + 1 < fila.size() && fila[f + 1].out) ++f;
            const float x0 = fila[k].x - fila[k].meia, x1 = fila[f].x + fila[f].meia;
            float y = 0.0f;
            for (std::size_t q = k; q <= f; ++q) y += fila[q].y;
            y /= static_cast<float>(f - k + 1);
            r.push_back({x0, y - 8.6f, x1 - x0, 13.4f});
            k = f + 1;
        }
    }
    return r;
}

inline float knobDiameterMM(const rasgo::modular::Widget& w) {
    return w.span > 1.0f ? w.span : 8.0f;
}

// O lugar do wordmark RASGO na base do módulo (26/11/2026): 22 mm de
// largura, na mesma altura em todos; no centro, ou à esquerda/direita se um
// controle de baixo esbarraria. Em mm, para o app e o navegador darem o mesmo
// resultado em qualquer zoom (antes a conta tinha margens em pixels e cada
// front-end decidia diferente). `aspecto` = altura/largura da marca.
// Devolve o x da marca em mm a partir da borda esquerda; < 0 se não cabe.
inline float wordmarkXMM(const rasgo::modular::Panel& p, float aspecto, float largura = 22.0f) {
    using Kind = rasgo::modular::Widget::Kind;
    const float W = panelWidthMM(p.hp);
    if (largura > W - 3.0f) return -1.0f;
    const float topo = kMM3U - 2.5f - largura * aspecto;
    const auto bate = [&](float xa, float xb) {
        for (const auto& w : p.widgets) {
            float cx, meia, fundo;
            if (w.kind == Kind::Jack) { cx = w.x; meia = 3.0f; fundo = w.y + 2.6f; }
            else if (w.kind == Kind::Knob) { const float d = knobDiameterMM(w); cx = w.x + d * 0.5f; meia = d * 0.6f; fundo = w.y + d; }
            else if (w.kind == Kind::Toggle) { cx = w.x + 2.0f; meia = 3.0f; fundo = w.y + 8.0f; }
            else continue;
            if (cx + meia > xa && cx - meia < xb && fundo > topo - 1.0f) return true;
        }
        return false;
    };
    const float c = 0.5f * (W - largura);
    if (!bate(c, c + largura)) return c;
    if (!bate(2.0f, 2.0f + largura)) return 2.0f;
    if (!bate(W - 2.0f - largura, W - 2.0f)) return W - 2.0f - largura;
    return c;
}

inline bool envelopeTimeSlider(const std::string& type, const std::string& bind) {
    return type == "ENVELOPE" && (bind == "attack" || bind == "decay" || bind == "release");
}

inline float logarithmicControlFraction(float value, float minimum, float maximum) {
    if (!(minimum > 0.0f && maximum > minimum)) return 0.0f;
    return std::clamp(std::log(std::clamp(value, minimum, maximum) / minimum)
                        / std::log(maximum / minimum), 0.0f, 1.0f);
}

// Pegada do widget em mm (canto sup-esq do painel na origem). É a área
// que responde ao clique — mais generosa que o desenho, de propósito.
inline RectMM footprintMM(const rasgo::modular::Widget& w) {
    using Kind = rasgo::modular::Widget::Kind;
    const float lbl = static_cast<float>(w.label.size());
    switch (w.kind) {
    case Kind::Knob: {
        const float d = knobDiameterMM(w);
        return {w.x - 0.5f, w.y - 1.0f, d + 2.0f, d + 7.0f};
    }
    case Kind::Slider: return {w.x - 1.0f, w.y - 1.0f, 10.0f, 36.0f};
    case Kind::Toggle:
        if (w.span > 1.0f)
            return {w.x + 2.0f - w.span * 0.5f - 0.5f, w.y - 0.5f,
                    std::max(w.span, 4.0f) + 1.0f, 9.0f};
        return {w.x - 0.5f, w.y - 0.5f, 5.0f + lbl * 1.7f, 6.0f};
    // o rótulo do jack (fonte cap ~9px, NÃO escala com o zoom) fica
    // centrado em `w.x`, acima do furo — modelamos a largura dele, senão
    // rótulos de 3+ letras a ~9 mm de distância se tocam
    case Kind::Jack: {
        const float half = std::max(3.0f, lbl * 1.6f);
        return {w.x - half, w.y - 7.0f, 2.0f * half, 13.0f};
    }
    case Kind::Display: return {w.x - 0.5f, w.y - 0.5f,
                                (w.span > 1.0f ? w.span : 16.0f) + 1.0f, w.height + 1.5f};
    case Kind::Label:  return {w.x - 0.5f, w.y - 0.5f, 2.0f + lbl * 1.9f, 3.5f};
    }
    return {w.x, w.y, 4.0f, 4.0f};
}

inline bool overlapMM(const RectMM& a, const RectMM& b) {
    return a.x < b.x + b.w && b.x < a.x + a.w
        && a.y < b.y + b.h && b.y < a.y + a.h;
}


// MATRIX (#33): a grade 4×4 de ganhos (`g<jk>`) NÃO é desenhada como 16
// knobs minúsculos, e sim como uma matriz de células clicáveis — linha j
// = entrada, coluna k = saída. As células ficam centradas onde os knobs
// estavam declarados, então o `Panel` do módulo continua íntegro: quem
// muda é só o desenho.
//
// Mora aqui, e não num front-end, porque os DOIS precisam concordar: se o
// painel X11 desenhar a célula num lugar e o app JUCE testar o clique em
// outro, o módulo fica intratável num deles. Mesma razão do
// `footprintMM`.
inline RectMM matrixCellMM(const int j, const int k) {
    const float cx = 27.0f + static_cast<float>(k) * 17.0f;
    const float cy = 35.0f + static_cast<float>(j) * 18.0f;
    return {cx - 8.0f, cy - 8.5f, 16.0f, 17.0f};
}

// `true` se o widget é uma das 16 células da matriz — quem desenha a
// grade precisa PULAR esses knobs.
inline bool isMatrixCellBind(const std::string& bind) {
    return bind.size() == 3 && bind[0] == 'g'
        && bind[1] >= '1' && bind[1] <= '4'
        && bind[2] >= '1' && bind[2] <= '4';
}

} // namespace rasgo::ui
