# Dossiê — Módulo 58: Deslocador de frequência (`SHIFTER`)

**Família:** TRANSFORM (modifica um sinal que passa)
**Estado:** **implementado** (2026-09-07)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Shifter.hpp`, `tests/test_shifter.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.6` (Tier 1) — Onda F. Harald Bode /
Bode-Moog frequency shifter (SSB, anos 1960 — conceito público)

## Estado da implementação

O `SHAPE` faz **ring-modulation**: multiplica IN por uma portadora,
gerando as bandas SOMA e DIFERENÇA simétricas em torno dela. Um
**deslocador de frequência** de verdade move o espectro INTEIRO por um
Δf **fixo em Hz** e entrega **só uma** banda lateral (SSB — single
sideband). Porque o deslocamento é aditivo em Hz e não multiplicativo em
razão, os intervalos entre parciais deixam de ser harmônicos → o som
fica metálico, sineiro, inarmônico. Nada no RASGO fazia isso.

- **`shift`** (−2000..2000 Hz, +CV em `shift_mod`, 1 unidade = 1000 Hz):
  o deslocamento. `0` = passa-direto (com o atraso do FIR).
- **`feedback`** (−0,95..0,95): a saída `up` volta pra entrada → o
  espectro sobe de novo, e de novo — glissando infinito (o "barber
  pole" / a espiral de Risset-Shepard). `feedback` negativo realimenta
  de `down` (desce sem parar). `tanh` no laço → nunca explode.
- **`tone`** (−1..1): inclina o molhado (1 polo). <0 abafa o agudo, >0
  realça.
- **`drift`** (0–1, **desvio Rasgo**): wobble lento e SEMEADO no Δf
  (±drift·50 %) — o deslocamento respira.
- **`mix`** (0–1): seco (IN) ↔ deslocado, **nas duas saídas**.

**Saídas:** `up` (espectro + Δf), `down` (espectro − Δf). A relação
entre as duas é o processo (idioma Three Sisters) — cabeie em destinos
diferentes.

**SSB por transformada de Hilbert:** FIR de 255 taps (kernel `2/(πk)`
para `k` ímpar, janela de Blackman) para a componente imaginária + linha
de atraso casada (127 amostras, ~2,6 ms) para a real → modulação em
quadratura (`re·cos ∓ im·sin`). **Rejeição de imagem > 50 dB acima de
~250 Hz**; abaixo disso a rejeição cai (o FIR curto não consegue Hilbert
no grave) e o som tende ao ring-mod — mesma limitação dos deslocadores
de hardware. `process()` não aloca (o buffer vem no `prepare()`).
`drift=0` → determinístico byte a byte.

**Testes (Debug + Release):** `shift=0` → energia fica em `f` (nada
mais); `shift=+120` numa senóide de 440 → a saída `up` tem energia em
560 (= 440+120), NADA em 440, e a imagem em 320 fica > 15× abaixo;
`down` → 320 forte, imagem 560 fraca; 300+600 Hz + `shift=+100` →
energia em 400 e 700 (NÃO em 600 = 300·2 — é aditivo, não escala);
`shift` negativo troca os papéis de `up`/`down`; CV `shift_mod` desloca;
`feedback=0,75` → a energia sobe em degraus acima da fundamental
(barber pole), tudo finito < 1,2; `mix=0` = bypass byte-exato; `tone`
baixo abafa o agudo; `drift` semeado (2 renders idênticos) e modesto
(RMS não colapsa nem dobra); tudo no talo → |out| < 1,3.

**Pendências (candidatos):** FIR mais longo / rede all-pass IIR pra
melhorar o grave; `shift` com taper exponencial no painel (controle fino
perto de 0); saída de quadratura crua (`re`/`im`) pra patch de Dome;
range "fino" (±100 Hz) por toggle; `feedback` com filtro no laço
(barber pole com timbre).

---

## 1. Problema musical e papel no fluxo

O deslocador de frequência é um clássico do estúdio (Bode, anos 1960;
usado por Stockhausen em *Mixtur* e *Hymnen*) e continua raro no formato
modular. Usos:

- **efeito de "fase infinita":** Δf de 1–10 Hz — as cópias deslocadas
  batem com o seco num beating que **nunca fecha** (um flanger sem ciclo).
- **des-harmonização:** Δf de 50–300 Hz num acorde ou pad → clangor de
  sino, vidro, metal — a altura "some", vira textura.
- **barber pole / Shepard:** `feedback` alto + Δf pequeno → glissando
  eterno (a corda de Risset).
- **estéreo do inaudito:** `up` num canal, `down` no outro → o espectro
  se abre em leque.

No fluxo: TRANSFORM, entre a voz e o filtro/espaço. Quase sempre com
`in` cabeado. Distinto do `SHAPE` (ring-mod simétrico) e do `SPECTRA`
(re-síntese que segue a altura).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

- **Harald Bode**, *A New Tool for the Exploration of Unknown
  Electronic Music Instrument Performances* (JAES, 1961) + o Bode/Moog
  Frequency Shifter — o conceito de SSB por rede de defasagem. Fechado,
  descontinuado; **conceito público, sem código**.
- **Modulação em banda lateral única (SSB)** — teoria de rádio (método
  Hartley, 1928; método Weaver, 1956). `shifted = re·cos(2πΔf t) ∓
  im·sin(2πΔf t)` onde `im` = Hilbert(`re`).
- **Transformada de Hilbert** — teoria clássica de processamento de
  sinais; realização por FIR (kernel `h[n] = 2/(πn)` ímpar, 0 par) +
  janela; ou por rede all-pass IIR (polyphase). O RASGO usa FIR de 255
  taps com janela de Blackman.
- **Zölzer, *DAFX: Digital Audio Effects*** — o capítulo de *frequency
  shifting* (a fórmula SSB + a rede de quadratura).
- **Barber-pole / ilusão de Shepard-Risset** — Roger Shepard (1964),
  Jean-Claude Risset (1969) — teoria pública; o `feedback` no
  deslocador é a realização modular.

**Desvio Rasgo obrigatório:** as DUAS saídas `up`/`down` simultâneas (o
Bode/Moog tinha uma chave); o `drift` semeado no Δf; o `feedback` com
`tanh` no laço (auto-limita, o "barber pole" nunca satura feio).

## 3. Modelo — matemática, estados, extremos

Por amostra:

```
df   = shiftSmooth · (1 + drift · lfo)                  lfo = passeio semeado
x    = tanh( dry + fbTap · |feedback| )                 fbTap = up  se fb≥0, senão down
buf[wr] = x ;  re = buf[wr − 127]                       atraso casado (M = 127)
im   = Σ_{k ímpar} h[k] · buf[wr − k]                   h[k] = 2/(πk) · Blackman
osc += df / sr  (mod 1)
c = cos(2π osc) ; s = sin(2π osc)
up = re·c − im·s          (espectro + df)
dn = re·c + im·s          (espectro − df)
toneZ += (½(up+dn) − toneZ) · toneCoef
tone<0 → up,dn ← lerp(·, toneZ, −tone) ;  tone>0 → up,dn += (· − toneZ)·tone
up = softClip(up) ; dn = softClip(dn)                   joelho em ±0,8
fbUp = up ; fbDown = dn
out_up = lerp(dry, up, mix) ; out_down = lerp(dry, dn, mix)
```

**Extremos:** `shift=0` → `re` é `x` atrasado, `im·s` com `s=0` → saída
= `x[n−127]` (passa-direto com atraso). `|shift|` grande → o espectro
pode passar de Nyquist (dobra) ou de 0 Hz (reflete pra frequências
negativas = espelha) — comportamento honesto do SSB. `feedback` perto
de 1 + Δf pequeno → glissando que sobe sem parar, `tanh` segura em ±1.
Grave abaixo de ~150 Hz → o FIR não faz Hilbert direito, `im` fica
pequeno → as duas bandas vazam (vira ring-mod).

## 4. Três modos obrigatórios

1. **repouso / passa-direto** — `shift≈0`, `feedback=0`, `mix=1`: a
   saída é o seco atrasado 127 amostras. `mix=0` = bypass exato (sem
   atraso).
2. **deslocamento** — `shift≠0`: `up`/`down` carregam as duas bandas.
   `mix` mistura o seco. Sem `feedback`.
3. **barber pole** — `feedback` alto + `shift` pequeno: glissando
   infinito. `feedback` positivo sobe, negativo desce.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (áudio), `shift_mod` (CV → soma em `shift`, ×1000 Hz).
**Saídas:** `up`, `down`.
**Parâmetros:** `shift` −2000..2000 Hz (def 0) · `feedback` −0,95..0,95
(def 0) · `tone` −1..1 (def 0) · `drift` 0–1 (def 0) · `mix` 0–1 (def 1).

## 6. Alternativas descartadas

- **rede all-pass IIR (polyphase Hilbert)** — sem latência, mais barata,
  mas os conjuntos de coeficientes testados (Niemitalo 4+4) davam
  quadratura boa só numa faixa estreita; um conjunto largo precisa de
  muitas seções. O FIR de 255 taps é robusto e a latência (2,6 ms) é
  irrelevante pro caso de uso. Fica como pendência.
- **ring-mod com portadora afinável** — isso é o `SHAPE`. O ponto do
  `SHIFTER` é a banda ÚNICA.
- **pitch shift (deslocamento em RAZÃO, preservando harmônicos)** —
  outra coisa; o `SPECTRA` (`shift`) e o `SAMPLER` (`repitch`) cobrem.
- **`shift` bipolar com um só output** (o Bode original) — as duas
  saídas simultâneas são mais RASGO (a relação como processo).

## 7. Critérios técnicos e perguntas de escuta

- `shift=0` → energia só em `f`.
- `shift=+Δ` → `up` em `f+Δ`, imagem em `f−Δ` > 15× abaixo (acima de
  ~250 Hz); `down` o espelho.
- deslocamento é ADITIVO em Hz (dois parciais harmônicos deixam de ser).
- `shift` negativo troca `up`↔`down`.
- CV `shift_mod` desloca (×1000 Hz).
- `feedback` alto → energia sobe em degraus (barber pole), finito.
- `mix=0` → bypass byte-exato.
- `tone<0` abafa o agudo do molhado.
- `drift` semeado (2 renders idênticos), modesto (RMS estável).
- tudo no talo → |out| < 1,3.
- **escuta:** Δf de 3 Hz num pad — o beating nunca fecha. Δf de 200 Hz
  num acorde — vira sino. `feedback` 0,8 + Δf 8 Hz — a espiral que sobe
  pra sempre.

## 8. Integração e painel

**Catálogo:** TRANSFORM, depois do `SHAPE`
(`… VCA · SHAPE · SHIFTER · CRUSH · PARAMETRIC · …`).
**Painel:** 10 HP. Display "shifter" (46 mm). SHIFT/FBK na 1ª fileira;
TONE/DRIFT na 2ª; MIX na 3ª. Jacks IN/SFT e UP/DN.
**LEARN:** 9 binds (3 níveis) + a definição do módulo.
**CTest:** `rasgo_modular_shifter_tests` (13 testes).
