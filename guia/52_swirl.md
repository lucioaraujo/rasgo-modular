# SWIRL — chorus / flanger / phaser / ensemble

**Família:** SPACE · **Módulo 52**
**Essência:** a metade "movimento" da caixa de efeitos — largura,
cintilância e o varrer de pente/notch que nem o reverb nem o delay
fazem. Os quatro num módulo.
**Dossiê técnico:** [`../dossies/52_swirl.md`](../dossies/52_swirl.md)
· **Fonte:** `src/dsp/Swirl.hpp`

---

## A ideia

Chorus, flanger, phaser e ensemble são a **mesma** ideia — um atraso
curto modulado, com ou sem realimentação; o phaser troca a linha de
atraso por all-pass. Por isso cabem num módulo só (`TYPE`). É o que dá
largura a uma voz mono, faz um pad "cantar" e um lead engordar.
Distinto do `LOOPER` (ecos audíveis) e do reverb — aqui você ouve o
**movimento**, não o eco.

## Por dentro

**Por que um atraso curto modulado produz chorus/flanger, em uma
explicação:** somar um sinal com uma cópia dele mesmo atrasada por um
tempo bem curto cria um **filtro em pente** (comb filter — o mesmo
fenômeno explicado no `Difference` de `RELACAO_DE_CABO.md` §1.3 e na
receita de realce de transiente): em certas frequências as duas cópias
se reforçam, em outras se cancelam, formando picos e vales regulares
no espectro. Se esse atraso é **fixo**, o resultado é uma coloração
estática. Se o tempo do atraso **varre** continuamente (por um LFO —
`RATE`/`DEPTH`), os picos e vales do pente **deslizam** pra cima e pra
baixo no espectro ao longo do tempo — esse deslizar contínuo é
literalmente o som "líquido"/"varrendo" do chorus e do flanger.

O que diferencia os quatro `TYPE` é a receita específica em cima dessa
ideia:

- **chorus** — atraso maior (~12 ms), sem realimentação, com uma
  segunda voz — o pente é suave e a percepção é mais de "dobrar a
  voz" que de um efeito metálico óbvio.
- **flanger** — atraso bem mais curto (~1,2 ms) **com** realimentação —
  o pente fica mais estreito e acentuado (menos vales, mais profundos),
  e a realimentação reforça isso ainda mais a cada volta — o clássico
  som "jato decolando". Perto de `FEEDBACK`=±1, o laço se sustenta
  sozinho (a mesma ideia de ciclo-limite explicada em `CABEAMENTO.md`
  §1) — o flanger passa a **cantar** mesmo sem entrada.
- **ensemble** — três vozes, cada uma com seu próprio LFO, em
  frequências **incomensuráveis** (que nunca se realinham exatamente
  de forma periódica simples entre si) — isso evita que as três
  modulações "batam" juntas em ciclos previsíveis, dando uma sensação
  de movimento mais rico e "vivo" do que um único LFO conseguiria (o
  som clássico de teclados de cordas Juno/Solina).
- **phaser** — troca a linha de atraso por uma cadeia de 6 filtros
  **all-pass** (a mesma ideia usada na difusão do `SPACE`, #10, só
  que aqui poucos estágios, não muitos): em vez de um pente com muitos
  dentes igualmente espaçados, o phaser produz **poucos** entalhes
  largos, que se movem com o LFO — um timbre "oco"/"giratório"
  distinto do chorus/flanger, sem o caráter metálico do pente denso.

`SPREAD` aplica o mesmo LFO ao canal direito, mas **defasado** em
relação ao esquerdo — os dois canais varrem de formas ligeiramente
diferentes, o que é percebido como largura estéreo. `AGE` acrescenta o
mesmo caráter de fita/BBD (companding, banda limitada, wobble) descrito
no `LOOPER` (#41) — é essa imperfeição que permite o flanger "cantar"
sozinho de forma orgânica, não perfeitamente periódica.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — a voz a mover. **Plugue aqui:** qualquer voz, ou a
  soma do `MIXER` (largura no patch inteiro).
- **`RTM`** (rate_mod) (controle) — CV que soma ao `RATE`. **Plugue
  aqui:** um LFO (a varredura respira fora do compasso), um `ENVELOPE`.
- **`MXM`** (mix_mod) (controle) — CV que soma ao `MIX`. **Plugue
  aqui:** um `ENVELOPE` — o efeito entra só no fim da frase.

### Saídas (ambas áudio)

- **`L`** / **`R`** — canais esquerdo e direito (a imagem estéreo vem do
  `SPREAD`). Vão a dois canais do `MIXER`, ou `L` direto e `R` por outro
  caminho.

## Os controles, um a um

**TYPE** (0–3) — chorus · flanger · ensemble · phaser.

**RATE** (0,02–8 Hz) — a velocidade do LFO. A CV `RTM` soma.

**DEPTH** (0–1) — a profundidade da modulação.

**FBK** (feedback, −1..1) — realimentação. Perto de ±1 (no flanger) +
`AGE` = auto-oscila.

**SPREAD** (0–1) — o LFO de R defasado — a largura estéreo.

**TONE** (−1..1) — 1 polo no molhado: <0 passa-baixa ("aveludado" BBD);
>0 passa-alta.

**AGE** (0–1) — o caráter BBD: companding + banda + wobble + ruído. `AGE`
= 0 → determinístico puro.

**MIX** (0–1) — seco ↔ molhado. 0 = bypass exato.

## Como cabear

**Engrossar uma voz:**
```
CHORD (OUT) → SWIRL (IN)     TYPE 0 (chorus), RATE lento, DEPTH ~0,4
SWIRL (L)/(R) → MIXER (ch1)/(ch2)
```

**Flanger com jato:**
```
voz → SWIRL (IN)     TYPE 1, FBK ~0,8, RATE ~0,3 Hz
```

**Phaser no pad:**
```
pad → SWIRL (IN)     TYPE 3, RATE bem lento
```

## Potencializar

- **Varredura fora do compasso:** um LFO no `RTM` que **não** seja
  divisor do `CLOCK` — a modulação "flutua" contra o groove.
- **Efeito por frase:** `ENVELOPE.env → MXM` — seco na maior parte, o
  chorus só floresce na cauda de cada frase.
- **Auto-oscilação como voz:** `TYPE` 1, `FBK` ±1, `AGE` alto, nada no
  `IN` — o flanger canta sozinho; cabeie num canal do `MIXER`.
- **Ensemble largo:** `TYPE` 2, `SPREAD` alto — o clássico "Solina" /
  Juno para strings.

## Se você conhece o Eurorack

Faz o papel de um módulo de modulação (4ms Ensemble Osc, o modo chorus
do Mimeophon, um phaser). A base é a linha de atraso + LFO (Roland
Dimension D / CE-1, MXR), o BBD (companding), o ensemble (Juno-60,
Solina) e o all-pass TPT (Zavalishin). O **`age`** é o desvio Rasgo — o
caráter de fita num knob.
