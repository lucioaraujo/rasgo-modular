# STAGES — gerador de segmentos

**Família:** MODULATE · **Módulo 54**
**Essência:** um objeto só que soa como quatro conforme você o liga —
envelope, LFO complexo, sequenciador de CV, ou oscilador bizarro. A
**forma composta** de N segmentos.
**Dossiê técnico:** [`../dossies/54_stages.md`](../dossies/54_stages.md)
· **Fonte:** `src/dsp/Stages.hpp`

---

## A ideia

O `FUNCTION` faz **uma** rampa; o `ENVELOPE` faz ADSR; o `STAGES` faz a
forma **composta** — 2 a 8 segmentos encadeados, cada um com seu nível e
sua curva. Conforme você configura:

- 3 segmentos + `LOOP` off = um envelope;
- 6 segmentos suaves + `LOOP` on = um LFO que nunca se repete igual;
- 8 segmentos com `HOLD` = 1 = um sequenciador de CV;
- `RATE` na faixa de áudio = um oscilador de forma bizarra.

E você esculpe a forma com poucos macros (`CONTOUR`/`TILT`/`HOLD`),
não desenhando *breakpoints* — a identidade RASGO de gerador, não editor.

## Por dentro

`SEGMENTS` divide uma volta completa em N trechos, cada um com seu
próprio nível-alvo. **`CONTOUR` decide o desenho geral desses
níveis-alvo** ao longo dos N segmentos — não segmento por segmento, o
formato inteiro de uma vez: em 0, os níveis sobem progressivamente
(uma escada subindo, 0→1) — bom pra um ataque longo em vários degraus;
em 0,5, sobem na primeira metade e descem na segunda (um arco, 0→1→0)
— a forma de um envelope genérico; em 1, descem progressivamente — um
release longo em vários degraus. Entre esses três, um blend contínuo.

**`HOLD` é o knob que decide se o `STAGES` soa como coisas
fundamentalmente diferentes:** em 0, a saída **desliza** suavemente
entre um nível-alvo e o próximo (uma rampa contínua — é assim que um
envelope ou um LFO se parecem, sempre em movimento). Em 1, a saída
**salta** pro nível-alvo do próximo segmento e **fica parada** ali até
o segmento seguinte (um degrau reto — exatamente o comportamento de um
sample-and-hold ou de um passo de sequenciador). Nenhuma outra
diferença de configuração muda tanto o "gênero" do módulo — é por isso
que `HOLD` é chamado de "o knob que faz o `STAGES` virar envelope OU
sequenciador".

`CURVE` decide a forma da transição **dentro** de cada segmento (só
importa com `HOLD`=0, já que `HOLD`=1 não tem transição, só um salto):
exponencial sai rápido e desacelera perto do alvo; logarítmica é o
oposto — começa devagar e acelera no fim (a mesma distinção de côncavo
×convexo do `ENVELOPE`, #6). `TILT` distorce **quanto tempo** cada
segmento ocupa dentro da volta — negativo alonga os segmentos do
começo (um "ataque" mais longo relativo ao resto), positivo alonga os
do fim (um "release" mais longo) — sem mudar o número de segmentos nem
seus níveis, só a proporção de tempo que cada um ocupa.

`LOOP` decide se essa volta inteira se repete sozinha pra sempre (LFO
complexo, sem precisar de gate nenhum) ou se ela roda **uma vez** por
disparo do `GATE` e depois segura no último nível até o próximo — a
mesma distinção *free-running*×*one-shot* que aparece em vários outros
geradores do Rasgo.

## Os jacks, um a um

### Entradas

- **`GATE`** (controle, gate) — no modo *one-shot* (`LOOP` off), a borda
  de subida inicia uma passagem. Ignorado no modo loop. **Plugue aqui:**
  `CLOCK.euclid`, `TRIGSEQ`, `SEQUENCE.eos`.
- **`RST`** (reset) (controle, disparo) — volta ao segmento 0. **Plugue
  aqui:** o `EOC` de outro `STAGES`, um `CLOCK` de compasso.
- **`RTM`** (rate_mod) (controle) — CV que soma ao knob `RATE`.

### Saídas (todas controle)

- **`OUT`** — a função (o contorno dos N segmentos). **Plugue em:**
  `FILTER.cutoff`, `OSC.1V/O` (se `HOLD` = 1, uma sequência de notas via
  `QUANTIZER`), `SPACE.mix`, qualquer `_mod`.
- **`EOC`** (end-of-cycle) (gate) — um pulso no fim da volta. **Plugue
  em:** o `RST` de outra instância (encadear formas), um `DECISION`.
- **`STEP`** (gate) — um pulso em **cada fronteira de segmento**.
  **Plugue em:** `ENVELOPE.gate` (um AD por segmento), `DRUM.GATE`,
  `TRIGSEQ`.

## Os controles, um a um

**SEGS** (segments, 2–8) — quantos degraus/rampas na volta. Mais = mais
resolução na forma.

**RATE** (0,02–20 Hz) — a velocidade da volta no modo LOOP. Cada segmento
leva `(1/rate)/segments`, ajustado por `TILT`.

**CNTR** (contour, 0–1) — a forma dos níveis-alvo: 0 escada subindo,
0,5 arco, 1 escada descendo (blend).

**CURVE** (−1..1) — a curva de transição de cada segmento: <0 exponencial
(rápido→lento), 0 linear, >0 logarítmica (devagar→rápido). Sem efeito
com `HOLD` = 1.

**HOLD** (0–1) — 0 = desliza suave (rampa); 1 = salta e segura (degrau).
**O knob que faz o STAGES virar envelope OU sequenciador** sem trocar de
módulo.

**TILT** (−1..1) — distorção das durações: <0 os segmentos do começo
mais longos (attack lento); >0 os do fim (release lento).

**JITR** (jitter, 0–1) — passeio lento e semeado nos níveis e durações —
a forma "respira", reprodutível. 0 = byte-idêntico.

**LOOP** (chave) — corre livre (LFO complexo) ↔ um disparo (envelope —
precisa do `GATE`; congela no último nível até o próximo).

## Como cabear

**LFO complexo:**
```
STAGES (OUT) → FILTER (FC)     LOOP on, SEGS 6, HOLD 0, CONTOUR ~0,4
```

**Sequenciador de CV:**
```
CLOCK (clock) → STAGES (RST)   ou deixe correr
STAGES (OUT) → QUANTIZER (cv) → OSC (1V/O)     HOLD = 1, SEGS 8
```

**Envelope multi-estágio:**
```
CLOCK (euclid) → STAGES (GATE)     LOOP off
STAGES (OUT) → VCA (CV)
```

**Encadeando dois:**
```
STAGES #1 (EOC) → STAGES #2 (RST)     (o 2º só roda quando o 1º termina)
```

## Potencializar

- **Um AD por segmento:** `STEP → ENVELOPE.gate` — cada fronteira de
  segmento dispara uma micro-articulação.
- **Forma que respira:** `JITR` ~0,3 num LFO complexo — a forma nunca
  repete exatamente, mas o render é reprodutível.
- **Ritmo derivado da forma:** `STEP → DRUM.GATE` — a percussão segue as
  fronteiras dos segmentos (padrões irregulares "musicais").
- **`TILT` como gesto:** um `FUNCTION` bem lento no `RTM` não; automatize
  o `TILT` à mão — a forma migra de "ataque lento" a "release lento" ao
  longo da peça.

## Se você conhece o Eurorack

Faz o papel do Mutable Stages, do Rossum Control Forge, do Blukač
Fractalist. A diferença: a forma é esculpida por **macros**
(`contour`/`tilt`/`hold`), não por edição de *breakpoints*. Distinto do
`FUNCTION` (uma rampa) e do `ENVELOPE` (ADSR fixo).
