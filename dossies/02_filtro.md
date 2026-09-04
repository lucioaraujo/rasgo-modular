# Dossiê — Módulo 2: Filtro (`FILTER`)

**Família:** TRANSFORM
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Filter.hpp`, `tests/test_filter.cpp`

## Estado da implementação (marco 1)

Feito: 3× SVF TPT (Cytomic) por canal, saídas `low`/`center`/`high`/`all`;
`spread` desloca as três frequências (±2 oitavas) - de "3 tomadas de um
filtro" a formante; `drive` com sine-shaping pré-filtro; suavização de
`cutoff` ~5 ms; **auto-oscilação** via `k` empurrado a ligeiramente
negativo perto de `resonance=1` + **não-linearidade NO LAÇO** (satura o
estado, não só a saída) → ciclo-limite estável em vez de NaN; limitador
suave na saída. Painel próprio (12 HP: display de resposta + CUTOFF/RESO/
SPREAD/DRIVE + 8 jacks).

**Testes (4/4 alvos, 3 configs):** com `spread=0`, `low` = LP e `high` =
HP do mesmo corte (200 Hz vs 5 kHz > 4×, banda passante ~0 dB); com
`spread=1`, `low` centra em ~200 Hz e `high` em ~3200 Hz (formante);
`resonance→1` **auto-oscila numa senoide na frequência de corte ±5%**,
sustentada e limitada; varredura de `cutoff` 20 Hz→Nyquist com ruído +
`drive` 0,7 sem NaN nem estouro; determinístico byte-idêntico; painel
fecha; integração no grafo (voz → filtro com cutoff modulado).

**Render:** `examples/primeiro_fragmento.cpp` atualizado - agora 3 LFOs
(um com drift) modulam rate da voz + cutoff + spread do filtro; `all` →
Cable → saída; ruptura aos 5 s. Timbre em movimento + cicatriz.

**Pendências (candidatos, não controles fictícios):** oversampling 2× no
`drive` (alias medido em nível moderado, não testado abaixo de -35 dB
ainda); inserts atravessáveis por banda (SPECTRA); modo formante com
vogais nomeadas; `LADDER` como módulo próprio (Top 100).

---

## 1. Problema musical e papel no fluxo

Depois da fonte (Módulo 1), o segundo bloco de qualquer patch. Mas num
instrumento onde "os fluxos proporcionam variedade sonora", o filtro não
deveria ser só "corta agudo / corta grave". A ideia adotada: **um filtro
cujas três saídas (grave / centro / agudo) são a MESMA frequência de corte,
e o caráter vem de COMO elas se relacionam** — de "três tomadas de um
filtro só" até "três filtros afastados" que formam uma resposta de
formante. A relação entre as saídas é o processo (princípio Warps, Atlas
§39; hardware de referência: Mannequins Three Sisters).

Papel: transforma a voz; recebe modulação de corte/ressonância; as três
saídas alimentam três caminhos diferentes do patch.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Andrew Simper / Cytomic**, "Solving the continuous SVF equations using trapezoidal integration and equivalent currents" (2013) | núcleo SVF **TPT/trapezoidal**: LP, BP, HP, notch, peak, all-pass de um cálculo só; zero-delay-feedback; estável e afinado até perto de Nyquist | paper público; reescrito do zero |
| **Mannequins Three Sisters** (Whimsical Raps) | três saídas (LOW/CENTER/HIGH) na mesma frequência; controle `SPREAD`/`FORMANT` que muda a relação entre elas; modo em que viram um filtro de formante | hardware, estudo de comportamento (sem código) |
| **Mutable Ripples (2020)** (`pichenettes/eurorack`, STM32F) | filtro de 4 polos com auto-oscilação limpa, ganho de compensação | MIT — estudo do algoritmo |
| **EMW FIXED FILTER BANK / RESONANT FILTER SEQUENCER** (hardware do autor) | bancos de banda fixa como matéria de timbre; precedente prático | — |
| **`biome.odt` (wavefolding/DAFx23)** | drive de entrada como caráter, com sine-shaping + oversampling pra antialias | técnica pública |

## 3. Modelo — matemática, estados, extremos

**Núcleo SVF TPT (Cytomic).** Por amostra, com `g = tan(π·fc/fs)`,
`k = 1/Q`:
```
v1 = (ic1eq + g·(x - ic2eq)) / (1 + g·(g + k))
v2 = ic2eq + g·v1
ic1eq = 2·v1 - ic1eq
ic2eq = 2·v2 - ic2eq
lp = v2;  bp = v1;  hp = x - k·v1 - v2
```
`ic1eq`/`ic2eq` são o estado (correntes equivalentes). `notch = x - k·bp`,
`peak = lp - hp`, `all = x - 2·k·bp`.

**As três irmãs.** `spread ∈ [0,1]` desloca as três frequências
efetivas em torno de `cutoff`:
```
f_low    = cutoff · 2^(-spread · S)
f_center = cutoff
f_high   = cutoff · 2^(+spread · S)     (S ~ 2 oitavas no máximo)
```
- `spread = 0`: três SVF na mesma frequência → `low` = LP, `center` = BP,
  `high` = HP do mesmo filtro (as "três tomadas");
- `spread` alto: três passa-banda afastados → resposta de **formante**;
- saída `all` = soma das três com compensação de ganho.

**Ressonância.** `resonance ∈ [0,1]` → `Q = 0.5 · 2^(resonance · 6.5)`
(Q de 0,5 a ~45). Acima de ~0,97 o SVF entra em auto-oscilação (senoide
na frequência de corte) — comportamento desejado, com limitador suave na
saída pra não estourar.

**Drive.** `drive ∈ [0,1]`: ganho de entrada `1 + drive·6` seguido de
sine-shaping `sin(x·π/2)/1` clampado — caráter de saturação antes do
filtro. Marco 1 sem oversampling (alias medido; oversampling 2× é 2ª
camada, como no Módulo 1).

**Extremos.** `cutoff` no piso (20 Hz) e teto (min(20 kHz, fs/2·0.49)):
`g` bem definido, sem estouro (`tan` longe de π/2). `spread` no máximo com
`cutoff` alto: `f_high` clampado a fs/2·0.49. `resonance` = 1: auto-oscila,
limitador segura. Reset: `ic1eq = ic2eq = 0` nas três instâncias. Entrada
não-finita saneada pra 0 antes do filtro.

## 4. Três modos obrigatórios

- **Autônoma:** com `cutoff`/`resonance`/`spread`/`drive` dos parâmetros,
  o filtro tem caráter fixo mas útil; `resonance` alta + entrada de ruído
  já é uma voz (auto-oscilação com excitação).
- **Performance:** `cutoff` e `spread` são os macros gestuais — varrer
  `spread` de 0 a 1 transforma "filtro" em "formante" de forma contínua e
  audível. `cutoff` com suavização ~5 ms.
- **Híbrida:** entradas `cutoff_mod` (1 V/oct), `res_mod`, `spread_mod`
  somam aos parâmetros. Uma saída da voz (Módulo 1) modulando `cutoff`
  aqui já é um patch generativo.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (áudio), `cutoff_mod` (1 V/oct), `res_mod`, `spread_mod`.
**Saídas:** `low`, `center`, `high`, `all`.
**Parâmetros:** `cutoff` (20–20000 Hz, log, default 800), `resonance`
(0–1, default 0,15), `spread` (0–1, default 0), `drive` (0–1, default 0).
**Limites:** saída limitada a [-1,1] (limitador suave). CPU: 3 SVF
(~15 mult/add cada) + shaping. Sem alocação. Estado fixo (6 floats de
correntes equivalentes + suavizadores).

## 6. Alternativas descartadas

- **Ladder Moog (Stilson/Smith / Huovilainen)**: caráter clássico, mas
  não dá as três saídas simultâneas nem a relação-como-processo. Candidato
  a um módulo `LADDER` separado depois (também está no Top 100 - AJH,
  Rossum Evolution).
- **Banco de 16 bandas (SPECTRA / ResEQ)**: rico, mas é outro módulo
  (família espectral, PERCEPTION + inserts atravessáveis) - candidato
  próprio, não o filtro básico.
- **Oversampling no drive já no marco 1**: mais caro e mais código;
  sine-shaping suave alia pouco em nível moderado. 2ª camada.
- **Morphing LP→BP→HP num único knob** (comum em software): o `spread`
  das três irmãs é mais expressivo e mais Rasgo.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** resposta em frequência de `low`/`center`/`high` com
`spread=0` bate com LP/BP/HP teóricos (±1 dB na banda passante, inclinação
correta); `spread` alto produz três picos separados nas frequências
previstas; `resonance→1` auto-oscila numa senoide estável na frequência de
corte (±1%); `drive` alto: THD medido, alias abaixo de -35 dB em nível
moderado; `cutoff` varrido de 20 a 20k sem estouro nem NaN; determinismo
byte-idêntico entre renders; sem alocação (teste).

**Escuta:** varrer `spread` soa como uma transformação contínua "filtro →
formante"? auto-oscilação soa musical ou estridente? o `drive` engrossa ou
só suja? Módulo 1 → `cutoff_mod` deste filtro → já dá vontade de mexer?

## 8. Integração e painel

Classe `Signal` (`type()` = `"FILTER"`), 4 entradas, 4 saídas, 4
parâmetros. `panel()` próprio: display de resposta de frequência ao vivo +
CUTOFF/RESONANCE/SPREAD/DRIVE + jacks. Testado isolado (varredura de
resposta, auto-oscilação, drive) antes do patch. Entra num render:
Módulo 1 (voz) → `FILTER` (cutoff modulado pelo LFO) → saída, com `spread`
variando — o segundo fragmento de música, agora com timbre em movimento.
