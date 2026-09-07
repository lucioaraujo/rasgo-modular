# Dossiê — Módulo 29: Osciloscópio + análise (`SCOPE`)

**Família:** METER / UTILITY
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Scope.hpp`, `tests/test_scope.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

Os `Display` do painel hoje desenham a forma de onda crua da saída 0 de
qualquer módulo — sem trigger, sem análise. O `SCOPE` é a **ferramenta
de medição** e — desvio Rasgo — as medições saem como **CV patchável**:
num osciloscópio de hardware a tela é um beco sem saída; aqui o brilho,
a altura detectada, o nível e o disparo **voltam pro patch** (brilho →
corte do filtro, altura → oscilador, nível → VCA, disparo → envelope).

- **`in`** (áudio) → **`thru`** (áudio, saída 0) — o sinal passa LIMPO,
  sem colorir (a saída 0 é o que o `Display` do painel desenha = o
  osciloscópio de verdade);
- **`trig`** (gate) — comparador com **histerese** (`reject`) em `in`
  (ou em `ext`, se conectada) contra o nível `trigger`, borda de subida
  ou descida (`edge`); pulso de ~1 ms por evento. É o trigger do
  osciloscópio E um disparador utilitário ("dispara quando isto cruza
  X subindo");
- **`level`** (CV 0..~1) — seguidor de amplitude (ataque rápido fixo
  ~2 ms, release por `response`); pico que decai;
- **`bright`** (CV 0..1) — **estimativa do centroide espectral** pelo
  método do diferenciador: `f_c ≈ (sr/2π)·√(E[Δx²]/E[x²])` (Parseval — a
  energia da derivada é o segundo momento do espectro), por janela de
  512 amostras, mapeado `√(f_c/8000)` e suavizado por `response`. Sem
  FFT: barato, determinístico, sem alocação;
- **`pitch`** (CV v/oct) — fundamental por **autocorrelação YIN** (de
  Cheveigné & Kawahara, 2002) sobre o sinal decimado 3× (≈ 16 kHz):
  diferença acumulada normalizada → 1º mínimo local abaixo do limiar
  (0,15) → interpolação parabólica. Robusto a harmônicos — serra,
  quadrada, acorde: erro < 1 % (o ZCR anterior reportava 2×/3× a altura
  nesses casos). Faixa útil ~53–1000 Hz; sai em **oitavas relativas a
  110 Hz** (o `freq` padrão do `OSC` — `SCOPE.pitch → OSC.pitch`
  rastreia direto); 0 quando não há período claro (ruído, silêncio);
- **`hold`** (0/1) — congela `level`/`bright`/`pitch` no último valor
  (o `trig` continua vivo).

Params: `trigger` (−1..1), `edge` (0 subida / 1 descida), `reject`
(0..1 → banda de histerese 0..~0,5), `response` (0..1 → ~2 ms..~800 ms,
quadrático), `hold` (0/1).

Sem RNG (100 % determinístico), sem alocação/lock/IO em `process()`.

**Testes (13 funções, Debug + Release):** `thru` == `in` amostra a
amostra (passa limpo); `trig` dispara 1×/ciclo numa senoide cruzando o
nível subindo, 0× se `edge`=descida no mesmo ponto; `reject` alto mata
disparo em sinal ruidoso perto do nível; `ext` conectada = fonte do
trigger (ignora `in`); `level` sobe rápido e cai devagar com `response`
alto; `bright` de uma senoide 100 Hz << `bright` de uma senoide 4 kHz <<
`bright` de ruído branco; `bright` monotônico varrendo a frequência;
`pitch` de senoide 110 Hz ≈ 0 v/oct, de 220 Hz ≈ +1, de 440 Hz ≈ +2
(±0,05); `pitch` = 0 pra ruído; `hold` congela as três leituras;
`SCOPE.pitch → OSC.pitch` faz o `OSC` rastrear a altura de entrada no
grafo; tudo finito; dois renders byte-idênticos; painel 14 HP sem
sobreposição.

**Pendências (candidatos):** o **espectro desenhado** (barras FFT janela
Hann) — é feature do painel lendo o buffer da saída 0, não porta de
módulo (portas são escalares por amostra); modo **XY / Lissajous** (idem
— o painel cruza `in`×`ext`); ~~autocorrelação pra `pitch`~~ FEITO
2026-09-07 (YIN decimado); `pitch` polifônico (mais de uma f₀) fica pra
depois; `trigger` como fração da amplitude medida
(auto-nível).

---

## 1. Problema musical e papel no fluxo

Ver, medir, e — o ponto — **realimentar**. Um patch generativo que
"escuta a si mesmo": o brilho médio da mistura abre e fecha um filtro; a
altura da voz principal afina um drone; o nível dispara um envelope de
_sidechain_; o `trig` do `SCOPE` num cruzamento de zero sincroniza um
LFO. Sem o `SCOPE` essas medições não existem como sinal — só como
pixels.

Papel: análise + monitor + trigger, no fim de uma cadeia ou em derivação
(`thru` deixa inserir inline sem perder o sinal). No `seedPatch` v2 entra
como destino de áudio (`in`) e as saídas de CV (`level`/`bright`/`pitch`)
como fontes lentas de modulação — o instrumento reagindo ao que produz.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Osciloscópio de bancada** (Tektronix etc., teoria) | trigger de nível + borda + histerese (holdoff); timebase | teoria pública |
| **Mordax DATA, ALM MUM M8, Intellijel µScope** | scope de rack como utilitário; saídas de CV derivadas do sinal | ficha/conceito, código não consultado |
| **Centroide espectral pelo diferenciador** (Parseval) | `∫ω²|X|²dω` = energia de `x'` → `f_c ≈ √(E[x'²]/E[x²])` | resultado matemático público (MPEG-7 low-level descriptors, análise de sinais) |
| **YIN** (de Cheveigné & Kawahara, *A fundamental frequency estimator for speech and music*, JASA 2002) | função de diferença acumulada normalizada + limiar absoluto + interpolação parabólica → f₀ robusto a harmônicos | artigo público (o ZCR anterior — Rabiner, análise de fala — reportava a oitava errada em som rico) |
| Saída `level` do `MASTER` do Rasgo | medição de nível como CV de saída | código do autor |

## 3. Modelo — matemática, estados, extremos

Coeficientes por bloco: `respT = 0.002 + response²·0.8` (s);
`relCoef = 1 − exp(−1/(respT·sr))`; `atkCoef = 1 − exp(−1/(0.002·sr))`;
`rejBand = reject·0.5`.

Por amostra (`x = in[f]`; `s = ext conectada ? ext[f] : x`):
```
thru = x

# nível (pico com decaimento)
pk = |x|
if pk > lvl: lvl += (pk − lvl)·atkCoef
else:        lvl += (pk − lvl)·relCoef

# centroide espectral (acumula, resolve por janela de 512)
sumX2 += x·x ;  d = x − xPrev ;  sumD2 += d·d ;  xPrev = x
if ++win == 512:
    fc = (sr/2π)·√(sumD2 / max(sumX2, 1e−12))
    bTarget = clamp(√(fc/8000), 0, 1)
    win = 0 ; sumX2 = sumD2 = 0
bright += (bTarget − bright)·relCoef            # se !hold

# pitch — YIN sobre o sinal decimado 3× (decSr ≈ 16 kHz)
decAcc += x
if ++decCnt == 3:
    dhist[dw++ mod kHist] = decAcc/3 ; decAcc = 0 ; decCnt = 0
    if ++hopCnt == 64:                        # ~12 ms
        hopCnt = 0
        if lvl > 0.02: analyzePitch()  else  lockCount = 0

analyzePitch():
    copia dhist (mais antigo→novo) → w[0..kHist)
    running = 0 ; bestTau = −1
    for tau in [1 .. 300]:
        d = Σ_{i<320} (w[i] − w[i+tau])²
        running += d ;  d'[tau] = d·tau/running          # YIN cmndf
        if tau ≥ 16 && bestTau < 0 && d'[tau] < 0.15 && mínimo local:
            bestTau = tau
    if bestTau < 0: lockCount−− ; return                  # sem período (ruído)
    p = bestTau + parábola(d'[bestTau∓1])
    if |p − lastP| < 0.25·lastP:  lockCount++ ; periodEst += (p−periodEst)·0.35
    else:                         lockCount = 0 ; periodEst = p
    lastP = p

if lockCount ≥ 2 && lvl > 0.02:  pitchTarget = log2( (decSr/periodEst) / 110 )
else:                            pitchTarget = 0
pitchOct += (pitchTarget − pitchOct)·relCoef    # se !hold

# trigger (comparador com histerese + borda)
hiThr = trigger + rejBand ;  loThr = trigger − rejBand
rising:  if s < loThr: armed = true ;  if armed && s > hiThr: FIRE ; armed = false
falling: if s > hiThr: armed = true ;  if armed && s < loThr: FIRE ; armed = false
on FIRE: trigCd = 0.001·sr
trig = trigCd > 0 ? 1 : 0 ;  if trigCd > 0: −−trigCd
```

**Estados:** `lvl`, `bright`, `pitchOct`, `xPrev`, `sumX2`, `sumD2`,
`win`, `pitchTarget`, `bTarget`, `armed`, `trigCd`; para o pitch YIN:
`dhist[622]` (buffer decimado, no header), `decAcc`, `decCnt`, `hopCnt`,
`dw`, `periodEst`, `lastP`, `lockCount`. Sem alocação de heap
(`analyzePitch` usa `float w[622]` na pilha).

**Extremos.** Silêncio → `level`→0, `bright`→0 (numerador e denominador
zeram; o `max(...,1e−12)` evita NaN), `pitch`→0, `trig` só se `trigger`
for exatamente 0 e o ruído numérico cruzar (o `rejBand` mínimo de
`reject=0` é 0 — então documenta-se: com `reject=0` e `trigger=0` um
sinal nulo pode chocalhar; `reject` pequeno resolve). DC puro → `bright`
0 (sem Δ), `pitch` 0 (sem cruzamento), `trig` dispara uma vez ao entrar.
`response=1` → leituras quase estáticas (média de ~1 s). `hold=1` →
`level`/`bright`/`pitch` param no lugar; `trig` continua. `ext`
conectada com áudio a taxa alta → o comparador dispara a cada cruzamento
(pode saturar em 1 se `trigCd` > período — comportamento honesto de um
holdoff curto).

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → `thru`=0, todas as CVs em 0, `trig` em
  0. É medidor: sem sinal, não inventa leitura. (Aceitável — um
  osciloscópio sem sonda não mostra nada.)
- **Performance:** `trigger`/`edge`/`reject` estabilizam a imagem ao
  vivo; `response` de "medidor nervoso" (segue transientes) a "média
  lenta" (tendência); `hold` congela pra ler.
- **Híbrida:** `MASTER.mix → SCOPE.in`, `SCOPE.bright → FILTER.cutoff`
  (o patch escuta o próprio brilho); `SCOPE.pitch → OSC.pitch` (um
  drone afina pela voz principal); `SCOPE.level → ENVELOPE.gate`
  (sidechain); `CLOCK.euclid → SCOPE.ext` + `SCOPE.trig → …` (trigger
  ritmado).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `ext` (Control — fonte alternativa do
trigger).
**Saídas:** `thru` (Audio), `trig` (Control, gate), `level` (Control),
`bright` (Control), `pitch` (Control, v/oct).
**Parâmetros:** `trigger` (−1..1, def 0), `edge` (0..1, def 0),
`reject` (0..1, def 0,1), `response` (0..1, def 0,3), `hold` (0..1,
def 0).
**Limites:** `thru` não é tocado (nem clampado — passa o que entra).
`bright`/`level` em [0,1]; `pitch` livre (tipicamente −2,5..+5,5).
CPU: 1 `exp` por bloco + 1 `sqrt`/`log2` por janela (512) + aritmética
por amostra. Sem alocação.

## 6. Alternativas descartadas

- **FFT de verdade no módulo:** o espectro é um vetor; as portas são
  escalares por amostra. Um `bright` escalar pelo diferenciador dá o
  que serve como modulação (brilho) sem FFT; o espectro *desenhado*
  fica pro painel (lê o buffer da saída 0). Pendência honesta, não
  limitação de design.
- **`gain` de entrada:** um medidor com trim de ganho mente sobre o que
  mede; `thru` tem que passar limpo. Sem `gain`.
- ~~**`pitch` por ZCR só:**~~ trocado por **autocorrelação YIN**
  (2026-09-07) — o ZCR reportava 2×/3× a altura em serra/quadrada/acorde.
  O YIN roda num sinal DECIMADO 3× (janela 320, lags 16–300 dec) e só a
  cada ~12 ms → O(lag·janela) por hop ≈ 2–4 % de um núcleo, sem
  alocação de heap (janela copiada pra `float w[622]` na pilha),
  determinístico. Polifonia (mais de uma f₀) fica pra depois.
- **`trig` de 1 amostra:** some no sub-bloco do painel e em cadeias que
  amostram esparso; ~1 ms é o mínimo audível/detectável.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `thru[f] == in[f]` exatamente; senoide 200 Hz amplitude
0,5, `trigger=0`, `edge=subida` → `trig` dispara 1×/ciclo (≈200
pulsos/s); `edge=descida` → dispara no cruzamento oposto (mesma
contagem, fase deslocada); `reject=0.8` + senoide + ruído 0,3 no nível
→ nº de disparos ≈ 1×/ciclo (não N); `bright(100 Hz) < bright(1 kHz) <
bright(4 kHz) < bright(ruído)`; varredura 50→8000 Hz → `bright`
monotônico não-decrescente (com tolerância); `pitch`: 110→0, 220→+1,
440→+2 v/oct (±0,05); ruído → `|pitch| < 0,1`; `hold=1` → `level`/
`bright`/`pitch` constantes por ≥ 1 s; dois renders byte-idênticos.

**Escuta:** `SCOPE.bright → FILTER.cutoff` num loop soa como o
instrumento "se ouvindo" (brilho estabiliza) ou como realimentação
descontrolada? `SCOPE.pitch → OSC.freq` cria um uníssono/oitava que
segue a voz de maneira musical? `response` no ponto certo faz `level`
virar um bom sinal de sidechain? o `trig` num cruzamento de zero
sincroniza um LFO sem clicar?

## 8. Integração e painel

Classe `Scope` (`type()` = `"SCOPE"`), 2 entradas, 5 saídas, 5
parâmetros. `panel()` próprio (~14 HP): `Display` grande no topo (a saída
0), knobs TRIG/EDGE/REJ/RESP/HOLD, jacks IN/EXT · THRU/TRIG/LVL/BRT/PIT.
Testado isolado (thru, trigger/borda/histerese, brilho monotônico,
pitch tracking, hold, determinismo) antes do patch. Cadeias canônicas:
`MASTER.mix → SCOPE.in` · `SCOPE.bright → FILTER.cutoff`;
`SCOPE.pitch → OSC.pitch`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`) — não há METER; entra em MIX junto de
`MIXER`/`MASTER` (o fim da cadeia).
