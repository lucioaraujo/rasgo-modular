# Dossiê — Módulo 32: Filtro áspero (`WASP`)

**Família:** TRANSFORM
**Estado:** **implementado — marco 3** (2026-09-04)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Wasp.hpp`, `tests/test_wasp.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O `FILTER` (Módulo 2) é o multimodo LIMPO — SVF TPT de precisão, três
saídas na mesma frequência, `spread` de formante, `tanh` suave. Faltava
o oposto: **um filtro de 12 dB com grão**, o caráter do EDP Wasp (1978)
— onde os "amp-ops" do Sallen-Key são na verdade **inversores CMOS 4069**
que ceifam DURO e ASSIMÉTRICO quando empurrados. É a voz que grita.

- **12 dB/oitava** (2 polos) — inclinação mais gentil que o ladder; o
  caráter vem da distorção, não da inclinação;
- **`grit`** (0–1) = quanto o joelho do ceifamento no laço abaixa —
  0 = quase-limpo (joelho em ±1, como o `FILTER`), 1 = joelho em ±0,2
  ("buzz" de inversor CMOS) + estágio de saída com ganho ×3;
- **`bias`** (−1..1) = assimetria do ceifamento **na entrada** (o
  inversor real não comuta em Vdd/2 exato) → harmônicos pares; um
  bloqueador de DC na saída tira o offset resultante;
- **auto-oscilação**: `resonance` alto empurra o amortecimento
  ligeiramente negativo; o SVF cresce e o ceifamento o prende num
  ciclo-limite. Um SVF de 2 polos filtra os próprios harmônicos, então
  o ciclo-limite *nu* fica perto de uma senoide (como o `FILTER`); o
  **estágio de saída** (o inversor de saída do Wasp, `grit`/`drive`
  ceifam DEPOIS do filtro, sem re-filtrar) é o que dá o rasgo reedy.
  O núcleo roda a 2× (antialias, 2026-09-04); uma auto-oscilação
  *francamente* palhetada ainda exige um modelo de inversor mais fiel —
  **pendência**;
- **corte estendido a ~24 kHz** — liberdade digital (o Wasp de hardware
  não chegava lá);
- **`mode`** (0–1) = LP ↔ BP ↔ HP contínuo (o Wasp tinha chave LP/HP);
- **`drive`** = ganho de entrada, empurra a não-linearidade;
- **`drift`** = wobble lento do corte (±0,15 oitava, escala de dezenas
  de segundos — assinatura RASGO).

Núcleo: SVF TPT/trapezoidal de Simper/Cytomic (paper público, já
reescrito no `FILTER`) — mesma base estável, **não-linearidade muito
mais agressiva e assimétrica** no laço. **NÃO** consultado: service
manual da Doepfer (cliente-only) nem o plugin VCV "Doepfer"
(proprietário) — ver a verificação de fontes em `PESQUISA_MODULOS.md §2.2`.

Sem alocação / lock / IO em `process()`. Determinístico (o `drift` é um
seno, sem RNG).

**Testes (14 funções, Debug + Release):** LP corta o agudo (RMS de
senoide a 3 kHz com corte a 300 Hz << RMS a 150 Hz); HP corta o grave;
`mode` varre LP→HP (a razão agudo/grave da saída cresce monotônica);
`resonance` 0,9 → pico em torno do corte (RMS de senoide na fc >> acima);
`resonance` no teto sem entrada → **auto-oscila** (saída não-nula,
finita, limitada) e `grit` muda o ciclo-limite (amplitude/forma);
`grit` alto → achata a forma de onda (menos fator de crista) no mesmo
sinal; `bias` ≠ 0 → **2º harmônico** (par) sobe e a saída tem média ≈ 0
(bloqueador de DC funciona); `drive` alto → satura mas não estoura
(finito, |out| ≤ 0,96); `cutoff_mod` +1 oitava dobra a fc efetiva;
**alias < 2 % da fundamental** com `drive`/`grit` altos (núcleo a 2×);
nunca produz NaN/Inf mesmo com resonance=1 + drive=8 + grit=1 + bias=−1;
dois renders byte-idênticos; grafo `OSC → WASP → MASTER`.

**Oversampling (feito — 2026-09-04):** o núcleo não-linear (wsat de
entrada + SVF TPT com wsat no laço + mistura de modo + wsat de saída)
roda a **2×** pelo `src/dsp/Oversampler.hpp` (upsample linear + FIR
meia-banda de 13 taps); `g`/`a1` recalculados com `sr·2`. O bloqueador
de DC + `softLimit` ficam no rate base. Medido: piso de alias em bins
não-harmônicos **0,03–0,08 % da fundamental** (−62 a −70 dB) mesmo com
`drive` 8 + `grit` 1. Atraso de grupo ~2,5 amostras. Determinístico
(FIR + aritmética, sem RNG).

**Pendências (candidatos):** modelo de inversor CMOS mais fiel (curva
de transferência medida, não a soft-clip `1−e^{-x}`) → auto-oscilação
francamente palhetada; modo do `FILTER` em vez de módulo à parte;
`fold` no laço (Wasp com realimentação extrema).

---

## 1. Problema musical e papel no fluxo

Nem todo filtro deve ser transparente. O Wasp é procurado justamente
pelo que faz de "errado": distorce quando ressoa, a auto-oscilação range,
empurrar o `drive` transforma o filtro num waveshaper com corte.
É a peça para bass agressivo, leads que cortam, drones que rangem — o
contraponto sujo do `FILTER` limpo.

Papel: transformação de timbre com caráter. `OSC → WASP → VCA`; `NOISE →
WASP` com `resonance` alto = voz percussiva afinada (o ciclo-limite
sujo); `ENVELOPE.env → cutoff_mod` = o clássico envelope de filtro, mas
com grão. No `seedPatch` v2: destino de `audio`, fonte de `voice`/`bus`;
`cutoff_mod`/`res_mod` são destinos de `mod`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Circuito do EDP Wasp** (1978, Chris Huggett) — análises independentes (René Schmitz, fóruns DIY); Befaco "Sallen-Key" é parente aberto | topologia Sallen-Key com inversores CMOS como estágio de ganho; ceifamento duro/assimétrico das etapas de ganho | esquema independente / DIY — conceito público |
| **Inversor CMOS 4069 como amplificador** (teoria) | curva de transferência íngreme em torno de Vdd/2, ganho alto, satura pros trilhos; assimetria NMOS/PMOS | teoria pública |
| **SVF TPT não-linear** (Simper/Cytomic 2013; Zavalishin "The Art of VA Filter Design") | integração trapezoidal ZDF; não-linearidade no estado prende a auto-oscilação num ciclo-limite | paper/livro públicos, já reescrito no `FILTER` |
| **`saturate`/`softLimit` do `FILTER` do Rasgo** | não-linearidade no laço + limitador de saída suave; `oscPush` que leva `k` a negativo | código do autor |

## 3. Modelo — matemática, estados, extremos

Suavização de `cutoff`: 1 polo, τ = 5 ms. `drift` (1 seno/bloco):
`driftPhase += 2π·0.03·(bloco/sr)` ; `driftOct = drift·0.15·sin(driftPhase)`.

Por amostra (por canal):
```
fc = clamp(smoothCutoff · 2^(cutoff_mod + driftOct), 20, 0.45·sr)
res = clamp(resonance + res_mod, 0, 1)
q   = 0.5 · 2^(res·8)
oscPush = res > 0.88 ? (res − 0.88)·8 : 0
k   = 1/q − oscPush²·0.05        # amortecimento → negativo só perto de res=1
g   = tan(π·fc/sr) ;  a1 = 1/(1 + g·(g + k))
th  = 1 − grit·0.8              # joelho do ceifador ;  outGain = 1 + grit·2

# entrada: ceifador ASSIMÉTRICO (o inversor de entrada)
xin = wsat(in · (1 + (drive−0.1)·4.5), th, bias)

# SVF TPT, ceifador no estado (prende o ciclo-limite)
v1 = a1·(ic1 + g·(xin − ic2))
v2 = ic2 + g·v1
ic1 = 2·wsat(v1, th, bias) − ic1
ic2 = 2·wsat(v2, th, bias) − ic2
lp = v2 ; bp = v1 ; hp = xin − k·v1 − v2

# morph de saída + ESTÁGIO DE SAÍDA (ceifa depois do filtro — não re-filtra,
# então é o que dá o buzz reedy e o grão ao áudio)
y = mode<0.5 ? lerp(lp, bp, mode·2) : lerp(bp, hp, (mode−0.5)·2)
y = wsat(y · outGain, th, bias)
out = dcBlock(y) ;  out = softLimit(out)
```
`wsat(v, th, bias)`: teto `ceil = 1 ∓ bias·0.42` (+ pra v<0, − pra v≥0;
o inversor não comuta em Vdd/2 exato → harmônicos pares); `knee = th·ceil`;
`|v|≤knee → v` ; senão `sign(v)·(knee + (ceil−knee)·(1 − e^{−(|v|−knee)/(ceil−knee+0.05)}))`.
`dcBlock`: `y = v − x₁ + 0.9985·y₁`. `softLimit`: linear até ±0,85, `tanh` depois.

**Estados (por canal):** `ic1`, `ic2`, `dcX1`, `dcY1`. Globais:
`smoothCutoff`, `driftPhase`. Sem alocação.

**Extremos.** `resonance`=1, sem entrada → auto-oscila (k<0, o `wsat`
segura); `grit`=1 → o estágio de saída achata o ciclo-limite. `drive`=8 +
`grit`=1 + `resonance`=1 → o `wsat` no laço e o `softLimit` na saída
garantem finitude (testado). `bias`=±1 → assimetria forte na entrada,
mas o `dcBlock` mantém média ≈ 0. `cutoff` a 24 kHz + `sr` 44,1 k → o
`clamp(0.45·sr)` prende a fc abaixo de Nyquist. `mode`=0,5 exato → BP
puro. `grit`=0 → joelho em ±1 e `outGain`=1 = comportamento próximo do
`FILTER` (mas 2 polos, não 3 irmãs).

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado, `resonance` no default → silêncio
  (é filtro, não gera). Com `resonance` alto → auto-oscila numa nota
  suja na fc (drone). Aceitável: um filtro sem entrada só "toca" se
  resosc.
- **Performance:** `cutoff` + `resonance` são o gesto principal; `grit`
  de "quase-limpo" a "buzz"; `mode` troca LP/BP/HP ao vivo; `drive`
  empurra pro waveshaping.
- **Híbrida:** `ENVELOPE.env → cutoff_mod` (envelope de filtro com
  grão); `LFO → cutoff_mod`; `DRIFT.a → res_mod` (a ressonância deriva);
  `SEQUENCE.pitch → cutoff_mod` (o filtro segue a melodia — key-track).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `cutoff_mod` (Control, v/oct), `res_mod`
(Control).
**Saídas:** `out` (Audio).
**Parâmetros:** `cutoff` (20–24000 Hz, def 700), `resonance` (0–1, def
0,35), `mode` (0–1, def 0), `drive` (0,1–8, def 1), `grit` (0–1, def
0,45), `bias` (−1–1, def 0), `drift` (0–1, def 0).
**Limites:** saída limitada a ~±0,9 (`softLimit`). CPU: 1 `tan` + 2–3
`tanh` por amostra + 1 `sin` por bloco. Sem alocação, sem RNG.

## 6. Alternativas descartadas

- **Modo do `FILTER`:** o `FILTER` já tem 4 parâmetros e um propósito
  claro (multimodo limpo + formante). O grão do Wasp exige uma
  não-linearidade agressiva e assimétrica que descaracterizaria o
  `FILTER`. Fica candidato revisitar como modo se o painel pedir.
- **Sallen-Key digital "de verdade" (não-ZDF):** erro de afinação e
  instabilidade; o SVF TPT com `wsat` mais duro dá o caráter com
  estabilidade garantida (e o núcleo roda a 2× pro alias — 2026-09-04).
- **Curva de inversor CMOS medida:** mais fiel, mas exige a medição (não
  temos) ou o SPICE do circuito (não vamos). O `tanh` íngreme com
  joelho ajustável (`grit`) é a aproximação honesta.
- **Sem `dcBlock`:** o `bias` assimétrico injeta DC que empilha no laço
  e desloca o ponto de operação; o bloqueador é obrigatório.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `mode`=0 (LP), fc=300, senoide 2 kHz → RMS out < 0,2·RMS
in; senoide 150 Hz → RMS out > 0,7·RMS in; `mode`=1 (HP) espelha;
varrer `mode` 0→1 com ruído branco → razão (energia >2 kHz)/(energia
<500 Hz) cresce monotônica; `resonance`=0,9, senoide varrendo → pico
claro perto de fc; `resonance`=1 sem entrada → |out| > 0,03, finito e limitado, e
`grit` muda a amplitude/forma do ciclo-limite; `grit`=1 vs `grit`=0 no mesmo
seno no joelho → fator de crista menor (onda achatada); `bias`=0,85 (drive baixo) → 2º harmônico (par) sobe
(h2/h1 > 0,05) e |média(out)| < 0,02; `drive`=8 → |out| ≤ 0,95, finito;
`cutoff_mod`=1 → fc efetiva ×2; dois renders byte-idênticos.

**Escuta:** a auto-oscilação soa "reedy"/suja como um Wasp, não como
um seno limpo? `grit` no meio dá aquele bass áspero sem virar ruído?
`bias` acrescenta corpo (pares) sem sujar demais? empurrar o `drive`
com `resonance` média vira um waveshaper musical? o `drift` faz o
filtro "respirar" num drone longo?

## 8. Integração e painel

Classe `Wasp` (`type()` = `"WASP"`), 3 entradas, 1 saída, 7 parâmetros.
`panel()` próprio (~10 HP): `Display` (resposta), knobs CUTOFF/RESO/MODE
· DRIVE/GRIT/BIAS · DRIFT, jacks IN/FC/Q in, OUT out. Testado isolado
(LP/HP, mode, ressonância, auto-oscilação suja, grit, bias/DC, drive,
finitude) antes do patch. Cadeias canônicas: `OSC → WASP.in` ·
`ENVELOPE.env → WASP.cutoff_mod` · `WASP.out → VCA`. Adicionado ao
catálogo do painel (`apps/panel/ModuleCatalog.hpp`, família TRANSFORM,
junto de `FILTER`/`LPG`/`VCA`/`SHAPE`/`CONTROL`/`SH`/`PARAMETRIC`).
