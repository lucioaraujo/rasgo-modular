# ENVELOPE — envelope + VCA

**Família:** MODULATE · **Módulo 6**
**Essência:** o gesto que articula — transforma um drone em frase, um
clique em nota, um ruído em percussão. A/D/S/R com um VCA embutido.
**Dossiê técnico:** [`../dossies/06_envelope.md`](../dossies/06_envelope.md)
· **Fonte:** `src/dsp/Envelope.hpp`

---

## A ideia

As fontes fazem som, mas o som está sempre ligado. Falta o gesto que
**articula**. Numa lógica generativa, "disparar uma nota" é uma coisa
só: o `CLOCK`/`DECISION` manda o gate, o `ENVELOPE` dá forma e volume.
Por isso o envelope e o VCA vêm juntos: você cabeia a voz no `IN`, o
gate no `GATE`, e sai a voz articulada no `OUT` — e a forma em si na
saída `ENV`, pra modular o que quiser.

## Por dentro

**O que é um contorno A/D/S/R, pra quem nunca ouviu o termo:** quase
nenhum som real liga instantaneamente no volume máximo e desliga
instantaneamente — ele **sobe** até um pico (o **A**taque), **cai** um
pouco desse pico (o **D**ecaimento), pode **segurar** num nível
enquanto a fonte continua ativa (o **S**ustain — o único dos quatro que
é um **nível**, não um tempo) e finalmente **cai** de vez quando a fonte
para (o **R**elease). É uma descrição genérica o bastante pra cobrir um
piano, um sopro, uma batida — mudando só os quatro números, o mesmo
contorno vira qualquer um desses gestos.

**Por que existem dois `MODE` diferentes, e o que cada um imita:** um
instrumento com **sustain de verdade** (um órgão, um sopro mantido)
precisa saber **quando você soltou a nota** pra decidir quando cair — é
o modo *gated* (ASR): o envelope sobe, cai até `SUS` e **fica ali**
até o `GATE` descer, só então entrando no `R`. Um instrumento de
**ataque percussivo** (uma tecla de piano, uma baqueta) já sabe, desde
o toque, a forma completa que o som vai fazer — não importa se você
segura o dedo lá ou não, o som decai do mesmo jeito. É o modo *trigger*
(AD): o contorno inteiro roda até o fim, ignorando a duração do gate.
`SUS`=0 no modo ASR produz esse mesmo efeito (sem platô pra segurar),
mas o modo AD garante que o contorno **sempre** completa, mesmo com um
gate curtíssimo.

**O que `CURVE` muda na forma:** uma curva **côncava** sai rápido do
ponto de partida e desacelera perto do alvo (a maior parte da subida já
aconteceu logo no início) — soa suave, gradual. Uma curva **convexa**
faz o oposto: começa devagar e acelera perto do alvo, então a maior
parte da mudança acontece **de repente, perto do fim** — o ouvido
percebe isso como um ataque mais "batido"/impactante, mesmo com o mesmo
tempo `ATK` configurado.

O `IN` passa por um VCA embutido, multiplicado pelo contorno —
`VCA`=1 significa "o som só existe enquanto o contorno estiver acima de
zero" (totalmente gateado); `VCA`=0 deixa o `IN` passar **sem** ser
afetado pelo contorno — útil quando você só quer a saída `ENV` como CV
e não precisa que o `ENVELOPE` também controle volume de nada.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — a voz a articular. **Plugue aqui:** a saída de um
  `FILTER`/`OSC`/qualquer voz. O `ENVELOPE` embute o VCA — não precisa
  de um `VCA` separado pra o caso comum.
- **`GATE`** (controle, gate) — dispara o envelope. **Plugue aqui:**
  `CLOCK.euclid`, `TRIGSEQ.t1`, `SEQUENCE.eos`, `DECISION.gate`,
  `SIGNAL-IN.gate`. No modo ASR importa a *duração*; no modo AD só o
  instante.
- **`TIME`** (controle, 1 V/oct) — CV que soma aos tempos (ATK/DEC/REL
  juntos). **Plugue aqui:** um `SEQUENCE` (envelopes mais curtos/longos
  por passo), `NOISE.smooth`.

### Saídas

- **`OUT`** (áudio) — a voz × o contorno (o VCA embutido). Vai ao
  `MIXER`.
- **`ENV`** (controle) — o contorno **em si**, como CV. **Plugue em:**
  `FILTER.cutoff` (o filtro abre com a nota — o gesto mais clássico),
  `SHAPE.fold`, `ADDITIVE.tilt`, `OPERATOR.index`, qualquer `_mod`.

## Os controles, um a um

**ATK** (attack, 0,001–10 s) — quanto leva pra chegar ao pico depois do
gate.

**DEC** (decay, 0,001–10 s) — a queda depois do pico (até `SUS`, se
houver).

**SUS** (sustain, 0–1) — o nível que o envelope segura enquanto o gate
fica alto. 0 = sem platô (AD de *pluck*).

**REL** (release, 0,001–10 s) — a queda depois que o gate desliga.

**CURVE** (0–1) — a forma da curva: côncava (suave) ↔ convexa ("batida",
ataque percebido rápido).

**VCA** (vca_depth, 0–1) — a profundidade do VCA embutido. 1 = o `OUT` é
totalmente gateado; 0 = o `OUT` passa o `IN` inteiro (só a saída `ENV`
importa).

**LVL** (level, 0–1) — o ganho de saída.

**TRIG** (mode, chave) — ASR (segura no gate) ↔ AD/trigger (completa o
contorno inteiro sempre).

## Como cabear

**A cadeia mínima de um sintetizador:**
```
OSC (SAW) → FILTER (in) → ENVELOPE (IN) → MIXER (ch1)
CLOCK (euclid) → ENVELOPE (GATE)
ENVELOPE (ENV) → FILTER (FC)      (o filtro abre com cada nota)
```

**Só como fonte de CV (o VCA de outra coisa):**
```
CLOCK → ENVELOPE (GATE)          VCA = 0 (o OUT não importa)
ENVELOPE (ENV) → LPG (CV)   ou   → CRUSH (MXM)   etc.
```

## Potencializar

- **Dois envelopes, um gate:** o mesmo `GATE` em dois `ENVELOPE` — um
  curto no `VCA` (o volume), um longo no `FILTER.FC` (o timbre decai
  mais devagar que a nota).
- **Timbre e volume relacionados:** `ENV → ADDITIVE.tilt` **e** o `OUT`
  articulado — o brilho e a amplitude sobem juntos.
- **Tempos que variam:** `SEQUENCE → TIME` — cada nota da frase tem um
  ataque diferente.
- **AD de *pluck*:** `SUS` = 0, modo TRIG, `DEC` curto — percussão
  afinada de qualquer voz.

## Se você conhece o Eurorack

Faz o papel de um ADSR com VCA (Doepfer A-140 + A-131), ou de um Maths /
Just Friends / Make Noise Contour. A diferença: envelope **e** VCA num
módulo (porque disparar uma nota é uma coisa só), e o `MODE` ASR/AD num
toggle.
