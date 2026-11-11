# TAP — o andamento batido por quem toca

**Família:** TIME · **Módulo 78**
**Essência:** bata o pulso e o patch entra no seu andamento, com o "um" no
lugar.
**Dossiê técnico:** [`../dossies/78_tap.md`](../dossies/78_tap.md)
· **Fonte:** `src/dsp/Tap.hpp`

---

## A ideia

Bata três ou mais vezes no jack TAP (um botão, uma tecla, palmas pelo
`SIGNAL-IN` → `SCOPE.ONS`). O TAP mede o intervalo (a mediana das batidas)
e passa a dar um **CLK** a cada tempo, sozinho, no seu andamento. A
**primeira** batida é o tempo forte, o "um": o **UM** volta a cada COMP
tempos. Se as batidas forem irregulares demais (DISP), o TAP não inventa um
andamento: espera uma contagem melhor. Uma pausa e uma contagem nova trocam
o andamento.

## Como cabear

```
SIGNAL-IN (L) → SCOPE (IN);  SCOPE (ONS) → TAP (TAP)
TAP (CLK) → CLOCK (EXT)        o patch inteiro no seu pulso
TAP (UM)  → SEQUENCE (RST)     a frase começa no seu "um"
```

## Receitas

1. **Contagem antes de tocar.** Bata "1, 2, 3, 4" e comece a tocar: o patch
   já está no andamento.
2. **O compasso de quem toca.** COMP 3 para uma valsa: o UM marca cada
   compasso de três.
3. **Andamento como timbre.** BPM (saída) no FC de um FILTER: quanto mais
   rápido, mais aberto.
