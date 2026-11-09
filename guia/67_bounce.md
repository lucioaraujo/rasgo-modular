# BOUNCE — bolas que quicam

**Família:** MODULATE · **Módulo 67**
**Essência:** bolas que caem e quicam: o ritmo que acelera sozinho até parar, e recomeça.
**Dossiê técnico:** [`../dossies/67_bounce.md`](../dossies/67_bounce.md)
· **Fonte:** `src/dsp/Bounce.hpp`

---

## A ideia

Solte uma bola: ela quica, cada vez mais baixo e mais rápido, até parar. O
BOUNCE faz isso com até três bolas. Cada quique é um gate; a força do quique é
um acento; a altura de cada bola é uma CV que sobe e desce. ELAST decide quanto
a bola devolve; GRAV, a pressa. Com AUTO, as bolas paradas são relançadas — o
módulo toca para sempre, nunca igual (WIND).

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **O quique.** `BOUNCE.GT` → `DRUM`; `ACC` → nível do `DRUM`.
2. **Três bolas, três vozes.** `H1`, `H2`, `H3` → `PITCH` de três `OSC` via
   `QUANTIZER`: melodias que caem.
3. **Gravidade que respira.** Um LFO lento → `GRAV`: o ritmo estica e encolhe.
4. **Acelerando para o resto.** `GT` → `CLK` de um `ARP` ou `CELL`.
