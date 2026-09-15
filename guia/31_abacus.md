# ABACUS — aritmética de CV

**Família:** DECISION · **Módulo 31**
**Essência:** a CV tratada como **número** — soma, resto, bit a bit — mais
um contador binário que gera ritmo e melodia sozinho. E um retificador de
verdade.
**Dossiê técnico:** [`../dossies/31_abacus.md`](../dossies/31_abacus.md)
· **Fonte:** `src/dsp/Abacus.hpp`

---

## A ideia

Contar é a operação mais simples que gera padrão. Um contador binário com
máscara dá ritmos sincopados sem sequenciador (Numeric Repetitor); o
resto (`mod`) dobra uma CV que sobe numa janela — melodia que "gira"; a
quantização a inteiros vira escada. O `ABACUS` junta essas operações num
módulo — e serve de **retificador** quando é só disso que se precisa.

## Por dentro

**Por que contar gera padrão, a ideia Lunetta:** um contador binário
simples, olhado bit a bit, já **é** um gerador de ritmos: o bit menos
significativo alterna a cada tique (a divisão mais rápida); o próximo
bit alterna a cada **dois** tiques (a metade da velocidade); o seguinte
a cada quatro, e assim por diante — cada bit é literalmente um divisor
de clock diferente, todos rodando ao mesmo tempo, de graça, só de
contar. `PATTERN` escolhe **qual** desses bits vira `P1`: um bit baixo
dá um divisor fino (troca rápido), um bit alto dá um divisor grosso
(troca devagar). `P2` faz `XOR` entre `P1` e o bit vizinho — a mesma
operação "só dispara quando exatamente um dos dois está ligado"
explicada no `LOGIC` (#22) — o que produz um padrão **sincopado**, que
não coincide simplesmente com `P1`.

**O que `MATH` faz com `A`/`B`, além de soma/subtração/multiplicação:**
`resto` (módulo) pega uma CV que sobe continuamente (como a rampa do
próprio contador) e a "enrola" de volta pra dentro de uma janela toda
vez que ela sai — o resultado sobe, sobe, sobe, e de repente **volta ao
início** e recomeça a subir, formando um arpejo ascendente que se
repete sem nunca precisar de um novo disparo. As operações **bit a
bit** (AND/OR/XOR/NAND) tratam `A` e `B` não como números contínuos,
mas como **inteiros pequenos** (5 bits): cada bit de `A` é combinado
com o bit correspondente de `B` segundo a mesma lógica booleana do
`LOGIC` (#22), e o resultado — reinterpretado de volta como tensão —
tende a saltar em degraus bruscos e digitais, bem diferente do
resultado suave de uma soma ou subtração comum.

**Sem `A` conectado**, o próprio contador interno vira a fonte — sua
rampa alimenta `MATH`/`QNT`/`RECT` — e é por isso que o `ABACUS`
consegue tocar melodia e ritmo **inteiramente sozinho**, sem nenhuma
entrada, só contando.

## Os jacks, um a um

### Entradas (todas controle)

- **`A`** — a CV principal. Sem cabo, a fonte é a rampa do contador
  interno. **Plugue aqui:** `TURING.cv`, um LFO, `SEQUENCE.pitch`.
- **`B`** — a segunda CV, usada em soma/subtração/multiplicação e nas
  operações bit a bit. **Plugue aqui:** outro LFO, um `DRIFT.a`.
- **`CLK`** (clock) (disparo) — avança o contador. Livre, usa `RATE`.
  **Plugue aqui:** `CLOCK.clock`.
- **`RST`** (reset) (disparo) — zera o contador e a fase interna.

### Saídas (todas controle)

- **`MTH`** (math) — `A ⊕ B` por `OP`. **Plugue em:** `QUANTIZER.cv`,
  `FILTER.cutoff`.
- **`QNT`** (quant) — `A` quantizado em `STEPS` degraus. **Plugue em:**
  `QUANTIZER.cv` → `OSC.1V/O` (o passeio vira escada).
- **`RCT`** (rect) — `A` retificado. **Plugue em:** `VCA.cv` (seguidor
  de amplitude simples), qualquer `_mod` unipolar.
- **`P1`** (gate) — o bit escolhido por `PAT` — um divisor limpo.
- **`P2`** (gate) — `P1 XOR` o bit seguinte — sincopado. **Plugue em:**
  `DRUM.GATE`, `ENVELOPE.gate`.
- **`CRY`** (carry) (gate) — pulsa quando o contador cruza um múltiplo
  de `MOD` — a "virada" do ritmo. **Plugue em:** `TRIGSEQ.fill`,
  `SEQUENCE.reset`.

## Os controles, um a um

**OP** (0–7) — a operação `A ⊕ B`: 0 soma · 1 subtração · 2 multiplicação
· 3 resto (mod `RANGE`) · 4–7 bit a bit (AND/OR/XOR/NAND).

**MOD** (modulus, 2–32) — o módulo do contador — reinicia a cada `MOD`
tiques.

**STEP** (steps, 2–16) — o nº de degraus da quantização em `QNT`.

**RNG** (range, 0,1–4) — a janela de tensão (±`RNG`) usada por
`MATH`/`QNT`/`RECT` e pela rampa do contador.

**RECT** (rect_mode, 0–3) — o modo do retificador: meia-onda + · meia-
onda − · onda completa · sinal.

**CNT** (count_step, −4..4) — quanto o contador soma (ou subtrai) a cada
`CLOCK`.

**PAT** (pattern, 0–1) — qual bit do contador vira `P1`. Bits baixos
trocam rápido (divisor fino); altos, devagar (divisor grosso).

**SLEW** (0–1) — suaviza `MATH` e `QNT`.

**RATE** (0,1–30 Hz) — o relógio interno do contador. Usado só se `CLK`
estiver livre.

## Como cabear

**Melodia + ritmo autônomos (sem entrada):**
```
CLOCK (clock) → ABACUS (CLK)
ABACUS (QNT) → QUANTIZER (cv) → OSC (1V/O)     (a rampa do contador vira escada)
ABACUS (P2)  → ENVELOPE (gate)                 (o ritmo sincopado do contador)
```

**Ritmo sincopado sem sequenciador:**
```
CLOCK (clock) → ABACUS (CLK)     MOD 8, PAT ~0,3
ABACUS (P1) → DRUM #1 (GATE)
ABACUS (P2) → DRUM #2 (GATE)
ABACUS (CRY) → DRUM #3 (GATE)    (a "virada" a cada 8)
```

**Só retificador:** `LFO → A`, `RCT → destino`, `RECT` no modo que você
precisa.

## Potencializar

- **XOR quebra o sinal:** `OP` = XOR (bit a bit) numa CV suave — a saída
  `MATH` vira degraus imprevisíveis mas repetíveis.
- **Resto = melodia que gira:** `OP` = resto numa rampa lenta — a CV
  "dá a volta" numa janela, um arpejo ascendente que reinicia.
- **`PAT` como divisor variável:** um `SEQUENCE` de CV no… não é
  entrada; automatize o `PAT` à mão pra o groove trocar de subdivisão.
- **Contador como forma:** `CARRY` num `HARMONY.advance` — a harmonia
  anda a cada "volta" do contador.

## Se você conhece o Eurorack

Faz o papel do Noise Engineering Numeric Repetitor (contador + máscara →
ritmo) + um retificador clássico (meia/onda-completa) + aritmética
modular. A base é o divisor binário / Gray code (teoria).
