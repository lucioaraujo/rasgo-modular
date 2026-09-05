# Dossiê — Módulo 18: Oscilador (`OSC`)

**Família:** SOURCE
**Estado:** **implementado — marco 3** (2026-09-02); `prox`
(profundidade/abafamento) adicionado em 2026-09-05 — ver `TAREFAS.md`,
registro "proximidade por oscilador"
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Oscillator.hpp`, `tests/test_oscillator.cpp`

## Estado da implementação (marco 3)

Feito: a **voz "neutra"** que faltava. O `FUNCTION` é gerador de função
(sem sync/sub/PWM); `MATTER`/`STRING` são vozes de caráter forte. O `OSC`
é o oscilador subtrativo clássico — várias formas ao mesmo tempo, 1 V/oct
preciso, para `SEQUENCE`→`QUANTIZER`→`OSC`→`FILTER`.

- **1 V/oct:** `f = freq · 2^(fine/1200) · 2^(pitch) · 2^(drift)`;
- **formas simultâneas** (5 saídas): `sine`, `tri`, `saw`, `pulse`, `sub`;
- **antialias:** PolyBLEP no wrap do `saw` e nas duas transições do
  `pulse` (0 e `pw`); o `tri` tem só quebra de 1ª derivada e alia bem
  menos (polyBLAMP fica como 2ª camada — §6);
- **PWM:** `pw` (0,02–0,98) + entrada `pwm`;
- **hard sync:** borda de subida em `sync` reinicia a fase do oscilador
  principal (o `sub`, que é flip-flop no wrap, acompanha);
- **sub-oscilador:** quadrada uma (`sub_2 = 0`) ou duas (`sub_2 = 1`)
  oitavas abaixo, por flip-flop no wrap da fase principal — fica sempre
  travado no principal, inclusive no sync;
- **FM linear through-zero:** entrada `fm` (áudio) soma Hz —
  `dp = (f + fm · fm_amount · 4f) / sr`, com `dp` podendo ficar negativo
  (a fase anda pra trás — TZFM de verdade, sem a "quebra" da FM de fase);
- **desvio Rasgo — `drift`:** passo aleatório lento e correlacionado na
  afinação (xorshift semeado em `prepare`), ±~½ semitom no máximo.
  `drift = 0` → saída determinística.

Determinístico por seed (dois renders byte-idênticos). Sem alocação em
`process()`, sem lock/IO.

**Testes (11/11, Debug + Release):** afinação (1 V/oct — 110/220/440 Hz
por autocorrelação, `pitch = +1` dobra); formas (seno ≈ senoidal, serra
rampa, pulso binível); PWM (razão cíclica de `pw` medida em `pw` 0,25 vs
0,75); sub-oitava (fundamental do `sub` = ½ ou ¼ do principal); hard sync
(saída periódica na taxa do `sync`); TZFM (limitada e finita com `fm`
forte; inalterada em `fm_amount = 0`); antialias (serra a 7 kHz: energia
de banda baixa pequena vs total); determinismo (byte-idêntico com
`drift` ligado); `drift` (afina passeia com `drift > 0`, estável em 0);
integração no grafo (`SEQUENCE → QUANTIZER → OSC → FILTER`); painel
fecha.

**Pendências (candidatos, não controles fictícios):** polyBLAMP nos
cantos do `tri` e do `sub` (2ª camada de antialias); oversampling 2× no
caminho não-linear da TZFM; `spread` — pilha de vozes desafinadas
(super-saw) como *relação entre as saídas* (padrão Rasgo `spread`/
`sweep`); wavefolding integrado; formas por wavetable morfável (Plaits);
sincronismo suave (soft sync).

---

## 1. Problema musical e papel no fluxo

Para tocar uma linha melódica escrita (`SEQUENCE`) numa escala
(`QUANTIZER`) com movimento harmônico (`HARMONY`), falta a ponta: um
oscilador que **rastreie 1 V/oct com precisão** e entregue uma forma
"crua" pra o `FILTER` esculpir. Hoje o patch melódico depende de `MATTER`
ou `STRING` — vozes lindas mas de personalidade marcada. O `OSC` é o
tijolo neutro do subtrativo: fonte de espectro rico e previsível.

Papel: recebe `pitch` (1 V/oct de `QUANTIZER.pitch` ou `SEQUENCE.pitch`),
entrega `saw`/`pulse`/… pra `FILTER.in`; `sync`/`fm` abrem timbres
clássicos; o `sub` engrossa o grave sem um segundo módulo.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **PolyBLEP** (Välimäki & Huovilainen; Martin Finke, "Bandlimited synthesis") | residual polinomial que corrige o salto do dente-de-serra / pulso na descontinuidade — barato, 1 amostra de cada lado | teoria/artigo público |
| **Hard sync clássico** (prática de VCO analógico; Roads, *Computer Music Tutorial*) | reiniciar a fase do oscilador na borda de um mestre → formante deslocável | conceito |
| **Through-zero FM** (Buchla 259; prática) | FM *linear* (soma de Hz, fase pode reverter) preserva a afinação percebida; a de fase não | conceito |
| **Sub-oscilador por divisão** (Roland Juno; Moog) | flip-flop no wrap da fase principal → quadrada uma oitava abaixo, sempre em fase | prática |
| **`drift`** (`AQUORBIUM/biome.odt`; `FUNCTION` do Rasgo) | random-walk lento correlacionado na afinação = "respira" sem entrada | código do autor |

Braids/Plaits (STM32F, MIT) são referência de **morph de forma** — fica
pra 2ª camada; aqui as formas são as quatro canônicas + sub.

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
f = freq · 2^(fine/1200) · 2^(pitch_in) · 2^(driftState)      [Hz]
kfm = fm_amount · 4 · f
dp = (f + fm_in · kfm) / sr                 (pode ser < 0 — TZFM)
se sync (borda ↑):  phase = 0
sine  = sin(2π·phase)
tri   = phase < 0,5 ? (4·phase − 1) : (3 − 4·phase)
saw   = (2·phase − 1) − polyBlep(phase, |dp|)
pw'   = clamp(pw + pwm_in, 0,02, 0,98)
pulse = (phase < pw' ? 1 : −1)
        + polyBlep(phase, |dp|) − polyBlep(frac(phase − pw' + 1), |dp|)
sub   = (subState ? 1 : −1)          subState inverte a cada wrap
                                     (a cada 2 wraps se sub_2)
phase += dp ;  wrap p/ [0,1)   (e −1 → +1 quando dp < 0)
drift: a cada ~1200 amostras  driftState += ruído·0,0006·drift² ,
        clamp(driftState, ±0,045)
```

**Estados:** `phase_`, `subPhase_`/`subState_`, `subCount_` (p/ `sub_2`),
`prevSync_`, `driftState_`, `driftCounter_`, `rngState_` (xorshift64*),
suavizadores de `freq`/`pw`. Sem alocação, sem RNG fora do `drift`.

**Extremos.** `freq` no teto (8 kHz) a 48 k → `dp ≈ 0,17`: PolyBLEP ainda
válido (`dp < 0,5`); acima disso a serra "quadra" (aceitável, é o limite
do método — 1ª camada). `fm` forte com `dp` muito negativo → a fase
reverte: tratado (wrap de −1). `pitch_in` extremo → `f` clampado a
[2 Hz, 0,45·sr]. `sync` mais lento que o osc → sync clássico; mais rápido
→ a fase quase não anda entre resets (pulso fino, ok). Reset → fase,
`subState`, `driftState` zerados.

## 4. Três modos obrigatórios

- **Autônoma:** `freq` fixo, sem entradas → drone afinado; `drift`
  pequeno tira o "morto" de um seno digital parado.
- **Performance:** `freq`/`pw` ao vivo; `sync` de um LFO/`CLOCK` = sweep
  de formante; `fine` pra afinar contra outro módulo.
- **Híbrida:** `pitch` de `QUANTIZER`/`SEQUENCE` (melodia escrita);
  `fm` de outro `OSC` (TZFM generativa); `pwm` de um `FUNCTION` lento.

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control, v/oct), `fm` (Audio), `pwm` (Control),
`sync` (Control, trig).
**Saídas:** `sine`, `tri`, `saw`, `pulse`, `sub` (todas Audio).
**Parâmetros:** `freq` (8–8000 Hz, def 110), `fine` (−100..100 cents),
`pw` (0,02–0,98, def 0,5), `fm_amount` (0–1), `drift` (0–1),
`sub_2` (0/1 — 0 = −1 oitava, 1 = −2), `sync_enable` (0/1).
**Limites:** saída em ~[−1,1] (formas normalizadas). CPU por amostra: 1
`sin`, 2–3 `polyBlep`, aritmética. Sem alocação.

## 6. Alternativas descartadas

- **BLIT/BLEP de tabela / minBLEP** já no marco 1: PolyBLEP dá 90% do
  resultado com ~10 linhas e sem tabela. minBLEP/oversampling = 2ª
  camada.
- **Uma saída "mix" com blend de formas:** o `MIXER` (Módulo 16) já
  soma; 5 saídas independentes é mais modular.
- **Sub derivado de fase escalada** (`phase·0,5`): não fecha uma quadrada
  no wrap. Flip-flop no wrap fecha e trava no sync.
- **FM de fase (DX7-style):** desafina ao aumentar o índice. TZFM linear
  preserva a afinação — é o que um modular quer.
- **`spread`/super-saw no marco 1:** N acumuladores de fase desafinados —
  bom, mas é a *relação entre saídas* (padrão Rasgo) e merece dossiê de
  desvio próprio. Anotado.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** 1 V/oct — `f` medido bate com `freq` (< 1% em
110/220/440 Hz) e `pitch = +1` dobra; `saw` é rampa monotônica entre
wraps; `pulse` é binível e a razão cíclica segue `pw`; `sub` uma/duas
oitavas abaixo (autocorrelação); hard sync → período de saída = período
do `sync`; TZFM finita e limitada com `fm` forte, transparente em
`fm_amount = 0`; serra a 7 kHz com pouca energia dobrada em banda baixa;
dois renders byte-idênticos; sem alocação.

**Escuta:** o seno soa "limpo" mas não "morto" com `drift` mínimo? a
serra a 2–4 kHz tem "chiado" de alias audível? o hard sync varre o
formante sem estalo? a TZFM soa "metálica afinada" (linear) e não
"desafinada" (fase)? o `sub` engrossa sem embolar o grave? `pw`
extremo (0,05) ainda tem corpo?

## 8. Integração e painel

Classe `Oscillator` (`type()` = `"OSC"`), 4 entradas, 5 saídas, 7
parâmetros. `panel()` próprio (12 HP: knobs FREQ/FINE/PW/FM/DRIFT,
toggles SUB2/SYNC, jacks 1V·O/FM/PWM/SYNC in e SIN/TRI/SAW/PLS/SUB out).
Testado isolado (afinação, formas, PWM, sub, sync, TZFM, antialias,
determinismo, drift) antes do patch. Cadeia melódica canônica:
`SEQUENCE → QUANTIZER → OSC.pitch` · `OSC.saw → FILTER → ENVELOPE → MIXER`.
Adicionado ao catálogo do painel (`apps/panel/ModuleCatalog.hpp`, família
SOURCE).
