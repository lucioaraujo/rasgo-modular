# LOGIC — recombinador de tempo

**Família:** TIME · **Módulo 22**
**Essência:** dois clocks em relação (3 contra 4), um gate que só passa
quando outro também está alto, uma linha na metade do andamento, um
flip-flop. A álgebra do ritmo.
**Dossiê técnico:** [`../dossies/22_logic.md`](../dossies/22_logic.md)
· **Fonte:** `src/dsp/Logic.hpp`

---

## A ideia

Ritmo generativo interessante quase nunca é um clock reto: é **dois**
clocks em relação, um gate condicional, uma linha na metade do andamento,
um acento que inverte a cada compasso. O `CLOCK` gera **um** fluxo, o
`SEQUENCE` lê **um** padrão. O `LOGIC` fica entre as fontes de clock e os
destinos de gate e recombina.

## Por dentro

**`DIVIDE`, o que é um "contador módulo-N":** o módulo conta as bordas
de `CLK` que chegam (1, 2, 3…) e, quando a contagem chega em `DIV`, ele
emite **um** pulso e recomeça a contar do zero — daí "1 vez a cada N".
É a mesma ideia de um metrônomo que só faz "clique" a cada 4 batidas em
vez de toda batida.

**`MULTIPLY`, como fazer o contrário sem um clock mais rápido de
fora:** o módulo **mede** quanto tempo passou entre a última borda de
`CLK` e a anterior (o "período") e, sabendo esse período, **agenda**
sub-pulsos igualmente espaçados dentro dele — se o período medido foi
de 500 ms e `MULT`=4, ele insere pulsos a cada 125 ms até a próxima
borda real chegar. Isso só funciona **depois** de medir pelo menos um
período — por isso o primeiro ciclo logo ao ligar ainda não tem
sub-pulsos (não há nada medido ainda pra basear neles).

**O que AND/OR/XOR significam, pra quem não conhece lógica booleana:**
os três comparam dois gates (`A`/`B`), instante a instante, decidindo
"passa" (1) ou "não passa" (0):

- **`AND`** só deixa passar quando **os dois** estão ligados ao mesmo
  tempo — é uma condição, "só toca se A **e** B coincidirem agora".
- **`OR`** deixa passar se **qualquer um** dos dois estiver ligado — a
  soma lógica: acontece sempre que pelo menos uma fonte dispara.
- **`XOR`** deixa passar quando **exatamente um** dos dois está ligado
  — nunca quando os dois coincidem. É por isso que serve pra
  sincopação: nos instantes em que as duas fontes "bateriam juntas",
  o `XOR` justamente **cala** — ele ativamente evita a coincidência,
  produzindo um ritmo picotado onde as fontes se revezam.

**`FLIP`, o flip-flop:** é literalmente um interruptor de luz — cada
vez que uma borda de subida chega em `A`, o estado **vira** (0→1 ou
1→0), e fica assim até a próxima borda, não importa quanto tempo passe.
Alimentado por um clock regular, a saída forma uma onda quadrada na
metade da frequência (2 bordas de `A` = 1 ciclo completo de `FLIP`) —
mas alimentado por gates irregulares, `FLIP` vira uma forma de
**alternância** genérica (ora um destino, ora outro), independente de
qualquer contagem.

## Os jacks, um a um

### Entradas (todas controle)

- **`CLK`** (clock) — o clock a dividir/multiplicar. Conectado,
  substitui o relógio interno. **Plugue aqui:** `CLOCK.clock`.
- **`A`** — a entrada A da lógica e do flip-flop. **Plugue aqui:**
  `SEQUENCE.gate`, `CLOCK.euclid`, `TURING.pulse`, `TRIGSEQ.t1`.
- **`B`** — a entrada B da lógica AND/OR/XOR. **Plugue aqui:** um segundo
  gate — `CLOCK.accent`, outro `TRIGSEQ`, um `DECISION.gate`.
- **`RST`** (reset) — zera o contador de divisão e o flip-flop.

### Saídas (todas controle, gate)

- **`DIV`** — o pulso do divisor/multiplicador, com `DELAY` e `GATE`
  aplicados. **Plugue em:** o `clock` de um `SEQUENCE`, `ENVELOPE.gate`.
- **`AND`** — `A AND B`: passa só quando os dois estão altos. **Plugue
  em:** `ENVELOPE.gate` — ritmo condicional.
- **`OR`** — `A OR B`: qualquer um dos dois.
- **`XOR`** — `A XOR B`: um ou o outro, nunca os dois — sincopação.
- **`FLIP`** — alterna a cada borda de `A`. **Plugue em:** o `addr` de um
  `SWITCH` (alterna dois destinos a cada batida), uma chave qualquer.

## Os controles, um a um

**RATE** (0,1–40 Hz) — o relógio interno. Usado só quando `CLK` está
livre — o `LOGIC` sozinho vira um mini-clock divisor.

**DIV** (divide, 1–32) — o contador módulo-N.

**MULT** (multiply, 1–8) — os sub-tiques por período de `CLK`.

**GATE** (gate_len, 0,02–0,98) — o *duty* do pulso `DIV`.

**DELAY** (0–1) — o atraso do pulso `DIV` (0–200 ms).

## Como cabear

**Meia velocidade:**
```
CLOCK (clock) → LOGIC (CLK)      DIV = 2
LOGIC (DIV) → SEQUENCE #2 (clock)     (uma segunda linha na metade do tempo)
```

**Ritmo composto:**
```
SEQUENCE (gate) → LOGIC (A)
TURING (pulse) → LOGIC (B)
LOGIC (XOR) → ENVELOPE (gate)     (dispara quando um OU o outro, nunca os dois)
```

**Alternar destinos:**
```
CLOCK (euclid) → LOGIC (A)
LOGIC (FLIP) → SWITCH (addr)     (a voz alterna entre dois caminhos a cada nota)
```

## Potencializar

- **Polirritmia real:** `DIV` de um `LOGIC` alimentando uma voz, `MULT`
  de outro alimentando outra — 2 contra 3 a partir do mesmo clock.
- **Gate condicional:** `AND` de um `TRIGSEQ` com um `DECISION` de baixa
  probabilidade — a nota só toca "às vezes", no compasso.
- **Delay de humanização:** `DELAY` pequeno + um `NOISE.smooth` no… não
  é entrada; use o `DELAY` fixo pra "atrasar" uma linha de percussão
  contra as outras (feel de arrasto).
- **Flip como seção:** `FLIP` num `CLOCK` bem lento → alterna dois
  patches de roteamento a cada N compassos.

## Se você conhece o Eurorack

Faz o papel de um divisor/multiplicador (Pamela's, 4ms QCD) + um módulo
de lógica booleana (Kinks, Doepfer A-166) + um flip-flop, num só. A base
é lógica booleana clássica e a extrapolação de período estilo Pamela's.
