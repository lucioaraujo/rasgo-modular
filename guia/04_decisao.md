# DECISION — acaso domado

**Família:** DECISION · **Módulo 4**
**Essência:** a fonte de **escolha** — decide (gate), sorteia (CV com
forma) e lembra (um laço que trava). Aleatoriedade com distribuição,
correlação e memória.
**Dossiê técnico:** [`../dossies/04_decisao.md`](../dossies/04_decisao.md)
· **Fonte:** `src/dsp/Decision.hpp`

---

## A ideia

Música generativa precisa de uma fonte de escolha: o que muda, quando
muda, quanto muda. Um LFO é previsível; ruído puro é sem forma. O ponto
médio — acaso *domado* — é o que Branches (gate de Bernoulli), Sapèl
(distribuição uniforme→gaussiana) e Marbles (déjà-vu) trouxeram pro
modular. O `DECISION` junta os três num módulo.

## Por dentro

A cada passo (do `TRIG` externo ou do `RATE` interno): o `GATE`
dispara com probabilidade `BIAS` — um "gate de Bernoulli", que é só o
nome técnico pra "joga uma moeda viciada a cada passo, e o gate acende
se der cara" (a mesma ideia da `conductance` de cabo, `RELACAO_DE_CABO.md`
§3, aplicada a um gate discreto em vez de um sinal contínuo). As saídas
`X`/`Y` recebem CVs sorteadas independentes, com a forma da distribuição
dada por `SHAPE` (a mesma ideia do `SPRD` do `NOISE`, #19: 0 uniforme, 1
sino), alcance por `SPREAD`, quantização por `STEPS`, suavização por
`SLEW`.

**Como `DEJAVU` faz "o laço travar sozinho", e por que é diferente do
`LOCK` do `TURING`:** o módulo mantém uma **memória circular** dos
últimos `LOOP` valores sorteados. A cada novo passo, em vez de sempre
sortear algo novo, existe uma probabilidade `DEJAVU` de simplesmente
**reler** o valor que está guardado na mesma posição da memória —
"o que aconteceu aqui da última vez que passei por este ponto do
loop". Baixo, a memória quase nunca é usada (acaso puro, sempre novo);
alto, a mesma sequência de `LOOP` passos tende a se repetir quase
inteira, como um loop que "gruda" sozinho. É uma mecânica diferente do
`LOCK` do `TURING` (#8): lá, cada **bit individual** decide, no momento
em que sai do registrador, se reentra igual ou muda — aqui, é uma
memória de **posições inteiras** numa janela fixa (`LOOP`), relida
como um bloco.

## Os jacks, um a um

### Entradas

- **`TRIG`** (trigger) (controle, disparo) — avança um passo. Presente,
  substitui `RATE`. **Plugue aqui:** `CLOCK.euclid`, `SEQUENCE.eos`,
  `TRIGSEQ.any`.
- **`BIAS`** (bias_mod) (controle) — CV que soma ao knob `BIAS`.
  **Plugue aqui:** um `FUNCTION` lento (a densidade de eventos sobe e
  desce), `DRIFT`.
- **`SPRD`** (spread_mod) (controle) — CV que soma ao knob `SPREAD`.

### Saídas

- **`X`** / **`Y`** (áudio — CV) — duas CVs sorteadas independentes.
  **Plugue em:** `QUANTIZER.cv` → uma melodia aleatória; `FILTER.cutoff`;
  qualquer `_mod`.
- **`GATE`** (controle, gate) — o gate de Bernoulli — dispara com
  probabilidade `BIAS`. **Plugue em:** `ENVELOPE.gate`, `MATTER.HIT`, o
  `advance` de um `HARMONY`, o `LOCK` de um `TURING`.

## Os controles, um a um

**RATE** (0,01–50 Hz) — o relógio interno. Usado só se `TRIG` estiver
livre.

**BIAS** (0–1) — a probabilidade do `GATE` disparar a cada passo. 0
nunca, 1 sempre.

**SPRD** (spread, 0–1) — o alcance de `X`/`Y`. 0 trava perto de 0; 1 usa
±1 inteiro.

**SHAPE** (0–1) — a forma da distribuição: 0 uniforme, 1 sino
(variações pequenas mais frequentes).

**STEPS** (1–32) — quantiza `X`/`Y` em degraus. 1 = contínuo.

**SLEW** (0–1) — suaviza a transição entre valores sorteados.

**DEJA** (dejavu, 0–1) — a probabilidade de reler a memória em vez de
sortear. Alto = o mesmo trecho de `LOOP` passos tende a repetir.

**LOOP** (loop_length, 1–16) — o tamanho da memória circular relida pelo
déjà-vu.

## Como cabear

**Melodia aleatória na escala:**
```
CLOCK (euclid) → DECISION (TRIG)
DECISION (X) → QUANTIZER (cv) → OSC (1V/O)
DECISION (GATE) → ENVELOPE (gate)
```

**Decisão estrutural:**
```
CLOCK (bem lento, via LOGIC.div) → DECISION (TRIG)     BIAS ~0,3
DECISION (GATE) → HARMONY (advance)     (a harmonia muda "às vezes")
```

## Potencializar

- **Loop que trava sozinho:** suba `DEJA` aos poucos com `LOOP` curto —
  o patch começa a repetir frases, como um loop que se forma.
- **Densidade que respira:** `FUNCTION` bem lento no `BIAS` — trechos
  cheios de eventos e trechos quase parados.
- **X e Y como par:** `X → altura`, `Y → timbre` — a nota e a cor mudam
  no mesmo passo mas de forma independente.
- **Cruze com o `DRIFT`:** `DRIFT.a → SPRD` — o alcance do acaso deriva
  ao longo de minutos.

## Se você conhece o Eurorack

Junta o Mutable Branches (Bernoulli), o Sapèl (distribuição), e o déjà-vu
do Marbles, num só. Distinto do `TURING` (registrador de bits — a
memória é o próprio conteúdo) e do `CHAOS` (EDO não-linear, não sorteio).
