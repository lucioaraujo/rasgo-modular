# TRIGSEQ — grade de trigs de percussão

**Família:** TIME · **Módulo 30**
**Essência:** quatro linhas de gate (bumbo/caixa/chimbal/perc) tocando
juntas. **Não é editor de passos — é gerador:** um ponto no `MAP` é um
estilo, `DENSITY` esculpe.
**Dossiê técnico:** [`../dossies/30_trigseq.md`](../dossies/30_trigseq.md)
· **Fonte:** `src/dsp/TrigSeq.hpp`

---

## A ideia

Percussão precisa de várias linhas em relação rítmica — bumbo firme,
caixa no contratempo, chimbal correndo por baixo. Fazer isso com um
`CLOCK` euclidiano por voz não dá relação entre elas; com um `TURING`, o
padrão é difícil de fixar num groove.

O `TRIGSEQ` dá o **groove como campo**: `MAP` morfa entre 4 caracteres
(reto / quebrado / suingado / esparso), `DENSITY` por linha esculpe,
`CHAOS` humaniza, `DRIFT` faz evoluir. É a peça que transforma um patch
de seed num *beat*, não numa textura. **Soa ao carregar** (identidade
RASGO).

## Por dentro

**O truque central: densidade como limiar sobre um mapa de pesos.**
Cada uma das 4 linhas tem, guardado internamente, um **peso** por
passo (não um "toca"/"não toca" fixo, mas "quão provável é este passo
ser importante nesse estilo de groove"). `DENSITY_N` é um **limiar**
comparado contra esses pesos: só os passos cujo peso ultrapassa o
limiar disparam. Em `DENSITY`=0, só o passo de **maior** peso da linha
passa (o "1" mais óbvio); subindo o limiar, cada vez **mais** passos
cruzam a marca e passam a disparar também — a linha vai de esparsa a
cheia sem nunca reembaralhar a ordem de importância dos passos (é a
mesma lógica de "mapa rítmico" do Mutable Grids: um mapa de pesos fixo,
lido por um limiar variável).

**`MAP`, o que "morfar entre 4 personalidades" quer dizer:** existem 4
mapas de pesos completos guardados (reto, quebrado, suingado, esparso)
— um por "estilo". `MAP` não escolhe um dos quatro, ele **interpola**
continuamente entre os vizinhos, então em qualquer ponto entre 0 e 1
você tem um mapa **misturado** — parte reto, parte quebrado, por
exemplo — e variar `MAP` ao vivo move o groove suavemente de um caráter
pro outro, sem trocas abruptas de padrão.

`SWING` atrasa os passos ímpares dentro do próprio compasso — a mesma
ideia do `SWING` do `CLOCK` (#5). `CHAOS` e `RATCHET` operam em
**dimensões diferentes** do "acontece ou não": `CHAOS` decide, por
probabilidade, se um passo que **deveria** disparar falha, ou se um
passo que **não deveria** dispara mesmo assim (nota-fantasma) — é
imperfeição humana, um passo a mais ou a menos. `RATCHET` não muda
**se** um passo dispara — muda **o que acontece** quando ele dispara:
em vez de um único golpe, o mesmo passo pode virar uma **rajada** de
vários disparos rápidos dentro do mesmo espaço de tempo (um floreio
rítmico, tipo um rufo rápido de baqueta, não um passo extra no padrão).

`FILL` (via gate ou o knob `FILL_AMT`) empurra **todas** as densidades
pra cima temporariamente — uma virada não muda o mapa de pesos nem a
personalidade, só baixa o limiar de todas as linhas de uma vez,
deixando passar passos que normalmente ficariam de fora.

## Os jacks, um a um

### Entradas

- **`CLK`** (clock) (controle, disparo) — avança um passo. Presente,
  substitui `RATE`. **Plugue aqui:** `CLOCK.clock` — pra o beat ficar em
  sincronia com o resto.
- **`RST`** (reset) (controle, disparo) — volta ao passo 0. **Plugue
  aqui:** um `CLOCK` de compasso.
- **`FILL`** (controle, gate) — nível alto empurra as densidades pra
  cima (`FILL_AMT`) — uma virada. **Plugue aqui:** um `DECISION.gate`
  esporádico, um pulso a cada 4 compassos via `LOGIC`, a tecla de um
  pedal (`SIGNAL-IN.gate`).
- **`MAP`** (map_cv) (controle) — CV que soma ao knob `MAP`. **Plugue
  aqui:** um `FUNCTION` bem lento (o groove muda de personalidade ao
  longo da peça), `DRIFT`.

### Saídas (todas gate)

- **`T1`** / **`T2`** / **`T3`** / **`T4`** — os gates das 4 linhas
  (bumbo / caixa / chimbal / perc). **Plugue em:** `DRUM.GATE`,
  `LPG.strike`, `MATTER.HIT`, `ENVELOPE.gate` — uma voz percussiva por
  linha.
- **`ACC`** (accent) — dispara quando **2 ou mais linhas coincidem** no
  mesmo passo. **Plugue em:** `DRUM.ACC`, `VCA.cv` — dá peso aos passos
  "cheios".
- **`ANY`** — o OR das 4 linhas. **Plugue em:** um `SPACE.time_mod` (um
  reverb que "bombeia" com o groove), `SCOPE`, um contador.

## Os controles, um a um

**LEN** (length, 2–16) — quantos dos 16 passos entram no ciclo.

**RATE** (0,1–20 Hz) — o relógio interno. Usado só se `CLK` estiver
livre.

**MAP** (0–1) — morfa entre 4 caracteres: reto (rock/house) → quebrado
(breakbeat) → suingado (hip-hop) → esparso (minimal/dub). Varre ao vivo
e o groove muda de personalidade sem trocar de padrão "na marra".

**DNS1–DNS4** (density, 0–1) — o limiar de disparo de cada linha. Mais
alto, mais passos disparam.

**SWING** (0–1) — atrasa os passos ímpares.

**CHAOS** (0–1) — probabilidade de notas-fantasma aparecerem ou disparos
previstos falharem. Humaniza.

**RATCH** (ratchet, 0–1) — probabilidade de um disparo virar uma rajada
de repetições rápidas.

**FILL** (fill_amt, 0–1) — quanto o `FILL` empurra as densidades pra
cima.

**DRIFT** (0–1) — passeio lento no `MAP` e nas densidades — o groove
evolui sozinho aos poucos.

## Como cabear

**Um beat de 4 vozes:**
```
CLOCK (clock) → TRIGSEQ (CLK)
TRIGSEQ (t1) → DRUM #1 (GATE) → MIXER (ch1)     (bumbo)
TRIGSEQ (t2) → DRUM #2 (GATE) → MIXER (ch2)     (caixa)
TRIGSEQ (t3) → DRUM #3 (GATE) → MIXER (ch3)     (chimbal)
TRIGSEQ (accent) → DRUM #1 (ACC)
```

## Potencializar

- **Groove que evolui:** `DRIFT` ~0,2 + um `FUNCTION` bem lento no
  `MAP` — 4 minutos depois o beat é outro, sem transição brusca.
- **Viradas automáticas:** um `LOGIC.div` (N = 8) no `FILL` — uma virada
  a cada 8 compassos.
- **Reverb que bombeia:** `ANY → SPACE.feedback_mod` invertido (via
  `CONTROL`) — a cauda "respira" com o groove.
- **Chimbal ao vivo:** `T3 → LPG.strike` com o `RESPONSE` do LPG num
  `ENVELOPE` — o chimbal "abre" nos acentos.

## Se você conhece o Eurorack

Faz o papel do Mutable Grids (mapa rítmico + limiar de densidade — o
conceito; as tabelas são próprias) + o acento derivado do TR-808/909 + a
probabilidade/fill de um Pamela's. Distinto do `CLOCK` (uma linha
euclidiana) e do `SEQUENCE` (uma melodia).
