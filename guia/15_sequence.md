# SEQUENCE — sequenciador de 8 passos

**Família:** TIME · **Módulo 15**
**Essência:** uma frase que **você escreve** — riff, baixo, ostinato — e
o que é generativo é *como ela é relida*: pra frente, invertida,
quicando, vagando.
**Dossiê técnico:** [`../dossies/15_sequence.md`](../dossies/15_sequence.md)
· **Fonte:** `src/dsp/StepSequencer.hpp`

---

## A ideia

O `TURING` faz a sequência que emerge do acaso; o `DECISION` decide passo
a passo. Mas às vezes a peça precisa de uma frase **escrita** — e o que
varia é a **leitura**. O `SEQUENCE` não é um objeto, é uma família de
comportamentos de leitura sobre um padrão de 8 passos: cada passo tem uma
altura (`P1`–`P8`) e um gate ligável (`G1`–`G8`), e `MODE` decide a
direção.

## Por dentro

8 passos. `P_N` (−1..1, escalado por `RANGE` oitavas) é a altura; `G_N`
liga/desliga o gate daquele passo — desligado, o passo **ainda muda a
CV de altura**, só não dispara um novo gate. Combinado com `GLIDE`
alto, isso faz a nota **anterior continuar soando**, escorregando pra
essa nova altura sem um novo ataque — útil pra "prolongar/curvar" uma
nota em vez de repeti-la.

`MODE` decide a **ordem** em que os passos são lidos, mantendo o mesmo
conteúdo escrito: pra frente e pra trás são óbvios; ping-pong vai até o
fim e **volta** pelo mesmo caminho (sem repetir o primeiro/último passo
duas vezes seguidas); aleatório sorteia **qualquer** passo a cada vez,
sem relação com o anterior — pode saltar pra qualquer ponto da frase.
**Browniano é diferente de aleatório**: em vez de saltar pra qualquer
lugar, ele sorteia um passo **vizinho** do atual (um a mais ou um a
menos) — a leitura "vaga" pela frase em passos curtos, então tende a
ficar por perto de onde já estava antes de se afastar, dando uma
sensação de "passeio" contínuo em vez dos saltos abruptos do modo
aleatório puro (a mesma distinção entre um passeio aleatório com
memória de posição e um sorteio independente a cada vez — como o
`brown` do `NOISE`, #19, comparado ao `white`).

## Os jacks, um a um

### Entradas

- **`CLK`** (clock) (controle, disparo) — avança um passo. Presente,
  substitui o `RATE` interno. **Plugue aqui:** `CLOCK.clock`,
  `LOGIC.div`.
- **`RST`** (reset) (controle, disparo) — volta ao passo 0 e reinicia a
  direção. **Plugue aqui:** um `CLOCK` de compasso, o `EOC` de um
  `STAGES`.

### Saídas

- **`PTCH`** (pitch) (áudio — CV de nota) — a altura do passo atual, com
  `GLIDE` aplicado. **Plugue em:** `QUANTIZER.cv` (travar na escala do
  `HARMONY`) → `OSC.1V/O`; direto na altura de qualquer voz.
- **`GATE`** (controle, gate) — o gate do passo atual (se o `G_N` dele
  estiver ligado). **Plugue em:** `ENVELOPE.gate`, `MATTER.HIT`,
  `LPG.strike`.
- **`EOS`** (end-of-sequence) (controle, gate) — um pulso toda vez que a
  sequência volta ao passo 0. **Plugue em:** `HARMONY.advance` (a
  progressão anda a cada volta da frase), `TURING.reset`, `DRIFT.advance`.

## Os controles, um a um

**LEN** (length, 1–8) — quantos dos 8 passos entram no padrão.

**MODE** (0–4) — a direção de leitura: frente / trás / ping-pong /
aleatório / browniano.

**RATE** (0,01–40 Hz) — o relógio interno. Usado só se `CLK` estiver
livre.

**GATE** (gate_len, 0,05–0,95) — o *duty* do gate de cada passo (clock
interno) ou a janela do gate (clock externo).

**GLIDE** (0–1) — portamento entre a altura de um passo e o próximo.

**RANGE** (0–2 oitavas) — quantas oitavas os valores `P1`–`P8` cobrem.

**P1–P8** (−1..1) — a altura de cada passo (escalada por `RANGE`).

**G1–G8** (chaves) — liga/desliga o gate de cada passo.

## Como cabear

**Baixo escrito, leitura generativa:**
```
CLOCK (clock) → SEQUENCE (CLK)
SEQUENCE (pitch) → QUANTIZER (cv) → OSC (1V/O)
SEQUENCE (gate) → ENVELOPE (gate)
SEQUENCE (eos) → HARMONY (advance)     (a progressão muda a cada volta)
```
`MODE` = 4 (browniano) — a frase vaga em torno de si mesma.

## Potencializar

- **A mesma frase, quatro caras:** automatize o `MODE` com um `SEQUENCE`
  de CV ou um `SH` — seções em que o riff toca reto, invertido, quicando.
- **Nota ligada:** `G3` desligado + `GLIDE` alto — o passo 3 "prolonga"
  a nota do passo 2 escorregando.
- **Contraponto:** dois `SEQUENCE` com `LENGTH` diferentes (5 e 8) no
  mesmo clock — os padrões defasam e voltam a cada 40 passos.
- **Progressão a cada volta:** `EOS → HARMONY.advance` — a harmonia
  avança exatamente quando a frase reinicia.

## Se você conhece o Eurorack

Faz o papel de um sequenciador de passos (Metropolix, René, Doepfer
A-155) — mas o foco é nos **5 modos de leitura**, não em ter 16 knobs. A
base é Hexen §119 (o sequenciador como família de comportamentos) e o
modo browniano do Mutable Grids. É o par escrito do `TURING` (#8).
