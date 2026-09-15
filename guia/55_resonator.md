# RESONATOR — banco de modos afinado, excitado de fora

**Família:** TRANSFORM · **Módulo 55**
**Essência:** você bate com o que quiser — um `DRUM` seco vira marimba,
um `NOISE` vira vento numa taça, uma voz vira um coro metálico. Três
saídas que trocam de dominância.
**Dossiê técnico:** [`../dossies/55_resonator.md`](../dossies/55_resonator.md)
· **Fonte:** `src/dsp/Resonator.hpp`

---

## A ideia

Bater numa coisa e ela cantar. O `MATTER` faz isso com um exciter
próprio (é voz fechada). O `RESONATOR` deixa **você escolher o que
bate**. E as saídas `LOW`/`MID`/`HIGH` não são um *crossover* fixo: são
zonas do banco que trocam de dominância conforme `TILT`/`STRUCTURE` —
cabeie `LOW` e `HIGH` em destinos diferentes e varra `TILT` pra ver a
energia migrar (o gesto Three Sisters).

## Por dentro

Até 24 ressoadores de 2 polos afinados a uma série a partir de `FREQ` —
a mesma ideia de "modo de vibração" explicada no `MATTER` (#9, ver "Por
dentro" lá), só que aqui **sem** exciter fechado: você é quem decide o
que bate. `STRUCTURE` estica as razões entre os modos (harmônico ↔
inarmônico, a mesma fórmula do `ADDITIVE`, #42) — corda afinada num
extremo, sino/metal no outro.

**Por que "nunca auto-oscila", diferente do `FILTER`/`WASP`:** cada
ressoador é ajustado pra **sempre perder um pouco de energia** a cada
volta (os pólos ficam abaixo de 1, nunca chegam a 1). Isso significa
que o banco pode **amplificar e prolongar** uma excitação que chega de
fora, mas nunca **criar** energia do nada — sem nenhum `IN`/`STRK`
alimentando, ele eventualmente silencia (ou fica só no ruído interno de
baixo nível que o mantém "arqueando" sutilmente, o canto de uma taça).
É o oposto do `FILTER` com `RESONANCE` perto de 1, que é ajustado
**de propósito** pra poder se sustentar sozinho.

`DAMP` faz os modos **agudos** perderem energia mais rápido que os
graves — é assim que uma corda/barra real se comporta (o atrito do ar e
a própria rigidez do material amortecem vibrações rápidas antes das
lentas), daí o efeito de "ataque brilhante, cauda escura" quando `DAMP`
sobe. `POSITION` é o mesmo pente-sobre-a-excitação do `MATTER`: bater
num nó de um modo específico não o excita.

**Por que `LOW`/`MID`/`HIGH` não são um crossover comum:** um crossover
de verdade cortaria o espectro em três **faixas de frequência fixas**,
não importa o que está soando. Aqui, as três saídas são três **grupos
dos próprios modos afinados do banco** (terço grave, médio e agudo da
série de parciais) — então quando `STRUCTURE`/`FREQ` mudam, o que cai
em cada grupo muda junto, e `TILT` redistribui **quanta energia** cada
grupo recebe (a mesma lógica de "relação entre saídas como gesto" do
`SPREAD` do `FILTER`, #2).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — **o que excita** o banco. **Plugue aqui:** um
  `DRUM` (o golpe seco vira tom com corpo), `NOISE`, uma voz, um
  `SAMPLER`. É o coração do módulo. Sem `IN` e sem `STRK`, um ruído
  interno mantém o banco arqueando.
- **`STRK`** (controle, disparo) — dispara o exciter de ruído interno.
  **Plugue aqui:** `CLOCK.euclid`, `TRIGSEQ`, `DECISION.gate`.
- **`FQM`** (controle, 1 V/oct) — multiplica `FREQ` (afina a série).
  **Plugue aqui:** `QUANTIZER.pitch`, `SEQUENCE.pitch`, `TURING.cv`.

### Saídas (todas áudio)

- **`LOW`** / **`MID`** / **`HIGH`** — o terço grave / médio / agudo dos
  parciais. Cabeie em destinos **diferentes** — `LOW → MIXER`,
  `HIGH → SPACE` — e varra `TILT`: a energia migra entre eles.

## Os controles, um a um

**FREQ** (20–5000 Hz) — a fundamental do banco. A CV `FQM` multiplica.

**STRUCTURE** (0–1) — as razões dos parciais: 0 harmônico (série exata);
até 1 esticado/inarmônico (rigidez de barra, sino, corda grossa).

**PARTIALS** (1–24) — quantos modos no banco. 1 = quase um filtro
passa-faixa muito ressonante.

**DECAY** (0–1) — o tempo de anel (Q). Curto = *pluck*; longo =
arco/drone. Nunca auto-oscila.

**DAMP** (0–1) — amortecimento dos **agudos** no anel — os parciais
altos decaem antes dos graves (corda de verdade: ataque brilhante, cauda
escura).

**TILT** (−1..1) — a inclinação espectral: <0 `LOW` domina; >0 `HIGH`
domina; 0 plano. **Varrer TILT cruza LOW↔HIGH** — a relação entre as
saídas é o processo.

**POSITION** (0–1) — o pente de *pluck*: onde a excitação "bate".
0,5 = só ímpares.

**MIX** (0–1) — seco (a excitação crua) ↔ ressoado. 0 = bypass exato.

## Como cabear

**Percussão afinada com corpo:**
```
TRIGSEQ (t1) → DRUM (GATE)
DRUM (OUT) → RESONATOR (IN)
SEQUENCE → QUANTIZER → RESONATOR (FQM)
RESONATOR (MID) → MIXER (ch1)
```

**Espectro que cruza:**
```
RESONATOR (LOW) → MIXER (ch1)   (pan esquerda)
RESONATOR (HIGH) → SPACE (in) → MIXER (ch2)   (pan direita)
FUNCTION (lento) → RESONATOR (via um CONTROL somando ao TILT)
```

## Potencializar

- **Taça que canta:** sem `IN`, `DECAY` alto, `STRUCTURE` médio — o
  ruído interno já arqueia; `FQM` de um `SEQUENCE` bem lento move a nota.
- **Marimba:** `DRUM` seco no `IN`, `DECAY` curto, `DAMP` médio,
  `STRUCTURE` baixo.
- **Coro metálico:** uma voz no `IN`, `STRUCTURE` alto, `PARTIALS` 24,
  `MIX` ~0,7.
- **Cruze com o `STRING`:** `STRING → RESONATOR.IN` — corda *dentro* de
  um sino.

## Se você conhece o Eurorack

Faz o papel de Mutable Rings/Elements no modo ressoador, e do Mannequins
Three Sisters (a relação entre as saídas). A base é o biquad ressonante
de 2 polos e a inarmonicidade de barra/corda rígida (acústica).
Distinto do `MATTER` (voz — exciter embutido), do `FILTER`/`FORMANT`
(bandas não afinadas a uma série).
