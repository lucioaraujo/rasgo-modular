# MOUSSE — deformação de um material mole

**Família:** MODULATE · **Módulo 69**
**Essência:** uma espuma que afunda com o golpe e volta devagar: um envelope mole.
**Dossiê técnico:** [`../dossies/69_mousse.md`](../dossies/69_mousse.md)
· **Fonte:** `src/dsp/Mousse.hpp`

---

## A ideia

Aperte uma espuma: parte dela afunda na hora, parte escorre devagar; solte, e
ela volta sem pressa. O MOUSSE é essa espuma. DEPTH é o quanto ela está
afundada — um envelope que não é feito de retas, mas de matéria. STIFF é a
dureza, SLOW o quanto é fluida, RET quantas vezes mais lenta é a volta. Com
FATIGUE, golpes seguidos amolecem a espuma. Sem nada no HIT, ela é tocada
sozinha.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **O brilho que afunda.** `DRUM` → `HIT`; `DEPTH` → `CUT` de um `FILTER`
   (atenuado e invertido): cada golpe escurece e o brilho volta devagar.
2. **Mão apoiada.** O `ENV` do `DYNAMICS` → `PRESS`: a mistura empurra a espuma.
3. **Respiração.** `REST` → `GATE` de um `ENVELOPE`: um som só quando a espuma
   descansa.
