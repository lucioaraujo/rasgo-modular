# CHORD — voz de acorde

**Família:** SOURCE · **Módulo 26**
**Essência:** um VCO parafônico — entra uma altura, sai um acorde de 2 a
4 vozes. Dez formatos, condução de vozes na troca, e casa com o
`HARMONY` pra a progressão tocar sozinha.
**Dossiê técnico:** [`../dossies/26_chord.md`](../dossies/26_chord.md)
· **Fonte:** `src/dsp/Chord.hpp`

---

## A ideia

Um modular é monofônico por natureza — uma nota por oscilador. Fazer um
acorde exige N osciladores + N quantizadores + roteamento, e mudar o
acorde ao vivo é impraticável. O `CHORD` empacota isso: uma entrada de
altura, um knob de formato, e sai um acorde afinado. Com o `HARMONY`
cabeado no `CHRD`, a progressão anda sozinha — o modo autônomo do
instrumento.

## Por dentro

**O que é uma "tabela de intervalo", concretamente:** um acorde é só um
conjunto de notas descritas pela **distância** (em semitons) até a
fundamental — "maior" é sempre fundamental + 4 semitons + 7 semitons,
não importa qual seja a fundamental. É por isso que basta **uma** tabela
de deslocamentos por formato de acorde: o `CHORD` soma esses
deslocamentos (em 1 V/oct, ver `CABEAMENTO.md` §4 — somar tensão
multiplica frequência) à altura que entra em `PITCH`, e o mesmo acorde
"maior" sai afinado certo em qualquer fundamental. De 2 a 4 osciladores
(`VOX`) tocam, cada um, a fundamental deslocada por um desses
intervalos da tabela escolhida — uníssono, oitavas, quinta, maior,
menor, sus4, maj7, min7, dim, add9.

**`INV` (inversão), pra quem não conhece o termo:** um acorde não
precisa ter suas notas em ordem crescente a partir da fundamental —
você pode pegar a nota mais grave e subi-la uma oitava, e o acorde
continua sendo "o mesmo acorde" (mesmas notas, classes de altura
iguais), só com outra distribuição no grave/agudo. `INV` faz
exatamente isso: sobe as *n* vozes mais graves uma oitava, mudando a
"cara"/densidade do acorde sem trocar as notas que o definem.

**`VLEAD` (condução de vozes), pra quem não conhece o termo:** ao trocar
de um acorde pro outro, existem várias formas de mover as vozes — cada
uma pode saltar direto pra sua nova nota (sem noção do que estava
tocando antes), ou cada voz pode ir **especificamente** pra nota mais
próxima do acorde novo que ainda faz parte dele — a técnica clássica de
arranjo coral/de cordas, que minimiza o quanto cada "cantor" precisa se
mexer entre um acorde e o seguinte. `VLEAD` em 0 é o primeiro caso
(saltos); subindo, cada voz do `CHORD` recebe esse tratamento — desliza
(pequeno glide) pra nota mais próxima do acorde novo, em vez de pular.

`DTUNE` desafina levemente as vozes entre si (coro): duas fontes quase
idênticas, mas nunca exatamente na mesma frequência, batem uma contra a
outra num ritmo lento (o "batimento") que soa mais rico/vivo que uma
única fonte parada.

**Por que a soma é escalada por `1/√vozes`:** quando você soma **N**
fontes de som independentes (sem relação de fase fixa entre elas), o
volume total percebido não cresce N vezes — cresce, em média,
**√N** vezes (as ondas às vezes se reforçam, às vezes se cancelam um
pouco, ao acaso). Dividir por `√vozes` compensa exatamente esse
crescimento, então o acorde soa com um nível parecido não importa se
você está usando 2 vozes ou 4.

## Os jacks, um a um

### Entradas

- **`PITCH`** (controle, altura) — a **fundamental** do acorde. Cada
  volt dobra a frequência; soma ao `FREQ`. **Plugue aqui:**
  `QUANTIZER.pitch` alimentado por um `SEQUENCE`/`TURING` (um baixo que
  vira acorde).
- **`CHRD`** (controle) — soma ao knob `CHORD` (escolhe o formato).
  **Plugue aqui:** `HARMONY` (via um `CONTROL` pra escalar), um
  `SEQUENCE` de CV, um `NOISE.sh` — o formato do acorde muda a cada
  compasso.
- **`FM`** (áudio) — modulação de frequência linear das vozes. **Plugue
  aqui:** um LFO (vibrato) ou outra voz (timbre metálico no acorde
  inteiro).

### Saída

- **`OUT`** (áudio) — o acorde somado. Vai ao `MIXER`, quase sempre via
  `FILTER` + `ENVELOPE` (um acorde cru é um bloco; o filtro e o envelope
  fazem dele um *pad*).

## Os controles, um a um

**FREQ** (16–4000 Hz) — a fundamental / ponto de partida da altura.

**CHORD** (0–1) — varre as 10 tabelas de formato, de uníssono a add9.
A CV `CHRD` soma aqui.

**VOX** (voices, 2–4) — quantas vozes empilham. 2 = intervalo simples;
4 = acorde cheio.

**INV** (inversion, 0–1) — sobe as *n* vozes mais graves uma oitava. Muda
a "cara" do acorde sem mudar as notas.

**DTUNE** (detune, 0–1) — desafina as vozes entre si (até ±0,25 st).
Um pouco = coro rico; muito = *super-saw*.

**WAVE** (0–1) — a forma de onda das vozes: serra → pulso → triângulo
(com antialiasing na descontinuidade).

**DRIFT** (0–1) — passeio lento e independente por voz na afinação.
Em 0, determinístico; um toque dá vida ao acorde parado.

**VLEAD** (voicing, 0–1) — condução de vozes na troca de acorde. Em 0,
todas as vozes saltam. Subindo, cada voz vai para a nota mais próxima
do acorde novo, com glide — as trocas ficam suaves, como um coral.

## Como cabear

**Autônomo** — `HARMONY.root` → `CONTROL` → `CHORD.CHRD`, `CLOCK` no
`HARMONY`: a progressão toca sozinha.

**Pad melódico:**
```
SEQUENCE → QUANTIZER → CHORD (PITCH)     (a fundamental é uma linha de baixo)
CHORD (OUT) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)
FUNCTION (lento) → FILTER (cutoff)       (o pad abre e fecha)
```

## Potencializar

- **Progressão que respira:** `VLEAD` alto + um `SEQUENCE` de CV no
  `CHRD` — os acordes trocam sem ninguém pular, como um arranjo de
  cordas.
- **Do acorde ao unísono:** um `ENVELOPE` no `CHRD` — o acorde "abre"
  no ataque de cada nota e fecha em uníssono na cauda.
- **Coro de verdade:** `DTUNE` médio + `DRIFT` pequeno + um `SPACE`
  depois — três camadas de movimento, tudo reprodutível.
- **Cruze com o `SPECTRA`:** mande o `CHORD` no `SPECTRA.in` com
  `freeze` — congela o acorde num pad infinito.

## Se você conhece o Eurorack

Faz o papel de Mutable Plaits no modelo "chord", ou de um Harmonaig
(quantizador de 4 notas). A diferença: `VLEAD` (condução de vozes) é
raro em hardware, e o par com o `HARMONY` (6 técnicas de movimento
harmônico reais) deixa a progressão inteira ser generativa.
