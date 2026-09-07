# Dossiê — Módulo 60: Vocoder de N bandas (`VOCODER`)

**Família:** TRANSFORM (junto de `FILTER`/`FORMANT`)
**Estado:** **implementado — Onda F** (2026-09-08)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Vocoder.hpp`, `tests/test_vocoder.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.6` (Tier 2) — Onda F. Homer Dudley,
*The Vocoder* (Bell Labs, 1938 — domínio público)

## Estado da implementação

O canal vocoder de Dudley: a envoltória de energia de um **MODULADOR**
(voz, fala) por banda de frequência controla o ganho da mesma banda numa
**PORTADORA** (um sinal rico — serra, pad, ruído). A portadora "fala".

O `FORMANT` (#45) ganhou um `mode` vocoder de **5 bandas** (as
ressonâncias de vogal — vocálico, não fala de banda larga). `VOCODER` é
o **dedicado**: até **20 bandas** log-espaçadas de 80 Hz a 8 kHz → fala
inteligível.

- **`bands`** (4–20, def 16): quantas bandas. Poucas = "robô" grosso;
  16–20 = inteligível.
- **`shift`** (−1..1): desloca as frequências da SÍNTESE em relação às
  da análise (`2^(shift·1,2)`) — *formant shift*: voz maior/menor sem
  mudar a fala.
- **`attack`** (0–1 → 1–60 ms) / **`release`** (0–1 → 20–600 ms): tempos
  dos seguidores de envelope por banda. Release curto = staccato
  inteligível; longo = as sílabas borram (pad falado).
- **`sibilance`** (0–1, def 0,35): quanto do agudo do modulador
  (> ~3,5 kHz, passa-alta) vai DIRETO pra saída — as fricativas (s, f,
  ch, x) que a análise de banda não representa bem.
- **`freeze`** (0/1): congela as envoltórias — a portadora fica
  "falando a última sílaba" para sempre. Tirar o `mod` depois de
  congelar deixa o pad.
- **`mix`** (0–1): portadora seca ↔ vocodada (0 = bypass bit-exato).

**Entradas:** `carrier`, `mod`, `pitch` (CV pra a portadora interna).
**Saída:** `out`. **`carrier` livre → uma serra interna** (110 Hz ·
`2^pitch`, + um fio de ruído — o "sopro" da voz) fala sozinha a partir
do `mod`. `process()` não aloca. Determinístico byte a byte.

**DSP:** por banda `k` (freq `f_k = 80·(f1/80)^(k/(N-1))`, largura
constante em oitavas `bwOct = 6,64/N`):
- **análise:** SVF TPT passa-faixa no `mod` (ganho normalizado por `·q`)
  + seguidor de envelope (ataque/release por `attack`/`release`);
- **expansão pra baixo** (~2:1): `ge = env · clamp(env·7, 0,05, 1)` — o
  piso de ruído do modulador não "abre" a banda (as consoantes ficam, o
  chiado de fundo não);
- **síntese:** SVF TPT passa-faixa na PORTADORA em `f_k · 2^(shift·1,2)`
  (ganho normalizado), multiplicado por `ge`;
- soma das bandas × makeup, + a sibilância (passa-alta do `mod` × `sib`),
  `tanh` de segurança, `mix` com a portadora seca.

**Testes (Debug + Release):** portadora de ESPECTRO PLANO (ruído) +
modulador seno 1200 Hz → a saída concentra a energia em 1050–1400 Hz
(≥ 2,5× o resto — o vocoder MOLDA o espectro); modulador de "sílabas"
(ruído + envelope 4 Hz) → a saída segue a envoltória (RMS na sílaba
> 2× no vão); `shift` positivo sobe a re-síntese; `freeze` + `mod`
removido → continua soando na mesma banda; sem `carrier` → a serra
interna fala; `mix=0` = bypass byte-exato; `jitter` — não tem (só o fio
de ruído da serra interna, semeado); dois renders byte-idênticos (com e
sem portadora); `bands=20` + tudo no talo → |out| < 1,1.

**Pendências (candidatos):** entrada `formant hold` (congelar SÓ as
frequências, deixar as amplitudes seguirem); banco de análise por FFT
(mais bandas, menos CPU que 40 SVF); *voiced/unvoiced* detection real
(hoje a sibilância é um blend fixo, não comuta); *pitch tracking* do
modulador → afina a portadora interna direto (falar cantando);
`bands` como `mode` de escala (bandas em intervalos musicais).

---

## 1. Problema musical e papel no fluxo

O vocoder é um dos efeitos mais reconhecíveis da música eletrônica
(Kraftwerk, Wonder, Daft Punk, ELO). O RASGO tinha o *mode* de 5 bandas
no `FORMANT` — bom pra o "coro que fala vogais", ruim pra fala
inteligível. `VOCODER` cobre o caso banda larga.

No fluxo: TRANSFORM. Quase sempre `SIGNAL-IN` (ou `SAMPLER`) → `mod`, e
um `OSC.saw`/`CHORD`/pad → `carrier`. Casos:
- **voz robô clássica:** serra no carrier, `bands` 12–16.
- **pad que fala:** `HALL` ou `ADDITIVE` no carrier, `release` alto.
- **percussão tonal:** `DRUM` no `mod` — cada batida "abre" a portadora
  no seu espectro → um pad ritmado pela bateria.
- **freeze:** uma palavra congelada vira um drone com aquela cor de
  vogal.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

- **Homer Dudley**, *The Vocoder* (Bell Labs Record, 1939) + *Remaking
  Speech* (JASA, 1939) — o canal vocoder: banco de análise (passa-faixas
  + retificador + passa-baixa) × banco de síntese. **Domínio público**
  (patente de 1938 expirada há décadas).
- **Banco de filtros de Q constante** — a base de todo vocoder de banda;
  teoria pública.
- **Seguidor de envelope** = retificação + passa-baixa (RC), com
  ataque/release assimétricos — clássico.
- **Sibilância / unvoiced passthrough** — os vocoders de estúdio (EMS
  5000, Roland VP-330, Moog) mandam o agudo do modulador direto pra as
  fricativas; técnica pública.
- **SVF TPT** (Simper/Cytomic "Solving the continuous SVF") — o mesmo
  filtro do `FILTER`/`WASP`/`FORMANT` do RASGO, reaproveitado.

**Desvio Rasgo:** a portadora interna autônoma (serra + sopro semeado —
o vocoder fala sozinho sem carrier cabeado); a expansão-pra-baixo por
banda (segue a envoltória mais fielmente que um vocoder analógico, sem o
"chão" de ruído).

## 3. Modelo

Ver o bloco DSP no "Estado da implementação". `q ≈ 1/Q` derivado da
largura de banda alvo em oitavas:
`q = ½·(2^bwOct − 1)/2^(bwOct/2)`. `atkC/relC` = coeficientes de
one-pole a partir de `attack`/`release` (quadrático). `synMul =
2^(shift·1,2)`.

**Extremos:** `bands=4` — 4 bandas largas, quase irreconhecível como
fala. `bands=20` — perto do limite de inteligibilidade de um vocoder de
banda. `mod` = ruído branco → todas as bandas "abrem" → a portadora
passa quase inteira (o vocoder vira quase bypass). `mod` silencioso →
saída silenciosa (nada abre as bandas). `carrier` senoidal puro → só as
bandas perto da senoide têm o que passar → fala fina/nasal.

## 4. Três modos obrigatórios

1. **vocoder** — `carrier` + `mod` cabeados: a portadora fala.
2. **autônomo** — `carrier` livre: serra interna (afinável por `pitch`)
   + sopro semeado; fala a partir só do `mod`.
3. **congelado** — `freeze=1`: as envoltórias das bandas param; a
   portadora sustenta a última cor espectral (mesmo tirando o `mod`).

## 5. Portas, parâmetros, limites

**Entradas:** `carrier` (Audio), `mod` (Audio), `pitch` (Control, v/oct).
**Saída:** `out` (Audio).
**Parâmetros:** `bands` (4–20, def 16), `shift` (−1..1, def 0),
`attack` (0–1, def 0,15), `release` (0–1, def 0,35), `sibilance`
(0–1, def 0,35), `freeze` (0/1, def 0), `mix` (0–1, def 1).
**Limites:** `out` com `tanh` de segurança. CPU: 2·`bands` SVF + `bands`
seguidores + 1 passa-alta por amostra (`bands=16` → ~34 SVF/amostra ≈
poucos % de um núcleo). Sem alocação.

## 6. Alternativas descartadas

- **estender o `mode` do `FORMANT` pra N bandas** — o `FORMANT` tem 5
  SVF fixos e uma tabela de vogal `constexpr`; N variável + banco de
  análise separado descaracteriza o módulo. Módulo próprio.
- **análise por FFT já** — mais bandas por menos CPU, mas latência de
  bloco e uma FFT no core. Fica como pendência (o mesmo debate do
  `SPECTRA`).
- **voiced/unvoiced com comutação dura** (o modulador ou é tom ou é
  ruído) — mais fiel à fala mas "pisca"; o blend contínuo de sibilância
  é mais musical.
- **saída estéreo** — um vocoder é mono por natureza (a fala é mono); o
  espaço vem depois (`SWIRL`/`HALL`).

## 7. Critérios técnicos e perguntas de escuta

- portadora plana + modulador tonal → a saída tem o pico na banda do
  modulador (≥ 2,5× o resto).
- modulador de sílabas → a saída segue a envoltória (RMS sílaba > 2×
  vão).
- `shift` transpõe a re-síntese; a análise não muda.
- `freeze` sustenta mesmo sem `mod`.
- sem `carrier` → a serra interna fala.
- `mix=0` = bypass byte-exato.
- determinístico (com e sem portadora).
- `bands=20` + `shift/sibilance` no talo + entrada rica → |out| < 1,1.
- **escuta:** `SIGNAL-IN` (fala) → `mod`, `OSC.saw` → `carrier`,
  `bands` de 4 a 20 — de "robô alienígena" a "quase inteligível".
  `freeze` numa vogal — um drone com aquela cor. `DRUM` → `mod` — um pad
  ritmado.

## 8. Integração e painel

**Catálogo:** TRANSFORM, junto de `FORMANT` (`FILTER · FORMANT · VOCODER
· RESONATOR · …`). **Painel:** 14 HP — BANDS/SHIFT/SIBIL (linha 1),
ATK/REL/MIX (linha 2), FRZ (toggle); jacks CAR/MOD/PIT + OUT.
**LEARN:** 11 binds (3 níveis) + a definição do módulo.
**CTest:** `rasgo_modular_vocoder_tests` (8 testes).
