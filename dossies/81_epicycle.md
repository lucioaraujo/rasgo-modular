# 81 — EPICYCLE: círculos dentro de círculos

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: o Batruc (epiciclos de Fourier, `GeometryCurves`) — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

Até 8 círculos: o centro de cada um gira na borda do anterior. A soma é uma série de Fourier desenhada, z(t) = Σ rₖ·e^{i(ωₖt + φₖ)}. Com poucos círculos e RATE lento, é gesto e ritmo; com muitos e no áudio (PTCH em 1 V/oct), é timbre.

## Comportamento

- **MODO**, escolhas diferentes e nomeadas (o crivo do Batruc):
  - QUADRADA: velocidades ímpares, raios 1/k;
  - SERRA: velocidades 1, 2, 3…, raios 1/k;
  - FIB-RAIO: raios 1/Fₙ;
  - FIB-VELOC: velocidades Fₙ.
- **DESV:** desafina as velocidades por √(k+2); a figura nunca fecha.
- **Saídas:** X, Y (normalizadas), R, ANG e GT (Y cruzando o zero ao subir).

## Teste (`tests/test_epicycle.cpp`)

- um círculo é um círculo, com |z| = 1;
- a SERRA dá h2/h1 = 0,498;
- a QUADRADA não tem pares e dá h3/h1 = 0,334;
- FIB-RAIO e FIB-VELOC têm componentes diferentes;
- DESV 0 fecha a figura a cada volta, e DESV 1 não.

## Na seed

Entra no rack da seed (regra de 9/11) na família MODULATE. Um papel dedicado
fica para depois da escuta do autor.
