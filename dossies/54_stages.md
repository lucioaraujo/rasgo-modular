# Dossiê — Módulo 54: Gerador de segmentos configuráveis (`STAGES`)

**Família:** MODULATE (GERA um sinal de controle — junto de `ENVELOPE`/
`FUNCTION`/`DRIFT`)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Stages.hpp`, `tests/test_stages.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.5` (Tier 1 — "o mais rico
conceitualmente") + Rossum Control Forge ★ (`§7 #43`); Blukač Fractalist
★ (`§7 #4`); matriz Mutable (`§3` — "fragmento que vira env/seq/osc/
ruído conforme quem conecta")

## Estado da implementação

O `FUNCTION` é UMA função tipo Maths (uma rampa deformável). Falta o
gerador de **N segmentos reconfiguráveis** cuja *função emergente* muda
com o `mode` de uso: vira envelope, LFO complexo, sequência de degraus
ou oscilador lento — conforme `hold`/`loop` e o que está cabeado.

- **`segments`** (2–8) = quantos degraus/rampas na volta.
- **`rate`** (0,02–20 Hz, + CV) = velocidade da volta no modo loop;
  cada segmento leva `(1/rate)/segments` ajustado por `tilt`.
- **`contour`** (0–1) = a FORMA dos níveis-alvo dos segmentos: `0` escada
  subindo (0→1), `0,5` arco (0→1→0), `1` escada descendo — blend
  contínuo. É o "desenho" da função.
- **`curve`** (−1..1) = a curva de transição de cada segmento:
  `<0` exponencial (rápido→lento), `0` linear, `>0` logarítmica.
- **`hold`** (0–1) = `0` cada segmento DESLIZA suave até o próximo nível
  (rampa — envelope/LFO); `1` SALTA e segura (degrau — S&H/sequência);
  entre, rampa parte do segmento e segura o resto.
- **`tilt`** (−1..1) = distorção das durações: `<0` segmentos do começo
  mais longos, `>0` os do fim (attack lento ↔ release lento).
- **`jitter`** (0–1, desvio Rasgo) = passeio lento **semeado** nos níveis
  e durações — a forma "respira" sem deixar de ser reprodutível
  (`jitter=0` → sem termo, byte-idêntico).
- **`loop`** (0/1) = corre livre (LFO complexo) ↔ um disparo (envelope —
  precisa do gate).

**Entradas:** `gate` (Control — inicia no modo one-shot; ignorado no
loop), `reset` (Control trig — volta ao segmento 0), `rate_mod` (Control).
**Saídas:** `out` (Control — a função), `eoc` (Control gate — pulso no fim
da volta), `step` (Control gate — pulso em cada fronteira de segmento).

`hold=0`, `loop=1`, `contour=0,5` → um LFO de arco suave. `hold=1`,
`loop=1` → uma sequência de `segments` degraus (um passeio determinístico
com `jitter`). `loop=0`, `hold` baixo, `segments=3`, `tilt<0` → um
envelope AD/AR. Sem alocação em `process()` (os `segments` níveis/durações
são pré-computados; recalculados só quando um parâmetro muda).
Determinístico (jitter semeado em `prepare()`).

**Testes (Debug + Release):** `loop=1`, `hold=0` → `out` é periódico em
`rate` e contínuo (sem salto > passo); `hold=1` → `out` assume exatamente
`segments` níveis distintos e é constante entre as fronteiras; `step`
dispara `segments` vezes por volta, `eoc` 1 vez; `contour=0` → níveis
sobem monotônicos; `contour=1` → descem; `tilt<0` → o 1º segmento dura
mais que o último (medido); `loop=0` + `gate` → uma passagem só,
congela no último nível até o próximo gate; `reset` → volta ao segmento
0; `curve<0` → a transição começa rápida (derivada maior no início);
`jitter>0` → a forma varia entre voltas mas dois renders são
byte-idênticos; `jitter=0` → determinístico puro; tudo finito, `out` em
[−1,1] (ou [0,1] conforme `contour`... ver §3).

**Pendências (candidatos):** tipos de segmento por-segmento selecionáveis
(STEP/RAMP/HOLD como o Stages, não um `hold` global); segmentos que
esperam o gate (o "verde" do Stages — HOLD real); `out` bipolar/unipolar
selecionável; `address` (CV escolhe o segmento — o endereçamento duplo do
Control Forge); encadear várias instâncias (o `eoc` de uma dispara a
próxima); forma de segmento além de curva (senoidal, degraus dentro do
segmento).

---

## 1. Problema musical e papel no fluxo

Um gerador de função que é **um objeto só** mas soa como quatro
dependendo de como você o liga: 3 segmentos + `loop=0` = um envelope; 6
segmentos suaves + `loop=1` = um LFO que nunca se repete igual; 8
segmentos com `hold=1` = um sequenciador de CV; `rate` na faixa de áudio
= um oscilador de forma bizarra. O `FUNCTION` faz UMA rampa; o `ENVELOPE`
faz ADSR; o `STAGES` faz a *forma composta* — e o `contour`/`tilt`/`hold`
esculpem ela com poucos macros (identidade RASGO: gerador, não editor de
breakpoints).

Papel: MODULATE. `STAGES → FILTER.cutoff` (um contorno de N segmentos no
timbre); `CLOCK → STAGES.gate` + `loop=0` (envelope disparado);
`STAGES.step → ENVELOPE.gate` (encadeia); `STAGES.out → OSC.pitch` +
`hold=1` (sequência de alturas).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Stages** (Émilie Gillet) | N segmentos reconfiguráveis; a função EMERGE de como se encadeiam (rampas → envelope, degraus → seq); loop | MIT (código não consultado — só o conceito público) |
| **Rossum Control Forge / Serge DUSG encadeado** | gerador de função programável de múltiplos estágios; endereçamento (alcançar um ponto sem quebrar o sistema fechado) | ficha/conceito |
| **Blukač Fractalist** | a MESMA lógica em taxa de controle E de áudio (só muda a escala de tempo) | ficha/conceito |
| **Envelope de segmentos** (Serge, Buchla 281) | rampa entre níveis com curva por segmento; teoria de gerador de envelope | domínio público |
| **Curva exp/log de segmento** (`pow`, `1−(1−t)^k`) | shaping da transição — a mesma do `ENVELOPE.curve` do Rasgo | código do autor |
| **`ENVELOPE`/`FUNCTION`/`SH` do Rasgo** | detecção de borda, fase, slew, acaso semeado por segmento | código do autor |

**Desvio Rasgo (Atlas §49):** os N segmentos DESENHADOS por macros
(`contour`/`tilt`/`hold`), não editados ponto a ponto; o `jitter`
determinístico (a forma respira, mas o render é reprodutível); a mesma
classe cobrindo envelope + LFO + sequência + osc (a "identidade mutável"
da matriz Mutable), sem `mode` explícito — o comportamento vem de
`loop`/`hold`/`rate` e do que está cabeado.

## 3. Modelo — matemática, estados, extremos

Parâmetros: `segments` (2–8, def 4), `rate` (0,02–20 Hz, def 0,5),
`contour` (0–1, def 0,5), `curve` (−1..1, def 0), `hold` (0–1, def 0),
`tilt` (−1..1, def 0), `jitter` (0–1, def 0), `loop` (0/1, def 1).

**Pré-cálculo** (só quando `segments`/`contour`/`tilt`/`jitter` mudam ou
1×/volta se `jitter>0`), para `k` ∈ 0..N−1:
```
# nível-alvo
u = k / (N−1)
asc  = u                                   # escada subindo
arch = 1 − |2u − 1|                         # arco
desc = 1 − u                               # descendo
lvl[k] = contour < 0,5 ? lerp(asc, arch, contour·2)
                       : lerp(arch, desc, (contour−0,5)·2)
lvl[k] += jitter · jn[k]                   # jn: passeio lento semeado
lvl[k] = clamp(lvl[k], −1, 1)

# duração relativa
w = 1 + tilt · (2u − 1)                    # tilt<0 → começo mais longo
w += jitter · jn2[k] · 0,5
dur[k] = max(0,01, w)
# normaliza: Σ dur = 1
```

Por amostra (modo loop; one-shot troca só a fonte de avanço):
```
phase += rate/sr · (loop ? 1 : gateRunning)
if phase >= 1: phase −= 1 ; eoc = pulso ; (one-shot: gateRunning = 0)
# acha o segmento
acc = 0 ; k = 0
while acc + dur[k] < phase: acc += dur[k] ; k++
segFrac = (phase − acc) / dur[k]
if k mudou desde a amostra anterior: step = pulso
from = lvl[k]
to   = loop ? lvl[(k+1) mod N] : (k < N−1 ? lvl[k+1] : lvl[k])
# hold: rampa (1−hold) do segmento, depois segura
t = clamp(segFrac / max(1e−3, 1 − hold), 0, 1)
tc = curveShape(t, curve)                  # exp/lin/log
out = from + (to − from) · tc
```
`curveShape(t, c)`: `c<0` → `t^(1 + |c|·4)` (exp); `c>0` →
`1 − (1−t)^(1 + c·4)` (log); `c=0` → `t`.

`reset` (borda ↑): `phase = 0`, `k = 0`.
No one-shot: `gate` borda ↑ → `gateRunning = 1`, `phase = 0`.

**Estados:** `phase_`, `prevK_`, `prevGate_`, `prevReset_`,
`gateRunning_`, `lvl_[8]`, `dur_[8]`, `jn_[8]`, `jn2_[8]`, `cycleCount_`,
`rng_`, `sr_`, `dirty_` (flag de recálculo). Sem alocação.

**Extremos.**
- `segments=2`, `hold=0`, `loop=1` → um triângulo/dente (dois níveis).
- `hold=1` → escada pura; `out` é constante entre fronteiras, salta nelas.
- `rate` na faixa de áudio (até 20 Hz aqui — não vai a osc de verdade na
  v1; pendência) → LFO rápido.
- `tilt=1` + `segments=3` + `loop=0` → um envelope com ataque
  instantâneo e release longuíssimo.
- `contour=0,5`, `segments` alto, `hold=0` → um arco liso (aproxima um
  seno com N segmentos).
- `jitter=1` → os níveis e durações passeiam bastante; ainda em [−1,1] e
  reprodutível.
- `loop=0` sem nunca receber `gate` → `out` fica no nível do segmento 0
  (silêncio de CV, correto).
- `curve` no talo + `hold=1` → o `curve` não faz nada (não há transição);
  aceito.

## 4. Três modos obrigatórios

- **autônoma:** `loop=1`, nada conectado → um LFO complexo de N segmentos
  a `rate`; com `jitter>0` nunca se repete exatamente. Move ao carregar.
- **performance:** `contour`/`tilt`/`hold` remodelam a forma ao vivo;
  `rate` a velocidade; `segments` a resolução.
- **híbrida:** `CLOCK → gate` + `loop=0` (envelope por disparo);
  `SEQUENCE → reset`; `STAGES.step → ENVELOPE.gate` (encadeia);
  `STAGES.out → FILTER.cutoff` + `hold=1` (contorno em degraus).

## 5. Portas, parâmetros, limites

**Entradas:** `gate` (Control), `reset` (Control trig), `rate_mod`
(Control).
**Saídas:** `out` (Control), `eoc` (Control gate), `step` (Control gate).
**Parâmetros:** ver §3.
**Limites:** `out` em [−1,1] (o `contour` mantém os níveis-alvo em
[0,1]; o `jitter` pode levar levemente a negativo — clampado). CPU: por
amostra 1 laço curto (≤ 8) + 1 `pow` (curve) + (com jitter, 1×/volta)
recálculo dos 8 níveis. Sem alocação, sem `sin` no caminho quente.

## 6. Alternativas descartadas

- **um módulo por uso** (ENVELOPE, LFO, SEQ separados) — já temos
  `ENVELOPE`, `FUNCTION`, `SEQUENCE`. O `STAGES` é a *forma composta* que
  vira qualquer um deles conforme a fiação; o valor é a unificação.
- **tipos de segmento por-segmento** (STEP/RAMP/HOLD como o Stages) —
  exige 8 seletores no painel; o `hold` global dá 80% do resultado com 1
  knob. Fica como pendência (o "verde" HOLD que espera o gate é o que
  mais falta).
- **editor de breakpoints** (arrastar pontos) — é editor, não gerador;
  contra a identidade RASGO (`TRIGSEQ`, `DECISION`, `ABACUS` autônomo).
- **`mode` explícito (ENV/LFO/SEQ/OSC)** — o comportamento já emerge de
  `loop`/`hold`/`rate`; um seletor seria redundante e "trancaria" o
  módulo num papel.
- **`rate` até áudio (osc de verdade)** — precisa de antialias por
  segmento (os cantos aliasam); fica pra `STAGES+` (o teto é 20 Hz na v1).
- **`jitter` sem seed** — variação irreprodutível é bug (regra do
  `SAMPLER`/`DRUM`/`TRIGSEQ`).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `loop=1`, `hold=0`, `rate=2`, `segments=4` → `out` tem
período de 0,5 s (autocorrelação) e `|Δout por amostra| < 0,02`;
`hold=1` → o nº de valores distintos de `out` numa volta é ≤ `segments`,
e a derivada é 0 exceto nas fronteiras; `step` conta `segments` pulsos e
`eoc` conta 1 por volta; `contour=0` → `lvl[k]` monotônico crescente
(medido nos platôs com `hold=1`); `contour=1` → decrescente; `tilt=−0,8`
→ o tempo no 1º nível > 2× o tempo no último (com `hold=1`, medível);
`loop=0`, um `gate` → `out` faz UMA passagem e depois fica constante no
último nível; um 2º `gate` → recomeça; `reset` no meio → `out` volta pro
nível 0; `curve=−0,8` → nos primeiros 10% do segmento `out` já percorreu
> 25% da distância (exp); `jitter=0,5` → `rms(volta_n − volta_1) > 0` mas
dois renders idênticos; `jitter=0` → dois renders byte-idênticos; tudo
finito.

**Escuta:** o LFO de arco (`contour=0,5`, `hold=0`) soa como um seno
"orgânico" ou como segmentos audíveis? a escada (`hold=1`) num
`OSC.pitch` dá uma melodia com lógica? o envelope (`loop=0`, 3 seg) tem
o "snap" de um AD de verdade? `tilt` esculpe o ataque/release de um jeito
útil? `jitter` faz a forma "respirar" sem virar ruído?

## 8. Integração e painel

Classe `Stages` (`type()` = `"STAGES"`), 3 entradas, 3 saídas, 8
parâmetros. `panel()` próprio (12 HP): `Display` (a forma — o
osciloscópio já mostra), knobs `SEGS`/`RATE`/`CNTR` (linha 1),
`CURVE`/`HOLD`/`TILT` (linha 2), `JITR` + toggle `LOOP` (linha 3);
jacks `GATE`/`RST`/`RTM` (entrada) · `OUT`/`EOC`/`STEP` (saída). Testado
isolado (loop suave, escada, contour, tilt, one-shot, reset, curve,
jitter, determinismo) antes do patch. Cadeias canônicas:
`STAGES → FILTER.cutoff`; `CLOCK → STAGES.gate` (`loop=0`);
`STAGES.step → ENVELOPE.gate`. Adicionado ao catálogo do painel
(`apps/panel/ModuleCatalog.hpp`, família MODULATE — junto de `ENVELOPE`/
`FUNCTION`/`DRIFT`/`CHAOS`/`SH`) e ao `LearnCatalog.hpp`.
