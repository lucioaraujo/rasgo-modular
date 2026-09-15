# DRIFT — campo de deriva

**Família:** MODULATE · **Módulo 27**
**Essência:** uma fonte de CV que se move em escala de **minutos**, com
memória e correlação. É o que faz um patch de seed **evoluir sozinho**.
**Dossiê técnico:** [`../dossies/27_drift.md`](../dossies/27_drift.md)
· **Fonte:** `src/dsp/Drift.hpp`

---

## A ideia

Um patch generativo bom, sem gesto humano, ainda "anda em círculos"
depois de um tempo: o `CLOCK` tica igual, o `FILTER` fica no mesmo corte,
a `HARMONY` roda mas o resto é estático. Falta o **desenvolvimento
lento** — o instrumento se comportar diferente aos 30 s e aos 4 min, sem
repetir nem saltar.

O `DRIFT` é essa camada. Não é um LFO (rápido demais) nem um S&H (salta).
É um passeio contínuo com **momentum** (a velocidade tende a continuar) e
**correlação** (as 4 saídas contam a mesma história de ângulos
diferentes). Plugue `a`/`b`/`c` em três parâmetros estruturais e a peça
se transforma sozinha.

## Por dentro

**O que é o "campo", e por que as 4 saídas são correlacionadas:** por
dentro existe **um único** valor passeando devagar — o "campo" — não
quatro passeios independentes. As saídas `A`–`D` são quatro **leituras**
desse mesmo campo, cada uma multiplicada por um peso próprio. Como
todas vêm da mesma fonte, elas sempre contam "a mesma história" — se o
campo sobe, as quatro tendem a subir junto, só que em proporções
diferentes (algumas mais, algumas menos, dependendo do peso) — daí
"correlacionadas": relacionadas entre si, não coincidências
independentes. `STRIDE` controla **quão diferentes** são esses pesos:
em 0, os pesos são quase iguais (as 4 saídas quase se sobrepõem); em 1,
bem diferentes (cada saída conta uma versão bem própria da mesma
história).

**O que "momentum" significa aqui, fisicamente:** um objeto pesado, uma
vez em movimento, tende a **continuar** naquela direção — não muda de
curso instantaneamente. `MOMENTUM` alto faz o campo se comportar assim:
a velocidade do passeio muda devagar, então a trajetória fica **suave**
e previsível por um tempo (sobe, continua subindo um pouco mais antes
de eventualmente virar). `MOMENTUM` baixo é o oposto — a velocidade
reage imediatamente a cada novo empurrão aleatório, então o campo muda
de direção com muito mais frequência, "nervoso".

**Como `ANCHOR` (âncora) evita que a deriva vague pra sempre sem
padrão:** a cada 8 tiques do passeio, o módulo **grava** onde o campo
está como um "marco" (uma paisagem já visitada). Depois, a cada passo
seguinte, existe uma chance de o campo ser puxado de volta pra **perto
de um desses marcos** em vez de continuar se afastando livremente —
e essa chance cresce **com o quadrado** de `ANCHOR` (dobrar `ANCHOR`
quadruplica a chance de retorno, não só dobra) — então em valores
baixos o efeito é quase imperceptível, e só perto do topo a deriva
realmente passa a "orbitar" territórios conhecidos. Musicalmente, isso
dá uma sensação de **retorno/recorrência** sem nunca repetir
exatamente — como um tema que reaparece transformado, não um loop.

## Os jacks, um a um

### Entradas

- **`ADV`** (controle, disparo) — força um passo do campo. **Plugue
  aqui:** `CLOCK` (via um divisor no `LOGIC`), `SEQUENCE.eos` — a
  cadência da deriva passa a ser por compasso/frase, não pelo `RATE`
  interno.
- **`RATE`** (controle) — CV que soma ao knob `RATE`.

### Saídas (todas controle)

- **`A`** / **`B`** / **`C`** / **`D`** — quatro leituras
  correlacionadas do campo, cada uma com um peso próprio. **Plugue em:**
  `FILTER.cutoff`, `SPACE.feedback`, `OSC.pw`, `MATTER.structure`,
  `QUANTIZER.range`, `CHORD.detune`… — parâmetros **estruturais**, não
  a altura das notas.
- **`FLD`** (field) — o passeio cru compartilhado, sem os pesos por
  saída. Use quando quiser uma modulação e as saídas A–D em outros
  destinos ao mesmo tempo.
- **`EVT`** (event) — um pulso a cada tique do passeio. **Plugue em:**
  um `TRIGSEQ.fill`, um `DECISION` — eventos ritmados pela deriva.

## Os controles, um a um

**RATE** (0,002–1 Hz) — o ritmo do passeio. Escala de minutos.

**DEPTH** (0–1) — o alcance: quanto o campo pode se afastar do repouso.

**MOMT** (momentum, 0–1) — quanto a velocidade tende a continuar. Alto =
trajetória suave, menos nervosa; baixo = muda de direção o tempo todo.

**STRD** (stride, 0–1) — 0 = as 4 saídas andam quase juntas; 1 = cada
uma pro seu lado.

**BIAS** (−1..1) — empurra o repouso do campo pra um lado (não centrado
em 0).

**ANCHR** (anchor, 0–1) — memória de topologia. 0 = passeio livre
(nunca volta); alto = a deriva orbita marcos já visitados.

## Como cabear

**A peça que evolui sozinha (o uso central):**
```
DRIFT (A) → FILTER (FC)          (o timbre muda ao longo de minutos)
DRIFT (B) → SPACE (FBK)          (a sala abre e fecha)
DRIFT (C) → CHORD (via CONTROL → CHRD)   (o acorde muda de formato)
```
`STRIDE` ~0,5, `MOMENTUM` alto, `DEPTH` a gosto.

**Deriva por frase:**
```
SEQUENCE (eos) → DRIFT (ADV)     (um passo do campo a cada fim de padrão)
```

## Potencializar

- **Forma com marcos:** `ANCHOR` ~0,6 — a peça "visita" e "revisita"
  estados, dando uma sensação de retorno sem loop exato.
- **Dois DRIFT em escalas diferentes:** um bem lento (minutos, na
  estrutura) e um menos lento (dezenas de segundos, no timbre).
- **`EVT` como semeador de eventos:** `EVT → DECISION.trig` — decisões
  que acontecem na cadência da deriva.
- **Combine com o `drift` dos módulos:** o knob `drift` de cada voz dá o
  tremor de segundos; o módulo `DRIFT` dá o desenvolvimento de minutos.
  Os dois juntos = vivo em duas escalas.

## Se você conhece o Eurorack

Faz o papel de um multi-LFO orgânico (Batumi, DivKid ochd, Wogglebug),
mas em escala **muito** mais lenta e com **memória**. A base é o
`deriveFromMemory` do ANTITOTEM e o `BiomaBrain` do AQUORBIUM (código do
autor). Distinto do `FUNCTION` (LFO — segundos), do `SH` (salta) e do
`CHAOS` (caótico, imprevisível).
