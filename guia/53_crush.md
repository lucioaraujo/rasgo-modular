# CRUSH — destruidor lo-fi

**Família:** TRANSFORM · **Módulo 53**
**Essência:** o lixo **digital** — baixar a taxa, baixar os bits,
estourar o inteiro, a falha da conexão ruim. Cinco vetores de dano,
todos reprodutíveis.
**Dossiê técnico:** [`../dossies/53_crush.md`](../dossies/53_crush.md)
· **Fonte:** `src/dsp/Crush.hpp`

---

## A ideia

Metade da estética digital-suja vem de quatro gestos: **baixar a taxa**
(a escada + o aliasing dos primeiros samplers), **baixar os bits** (o
degrau grosso), **estourar** (o *wrap* de inteiro, não o clip macio) e a
**falha** (o dropout, a amostra travada — a "conexão ruim"). O `SHAPE`
distorce de forma analógica; o `WASP` é um filtro áspero. Nenhum faz o
lixo digital. O `CRUSH` põe isso no caminho do sinal.

Tudo é **semeado** — o mesmo patch soa igual duas vezes. Dano
reprodutível, ao contrário de um bug.

## Por dentro

**Baixar a taxa, de propósito, sem anti-alias:** um sample-and-hold
interno (a mesma ideia do `sh` do `NOISE`, #19 — segura um valor até o
próximo pulso) roda numa taxa (`RATE`) mais baixa que a taxa de
amostragem real do sistema, criando os mesmos degraus grosseiros e o
mesmo *aliasing* explicado no "Por dentro" do `OSC` (#18) — só que lá o
PolyBLEP existe pra **evitar** isso, e aqui não existe filtro anti-alias
nenhum de propósito: o aliasing (harmônicos errados dobrando de volta
pra dentro da faixa audível) **é** o efeito que se quer, não um defeito
a corrigir.

**O que "quantizar a amplitude" (`BITS`) significa, concretamente:**
uma amostra de áudio normalmente pode assumir um número **enorme** de
valores possíveis entre −1 e 1 (2 elevado a 16, 24 bits…). `BITS`
reduz drasticamente quantos valores são permitidos — com `BITS`=1, só
existem **2** valores possíveis (basicamente +1 ou −1: qualquer sinal
vira uma onda quadrada bruta); com `BITS`=16, há valores suficientes
pra soar praticamente transparente. Entre os extremos, cada amostra é
**arredondada** pro degrau permitido mais próximo — esse arredondamento
constante é o que soa como o "chiado quantizado"/degrau grosso
característico do lo-fi digital.

**`DRIVE` + `WRAP`, o estouro de inteiro:** `DRIVE` empurra o sinal
**além** da faixa que caberia normalmente antes de quantizar. `WRAP`
decide o que fazer com esse excesso: em 0, ele é simplesmente
**cortado** no limite (um clipe comum, achatado). Em 1, o excesso
**"enrola"** — em vez de parar no limite, o valor continua contando a
partir do **outro** extremo (como um contador que, ao passar do máximo
que consegue representar, volta pro mínimo em vez de travar — o mesmo
fenômeno real de *overflow* de números inteiros em computação). O
resultado soa muito mais bruto que um clipe: em vez de um teto suave, é
uma descontinuidade abrupta repetida, cheia de harmônicos até o topo da
faixa audível.

`GLITCH` sorteia, a cada nova amostra-de-hold, se ela vai falhar de
alguma forma (travar num valor, cair em silêncio, ou repicar) — uma
falha semeada, então reprodutível. `JITTER` faz o próprio **relógio**
do sample-and-hold tremer (em vez de segurar por um tempo fixo, o tempo
varia um pouco a cada vez) — isso desafina sutilmente o resultado a
cada instante, o "wow" digital (o equivalente eletrônico do wow/flutter
de fita analógica, mas produzido por instabilidade de clock em vez de
motor). `TONE` é só um filtro simples de 1 polo depois de tudo isso,
pra domar ou realçar o que sobrou.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a destruir. **Plugue aqui:** qualquer voz
  ou o barramento. Desconectado + `MIX` > 0 → o S&H amostra o próprio
  ruído branco → uma fonte lo-fi/glitch.
- **`RTM`** (controle) — soma ao knob `RATE`. **Plugue aqui:** um LFO
  (varre a taxa — o "engoli-fita ao contrário"), um `ENVELOPE`.
- **`MXM`** (controle) — soma ao knob `MIX`. **Plugue aqui:** um
  `ENVELOPE` — o dano entra **por dose** na cauda da nota.

### Saída

- **`OUT`** (áudio) — o sinal destruído, misturado com o seco por `MIX`.
  Vai ao `MIXER`.

## Os controles, um a um

**RATE** (100 Hz – 24 kHz) — a taxa do S&H interno. Baixo = escada
grossa + aliasing forte (a "voz de robô" dos 8-bit).

**BITS** (1–16) — quantiza a amplitude. 1 ≈ onda quadrada; 16 ≈
transparente.

**DRIVE** (0–4) — ganho **antes** da quantização; empurra pro transbordo.

**WRAP** (0–1) — como o sinal que passa de ±1 se comporta: 0 clipa; 1
enrola (dente de serra brutal). Com `DRIVE` alto + `WRAP` = 1, enrola
várias vezes → harmônicos até Nyquist.

**GLITCH** (0–1) — probabilidade de falha por amostra-de-hold: amostra
travada, *dropout* (silêncio) ou repique. Semeado.

**JITTER** (0–1) — instabilidade da taxa do S&H → instabilidade de
afinação (o "wow" digital). Semeado.

**TONE** (−1..1) — filtro de 1 polo na saída: <0 passa-baixa (dócil);
>0 passa-alta (só o lixo agudo); 0 neutro.

**MIX** (0–1) — seco ↔ destruído. 0 = bypass exato.

## Como cabear

**Sujar uma voz:**
```
OSC (SAW) → FILTER (in) → CRUSH (IN) → MIXER (ch1)
```
`RATE` ~4 kHz, `BITS` ~8, `MIX` a gosto.

**Dano por dose:**
```
qualquer voz → CRUSH (IN) → MIXER
ENVELOPE (env) → CRUSH (MXM)      (a nota começa limpa e se degrada)
```

## Potencializar

- **Resolução que cai:** `ENVELOPE.env → BITS` (via `CONTROL` invertido)
  — a nota perde bits na cauda.
- **Vinil digital:** `GLITCH` ~0,05 + `JITTER` ~0,2 + `TONE` levemente
  negativo — falhas esparsas e um *wow* sutil.
- **Fonte de glitch:** nada no `IN`, `MIX` = 1, `GLITCH` alto — uma voz
  de ruído picotado (cabeie no seu canal do `MIXER`).
- **Antes do reverb:** `CRUSH → SPACE` — a reverberação suaviza o lixo e
  dá um "digital fantasmagórico".

## Se você conhece o Eurorack

Faz o papel do Schlappi 100 Grit, do OTO Biscuit, do µBraids no modo
lo-fi. A base é teoria de amostragem (decimação sem anti-alias,
quantização uniforme), o overflow de complemento de dois (`wrap`), e o
*dropout* de S/PDIF (fenômeno, reescrito). Distinto do `SHAPE`/`WASP`
(distorção analógica) e do `wear` do `SAMPLER` (degradação por disparo).
