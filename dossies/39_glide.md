# Dossiê — Módulo 39: Portamento por nota (`GLIDE`)

**Família:** UTILITY / PITCH
**Estado:** **implementado — Onda A** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Glide.hpp`, `tests/test_glide.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda A, #39); gap do §6 (baixo acid)

## Estado da implementação

O `CONTROL` já tem um `slew` — mas é um lag RC sempre ligado, bom pra
seguidor de envelope e pra amaciar um offset, não pra a **condução de
melodia**. `GLIDE` é o primitivo que faltava: um slew que **decide por
nota se escorrega ou salta**, com subida ≠ descida e três modos.

- **`pitch`** (CV, 1 V/oct ou qualquer) → **`out`** = a mesma CV,
  deslizada;
- **`time`** (0–2 s) = tempo de uma mudança de 1,0 (uma oitava) na
  **subida**;
- **`fall`** (−1..1) = assimetria: tempo de descida = `time · 6^fall`
  (−1 = descida 6× mais rápida, +1 = 6× mais lenta, 0 = simétrico);
- **`curve`** (0–1) = inclinação **linear** (rate constante, MS-20 /
  Minimoog) ↔ **exponencial** (RC) — o contínuo, padrão do `ENVELOPE`;
- **`mode`** (0/1/2):
  - **0 — sempre**: portamento clássico, toda mudança escorrega;
  - **1 — slide-gated (303)**: escorrega só enquanto `slide` está alto;
    cada passo do sequenciador decide (o *slide* do TB-303);
  - **2 — legato**: escorrega só se `gate` continua alto na troca de
    nota; um `gate` novo (ataque destacado) faz saltar;
- **`moving`** (gate) = alto enquanto está deslizando;
- **`done`** (trig) = pulso de ~2 ms quando chega ao alvo — pra um
  downstream reagir a "chegou" (acento, ratchet, mudança de timbre).

Desvio Rasgo: os três modos num só módulo + `fall` assimétrico + a saída
`done`. Precisão importa (é régua de afinação) — **sem `drift`**.

Sem alocação / lock / IO em `process()`. Determinístico (sem RNG).

**Testes (Debug + Release):** `mode 0` desliza toda mudança (derivada por
amostra limitada, chega monotônico); `time ~ 0` → salto seco;
`mode 1` sem `slide` → segue `pitch` amostra a amostra; com `slide` alto
→ desliza; `mode 2` no ataque (`gate` ↑) → salta, `gate` sustentado →
desliza; `fall = -1` → descida bem mais rápida que subida;
`curve` 0 vs 1 → trajetória linear vs exponencial (meia-vida);
`moving` alto durante o deslize, baixo parado; `done` pulsa uma vez na
chegada; sem `pitch` conectado → `out` mantém o último valor; tudo
finito; dois renders byte-idênticos.

**Pendências (candidatos):** `bend` (pequeno overshoot musical na
chegada); quantização do alvo (deslizar entre graus de escala); `time`
por CV com atenuverter; visual da trajetória no painel.

---

## 1. Problema musical e papel no fluxo

`SEQUENCE → QUANTIZER → OSC` toca notas em degraus secos. Um baixo acid,
uma linha de lead cantada, um TB-303 — todos vivem do **como uma nota
chega na outra**: o slide. Sem `GLIDE`, isso exigiria um `CONTROL` com
`slew` sempre ligado (perde o "salta quando quero, escorrega quando
quero") ou nada. `GLIDE` põe a decisão no fluxo: entra a CV de nota, sai
a CV conduzida, e o *quando escorregar* vem do `slide`/`gate` — do
próprio sequenciador.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Roland TB-303** — o *slide*: um bit por passo decide se a nota
  atual escorrega da anterior; tempo de slide fixo (~60 ms). `mode 1` +
  `time` generaliza (tempo ajustável, entrada de gate).
- **portamento MS-20 / Minimoog** — slew de **inclinação constante**
  (rate, não meia-vida): notas distantes levam mais tempo, o "escorregão"
  tem velocidade fixa. `curve = 0`.
- **glide RC clássico** — 1 polo passa-baixa: aproximação exponencial,
  meia-vida constante, o começo do escorregão é rápido e a chegada
  arrasta. `curve = 1`.
- **portamento legato (mono synths)** — só escorrega entre notas
  ligadas; `mode 2`.
- **Bela Gliss, glide processor (EMW)** — glide como módulo dedicado com
  rise/fall separados.

## 3. Modelo — por amostra

```
alvo   = pitch (ou último valor se sem cabo)
salta:
  mode 0 → nunca
  mode 1 → slide < 0,5
  mode 2 → gate subindo  OU  gate < 0,5
se salta:            y = alvo
senão:
  d = alvo − y
  t = d ≥ 0 ? time : time·6^fall
  se t ≤ dt:         y = alvo
  senão:
    linStep = dt/t                    (mudança de 1,0 por t s)
    lin  = y + clamp(d, −linStep, +linStep)
    expo = y + d·(1 − e^(−dt/t))
    y    = lin + (expo − lin)·curve
moving = |alvo − y| > 1e-4
done   = (moving caiu pra 0 agora) → pulso de ~2 ms
```

**Extremos:** `time = 0` → salto seco (o `t ≤ dt` pega). `time = 2 s`,
`fall = 1` → descida de ~12 s (glide muito lento). `d` enorme (pulo de
várias oitavas) → o mesmo `t` (inclinação constante em `curve = 0`
significa mais tempo real; RC em `curve = 1` mantém a meia-vida). `pitch`
sem cabo → `alvo = y` → `out` congela no último valor (não salta pra 0).

## 4. Três modos obrigatórios

- **autônoma:** `mode 0`, `time` médio — todo patch `SEQUENCE → GLIDE →
  OSC` ganha condução; sem nenhum cabo em `slide`/`gate` funciona.
- **performance:** `time`, `fall` e `curve` são os macros; `mode` troca o
  caráter (portamento contínuo ↔ slide de sequência ↔ legato).
- **híbrida:** `slide` vindo de uma linha do `TRIGSEQ` ou do `TURING` —
  o *quando escorregar* também é generativo.

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Audio), `slide` (Control/gate), `gate`
(Control/gate).
**Saídas:** `out` (Audio), `moving` (Control/gate), `done` (Control/trig).
**Parâmetros:** `time` (0–2 s, def 0,08), `fall` (−1..1, def 0),
`curve` (0–1, def 0,3), `mode` (0–2, def 0).
**Limites:** `out` segue a faixa da entrada (não clampa — é CV de
pitch). CPU: por amostra 1 `exp` (só quando desliza) + 1 `pow` por
bloco. Sem alocação.

## 6. Alternativas descartadas

- **modo dentro do `CONTROL`** — encheria um módulo de precisão de
  lógica de gate; e o `GLIDE` quer 3 modos + 2 saídas de estado.
- **slew só linear** — perde o caráter RC (o "arrasto" na chegada que
  define o portamento de sintetizador).
- **`bend`/overshoot no v1** — musical, mas é enfeite; entra depois.

## 7. Integração e painel

10 HP: display da trajetória (cheio), knobs `TIME`/`FALL` e
`CURVE`/`MODE` (2×2), entradas `PITCH`/`SLIDE`/`GATE` numa fileira,
saídas `OUT`/`MOV`/`DONE` na de baixo. Catálogo: família **TRANSFORM**
(junto do `CONTROL`/`MULT`/`SH`).

Cadeia canônica: `SEQUENCE.pitch → GLIDE.pitch → QUANTIZER → OSC`;
`SEQUENCE.gate → GLIDE.gate` (legato) ou uma linha de `TRIGSEQ → GLIDE.slide`
(slide generativo).
