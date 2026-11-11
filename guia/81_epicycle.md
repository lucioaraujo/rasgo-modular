# EPICYCLE — círculos dentro de círculos

**Módulo 81** · **Família:** MODULATE
**Dossiê técnico:** [`../dossies/81_epicycle.md`](../dossies/81_epicycle.md)

---

## A ideia

Até 8 círculos: o centro de cada um gira na borda do anterior. A soma é uma série de Fourier desenhada, z(t) = Σ rₖ·e^{i(ωₖt + φₖ)}. Com poucos círculos e RATE lento, é gesto e ritmo; com muitos e no áudio (PTCH em 1 V/oct), é timbre.

- **MODO**, escolhas diferentes e nomeadas (o crivo do Batruc):
  - QUADRADA: velocidades ímpares, raios 1/k;
  - SERRA: velocidades 1, 2, 3…, raios 1/k;
  - FIB-RAIO: raios 1/Fₙ;
  - FIB-VELOC: velocidades Fₙ.
- **DESV:** desafina as velocidades por √(k+2); a figura nunca fecha.
- **Saídas:** X, Y (normalizadas), R, ANG e GT (Y cruzando o zero ao subir).

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.
