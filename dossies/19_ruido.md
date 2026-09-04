# Dossiê — Módulo 19: Ruído e aleatório (`NOISE`)

**Família:** SOURCE + UTILITY
**Estado:** **implementado — marco 3** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Noise.hpp`, `tests/test_noise.cpp`

## Estado da implementação (marco 3)

Feito: a **fonte de acaso contínuo** que faltava. `DECISION` decide
eventos discretos; `NOISE` dá o piso de ruído (percussão, vento,
textura) **e** as fontes de modulação aleatória que todo patch modular
usa.

- **`white`** — ruído branco (xorshift64*, uniforme em [−1,1));
- **`pink`** — filtro de ruído rosa de Paul Kellet ("economy", 7 estágios
  — domínio público) → −3 dB/oitava;
- **`brown`** — passeio aleatório com vazamento (integra o branco, decai)
  → −6 dB/oitava;
- **`sh`** — **sample-and-hold**: segura um valor até o próximo pulso (de
  `trigger` externo ou do relógio interno em `rate`); amostra a entrada
  `in` se conectada, senão o próprio branco;
- **`smooth`** — CV que **passeia**: a cada pulso escolhe um alvo novo e
  desliza até ele com constante de tempo `slew` (Buchla 266 "smooth
  random"). `slew = 0` → igual ao S&H (degraus); `slew = 1` → deriva
  lenta de segundos;
- **`spread`** (0–1) — desvio Rasgo: a distribuição do S&H e do smooth
  vai de **uniforme** (0) a **sino** (1, média de 4 uniformes) — acaso
  *estruturado*, não plano (padrão `shape` do `DECISION`).

Dois streams xorshift semeados em `prepare()` (um pro branco, um pro
S&H/smooth) → determinístico (dois renders byte-idênticos). Sem alocação,
sem lock/IO em `process()`.

**Testes (11/11, Debug + Release):** branco tem média ~0 e espectro
plano; rosa cai ~−3 dB/oit e brown ~−6 dB/oit (energia de banda alta vs
baixa); S&H só muda no pulso (contagem de degraus = contagem de pulsos)
e segura entre pulsos; S&H amostra a entrada `in` quando conectada;
`smooth` com `slew` alto varia devagar (derivada por amostra pequena) e
com `slew = 0` dá degraus; `spread = 1` concentra o S&H perto de 0
(variância menor que uniforme); todas as saídas em [−1,1] e finitas;
dois renders byte-idênticos; integração no grafo (`CLOCK → NOISE.trigger`
→ `sh` → `OSC.pitch`).

**Pendências (candidatos, não controles fictícios):** ruído azul/violeta
(+3/+6 dB/oit); `slew` assimétrico (rise ≠ fall); gate aleatório
(Bernoulli por pulso — mas `DECISION` já faz); dois canais de S&H
correlacionados (`correlation`, como Marbles `X`); ruído de grão
(crackle/dust).

---

## 1. Problema musical e papel no fluxo

Sem `NOISE`: não há bumbo (transiente de ruído), não há chimbal/caixa
(ruído filtrado), não há vento/mar, e — pior pra um modular — **não há
fonte de modulação aleatória**. Todo patch generativo quer um S&H
alimentando uma altura, um smooth-random abrindo um filtro devagar, um
piso de ruído no fundo. `MATTER`/`STRING` usam ruído *internamente* mas
não o expõem; `DECISION` decide *eventos*, não dá um sinal contínuo.

Papel: fonte. `white`/`pink`/`brown` → `FILTER`/`ENVELOPE` (percussão,
textura); `sh` → `QUANTIZER`/`OSC.pitch` (melodia aleatória); `smooth` →
qualquer parâmetro via cabo (deriva orgânica). O `trigger` vem de um
`CLOCK`/`SEQUENCE`/`DECISION`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Paul Kellet — "economy" pink noise filter** (music-dsp list, domínio público) | 7 filtros de 1 polo somados aproximam −3 dB/oit com custo baixíssimo | domínio público |
| **Voss-McCartney** (pink noise por soma de fontes em oitavas) | princípio do rosa; a versão Kellet é a otimização | domínio público |
| **Sample-and-hold clássico** (Buchla 265/266, Doepfer A-118, qualquer texto) | segura o valor de uma fonte até um gatilho | prática |
| **Buchla 266 "Source of Uncertainty" — smooth random** | tensão aleatória que *passeia* (integra/desliza entre alvos) — não degraus | conceito |
| **`DECISION` do Rasgo (`shape`)** | uniforme→sino pela média de N uniformes = acaso estruturado | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
w   = xorshiftWhite() / 2^31        [-1,1)   (branco)
pink: b0..b6 (Kellet):
  b0 = 0.99886·b0 + w·0.0555179 ; … ; b5 = −0.7616·b5 − w·0.0168980
  pink = (b0+b1+b2+b3+b4+b5+b6 + w·0.5362) · 0.11
  b6 = w·0.115926
brown: br = 0.99·br + w·0.05 ; brown = clamp(br·3.8, −1, 1)

pulso? (trigger ext borda ↑  OU  fase interna: phase += rate/sr, wrap):
  d      = draw()            (draw = spread<eps ? u : mix(u, mean4u, spread))
  held   = in conectada ? in[frame] : d
  target = d
sh     = held
smooth += (target − smooth) · slewCoef      (slewCoef de `slew`)
```
`draw()` usa o stream `rngSH_`; `u` = uniforme [−1,1); `mean4u` = média
de 4 uniformes (≈ sino, var. menor).

**Estados:** `b_[7]` (pink), `br_` (brown), `held_`, `smoothTarget_`,
`smooth_`, `phase_` (relógio interno), `prevTrig_`, `rngWhite_`,
`rngSH_`. Sem alocação.

**Extremos.** `rate` no teto (2 kHz) → S&H vira quase ruído (aceitável, é
o limite). Sem `trigger` e `rate → 0` → S&H/smooth congelam no último
valor (correto). `slew = 1` a `rate` alto → smooth mal sai do lugar
(muitos alvos, glide lento) — vira quase DC ruidoso. `in` conectada com
DC → S&H segura o DC. Reset → `pink`/`brown`/`smooth` zerados, RNG
re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** `white`/`pink`/`brown` sem entrada nenhuma — piso de
  ruído / vento. `smooth` com `rate` baixo e `slew` alto = deriva de
  fundo sem nada conectado.
- **Performance:** `rate` e `spread` ao vivo mudam o caráter do S&H
  (metralhadora ↔ passos deliberados). `slew` abre/fecha a "cola" do
  smooth.
- **Híbrida:** `CLOCK`/`SEQUENCE` no `trigger`; `sh` → `QUANTIZER` (linha
  melódica aleatória travada na escala); `smooth` → `FILTER.cutoff` (o
  timbre respira sozinho); `in` de um `TURING` → S&H re-amostra o laço.

## 5. Portas, parâmetros, limites

**Entradas:** `trigger` (Control, trig — S&H/smooth), `in` (Audio —
fonte externa opcional pro S&H).
**Saídas:** `white`, `pink`, `brown`, `sh`, `smooth` (todas Audio).
**Parâmetros:** `rate` (0,01–2000 Hz, def 8), `slew` (0–1, def 0,3),
`spread` (0–1, def 0).
**Limites:** todas as saídas em [−1,1]. CPU por amostra: 1 xorshift + 7
multiplicações-acumulações (pink) + aritmética. Sem alocação.

## 6. Alternativas descartadas

- **FFT / filtro de ordem alta pro rosa:** o Kellet de 7 polos é
  −3 dB/oit "bom o suficiente" com custo trivial. Filtro exato é 2ª
  camada.
- **Ruído gaussiano por Box-Muller no branco:** o uniforme xorshift soa
  igual num contexto de áudio e é mais barato; o `spread` já dá a opção
  de sino onde importa (S&H/smooth).
- **Um `color` que faz crossfade white↔pink↔brown numa saída só:** 3
  saídas independentes é mais modular (o `MIXER` faz o crossfade).
- **`smooth` como filtro passa-baixa do branco:** dá ruído colorido, não
  uma tensão que *passeia* entre alvos. O modelo de alvo+glide é o do
  Buchla 266 e é o que um modulador quer.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** branco — média ≈ 0, espectro ~plano (energia por banda
comparável); rosa — banda alta < banda baixa (~−3 dB/oit medido); brown
— queda mais forte (~−6 dB/oit); S&H — nº de degraus = nº de pulsos, e
segura exatamente entre pulsos; S&H amostra `in` quando conectada;
`smooth` com `slew` alto tem derivada por amostra pequena; `spread = 1`
→ variância do S&H menor que uniforme; tudo em [−1,1] e finito; dois
renders byte-idênticos.

**Escuta:** o branco soa "branco" (não "chiado digital metálico")? o
rosa soa "cheio" como chuva/mar? o brown soa "grave e mole" como vento
forte? o S&H a `rate` médio dá aquele padrão "computador dos anos 70"?
o `smooth` num filtro soa vivo (respira) ou nervoso? `spread` alto faz o
S&H "escolher com intenção" em vez de saltar?

## 8. Integração e painel

Classe `Noise` (`type()` = `"NOISE"`), 2 entradas, 5 saídas, 3
parâmetros. `panel()` próprio (12 HP — 5 saídas precisam de largura pra
espaçar os jacks: knobs RATE/SLEW/SPREAD, jacks
TRIG/IN in e WHT/PNK/BRN/S&H/SMTH out). Testado isolado (espectro,
S&H, smooth, spread, determinismo) antes do patch. Cadeias canônicas:
`NOISE.pink → FILTER → ENVELOPE` (caixa/chimbal); `CLOCK → NOISE.trigger`
· `NOISE.sh → QUANTIZER → OSC.pitch` (melodia aleatória); `NOISE.smooth →`
cabo pra qualquer `*_mod`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família SOURCE).
