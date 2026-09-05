# Dossiê — Módulo 27: Campo de deriva (`DRIFT`)

**Família:** DECISION / UTILITY
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Drift.hpp`, `tests/test_drift.cpp`

## Estado da implementação (marco 3)

**Modulação transforma o presente; deriva transforma o que o
instrumento considera seu estado normal.** (brief `CRI-DRF-001` do
ANTITOTEM, `docs/PESQUISA_DERIVA_GENERATIVA.md`.) O Rasgo Modular tinha
modulação (LFO, S&H, envelope) e `drift` interno por módulo, mas
faltava a **deriva estrutural lenta** — a que muda de andamento em
minutos os parâmetros que o patch trata como fixos.

- **campo compartilhado** — um LFSR de 8 bits avançado devagar (a
  "cadência da deriva") + um **passeio com momentum**: a velocidade
  ACUMULA (`vel += impulso; vel *= inércia`), então uma tendência tende
  a continuar (ANTITOTEM: *"se está aumentando densidade, tende a
  continuar"*);
- **4 saídas `a`/`b`/`c`/`d`** — cada uma lê o MESMO campo com pesos
  diferentes (padrão `correlatedValues` do `BiomaBrain` do AQUORBIUM —
  *várias leituras levemente diferentes de um estado único*, não
  geradores independentes) + o próprio LFO senoidal lento numa razão de
  frequência distinta. `stride` (0–1): de "andam juntas" (0) a "cada uma
  pro seu lado" (1);
- **`anchor`** (0–1, def 0, 2026-09-04) — **memória de topologia**: a
  cada 8 tiques grava o estado (LFSR + campo) como "marco"; em cada
  tique, com probabilidade ∝ `anchor²`, VOLTA pro marco em vez de dar um
  passo novo. A deriva passa a ORBITAR paisagens em vez de vagar pra
  sempre. `anchor = 0` → nunca grava nem volta (idêntico ao antigo);
- **`field`** — o passeio com momentum cru (a "meteorologia" do patch);
- **`event`** — pulso a cada tique da deriva (patchável como relógio
  lento / trigger de mudança);
- **`advance`** (entrada trig) — se conectada, a deriva anda na borda ↑
  (cadência por compasso/frase, estilo `deriveFromMemory()` do
  ANTITOTEM) em vez do relógio interno `rate`.

Uso: `DRIFT.a → connectToParameter(FILTER.cutoff)`,
`DRIFT.b → SPACE.size`, `DRIFT.c → CLOCK.fill`… — o `seedPatch` já pluga
2–4 saídas em parâmetros estruturais por caráter, então **todo patch de
seed evolui sozinho**.

Um stream xorshift semeado em `prepare()` → determinístico (dois renders
byte-idênticos). Sem alocação / lock / IO em `process()`.

**Testes (12 funções, Debug + Release):** `anchor` alto → a trajetória
difere da de `anchor=0` e o campo vaga menos (desvio-padrão ≤),
determinístico e limitado; as saídas se movem devagar
(derivada por segundo pequena — deriva, não modulação de áudio);
`depth = 0` → saídas presas em `bias`; `rate` maior → mais tiques por
segundo (contagem de `event`); `momentum` alto → autocorrelação da
velocidade maior (tendências persistem) vs `momentum = 0` (passeio quase
branco); `stride = 0` → as 4 saídas altamente correlacionadas,
`stride = 1` → descorrelacionadas; `advance` conectada → anda só na
borda (nº de mudanças = nº de bordas); `bias` desloca o centro; tudo em
[−1, 1] e finito; dois renders byte-idênticos; grafo
`CLOCK → DRIFT.advance` · `DRIFT.a → FILTER.cutoff` (o corte anda em
minutos).

**Pendências (candidatos):** `event` com probabilidade (nem todo
tique dispara); deriva cruzada explícita (uma saída empurra o `depth`
de outra); dois campos com acoplamento (constelação de derivas).

---

## 1. Problema musical e papel no fluxo

Um patch generativo bom sem gesto humano ainda "anda em círculos" depois
de um tempo: o `CLOCK` tica igual, o `FILTER` fica no mesmo corte, a
`HARMONY` roda mas o resto é estático. Falta o **desenvolvimento lento**
— o instrumento se comportar diferente aos 30 s e aos 4 min, sem
repetir nem saltar.

`DRIFT` é essa camada: uma fonte de CV que se move em escala de minutos,
com memória (momentum) e correlação (as 4 saídas contam a mesma
história de ângulos diferentes). Papel: fonte de controle estrutural.
`DRIFT → cutoff/ressonância/fill/lock/tamanho de sala/detune` via cabo
ou `connectToParameter`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **ANTITOTEM `deriveFromMemory()` / brief `CRI-DRF-001`** (`docs/PESQUISA_DERIVA_GENERATIVA.md`) | deriva = deslocamento de estado com MOMENTUM (velocidade acumula e retroalimenta a intensidade), cadência por loop, memória de topologia | código do autor |
| **AQUORBIUM `BiomaBrain::nextDrift` / `correlatedValues`** | um LFSR compartilhado; cada saída = soma PONDERADA DIFERENTE dos mesmos bits → correlacionadas mas distintas; + LFO próprio por saída (`wave·0.72 + heldCorr·0.28`) | código do autor |
| **`BuchlaRandomSource` / Buchla 266 "smooth random"** | tensão que passeia devagar entre alvos; várias leituras de um estado | conceito |
| **Brownian / random walk limitado** | passeio com teto, retorno suave ao repouso | teoria pública |

## 3. Modelo — matemática, estados, extremos

Constantes: `ratio[4] = {1.0, 1.37, 1.83, 2.41}` (razões de LFO
distintas), `weight[4][8]` (padrões de peso distintos, somam ~1),
`lfsrTaps = 0xB8` (x^8 + x^6 + x^5 + x^4 + 1).

Por amostra:
```
rateHz = clamp(rate + rate_mod, 0.001, 4)
tick = advance conectada ? borda↑(advance)
     : (clockPhase += rateHz/sr) >= 1  (wrap)

if tick:
  # LFSR de 8 bits
  fb = parity(reg & lfsrTaps) ; reg = ((reg << 1) | fb) & 0xFF ; if reg==0 reg=1
  # passeio com momentum
  impulse = (nextWhite()) * depth * 0.09
  vel = vel·(0.45 + 0.5·momentum) + impulse·(1 − 0.55·momentum)
  field += vel
  field += (bias − field) · (0.03 + (1−momentum)·0.14)    # retorno; fraco c/ momentum
  field = clamp(field, −1, 1)
  for k: heldCorr[k] = (Σ_i bit_i·weight[k][i]) · 2 − 1     # leitura ponderada, [-1,1]
  eventTimer = 0.02·sr                                       # pulso de `event`

# contínuo (por amostra), com glide entre tiques:
for k in 0..3:
  phase[k] += (rateHz·0.35·ratio[k]) / sr ; wrap
  own = sin(phase[k]·2π)
  raw = field·(0.72 − 0.42·stride) + own·(0.22 + 0.5·stride) + heldCorr[k]·0.16
  target[k] = bias + depth · clamp(raw, −1, 1)
  out[k] += (target[k] − out[k]) · glideCoef                 # ~2 s
fieldOut += (bias + depth·field − fieldOut) · glideCoef
event = eventTimer-- > 0 ? 1 : 0
```

**Estados:** `reg` (LFSR), `field`, `vel`, `phase[4]`, `heldCorr[4]`,
`out[4]`, `fieldOut`, `clockPhase`, `prevAdvance`, `eventTimer`, `rng_`.
Sem alocação.

**Extremos.** `depth = 0` → tudo colado em `bias` (deriva desligada,
`DRIFT` vira offset). `rate` no teto (4 Hz) → deixa de ser deriva e vira
um LFO/S&H irregular (limite aceitável). `momentum = 1` → o passeio pode
encostar e "grudar" num extremo por muito tempo (é o ponto: uma
tendência longa) — o `clamp` segura. `advance` conectada a um clock
rápido → a deriva anda por passo (fica nervosa) — o glide de ~2 s
segura a saída suave mesmo assim. Reset → tudo zerado, `reg`/`rng`
re-semeados, `out[k] = bias`.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado; `rate` baixo + `depth` médio → 4 CVs que
  passeiam devagar em torno de `bias`, contando a mesma história de
  ângulos diferentes. É o modo principal.
- **Performance:** `depth`/`stride` ao vivo = de "os parâmetros mal se
  mexem" a "o patch inteiro respira em direções diferentes"; `bias`
  desloca o centro de tudo que a deriva controla.
- **Híbrida:** `CLOCK`/`SEQUENCE` no `advance` → a deriva anda em
  compassos/frases (cadência musical, não contínua); `event` → dispara
  uma mudança (troca de passo de sequência, re-lock de um `TURING`).

## 5. Portas, parâmetros, limites

**Entradas:** `advance` (Control, trig), `rate_mod` (Control).
**Saídas:** `a`, `b`, `c`, `d`, `field`, `event` (todas Control).
**Parâmetros:** `rate` (0,002–1 Hz, def 0,05), `depth` (0–1, def 0,5),
`momentum` (0–1, def 0,5), `stride` (0–1, def 0,45), `anchor` (0–1,
def 0), `bias` (−1…+1, def 0).
**Limites:** saídas em [−1, 1]. CPU: 1 `sin` por saída por amostra
(4 total) + aritmética; o LFSR/passeio só no tique. Sem alocação.

## 6. Alternativas descartadas

- **4 random-walks independentes:** dá ruído descorrelacionado; o valor
  do módulo É a correlação (uma história, N ângulos) — padrão
  `BiomaBrain`.
- **`random()` puro por tique:** sem memória. A deriva do brief
  `CRI-DRF-001` é explicitamente *deslocamento de estado*, não sorteio.
- **Só o LFSR (sem o passeio de momentum):** o LFSR dá correlação mas
  não "tendência que persiste". O momentum é o que faz aos 4 min ≠ aos
  30 s.
- **Memória de topologia no marco 1:** re-rotear cabos é do painel (uma
  "deriva de topologia" pós-review), não deste módulo de CV. Candidato.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** |Δ saída por segundo| pequeno em `rate` baixo (é deriva,
não modulação); `depth = 0` → saída == `bias` (±1e-4); nº de pulsos
`event` ∝ `rate`; `momentum` alto → autocorrelação lag-longo da série de
`field` maior que `momentum = 0`; correlação entre `a` e `d`: alta em
`stride = 0`, baixa em `stride = 1`; `advance` conectada → o campo só
muda na borda ↑ (nº de patamares = nº de bordas + 1); `bias` desloca a
média; tudo em [−1, 1] e finito; dois renders byte-idênticos.

**Escuta:** com `DRIFT.a → FILTER.cutoff`, o timbre "abre e fecha em
minutos" de um jeito que parece intenção, não LFO? as 4 saídas em 4
destinos: dá pra ouvir que "combinam" (mesma história) sem soar
idênticas? `momentum` alto: o patch "insiste" numa direção antes de
virar? `event` re-lockando um `TURING`: a mudança cai num ponto que
faz sentido?

## 8. Integração e painel

Classe `Drift` (`type()` = `"DRIFT"`), 2 entradas, 6 saídas, 6
parâmetros. `panel()` próprio (10 HP): knobs RATE/DEPTH/MOMT/STRD/
BIAS/ANCHR (2 colunas), display cheio, e **3 fileiras de jacks**
(passe de ergonomia 2026-09-06): entradas ADV/RATE · 4 saídas de campo
correlacionadas A/B/C/D · campo bruto FLD + gatilho de virada EVT.
Testado isolado (velocidade, momentum, correlação, cadência por
`advance`, determinismo) antes do patch.
Cadeias canônicas: `DRIFT.a → connectToParameter(FILTER.cutoff)` ·
`DRIFT.b → SPACE.mix` · `CLOCK → DRIFT.advance`. O `seedPatch` pluga
2–4 saídas em parâmetros estruturais por caráter — todo patch de seed
passa a **evoluir sozinho**. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família DECISION — junto do `DECISION`).
