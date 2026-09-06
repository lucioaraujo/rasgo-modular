# Dossiê — Módulo 51: Averager de porta com reconstrução (`BOXCAR`)

**Família:** DECISION
**Estado:** **implementado** (2026-09-06; decisões do autor tomadas)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Boxcar.hpp`, `tests/test_boxcar.cpp` (11 testes)
**Toca também:** `src/dsp/Noise.hpp` — ganhou o param `poisson` (modo
Poisson livre, ver decisão 3).
**Candidato:** `PESQUISA_MODULOS.md §2.4` "Fora de onda" (2026-09-06)
**Gatilho:** o autor achou o **AI Synthesis AI250 BXR** interessante
(14 HP, Daisy Seed; "audio mangler / CV generator / VCO" inspirado no
*boxcar averager* — equipamento de teste nuclear, ref. Stanford Research
SR200 NIM / SR-235 Analog Processor). Pedido: "faça o dossiê-proposta".

## Estado — aprovado, a implementar

**O que é um *boxcar averager*.** Instrumento de laboratório (*gated
integrator* / integrador boxcar) para **extrair um sinal repetitivo de
dentro do ruído**: um *trigger* travado no evento repetitivo; um **delay
de abertura** posiciona uma janela num ponto do período; uma **largura de
abertura** (*aperture*) diz quanto tempo a janela integra; a **média de N
repetições** (janela retangular — daí "boxcar" — ou média exponencial)
faz o ruído tender a zero e o sinal coerente "emergir". Varrendo o delay
ao longo do período, reconstrói-se a forma de onda inteira (*scanning
boxcar*). Teoria de instrumentação de **domínio público** (Wikipedia
"Boxcar averager"; manuais SR200/SR250 são documentos públicos).

**O que o RASGO ganharia (e o que NÃO é).** O `SH` faz *grab* instantâneo
e *track & hold*; o `SCOPE` mede e devolve escalares (pitch, envelope,
centroide); o `ABACUS` faz aritmética/lógica de CV. Falta o objeto que
**integra numa janela, empilha capturas e reconstrói uma forma de onda** —
o `SCOPE` construtivo em vez de redutor, primo do `SH` com memória e
inércia. **Não** é um VCO novo (o RASGO já tem OSC/WAVETABLE/ADDITIVE/
CHORD) — o "modo oscilador" aqui relê o **buffer que ele mesmo montou**, a
onda é "o que ele ouviu, mediado" (desvio Warps/Rings: a relação é o
processo).

**Decisões do autor (2026-09-06 — tomadas):**

1. **Nome: `BOXCAR`.** Sem impedimento. "Boxcar" é vocabulário genérico
   de processamento de sinal (*boxcar function / window / averager*),
   não marca; o próprio AI Synthesis batizou o deles **"BXR"**, não
   "Boxcar", e não há marca de eurorack "Boxcar" à vista. Marca sobre
   termo descritivo/genérico é fraca e específica de classe; para um
   módulo de projeto AGPL o risco é mínimo. A regra de licença do RASGO
   é sobre **copyright de código** (não copiar código) — satisfeita:
   pega-se o conceito do instrumento de domínio público, não o firmware
   do AI250 (que publica só o `.bin`).
2. **Família: DECISION.** Ao lado do `ABACUS`/`QUANTIZER`/`HARMONY`/
   `DECISION` — é o parente construtivo do verbo reservado
   PERCEPTION/INFERENCE.
3. **"Geiger gates": os dois.** (a) saída secundária `geiger` do `BOXCAR`
   (temático); (b) **`NOISE` ganha um modo Poisson livre** — um `mode`
   ou param novo que troca a saída de gate/trigger interna por um
   processo de Poisson (`t = −ln(U)/λ`, xorshift semeado), preenchendo a
   lacuna "ritmo aleatório não preso ao clock". As duas implementações
   compartilham a mesma fórmula; a do `NOISE` é uma edição mínima e
   separada, feita junto.
4. **Modo oscilador: na v1.** `mode 2` relê o `recon_` a um `rate`
   próprio, livre do `trig`. Cuidados: `recon_` interpolado (linear
   entre bins) na releitura pra atenuar aliasing; buffer vazio → silêncio
   até as capturas preencherem.
5. **`in`/`out` como Audio + Control.** O tipo é compartilhado no RASGO;
   `K` (bins de fase) alto (≈2048) pra o áudio-rate não serrilhar demais.

**Módulo 51.** `tests/test_boxcar.cpp` = alvo CTest 62. Em
`ModuleCatalog.hpp` (DECISION), `LearnCatalog.hpp`, `test_panel_layout.cpp`;
docs `00_indice` / `PESQUISA §2.4` / `RASGO_MODULAR.md §36.3` + `§4` +
contagens (50 módulos, 62 CTest).

**Ajustes na implementação (vs. o modelo de §3):**
- **`mode 0` (follower) segura `recon_[bin]`, não o `m` cru.** Com
  `average` = 1 é a última janela (S&H clássico); com N alto é o S&H
  "sem tremor" (o ruído descorrelato já saiu). Mais útil que segurar o
  `m` bruto, que nunca se beneficiava da média.
- **A janela escreve o ARCO de bins `[delayPos, delayPos+aperture]`**, não
  1 bin — a abertura É uma suavização em fase, e com `scan` isso enche o
  buffer com muito menos capturas. `aperture`→0 = 1 bin (boxcar pontual).
- **Piso de ruído interno de −34 dB quando `in` desconectado** (stream
  xorshift `rngN_` próprio, semeado) — dá o modo autônomo (`out` toca uma
  textura fraca, `geiger` pulsa ao carregar). Só quando `in` livre; não
  afeta o `blend = 0` bypass.
- **`mode` default = 0** (era 1 na proposta): sem `scan`, o `mode 1`/`2`
  só enchem uma fatia do buffer; o follower responde na hora.
- `NOISE.poisson`: knob `POIS`, painel do `NOISE` foi de 12 → 14 HP.

---

## 1. Problema musical e papel no fluxo

Três gestos que nenhum módulo do RASGO faz hoje:

- **revelar um sinal enterrado no ruído** — feed ruído + um tom fraco
  sincronizado ao `trig`; ao longo de segundos o tom "se desenha" como
  uma foto na cuba (o verbo **DAMAGE/REPAIR**, lado REPAIR);
- **um S&H que mede uma FATIA, não um instante** — a média de uma janela
  é robusta a transientes e ruído; um envelope-follower que não treme;
- **varrer uma cabeça de leitura por uma forma de onda capturada** — o
  `delay`/`aperture` na mão passeiam por dentro do que o módulo ouviu.

Papel: análise → controle (e opcionalmente transformação de áudio).
`CLOCK → BOXCAR.trig` · `OSC desafinado → in` · `out → FILTER.cutoff`
(envelope limpo); ou `AUDIO-IN → in` + `out` como áudio (o *mangler*).
Sozinho: relógio interno + `geiger` → uma wavetable que ele constrói do
próprio piso de ruído, tocada por um trem de pulsos de Poisson.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Boxcar averager / gated integrator** (Wikipedia; Stanford Research SR200 NIM, SR250, SR235 — manuais públicos) | *trigger* + delay de abertura + *aperture* + média de N capturas; *scanning* reconstrói a onda | teoria de instrumentação, domínio público |
| **AI Synthesis AI250 BXR** (referência funcional — 14 HP, Daisy) | o boxcar como *mangler* musical; knobs Function/Argument (comparadores); "Geiger gates"; VCO interno | **ficha/demos**; firmware publica só o `.bin`, sem fonte — **conceito, não código** |
| **Sample & hold com janela** (qualquer texto de amostragem) | integrar/promediar durante o gate em vez de pegar 1 amostra | teoria pública |
| **Edge trigger de osciloscópio** | cruzamento de limiar como referência de repetição (auto-trigger, sem cabo no `trig`) | prática de domínio público |
| **Processo de Poisson** (`t = −ln(U)/λ`) | trem de gates aleatório de taxa λ, *livre* (não quantizado a clock) — os "Geiger gates" | teoria pública |
| **`SH` / `SCOPE` / `ABACUS` do RASGO** (código do autor) | detecção de borda, relógio interno, medição que devolve CV, EMA com piso | código do autor |
| **Mutable Warps/Rings — "a relação é o processo"** (desvio já no Atlas) | o "modo oscilador" relê o buffer reconstruído: a onda é o que foi ouvido | conceito |

**Desvio Rasgo obrigatório** (regra de licença — origem pública, desvio
próprio): (a) o *scanning* que vira **oscilador** relendo o buffer
próprio; (b) o **período medido** do `trig` normaliza os bins de fase —
uma referência em rubato ainda reconstrói (só borra), o boxcar de
laboratório assume período fixo; (c) `geiger` como saída de Poisson
**livre** integrada ao grafo (um `LFO` e um contador Geiger são
intercambiáveis).

## 3. Modelo — matemática, estados, extremos

Parâmetros (proposta): `delay` (0–1, fração do período), `aperture`
(0–1, fração do período; →0 = amostra pontual), `average` (1–64,
profundidade N da média móvel), `scan` (−1..1, velocidade/direção da
varredura do `delay`; 0 = estático), `mode` (0 follower · 1 reconstruct
· 2 oscillator), `rate` (0,05–40 Hz — `trig` interno + releitura no
modo 2), `thresh` (−1..1 — limiar do auto-trigger quando `trig` livre),
`geiger` (0–1 — densidade de Poisson, 0 = saída muda), `blend` (0–1 —
`in` ↔ resultado).

Entradas: `in` (Audio/Control), `trig` (Control trig), `sweep` (Control —
CV somada a `delay`), `thr` (Control — CV somada a `thresh`).
Saídas: `out` (Audio/Control), `geiger` (Control gate).

Estados: `recon_[K]` (K≈1024, média corrente por bin de fase),
`hits_[K]` (nº de capturas por bin, satura em N), `periodEma_` (período
medido, em amostras), `phase_` (amostras desde o último `trig`),
`prevTrig_`, `prevIn_` (auto-trigger), `winSum_`/`winCount_`/`winOpen_`,
`scanPos_` (0–1), `heldOut_`, `freePhase_` (modo 2), `poissonCd_`,
`rng_` (xorshift semeado), `yOut_` (slew opcional).

Por amostra:
```
# referência de repetição
if trig conectado: edge = (trig ≥ 0.5) && !prevTrig
else:              edge = (in ≥ thr) && (prevIn < thr)          # edge trigger
if edge:
    if phase_ > 4: periodEma_ += (phase_ − periodEma_) · 0.25   # T medido
    phase_ = 0
    winOpen_ = false
    delayPos = clamp01(delay + sweep + scan·scanPos_)
    winStartPhase = delayPos · periodEma_
    scanPos_ += scan·(1/scanCycles); wrap [0,1)

# janela de abertura
apS = max(1, aperture · periodEma_)
if phase_ ≥ winStartPhase && phase_ < winStartPhase + apS:
    winSum_ += in ; winCount_++ ; winOpen_ = true
if winOpen_ && phase_ ≥ winStartPhase + apS:                    # fecha
    m = winSum_ / max(1, winCount_)
    bin = round(delayPos · (K−1))
    k = min(average, ++hits_[bin])
    recon_[bin] += (m − recon_[bin]) / k                        # média de N, EMA c/ piso
    heldOut_ = m
    winSum_ = winCount_ = 0 ; winOpen_ = false

phase_ += 1 ; prevTrig_ = trig ; prevIn_ = in

# saída
mode 0: o = heldOut_                                            # segurado entre capturas
mode 1: o = readRecon( frac(phase_/periodEma_) )               # replay em sincronia
mode 2: freePhase_ += rate/sr ; o = readRecon( frac(freePhase_) )
yOut_ += (o − yOut_) · slewCoef                                 # slew leve opcional
# readRecon(u): índice fracionário u·(K−1), interpolação linear entre
# recon_[i] e recon_[i+1] (wrap) — atenua aliasing na releitura
out = lerp(in, yOut_, blend)

# geiger (Poisson livre, semeado)
if geiger > 0:
    poissonCd_ -= dt
    if poissonCd_ ≤ 0:
        emit pulso de 5 ms
        λ = 0.5 + geiger·geiger·40           # ~0,5 a ~40 ev/s
        poissonCd_ = −ln(rnd01()) / λ
```
`slewCoef` = `slew<=0 ? 1 : 1 − exp(−dt/(slew²·0,3))`. Sem alocação em
`process()` (o `recon_`/`hits_` alocam em `prepare()`). Determinístico: a
média é aritmética exata; o `geiger` é xorshift semeado (dois renders
byte-idênticos).

**Extremos.**
- `aperture → 0` + `scan = 0` + `mode 0` → S&H comum, travado numa fase
  fixa `delay`.
- `average` grande + `in` ruidoso → `out` converge pra parte coerente;
  variância do ruído cai ~1/N.
- `scan ≠ 0` + `mode 1` + `in` = ruído + tom fraco sincronizado → o tom
  se reconstrói ao longo de ~N·scanCycles períodos ("revelação").
- `trig` livre e `in` nunca cruza `thresh` → `edge` nunca ocorre → cai no
  `rate` interno; vira um follower lento.
- `mode 2` com `recon_` ainda zerado → silêncio até as capturas
  preencherem os bins.
- período do `trig` muda (rubato) → os K bins são normalizados pela fase
  (`phase_/periodEma_`), então continua reconstruindo, só borra.
- `blend = 0` → passa `in` limpo (bypass); `blend = 1` → só o resultado.
- `geiger = 0` → a saída `geiger` fica muda (não desperdiça RNG).

## 4. Três modos obrigatórios

- **autônoma:** nada conectado → `rate` interno, `mode 2`, `geiger` médio
  → toca uma wavetable que ele monta do próprio piso de ruído numérico,
  disparada por um trem de Poisson. Soa ao carregar.
- **performance:** `delay`/`aperture` na mão = varrer uma cabeça de
  leitura pela onda capturada; `average` = quanto ele borra a história;
  `scan` liga a varredura; `blend` revela.
- **híbrida:** `CLOCK → trig` + `OSC desafinado`/`AUDIO-IN → in`;
  `out → FILTER.cutoff` (envelope limpo) ou `out` como áudio (mangler);
  `SEQUENCE → sweep` (frase de posições de abertura);
  `geiger → ENVELOPE.gate` (acento estocástico).

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio/Control), `trig` (Control trig), `sweep`
(Control), `thr` (Control).
**Saídas:** `out` (Audio/Control), `geiger` (Control gate).
**Parâmetros:** `delay` (0–1, def 0), `aperture` (0–1, def 0,1),
`average` (1–64, def 8), `scan` (−1..1, def 0), `mode` (0–2, def 1),
`rate` (0,05–40 Hz, def 2), `thresh` (−1..1, def 0), `geiger` (0–1,
def 0), `blend` (0–1, def 1).
**Limites:** `out` segue o material de `in` (a média não amplifica);
clamp de segurança a ±8 como o `ABACUS`. CPU: por amostra ~2 comparações
+ 1 acesso a `recon_` + 1 `exp` por bloco (slew) + 1 `log` por evento de
`geiger`. `prepare()` aloca `2·K` floats (K≈1024 → ~8 KB). Sem RNG fora
do `geiger`.

## 6. Alternativas descartadas

- **VCO interno com 11 shapes** (como o AI250) — o RASGO já tem
  OSC/WAVETABLE/ADDITIVE/CHORD. O "modo oscilador" reusa o buffer
  reconstruído; não há tabela fixa nova.
- **Modo do `SH`** — o `SH` é grab instantâneo + track&hold + correlação
  de 2 canais, 9 params, propósito fechado. Integração em janela +
  empilhamento de N capturas + reconstrução por varredura é outra
  topologia (precisa do buffer de fase). Complementar, não duplicado.
- **Modo do `SCOPE`** — o `SCOPE` reduz (mede → escalar). O `BOXCAR`
  constrói (mede → forma de onda inteira). É o `SCOPE` do avesso.
- **Modo do `ABACUS`** — o `ABACUS` é aritmética/lógica entre 2 CVs +
  contador. A única peça "Function/Argument" do AI250 que interessa aqui
  é o **auto-trigger por limiar** (edge trigger), barato de somar e que o
  `ABACUS` não tem.
- **"Geiger gates" como módulo próprio** — é uma saída utilitária pequena
  e temática; cabe como saída secundária (ver decisão 3). Se o autor
  preferir, vira modo Poisson do `NOISE`.
- **Média linear com ring buffer de N por bin** — caro (K·N floats). A
  EMA com piso `min(N, hits)` dá o mesmo caráter com 1 float por bin.
- **Período fixo (assumido) como no boxcar de bancada** — quebra em
  qualquer patch com andamento vivo; o período medido + normalização de
  fase é o desvio Rasgo.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos (a verificar em `tests/test_boxcar.cpp`, Debug + Release):**
- `aperture` pequeno + `scan=0` + `mode 0` → nº de degraus na saída = nº
  de `trig`, e segura entre eles (idêntico a um S&H de fase fixa);
- `in` = seno coerente + ruído branco forte, `average=32`, `mode 1` →
  RMS(erro vs. seno puro) cai monotônico com o nº de capturas, e a
  variância residual ≈ σ²_ruído / N;
- `in` = só ruído (sem componente coerente) → `recon_` converge pra ~0
  (a média de ruído descorrelacionado);
- `scan=1` + `mode 1` → depois de ~N·scanCycles períodos, `out`
  reproduz a forma de `in` com atraso de 1 período;
- `trig` livre + `in` cruzando `thresh` → capturas acontecem nos
  cruzamentos (edge trigger), nº de capturas = nº de cruzamentos
  ascendentes;
- período do `trig` dobrado no meio do render → `mode 1` continue
  reconstruindo (sem NaN, só re-normaliza);
- `geiger=0` → saída `geiger` sempre 0; `geiger=1` → densidade média de
  pulsos ≈ λ(1) ev/s, e **dois renders byte-idênticos** (xorshift
  semeado);
- `blend=0` → `out == in` amostra a amostra; tudo finito; sem alocação
  em `process()`.

**Escuta:** a "revelação" (ruído → tom) soa mágica ou só abafada?
varrer `delay`/`aperture` na mão dá um gesto tátil (cabeça de leitura)
ou é imperceptível? o `mode 2` (wavetable auto-construída) tem vida
própria ou vira DC? o `geiger` no `ENVELOPE.gate` soa como chuva/
contador ou como erro? `average` alto num envelope-follower: "não treme"
ou "não responde"?

## 8. Integração e painel (proposta)

Classe `Boxcar` (`type()` = `"BOXCAR"`), 4 entradas, 2 saídas, 9
parâmetros. `panel()` próprio, **14 HP**, família DECISION (ao lado de
`ABACUS`/`QUANTIZER`/`HARMONY`/`DECISION`). `Display` = o buffer `recon_`
como mini-osciloscópio (o que ele está reconstruindo) + o marcador da
posição de abertura. Rótulos ≤5 chars: knobs **DLY / APER / AVG / SCAN**
(linha 1), **MODE / RATE / THR / GEI** (linha 2), **BLEND**; jacks
**IN / TRIG / SWP / THR** (entrada), **OUT / GEIG** (saída). Testado
isolado (S&H de fase fixa, redução de ruído, reconstrução, auto-trigger,
período variável, Poisson determinístico) antes de qualquer patch.
Cadeias canônicas: `CLOCK → BOXCAR.trig` · `OSC → in` · `out →
FILTER.cutoff_mod`; `AUDIO-IN → in` + `out` (áudio, mangler);
`BOXCAR.geiger → ENVELOPE.gate`. Entra em `apps/panel/ModuleCatalog.hpp`
(família DECISION) e `apps/panel/LearnCatalog.hpp` (11 binds).
