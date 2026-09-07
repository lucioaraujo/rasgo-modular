# Dossiê — Módulo 57: Resíntese espectral (`SPECTRA`)

**Família:** SOURCE (gera — mas o material vem de `in`; sem `in` é fonte autônoma)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Spectra.hpp`, `tests/test_spectra.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.6` (Tier 1) — a lacuna análise → síntese;
Rossum Panharmonium / Intellijel Rainmaker spectral (conceito, não código);
phase vocoder (Flanagan & Golden 1966)

## Estado da implementação

Nenhum módulo RASGO **ouve um sinal e o re-sintetiza**. O `ADDITIVE`
constrói um espectro do zero por macros; o `MEMORY` faz grão no TEMPO
(pedaços do buffer); o `RESONATOR` filtra uma excitação por um banco
afinado. `SPECTRA` fecha a ponte: analisa o espectro de curto prazo de
`in` e re-oscila como um banco de senóides que **segue** o som.

- **ANÁLISE:** 64 passa-faixas ressonantes de 2 polos, log-espaçados de
  35 Hz a 14 kHz, largura de banda ~1/9 de oitava (as bandas se
  sobrepõem — um parcial ENTRE bandas ainda é detectado). Cada banda tem
  um seguidor de pico (ataque 3 ms, release 60 ms). A cada ~6 ms (hop) o
  módulo acha os máximos locais acima de −40 dB do maior, ordena por
  força, pega os `voices` primeiros e — com interpolação parabólica em
  log-freq entre as 3 bandas vizinhas — refina a frequência de cada um.
- **SÍNTESE:** `voices` (2–24) senóides de fase contínua. Cada uma herda
  a frequência/amplitude de um pico e **DESLIZA** (sem zíper) para lá; a
  constante de tempo é `blur` (0 = ~5 ms, trava no som; 1 = ~600 ms,
  borra). `freeze` (toggle + gate) para de reatribuir → o banco continua
  oscilando no último espectro (*spectral freeze*). `shift` (±2 oct, + CV
  1 V/oct em `pitch`) e `stretch` (−1..1 — afasta/junta os parciais em
  torno do meio, inarmônico) transpõem a re-síntese SEM tocar na
  análise. `tone` (−1..1) inclina o espectro da saída. `jitter` (0–1,
  **desvio Rasgo**) = wobble lento e SEMEADO por voz (±3 %). `mix`
  seco↔ressintetizado. Saídas `out` (L) / `r` (R) — vozes ímpares/pares
  panoramizadas.

**`in` livre → fonte autônoma:** um ruído interno de −30 dB + dois
"parciais fantasma" que derivam devagar (semeados, sem RNG na fase)
alimentam a análise → `SPECTRA` sozinho é um drone tonal que evolui.

`process()` não aloca (o banco é fixo, os buffers vêm no `prepare()`).
`jitter=0` + `in` cabeado → determinístico byte a byte; `jitter=0` sem
entrada → também (o ruído/drift internos são semeados).

**Testes (Debug + Release):** senóide de 330 Hz → a saída concentra a
energia em ~330 (banda 300–360 ≥ 4× o resto); 200+400+600 → energia nos
três parciais, pouca entre eles; `shift=+1 oct` → 300 Hz vira ~600 (e
some de 300); `pitch` CV +1 oct = idem; `blur` alto → a energia demora
mais a assentar (transiente borrado); `freeze` + entrada REMOVIDA → o
som continua em ~350; sem entrada → drone audível, tudo finito; L ≠ R;
`mix=0` = bypass byte-exato; `jitter=0` → dois renders byte-idênticos
(com e sem entrada); `voices=24`, tudo no talo → |out| < 1,3.

**Pendências (candidatos):** análise por FFT real (mais resolução de
frequência que o banco log de 64); rastreio de parciais com continuidade
(*partial tracking* McAulay–Quatieri — evita o "salto" quando dois picos
trocam de ordem); `blur` como duas constantes (freq lenta, amp rápida);
saída de CV com o número de parciais ativos / o centróide; `stretch`
como curva (não só linear); congelar SÓ a frequência ou SÓ a amplitude.

---

## 1. Problema musical e papel no fluxo

O RASGO tem muitas maneiras de FAZER espectro (`OSC`, `ADDITIVE`,
`OPERATOR`, `WAVETABLE`, `PULSAR`) e de MEDIR som (`SCOPE` → CV). Não
tinha como usar **o espectro de um som como partitura de outro**. É a
técnica por trás de meia dúzia dos módulos mais cobiçados fora do RASGO
(Panharmonium, Rainmaker no modo espectral, Clouds spectral, o
"resynthesis" de vários granulares) e não estava representada.

No fluxo: `SPECTRA` fica em SOURCE porque a saída é um banco de
osciladores, não um sinal filtrado. Mas ele quase sempre é cabeado com
algo em `in` — uma voz, um `DRUM`, uma gravação do `SAMPLER`, o
`SIGNAL-IN`. Casos de uso:

- **sombra harmônica:** `OSC` → `SPECTRA.in`, `voices` alto → quase um
  clone; baixe `voices` e ligue `stretch` → uma sombra inarmônica que
  acompanha a melodia.
- **pad de qualquer coisa:** toque um acorde num sampler → `freeze` →
  um pad infinito daquele instante; solte o `freeze` e o pad volta a
  seguir o som.
- **coro fantasma:** `blur` alto num vocal/pad → as vozes se arrastam
  entre os parciais → um halo que "derrete".
- **autônomo:** sem `in`, um drone que respira — bom leito para um patch
  de seed.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

- **Phase vocoder** — Flanagan & Golden, *Phase Vocoder* (Bell System
  Technical Journal, 1966). A ideia canônica: STFT → magnitude/fase por
  bin → manipular → reconstruir. `SPECTRA` NÃO faz IFFT/overlap-add;
  usa um banco de análise + banco de osciladores (mais "modular", sem
  latência de bloco), mas o princípio análise→re-síntese é esse.
- **Síntese por modelagem espectral (SMS)** — Xavier Serra, tese (CCRMA,
  1989) + Serra & Smith, *Spectral Modeling Synthesis* (CMJ, 1990): o
  som como soma de senóides (parciais) + resíduo. `SPECTRA` faz só a
  parte determinística (as senóides).
- **Modelo sinusoidal** — McAulay & Quatieri, *Speech Analysis/Synthesis
  Based on a Sinusoidal Representation* (IEEE ASSP, 1986): rastrear
  picos espectrais de quadro a quadro. `SPECTRA` faz uma versão simples
  (reatribui os `voices` picos mais fortes a cada hop); o *partial
  tracking* com continuidade fica como pendência.
- **Spectral freeze** — técnica pública (ex. o objeto `pvoc`/`freeze` do
  Puckette/Pd, o "spectral hold"): parar de atualizar o quadro de
  análise e continuar re-sintetizando.
- **Banco de análise log / constant-Q** — teoria de bancos de filtro de
  Q constante (a base do CQT de Brown, 1991). O `SCOPE` do RASGO já usa
  um banco Goertzel log de ~24 bandas pro desenho de espectro; `SPECTRA`
  usa 64 ressoadores de 2 polos.
- **Rossum Panharmonium / Intellijel Rainmaker (modo espectral)** —
  fichas públicas de recursos (**conceito, não código** — ambos
  fechados). Daí vem a IDEIA de "banco de osciladores de análise" +
  `blur` + `freeze` + transposição da re-síntese. O `BiomaX` (§4) NÃO
  tem prior art espectral — este é modelo público puro.

**Desvio Rasgo obrigatório:** o modo autônomo (ruído + parciais fantasma
semeados quando `in` está livre — o Panharmonium precisa de entrada); o
`jitter` semeado; `stretch` inarmônico (o Panharmonium só transpõe
linear); e a saída estéreo por panoramização de voz.

## 3. Modelo — matemática, estados, extremos

**Banco de análise** (`prepare`, 1×): banda `b` em
`f_b = 35 · (14000/35)^(b/63)`. Ressoador de 2 polos normalizado
(numerador `x[n] − x[n−2]`, pico ≈ unitário):

```
r_b = exp(−π · (0,11 · f_b) / sr)          largura ~1/9 oct
a1_b = 2 r_b cos(2π f_b / sr)   ;   a2_b = −r_b²   ;   b0_b = 1 − r_b
y_b[n] = b0_b (x[n] − x[n−2]) + a1_b y_b[n−1] + a2_b y_b[n−2]
env_b += (|y_b[n]| − env_b) · (|y_b| > env_b ? k_atk : k_rel)
```

**Hop** (a cada `⌊0,006·sr⌋` amostras, a menos que congelado): candidatos
= `b` com `env_b ≥ env_{b−1}`, `env_b > env_{b+1}`, `env_b > 0,01·max`.
Ordena decrescente. Para os `voices` primeiros: interpolação parabólica
de `ln(env)` em `{b−1, b, b+1}` → deslocamento `δ ∈ [−1,1]` de banda em
log-freq → `f_pico = exp(ln(f_b) + δ · passo)`; `amp_pico = env_b · 1,5`.
Vozes sem pico: `amp_alvo = 0`.

**Síntese** (por amostra): cada voz
`freq += (freq_alvo − freq)·g` e `amp += (amp_alvo − amp)·g`, com
`g = 1 − exp(−1/((0,005 + blur²·0,6)·sr))`. Depois
`f_k = freq · 2^(shift + pitchCv) · (1 + stretch·0,35·rel_k) · (1 + jitter·w_k·0,03)`
com `rel_k = k/(voices−1) − 0,5` e `w_k` um passeio semeado lento.
`s_k = amp · f(f_k, tone) · sin(2π φ_k)`, `φ_k += f_k/sr`.
`tone ≠ 0` → `× (f_k/800)^(tone·1,3)`. Soma com pan ±0,28 por paridade,
`tanh(·1,5)`, `mix` com `in`.

**Extremos:** `voices=2` — caricatura de 2 parciais; `voices=24` —
re-síntese densa. `blur=0` — segue transientes (pode "borbulhar" se o
som muda rápido); `blur=1` — arrasta ~0,6 s. `stretch=±1` — parciais
espalhados quase 1 oitava (metal/vidro). `freeze` + `mix=1` sem `in` —
pad puro. `in` = ruído branco → todas as bandas parecidas → os picos são
instáveis, a re-síntese "cintila" (comportamento honesto — ruído não tem
parciais).

## 4. Três modos obrigatórios

1. **autônomo / repouso** — `in` livre. Ruído interno de −30 dB + 2
   parciais fantasma (180 Hz, 430 Hz) que derivam ±40 % em escala de
   ~1 min (semeado). Drone tonal que evolui. `mix` irrelevante (não há
   seco).
2. **seguidor** — `in` cabeado, `freeze=0`. A re-síntese persegue o
   espectro de `in` na velocidade de `blur`. `mix` mistura o som cru
   com a re-síntese.
3. **congelado** — `freeze=1` (toggle ou gate). A análise para; o banco
   segura o último espectro. Tirar `in` depois de congelar deixa a nota.
   Soltar volta ao modo 2 do ponto onde está.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (áudio a analisar), `pitch` (CV 1 V/oct → soma em
`shift`), `freeze` (gate → congela).
**Saídas:** `out` (L), `r` (R).
**Parâmetros:** `voices` 2–24 (def 12) · `blur` 0–1 (def 0,3) ·
`shift` −2..2 oct (def 0) · `stretch` −1..1 (def 0) · `tone` −1..1
(def 0) · `jitter` 0–1 (def 0) · `freeze` 0/1 (def 0) · `mix` 0–1
(def 1).

## 6. Alternativas descartadas

- **FFT + IFFT + overlap-add (phase vocoder de verdade)** — mais fiel,
  mas: latência de bloco (≥ N/2 amostras), precisa de uma FFT no core
  (dependência nova ou ~40 linhas de radix-2), e o "phasiness" clássico.
  O banco de osciladores de análise (Panharmonium) é sem latência e mais
  "instrumento". FFT fica como pendência se a resolução do banco de 64
  incomodar.
- **rastrear os picos com histerese/continuidade** (não deixar duas
  vozes trocarem de parcial) — melhora o *portamento* mas complica; o
  `blur` já mascara a maior parte. Pendência.
- **`voices` senóides livres SEM análise (só um banco aditivo)** — isso
  é o `ADDITIVE`. O ponto do `SPECTRA` é a análise dirigir.
- **resíduo/ruído (a parte não-senoidal do SMS)** — nichado; a família
  `NOISE`/`MEMORY` cobre textura de ruído. Fica de fora.

## 7. Critérios técnicos e perguntas de escuta

- senóide pura em `f` → energia da saída concentrada em `f` (± erro do
  banco, ~1 %), pouca fora; RMS > 0,08.
- 2–3 parciais harmônicos → energia em cada um, < 60 % de vazamento
  entre eles.
- `shift`/`pitch` CV transpõem a re-síntese; a análise não muda.
- `blur` alto → o transiente de ataque borra (early/late RMS cai).
- `freeze` → tirar `in` e o som fica na altura congelada.
- sem `in` → drone audível, finito, evolui (não é estático nem exabrupto).
- `mix=0` → bypass byte-exato.
- `jitter=0` → determinístico (com e sem entrada).
- tudo no talo (`voices=24`, `shift/stretch/tone/jitter` no máximo,
  entrada rica) → |out| < 1,3, finito.
- **escuta:** um `OSC` serra em `in`, `voices` de 2 a 24 — de "sombra
  simplificada" a "quase o mesmo som". `freeze` num acorde — o pad
  segura. `blur` alto num pad que muda de nota — o halo se arrasta.

## 8. Integração e painel

**Catálogo:** SOURCE, depois do `PULSAR`
(`OSC · WAVETABLE · ADDITIVE · OPERATOR · PULSAR · SPECTRA · PLL · …`).
**Painel:** 14 HP. Display "spectra" (66 mm — barras dos parciais
ativos, pendência de desenho). VOICE/BLUR/SHIFT na 1ª fileira;
STRCH/TONE/JITR na 2ª; FRZ (toggle) + MIX na 3ª. Jacks IN/PIT/FRZ e L/R.
**LEARN:** 12 binds (3 níveis) + a definição do módulo.
**CTest:** `rasgo_modular_spectra_tests` (11 testes).
