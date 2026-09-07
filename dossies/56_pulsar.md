# Dossiê — Módulo 56: Síntese pulsar (`PULSAR`)

**Família:** SOURCE (gera — junto de `OSC`/`ADDITIVE`/`OPERATOR`/`WAVETABLE`)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Pulsar.hpp`, `tests/test_pulsar.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.5` (Tier 2) + `BiomaPulsar` (§4);
Curtis Roads, *Microsound*

## Estado da implementação

Nem o `ADDITIVE` (soma parciais) nem o `OPERATOR` (FM) nem o `MEMORY`
(granular de buffer) fazem síntese **pulsar** (Curtis Roads): um trem de
*pulsarets* — um grão curto (o pulsaret `w`) seguido de silêncio, com
período total `p`. Duas frequências INDEPENDENTES:

- **`freq`** (20–2000 Hz, + CV 1 V/oct) = a taxa de repetição dos
  pulsarets = **a altura** (1/`p`).
- **`formant`** (razão 0,1–8×, + CV) = a frequência INTERNA do pulsaret
  (1/`d`) = **o formante**, independente da altura. `formant > 1` →
  pulsaret mais curto que o período → silêncio entre eles (o "duty" que
  controla o brilho sem mexer no pitch). `formant < 1` → pulsarets se
  sobrepõem (síntese quase clássica).
- **`shape`** (0–1) = a forma do pulsaret: 1 ciclo de seno → 2–3 ciclos
  (sinc-ish) → pulso estreito.
- **`window`** (0–1) = a janela do grão: retangular → Hann → expodec
  (percussivo, ataque seco).
- **`jitter`** (0–1, desvio Rasgo) = jitter **semeado** no período e na
  amplitude — do trem cristalino ao ruído texturado.
- **`mask`** (0–1) = probabilidade de PULAR um pulsaret (o *masking* de
  Roads) — do denso ao esparso/rítmico. Semeado.
- **`spread`** (0–1) = pulsarets alternados L/R + leve desafino → largura.
- **`level`** (0–1) = saída.

**Saídas:** `out` (L), `r` (R). Fonte — soa ao carregar (não precisa de
entrada). `jitter=0`, `mask=0` → trem perfeitamente periódico
(determinístico). Pool de 4 vozes de grão (para `formant < 1`, os
pulsarets sobrepõem sem cortar). `process()` não aloca.

`formant=1`, `shape=0`, `window=0,5` → quase um oscilador com um leve
caráter de formante. `formant=4`, `window=1` → o "grão" percussivo
clássico da pulsar synthesis (brilhante, o formante bem acima da altura).
`mask=0,7` + `jitter=0,3` → uma textura granular esparsa e viva.

**Testes (Debug + Release):** `freq=110`, `formant=1`, `mask=0`,
`jitter=0` → a saída é periódica em 110 Hz (pico forte em 110 e
harmônicos); `formant=4` → o CENTRÓIDE espectral sobe (pico de energia
perto de 4·110) SEM o pico de 110 se mover (a altura é a mesma); varrer
`formant` de 1 a 8 → o centróide sobe monotônico, `magAt(110)` estável;
`window` retangular vs Hann → o retangular tem MUITO mais energia de
agudo (bordas duras); `mask=0,8` → a densidade de energia cai (RMS ~1/5)
e há janelas de silêncio; `jitter=0,8` → o pico de 110 borra (largura
espectral maior) mas ainda soa; `spread>0` → L≠R; `level` escala linear;
`jitter`/`mask` fixos → dois renders byte-idênticos; `jitter=mask=0` →
determinístico puro; tudo finito e |out| < ~1,2; `freq` a 1 kHz +
`formant` alto → sem alias audível acima do esperado (o `shape` é
band-limited-ish por construção — ver §3).

**Pendências (candidatos):** *pulsaret* de tabela arbitrária (do
`WAVETABLE` ou do `AUDIO-IN` — captura de grão); trem duplo com
`formant`/`freq` independentes (o "dual pulsar" de Roads);
*channelization* estocástica (o pulsaret muda de forma por evento);
*burst masking* (grupos de N acesos/apagados) além do Bernoulli; ant
alias por oversampling do grão nas taxas altas; `formant` em Hz absoluto
(além da razão).

---

## 1. Problema musical e papel no fluxo

A síntese pulsar mora entre a granular e a de formante: um trem de grãos
rápido o bastante pra ter altura, com o DUTY do grão controlando o
timbre INDEPENDENTE da altura — você abaixa `formant` e o som fica mais
oco/formântico sem desafinar. É o que dá aquelas texturas "eletrônicas
vivas" de Roads/Xenakis. `PULSAR → FILTER → MASTER` = uma voz;
`LFO → PULSAR.formant` = o formante varre; `TRIGSEQ → PULSAR.mask` (via
CV) = grãos rítmicos.

Distinção: o `MEMORY` é granular de um BUFFER (grãos de material
gravado); o `PULSAR` sintetiza o grão (o pulsaret). O `ADDITIVE`/
`OPERATOR` não têm o silêncio entre grãos que define o duty.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Curtis Roads, *Microsound*** (2001) + o papers de pulsar synthesis | pulsaret + silêncio; `freq`/`formant` independentes; masking estocástico; janelas (Hann/Gauss/expodec) | teoria publicada, domínio conceitual |
| **Iannis Xenakis** (grãos, screens) | o grão como unidade; densidade e distribuição estocástica | teoria/histórico |
| **`BiomaPulsar`** (§4, código do autor) | trem de pulsarets, o duty como controle | código do autor (AGPL — só o algoritmo) |
| **Janela de Hann / expodec** | `0,5 − 0,5·cos(2πt)`; `exp(−k·t)` | domínio público |
| **`MEMORY`/`DRUM` do Rasgo** | pool de vozes de grão, envelope de grão, acaso semeado por evento | código do autor |

**Desvio Rasgo (Atlas §49):** o `mask`/`jitter` **semeados** (a textura
é reprodutível — dois renders iguais); o `formant` como RAZÃO da
fundamental (segue o pitch por padrão, ou some dele com CV — a decisão
de identidade "sem quantização mágica"); `spread` derivando o trem por
alternância L/R, não por um 2º oscilador.

## 3. Modelo — matemática, estados, extremos

Parâmetros: `freq` (20–2000 Hz, def 110), `formant` (0,1–8, def 1),
`shape` (0–1, def 0), `window` (0–1, def 0,5), `jitter` (0–1, def 0),
`mask` (0–1, def 0), `spread` (0–1, def 0), `level` (0–1, def 0,7).

Entradas: `pitch` (Control 1 V/oct), `formant_mod` (Control).
Saídas: `out` (L), `r` (R).

**Escalonador** (a fase da fundamental):
```
f0 = clamp(freq · 2^pitch, 5, sr/2)
phase += f0/sr
if phase >= 1:
    phase −= 1
    period_jit = 1 + jitter·(rnd−0,5)·0,9        # próximo período varia
    amp_jit    = 1 − jitter·rnd·0,6
    if rnd >= mask:                              # masking: acende o pulsaret
        v = próxima voz do pool (round-robin, 4 vozes)
        v.active = true ; v.pos = 0 ; v.amp = amp_jit
        v.formantHz = clamp(f0 · formant · formCv, 5, sr·0,45)
        v.dur = sr / v.formantHz                 # 1 ciclo do formante = 1 pulsaret
        v.pan = spread>0 ? (idx%2 ? +spread : −spread) : 0
        idx++
```

**Rendering** (soma das vozes ativas):
```
oL = oR = 0
para cada voz v ativa:
    t = v.pos / v.dur                            # 0..1 dentro do pulsaret
    if t >= 1: v.active = false ; continue
    car = shapeWave(t, shape)                    # 1..3 ciclos conforme shape
    win = windowFn(t, window)                    # rect / Hann / expodec
    s = car · win · v.amp
    gL = 0,5·(1 − v.pan) ; gR = 0,5·(1 + v.pan)  # pan simples
    oL += s·gL ; oR += s·gR
    v.pos += 1
oL,oR = tanh(oL·1,4)·level ; idem R              # segurança
```

`shapeWave(t, s)`: `cyc = 1 + s·2` ; `car = sin(2π·t·cyc)` ;
`+ s·0,3·sin(6π·t·cyc)` (um pouco de agudo com `shape`). Band-limited
por natureza (senos); o pulso estreito (`s→1`) tem mais agudo mas o
`window` amacia as bordas.

`windowFn(t, w)`: `rect = 1` ; `hann = 0,5 − 0,5·cos(2π·t)` ;
`expo = exp(−4·t)·(t<1)` ; `w<0,5 ? lerp(rect, hann, w·2) :
lerp(hann, expo, (w−0,5)·2)`.

**Estados:** `phase_`, `nextPeriodScale_`, `idx_`, `voices_[4]`
(`{active, pos, dur, amp, pan}`), `rng_`, `sr_`. Sem alocação.

**Extremos.**
- `formant=1`, `window=0` (rect), `shape=0` → 1 ciclo de seno por
  período, sem silêncio, janela dura → quase uma onda dente-de-serra
  (aliasa um pouco na borda — aceito, é a natureza do rect).
- `formant` alto (8×) → pulsaret curtíssimo (1/8 do período) → 7/8 de
  silêncio → o formante em 8·`freq`, o espectro tem um pico largo lá.
- `formant < 1` → pulsaret mais longo que o período → as 4 vozes se
  sobrepõem; com `formant` muito baixo (0,1) e `freq` alto → mais de 4
  sobreposições → a 5ª rouba a voz mais velha (glitch mínimo, aceito).
- `mask=1` → nenhum pulsaret acende → silêncio.
- `jitter=1` → o período varia ±45% → altura instável, textura ruidosa;
  ainda periódico "em média".
- `freq` no mínimo (20 Hz) → quase um trem de cliques audíveis
  individualmente (fronteira granular).
- `level=0` → silêncio.
- `spread=0` → `out == r` (mono).

## 4. Três modos obrigatórios

- **autônoma:** nada conectado → o trem toca em `freq`/`formant` default
  → um tom com caráter de formante. Soa ao carregar. `mask`/`jitter`
  leves = já respira.
- **performance:** `formant`/`shape`/`window` na mão dão o timbre;
  `mask`/`jitter` a densidade e a vida; `freq` a altura.
- **híbrida:** `SEQUENCE → pitch` (melodia), `LFO → formant_mod` (o
  formante varre), `DECISION → mask` (via CV — grãos probabilísticos no
  compasso), `PULSAR → FILTER → MASTER`.

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control 1 V/oct), `formant_mod` (Control).
**Saídas:** `out` (Audio), `r` (Audio).
**Parâmetros:** ver §3.
**Limites:** `out`/`r` em ~[−1,1] (`tanh` + `level`). CPU: por amostra
≤ 4 vozes × (1 `sin` + 1 `cos`/`exp` do window). `prepare`: nada pesado.
Sem alocação, sem buffer. RNG só com `mask`/`jitter` > 0.

## 6. Alternativas descartadas

- **modo do `MEMORY`** — o `MEMORY` granula um BUFFER; o `PULSAR`
  SINTETIZA o grão. O silêncio entre pulsarets (o duty como timbre) é o
  ponto e o `MEMORY` não tem.
- **modo do `ADDITIVE`/`OPERATOR`** — nenhum tem o trem de grãos com
  silêncio; a síntese pulsar é grão-a-taxa-de-nota, não parcial nem FM.
- **pulsaret de tabela já na v1** — precisa acoplar ao `WAVETABLE`/
  `AUDIO-IN`; fica como pendência (o `shape` procedural cobre o
  essencial).
- **`formant` em Hz absoluto** — como RAZÃO ele segue o pitch por
  padrão (musical) e some com CV; Hz absoluto vira pendência.
- **`mask`/`jitter` sem seed** — textura irreprodutível é bug (regra do
  `SAMPLER`/`DRUM`/`TRIGSEQ`).
- **pool de vozes ilimitado** — 4 cobre `formant ≥ 0,25`; abaixo disso
  a 5ª rouba a mais velha (glitch mínimo, melhor que alocar).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `freq=110`, `formant=1`, `mask=jitter=0` → `magAt(110)` >
5× o piso e a saída é periódica (autocorrelação em 110 Hz); `formant=4`
vs `formant=1` → o centróide espectral sobe > 2× enquanto `magAt(110)`
varia < 20%; varrer `formant` 1→8 → centróide monotônico crescente;
`window=0` vs `window=0,5` → `bandEnergy(6k..12k) / bandEnergy(0..2k)`
do rect > 3× o do Hann; `mask=0,8` → `rms` cai pra < 0,3× o de `mask=0`
E há trechos com `|out| < 1e-4` > 20% do tempo; `jitter=0,8` → a largura
do pico de 110 (energia em 90..130 / energia em 110) sobe > 3×;
`spread=0,8` → `rms(L−R) > 0`; `level=0,3` vs `0,6` → RMS ~2×; dois
renders com `mask`/`jitter` fixos byte-idênticos; `freq=1000`,
`formant=6` → `|out| < 1,2`, finito, sem energia impossível acima de
Nyquist.

**Escuta:** abaixar `formant` sem mexer em `freq` deixa o som mais
"oco/formântico" sem desafinar? o trem cristalino (`jitter=mask=0`) tem
um brilho vidrado? `mask` alto dá uma granulação rítmica ou só corta o
som? a janela expodec (`window=1`) dá um ataque percussivo? soa como
Roads/Xenakis ou como um oscilador comum?

## 8. Integração e painel

Classe `Pulsar` (`type()` = `"PULSAR"`), 2 entradas, 2 saídas, 8
parâmetros. `panel()` próprio (12 HP): `Display` (a saída), knobs
`FREQ`/`FRMT`/`SHAPE` (linha 1), `WIND`/`JITR`/`MASK` (linha 2),
`SPRD`/`LEVEL` (linha 3); jacks `PIT`/`FQM` (entrada) · `L`/`R` (saída).
Testado isolado (periodicidade, formante independente, janela, mask,
jitter, spread, level, determinismo, alias) antes do patch. Cadeias
canônicas: `PULSAR → FILTER → MASTER`; `LFO → PULSAR.formant_mod`;
`SEQUENCE → PULSAR.pitch`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família SOURCE — junto de `OSC`/
`ADDITIVE`/`OPERATOR`/`WAVETABLE`) e ao `LearnCatalog.hpp`.
