# TURNTABLE — toca-discos com massa

**Família:** SPACE · **Módulo 50**
**Essência:** o mesmo buffer do `SAMPLER`, lido por um prato com
**inércia** — o scratch, o backspin, o *power-off* que arrasta a
afinação, o beatmatch por empurrãozinho, o *tape stop*.
**Dossiê técnico:** [`../dossies/50_turntable.md`](../dossies/50_turntable.md)
· **Fonte:** `src/dsp/Turntable.hpp`

---

## A ideia

Tudo que vive do gesto de mão num prato não sai de `MEMORY`/`LOOPER`/
`SAMPLER`, que leem a posição de forma "digital" (salto, grão, incremento
fixo). O `TURNTABLE` põe a **inércia** no patch: a posição de leitura é a
integral de uma velocidade angular com massa. `LFO → SCRATCH` faz o disco
scratchear no compasso; `ENVELOPE → BRAKE` faz um *tape stop* na virada;
`SEQUENCE → SCRATCH` toca uma frase de scratch rítmica.

**Beatmatch é gesto — não há quantização de BPM.** É onde o Rasgo
diverge da abordagem do Navalha 2, de propósito.

## Por dentro

**Por que "a posição é a integral de uma velocidade", e não um salto
direto:** no `SAMPLER`, mudar de posição de leitura é instantâneo —
um número muda, pronto. Um prato de verdade **não pode** fazer isso: um
disco físico tem massa, e mudar sua velocidade de rotação exige
**tempo** (a força do motor tem que vencer a inércia do próprio disco
girando). Modelar isso significa que a posição de leitura, a cada
instante, não é um valor "escolhido" — é o resultado de **acumular**
(integrar) a velocidade real do prato até agora, e essa velocidade real
**também** muda gradualmente em direção a um alvo, nunca de uma vez.
`SPEED` é só esse **alvo**; `TORQUE` decide o quão rápido a velocidade
de verdade consegue se aproximar dele (baixo = o motor é fraco, demora
pra "pegar" — o *wow* de partida de um prato ligando).

**`GRAB`, o que significa "a mão agir sobre a física":** a CV `SCR`
representa uma mão empurrando o disco — ela não define a velocidade
diretamente, ela **soma uma força** ao sistema físico, e `GRAB` decide
**quão forte** essa força é comparada à força do próprio motor.
`GRAB` baixo: a mão só consegue desviar levemente a velocidade real do
alvo do motor — um empurrãozinho, o efeito é um *pitch-bend* sutil
(beatmatch). `GRAB` alto: a mão consegue **dominar** completamente o
que o motor está tentando fazer, inclusive jogar o disco pra trás — o
scratch de verdade, onde a mão manda mais que o motor. Como é uma
força somada, não uma substituição, a mão **sempre** pode agir, mesmo
com `TORQUE`=0 (motor essencialmente desligado) — é o "vinil sem
motor", onde só a mão move o prato.

`FRICTION` decide **duas** coisas com a mesma física: quão rápido o
prato desacelera quando o freio (`BRK`) é acionado, e quão rápido ele
**retorna** à velocidade do motor depois que a mão solta o disco (num
prato real, o atrito e o motor "puxam" o disco de volta ao normal
depois de um scratch).

**Por que a saída tem acoplamento AC:** se o prato para completamente
(velocidade zero), a leitura fica travada num único valor de amostra
constante — sem tratamento, isso deixaria um **nível DC** parado na
saída (não silêncio de verdade, um "degrau" constante que pode estalar
quando o prato volta a girar). O acoplamento AC deixa esse valor
parado **decair suavemente a zero** (~40 ms) em vez de ficar preso,
então um prato parado realmente vira silêncio, sem clique na retomada.

## Os jacks, um a um

### Entradas

- **`TRIG`** (controle, disparo) — põe a agulha (`readPos = START ×
  comprimento`, rampa de ~5 ms) e **liga o motor**. **Plugue aqui:**
  `CLOCK`, `SEQUENCE.eos`, um gate.
- **`IN`** (áudio) — o que gravar (enquanto `REC` estiver alto).
  **Plugue aqui:** `SIGNAL-IN.L`, a soma do `MIXER`, uma voz.
- **`REC`** (controle, gate) — enquanto alto, grava `IN` no buffer
  (~8 s).
- **`SCR`** (scratch) (controle) — a mão no disco. Pequeno = pitch-bend;
  grande e oscilando = scratch. **Plugue aqui:** um LFO (o disco
  scratcheia no compasso, sem `TRIG`), um `SEQUENCE` de CV (frase de
  scratch), `NOISE.smooth`.
- **`BRK`** (brake) (controle, gate) — enquanto alto, o freio: o prato
  desacelera (com `FRICTION`). **Plugue aqui:** um `ENVELOPE` (tape stop
  na virada), `CLOCK.euclid`.

### Saída

- **`OUT`** (áudio) — o buffer lido pelo prato. Acoplamento AC na saída
  (prato parado → a amostra congelada some em ~40 ms, sem degrau de DC).
  Vai ao `MIXER`.

## Os controles, um a um

**SPEED** (−1..1) — a velocidade **alvo**: ±0,5× a ±2× (33⅓ ↔ 45 ↔
lento). Negativo = disco pra trás.

**TORQ** (torque, 0–1) — a força do motor: quão rápido o prato **atinge**
a velocidade. Baixo = *wow* longo de partida (~1 s). No mínimo, o motor
mal puxa — só a mão (`SCR`) move o prato ("vinil sem motor").

**FRIC** (friction, 0–1) — quão rápido o prato para no `BRAKE` e quanto
ele "volta" sozinho depois de um scratch.

**GRAB** (grab, 0–1) — a firmeza da mão: quanto a CV `SCR` joga o prato.

**START** (0–1) — onde a agulha cai no `TRIG`.

**WEAR** (0–1) — desgaste do vinil: estalos que crescem + micro-
instabilidade de rotação. Determinístico (semeado).

**LOOP** (chave) — one-shot (o disco "acaba" e trava) ↔ groove travado
(a leitura dá a volta no buffer).

## Como cabear

**Scratch no compasso:**
```
SIGNAL-IN (L) → TURNTABLE (IN)
CLOCK (de 1 compasso) → TURNTABLE (REC)
LFO (síncrono ao clock) → TURNTABLE (SCR)     GRAB médio
TURNTABLE (OUT) → MIXER (ch1)
```

**Tape stop na virada:**
```
ENVELOPE (env) → TURNTABLE (BRK)     FRICTION médio
CLOCK (a cada 8 compassos) → ENVELOPE (gate)
```

## Potencializar

- **Frase de scratch:** `SEQUENCE.pitch → SCR` — uma linha rítmica de
  empurrões (baby scratch, transformer).
- **Partida analógica:** `TORQ` baixo + `TRIG` a cada seção — o
  *pitch-up* de motor pegando.
- **Backspin:** um `ENVELOPE` bem rápido, invertido, no `SCR` — o disco
  volta e re-dispara.
- **Vinil sem motor:** `TORQ` = 0, só um LFO no `SCR` — o disco só se
  move quando a "mão" empurra.

## Se você conhece o Eurorack

Não há equivalente comum — é o modelo físico de um Technics SL-1200
(motor/torque/rampa, nenhum circuito). A técnica de scratch de DJ é
documentação pública. Compartilha o buffer e a camada de I/O com o
`SAMPLER` (#48). Distinto dele: leitura contínua com inércia, não salto
de fatia; sem quantização de BPM.
