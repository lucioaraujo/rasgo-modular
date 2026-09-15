# CHAOS — campo caótico de poço duplo

**Família:** MODULATE · **Módulo 36**
**Essência:** uma equação diferencial não-linear de verdade — dois
integradores perseguindo uma força de poço duplo. Genuinamente
imprevisível, mas determinístico por seed.
**Dossiê técnico:** [`../dossies/36_chaos.md`](../dossies/36_chaos.md)
· **Fonte:** `src/dsp/Chaos.hpp`

---

## A ideia

O `DECISION` e o `TURING` **sorteiam** (pseudo-aleatório, mesmo que
estruturado); o `DRIFT` soma ruído filtrado num passeio. O `CHAOS` é
outra coisa: uma EDO não-linear onde dois integradores (`x`, `y`)
perseguem uma força restauradora de **poço duplo** (`x − x³`, estável
perto de `x ≈ −1` e `x ≈ +1`). O sistema orbita um poço, e de vez em
quando **salta** para o outro — de um jeito que você não consegue
prever, mas que se repete idêntico com a mesma seed.

## Por dentro

**O que é um "poço duplo", como imagem física:** imagine uma bolinha
numa superfície com dois vales separados por uma colina — a bolinha
tende a **assentar** no fundo de um dos dois vales (os pontos estáveis,
aqui perto de `x ≈ −1` e `x ≈ +1`) e resiste a subir a colina do meio
(perto de `x = 0`, onde a força a empurra de volta pro vale mais
próximo, não pra frente). A fórmula `x − x³` é exatamente essa força:
perto dos vales ela restaura suavemente; perto do meio, ela é fraca ou
até empurra pra longe do centro — nunca ajuda a bolinha a **atravessar**
sozinha.

**Por que, sem ajuda, o sistema nunca troca de poço:** uma bolinha que
já perdeu energia suficiente pra assentar num vale (o amortecimento,
`DAMPING`, garante essa perda) **não tem energia de sobra** pra subir
de volta a colina e cair no outro lado — ela fica presa ali pra sempre,
de forma perfeitamente previsível. É por isso que existe um **chute
periódico**: a cada ciclo de `RATE`, um empurrão aleatório é somado ao
sistema — às vezes pequeno demais pra fazer diferença, às vezes grande
o bastante pra jogar a bolinha por cima da colina, pro outro vale. Cujo
chute exatamente vai bastar depende, de forma extremamente sensível, de
onde a bolinha estava e com que velocidade — é essa sensibilidade (uma
mudança minúscula na condição inicial leva a um resultado
completamente diferente depois de algum tempo) que caracteriza um
sistema **caótico de verdade**, não só "aleatório": a mesma equação,
com a mesma semente, sempre repete a mesma sequência de saltos —
determinístico e ainda assim impossível de prever de cabeça.

`DRIVE` controla tanto a força que puxa de volta pros vales quanto o
tamanho do chute — mais `DRIVE` significa vales mais "profundos" (mais
difícil escapar) mas também empurrões mais fortes (mais fácil escapar
quando o chute acontece) — o equilíbrio entre os dois decide se o
sistema salta com frequência ou raramente. `DAMPING` alto tira energia
rápido demais pra qualquer chute importar — o sistema assenta num poço
só e quase não se move; baixo deixa energia suficiente sobrar entre os
chutes pra o sistema "balançar largo" e cruzar de um lado pro outro
com mais liberdade.

## Os jacks, um a um

### Entradas

- **`RSD`** (reseed) (controle, disparo) — reposiciona `x`/`y` num ponto
  novo aleatório, sem esperar o sistema escapar sozinho. **Plugue
  aqui:** `CLOCK`, `DECISION.gate`, `SEQUENCE.eos` — força uma "virada"
  no compasso.
- **`RTM`** (rate_mod) (controle) — CV que soma ao knob `RATE`.

### Saída

- **`OUT`** (áudio) — a posição do sistema, sempre em [−1, 1]. Em `RATE`
  baixo é CV; em `RATE` alto é uma textura de áudio. **Plugue em:**
  `FILTER.cutoff`, `OSC.1V/O` (altura errática), `SPACE.time_mod`,
  qualquer `_mod` — ou num canal do `MIXER` (via `VCA`) como textura.

## Os controles, um a um

**RATE** (0,02–400 Hz) — a velocidade da dinâmica. De CV bem lenta a
textura de áudio.

**DRIVE** (0–1) — a força da não-linearidade: quanto mais alto, mais
forte o sistema é puxado entre os dois poços, e maior o chute.

**DAMP** (damping, 0,05–1) — o amortecimento. Alto = assenta num poço só
(quase parado); baixo = oscila largo e "caça" entre os dois de forma
imprevisível.

**FRZ** (freeze, chave) — congela a dinâmica no valor atual.

## Como cabear

**Modulação errática de um parâmetro:**
```
CHAOS (OUT) → FILTER (FC)      RATE ~0,3 Hz, DRIVE ~0,7, DAMP ~0,2
```
O corte "caça" entre duas regiões sem nunca repetir.

**Melodia caótica:**
```
CHAOS (OUT) → QUANTIZER (cv) → OSC (1V/O)
CLOCK (euclid) → CHAOS (RSD)
```
Cada pulso pode ou não fazer o sistema saltar de poço — a melodia tem
"regiões" e transições súbitas.

## Potencializar

- **Textura de áudio:** `RATE` ~100–300 Hz, `OUT → VCA → MIXER` — um
  drone que oscila entre duas cores de forma orgânica.
- **DAMP como suspense:** um `ENVELOPE` no `RTM`? não — automatize o
  `DAMP` com um `FUNCTION` bem lento: o sistema fica "quieto num poço" e
  depois "solta" e caça.
- **Duas escalas de caos:** um `CHAOS` lento na estrutura + um rápido na
  textura.
- **Reseed rítmico:** `TRIGSEQ.t4 → RSD` — as viradas do caos entram no
  groove.

## Se você conhece o Eurorack

Faz o papel de um módulo de caos analógico (Ian Fritz chaos, Nonlinear
Circuits, o modo caótico do Wogglebug). A base é o campo de poço duplo
de Ian Fritz (2007), do `ChaosSources.h` do ANTITOTEM. Distinto do
`DRIFT` (passeio lento com memória) e do `NOISE`/`SH` (sorteio
pseudo-aleatório).
