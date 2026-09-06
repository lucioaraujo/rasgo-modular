# Dossiê — Módulo 40: Oscilador de tabela procedural (`WAVETABLE`)

**Família:** SOURCE
**Estado:** **implementado — Onda A** (2026-09-06)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Wavetable.hpp`, `tests/test_wavetable.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.4` (Onda A, #40)

## Estado da implementação

O `OSC` é subtrativo (5 formas fixas antialias). `WAVETABLE` é o
território que falta: um **eixo de forma** varrido por CV, com quadros
band-limited pra qualquer afinação.

**Sem arquivo de dados** (decisão do autor 2026-09-06, opção A): os
quadros são **gerados no `prepare()`** por uma receita espectral fixa —
`saw → square → formante → seno` — determinística, sem RNG. 16 quadros,
1024 amostras/ciclo, 10 mip-maps band-limited (harmônica máxima 512, 256,
… 1) pra o antialiasing seguir a afinação.

- **`pos`** (0–1, + CV) = posição no eixo de forma;
- **`warp`** (0–1) = distorção de fase estilo Casio CZ / "WAVE CUT" do
  EMW WAVE-6: comprime a primeira metade do ciclo numa janela que
  encolhe — de identidade (0) a quase-pulso brilhante (1);
- **`freq`**/`fine`/`pitch` (1 V/oct)/`fm` (linear) = afinação, padrão
  do `OSC`;
- **`drift`** (0–1) = varredura lenta autônoma de `pos` — a wavetable
  "respira". `drift = 0` + sem captura → **determinístico**.

**Captura ao vivo** (desvio Rasgo — resposta a "se houver fonte no
`AUDIO-IN`, funcionaria carregar de fora?"): entradas **`capture`**
(áudio) + **`grab`** (trigger). Na borda de `grab`, uma janela de 1024
amostras de `capture` é copiada, tem o DC removido e é normalizada — vira
o quadro no **topo** do `pos` (de ~0,9 a 1,0 faz o crossfade do último
quadro procedural pra o capturado). Sem detecção de pitch — a fonte se
afina de ouvido, e o "erra um pouco" é o charme do wavetable capturado.
Standalone = procedural; com `AUDIO-IN` (ou um `OSC`, um `SPACE`…) no
`capture` = tabela viva. O quadro capturado não é band-limited (mip
único) — o aliasing em afinação alta é caráter aceito, anotado.

Sem alocação / lock / IO em `process()` (só `prepare()` aloca as
tabelas, ~1 MB). Determinístico exceto por `drift`/captura.

**Testes (Debug + Release):** `pos = 1.0`, `warp = 0` → seno puro (só
harmônica 1, THD baixa); `pos = 0` → serra (espectro 1/h, todas as
harmônicas); `pos` intermediário → mistura monotônica dos dois; `warp`
sobe o centroide espectral (mais brilho) sem estourar; afinação segue
1 V/oct (uma oitava = ×2 na taxa de cruzamentos de zero); mip escolhido
mantém a harmônica máxima < Nyquist em todo o alcance de `freq` (sem
componente acima de sr/2 audível); `fm` linear desvia a taxa; `drift = 0`
→ dois renders byte-idênticos; `grab` copia a janela e o quadro
capturado aparece em `pos ≈ 1`; sem `capture` conectado → `grab` não faz
nada, `pos = 1` continua seno; tudo finito.

**Pendências (candidatos):** band-limit do quadro capturado (mip por
decimação); 2º eixo de tabela (grade 2D, `pos_y`); import de `.wav` como
*decisão consciente por arquivo* se A não bastar (opção B do §2.4);
`unison`/super-saw da tabela; edição de espectro no painel.

---

## 1. Problema musical e papel no fluxo

`SEQUENCE → QUANTIZER → OSC → FILTER` é síntese subtrativa clássica. Uma
paleta inteira — pads que evoluem, leads digitais, o "digital que soa
analógico" do WAVE-6 — vive de **varrer uma forma de onda no tempo**, não
de tirar harmônicos de uma serra. `WAVETABLE` põe esse eixo no patch:
entra 1 V/oct, sai áudio, e `pos` (knob, LFO, envelope, `DRIFT`) move a
forma. A captura fecha o círculo com o `AUDIO-IN` — o instrumento pode
"amostrar um ciclo" do que ouve.

## 2. Fontes ESTUDADAS (conceito, não código)

- **Wavetable clássico (PPG / Waldorf / Serum)** — banco de quadros de
  1 ciclo, interpolação entre quadros, mip-maps por band-limit. Teoria
  pública; tabelas GERADAS aqui, não importadas.
- **EMW WAVE-6** (hardware do autor, `PESQUISA §9 §12`) — wavetable de
  6 vozes + **WAVE CUT** tipo PWM → o `warp`.
- **Casio CZ phase distortion** — comprimir a fase pra clarear a forma
  sem mudar a tabela; o modelo do `warp`.
- **síntese aditiva band-limited** — cada mip é a soma das harmônicas
  abaixo de um teto; escolher o mip pela fundamental = antialiasing por
  construção. Ref: teoria de série de Fourier (domínio público).
- **captura de ciclo único** (Serge Wave Multipliers / hardware barato) —
  "grab" de uma janela do sinal de entrada como forma de onda.

## 3. Modelo

**Geração (`prepare`, fora do RT):**
```
sinLUT[i] = sin(2π i / 1024),  i em [0,1024)
pra cada quadro f em [0,16):
  amp[h] = recipe(h, f/15)          // ver §3.1
  pra cada amostra s em [0,1024):
    acc = 0
    pra h em 1..512:
      acc += amp[h] · sinLUT[(h·s) mod 1024]
      se h é potência de 2:  tbl[f][mip(h)][s] = acc   // snapshot band-limited
```
`mip(h)`: `h = 512>>m` → mip `m`. mip 0 = 512 harmônicas, mip 9 = 1.

**Render (por amostra):**
```
f0 = freq · 2^(fine/1200 + pitch_in)
f0 · = 1 + fm_in · fm_amount · 4           (FM linear; f0 clampado)
phase += f0/sr;  phase −= floor(phase)
wp = warp(phase)                           (distorção de fase CZ)
mip = clamp(ceil(log2(1024·f0/sr)), 0, 9)  (harmônica máx · f0 < Nyquist)
slot = clamp(pos + pos_in + drift, 0, 1) · maxSlot
  maxSlot = captura válida ? 16 : 15
a = readFrame(⌊slot⌋,   mip, wp)
b = readFrame(⌊slot⌋+1, mip, wp)
out = a + (b−a)·(slot − ⌊slot⌋)
readFrame(16, ·, ·) = readCap(wp)          (interp linear no captureBuf)
```

**`warp(ph)`** (CZ / WAVE CUT): `k = 0,5 − 0,45·warp`;
`ph < k ? ph·(0,5/k) : 0,5 + (ph−k)·(0,5/(1−k))`. `warp = 0` → identidade.

### 3.1 Receita espectral

4 arquétipos, crossfade linear ao longo de `pos ∈ [0,1]` (`seg = pos·3`):
- **serra** (`pos 0`): `amp[h] = 1/h`, todas;
- **quadrada** (`pos ⅓`): `amp[h] = 1/h` só ímpares;
- **formante** (`pos ⅔`): três sinos gaussianos centrados em `h ≈ 3, 8,
  14` (σ ≈ 1,5) — vogal aproximada;
- **seno** (`pos 1`): `amp[1] = 1`, resto 0.
Cada quadro normaliza a soma pra pico ≈ 0,9.

**Extremos:** `freq` no máximo (8 kHz) → mip 0..1, ~2 harmônicas → quase
seno, sem alias. `warp = 1` + serra → forma quase-impulso, mip alto puxa
pra baixo o conteúdo. `pos` fora de [0,1] (CV) → clampado. `capture` sem
cabo → `grab` ignorado, `maxSlot = 15`. Buffer de captura com DC forte →
removido; silêncio → normalização por um piso (não divide por zero).

## 4. Três modos obrigatórios

- **autônoma:** `freq` audível, `pos`/`warp` médios, `drift` leve → um
  timbre que evolui sozinho; sem nenhum cabo funciona.
- **performance:** `pos` e `warp` são os macros expressivos; `freq` a
  afinação.
- **híbrida:** `pos` de um `ENVELOPE`/`LFO`/`DRIFT`; `capture` de um
  `AUDIO-IN` ou de outra voz do grafo — a tabela vem do que o
  instrumento ouve.

## 5. Portas, parâmetros, limites

**Entradas:** `pitch` (Control v/oct), `pos` (Control), `fm` (Audio),
`capture` (Audio), `grab` (Control trig).
**Saídas:** `out` (Audio).
**Parâmetros:** `freq` (8–8000 Hz, def 110), `fine` (−100..100 cent),
`pos` (0–1), `warp` (0–1), `fm_amount` (0–1), `drift` (0–1).
**Limites:** `out` em ~[−1,1] (tabelas normalizadas a 0,9; `warp` não
adiciona ganho). CPU: por amostra 4 leituras de tabela interpoladas +
1 `log2` + `warp`. `prepare` gera ~8 M amostras·harmônica (~50 ms) e
aloca ~1 MB por instância.

## 6. Alternativas descartadas

- **importar `.wav`** (opção C) — quebra "sem dependência" e o
  determinismo do patch. A captura ao vivo dá o mesmo sem o custo.
- **tabelas embutidas** (opção B) — mais caráter, mas vira dado no repo;
  fica como evolução se a receita procedural soar pobre.
- **um mip só + oversampling forte** — mais CPU no RT e ainda alia; o
  mip-map resolve por construção.
- **`ADDITIVE` no lugar** — é o #42 (Onda B); `WAVETABLE` é a forma
  varrida, não o controle harmônico por parcial.

## 7. Integração e painel

12 HP, família **SOURCE** (junto do `OSC`/`PLL`/`CHORD`). Display da
forma (o osciloscópio da saída já mostra). Knobs `FREQ`/`FINE`/`POS`/
`WARP` (linha 1), `FM`/`DRIFT` (linha 2); jacks `1V/O`/`POS`/`FM` +
`CAP`/`GRAB` + `OUT`.

Cadeias canônicas: `SEQUENCE → QUANTIZER → WAVETABLE → FILTER`;
`ENVELOPE.env → WAVETABLE.pos` (a forma abre com a nota);
`AUDIO-IN → WAVETABLE.capture`, `CLOCK → WAVETABLE.grab` (tabela viva
que se renova no compasso).
