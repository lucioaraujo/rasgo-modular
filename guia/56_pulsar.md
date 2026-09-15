# PULSAR — síntese pulsar

**Família:** SOURCE · **Módulo 56**
**Essência:** um trem de grãos (pulsaret + silêncio) com **duas
frequências independentes**: uma é a altura, a outra é o timbre — e o
timbre não desafina.
**Dossiê técnico:** [`../dossies/56_pulsar.md`](../dossies/56_pulsar.md)
· **Fonte:** `src/dsp/Pulsar.hpp`

---

## A ideia

A síntese pulsar (Curtis Roads) mora entre a granular e a de formante:
um trem de grãos rápido o bastante pra ter altura, com o **duty** do
grão controlando o timbre **independente da altura**. Você abaixa
`FORMANT` e o som fica mais oco/formântico sem desafinar. É o que dá
aquelas texturas "eletrônicas vivas" de Roads e Xenakis.

## Por dentro

**A ideia central, devagar:** cada disparo lança um **pulsaret** — um
grão curto de som (algumas oscilações dentro de um envelope) — seguido
de um trecho de **silêncio total**, e o par (grão + silêncio) se repete
em loop. Existem então **duas** durações independentes em jogo: quanto
tempo dura o par inteiro (o "período", `p`) e quanto tempo dura só o
grão dentro dele. É essa segunda liberdade — o grão não precisa
preencher o período inteiro — que separa pulsar de um oscilador comum.

**Por que `FREQ` e `FORMANT` não interferem um no outro:** repetir
**qualquer** forma periodicamente, no período `p`, sempre produz um
espectro em **harmônicos exatos de `1/p`** (um pente de frequências
igualmente espaçadas — é uma propriedade matemática de qualquer sinal
periódico, não uma escolha de projeto). Mudar `FREQ` move **onde** esse
pente inteiro fica (a altura). O que muda com `FORMANT` é **a forma do
grão em si**, dentro de cada período — e mudar a forma do grão não move
os dentes do pente (eles continuam nos mesmos múltiplos de `FREQ`), só
muda **quais dentes têm mais ou menos energia** (o "envelope espectral"
por cima do pente, formando picos formânticos). É por isso que `FORMANT`
soa como "abrir/fechar a boca" (mexe no timbre/vogal) sem tocar na
nota — exatamente o mesmo tipo de separação altura×timbre que o
`FORMANT` (#45, um módulo à parte) faz por filtragem, só que aqui é
consequência direta de como o grão é sintetizado.

`MASK` pula pulsarets inteiros por sorteio (probabilidade de "não tocar
desta vez") — é o *masking* de Curtis Roads: em vez de desenhar um
ritmo, você **rareia** um trem cheio subtraindo eventos, e o padrão
resultante nasce do acaso, não de uma grade escrita. `WINDOW` decide a
**forma do envelope** de cada grão — do quase retangular (liga e desliga
abrupto, mais harmônicos altos, mais brilhante/áspero nas bordas) ao
expodec (sobe rápido e cai em curva, mais parecido com um golpe de
percussão).

## Os jacks, um a um

### Entradas

- **`PIT`** (controle, altura) — 1 V/oct somada a `FREQ`. **Plugue
  aqui:** `SEQUENCE.pitch`, `QUANTIZER.pitch`, `TURING.cv`.
- **`FQM`** (controle) — CV (em oitavas) somada a `FORMANT` — move o
  **timbre** sem tocar na altura. **Plugue aqui:** um LFO (um "wah" sem
  filtro), um `ENVELOPE`, `DRIFT`.

### Saídas

- **`L`** / **`R`** (áudio) — canais esquerdo e direito (mono se
  `SPREAD` = 0). Vão para dois canais do `MIXER`, ou `L` para um
  `FILTER` e `R` para um `SPACE`.

## Os controles, um a um

**FREQ** (20–2000 Hz) — a taxa de repetição dos pulsarets = **a altura**.
Abaixe até ~30 Hz e cada pulsaret vira um evento separado (ritmo, não
nota).

**FORMANT** (0,1–8×) — a frequência interna do pulsaret = **o timbre**.
Baixo = oco / formântico; alto = brilhante / nasal. Em ≈1, o pulsaret
preenche o período inteiro (*duty* 100%).

**SHAPE** (0–1) — a forma do pulsaret: 0 = 1 ciclo de seno; até 1 = 2–3
ciclos + um harmônico agudo (pulso mais estreito e rico).

**WINDOW** (0–1) — o envelope do grão: 0 ≈ retangular (transientes
duros, brilhante); 0,4 = Hann (limpo); 1 = expodec (ataque rápido +
cauda — percussivo).

**JITTER** (0–1) — desvio semeado no período e na amplitude de cada
pulsaret — de trem rígido a nuvem irregular. Determinístico.

**MASK** (0–1) — a probabilidade de **pular** um pulsaret. 0 = trem
cheio; 1 = quase tudo silêncio. Rareia a textura sem mudar a altura —
cria padrões rítmicos.

**SPREAD** (0–1) — pulsarets alternados jogados para L/R — largura
estéreo por granulação. 0 = mono.

**LEVEL** (0–1) — a saída, com *softclip* antes.

## Como cabear

**Voz:**
```
SEQUENCE → QUANTIZER → PULSAR (PIT)
PULSAR (L) → FILTER (in) → MIXER (ch1)
FUNCTION (lento) → PULSAR (FQM)       (o timbre varre sem desafinar)
```

**Nuvem rítmica:** `FREQ` ~30–60 Hz, `MASK` ~0,7, `JITTER` ~0,4 — grãos
esparsos e irregulares sobre a mesma nota.

## Potencializar

- **Formante que canta:** um LFO no `FQM` na velocidade de uma vogal —
  o som "fala" sem nenhum filtro de formante.
- **Ritmo por subtração:** um `SEQUENCE` de CV no `MASK` (via `CONTROL`)
  — a densidade dos grãos segue um padrão escrito.
- **Do tom ao ritmo, ao vivo:** um `FUNCTION` bem lento no `PIT` que
  atravesse a fronteira ~30 Hz — a "nota" se desmonta em pulsos e
  volta.
- **Cruze com o `SPACE`:** `L`/`R` do `PULSAR` para um `SPACE` com
  `feedback` alto — a nuvem de grãos vira uma textura contínua.

## Se você conhece o Eurorack

É síntese pulsar de Curtis Roads (*Microsound*, 2001) — o conceito
aparece pouco em hardware. Distinto do `MEMORY` (granular de um
*buffer* — grãos de material gravado) e do `OPERATOR`/`ADDITIVE` (não
têm o silêncio entre grãos que define o *duty*). É fonte autônoma —
soa ao carregar.
