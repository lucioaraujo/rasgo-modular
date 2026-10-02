// Testa a geometria compartilhada do cabo (`src/ui/CableGeometry.hpp`)
// usada tanto pelo desenho (`drawCable`) quanto pelo hit-test do corpo do
// cabo, que abre o inspector de relação/condução/ruptura seletiva
// (ver `RASGO_MODULAR.md §36.9`, `guia/RELACAO_DE_CABO.md`).
//
// Pura geometria 2D — sem X11, sem SignalGraph.

#include "ui/CableGeometry.hpp"

#include <cassert>
#include <cstdio>
#include <vector>

using rasgo::ui::cablePoints;
using rasgo::ui::distanceToPolyline;
using rasgo::ui::pointNearCable;
using rasgo::ui::nearestCable;
using rasgo::ui::CableEnds;

namespace {

int failures = 0;

void expectTrue(bool cond, const char* what) {
    if (!cond) {
        std::fprintf(stderr, "FALHOU: %s\n", what);
        ++failures;
    }
}

// Um ponto sobre a curva (o próprio ponto médio, k=7 de 15) conta
// como hit com um threshold pequeno.
void testHitOnCurve() {
    const auto pts = cablePoints(100.0f, 200.0f, 300.0f, 220.0f);
    const auto mid = pts[7];
    expectTrue(pointNearCable(mid.x, mid.y, 100.0f, 200.0f, 300.0f, 220.0f,
                              2.0f),
              "ponto exatamente sobre a curva deve contar como hit");
}

// Um ponto bem longe da curva (acima dos dois extremos, já que a
// curva sempre "sagga" pra baixo) nunca conta como hit.
void testMissFarAway() {
    expectTrue(!pointNearCable(200.0f, 0.0f, 100.0f, 200.0f, 300.0f, 220.0f,
                               5.0f),
              "ponto longe da curva (acima dos extremos) não deve contar como hit");
}

// O threshold é respeitado: logo abaixo dele conta, logo acima não.
void testThresholdBoundary() {
    const auto pts = cablePoints(0.0f, 0.0f, 100.0f, 0.0f);
    // ponto perpendicular ao segmento inicial, a uma distância conhecida
    const float d = distanceToPolyline(pts[0].x, pts[0].y + 3.0f, pts);
    expectTrue(pointNearCable(pts[0].x, pts[0].y + 3.0f, 0.0f, 0.0f, 100.0f,
                              0.0f, d + 0.5f),
              "threshold um pouco maior que a distância real deve contar como hit");
    expectTrue(!pointNearCable(pts[0].x, pts[0].y + 3.0f, 0.0f, 0.0f, 100.0f,
                               0.0f, d - 0.5f),
              "threshold um pouco menor que a distância real não deve contar como hit");
}

// A curva sempre tem 15 pontos e começa/termina exatamente nos extremos
// pedidos — a mesma garantia que `drawCable` depende pra não desalinhar
// visual/clicável.
void testEndpointsExact() {
    const auto pts = cablePoints(10.0f, 20.0f, 90.0f, 40.0f);
    expectTrue(pts.size() == 15, "cablePoints sempre devolve 15 pontos");
    expectTrue(pts.front().x == 10.0f && pts.front().y == 20.0f,
              "o primeiro ponto é exatamente a origem pedida");
    expectTrue(pts.back().x == 90.0f && pts.back().y == 40.0f,
              "o último ponto é exatamente o destino pedido");
}

// Dois cabos passando perto do mesmo ponto: ganha o MAIS PRÓXIMO, mesmo
// que esteja depois na lista (antes ganhava o primeiro que coubesse na
// tolerância).
void testNearestWins() {
    const std::vector<CableEnds> cs = {
        {0.0f, 0.0f, 200.0f, 0.0f},     // passa ~mais longe do ponto
        {0.0f, 6.0f, 200.0f, 6.0f},     // passa mais perto
    };
    const auto pts = cablePoints(0.0f, 6.0f, 200.0f, 6.0f);
    const auto p = pts[7];                       // sobre o 2º cabo
    expectTrue(nearestCable(p.x, p.y, cs, 20.0f) == 1,
              "com dois cabos na tolerância, o mais próximo ganha");
    expectTrue(nearestCable(p.x, p.y + 500.0f, cs, 20.0f) == -1,
              "longe de todos: nenhum cabo");
}

// A regressão do front-end JUCE: o ponto vem PRIMEIRO. Um ponto sobre o
// cabo tem que acertar; e o jack de ORIGEM de outro cabo, longe da curva,
// não pode — era o que a ordem trocada testava sem querer.
void testPointComesFirst() {
    const std::vector<CableEnds> cs = {{100.0f, 100.0f, 400.0f, 100.0f}};
    const auto mid = cablePoints(100.0f, 100.0f, 400.0f, 100.0f)[7];
    expectTrue(nearestCable(mid.x, mid.y, cs, 4.0f) == 0,
              "ponto sobre o corpo do cabo acerta o cabo");
    expectTrue(nearestCable(250.0f, 0.0f, cs, 4.0f) == -1,
              "ponto acima do cabo, fora da tolerância, não acerta");
}

} // namespace

int main() {
    testNearestWins();
    testPointComesFirst();
    testHitOnCurve();
    testMissFarAway();
    testThresholdBoundary();
    testEndpointsExact();
    if (failures == 0) {
        std::printf("rasgo_modular_cable_geometry_tests: OK\n");
        return 0;
    }
    std::fprintf(stderr, "%d falha(s)\n", failures);
    return 1;
}
