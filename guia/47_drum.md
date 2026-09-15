# DRUM — voz de percussão

**Família:** SOURCE · **Módulo 47**
**Essência:** um gate → um golpe. Bumbo, caixa, tom, prato — o corpo com
*pitch-sweep* e o estalo de ataque, num mapa 808 ↔ 909 ↔ acústico.
**Dossiê técnico:** [`../dossies/47_drum.md`](../dossies/47_drum.md)
· **Fonte:** `src/dsp/Drum.hpp`

---

## A ideia

O `TRIGSEQ` gera a grade rítmica, mas precisa de vozes pra tocar. Montar
um bumbo com `MATTER` + `NOISE` + `ENVELOPE` toda vez é trabalhoso e
ocupa três módulos. O `DRUM` é a voz pronta: `TRIGSEQ.t1 → DRUM.GATE`,
ajusta `TONE`/`DECAY`/`MAP`, e tem um bumbo. Quatro `DRUM` = um kit.
Como `TONE`/`SNAP`/etc aceitam CV, `SEQUENCE → DRUM.PIT` toca uma linha
de toms.

## Por dentro

Três camadas somadas por golpe — a mesma receita por trás de qualquer
síntese de bumbo eletrônico clássico:

- **corpo** — uma senoide (o "tom" grave do tambor) com **envelope de
  altura**: no instante do golpe a frequência **salta pra cima** (até
  6× o valor de `TONE`) e **cai de volta** rapidamente. Por que isso
  soa como um tambor de verdade: uma pele de bumbo real, no impacto,
  fica **esticada** por uma fração de segundo (mais tensa = mais aguda)
  e relaxa de volta à tensão de repouso quase na hora — o pitch-sweep é
  literalmente essa física, simplificada numa curva. É o *pow*
  característico do bumbo 808.
- **estalo** — ruído branco (ver `19_ruido.md`) passado por um filtro
  **passa-alta** (deixa passar só os agudos, corta os graves), com um
  envelope próprio bem mais curto que o do corpo. Isso simula o
  **impacto inicial** — a baqueta/pele batendo, antes do corpo grave
  assentar. O corte desse passa-alta **sobe** com `MAP`: baixo, deixa
  passar mais grave junto (estalo mais surdo, 808); alto, só os agudos
  bem finos passam (estalo mais brilhante, mais próximo de um ataque
  acústico).
- **envelope de amplitude** — controla o volume geral do golpe inteiro
  ao longo do tempo: sobe instantaneamente no gate e cai numa curva
  **exponencial** (perde uma fração constante do volume que resta a
  cada instante — por isso o começo do decaimento é mais perceptível
  que o fim), de ~20 ms (quase um clique) a ~2 s (`DECAY`).

`MAP` também mistura o corpo puro com uma versão dele passada por
`tanh` (a mesma curva "achata suavemente os picos" do §1 de
`RELACAO_DE_CABO.md`) — achatar os picos de uma senoide grave introduz
harmônicos que soam como um **clique** no ataque, a marca registrada do
909 (mais agressivo/definido que o 808, mais "puro"). `DRIVE` satura a
**saída inteira** do mesmo jeito, por cima de tudo — o "crunch" que
aparece quando você empurra o nível.

`DRIFT` humaniza: a cada golpe, um sorteio **semeado no próprio
instante do disparo** varia um pouco a altura/decay/nível — pequeno o
bastante pra não soar "errado", grande o bastante pra tirar a
repetição idêntica de golpe a golpe. Como a semente vem do disparo, os
mesmos gates, na mesma ordem, sempre produzem exatamente a mesma
sequência de variações — humanizado, mas reprodutível.

## Os jacks, um a um

### Entradas

- **`GATE`** (controle, disparo) — um pulso, um golpe. **Plugue aqui:**
  `TRIGSEQ.t1..t4`, `CLOCK.euclid`, `SEQUENCE.eos`, `DECISION.gate`.
- **`ACC`** (controle) — CV de acento: escala o nível *e* o brilho do
  golpe. **Plugue aqui:** `TRIGSEQ.accent` (os golpes acentuados soam
  mais fortes e brilhantes).
- **`PIT`** (controle, altura) — 1 V/oct somada ao knob `TONE`. **Plugue
  aqui:** `SEQUENCE.pitch` (toms afinados numa melodia), `QUANTIZER`.

### Saída

- **`OUT`** (áudio) — a voz (corpo + estalo, saturada por `DRIVE`). Vai
  a um canal do `MIXER` (um por `DRUM`).

## Os controles, um a um

**TONE** (20–1000 Hz) — a altura do corpo. Grave = bumbo; médio =
tom/caixa; agudo = clave. A CV `PIT` soma aqui.

**BEND** (0–1) — a profundidade do envelope de altura. 0 = tonal (fica
em `TONE`); 1 = varredura de bumbo (salta 6× e desce). É o *pitch-sweep*
que dá peso ao transiente.

**DECAY** (0–1) — o tempo de decaimento (~20 ms a ~2 s). Curto =
click/laser; longo = sub que sustenta.

**SNAP** (0–1) — a dose do estalo de ataque: o *thwack* da caixa, o
chiado do chimbal, o click do bumbo.

**MAP** (0–1) — o caráter: 808 → 909 → acústico. Sobe: o corpo ganha
clique, o ruído fica mais agudo, entra um pouco de drive.

**DRIVE** (0–1) — saturação de saída (tanh + makeup) — o crunch do 909.

**ROLL** (0–1) — auto-disparo interno: acima de 0, o próprio `DRUM`
gera seus próprios gates internos, a uma taxa que sobe com o knob
(~2 Hz — um rufo lento e contável — até ~40 Hz — rápido demais pra
contar, vira um zumbido/buzz de tom definido). Em 0, só o `GATE`
externo dispara; nesse caso o módulo fica em silêncio até algo chegar
— é o único jeito de fazer o `DRUM` tocar **sozinho**, sem nenhum
`CLOCK`/`TRIGSEQ` alimentando.

**DRIFT** (0–1) — humanização: cada golpe varia levemente
altura/decay/nível. 0 = golpes idênticos.

## Como cabear

**Um kit:**
```
CLOCK → TRIGSEQ
TRIGSEQ (t1) → DRUM #1 (GATE)   → MIXER (ch1)   (bumbo: TONE grave, BEND alto)
TRIGSEQ (t2) → DRUM #2 (GATE)   → MIXER (ch2)   (caixa: TONE médio, SNAP alto)
TRIGSEQ (t3) → DRUM #3 (GATE)   → MIXER (ch3)   (chimbal: TONE agudo, DECAY curto, MAP alto)
TRIGSEQ (accent) → DRUM #1 (ACC)
```

**Toms melódicos:** `SEQUENCE.pitch → DRUM.PIT`, `SEQUENCE` disparado
pelo mesmo clock que o `GATE`.

## Potencializar

- **Chimbal que abre:** `ENVELOPE.env → DRUM.DECAY` — o gate curto do
  compasso vira um decay longo nos acentos, como abrir o chimbal.
- **Bumbo vivo:** `DRIFT` pequeno + `ROLL` num toque bem baixo — o
  bumbo "arrasta" sutilmente.
- **Fill automático:** um `DECISION` no `GATE` além do `TRIGSEQ` (via
  `LOGIC` OR) — viradas probabilísticas.
- **Cruze com o `RESONATOR`:** `DRUM → RESONATOR.in` — o golpe seco
  excita um banco de modos afinado (tom com corpo de sino).

## Se você conhece o Eurorack

Faz o papel de uma voz de percussão sintética (vpme QD, Erica Pico Drum,
Tiptop). A base é a topologia do TR-808/909 (o *bridged-T* do 808, o
híbrido do 909 — circuitos documentados no DIY). O `MAP` num knob
contínuo (808→909→acústico) é a ideia central. Distinto do
`MATTER`/`STRING` (ressoadores — percussão afinada, anel longo): o `DRUM`
é o golpe seco empacotado.
