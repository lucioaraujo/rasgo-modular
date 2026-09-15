# LOOPER — delay de linha com fita

**Família:** SPACE · **Módulo 41**
**Essência:** os três gestos da fita — repetir uma frase para sempre
(hold), soltar ela ao contrário (reverse), o eco apodrecendo a cada
volta (age) — mais eco multi-cabeça.
**Dossiê técnico:** [`../dossies/41_looper.md`](../dossies/41_looper.md)
· **Fonte:** `src/dsp/Looper.hpp`

---

## A ideia

Metade da música com fita / *tape echo* vive de três gestos: **repetir**
(hold), **inverter** (reverse) e **o eco apodrecendo** (age). O delay
digital limpo é o caso `AGE` = 0; os outros três são o que dá vida.
Distinto do `SPACE` (reverb) e do `MEMORY` (granular): aqui é uma linha
de atraso, com ecos audíveis.

## Por dentro

Um buffer circular de ~2,2 s: o áudio é escrito continuamente e lido de
volta de um ponto `TIME` atrás — a mesma ideia do multitap do `SPACE`
(#10), só que aqui geralmente com **uma** distância, pensada como eco
de fita, não como padrão de várias tomadas.

**Wow & flutter, o que são de verdade:** numa fita de verdade, o motor
que puxa a fita nunca gira em velocidade **perfeitamente** constante —
existem duas escalas de instabilidade mecânica: uma bem lenta (o
"wow", perto de 1 Hz — a fita acelera e desacelera devagar, tipo uma
onda) e outra mais rápida (o "flutter", perto de 6–7 Hz — um tremor
mais fino, de vibração mecânica). `AGE` recria isso literalmente com
**dois LFOs** nessas duas frequências específicas, modulando
sutilmente a velocidade de leitura — é essa combinação exata de duas
escalas de tremor, não uma modulação genérica qualquer, que faz o
ouvido reconhecer "fita" em vez de "chorus digital". Junto com isso,
`AGE` escurece o laço (perda de agudo a cada volta — a mesma física de
absorção do `HALL`), satura suavemente, e soma um chiado semeado
(reprodutível).

**`HOLD`, o que muda estruturalmente:** o buffer **para de receber**
qualquer coisa nova do `IN`, mas continua sendo **lido** em loop, na
mesma janela onde estava — o conteúdo capturado se repete
indefinidamente, sem nenhuma realimentação adicionando mais camadas
(diferente de deixar `FEEDBACK` alto, que continua acumulando o que
entra). É uma "fotografia sonora" congelada num loop, não uma cauda
crescendo.

**`REVERSE`, por que não estala:** ler um buffer de trás pra frente, se
feito de forma ingênua, criaria um salto abrupto exatamente no
instante da troca de direção (o mesmo problema de "quebra abrupta" do
`OSC`/`SWITCH`). O módulo evita isso com um **crossfade** suave (janela
de Hann — uma curva em formato de sino que sobe e desce suavemente)
entre a leitura antiga e a nova, então a troca de direção nunca produz
um clique perceptível.

`HEADS` (1–4) adiciona cabeças de leitura extras, cada uma numa fração
diferente do `TIME` principal (0,75×, 0,5×, 0,25×) — a mesma ideia de
multitap do `SPACE`, aqui aplicada especificamente pra recriar o
comportamento de um eco de fita de múltiplas cabeças (Roland RE-201).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — a voz a ecoar/loopar. **Plugue aqui:** qualquer
  voz, `ENVELOPE.out`.
- **`TIME`** (controle) — CV que soma ao `TIME` (em segundos). **Plugue
  aqui:** um LFO lento (o laço "estica" e "encolhe" como fita puxada à
  mão), `SEQUENCE` de CV (tempos por passo).
- **`FRZ`** (freeze) (controle, gate) — enquanto alto, equivale a `HOLD`
  ligado. **Plugue aqui:** `CLOCK.euclid`, um pedal.
- **`REV`** (controle, gate) — enquanto alto, equivale a `REVERSE`
  ligado. **Plugue aqui:** `DECISION.gate`, `TRIGSEQ.t4`.

### Saídas (ambas áudio)

- **`OUT`** — seco + molhado, misturados por `MIX`. Vai ao `MIXER`.
- **`WET`** — só o laço, sem o seco. Útil pra rotear o eco separado.

## Os controles, um a um

**TIME** (0,001–2 s) — o comprimento do atraso: de eco curto a laço de
2 s.

**FBK** (feedback, 0–1,1) — quanto da saída volta pro laço. Acima de 1
auto-oscila. Passa pelo filtro de `AGE` e por um `tanh`.

**AGE** (0–1) — o caráter de fita/BBD: perda de agudo, wow & flutter,
saturação, chiado. `AGE` = 0 → delay digital limpo.

**MIX** (0–1) — seco ↔ molhado na `OUT`.

**HEADS** (1–4) — 1 = eco simples; 2–4 = eco de fita multi-cabeça (as
cabeças extras leem 0,75 / 0,5 / 0,25× do `TIME` e somam — eco denso e
rítmico).

**HOLD** (chave / gate `FRZ`) — congela o laço: para de escrever, repete
a janela atual para sempre, sem realimentação nova.

**REV** (reverse, chave / gate `REV`) — lê o laço de trás pra frente,
sem clique.

## Como cabear

**Tape echo:**
```
voz → LOOPER (IN) → MIXER (ch1)     TIME ~0,35 s, FBK ~0,5, AGE ~0,3
```

**Loop como base:**
```
voz → LOOPER (IN) → MIXER (ch1)
```
Toque uma frase, ligue `HOLD` no fim dela, e improvise por cima — o
trecho vira base.

**Frippertronics:** `FBK` perto de 1, `TIME` longo, `HEADS` 3–4 — o
material se acumula em camadas.

## Potencializar

- **Loop ao contrário fixo:** `REV` + `HOLD` juntos — um trecho fixo
  tocando de trás pra frente em loop.
- **Cópia de cópia:** `HOLD` ligado + suba o `AGE` — cada repetição fica
  mais escura e trêmula.
- **Eco rítmico:** `HEADS` 3, `TIME` = uma colcheia — as cabeças caem em
  subdivisões do compasso.
- **Auto-oscilação como voz:** `FBK` ~1,05 sem entrada — o laço se
  sustenta e o `AGE` vira o timbre da cauda; cabeie num canal do
  `MIXER`.

## Se você conhece o Eurorack

Faz o papel de um delay de linha com fita (Make Noise Mimeophon, 4ms
DLD, o RE-201 Space Echo pelo `HEADS`). A base é a teoria de BBD/fita
(companding, banda, wow) e o reverse granular de Roads. O `TAPE` do
Rasgo virou o `HEADS` daqui — não é módulo separado.
