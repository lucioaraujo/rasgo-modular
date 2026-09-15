# STRING — corda por guia-de-onda

**Família:** SOURCE · **Módulo 11**
**Essência:** uma corda tocada — pinçada, martelada ou arcada — modelada
por um laço de atraso (Karplus-Strong estendido). Ataque e corpo que a
síntese modal não dá naturalmente.
**Dossiê técnico:** [`../dossies/11_string.md`](../dossies/11_string.md)
· **Fonte:** `src/dsp/StringVoice.hpp`

---

## A ideia

O `MATTER` faz o corpo somando modos — ótimo pra sinos, placas, tigelas.
Corda pinçada e corda arcada são outro território: o do **guia-de-onda**.
É mais barato (um laço, não 24 ressoadores), tem um ataque e um "corpo"
que a soma de modos não captura de graça, e o gesto de `position` —
onde a corda é pinçada — é físico e imediato.

Ter os dois modelos cobre quase todo o mundo dos objetos que soam: o
`MATTER` pra o que ressoa, o `STRING` pra o que tem corda.

## Por dentro

**O que é um "guia-de-onda" / Karplus-Strong, pra quem nunca ouviu o
termo:** em vez de somar modos como o `MATTER`, esta técnica simula
uma corda com um **laço de atraso** — um buffer que guarda um pedaço de
áudio e o **repete em círculo**, um pouco alterado a cada volta. O
comprimento desse laço (quantas amostras cabem nele) decide o **tempo
de uma volta completa** — e o tempo de uma volta completa **é** o
período da nota (o inverso da frequência): um laço curto dá voltas
rápidas (nota aguda), um laço longo dá voltas devagar (nota grave). É
por isso que `FREQ` controla o comprimento do laço, não uma forma de
onda.

**Como o ruído vira nota afinada:** você injeta uma rajada de ruído no
laço (o "golpe" do `PLK`) — o ruído em si não tem altura nenhuma. Mas a
cada volta, o mesmo trecho de áudio passa de novo pelo mesmo filtro que
tira um pouco do agudo (`damping`) e perde um pouco de energia
(`decay`) — as frequências que **não combinam** com o tamanho do laço
se cancelam sozinhas ao longo de várias voltas (elas "batem fora de
fase" com a volta anterior), enquanto a frequência que **combina**
exatamente com o comprimento do laço se reforça a cada passagem. Em
poucos milissegundos, o que começou como ruído puro já convergiu numa
onda periódica, afinada — e vai perdendo energia (`decay`) e brilho
(`damping`) enquanto continua circulando, exatamente como uma corda
real pinçada.

`position` filtra a excitação **antes** dela entrar no laço, do mesmo
jeito físico descrito no `MATTER` (#9, ver "Por dentro" lá): pinçar
perto de um nó de um harmônico específico não o excita, então aquele
harmônico simplesmente não aparece no som.

**Por que "arcar" (excitação contínua em `IN`) não diverge:** uma corda
de verdade, tocada com arco, recebe energia **o tempo todo**, não só
num golpe — e ainda assim não sai tocando cada vez mais alto pra sempre,
porque a fricção do arco também **tira** energia proporcional à
amplitude. O `tanh` no laço (controlado por `DRIVE`) faz o mesmo papel
aqui: quanto mais forte o laço fica, mais essa curva **acha suavemente**
o excesso (a mesma ideia do §1 de `RELACAO_DE_CABO.md`) — o sistema
converge pra um **ciclo-limite** (um nível estável de oscilação
sustentada) em vez de crescer sem parar. É o mesmo princípio de
estabilidade que deixa o `FILTER`/`WASP` auto-oscilarem sem estourar
perto de `resonance`=1.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — excitação **contínua**: em vez de um golpe, um sopro
  de ar. **Plugue aqui:** um LFO, `NOISE`, um sinal de baixo nível —
  ele "arca" a corda, que passa a soar sustentada em vez de decair. O
  `tanh` no laço segura o volume.
- **`PLK`** (controle, disparo) — pinça a corda (uma rajada de ruído
  filtrada pela `POS`). **Plugue aqui:** `CLOCK.euclid`, `TRIGSEQ`,
  `DECISION.gate`, `SEQUENCE.eos`. É a forma normal de tocar notas.
- **`1V/O`** (controle, altura) — multiplica `FREQ` (o comprimento do
  laço). **Plugue aqui:** `QUANTIZER.pitch`, `SEQUENCE.pitch`,
  `TURING.cv`.
- **`DMP`** (controle) — soma ao knob `DAMP` (damping). **Plugue aqui:**
  um `ENVELOPE` (a corda escurece na cauda), um LFO lento.

### Saída

- **`OUT`** (áudio) — a corda (seco + ressonância, dosado por `MIX`).
  Vai ao `MIXER`, geralmente via um `SPACE` — uma corda seca soa
  pequena.

## Os controles, um a um

**FREQ** (20–4000 Hz) — a afinação da corda (o comprimento do laço).

**DECAY** (0–1) — o *sustain*: quanto tempo a corda segue soando depois
de pinçada. Baixo = *staccato* seco; alto = quase infinito.

**DAMP** (damping, 0–1) — o brilho. Alto deixa a corda escura, apagando
os harmônicos agudos mais rápido (uma corda grave, encordoamento velho);
baixo mantém o brilho (corda fina, nova).

**POS** (position, 0,02–0,5) — onde a corda é pinçada. Varra: em certos
pontos alguns harmônicos somem — a física real. Perto de 0,5, sem
harmônicos pares (som de "clavinet").

**EXCIT** (exciter, 0–1) — quanto ruído entra no golpe (`PLK`). Dose do
"dedo" no ataque.

**DRIVE** (0–1) — satura o laço da corda. Mais grão, menos limpo — de
corda acústica a corda distorcida.

**MIX** (0–1) — seco ↔ ressonância. Em 1 (padrão) você ouve só a corda.

## Como cabear

**Autônomo** — `PLK` de um `CLOCK` euclidiano, `POS` de um `TURING`:
um baixo pinçado que muda de timbre a cada nota.

**Performance** — `FREQ` e `POS` na mão; `DECAY` alternando *staccato* e
sustentado.

**Híbrido** — voz melódica com espaço:

```
SEQUENCE → QUANTIZER → STRING (1V/O)
CLOCK (euclid) → STRING (PLK)
STRING (OUT) → SPACE (in) → MIXER (ch1)
ENVELOPE (env) → STRING (DMP)         (a corda escurece na cauda de cada nota)
```

Arcada: um LFO devagar no `IN`, `DECAY` alto, `PLK` desligado — uma nota
contínua que "canta".

## Potencializar

- **Do dedo ao arco:** comece pinçando (`PLK`), depois mande um
  `NOISE.brown` de nível baixo no `IN` e desligue o `PLK` — a corda
  passa a soar continuamente, como um arco.
- **Glissando:** um `FUNCTION` bem lento no `1V/O` (ou `GLIDE` entre as
  notas do `SEQUENCE`) — a corda desliza.
- **Corda quebrada:** suba `DRIVE` e mande um `ENVELOPE` no `DAMP` — o
  ataque fica sujo e a cauda limpa.
- **Cruze com o `RESONATOR`:** mande a `STRING` no `RESONATOR.in` — a
  corda excita um banco de modos afinado, e você tem corda *dentro* de
  um sino.

## Se você conhece o Eurorack

É Karplus-Strong estendido — Mutable Rings tem esse modo, e há vários
módulos de corda física (Jaffe & Smith, J.O. Smith). A diferença do
Rasgo: o `tanh` no laço torna o modo arco (`IN` contínuo) **estável por
construção** — ele vira ciclo-limite, não estoura. E `position` é o gesto
físico num knob. O parente é o `MATTER` (#9), a mesma família de vozes
tocadas, pelo caminho modal.
