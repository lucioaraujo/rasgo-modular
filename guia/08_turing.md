# TURING — registrador de deslocamento

**Família:** TIME · **Módulo 8**
**Essência:** uma sequência que **emerge do acaso e depois se
solidifica**. Você não escreve a melodia — abre o `LOCK`, deixa o acaso
preencher, e fecha o `LOCK` quando algo soa bem.
**Dossiê técnico:** [`../dossies/08_turing.md`](../dossies/08_turing.md)
· **Fonte:** `src/dsp/TuringLoop.hpp`

---

## A ideia

Composição por escuta e seleção, não por programação. É o oposto do
*piano-roll*: um registrador de deslocamento de `LENGTH` estágios roda a
cada clock; `LOCK` é a probabilidade de o laço se **preservar** em vez de
mudar. Em `LOCK` = 0, acaso puro; em `LOCK` = 1, o laço trava e repete
para sempre. Você mexe o `LOCK` até um trecho soar bem e o congela.

## Por dentro

**O que é um "registrador de deslocamento", pra quem nunca ouviu o
termo:** é uma fileira de `LENGTH` caixinhas, cada uma guardando um bit
(0 ou 1). A cada clock, todo mundo **empurra pra frente** um lugar — o
bit que sai pela ponta some, e um bit **novo** entra pela outra ponta.
Se o bit novo fosse sempre aleatório, isso seria só ruído binário. O
truque do `LOCK`: com probabilidade `1 − LOCK`, o bit novo é de fato
sorteado (ou uma versão só levemente alterada do bit que **estava
saindo** — `MUTATE` decide o quanto); com probabilidade `LOCK`, o bit
que sai **reentra igual**, como se o registrador fosse fechado num
círculo. Em `LOCK`=1, todo bit reentra sempre igual — depois de
`LENGTH` clocks a sequência já deu a volta completa e passa a
**repetir para sempre**, exatamente. Em `LOCK`=0, todo bit é sempre
novo — nunca fecha o círculo, acaso puro sem memória.

A frente do registrador (os últimos `LENGTH` bits, lidos como um
número) vira a CV de saída, escalada por `RANGE`/`OFST` e opcionalmente
quantizada em degraus (`STEPS`). **`CV2`** lê o **mesmo** registrador,
mas soma **pesos diferentes** em 4 dos seus estágios — a mesma ideia
do "campo lido com pesos diferentes" do `DRIFT` (#27, ver "Por
dentro" lá): as duas CVs vêm da mesma memória/acaso subjacente, então
se movem **em relação**, mas nunca são idênticas. `PLS` simplesmente lê
o bit da frente como 0/1 puro (gate), sem escala nem quantização — o
mesmo acaso/memória virando ritmo em vez de altura.

## Os jacks, um a um

### Entradas

- **`CLK`** (clock) (controle, disparo) — avança um passo do
  registrador. Presente, substitui o `RATE` interno. **Plugue aqui:**
  `CLOCK.clock`, `LOGIC.div`, `SEQUENCE.eos`.
- **`LOCK`** (lock_mod) (controle) — CV que soma ao knob `LOCK`.
  **Plugue aqui:** um `DECISION.x` lento (o instrumento alterna sozinho
  entre "improvisar" e "repetir"), um `FUNCTION` bem lento.

### Saídas

- **`CV`** (áudio — CV de nota) — a frente do registrador, bipolar,
  escalada e quantizada. **Plugue em:** `QUANTIZER.cv → OSC.1V/O` (a
  melodia numa escala), `FILTER.cutoff` (o timbre segue o mesmo acaso).
- **`CV2`** (áudio — CV) — uma segunda CV, correlacionada com `CV` mas
  distinta. **Plugue em:** um segundo destino — `SPACE.mix`, a altura de
  uma segunda voz.
- **`PLS`** (pulse) (controle, gate) — o estágio da frente como bit
  (0/1). **Plugue em:** `ENVELOPE.gate`, `DRUM.GATE` — um ritmo derivado
  do mesmo acaso/memória.

## Os controles, um a um

**RATE** (0,01–50 Hz) — o relógio interno. Usado só se `CLK` estiver
livre.

**LEN** (length, 2–16) — o nº de estágios do registrador — o tamanho do
laço que pode ficar travado.

**LOCK** (0–1) — a probabilidade de o laço se preservar. 0 = acaso puro;
1 = travado (repete para sempre). O knob central do módulo.

**MUT** (mutate, 0–1) — quando o laço muda, o quanto: perto de 0 é
mutação pequena (ruído sobre o valor antigo); 1 é sorteio novo.

**RANGE** (0–1) — o alcance da CV de saída, em torno do `OFST`.

**STEPS** (1–32) — quantiza a CV em degraus. 1 = contínuo.

**OFST** (offset, −1..1) — o centro da CV de saída.

## Como cabear

**Melodia que você seleciona:**
```
CLOCK (clock) → TURING (CLK)
TURING (CV) → QUANTIZER (cv) → OSC (1V/O)
TURING (PLS) → ENVELOPE (gate)
```
Toque com `LOCK` em ~0,5, ouça, e feche o `LOCK` quando o loop agradar.

**Duas vozes correlacionadas:**
```
TURING (CV)  → OSC #1 (via QUANTIZER)
TURING (CV2) → OSC #2 (via QUANTIZER)     (contraponto que "combina")
```

## Potencializar

- **Improvisa e repete sozinho:** `DECISION.x` (bem lento) no `LOCK` — o
  instrumento alterna entre vagar e travar por conta própria.
- **Loop que muta devagar:** `LOCK` ~0,9, `MUTATE` baixo — o loop
  "quase" repete, com pequenas variações que se acumulam.
- **Ritmo e melodia do mesmo acaso:** `CV` na altura, `PLS` no gate — a
  linha rítmica e a melódica "conversam" porque vêm do mesmo
  registrador.
- **Par com o `SEQUENCE`:** `SEQUENCE` toca a frase escrita, o `TURING`
  improvisa entre elas — `SEQUENCE.eos → TURING.CLK` alterna.

## Se você conhece o Eurorack

É o Music Thing Turing Machine (com os expansores Volts em `CV2` e
Pulses em `PLS`). Distinto do `SEQUENCE` (frase escrita, relida de
5 modos) e do `DECISION` (memória como tabela relida por probabilidade).
