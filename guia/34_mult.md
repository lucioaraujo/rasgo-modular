# MULT — múltiplo processado

**Família:** ROUTE · **Módulo 34**
**Essência:** uma fonte de CV → quatro versões, cada uma com atenuversor
e offset próprios. O distribuidor. Ocioso, um banco de quatro tensões.
**Dossiê técnico:** [`../dossies/34_mult.md`](../dossies/34_mult.md)
· **Fonte:** `src/dsp/Mult.hpp`

---

## A ideia

No grafo digital do Rasgo o *fan-out* já é livre — um cabo sozinho manda
o mesmo sinal pra quantos destinos você quiser (ver
[`CABEAMENTO.md`](CABEAMENTO.md)). Um múltiplo que só copia não faria
nada.

Este **processa cada saída**. A decisão que ele resolve é comum: "esse
LFO vai pro filtro *e* pro VCA, mas com pesos diferentes, e no VCA
invertido". Com cabo puro isso exige um atenuversor em cada ponta. O
`MULT` junta a distribuição e o ajuste num módulo. Sem nada na entrada,
vira um banco de quatro tensões manuais.

## Por dentro

`out_k = slew(scale_k · in + offset_k)` — a mesma fórmula do
atenuversor+offset do `CONTROL` (#21, ver "Por dentro" lá pro porquê de
`scale` negativo inverter), só que aqui repetida **4 vezes** sobre a
**mesma** entrada, cada cópia com seu `scale`/`offset` próprios — daí
"mini-`CONTROL` por tomada". `slew` é compartilhado pelas 4 (suaviza
todas juntas, não individualmente). Sem `IN` conectado, cada saída vira
simplesmente seu `offset` — uma tensão fixa e ajustável, útil como
banco manual de 4 CVs (afinar 4 osciladores na mão, por exemplo). O
modo `DUAL` divide o módulo em **dois** múltiplos independentes de 1→2
(`IN`→O1/O2, `IN2`→O3/O4) em vez de um só de 1→4 — dois LFOs diferentes
espalhados em duas saídas cada, no mesmo painel.

## Os jacks, um a um

### Entradas (controle)

- **`IN`** — a fonte a distribuir. Alimenta as 4 saídas (ou só as duas
  primeiras, no modo `DUAL`). **Plugue aqui:** um LFO, um `ENVELOPE`,
  `DRIFT.a`, `NOISE.smooth`, `SEQUENCE.pitch`.
- **`IN2`** — segunda fonte, usada só no modo `DUAL`: alimenta `O3`/`O4`.

### Saídas (controle)

- **`O1`**–**`O4`** — cada uma entrega `SCALE·entrada + OFFSET`, com
  `SLEW`. **Plugue em:** quatro entradas de parâmetro diferentes
  (`FILTER.FC`, `VCA.cv`, `SHAPE.FCV`, `SPACE.mix_mod`…).

## Os controles, um a um

**DUAL** (chave) — desligado: 1→4 (todas seguem `IN`). Ligado: dois
múltiplos independentes de 1→2 (`IN`→O1/O2, `IN2`→O3/O4).

**SCL1…SCL4** (scale, −2..2) — o atenuversor de cada saída. 1 = passa;
abaixo, atenua; acima, amplifica; **negativo, inverte**. 0 = ignora a
entrada (só o offset).

**OFF1…OFF4** (offset, −1..1) — a tensão constante somada a cada saída.
Sem `IN`, o offset **é** a saída (banco de tensão manual).

**SLEW** (0–1) — suaviza as quatro saídas ao mesmo tempo. 0 = direto.

## Como cabear

**Uma modulação espalhada com pesos:**
```
ENVELOPE (env) → MULT (IN)
MULT (O1) → FILTER (FC)      SCALE1 = +1     (o filtro abre)
MULT (O2) → VCA (cv)         SCALE2 = −0,5   (o volume cede um pouco)
MULT (O3) → SHAPE (FCV)      SCALE3 = +0,7
MULT (O4) → PARAMETRIC (AMT)
```

**Banco de tensão manual (sem entrada):** os quatro `OFFSET` viram
quatro CVs fixas — pra afinar 4 osciladores, dar bias em 4 filtros.

## Potencializar

- **Movimento contrário:** um LFO no `IN`, `O1` pro corte do `FILTER`
  (`SCALE` +1) e `O2` pro nível de um `VCA` (`SCALE` −1) — o filtro abre
  enquanto o volume fecha, num gesto só.
- **Distribuição amarrada:** suba o `SLEW` e a modulação nos 4 destinos
  "gruda" e se move junta.
- **DUAL como dois LFOs:** um LFO em `IN`, um envelope em `IN2` — dois
  moduladores, quatro versões, um painel.

## Se você conhece o Eurorack

Faz o papel de um múltiplo bufferizado (Doepfer A-180, Intellijel Buff
Mult) **mais** um atenuversor + offset por saída (Maths, Serge) **mais**
um *voltage spreader* (Frap Tools). Num rack físico são três peças —
aqui é uma, porque a cópia pura não tem valor no digital. Não tem
`drift`: é utilidade de precisão. Distinto do `CONTROL` (2 canais, mais
funções por canal).
