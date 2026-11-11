# LADDER (ESCADA) — o filtro do grave

**Família:** TRANSFORM · **Módulo 74**
**Essência:** o filtro em escada, de transistor a diodo, com a ressonância
afinada e o grave que não some.
**Dossiê técnico:** [`../dossies/74_ladder.md`](../dossies/74_ladder.md)
· **Fonte:** `src/dsp/Ladder.hpp`

---

## A ideia

O `FILTER` é limpo e estável. O baixo subtrativo pede outra coisa: o
grave que afunda quando a ressonância sobe, e o filtro que se comprime e
encorpa quando é empurrado. É o filtro em escada (R. Moog, 1969): quatro
degraus de 6 dB em cascata, e a saída do último devolvida à entrada como
ressonância.

Três coisas o distinguem:

- **DIODO** acopla os degraus. Em 0, os degraus são isolados (a escada de
  transistores: redonda). Em 1, cada degrau carrega o vizinho (a escada de
  diodos: ácida). No meio, há escadas que não existem em circuito.
- **O pico fica afinado no CUT** em qualquer DIODO. Em RESO 1, a escada
  auto-oscila numa senoide afinada, e uma nota em 1 V/oct no FC a toca como
  oscilador. RESO 1 é o limiar calculado para cada escada, então RESO 0,9
  significa a mesma proximidade do limiar em qualquer DIODO.
- **COMP** devolve o grave. Na escada original, o grave cai quando a
  ressonância sobe. Em COMP 1, o ganho no grave volta exatamente a 1; em 0,
  o grave afunda como no original.

DRIVE empurra a entrada contra a saturação dentro do laço, e o nível de
saída é compensado. A saturação roda a 2× para não gerar chiado de
aliasing.

LP4 é a saída de 24 dB/oct. LP2 é a tomada do 2º degrau, de 12 dB/oct:
mais brilhante, com a mesma ressonância.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Como cabear

**O baixo subtrativo:**
```
OSC (SAW)        → LADDER (IN)
ENVELOPE (ENV)   → LADDER (FC)
LADDER (LP4)     → VCA → MIXER
RESO 0,7 · COMP 1 · CUT ~200 Hz
```

## Receitas

1. **Baixo que encorpa sem sumir.** O cabeamento acima. Suba a RESO com
   COMP em 1: o pico canta e o grave fica.
2. **Ácido.** DIODO 1, RESO 0,85, DRIVE 0,6, um `GLIDE` com `slide` na
   altura do OSC e um envelope curto no FC.
3. **A escada que canta.** RESO 1, sem entrada: a nota do `QUANTIZER` no
   FC toca a auto-oscilação como uma senoide.
4. **Do redondo ao ácido.** Um LFO lento (`FUNCTION`) num cabo para DIODO:
   o timbre muda sem mudar de nota.

## Se você conhece o Eurorack

Faz o papel dos filtros em escada (os clones do Moog e os de diodo à 303;
no Cardinal: kocmoc LADR e DIOD, Bidoo lIMbO). A diferença: o
transistor-diodo é um knob contínuo, não uma chave, e o pico fica afinado
no CUT em qualquer posição. O par natural é o `FILTER` (#2), para o que
a escada não faz (passa-altas, passa-banda, formantes).
