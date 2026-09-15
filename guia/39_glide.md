# GLIDE — portamento por nota

**Família:** TRANSFORM · **Módulo 39**
**Essência:** o *slide* — decide **quando** uma nota escorrega até a
outra e quando salta. Entra a CV de altura, sai a CV conduzida.
**Dossiê técnico:** [`../dossies/39_glide.md`](../dossies/39_glide.md)
· **Fonte:** `src/dsp/Glide.hpp`

---

## A ideia

`SEQUENCE → QUANTIZER → OSC` toca notas em degraus secos. Um baixo acid,
uma linha de lead cantada, um TB-303 — todos vivem do **como uma nota
chega na outra**. Sem `GLIDE`, isso exigiria um `slew` sempre ligado
(perde o "salta quando quero"). O `GLIDE` põe a decisão no fluxo: o
*quando escorregar* vem de um gate, tocado pelo próprio sequenciador.

## Por dentro

Um deslize entre o valor atual e o alvo — mas **`TIME` não é uma
duração fixa por nota**, é calibrado **pra uma mudança de uma oitava**.
Isso importa: um portamento de verdade (um dedo escorregando numa
corda, ou um circuito RC de um sintetizador analógico) tem uma
**velocidade** de deslize, não um tempo fixo por salto — pular uma
oitava inteira demora mais que pular um semitom, na mesma velocidade.
Calibrar `TIME` pra "uma oitava" garante isso: um salto de terça
(menor que uma oitava) desliza proporcionalmente mais rápido que um
salto de oitava inteira, com o mesmo knob — em vez de todo salto,
grande ou pequeno, demorar exatamente `TIME` segundos (o que soaria
"mecânico" e desproporcional num salto pequeno).

`FALL` deixa subida e descida **assimétricas** — multiplicando o tempo
da subida por `6^FALL` pra achar o da descida: em `FALL`=−1, a descida
é 6× mais **rápida** que a subida (sobe devagar, desce rápido); em
`FALL`=+1, o oposto. `CURVE` decide **como** a velocidade se comporta
ao longo do deslize: linear mantém velocidade **constante** do início
ao fim (chega exatamente no tempo `TIME`, sem antecipar nem atrasar — o
portamento "reto" de um MS-20/Minimoog); exponencial começa rápido e
vai **desacelerando** conforme se aproxima do alvo (nunca chega
"oficialmente", só encosta bem perto depois de bem mais tempo que
`TIME` — a curva RC clássica, o mesmo tipo de curva do decaimento de
um `ENVELOPE`).

`MODE` decide **quando** o deslize acontece, e não é um detalhe
menor — é a diferença entre três instrumentos diferentes de tocar:
sempre desliza (todo salto escorrega, sem exceção); só com `SLIDE`
(cada passo do sequenciador **decide individualmente** se escorrega ou
salta — o *slide* nota-a-nota do TB-303); legato (a decisão vem do
`GATE`: se a nota nova chega **sem** soltar o gate da anterior — dois
dedos que se revezam numa tecla, por exemplo —, escorrega; um gate que
sobe do zero, marcando um ataque destacado, sempre salta).

## Os jacks, um a um

### Entradas

- **`PITCH`** (áudio — CV de nota) — a altura a conduzir (1 V/oct, ou
  qualquer CV). **Plugue aqui:** `SEQUENCE.pitch`, `QUANTIZER.pitch`,
  `TURING.cv`. Sem cabo, a saída congela no último valor.
- **`SLIDE`** (controle, gate) — habilita o escorregão **no MODE 1**.
  **Plugue aqui:** uma linha do `TRIGSEQ`, uma saída de gate do
  `SEQUENCE`, `TURING` — *slide* generativo, passo a passo.
- **`GATE`** (controle, gate) — o gate da nota, **para o MODE 2
  (legato)**: borda de subida = ataque destacado (salta); sustentado =
  desliza. **Plugue aqui:** o mesmo gate que dispara o `ENVELOPE`.

### Saídas

- **`OUT`** (áudio — CV conduzida) — a altura já com o glide. **Plugue
  em:** `OSC.1V/O`, `MATTER.1V/O`, qualquer entrada de altura — **no
  lugar** do cabo direto do `QUANTIZER`.
- **`MOV`** (moving) (controle, gate) — alto enquanto desliza. **Plugue
  em:** um `VCA.cv` (o volume cede no glide), um `FILTER.cutoff`.
- **`DONE`** (controle, disparo) — um pulso de ~2 ms quando chega ao
  alvo. **Plugue em:** um `ENVELOPE.gate` de acento, um `TRIGSEQ.fill`,
  uma troca de timbre que reage a "chegou".

## Os controles, um a um

**TIME** (0–2 s) — o tempo da subida, pra uma mudança de uma oitava.
0 = salto seco.

**FALL** (−1..1) — assimetria: descida = `TIME · 6^FALL`. −1 = descida
6× mais rápida; +1 = 6× mais lenta; 0 = simétrico.

**CURVE** (0–1) — o formato. 0 = linear (chega no tempo exato); 1 =
exponencial (arrasta na chegada, só encosta depois de ~3·`TIME`).

**MODE** (0–2) — 0 = sempre desliza; 1 = só com `SLIDE` alto (303);
2 = legato (só se o `GATE` segue alto).

## Como cabear

**Baixo acid (o slide do 303):**
```
SEQUENCE (pitch) → GLIDE (PITCH)
SEQUENCE (gate de slide) → GLIDE (SLIDE)     MODE = 1
GLIDE (OUT) → OSC (1V/O)
```

**Lead legato:**
```
QUANTIZER (pitch) → GLIDE (PITCH)
CLOCK (euclid) → GLIDE (GATE)  e  → ENVELOPE (gate)     MODE = 2
GLIDE (OUT) → OSC (1V/O)
```
Notas ligadas escorregam; notas destacadas saltam.

## Potencializar

- **Glide generativo:** `DECISION.gate → GLIDE.SLIDE` (MODE 1) — o
  *slide* acontece por probabilidade, nota a nota.
- **Timbre que reage à chegada:** `DONE → ENVELOPE.gate` de um segundo
  envelope no `cutoff` — um "acento de chegada" em cada nota deslizada.
- **Volume que cede no glide:** `MOV → VCA.cv` invertido (via `CONTROL`)
  — a nota abaixa enquanto desliza e volta ao chegar.
- **Portamento sobre qualquer CV:** não só altura — um `LFO → GLIDE →
  FILTER.cutoff` suaviza saltos de qualquer modulação.

## Se você conhece o Eurorack

Faz o papel de um processador de glide (Bela Gliss, EMW Glide) e do
portamento de um MS-20/Minimoog. A diferença: os **três modos** (sempre
/ slide-gated / legato) põem o *quando* no fluxo, e as saídas `MOV`/
`DONE` deixam o resto do patch reagir ao glide.
