# FORMANT — ressoador de vogais

**Família:** TRANSFORM · **Módulo 45**
**Essência:** cinco passa-faixas paralelos nas posições que **definem**
uma vogal — a voz sintética, o *talkbox*, o pad que pronuncia A→E→I→O→U.
E um modo vocoder de 5 bandas.
**Dossiê técnico:** [`../dossies/45_formant.md`](../dossies/45_formant.md)
· **Fonte:** `src/dsp/Formant.hpp`

---

## A ideia

A voz sintética, o pad que fala vogais, o Serge ResEQ tocado como
instrumento — não sai de `OSC → FILTER`. O `FILTER` tem *um* pico; a voz
tem **cinco**, em posições que dizem qual vogal é. O `FORMANT` põe esse
banco no patch: entra áudio, sai áudio, e `VOW`/`SHF` são o gesto de
"falar".

## Por dentro

**O que é um "formante", fisicamente:** quando você fala uma vogal, sua
garganta e boca formam uma câmara com uma certa forma — e essa câmara,
como qualquer cavidade, **ressoa** mais forte em certas frequências,
não importa a altura da sua voz (você consegue cantar "aaa" grave ou
agudo e ainda reconhecer que é "a"). Essas frequências de ressonância
da câmara — os **formantes** — são o que realmente diferencia uma
vogal da outra; a altura da nota é independente disso. Cada vogal tem
um conjunto característico de ~5 formantes (frequência, força, largura
de cada um). O `FORMANT` recria essa câmara com **cinco filtros
passa-faixa em paralelo** (cada um deixa passar só uma fatia estreita
de frequência), ajustados pras posições de uma vogal — e por isso
"colorir" qualquer som rico em harmônicos com esse banco faz ele soar
como se estivesse sendo "pronunciado".

`VOWEL` varre a sequência A→E→I→O→U interpolando **continuamente**
entre as tabelas de formante vizinhas — não são 5 posições discretas,
é um percurso contínuo entre elas, então dá pra parar em qualquer ponto
intermediário (um som "entre A e E", por exemplo). `SHIFT` escala
**todas** as cinco frequências pelo mesmo fator — fisicamente, é como
esticar ou encolher o comprimento da câmara vocal inteira: uma câmara
maior (trato vocal maior, como de uma pessoa adulta grande) ressoa mais
grave; uma menor (criança), mais agudo — sem mudar **qual** vogal está
sendo formada, só o "tamanho" de quem a pronuncia.

`RES` estreita as 5 bandas: larga (baixo `RES`) deixa passar uma fatia
generosa ao redor de cada formante (coloração suave, reconhecível mas
sutil); estreita (`RES` alto) deixa passar só uma fatia bem fina — se o
que entra for rico o bastante (como ruído), cada banda estreita quase
vira uma **senoide própria** (um "canto" nas 5 alturas dos formantes,
em vez de uma coloração).

**O que o modo `VOCODER` embutido faz de diferente:** em vez de usar a
tabela fixa de uma vogal pra decidir a força de cada uma das 5 bandas,
ele **mede**, em tempo real, a energia do sinal em `MOD` (o
modulador — tipicamente uma voz falando) **nas mesmas 5 frequências**
de formante, e usa **essas** medidas pra controlar os ganhos das
bandas — ou seja: a "vogal" que sai não vem mais de um knob, vem do que
está sendo dito no `MOD`, agora. É a mesma ideia central do vocoder
dedicado (`VOCODER`, #60), só que com 5 bandas fixas nas posições de
formante em vez de uma grade genérica — por isso soa mais vocálico e
menos inteligível que o `VOCODER` de verdade (que usa 16–20 bandas).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a colorir. **Plugue aqui:** `OSC.saw`,
  `CHORD`, um pad, `NOISE` (cada vogal vira 5 tons). Precisa ser rico —
  o `FORMANT` só realça o que já está lá.
- **`VOW`** (controle) — soma ao knob `VOWEL`. **Plugue aqui:**
  `SEQUENCE` (uma melodia de vogais sobre um drone), um LFO, `TURING`.
- **`SHF`** (controle) — soma ao knob `SHIFT` (o trato vocal). **Plugue
  aqui:** um `ENVELOPE`, um LFO — a "cabeça" da voz cresce e encolhe.
- **`MOD`** (áudio) — a voz/fala cuja envoltória espectral vai controlar
  as 5 bandas. **Só faz efeito com `VOCODER` > 0.** **Plugue aqui:**
  `SIGNAL-IN`, um `SAMPLER`, um `DRUM`.

### Saída

- **`OUT`** (áudio) — seco + as 5 bandas, misturados por `MIX`. Vai ao
  `MIXER`.

## Os controles, um a um

**VOWEL** (0–1) — a posição na sequência A→E→I→O→U. A CV `VOW` soma aqui.

**SHIFT** (−1..1) — escala todas as frequências de formante (≈ 0,35× a
2,8×). Pra baixo = voz grande/grave; pra cima = pequena/aguda. Muda o
timbre sem mudar a altura da fonte.

**RES** (0–1) — estreita as 5 bandas. 0 = coloração sutil; 1 = bandas que
cantam/apitam (com ruído no `IN`, cada vogal vira 5 tons senoidais).

**VOCODER** (0–1) — mistura para o modo vocoder: os ganhos das 5 bandas
passam a seguir a energia do `MOD` em cada frequência de formante, em vez
da tabela de vogal. 0 = FORMANT clássico; 1 = vocoder de 5 bandas
(grosso, mas vocálico). Sem `MOD` cabeado, não faz efeito.

**MIX** (0–1) — seco ↔ ressoado. 0 = passa-direto.

**DRIFT** (0–1) — cada formante ganha um *wobble* lento e independente
(±3%) — a voz respira. Determinístico.

## Como cabear

**Pad que fala vogais:**
```
CHORD (OUT) → FORMANT (IN) → MIXER (ch1)
SEQUENCE → FORMANT (VOW)          (a melodia de vogais)
```

**Vocoder de vogal:**
```
OSC (SAW) → FORMANT (IN)
SIGNAL-IN (L) → FORMANT (MOD)     VOCODER = 1
```
`VOWEL` escolhe quais 5 frequências vocodar.

## Potencializar

- **Voz que aperta:** `ENVELOPE.env → RES` — a voz "aperta" no ataque de
  cada nota e relaxa.
- **Trato vocal vivo:** um LFO lento no `SHF` — a voz oscila entre
  grande e pequena.
- **Cruze com o `NOISE`:** `NOISE.pink → FORMANT`, `RES` alto, `VOW` de
  um `SEQUENCE` — um coro de tons senoidais que muda de vogal.
- **Do FORMANT ao `VOCODER` (#60):** quando quiser fala **inteligível**
  (16–20 bandas), use o `VOCODER` dedicado — o modo daqui é vocálico e
  grosso de propósito.

## Se você conhece o Eurorack

Faz o papel do Frap Fumana e do 4ms SMR — bancos de formantes paralelos.
A base é a teoria fonte-filtro de Fant (1960) e as tabelas de formante
de vogais cantadas (Csound). Distinto do `PARAMETRIC` (EQ estático em
série), do `FILTER` (um pico) e do `RESONATOR` (modos afinados a uma
série, excitados).
