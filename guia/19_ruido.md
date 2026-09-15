# NOISE — ruído e acaso

**Família:** SOURCE · **Módulo 19**
**Essência:** oito cores de ruído + sample-and-hold + tensão que passeia,
todas nos jacks ao mesmo tempo. A fonte de textura *e* a fonte de acaso
de todo patch generativo.
**Dossiê técnico:** [`../dossies/19_ruido.md`](../dossies/19_ruido.md)
· **Fonte:** `src/dsp/Noise.hpp`

---

## A ideia

Sem `NOISE` não há bumbo (o transiente de ruído), não há chimbal nem
caixa (ruído filtrado), não há vento nem mar — e, pior pra um modular,
**não há fonte de modulação aleatória**. Todo patch generativo quer um
sample-and-hold alimentando uma altura, um *smooth random* abrindo um
filtro devagar, um piso de ruído no fundo.

O `NOISE` faz as duas coisas: as saídas de áudio (`white`…`violet`,
`bit`) são textura; as saídas `sh` e `smooth` são acaso pra você cabear
em qualquer parâmetro. Tudo sai ao mesmo tempo — você puxa da cor que
quiser.

## Por dentro

**O que é "ruído branco":** uma sequência de valores **totalmente
imprevisíveis** amostra a amostra — cada amostra sorteada sem relação
nenhuma com a anterior. O nome vem de uma analogia com a luz branca: do
mesmo jeito que a luz branca contém todas as cores (frequências) de luz
em partes iguais, o ruído branco contém **toda frequência audível, com
a mesma energia** — é por isso que soa como um "chiado" parelho, sem
altura definida, e serve de matéria-prima neutra pra qualquer coisa que
precise de textura ou de acaso.

As outras cores são o branco **passado por um filtro** que muda **quais
frequências dominam** — cada filtro é descrito por quanto ele
sobe/desce por oitava (dobrar de frequência):

- **`pink`** — um filtro que perde energia devagar conforme a
  frequência sobe (**−3 dB/oitava**): os agudos ficam um pouco mais
  fracos que os graves, mas ainda estão lá. É o "ruído natural" — o som
  de vento, de mar, de chuva, porque é assim que ruído de fenômenos
  físicos costuma se distribuir.
- **`brown`** (marrom, ou "ruído de Brown/browniano") — um **passeio
  aleatório**: em vez de sortear cada amostra do zero, ele **soma um
  passo aleatório pequeno à amostra anterior**, então o valor deriva
  devagar em vez de pular — isso reforça muito mais os graves
  (**−6 dB/oitava**, o dobro da inclinação do rosa) e quase apaga os
  agudos. Soa grave, surdo, "encorpado" — bom como base de bumbo.
- **`blue`/`violet`** — o oposto: o branco é **diferenciado** (a
  operação inversa da soma que gera o marrom — mede a **diferença**
  entre uma amostra e a anterior), o que reforça os agudos
  (**+3 dB/oitava** pro azul, **+6** pro violeta, diferenciando duas
  vezes). Cada vez mais a energia se concentra lá em cima — o violeta
  já soa quase só como chiado fino, ideal pra excitar um chimbal.
- **`bit`** não é uma cor filtrada — é outra coisa: o valor pula entre
  **+1 e −1 puro** a cada amostra sorteada, sem nenhum degrau
  intermediário. Isso cria uma textura **dura e digital**, cheia de
  harmônicos regulares (não é "colorida" como as outras, é mais
  "quebrada").

**Sample-and-hold (`sh`), literalmente:** a cada pulso (do relógio
interno `RATE` ou de um `TRIG` externo), o módulo sorteia (ou lê a
entrada `IN`, se tiver cabo) um valor **novo** e **segura exatamente
esse número, parado**, até o próximo pulso — daí o nome "amostra e
segura". O resultado visualmente é uma escada de degraus: reto entre
pulsos, um salto instantâneo a cada pulso.

**`smooth`** faz a mesma decisão de "sorteia um valor novo a cada
pulso", mas em vez de saltar pra ele instantaneamente, **desliza**
suavemente até lá ao longo do tempo até o próximo pulso — não tem
degraus, é uma curva contínua entre os pontos sorteados.

**`spread`** muda **como** o sorteio escolhe o próximo valor: no
mínimo (0), qualquer valor entre os extremos tem a mesma chance —
saltos grandes acontecem tão fácil quanto pequenos. Subindo, a chance
se concentra mais **perto do valor atual/central** (uma curva de sino)
— a maioria dos sorteios vira pequenos ajustes, e saltos grandes ficam
raros.

**`poisson`** troca **quando** os pulsos acontecem: no mínimo (0), os
pulsos vêm num relógio **regular**, sempre no mesmo intervalo. No
máximo (1), os pulsos vêm de um **processo de Poisson** — cada
intervalo entre um pulso e o próximo é sorteado independentemente (às
vezes bem curto, às vezes bem longo), mas **em média** acontecem
`RATE` vezes por segundo. É o mesmo processo que descreve, por exemplo,
decaimento radioativo (daí o apelido "contador Geiger") — eventos
raros, cada um surpreendente, sem nenhum ritmo perceptível.

## Os jacks, um a um

### Entradas

- **`TRIG`** (controle, disparo) — substitui o relógio interno do S&H /
  smooth. Presente, `RATE` é ignorado. **Plugue aqui:** `CLOCK.euclid`,
  `SEQUENCE.eos`, `DECISION.gate` — o acaso passa a andar no seu tempo.
- **`IN`** (áudio) — a fonte que o S&H **amostra**. Sem cabo, o S&H usa
  o sorteio interno. **Plugue aqui:** um LFO, outra voz — o S&H
  "congela" fatias dela.

### Saídas (todas áudio)

Textura — vão para o `MIXER` (geralmente via `FILTER`/`ENVELOPE`) ou
para excitar um `MATTER`/`STRING`/`RESONATOR`:

- **`WHT`** branco · **`PNK`** rosa (vento, mar) · **`BRN`** marrom
  (grave, surdo — bom pra bumbo) · **`BLU`** azul · **`VLT`** violeta
  (quase só chiado — chimbal) · **`BIT`** 1 bit (glitch digital).

Acaso — vão para **qualquer entrada de parâmetro**:

- **`S&H`** — degraus. **Plugue em:** `OSC.1V/O` (via `QUANTIZER`) pra
  melodia aleatória, `FILTER.cutoff`, qualquer `_mod`.
- **`SMTH`** — desliza. **Plugue em:** parâmetros que você quer que
  derivem devagar — corte de filtro, `pos` de wavetable, `structure`.

## Os controles, um a um

**RATE** (0,01–2000 Hz) — o ritmo do relógio interno do S&H/smooth.
Ignorado se `TRIG` estiver conectado.

**SLEW** (0–1) — o tempo de deslize da saída `SMTH`. Em 0 ela salta
(vira quase um S&H); em 1, ~2 s pra chegar em cada valor novo.

**SPRD** (spread, 0–1) — a forma do sorteio de `S&H` e `SMTH`. 0 =
uniforme (cai igual em qualquer valor); 1 = sino (concentra perto do
centro — variações pequenas mais frequentes).

**POIS** (poisson, 0–1) — troca o relógio periódico por Poisson livre.
0 = pulsos regulares; 1 = tempos totalmente aleatórios, com `RATE` como
taxa *média*. É o par da saída `geiger` do `BOXCAR`. Semeado (reprodutível).

## Como cabear

**Como textura:**
```
NOISE (PNK) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)   (vento/mar)
NOISE (BRN) → DRUM (in)  ou  → MATTER (in)                (transiente de percussão)
```

**Como acaso:**
```
CLOCK (euclid) → NOISE (TRIG)
NOISE (S&H) → QUANTIZER (cv) → OSC (1V/O)     (melodia aleatória na escala)
NOISE (SMTH) → FILTER (cutoff)                (o filtro deriva sozinho)
```

## Potencializar

- **Melodia que muda de humor:** `SPREAD` num LFO lento — as notas
  aleatórias oscilam entre "pequenos passos" e "saltos grandes".
- **Geiger:** `POIS` em 1, `S&H` disparando um `ENVELOPE` — eventos
  esparsos e irregulares, nunca no compasso.
- **Chimbal de verdade:** `VLT` (violeta) → `FILTER` passa-alta →
  `ENVELOPE` bem curto disparado pelo `TRIGSEQ`.
- **Piso vivo:** `SMTH` bem lento somado (via `CONTROL`) a vários
  parâmetros do patch — tudo deriva um pouco, junto.

## Se você conhece o Eurorack

Faz o papel de um módulo de ruído (Doepfer A-118) + um S&H (Wogglebug,
Marbles) + *smooth random* (Buchla 266). A diferença: as **8 cores e as
2 formas de acaso saem simultâneas** — não há um seletor. E o `poisson`
traz o "contador Geiger" (tempos aleatórios sem clock) que quase nenhum
módulo de ruído tem.
