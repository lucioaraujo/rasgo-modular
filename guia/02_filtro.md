# FILTER — filtro de estado variável

**Família:** TRANSFORM · **Módulo 2**
**Essência:** um filtro cujas três saídas (grave / centro / agudo) são a
**mesma** frequência de corte — o caráter vem de como elas se relacionam.
Auto-oscila.
**Dossiê técnico:** [`../dossies/02_filtro.md`](../dossies/02_filtro.md)
· **Fonte:** `src/dsp/Filter.hpp`

---

## A ideia

Depois da fonte, o segundo bloco de quase todo patch. Mas num
instrumento onde os fluxos dão a variedade sonora, o filtro não devia
ser só "corta agudo / corta grave". Aqui as **três saídas** (`LO`, `CTR`,
`HI`) partem da mesma frequência de corte, e o gesto é o `SPREAD` que as
afasta — de "três tomadas de um filtro só" a "três filtros separados"
que formam uma resposta de formante. A relação entre as saídas é o
processo.

## Por dentro

**O que é um "filtro de estado variável" (SVF), pra quem nunca ouviu o
termo:** a maioria dos filtros analógicos simples só entrega **um**
tipo de resposta por vez (só grave, ou só agudo). Um SVF é uma
topologia que calcula **as três** ao mesmo tempo, do mesmo núcleo —
passa-baixa, passa-faixa e passa-alta saem juntas, sempre na mesma
frequência de corte, porque são literalmente três formas de ler o
**mesmo** estado interno do filtro. É por isso que o `FILTER` pode
oferecer `LO`/`CTR`/`HI` sem triplicar o processamento: são três
janelas pro mesmo cálculo. (TPT é só o método digital usado pra manter
esse comportamento estável em qualquer frequência de amostragem, sem
detalhe que importe pra tocar.)

**Por que `SPREAD` cria um "formante":** em `SPREAD`=0, as três saídas
compartilham exatamente a mesma frequência de corte — `LO`/`CTR`/`HI`
seriam, na prática, três variantes (grave/faixa/agudo) **da mesma
região** do espectro. Subindo `SPREAD`, o corte de cada uma se afasta
de propósito — `LO` desce, `HI` sobe, `CTR` fica no meio — e agora você
tem **três filtros de fato diferentes**, cada um deixando passar uma
faixa distinta. É a mesma ideia do banco de 5 formantes do `FORMANT`
(#45), só que aqui com 3 bandas móveis em vez de 5 fixas — por isso
"relação entre saídas" é o gesto: mandar `LO` e `HI` pra lugares
diferentes e variar `SPREAD` redistribui a energia entre eles ao vivo.

**Por que `RESONANCE` alto vira um oscilador:** ressonância funciona
realimentando de volta pro filtro um pouco da energia que já saiu, bem
na frequência de corte — isso **reforça** exatamente aquela faixa
estreita a cada volta. Perto do máximo, essa realimentação é forte o
bastante pra o filtro **se sustentar sozinho**, sem nenhuma entrada: o
menor ruído residual (ou mesmo silêncio digital) já é suficiente pra
alimentar um ciclo que se autoperpetua — o filtro deixa de "colorir" um
sinal e passa a **ser** a fonte, oscilando exatamente na frequência de
corte. É o mesmo fenômeno físico do microfone que "chia" quando chega
perto demais da caixa de som (realimentação acústica), só que aqui é
controlado e afinável.

`DRIVE` satura o sinal **antes** de ele entrar no filtro — a ordem
importa: saturar primeiro **adiciona harmônicos novos** ao sinal
(especialmente nos graves, que é onde a saturação costuma "engordar"
mais), e só depois o filtro decide o que sobra desses harmônicos
novos. Saturar depois do filtro (como o `DRIVE` de outros módulos)
coloriria harmônicos que já passaram pelo corte — um resultado
diferente.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a filtrar. **Plugue aqui:** `OSC`, `MATTER`,
  `NOISE`, qualquer voz ou processador anterior.
- **`FC`** (controle, 1 V/oct) — CV que **soma** ao knob `CUT`. **Plugue
  aqui:** `ENVELOPE.env` (o corte abre com a nota — o gesto mais
  clássico), um `FUNCTION` (LFO — *wah*), `NOISE.smooth`, `DRIFT`. O
  knob `CUT` continua funcionando com o cabo plugado.
- **`Q`** (controle) — soma ao knob `RESO`. **Plugue aqui:** um
  `ENVELOPE`, um LFO — a ressonância pulsa.
- **`SPR`** (controle) — soma ao knob `SPREAD`. **Plugue aqui:** um LFO
  lento — o formante das três saídas abre e fecha.

### Saídas (todas áudio)

- **`ALL`** — a soma das três. **É a saída mais usada** — cabeie no
  `MIXER` (via `ENVELOPE`).
- **`LO`** / **`CTR`** / **`HI`** — as três bandas. Cabeie em destinos
  **diferentes** (`LO → MIXER`, `HI → SPACE`) e varra `SPREAD`: a
  energia migra entre os caminhos.

## Os controles, um a um

**CUT** (cutoff, 20–20000 Hz) — a frequência de corte. Gire e ouça o
timbre escurecer. A CV `FC` soma aqui.

**RESO** (resonance, 0–1) — realça as frequências perto do corte. Alto
empurra pra auto-oscilação — o filtro passa a soar como um oscilador.

**SPREAD** (0–1) — afasta as três saídas em frequência. 0 = as três
iguais (um filtro só); 1 = três bem separados (uma resposta multimodo /
formante).

**DRIVE** (0–1) — ganho de entrada — satura o sinal antes de filtrar.
Com `CUT` baixo, o grave engorda antes de ser cortado.

## Como cabear

**Voz subtrativa:**
```
OSC (SAW) → FILTER (IN)
FILTER (ALL) → ENVELOPE (IN) → MIXER (ch1)
ENVELOPE (env) → FILTER (FC)      (o corte abre com a nota)
```

**Auto-oscilando como voz:** `RESO` perto de 1, `CUT` numa nota grave,
nada no `IN` — `ALL` é uma senoide afinada. Cabeie `QUANTIZER.pitch` no
`FC` (1 V/oct) e toque melodias com o filtro.

## Potencializar

- **Formante móvel:** `LO` e `HI` em dois canais do `MIXER` com pans
  opostos, um LFO lento no `SPR` — o timbre "abre" no palco.
- **Filtro que canta:** `RESO` alto, `ENVELOPE` no `FC`, um `NOISE.pink`
  no `IN` — cada nota é um "pio" ressonante com cauda de ruído.
- **Ducking:** `ENVELOPE` invertido (via `CONTROL` scale negativo) no
  `FC` — o filtro *fecha* na batida.
- **Cruze com o `WASP`:** os mesmos gestos, mas o `WASP` distorce quando
  ressoa — troque um pelo outro pra sujar.

## Se você conhece o Eurorack

Faz o papel de um VCF de estado variável (Doepfer A-121, Intellijel
Ripples). A ideia das **três saídas na mesma frequência + `spread`** vem
do Mannequins Three Sisters — a relação entre as saídas como gesto. O
parente sujo é o `WASP` (#32).
