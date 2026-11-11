# COMPARADOR — o limiar como acontecimento

**Família:** DECISION · **Módulo 80**
**Essência:** mistura dois sinais com o zero no meio (somar, opor, anular) e
transforma a relação entre eles num gate: quem está acima, e quando cruzam.
**Dossiê técnico:** [`../dossies/80_comparador.md`](../dossies/80_comparador.md)
· **Fonte:** `src/dsp/Comparador.hpp`

---

## A ideia

Os knobs **A** e **B** têm o zero no centro. Para a direita, a entrada soma;
para a esquerda, entra invertida. Com A à direita e B à esquerda, o **MIX**
é a **diferença** entre os dois sinais. Com o mesmo sinal nos dois, ele se
**anula**. **OFS** desloca tudo, e **INV** é o MIX de cabeça para baixo.

O **COMP** é a decisão: o gate fica alto enquanto o MIX está acima do
**LIM**. A **HIST** (histerese) impede que ele tremule quando o sinal tem
ruído perto do limiar. O **CRUZ** pulsa a cada mudança, nas duas bordas.

## Receitas

1. **Dois LFOs que se cruzam viram ritmo.** Dois `FUNCTION` lentos, de
   velocidades diferentes, em A e B; o COMP no GATE de um `ENVELOPE` e o
   CRUZ num `DRUM`.
2. **Disparo por intensidade.** O LVL de um `SCOPE` em A e o LIM no meio:
   quando quem toca passa de um volume, outra voz entra.
3. **A diferença como voz.** Duas vozes quase iguais em A e B (A + 1, B − 1):
   o MIX é só o que uma tem e a outra não.
