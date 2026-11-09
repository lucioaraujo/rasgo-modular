# PENDULUM — pêndulo duplo

**Família:** MODULATE · **Módulo 70**
**Essência:** um pêndulo pendurado em outro: balanço regular que, com energia, vira caos.
**Dossiê técnico:** [`../dossies/70_pendulum.md`](../dossies/70_pendulum.md)
· **Fonte:** `src/dsp/Pendulum.hpp`

---

## A ideia

Um pêndulo com outro pendurado na ponta. Com pouca energia, balança quase como
um relógio; com muita, o braço de baixo dá voltas e o movimento nunca se
repete. As posições das duas pontas saem como CV; CROSS marca o ritmo do
balanço; FLIP, cada volta inteira — o momento em que o caos aparece. Com AUTO, o
pêndulo nunca para.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **Melodia que balança.** `X2` → `QUANTIZER` → `OSC`; `CROSS` → `ENVELOPE`.
2. **O caos marca a forma.** `FLIP` → `ADVANCE` do `HARMONY` ou `KICK` do
   `SPRING`.
3. **Espaço que gira.** `X1` → pan de um canal do `MIXER`, `Y2` → `MIX` de um
   `SPACE`.
