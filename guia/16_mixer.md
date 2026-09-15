# MIXER — mixer de 4 canais

**Família:** OUT · **Módulo 16**
**Essência:** o somador com nível e pan por canal. Onde as vozes deixam
de brigar pelo mesmo ponto e o instrumento vira estéreo.
**Dossiê técnico:** [`../dossies/16_mixer.md`](../dossies/16_mixer.md)
· **Fonte:** `src/dsp/Mixer.hpp`

---

## A ideia

Os módulos de voz produzem sinal, mas no Rasgo o fan-in é **explícito** —
não dá pra ligar 3 vozes na mesma entrada. Faltava o somador com
orçamento de ganho — e, já que estamos somando, o lugar natural pra
**posicionar cada voz no palco**. Sem o `MIXER`, o instrumento é mono e
as vozes se empilham.

**É por aqui que o som chega até você:** toda voz tem que passar por um
canal do `MIXER`, e o `MIXER` vai pro `MASTER` (ver
[`CABEAMENTO.md`](CABEAMENTO.md)).

## Por dentro

4 canais de áudio (`ch1`–`ch4`), cada um com ganho, pan e mute, somados
numa saída estéreo. Dois detalhes valem a explicação:

**Por que o ganho é em dB, não num multiplicador direto:** decibéis são
uma escala **logarítmica** — a mesma razão explicada no "Por dentro" do
`VCA` (#20): o ouvido percebe volume de forma exponencial, não linear,
então mover o slider por uma quantidade fixa em dB sempre soa como "a
mesma mudança de volume percebida", em qualquer ponto da faixa — bem
diferente de um multiplicador linear, onde o mesmo passo soa muito
mais dramático perto de zero do que perto do máximo.

**A lei de pan de potência constante, o problema que resolve:** um
crossfade **linear** simples entre esquerda e direita (a mesma questão
já vista no `CURVE` do `PLANAR`, #43) faz o volume **percebido** cair
no centro — os dois canais, cada um pela metade, não se somam de forma
simples em energia. A lei de potência constante usa uma curva diferente
(não linear) desenhada especificamente pra manter o **nível percebido**
igual em qualquer posição do pan, do extremo esquerdo ao direito,
passando pelo centro sem esse "buraco" de volume.

## Os jacks, um a um

### Entradas (áudio)

- **`1`** / **`2`** / **`3`** / **`4`** — os 4 canais. **Plugue aqui:**
  a saída de áudio de qualquer voz ou processador — `ENVELOPE.out`,
  `FILTER.all`, `DRUM.out`, `SPACE.out`, `SWIRL.l`/`r`…

### Saída

- **`L+R`** (out) (áudio) — a soma estéreo dos 4 canais. **Plugue em:**
  `MASTER.IN` (no patch de seed já vem feito). Também num `SPACE`/`HALL`
  se quiser um reverb do barramento antes do master.

## Os controles, um a um

**CH1–CH4** (gain, slider, −60 a +12 dB) — o nível de cada canal.

**PAN** (−1..1) — a posição estéreo de cada canal (lei de potência
constante).

**MUTE** (chave) — silencia o canal sem mexer no `GAIN`.

**OUT** (out_gain, −24 a +12 dB) — o nível do barramento de saída,
depois da soma.

## Como cabear

**Um patch de várias vozes:**
```
voz principal → FILTER → ENVELOPE → MIXER (ch1)
segunda voz   → MIXER (ch2)      pan à esquerda
percussão     → MIXER (ch3)
reverb (SPACE.wet) → MIXER (ch4)
MIXER (L+R) → MASTER (IN)        (já vem feito no seed)
```

## Potencializar

- **Um canal como retorno de efeito:** `SPACE.wet` ou `HALL.l`/`r` num
  canal próprio — você mixa seco e molhado como faixas separadas.
- **Sub-mix:** um `VCA4.mix` ou um segundo `MIXER` alimentando um canal
  deste — libera canais quando o patch cresce.
- **Automação de balanço:** os pans e níveis do `MIXER` ficam **de fora
  da mão caótica** (`VARIA`) — são seus. Mas você pode cabear um LFO num
  `pan` via `connectToParameter` se quiser movimento.
- **Mute rítmico:** o painel não modula `mute` por cabo; use um `VCA` no
  caminho do canal com um gate se quiser "cortar" no compasso.

## Se você conhece o Eurorack

Faz o papel de um mixer de performance (WMD/SSF, Intellijel, Happy
Nerding) — 4 canais, pan, mute. A lei de pan é de potência constante
(fato público). O par é o `MASTER` (#17) — o estágio final; e o `VCA4`
(#59) é um mixer de brinde. Os pans nascem sempre no centro em todo seed
(decisão do autor).
