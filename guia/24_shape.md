# SHAPE — modelador de timbre

**Família:** TRANSFORM · **Módulo 24**
**Essência:** síntese por distorção da costa oeste — pega um seno pobre e
enriquece com ring-mod, dobras e saturação, numa cadeia com VCA no fim.
**Dossiê técnico:** [`../dossies/24_shape.md`](../dossies/24_shape.md)
· **Fonte:** `src/dsp/Shape.hpp`

---

## A ideia

O Rasgo tem filtro (subtrativo), EQ (`PARAMETRIC`) e o `MATTER`/`STRING`
(físico). Faltava a **síntese por distorção controlada** — o gesto
Buchla/Serge de pegar um seno e enriquecer com dobras (*wavefolding*) e
ring-mod. O `SHAPE` põe isso num módulo, com os controles de caráter
(`symmetry`, `wrap`, `sat`) que a relação de cabo não expõe.

## Por dentro

Uma cadeia em ordem, cada estágio processando o resultado do anterior:

- **ring-mod** (`RING` mistura `IN` com `IN × MOD`) — a mesma
  multiplicação de dois sinais do `RingMod` de cabo
  (`RELACAO_DE_CABO.md` §1.1): bandas soma/diferença, sem tocar em
  filtro nenhum.
- **wavefolder** (`FOLD`) — a mesma reflexão explicada em
  `RELACAO_DE_CABO.md` §1.2 (o sinal "quica" de volta ao passar de um
  limite, em vez de ser cortado), só que aqui a intensidade é um knob
  fixo (ou a CV `FCV`), não um companion dinâmico por cabo. `SYMMETRY`
  desloca **onde** fica o centro dessa dobra — uma dobra perfeitamente
  centrada preserva a simetria da onda (só harmônicos ímpares); deslocar
  o centro quebra essa simetria e introduz harmônicos **pares** (mais
  "buzz"/nasal — a mesma lógica do `BIAS` do `WASP` e do `ODD` do
  `ADDITIVE`).
- **`WRAP`** — uma opção **diferente** de lidar com o sinal passando dos
  limites: em vez de refletir (quicar, como o `FOLD`), ele **corta e
  reentra do outro lado** (como um relógio que passa de 23:59 direto
  pra 00:00, sem "voltar"). O resultado é mais bruto — descontinuidades
  reais na onda, mais parecido com um dente-de-serra sendo violentamente
  recortado do que com uma dobra suave.
- **`SAT`** (`tanh`) — a mesma curva "achata suavemente os picos"
  (§1 de `RELACAO_DE_CABO.md`), aqui usada só pra arredondar o que
  sobrou depois de tudo isso, sem gerar as reflexões nítidas do `FOLD`.
- **`LEVEL`** — um VCA simples no fim da cadeia.

O núcleo não-linear inteiro roda numa taxa **2× mais alta** internamente
(e depois volta pra taxa normal filtrando o excesso) — dobrar/wrapar
gera harmônicos altos o bastante pra criar aliasing se calculados na
taxa normal; calcular a 2× dá mais margem antes de bater no limite de
Nyquist.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a modelar. **Plugue aqui:** `OSC.sine` ou
  `OSC.tri` (o clássico — um seno pobre vira rico), qualquer voz.
- **`MOD`** (áudio) — o segundo sinal do ring-mod. **Plugue aqui:** a
  saída de **outro `OSC`** (ring-mod clássico — sinos, metais), um LFO.
  Misturado por `RING`.
- **`FCV`** (controle) — soma ao knob `FOLD`. **Plugue aqui:**
  `ENVELOPE.env` (a dobra aumenta no ataque), um LFO.

### Saída

- **`OUT`** (áudio) — o sinal pela cadeia `ring → fold → wrap → sat →
  level`. Vai ao `MIXER` (às vezes via `FILTER` pra domar o agudo).

## Os controles, um a um

**RING** (0–1) — mistura `IN` com `IN × MOD`. 0 = passa `IN`; 1 = só o
produto (ring-mod puro).

**FOLD** (0–1) — a quantidade de dobra. O *wavefolder* reflete o sinal
quando ele passa dos limites, criando harmônicos altos. A CV `FCV` soma
aqui.

**SYMMETRY** (−1..1) — desloca o centro da dobra. Fora do zero, adiciona
harmônicos **pares** (som mais "buzz", nasal) em vez de só ímpares.

**WRAP** (0–1) — mistura a dobra triangular (reflete) com *wrap-around*
seco (corta e reentra do outro lado — dente de serra brutal).

**SAT** (0–1) — saturação (`tanh`) depois da dobra. Arredonda os picos.

**LEVEL** (0–1) — ganho de saída (VCA embutido).

**DRIFT** (0–1) — passeio lento e correlacionado na quantidade de dobra.

## Como cabear

**Enriquecer um seno:**
```
OSC (TRI) → SHAPE (IN) → FILTER (in) → MIXER (ch1)
ENVELOPE (env) → SHAPE (FCV)      (a nota "abre" espectralmente)
```

**Ring-mod clássico (sinos):**
```
OSC #1 → SHAPE (IN)
OSC #2 → SHAPE (MOD)             RING alto, FOLD 0
```
Afine os dois em razões não-inteiras.

## Potencializar

- **Fold vivo:** um LFO no `FCV` mais rápido que a nota — a dobra pulsa
  e o timbre "borbulha".
- **Do doce ao brutal:** um `SEQUENCE` de CV no `FOLD` — cada nota com
  uma quantidade de riqueza diferente.
- **`SHAPE` como sujador de barramento:** a soma do `MIXER` → `SHAPE`
  com `SAT` alto e `FOLD` baixo — cola e engorda o patch.
- **Cruze com o `LPG`:** `SHAPE → LPG` com `strike` — o *pluck*
  costa-oeste completo (fold + a curva do vactrol).

## Se você conhece o Eurorack

Faz o papel do Buchla 259/258 "Timbre" (fold + symmetry) e dos Serge Wave
Multipliers, mais um ring-mod de 4 quadrantes, num só módulo. Distinto do
`WASP`/`FILTER` (distorção *com* corte de frequência) e do `CRUSH`
(distorção digital).
