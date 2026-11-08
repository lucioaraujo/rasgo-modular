# ARP — arpejador

**Família:** TIME · **Módulo 64**
**Essência:** percorre as notas de um acorde, uma por passo, e soa sozinho.
**Dossiê técnico:** [`../dossies/64_arp.md`](../dossies/64_arp.md)
· **Fonte:** `src/dsp/Arp.hpp`

---

## A ideia

Um acorde tocado nota por nota vira melodia: é o arpejo. O ARP escolhe as
notas (um dos 10 formatos do `CHORD`, com inversão e oitavas) e o caminho
por elas (subindo, descendo, indo e voltando, ao acaso, embaralhado, do
centro para fora ou de fora para o centro). Sem nada ligado, ele toca no
seu próprio relógio.

## Por dentro

A cada passo do relógio (o `CLK`, ou o interno em `RATE`), o ARP pega a
próxima posição do caminho escolhido em `MODE` e manda a nota em `PTCH`
e um gate em `GT`. No fim de cada volta, `EOS` dá um pulso. `SKIP` faz
alguns passos ficarem em silêncio — sempre os mesmos para o mesmo patch.

## Os jacks, um a um

### Entradas

- **`CLK`** — um passo por pulso. **Plugue aqui:** `CLOCK.euclid`,
  `TRIGSEQ`, `LOGIC`.
- **`RST`** — volta ao começo do ciclo.
- **`ROOT`** (1 V/oct) — a tônica. **Plugue aqui:** `HARMONY.root` (o
  arpejo anda com a progressão), um `SEQUENCE` lento, um `TURING`.
- **`CHD`** — CV que troca o formato do acorde.

### Saídas

- **`PTCH`** (1 V/oct) — a nota. **Plugue em:** o PITCH de um `OSC`,
  `WAVETABLE`, `STRING`, `OPERATOR`.
- **`GT`** — o gate de cada nota. **Plugue em:** um `ENVELOPE`, o TRIG de
  uma `STRING` ou de um `DRUM`.
- **`EOS`** — o começo de cada volta. **Plugue em:** o `advance` do
  `HARMONY`.

## Os controles, um a um

**CHORD** (0–9) — o formato. **INV** (0–3) — a inversão. **OCT** (1–4) —
quantas oitavas. **MODE** (0–7) — o caminho: sobe, desce, ida e volta, ida
e volta com as pontas repetidas, aleatório, embaralhado, divergir,
convergir. **SKIP** (0–1) — a chance de pausa. **RATE** (0,01–40 Hz) — o
relógio interno. **GATE** (5–95 %) — a duração de cada nota.

## Receitas

1. **Arpejo que soa ao carregar.** `ARP.PTCH` → `OSC.pitch`;
   `ARP.GT` → `ENVELOPE` → `VCA` (o `OSC` no `VCA`) → `MIXER`.
2. **Progressão arpejada.** Um `CLOCK` lento → `HARMONY.advance`;
   `HARMONY.root` → `ARP.ROOT`; `ARP.EOS` também pode avançar o `HARMONY`:
   o acorde muda a cada volta.
3. **Linha improvisada.** MODE 5 (embaralhado), SKIP 0,3, OCT 3, RATE 8:
   parece uma linha tocada, e se repete igual no mesmo patch.
4. **Cordas que respondem.** `ARP.GT` → `STRING.trig`, `ARP.PTCH` →
   `STRING.pitch`, MODE 6 (divergir): o acorde se abre do centro.
