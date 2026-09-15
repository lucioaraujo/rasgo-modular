# SAMPLER — toca-fatias

**Família:** SPACE · **Módulo 48**
**Essência:** o toca-fitas / MPC — grava um trecho ao vivo (ou de
arquivo), pica em fatias, e dispara elas fora de ordem. Varispeed,
repitch, desgaste.
**Dossiê técnico:** [`../dossies/48_sampler.md`](../dossies/48_sampler.md)
· **Fonte:** `src/dsp/Sampler.hpp`

---

## A ideia

Pegar um *break* e picar, tocar um *sample* afinado por um sequenciador,
o *stutter*, o vinil ao contrário — não sai do `MEMORY` (granular) nem do
`LOOPER` (delay). O `SAMPLER` põe o toca-fatias no patch:
`TRIGSEQ.t1 → SAMPLER.TRIG`, `POS` de um `SEQUENCE` browniano, e o break
se recombina. `SIGNAL-IN → SAMPLER.IN` + um `CLOCK → REC` = *resampling*
do que o instrumento toca.

## Por dentro

Um gate `REC` grava o `IN` no buffer (~8 s) enquanto estiver alto; na
descida, o comprimento gravado fica **congelado** — a partir daí é
esse trecho fixo que existe pra tocar, não mais um fluxo ao vivo.
`SLICES` divide esse trecho gravado em N pedaços **iguais** (não corta
nos transientes de verdade — é uma divisão matemática do comprimento
total); a CV `POS` escolhe qual desses N pedaços vai tocar no próximo
`TRIG`, então uma sequência de valores de `POS` recombina a ordem das
fatias livremente, diferente da ordem em que foram gravadas.

**`SPEED` × `REPITCH`, a diferença real entre varispeed e pitch-shift:**
`SPEED` sozinho é **varispeed** — literalmente ler o buffer mais rápido
ou mais devagar, exatamente como acelerar/desacelerar uma fita ou um
disco. Isso muda a **duração** e a **altura** ao mesmo tempo, sempre
juntas (2× mais rápido = uma oitava mais agudo E a metade do tempo —
não dá pra separar um do outro nessa técnica; negativo lê de trás pra
frente). `REPITCH`=1 desacopla os dois: a **velocidade de leitura**
fica livre (controlada por `PIT` como uma transposição pura), mas um
**pitch-shifter** (que recorta e realinha fragmentos do áudio
internamente, técnica diferente de simplesmente variar a velocidade)
compensa a mudança de duração que isso causaria — o resultado toca na
altura pedida, na **mesma duração** que teria em velocidade normal, em
vez de esticar/encolher junto.

`WEAR` aplica, a cada novo disparo, um desgaste **sorteado no
instante** (jitter no ponto de início, redução de taxa, bit-crush) —
determinístico (a mesma seed sempre reproduz o mesmo desgaste), mas
diferente a cada disparo dentro de uma mesma sessão, simulando o
desgaste progressivo e imprevisível de mídia física repetida (fita,
vinil) sem realmente destruir o material gravado.

## Os jacks, um a um

### Entradas

- **`TRIG`** (controle, disparo) — dispara a fatia atual. **Plugue
  aqui:** `TRIGSEQ.t1`, `CLOCK.euclid`, `SEQUENCE.eos`.
- **`IN`** (áudio) — o que gravar (enquanto `REC` estiver alto).
  **Plugue aqui:** `SIGNAL-IN.L`, a soma do `MIXER` (resample do próprio
  patch), uma voz.
- **`REC`** (controle, gate) — enquanto alto, grava `IN`. Na descida,
  congela. **Plugue aqui:** um `CLOCK` de compasso, um gate que você
  segura.
- **`POS`** (controle) — CV (0–1) que escolhe a fatia. **Plugue aqui:**
  `SEQUENCE` browniano, `DECISION.x`, `NOISE.sh`.
- **`PIT`** (pitch) (controle, 1 V/oct) — transposição (via velocidade
  ou pitch-shifter, conforme `REPIT`). **Plugue aqui:**
  `QUANTIZER.pitch`, `SEQUENCE.pitch`.

### Saída

- **`OUT`** (áudio) — a fatia tocada, com de-click nas bordas. Vai ao
  `MIXER`.

## Os controles, um a um

**START** (0–1) — o ponto de partida dentro da fatia selecionada.

**SPEED** (−1..1) — varispeed: ±0,25× a ±4×. Negativo = de trás pra
frente. Sem `REPIT`, a altura acompanha (2× = uma oitava acima).

**SLICE** (slices, 1–16) — divide o buffer em N fatias iguais. O CV de
`POS` escolhe qual.

**REPIT** (repitch, chave) — 0 = transposição via velocidade; 1 =
velocidade solta, altura por pitch-shifter (duração preservada).

**WEAR** (0–1) — desgaste por disparo: jitter de início + redução de
taxa + bit-crush. Determinístico (semeado no disparo).

**LOOP** (chave) — one-shot ↔ loop da fatia.

## Como cabear

**Chop de um break:**
```
SIGNAL-IN (L) → SAMPLER (IN)
CLOCK (de 1 compasso) → SAMPLER (REC)     (grava 1 compasso)
SLICE = 8
TRIGSEQ (t1) → SAMPLER (TRIG)
SEQUENCE (browniano) → SAMPLER (POS)
SAMPLER (OUT) → MIXER (ch1)
```

**Sample afinado:**
```
SEQUENCE → QUANTIZER → SAMPLER (PIT)     REPIT = 1 (duração fixa)
CLOCK (euclid) → SAMPLER (TRIG)
```

## Potencializar

- **Resample do próprio patch:** `MIXER.L+R → SAMPLER.IN`, `REC` a cada
  N compassos — o instrumento "grava a si mesmo" e toca de volta,
  recombinado.
- **Stutter:** `SLICE` alto + `TRIG` numa rajada rápida (`LOGIC.mult`) +
  `POS` fixo — a mesma fatia repetida em staccato.
- **Vinil ao contrário:** `SPEED` negativo + `LOOP`.
- **Degradação por seção:** `ENVELOPE` ou `DRIFT` no `WEAR` — o sample
  "envelhece" ao longo da peça.

## Se você conhece o Eurorack

Faz o papel de um toca-fatias (Bitbox, Assimil8or, 1010 Blackbox) — o
*chop* do MPC, o varispeed do Akai/E-mu. É um porte do `SlicePlayer` do
Navalha 2 (crédito Glerm Soares / Lúcio Araújo); o pitch-shifter é o
`G09.pitchshift.pd` de Puckette. Distinto do `MEMORY` (grãos
assíncronos), do `LOOPER` (delay) e do `TURNTABLE` (leitura com
inércia).
