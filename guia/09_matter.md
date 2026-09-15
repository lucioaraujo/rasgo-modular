# MATTER — corpo ressonante

**Família:** SOURCE · **Módulo 9**
**Essência:** um banco de 24 modos ressonantes — transforma qualquer
excitação (um clique, um gate, ruído, uma voz) no som de uma coisa:
corda, sino, placa, tubo.
**Dossiê técnico:** [`../dossies/09_matter.md`](../dossies/09_matter.md)
· **Fonte:** `src/dsp/Matter.hpp`

---

## A ideia

Um oscilador filtrado soa como eletrônica. Um objeto físico soa como
**algo sendo tocado** — porque ele tem muitos modos de vibração ao mesmo
tempo, em razões que dizem se é metal, madeira, vidro, e porque *onde*
você bate decide *quais* modos acordam.

O `MATTER` é esse objeto. Você não afina harmônicos — você escolhe a
**estrutura** (de corda a sino) e a **posição** do golpe, e excita:
com o disparo interno, com ruído, ou com uma voz de fora. Numa lógica
generativa, um `CLOCK` batendo no `MATTER` já é percussão afinada de
graça.

## Por dentro

**O que é um "modo", pra quem nunca ouviu o termo:** bata numa colher,
numa corda de violão, num copo — o som que sai **não é uma frequência
só**. É uma pilha de várias frequências soando ao mesmo tempo, cada uma
decaindo no seu próprio ritmo — cada uma dessas é um **modo de
vibração** do objeto: um jeito específico que ele consegue vibrar,
com sua própria frequência natural. Um sintetizador **modal** (como o
`MATTER`) não desenha uma forma de onda — ele simula uma **pilha de
osciladores amortecidos** (aqui, 24), cada um representando um modo, e
deixa você excitar essa pilha de fora, como bater no objeto de
verdade. É por isso que soa "físico": é literalmente o mesmo princípio
que faz objetos reais soarem como eles soam.

**Por que `structure` vai de "corda" a "sino":** a **razão** entre as
frequências dos modos é o que diferencia os materiais. Uma corda
esticada (violão, piano) tem modos em razões quase exatamente inteiras
— 1×, 2×, 3×, 4× a fundamental — porque é fina e flexível, então soa
**afinada**, com uma nota clara e harmônicos que reforçam essa nota.
Uma barra rígida, um sino, uma placa de metal são **rígidos demais**
pra vibrar assim tão limpo — a própria rigidez deforma essas razões pra
longe dos números inteiros (razões **inarmônicas**), e é exatamente
essa distorção que o ouvido reconhece como "metal"/"sino" em vez de
"corda". `structure` em 0 usa razões harmônicas (corda); subindo, as
razões vão esticando progressivamente pra longe do inteiro (sino).

**Por que `position` muda quais modos soam:** cada modo de vibração tem
pontos que **não se movem** (os *nós*) e pontos que se movem ao máximo
(os *ventres*) — é a física de uma onda estacionária. Se você excita o
objeto **exatamente** num nó de um certo modo, esse modo específico
**não recebe energia nenhuma** (você não conseguiu balançar o que já
estava parado ali) — mesmo que os outros modos, cujos nós ficam em
outro lugar, respondam normalmente. É a mesma razão pela qual tocar uma
corda de violão perto do centro soa "cheio" e perto do cavalete soa
"fino/nasal": a posição do toque decide quais harmônicos ficam fortes
ou desaparecem. `position` no `MATTER` é exatamente esse ponto de
excitação, num knob.

`damping` decide quanto tempo cada modo continua vibrando depois de
excitado antes de perder energia e morrer (um objeto "vivo"/ressonante
como um sino tem `damping` alto; um objeto "morto"/abafado como um
travesseiro tem `damping` baixo). `brightness` decide quantos dos modos
mais **agudos** da pilha realmente têm energia — baixo, só os modos
graves soam (som surdo); alto, a pilha inteira participa (som
brilhante, denso de parciais).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o que **excita** os modos. **Plugue aqui:** um
  clique/gate, `NOISE` (branco ou pink), uma voz, um `DRUM`. É o que
  faz o corpo soar; sem `IN` e sem `HIT`, o `MATTER` fica quieto.
- **`HIT`** (controle, disparo) — dispara uma **rajada curta de ruído
  interna** (um golpe). **Plugue aqui:** `CLOCK.euclid`,
  `TRIGSEQ.t1`, `DECISION.gate`, `SEQUENCE.eos`. É a forma mais direta
  de tocar o `MATTER` como percussão.
- **`1V/O`** (controle, altura) — multiplica `FREQ` (afina o corpo).
  **Plugue aqui:** `QUANTIZER.pitch`, `SEQUENCE.pitch`, `TURING.cv`.
- **`STR`** (controle) — soma ao knob `STRC` (structure). **Plugue
  aqui:** um `DECISION`/`TURING`/`NOISE.smooth` — o material muda a cada
  nota, de corda a sino.

### Saída

- **`OUT`** (áudio) — a mistura entre a excitação crua e a ressonância
  dos modos (dosada por `MIX`). Vai para o `MIXER`, ou para um `SPACE` /
  `FILTER` primeiro.

## Os controles, um a um

**FREQ** (20–8000 Hz) — a fundamental do banco de modos.

**STRC** (structure, 0–1) — **corda** (0, modos harmônicos, som afinado e
doce) → **sino / metal** (1, modos esticados, inarmônico, badalar). O
knob de "que material é isto".

**BRITE** (brightness, 0–1) — quantos modos agudos soam. Baixo = surdo,
abafado; alto = brilhante, muitos parciais.

**DAMP** (damping, 0–1) — quanto tempo os modos seguem soando depois de
excitados. Baixo = *pluck* curto; alto = anel longo, quase drone.

**POS** (position, 0,02–0,5) — onde o objeto é golpeado. Varra POS com um
golpe constante: alguns modos somem, outros aparecem — é a física de
tocar num ponto diferente. Em 0,25, os harmônicos pares somem.

**EXCIT** (exciter, 0–1) — quanto ruído entra na rajada do golpe interno
(`HIT`). Baixo = golpe "limpo"; alto = mais ar, mais ataque.

**MIX** (0–1) — excitação crua ↔ ressonância dos modos. Em 1 (padrão)
você ouve só o corpo.

## Como cabear

**Autônomo** — `HIT` de um `CLOCK`, `STRC` de um `TURING`: percussão
afinada que muda de material sozinha.

**Performance** — `STRC` e `POS` na mão ao vivo, `DAMP` decidindo entre
*pluck* e drone.

**Híbrido** — como voz melódica:

```
SEQUENCE → QUANTIZER → MATTER (1V/O)
CLOCK (euclid) → MATTER (HIT)
MATTER (OUT) → SPACE (in) → MIXER (ch1)
DECISION → MATTER (STR)             (o material varia a cada nota)
```

Ou como ressoador de fora: qualquer voz → `MATTER.IN` com `MIX` alto.

## Potencializar

- **De pluck a coral:** suba `DAMP` devagar durante a peça (ou um
  `DRIFT` nele) — as notas passam a se sobrepor e formam um acorde
  sustentado.
- **Material vivo:** um `NOISE.smooth` lento no `STR` — o corpo oscila
  entre madeira e metal ao longo de minutos.
- **Excitação de caráter:** em vez do golpe interno, mande um `DRUM` ou
  um `SAMPLER` no `IN` — o `MATTER` ressoa o transiente de outra fonte.
- **Cruze com o `RESONATOR`:** o `MATTER` é a voz fechada; o `RESONATOR`
  é o mesmo banco de modos aberto pra você bater de fora, com três
  saídas que se cruzam.

## Se você conhece o Eurorack

Faz o papel de **Mutable Rings / Elements** no modo de voz — síntese
modal / por modelagem física. A diferença: `structure` é um contínuo
único de corda a sino (não um seletor de modelos), e `position` é o
gesto físico exposto num knob. O parente aberto é o `RESONATOR` (#55),
que é o Rings/Elements no modo ressoador (você excita de fora, saídas
`low`/`mid`/`high`).
