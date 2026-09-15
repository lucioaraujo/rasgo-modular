# FUNCTION — gerador de função

**Família:** MODULATE · **Módulo 1**
**Essência:** uma rampa que é envelope, LFO ou oscilador conforme a
taxa — a mesma matemática em escalas de tempo diferentes.
**Dossiê técnico:** [`../dossies/01_gerador_de_funcao.md`](../dossies/01_gerador_de_funcao.md)
· **Fonte:** `src/dsp/FunctionGenerator.hpp`

---

## A ideia

Numa lógica generativa, uma fonte de modulação não devia ter função
fixa. O `FUNCTION` é **uma rampa** — e envelope, LFO e oscilador são a
mesma função, só muda a velocidade. Em `RATE` baixo (0,01 Hz) é um LFO
lentíssimo; em `RATE` alto (kHz) é um tom audível; no meio, um envelope
disparável. A função depende de onde ele está no fluxo, não de um
seletor de modo.

É o **LFO padrão** do Rasgo — quando uma página diz "cabeie um LFO
aqui", geralmente é um `FUNCTION` em taxa baixa.

## Por dentro

**O que é um "acumulador de fase", pra quem nunca ouviu o termo:** é só
um número que sobe devagarinho, amostra por amostra, de 0 até 1 — e
quando chega em 1, volta pra 0 e recomeça. Pense num ponteiro de
relógio dando voltas: a **fase** é "em que ponto da volta ele está
agora" (0 = começando, 0,5 = na metade, 1 = completando). `RATE` decide
**quantas voltas por segundo** esse ponteiro dá — é aí que a mesma
engrenagem vira LFO (poucas voltas por minuto), envelope (uma volta só,
disparada) ou oscilador de áudio (centenas/milhares de voltas por
segundo, rápido demais pra contar — o ouvido ouve isso como um tom, não
como um movimento).

**O que `SLOPE` faz com essa fase:** ele decide **que forma** a saída
tem ao longo de uma volta — não a velocidade, a forma. Pensando na
volta como indo de 0% a 100%:

- **`SLOPE`=0:** a saída começa no topo e desce **a volta inteira** até
  o fim, aí pula de volta ao topo — um dente-de-serra **descendente**.
- **`SLOPE`=0,5:** a saída sobe na **primeira metade** da volta e desce
  na **segunda metade** — um triângulo simétrico.
- **`SLOPE`=1:** a saída sobe **a volta inteira** e cai só no instante
  de reiniciar — um dente-de-serra **ascendente**.
- **valores entre esses três:** a proporção de subida/descida desliza
  continuamente — `SLOPE`=0,1 sobe rapidinho e desce devagar quase o
  resto todo da volta (quase um dente-de-serra ↓, mas com uma pontinha
  de subida); `SLOPE`=0,9 é o espelho disso.

Ou seja: **`SLOPE` não muda o que a rampa É, muda o formato dela** — e
esse mesmo formato serve pra três papéis diferentes dependendo da
velocidade: em `RATE` de LFO, `SLOPE`=0 é um LFO que cai devagar e
salta rápido (útil pra "soltar" algo de repente); em `RATE` de
envelope disparado (ver "como cabear" abaixo), `SLOPE`=0 vira um
decaimento simples (ataque instantâneo, cauda que morre); em `RATE` de
áudio, `SLOPE` muda o **timbre** — cada forma tem um conteúdo harmônico
diferente (serra é rica em todos os harmônicos, triângulo é mais pobre,
só os ímpares e mais fracos).

`SYNC` (com a chave ligada) força a fase de volta a 0 na hora que chega
um pulso externo — é literalmente "reinicia a volta agora", útil pra
manter o LFO **alinhado** com um clock em vez de derivar livre.

`DRIFT` faz a **taxa efetiva** (não `RATE` em si, o valor real usado a
cada instante) passear devagar pra cima e pra baixo, sozinha, de um
jeito lento e correlacionado (não é ruído aleatório rápido — é mais como
uma respiração). **Por que isso importa:** um LFO perfeitamente
constante repete a **mesma** volta, idêntica, pra sempre — o que soa
"mecânico", porque nenhum movimento do mundo real é assim tão perfeito
(uma mão balançando, um circuito analógico, todos variam um pouco a
cada repetição). Um toque de `DRIFT` reintroduz essa imperfeição de
propósito.

## Os jacks, um a um

### Entradas

- **`RATE`** (controle, 1 V/oct) — CV que **multiplica** o knob `RATE`.
  **Plugue aqui:** um `SEQUENCE` (a velocidade do LFO muda por passo),
  outro `FUNCTION` lento (FM de LFO), `NOISE.smooth`.
- **`SLOPE`** (controle) — soma ao knob `SLOPE`. **Plugue aqui:** um
  `ENVELOPE` — a forma da onda muda ao longo da nota.
- **`SYNC`** (controle, disparo) — reinicia a fase. **Só com a chave
  `SYNC` ligada.** **Plugue aqui:** `CLOCK.euclid`, `SEQUENCE.eos` — o
  LFO trava no compasso.

### Saídas (ambas áudio)

- **`UNI`** (0..1) — **unipolar**: nunca fica negativa, só varia entre
  "nada" (0) e "máximo" (1). Faz sentido em qualquer parâmetro que não
  tem um lado "negativo" que faça sentido físico — não existe volume
  negativo, nem "abertura de filtro negativa". **Plugue em:** entradas
  de amplitude — `VCA.cv`, abertura de filtro pra um lado, `pos` de
  wavetable.
- **`BI`** (−1..1) — **bipolar**: varia dos dois lados de zero — sobe
  acima e desce abaixo de um ponto central. Faz sentido em parâmetros
  onde "pra cima" e "pra baixo" são igualmente válidos — uma altura
  pode subir OU descer a partir da nota base (vibrato), uma modulação
  de FM pode adiantar OU atrasar a fase. **Plugue em:** entradas de
  altura (`OSC.1V/O` — vibrato), `FM`, qualquer `_mod` que deva ir pros
  dois lados de um valor central. Ligar `BI` numa entrada que só faz
  sentido positiva (como `VCA.cv`) não quebra nada, mas desperdiça
  metade do curso da modulação em valores negativos que o destino vai
  tratar como "zero" na prática.

## Os controles, um a um

**RATE** (0,01–12000 Hz) — a velocidade da rampa. De LFO lentíssimo a
oscilador de áudio. A CV `RATE` multiplica.

**SLOPE** (0–1) — a forma: 0 serra ↓, 0,5 triângulo, 1 serra ↑.

**DRIFT** (0–1) — passeio lento e correlacionado na taxa efetiva. Em 0,
determinístico. Um toque tira a rigidez de um LFO metrônomico.

**SYNC** (chave) — liga a entrada `SYNC`.

## Como cabear

**LFO no filtro (*wah* lento):**
```
FUNCTION (BI) → FILTER (FC)     RATE baixo, SLOPE ~0,5
```

**Envelope disparado (sem sustain):**
```
CLOCK (euclid) → FUNCTION (SYNC)      chave SYNC ligada, SLOPE = 0 (serra ↓)
FUNCTION (UNI) → VCA (CV)
```
Cada pulso reinicia a rampa, que decai — um AD simples.

**Vibrato:**
```
FUNCTION (BI) → OSC (FM)     RATE ~5 Hz, FM_AMOUNT baixo no OSC
```

## Potencializar

- **LFO que respira:** `DRIFT` ~0,3 — o LFO acelera e desacelera
  sozinho, nunca mecânico.
- **LFO travado no compasso mas com fase própria:** `SYNC` de um
  divisor do `CLOCK` (via `LOGIC`) — reinicia a cada 2 ou 4 compassos.
- **FM de LFO:** um `FUNCTION` A lento no `RATE` de um `FUNCTION` B — o
  segundo pulsa em ondas.
- **Como oscilador extra:** `RATE` na faixa de áudio, `BI` → `MIXER`
  (via `VCA`/`ENVELOPE`) — uma serra ou triângulo crua, de brinde.

## Se você conhece o Eurorack

Faz o papel de Mutable Tides / Stages, ou de um LFO/função (Batumi, Maths
como função). A ideia: **envelope, LFO e oscilador são a mesma rampa** —
não há botão de modo, só a taxa. Para formas compostas de vários
segmentos, veja o `STAGES` (#54).
