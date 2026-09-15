# PLANAR — morph vetorial XY

**Família:** ROUTE · **Módulo 43**
**Essência:** quatro timbres nos cantos de um quadrado e um ponto que
anda entre eles — por mão, por LFO, ou por um **gesto gravado**. E a
posição sai como CV pra mover o resto do patch.
**Dossiê técnico:** [`../dossies/43_planar.md`](../dossies/43_planar.md)
· **Fonte:** `src/dsp/Planar.hpp`

---

## A ideia

Síntese vetorial (Prophet VS, Korg Wavestation) e o joystick Buchla 208
são um gesto que o rack só faz com dois *crossfaders* amarrados na mão.
O `PLANAR` põe o plano no patch: quatro fontes (`A`/`B`/`C`/`D`) nos
cantos, um ponto `x`/`y` que interpola por peso bilinear. E como a
posição efetiva **sai** em `X'`/`Y'` como CV, o mesmo gesto dirige o
resto do patch — o `PLANAR` vira um sequenciador de trajetória contínuo.

## Por dentro

**O que é "peso bilinear", concretamente:** imagine as 4 fontes nos
cantos de um quadrado. Um ponto **exatamente** num canto usa só a
fonte daquele canto (peso 1) e nada das outras (peso 0); um ponto no
**centro** usa as 4 em partes praticamente iguais; um ponto na borda,
entre dois cantos, mistura só esses dois. "Bilinear" só quer dizer que
esse cálculo de peso é feito **duas vezes** — uma vez ao longo do eixo
`x`, outra ao longo do `y` — e combinado; é o mesmo princípio do
crossfade de um `MIXER` (uma linha), generalizado pra um plano inteiro.

**Por que existe `CURVE` (linear × potência constante), e o problema
que resolve:** se você simplesmente soma os pesos lineares de 2 fontes
de áudio (0,5 de cada, no meio do caminho entre elas), o volume
**percebido** no meio do crossfade soa **mais baixo** que nas pontas —
os dois sinais de áudio não se reforçam de forma simples ao serem
somados (a mesma questão do `1/√vozes` do `CHORD`, #26, só que aqui
ao contrário: os pesos lineares **subestimam** o volume no meio, em vez
de superestimar). "Potência constante" usa uma fórmula de peso
diferente (`1/√Σw²`) desenhada especificamente pra manter a energia
total **constante** em qualquer ponto do plano — o mesmo princípio por
trás das leis de panorâmica de mixagem de áudio profissional. Pra CV
(onde não existe "afundar" perceptível, e sim uma soma matemática que
você quer ver somando exatamente 1), o modo linear é mais previsível.

**`GESTURE`, o gravador de trajetória:** enquanto o gate estiver alto,
o módulo grava, amostra a amostra, **por onde** o ponto `x`/`y` passou
(por até ~4 s) — não é um "estado", é um **caminho completo** no
tempo. Na descida do gate, esse caminho gravado passa a se **repetir
em loop**, sozinho, sem mais precisar da sua mão nos knobs — a posição
volta a andar exatamente pelo trajeto gravado, indefinidamente. É
literalmente capturar um gesto físico (como desenhar com um joystick)
e devolvê-lo como automação reproduzível.

## Os jacks, um a um

### Entradas

- **`A`** / **`B`** / **`C`** / **`D`** (áudio) — as quatro fontes nos
  cantos. **Plugue aqui:** 4 `OSC`, 4 `WAVETABLE`, dois pares de
  percussão, ou 4 CVs (morph de modulação).
- **`X`** / **`Y`** (controle) — CV que soma aos knobs `X`/`Y` — move o
  ponto. **Plugue aqui:** dois LFOs de velocidades diferentes (o ponto
  desenha figuras), dois envelopes, `DRIFT.a`/`DRIFT.b`.
- **`GST`** (gesture) (controle, gate) — grava/reproduz a trajetória.
  **Plugue aqui:** um gate que você segura (`SIGNAL-IN.gate`), um
  `TRIGSEQ.t1` longo.

### Saídas

- **`OUT`** (áudio) — as 4 fontes misturadas pela posição. Vai ao
  `MIXER`.
- **`X'`** / **`Y'`** (x_out/y_out) (controle) — a posição **efetiva**
  como CV (já com `SMOOTH`, gesto, drift). **Plugue em:** qualquer
  `_mod` — o gesto do plano move o filtro, o reverb, o que for.

## Os controles, um a um

**X** / **Y** (0–1) — a posição do ponto. As CVs `X`/`Y` somam aqui.

**CURVE** (0–1) — linear (morph de CV) ↔ potência constante (áudio não
afunda no centro).

**SMTH** (smooth, 0–1) — glide de 1 polo no ponto (τ de ~0 a ~0,5 s).

**RATE** (0–1) — a velocidade do loop do gesto e da deriva.

**DRIFT** (0–1) — passeio 2D determinístico do ponto.

## Como cabear

**Pad vetorial que se move:**
```
4 × OSC (afinados em intervalos) → PLANAR (A/B/C/D)
LFO #1 (lento)  → PLANAR (X)
LFO #2 (mais lento) → PLANAR (Y)
PLANAR (OUT) → FILTER (in) → MIXER (ch1)
```
`CURVE` alto. O ponto desenha uma figura de Lissajous e o timbre morfa.

**Gesto gravado dirigindo o patch:**
```
PLANAR (OUT) → MIXER (ch1)
PLANAR (X') → SPACE (mix_mod)
PLANAR (Y') → FILTER (FC)
```
Segure o `GST`, desenhe uma trajetória com os knobs, solte — ela vira
loop e move o reverb e o filtro junto com o timbre.

## Potencializar

- **Trajetória como sequência:** grave um gesto que "visita" os 4 cantos
  numa ordem — vira um sequenciador de 4 timbres contínuo, com
  transições.
- **Morph de modulação:** 4 formas de LFO em `A`–`D`, o ponto num
  envelope — a *forma* da modulação muda ao longo da nota.
- **Deriva sobre gesto:** `DRIFT` pequeno + um gesto gravado — a
  trajetória "erra" um pouco a cada volta.
- **X' e Y' cruzados:** `X' → um destino`, `Y' → outro` com uma relação
  inversa — dois parâmetros que se movem em oposição pelo mesmo gesto.

## Se você conhece o Eurorack

Faz o papel do Intellijel Planar 2 e do joystick Buchla 208 — morph
vetorial com gestos graváveis. A base é a síntese vetorial Prophet VS /
Wavestation (domínio público, anos 80) e o pan de potência constante. O
**desvio Rasgo**: a posição efetiva sai como CV. Distinto do `SWITCH`
(comuta) e do `MATRIX` (grade de ganhos).
