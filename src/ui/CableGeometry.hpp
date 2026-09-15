#pragma once

// Geometria do cabo, compartilhada entre o desenho (`drawCable` em
// `panel_main.cpp`) e o hit-test no corpo do cabo (pra abrir o
// inspector de relação/condução/ruptura). Extraído pra cá pra:
//  - nunca divergir visual/clicável (o mesmo cálculo de pontos serve
//    pros dois usos);
//  - ser testável sem Xlib (`tests/test_cable_geometry.cpp`).
//
// Sem dependência de X11/SignalGraph — pura geometria 2D.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace rasgo::ui {

struct CablePoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Os mesmos 15 pontos da curva bezier quadrática "barriga pra baixo"
// que `drawCable` desenha (`panel_main.cpp`): controle no meio do X,
// abaixo do mais baixo dos dois extremos, por uma folga (`sag`) que
// cresce com a distância horizontal.
inline std::array<CablePoint, 15> cablePoints(float x0, float y0,
                                               float x1, float y1) {
    const float sag = 18.0f + std::fabs(x1 - x0) / 6.0f;
    const float cx = (x0 + x1) / 2.0f;
    const float cy = std::max(y0, y1) + sag;
    std::array<CablePoint, 15> pts{};
    for (int k = 0; k < 15; ++k) {
        const float t = static_cast<float>(k) / 14.0f;
        const float u = 1.0f - t;
        pts[static_cast<std::size_t>(k)] = {
            u * u * x0 + 2.0f * u * t * cx + t * t * x1,
            u * u * y0 + 2.0f * u * t * cy + t * t * y1};
    }
    return pts;
}

// distância mínima de (px,py) a qualquer segmento da polilinha
inline float distanceToPolyline(float px, float py,
                                 const std::array<CablePoint, 15>& pts) {
    float best = 1.0e9f;
    for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
        const float ax = pts[i].x, ay = pts[i].y;
        const float bx = pts[i + 1].x, by = pts[i + 1].y;
        const float abx = bx - ax, aby = by - ay;
        const float len2 = abx * abx + aby * aby;
        float t = len2 > 1.0e-6f
            ? ((px - ax) * abx + (py - ay) * aby) / len2
            : 0.0f;
        t = std::max(0.0f, std::min(1.0f, t));
        const float qx = ax + t * abx, qy = ay + t * aby;
        const float dx = px - qx, dy = py - qy;
        const float d = std::sqrt(dx * dx + dy * dy);
        if (d < best) best = d;
    }
    return best;
}

// true se (px,py) está a `threshold` pixels ou menos do corpo do cabo
// entre (x0,y0) e (x1,y1) — mesmo `threshold` usado pra qualquer jack
// (ver `jackAt` em `panel_main.cpp`).
inline bool pointNearCable(float px, float py, float x0, float y0,
                            float x1, float y1, float threshold) {
    return distanceToPolyline(px, py, cablePoints(x0, y0, x1, y1))
        <= threshold;
}

} // namespace rasgo::ui
