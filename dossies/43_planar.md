# Dossiê — Módulo 43: Morph vetorial XY (`PLANAR`)

**Família:** ROUTE / MORPH (grupo ROUTE na paleta, junto de `SWITCH`/
`MATRIX`/`MULT` — `RASGO_MODULAR.md §4.1`)
**Estado:** **implementado — Onda B** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Planar.hpp`, `tests/test_planar.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda B, #43)

## Estado da implementação

Quatro fontes de áudio nos cantos de um quadrado (A sup-esq, B sup-dir,
C inf-esq, D inf-dir); um ponto `x`/`y` interpola entre elas por peso
bilinear. O que o `MIXER` faz numa linha, o `PLANAR` faz num plano — e o
ponto é **endereçável de três jeitos**:

1. **knobs + CV** (`x`/`y` somam);
2. **gesto gravado** — enquanto o gate `gesture` está alto, grava o ponto
   (decimado 32×, buffer de ~4 s); na descida, se gravou ≥ 3 quadros, o
   gesto passa a **tocar em loop** — o knob passa a ser ignorado e só a
   CV de `x`/`y` dá *nudge*. Toque curto (< 3 quadros) = limpa, volta ao
   ao vivo;
3. **deriva 2D autônoma** (`drift`) — quando não há gesto, o ponto passeia
   por uma soma de três senóides lentas incomensuráveis (Lissajous que
   não fecha em < ~6 min). **Determinística**, sem RNG.

**Desvio Rasgo:** a posição efetiva (já suavizada) **sai** como CV em
`x_out`/`y_out`. O gesto que você desenha num `PLANAR` de áudio pode
dirigir o `cutoff` de um `FILTER`, o `pos` de um `WAVETABLE`, outro
`PLANAR`… — "a relação é o processo".

- **`curve`** (0–1) = linear (0) ↔ potência constante (1). Linear é o
  morph honesto de CV (pesos somam 1); potência constante escala os
  pesos por `1/√Σw²` pra a saída não afundar ~6 dB no centro do quadrado
  (o certo pra áudio de fontes descorrelacionadas).
- **`smooth`** (0–1) = glide de 1 polo no ponto, `τ` de ~1 ms a ~0,5 s —
  de resposta imediata a *slew* que arrasta.
- **`rate`** (0–1, 0,5 = 1×) = `2^((rate−0,5)·4)` → 1/16× a 16×. Governa
  a velocidade do loop do gesto **e** da deriva.
- **`drift`** (0–1) = amplitude do passeio (±0,35 no quadrado).

**Segurança de saída** (gate 4): morph linear de fontes ≤ 1 é uma
combinação convexa → nunca passa de |1|, passa **bit-exato**. Só a
potência constante pode pedir até ~2× no centro; aí um `softclip`
(`tanh` acima de |v| = 1, assíntota ±1,5) satura suave. Saída em
(−1,5; 1,5), transparente até 1,0.

Sem alocação/lock/IO em `process()` (só `prepare()` aloca os 2 buffers de
gesto, ~48 KB). Determinístico sempre — a máquina de estado do gesto e as
fases de deriva evoluem só do tempo e das entradas.

**Testes (Debug + Release):** canto puro (`x=y=0`) → só A na saída;
centro (`x=y=0,5`, `curve=0`) → média dos quatro, −6 dB; centro com
`curve=1` → nível de volta a ~0 dB (potência constante); `x` varre
A→B linearmente com `curve=0`; `smooth` alto → degrau em `x` vira rampa
de centenas de ms; deriva move o ponto com `drift>0` e fica parada com
`drift=0`; grava um gesto (gate 200 ms varrendo `x`), solta, e
`x_out` reproduz a varredura em loop; toque curto não deixa gesto;
`x_out`/`y_out` seguem o ponto; dois renders byte-idênticos (inclui
`drift>0` e gesto); saída sempre finita e < 1,5 (morph linear ≤ 1).

**Pendências (candidatos):** `size`/zoom do gesto (Planar 2); rotação/
espelho do gesto; N fontes (hexágono, não só quadrado); quantização do
ponto a uma grade; segunda saída de gesto atrasada (canon XY); gravar
vários gestos e escolher por CV.

---

## 1. Problema musical e papel no fluxo

Síntese vetorial (Prophet VS, Korg Wavestation) e o joystick Buchla 208
são um gesto que o rack modular só faz com dois crossfaders amarrados na
mão. `PLANAR` põe o plano no patch: quatro timbres (quatro `OSC`, quatro
`WAVETABLE`, dois pares de percussão…) e um ponto que anda entre eles —
por mão, por LFO/envelope em `x`/`y`, ou por um gesto gravado. E como a
posição sai como CV, o mesmo gesto move o resto do patch: o `PLANAR` vira
um **sequenciador de trajetória** contínuo.

Distinção dos vizinhos: o `SWITCH` comuta (uma fonte por vez, com
*glide*); o `MIXER` soma com nível/pan fixos; o `MATRIX` é roteamento
N×M de ganhos. `PLANAR` é a interpolação contínua de 4 num ponto 2D, com
o ponto gravável.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Síntese vetorial** (Sequential Prophet VS, Korg Wavestation) —
  joystick que faz *crossfade* de 4 osciladores; a "envelope vetorial"
  que grava e reproduz o movimento do joystick. Domínio do conceito
  (anos 80), documentado à exaustão.
- **Buchla 208 / Easel** — o joystick como fonte de gesto e de CV
  simultânea.
- **Intellijel Planar 2** (*referência funcional* ★, `PESQUISA §7 #33`) —
  o formato Eurorack: 4 entradas, joystick, gravação de trajetória em
  loop, saídas X/Y de CV, *size*. Fechado; só o **conjunto de gestos**
  (gravar/reproduzir/emitir posição), não o código. A origem pública é a
  síntese vetorial acima.
- **Crossfade de potência constante** — `Σw² = const` mantém a potência
  de saída pra fontes descorrelacionadas; teoria de mixagem (pan law),
  domínio público. Aqui generalizada de 1D (pan) pra os 4 pesos
  bilineares.

**Desvio Rasgo (Atlas §49):** a saída de posição como CV (o gesto dirige
o grafo), a deriva 2D determinística (o ponto passeia sozinho, sem RNG,
reprodutível), e a integração — nada de joystick de hardware; o "gesto" é
uma trajetória de CV gravada de qualquer fonte do patch.

## 3. Modelo

**Pesos bilineares** (`x`, `y` ∈ [0,1], já suavizados):
```
wa = (1−x)(1−y)   wb = x(1−y)   wc = (1−x)y   wd = xy      (Σ = 1)
g     = 1 / √max(wa²+wb²+wc²+wd², 1e−6)
scale = 1 + curve·(g − 1)                (curve 0 → 1 ; curve 1 → g)
raw   = scale · (wa·A + wb·B + wc·C + wd·D)
out   = softclip(raw)
```
`softclip(v)`: `|v|≤0,92 → v`; senão `±(0,92 + 0,08·tanh((|v|−0,92)/0,08))`.

**Ponto alvo por amostra:**
```
gravando:   tx,ty = clamp01(xKnob + xCv), clamp01(yKnob + yCv)
            a cada 32 amostras: grava (tx,ty) em gx_/gy_[recPos_++]
tem gesto:  playPos_ += rateMul/32 ; wrap [0, recLen_)
            g = lerp(buf, playPos_)
            tx,ty = clamp01(g + cv)          (knob ignorado; CV = nudge)
ao vivo:    tx,ty = clamp01(knob + cv + drift)
smX_ += (tx − smX_)·(1 − e^(−dt/τ)),  τ = 0,00006 + smooth²·0,5
```

**Deriva** (`drift>0`, só ao vivo): três fases avançam a
`{0,030; 0,022; 0,041}·rateMul` Hz;
```
drX = 0,35·drift·(0,6·sin 2π·ph1 + 0,4·sin(2π·ph3 + 1,3))
drY = 0,35·drift·(0,6·sin(2π·ph2 + 0,7) + 0,4·sin(2π·ph1·1,37 + 2,1))
```

**Máquina de estado do gesto:** borda de subida de `gesture` → grava do
zero. Borda de descida → `recPos_ ≥ 3` ? vira loop : limpa.

**Extremos:** `x`/`y` fora de [0,1] por CV → clampados (o ponto encosta na
borda, não sai). `recLen_` mínimo garantido ≥ 3 antes de `hasGesture_`.
`rate` no mínimo → loop 16× mais lento (gesto de 4 s dura ~64 s). Todas
as 4 entradas em ±1 e em fase, `curve=1`, centro → `raw ≈ 2` → `softclip`
segura em < 1,5. Nenhuma entrada conectada → saída 0 (silêncio, nunca
lixo).

## 4. Três modos obrigatórios

- **autônoma:** 4 fontes de áudio nos cantos, `drift` leve, `smooth`
  médio → o ponto passeia sozinho e a textura se recombina; sem nenhum
  cabo de controle já é uma peça.
- **performance:** `x`/`y` nos knobs (ou num joystick MIDI via
  adaptador) são o gesto; `curve` decide se é morph de áudio ou de CV.
- **híbrida:** grava um gesto varrendo o quadrado, solta pra ele tocar em
  loop, e cabeia `x_out → FILTER.cutoff` — o mesmo movimento colore o
  filtro; `ENVELOPE → y` empurra o ponto na vertical a cada nota.

## 5. Portas, parâmetros, limites

**Entradas:** `a`/`b`/`c`/`d` (Audio — cantos), `x`/`y` (Control),
`gesture` (Control gate).
**Saídas:** `out` (Audio), `x_out`/`y_out` (Control — a posição suavizada).
**Parâmetros:** `x` (0–1, def 0,5), `y` (0–1, def 0,5), `curve` (0–1,
def 0,5), `smooth` (0–1, def 0), `rate` (0–1, def 0,5 = 1×), `drift`
(0–1, def 0).
**Limites:** `out` em (−1,5; 1,5) (`softclip` acima de 1; morph linear é
bit-exato). CPU: por amostra ~4 mul-add +
1 `sqrt` + 3 `sin` (deriva) + 1 `exp` no cálculo do coef (poderia subir
pro nível de bloco — pendência de orçamento). `prepare` aloca 2×6000
floats (~48 KB).

## 6. Alternativas descartadas

- **joystick de hardware / entrada MIDI direta** — fere "o painel abre e
  soa sem periférico"; o adaptador MIDI (`§36.8`) leva o joystick pra
  `x`/`y` como qualquer CV.
- **gravar o gesto em taxa de áudio** (sem decimar) — 4 s = 192 k
  amostras × 2, e o ponto não tem conteúdo acima de ~100 Hz; decimar 32×
  (1,5 kHz) sobra e corta o buffer pra 48 KB.
- **RNG no `drift`** — a soma de 3 senóides incomensuráveis já dá passeio
  não-repetitivo e determinístico; RNG só tiraria a reprodutibilidade.
- **N fontes / polígono** — o quadrado é o caso vetorial clássico e casa
  com os 4 cantos do painel; hexágono fica como pendência.
- **`ADDITIVE`-style gain follower** em vez de `softclip` — o follower
  bombearia com o gesto; a potência constante já é o modelo de nível
  correto, o `softclip` é só a rede de segurança.

## 7. Integração e painel

12 HP, família **ROUTE** (junto de `SWITCH`/`MATRIX`/`MULT`). Display do
quadrado com o ponto (o painel gráfico pode desenhar a trajetória do
gesto). Knobs `X`/`Y`/`CURVE` (linha 1), `SMTH`/`RATE`/`DRIFT` (linha 2);
jacks `A`/`B`/`C`/`D` (cantos), `X`/`Y`/`GST` (controle), `OUT`/`X'`/`Y'`.

Cadeias canônicas: `4× OSC → PLANAR.a..d`, `LFO → PLANAR.x`,
`LFO → PLANAR.y` (síntese vetorial); `PLANAR.x_out → FILTER.cutoff`
(o gesto colore o patch); `CLOCK → PLANAR.gesture` não faz sentido (é
gate longo) — o gesto é gravado à mão ou de um `FUNCTION` em modo
*one-shot* longo.
