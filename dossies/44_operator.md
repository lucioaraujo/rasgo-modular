# Dossiê — Módulo 44: Voz FM multi-operador (`OPERATOR`)

**Família:** SOURCE
**Estado:** **implementado — Onda B** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Operator.hpp`, `tests/test_operator.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda B, #44)

## Estado da implementação

O `OSC` tem TZFM linear de **um par** (uma portadora, uma moduladora
externa). `OPERATOR` é FM de verdade: **4 operadores** (senóides puras),
**8 algoritmos** de roteamento (do stack em série ao aditivo paralelo),
razões de frequência **quantizadas** a um conjunto musical, e
**realimentação** no operador A (auto-FM → dente-de-serra).

- **`algo`** (0–7) = qual operador modula qual. A ordem é sempre A→B→C→D
  (nenhum algoritmo tem laço entre operadores — só A tem feedback), então
  o cálculo é uma passada fixa, sem ordenação topológica.
- **`ratio_b`/`ratio_c`/`ratio_d`** (0–9) = razão de B/C/D em relação a A
  (que fica em `f0`), **quantizada** à tabela
  `{0,5; 1; 1,5; 2; 2,5; 3; 4; 5; 7; 9}` — inteiras = harmônico (timbre
  musical), quebradas = inarmônico (sino, metal).
- **`index`** (0–1, + CV) = profundidade de modulação global. `0` = os 4
  operadores são senóides puras; `1` ≈ 6 ciclos de desvio de fase (bem
  brilhante). Mapeado ao quadrado pra dar resolução no pé.
- **`feedback`** (0–1) = A modula a própria fase (média das 2 últimas
  amostras, à la DX7 — evita o laço instantâneo). Sozinho já leva a
  senóide de A a um dente-de-serra.
- **`drift`** (0–1, desvio Rasgo) = micro-desafino lento e independente
  por operador (soma de senóides incomensuráveis, **sem RNG** — o timbre
  FM "vive" sem deixar de ser reprodutível). `drift=0` → sem termo.

**Afinação** padrão do `OSC`: `freq`/`fine`/`pitch` (1 V/oct). Sem
entrada de FM externa em áudio (o módulo INTEIRO é FM; pra vibrato,
`pitch`). Sem envelope embutido — é um oscilador; patch `ENVELOPE → index`
pra o "ataque DX" e `ENVELOPE → VCA` pra a amplitude.

**Segurança de saída** (gate 4): soma das portadoras ÷ nº de portadoras
(senóides → |·| ≤ 1) + `softclip` (`tanh` acima de 1, assíntota ±1,5)
pra o feedback forte e o caso raro de portadoras em fase. Aditivo de 4
portadoras descorrelacionadas fica ~12 dB abaixo de uma portadora só —
`VCA`/`MIXER` depois normalizam; a alternativa (seguidor de ganho) faria
o nível bombear com `index`.

Sem alocação/lock/IO em `process()` (só `prepare()` aloca a LUT de 2048).
Determinístico sempre.

**Os 8 algoritmos** (● portadora, → modula):
```
0  A→B→C→D●                         stack de 4 (mais "FM")
1  A→B→C● , D●                      stack de 3 + senóide solta
2  A→B● , C→D●                      dois pares
3  A→C , B→C→D●                     A,B em C ; C em D
4  A→D , B→D , C→D●                 três moduladoras numa portadora
5  A→B● , C● , D●                   um par + duas senóides
6  A→B● , A→C● , A→D●               uma moduladora em três portadoras
7  A● , B● , C● , D●                aditivo (órgão)
```

**Testes (Debug + Release):** `algo 7`, `index 0`, razões 1/1/1 → senóide
pura (mag h2 < 3 % de h1); `algo 0`, `index` alto → espectro rico
(harmônica 5 sobe > 10× vs `index 0`); `index` maior → mais energia de
agudo (centroide sobe monotônico); razão de B quebrada (2,5) com `algo 0`
→ parciais inarmônicos (energia em 2,5·f0, autocorrelação a 1/f0 cai);
`feedback` sozinho leva A a dente-de-serra (aparece h2, h3…); 1 V/oct
(oitava = ×2 nos cruzamentos de zero); os 8 algoritmos dão saída finita
e < 1,5; dois renders byte-idênticos com `drift>0`; extremos limitados.

**Pendências (candidatos):** EGs por operador (o DX real — grande, fica
pra um `OPERATOR+`); mais algoritmos (6-op como o DX7); `ratio` fino
(detune em cents por operador); entrada de FM externa por operador;
tabela de razões editável; nível por operador (hoje portadoras somam com
peso igual).

---

## 1. Problema musical e papel no fluxo

O baixo elétrico, o e-piano, o sino, o metal, o "pluck" digital dos anos
80 — toda a paleta FM — não sai de `OSC → FILTER`. O `OSC` faz TZFM de um
par, que dá o metálico simples; não dá pra empilhar operadores nem
escolher o algoritmo. `OPERATOR` põe o motor DX no patch: entra 1 V/oct,
sai áudio, e `algo`/`index`/as razões moldam o timbre — todos aceitam CV,
então `ENVELOPE → index` é o ataque FM clássico, `LFO → ratio` faz o
timbre derreter.

Distinção: o `OSC` é subtrativo + TZFM de 1 par; o `WAVETABLE` varre uma
forma; o `ADDITIVE` controla parcial a parcial; o `CHORD` empilha vozes.
`OPERATOR` é síntese FM de 4 operadores — o espectro emerge da
modulação, não é endereçado direto.

## 2. Fontes ESTUDADAS (conceito, não código)

- **John Chowning, "The Synthesis of Complex Audio Spectra by Means of
  Frequency Modulation"** (1973) — a teoria: `sin(ωc·t + I·sin(ωm·t))`,
  bandas laterais em `ωc ± k·ωm` com amplitude `J_k(I)` (Bessel).
  Domínio público (artigo acadêmico).
- **Yamaha DX7 / DX21 / TX81Z** (*referência funcional*, `PESQUISA §7`) —
  o formato: operadores = senóides, algoritmos = grafo de modulação,
  feedback num operador, razões coarse/fine. 4 operadores + 8 algoritmos
  é o conjunto do DX21/DX100/TX81Z. Só o **conceito** (senóide+algoritmo+
  feedback), nenhum ROM nem tabela de EG — a patente do DX expirou, a
  técnica é a de Chowning.
- **Akemie's Castle / YM2151 (OPM)** (`PESQUISA §7 #88`) — Eurorack com
  chip FM real; os 8 algoritmos do OPM inspiram os daqui (adaptados —
  ordem A→B→C→D fixa pra o cálculo ser uma passada).
- **Bastl Pizza** (`PESQUISA §7 #98`) — FM de 2 operadores + wavefolder
  em Eurorack; o "FM simples que soa complexo".

**Desvio Rasgo (Atlas §49):** o `drift` de desafino determinístico por
operador (o patch FM respira sem RNG), as razões quantizadas a um
conjunto musical (não um número livre — decisão, não ajuste fino), e a
integração — `index`/`ratio`/`algo` todos endereçáveis por CV do grafo,
então o timbre FM é um **processo**.

## 3. Modelo

**Preparação:** `lut[i] = sin(2π i / 2048)`.

**Por amostra:**
```
f0 = freq · 2^(fine/1200 + pitch_in)                  (clamp 0,01 .. 0,45·sr)
idx  = clamp01(index_knob + index_in)
depth = idx² · 6                                       (ciclos de desvio)
fb    = feedback²                                     (0 .. 1 ciclo, ≈ DX7 máx)

rA=1 ; rB,rC,rD = tabela[round(ratio_x)]               (× (1 + drift_x))
ph_op += f0 · r_op · dt ; ph_op −= floor(ph_op)

aFb = fb · ½(A₋₁ + A₋₂)
A = lut(ph_A + aFb)
B = lut(ph_B + depth · (algo.aToB · A))
C = lut(ph_C + depth · (algo.aToC·A + algo.bToC·B))
D = lut(ph_D + depth · (algo.aToD·A + algo.bToD·B + algo.cToD·C))
A₋₂ = A₋₁ ; A₋₁ = A

soma = Σ (algo.carrier[op] · op) ; n = nº de portadoras
out = softclip(soma / max(1,n))            (n senóides ÷ n → |·| ≤ 1)
```
`lut(x)`: `x −= floor(x)` (a modulação joga a fase pra fora de [0,1)),
interp linear na tabela de 2048.

**`drift`:** 4 fases lentas a `{0,021; 0,029; 0,037; 0,045}·(1+…)` Hz;
`drift_op = 0,004 · drift · sin(2π · fase_op + op·1,7)`.

**Extremos:** `index=0` → 4 senóides (nenhuma modulação); a de `algo 7`
com razões 1 é uma senóide pura. `feedback=1` sozinho → A vira
dente-de-serra (estável — a média de 2 amostras evita o estouro do laço
instantâneo). `index=1` + `algo 0` → espectro denso mas limitado (as
bandas laterais além de Nyquist dobram — aliasing de FM, caráter aceito e
anotado, como no DX real; sem oversampling nesta v1). Razão no índice
máximo (9) + `f0` alto → operador perto de Nyquist, `lut` não alia
(band-limited por ser senóide) mas as bandas laterais sim.

## 4. Três modos obrigatórios

- **autônoma:** `freq` audível, `algo` 0–2, `index` médio, `drift` leve →
  um timbre FM que evolui sozinho; sem cabo nenhum já é uma voz.
- **performance:** `algo`, `index` e as 3 razões são os macros; `freq` a
  afinação.
- **híbrida:** `ENVELOPE.env → index` (o ataque FM — brilhante no
  transiente, escurece na cauda); `SEQUENCE → 1V/O`; `LFO → ratio_c`
  (timbre que derrete e volta).

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control v/oct), `index` (Control).
**Saídas:** `out` (Audio).
**Parâmetros:** `freq` (8–8000 Hz, def 110), `fine` (−100..100 cent),
`algo` (0–7, def 0), `ratio_b`/`ratio_c`/`ratio_d` (0–9, def 1/2/3),
`index` (0–1, def 0,3), `feedback` (0–1, def 0), `drift` (0–1, def 0).
**Limites:** `out` em (−1,5; 1,5) (`softclip`; a soma ÷ nº de portadoras
já é ≤ 1). CPU: por amostra 4 leituras de LUT + ~10 mul-add + 4 `sin`
(drift). `prepare` aloca 2048 floats (~8 KB).

## 6. Alternativas descartadas

- **EGs por operador** (o DX real) — 4 envelopes de 4 estágios + rate
  scaling é metade de um DX7; fica pra `OPERATOR+`. A v1 é o oscilador;
  `ENVELOPE` externo cobre o essencial.
- **6 operadores / 32 algoritmos** (DX7 pleno) — 4-op + 8 algoritmos é o
  DX21/TX81Z, cobre a paleta e cabe no painel. 6-op é evolução.
- **oversampling do núcleo FM** — o aliasing de banda lateral é parte do
  som FM digital (o DX aliava); 2× dobraria o custo por um ganho
  discutível. Fica anotado.
- **razão como número livre** — quantizar a um conjunto musical é
  decisão de identidade (o `QUANTIZER` faz isso pra altura; aqui pra
  timbre). `fine`/detune por operador fica como pendência.
- **ordenação topológica geral dos operadores** — os 8 algoritmos foram
  escolhidos pra caber numa passada A→B→C→D; um editor de matriz livre
  precisaria de topo-sort e é outro módulo.

## 7. Integração e painel

14 HP, família **SOURCE** (junto de `OSC`/`WAVETABLE`/`ADDITIVE`). Display
do timbre/forma. Knobs `FREQ`/`FINE`/`ALGO`/`INDEX`/`FBK` (linha 1),
`RB`/`RC`/`RD`/`DRIFT` (linha 2); jacks `1V/O`/`IDX` + `OUT`.

Cadeias canônicas: `SEQUENCE → QUANTIZER → OPERATOR → VCA`;
`ENVELOPE.env → OPERATOR.index` (ataque FM) + `ENVELOPE.env → VCA`;
`LFO → OPERATOR.ratio_c` (timbre em movimento).
