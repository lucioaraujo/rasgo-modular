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

### Entradas (controle)

- **`P1`**–**`P4`** — CV de pan (−1..+1) de cada canal, somada ao lugar em
  qualquer modo. **Plugue aqui:** um `PENDULUM` (x1/x2), um LFO lento.
- **`FRASE`** — no modo GESTO, cada gatilho aqui move um canal. **Plugue
  aqui:** o EOS de um `ARP`/`SEQUENCE`, o COL do `COHERENCE`, o MUD do
  `KEY`.

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

## O PALCO: pans que andam com intenção

A coluna da direita é o **PALCO**: um jeito de o pan não ficar parado no
centro, mas também não andar ao acaso.

**MODO**:
- **MANUAL** — o PAN é o lugar, e nada se move sozinho.
- **RODA** — os canais giram em torno das suas casas, defasados por
  igual.
- **CÂNONE** — um trajeto lento, que cada canal repete atrasado e
  espelhado.
- **GESTO** — a cada gatilho em FRASE (ou a cada tanto), um canal desliza
  para um lugar novo, escolhido para reequilibrar.
- **PÊNDULO** — só as entradas P1–P4 movem (ligue um `PENDULUM`).

Fora do MANUAL, o knob PAN de cada canal é a **casa** (o centro do
movimento). **VELOC** dá o período (de 60 s a 6 s) e **ABERT** dá a
amplitude.

**O contrapeso, um móbile.** Fora do MANUAL, cada canal pesa o som que
está tocando (a parte aguda; o grave mora no centro). Como numa alavanca, o
mais pesado anda menos. Quando o palco pende para um lado, quem corrige é
o leve, e o palco balança e assenta (cerca de 2,5 s). É uma regra de
composição inspirada nos móbiles de Calder, não uma simulação física.

O visor embaixo mostra o palco. O tempo desce, cada canal deixa o rastro
do lugar, o ponto cresce com o peso, e o traço embaixo é o centro de
gravidade.

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
- **Movimento:** o PALCO (acima) faz os pans andarem. Para um movimento
  seu, cabeie um LFO numa entrada P1–P4. A mão caótica (`VARIA`) só mexe
  em VELOC e ABERT; os níveis, os pans e o MODO são seus.
- **A frase move o palco:** no modo GESTO, ligue em FRASE o fim de frase de
  quem toca (o EOS do `ARP`, o COL do `COHERENCE`, o MUD do `KEY`).
- **Mute rítmico:** o painel não modula `mute` por cabo; use um `VCA` no
  caminho do canal com um gate se quiser "cortar" no compasso.

## Se você conhece o Eurorack

Faz o papel de um mixer de performance (WMD/SSF, Intellijel, Happy
Nerding) — 4 canais, pan, mute. A lei de pan é de potência constante
(fato público). O par é o `MASTER` (#17) — o estágio final; e o `VCA4`
(#59) é um mixer de brinde. Nenhum mixer do Cardinal pesa o estéreo pelo
som: lá, todo pan é uma posição dada. Na seed, com 2 ou mais vozes, o palco
abre nas casas (voz no centro, contravoz, textura e eco a cada lado) e
escolhe um modo; com 1 voz, fica em MANUAL no centro.
