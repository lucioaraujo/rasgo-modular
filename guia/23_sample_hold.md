# SH — sample & hold duplo

**Família:** MODULATE · **Módulo 23**
**Essência:** duas tensões que seguram um valor a cada pulso — com um
**botão de correlação** entre os dois acasos, de gêmeos a espelho.
**Dossiê técnico:** [`../dossies/23_sample_hold.md`](../dossies/23_sample_hold.md)
· **Fonte:** `src/dsp/SampleHold.hpp`

---

## A ideia

Todo patch generativo quer duas tensões que se movem **em relação**: uma
altura e um corte que "combinam", dois osciladores que derivam juntos,
uma pergunta e uma resposta. O `NOISE.sh` dá uma. Duas fontes
independentes dão acaso descorrelacionado. O que falta é o botão de
correlação — e é isso que o `SH` traz: `CORR` cruza de "os dois sorteiam
o mesmo valor" a "espelhados" a "cada um pro seu lado".

## Por dentro

Por canal: a cada pulso (de `TRIG` externo ou do relógio interno
`RATE`), a saída segura o valor de `IN` (ou, sem `IN` cabeado, um
sorteio interno — a mesma mecânica do `sh` do `NOISE`, #19). `SLEW`
desliza até o valor novo em vez de saltar (ver o "Por dentro" do
`NOISE` pra `smooth`); `SLOPE` faz esse deslize assimétrico — subir
rápido e descer devagar, ou o oposto (a mesma ideia do `FALL` do
`GLIDE`, #39, aplicada ao S&H em vez de à altura).

**Como `CORRELATION` liga dois sorteios independentes, continuamente:**
nos extremos é simples de entender — em `CORR`=+1, os dois canais
literalmente sorteiam **o mesmo número** a cada pulso (gêmeos: sempre
concordam); em `CORR`=−1, um canal sorteia um número e o outro recebe o
**oposto** dele (espelhados: quando um sobe, o outro desce na mesma
medida); em `CORR`=0, cada canal sorteia o seu, sem relação nenhuma. O
que `CORRELATION` faz entre esses extremos é misturar, em proporções
variáveis, "o sorteio próprio do canal" com "uma cópia (ou o oposto) do
sorteio do outro canal" — perto de +1, a maior parte vem da cópia
(quase gêmeos, mas com uma pitada de diferença própria); perto de 0,
maior parte vem do sorteio independente. Isso só faz sentido nos canais
**sem `IN` cabeado**, porque só aí existe um "sorteio interno" pra
correlacionar — um canal amostrando um `IN` externo já está seguindo
uma fonte real, não um acaso pra combinar com o outro canal.

**`TRACK` (track & hold), a diferença pro sample & hold comum:** um
sample & hold clássico captura **um instante** — o valor exato de `IN`
no momento da borda do pulso — e ignora tudo que `IN` faz depois, até o
próximo pulso. *Track & hold* muda essa regra: **enquanto** o trigger
estiver alto (não só na borda), a saída **segue `IN` ao vivo**,
continuamente; só quando o trigger cai é que ela **congela** no último
valor visto. É útil pra "abrir uma janela" de um tempo variável sobre
um sinal contínuo (um LFO, por exemplo) em vez de amostrar num instante
fixo.

## Os jacks, um a um

### Entradas

- **`IN1`** / **`IN2`** (controle) — a fonte que cada canal **amostra**.
  Sem cabo, o canal usa o sorteio interno (correlacionável). **Plugue
  aqui:** um LFO, `NOISE.pink`, outra voz — o S&H "congela" fatias dela.
- **`T1`** / **`T2`** (controle, disparo) — o pulso de amostragem.
  Presente, substitui `RATE`. **Plugue aqui:** `CLOCK.euclid`,
  `SEQUENCE.eos`, `DECISION.gate`. Cabeie o mesmo trigger nos dois pra
  eles amostrarem juntos.

### Saídas (ambas controle)

- **`O1`** / **`O2`** — o valor amostrado de cada canal, com
  `SLEW`/`SLOPE` aplicados. **Plugue em:** `QUANTIZER.cv` →
  `OSC.1V/O` (melodia aleatória), `FILTER.cutoff`, qualquer `_mod`.

## Os controles, um a um

**RATE** (0,02–40 Hz) — o relógio interno. Usado só se `T1`/`T2` estiver
livre.

**SPRD** (spread, 0–1) — puxa o sorteio interno de uniforme para sino
(variações pequenas mais frequentes). Só os canais sem `IN`.

**SLOPE** (−1..1) — a assimetria do deslize: >0 desliza pra baixo
devagar e sobe rápido (portamento de *pluck*); <0 o oposto; 0 simétrico.

**SLW1** / **SLW2** (slew, 0–1) — o tempo de deslize de cada canal até o
valor amostrado. 0 = degrau instantâneo.

**CORR** (correlation, −1..1) — a correlação entre os dois acasos
internos: −1 espelhados, 0 independentes, 1 gêmeos (sorteiam o mesmo
valor). Só afeta os canais sem `IN` cabeado.

**TRK1** / **TRK2** (track, chave) — *track & hold*: segue `IN` ao vivo
enquanto o trigger fica alto, em vez de amostrar só na borda.

## Como cabear

**Melodia + timbre relacionados:**
```
CLOCK (euclid) → SH (T1)  e  → SH (T2)
SH (O1) → QUANTIZER (cv) → OSC (1V/O)
SH (O2) → FILTER (FC)
```
`CORR` ~0,6 — a nota e o corte "combinam" mas não são iguais.

**Duas CVs aleatórias correlacionáveis (sem entrada):**
```
CLOCK → SH (T1/T2)
SH (O1) → destino A
SH (O2) → destino B
```
Gire `CORR` de −1 a +1 e ouça os dois destinos se moverem em espelho,
independentes, ou em uníssono.

## Potencializar

- **Pergunta e resposta:** `CORR` = −1 — quando um destino sobe, o outro
  desce; um "diálogo" entre duas vozes.
- **Portamento de acaso:** `SLEW` alto + `SLOPE` positivo — a melodia
  aleatória escorrega entre as notas como um *pluck* solto.
- **Track & hold:** `TRK1` ligado + um LFO no `IN1` + um gate longo — o
  canal segue o LFO nos trechos "abertos" e congela nos "fechados".
- **Cruze com o `DRIFT`:** `DRIFT.a → SH.IN1` — o acaso do `SH` amostra
  um passeio lento, então os valores sorteados também derivam ao longo
  de minutos.

## Se você conhece o Eurorack

Faz o papel de um S&H duplo (Doepfer A-148) + *smooth random* (Buchla
266) + o `X`/spread do Mutable Marbles. A diferença: o knob `CORRELATION`
(gêmeos↔espelho) entre os dois acasos internos. A outra fonte de acaso
é o `NOISE` (#19), com 8 saídas simultâneas.
