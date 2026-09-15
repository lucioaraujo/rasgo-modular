# LPG — low-pass gate

**Família:** TRANSFORM · **Módulo 25**
**Essência:** um filtro e um VCA controlados juntos por um envelope de
vactrol — rápido no ataque, com a cauda se arrastando. O *pluck* da
costa oeste.
**Dossiê técnico:** [`../dossies/25_lpg.md`](../dossies/25_lpg.md)
· **Fonte:** `src/dsp/Lpg.hpp`

---

## A ideia

O Rasgo tem `FILTER`, `VCA` e `ENVELOPE`, mas montar um LPG com eles dá
um resultado *quase* certo e sem a alma: falta a **curva do vactrol**,
que não é um AD reto — é rápido-devagar, com a cauda se arrastando e
freando perto de zero. É o som Buchla, o *pluck* de marimba / kalimba
eletrônica.

## Por dentro

**O que é um vactrol, pra quem nunca ouviu o termo:** é um componente
que junta uma lâmpada/LED com um resistor sensível à luz (LDR) num
mesmo invólucro — a lâmpada acende (controlada eletricamente), a luz
bate no resistor, e a **resistência** do resistor muda conforme a luz.
A parte interessante, fisicamente: o material do resistor **reage
rápido** quando a luz aumenta, mas tem uma espécie de "memória" que o
faz **relaxar devagar** quando a luz diminui — subir é rápido, descer é
lento e vai freando perto do fim, não é uma reta. É exatamente essa
assimetria física, não desenhada por escolha de circuito mas uma
propriedade do material, que dá ao vactrol seu caráter — e é isso que
o `LPG` modela: sobe em ~2 ms sempre (rápido, como a luz acendendo);
desce devagar e freia perto de 0 (como o resistor relaxando).

**Por que controlar filtro E volume juntos com a mesma curva, em vez de
só um dos dois:** um objeto físico batido (uma marimba, uma kalimba)
não só fica **mais quieto** conforme o som decai — também fica **mais
escuro** (perde brilho) ao mesmo tempo, porque a energia nas frequências
altas se dissipa mais rápido que nas baixas (a mesma física do `DAMP`
do `MATTER`/`RESONATOR`). Um envelope comum só no volume (como um
`VCA` sozinho) não captura esse escurecimento; o `LPG` aplica a
**mesma** curva de vactrol simultaneamente a um filtro de 2 polos e a
um VCA, e o resultado imita esse comportamento acústico duplo.
`MODE` decide a proporção: 0 usa só o filtro (o som nunca silencia de
verdade, só escurece); 1 usa só o VCA (escurece nada, só silencia); 0,5
usa os dois plenamente — o LPG "clássico", onde o som fecha e escurece
junto.

`BOUNCE` soma uma senoide amortecida logo depois do golpe — um pequeno
**repique mecânico** (o vactrol de verdade, ao ligar/desligar
rapidamente, pode oscilar um pouco antes de assentar) que acrescenta
um "clique" característico de madeira/percussão logo no início do
envelope.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a "gatear". **Plugue aqui:** `OSC` (linha
  de sinos), `NOISE` (percussão de ruído filtrado), `MATTER`/`STRING`
  (dá o amortecimento do golpe que o modal não tem).
- **`STRK`** (controle, disparo) — o **golpe**: abre o envelope na borda
  de subida. **Plugue aqui:** `CLOCK.euclid`, `TRIGSEQ.t1`,
  `SEQUENCE.eos`, `DECISION.gate`.
- **`CV`** (controle) — abre o envelope proporcionalmente (em vez de um
  golpe seco). **Plugue aqui:** `ENVELOPE.env`, um LFO — o LPG "respira"
  em vez de *plucar*.

### Saída

- **`OUT`** (áudio) — áudio × envelope de vactrol. Vai ao `MIXER`.

## Os controles, um a um

**MODE** (0–1) — crossfade filtro ↔ VCA. 0 = só filtro; 1 = só VCA;
0,5 = os dois (o LPG clássico).

**RESPONSE** (0–1) — o tempo da cauda pós-golpe (~30 ms a 2,5 s) — a
"memória" do LDR. A subida é sempre rápida; só a descida muda.

**OFFSET** (0–1) — abertura de repouso: quanto o LPG fica aberto mesmo
sem golpe. 0 = fecha de vez; alto = deixa passar um fio de som sempre.

**RESONANCE** (0–1) — a ressonância do filtro de 2 polos.

**BOUNCE** (0–1) — *overshoot* pós-golpe: uma senoide amortecida soma ao
envelope logo depois do `STRK` — o "repique" do vactrol.

**DRIFT** (0–1) — passeio lento e correlacionado na cauda de resposta.

## Como cabear

**Linha de sinos:**
```
SEQUENCE → QUANTIZER → OSC (1V/O)
OSC (TRI) → LPG (IN) → MIXER (ch1)
CLOCK (euclid) → LPG (STRK)
```
`MODE` ~0,5, `RESPONSE` médio, `RESONANCE` baixo.

**Percussão de ruído:** `NOISE.pink → LPG (IN)`, `RESPONSE` curto,
`RESONANCE` médio — um "tick" com corpo.

## Potencializar

- **Marimba:** `RESPONSE` curto + `BOUNCE` médio + `OSC.sine` no `IN` —
  o repique dá o ataque de madeira.
- **Cauda que varia:** `NOISE.smooth` no `CV` além do `STRK` — cada
  golpe decai um tempo diferente.
- **Drone que pulsa:** `OFFSET` alto + um LFO lento no `CV` — o LPG
  nunca fecha, só modula a abertura.
- **Cruze com o `MATTER`:** `MATTER → LPG` — o modal não tem
  amortecimento de golpe; o LPG dá.

## Se você conhece o Eurorack

Faz o papel do Buchla 292 / série 200 LPG, do Make Noise Optomix
(*crossfade* filtro↔VCA) e do modo LPG do Mannequins Three Sisters. A
diferença: o `BOUNCE` (overshoot de vactrol) e a resposta assimétrica
modelada de uma fotocélula.
