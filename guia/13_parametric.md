# PARAMETRIC — EQ paramétrico de 4 estágios

**Família:** TRANSFORM · **Módulo 13**
**Essência:** a ferramenta cirúrgica — quatro estágios de EQ (corte,
prateleira, realce) com fórmulas RBJ, pra precisão em vez de gesto.
**Dossiê técnico:** [`../dossies/13_parametric.md`](../dossies/13_parametric.md)
· **Fonte:** `src/dsp/Parametric.hpp`

---

## A ideia

O `FILTER` é pra *tocar* — auto-oscila, tem `spread`, é expressivo. Às
vezes o patch só precisa de **precisão**: tirar 3 dB numa ressonância
chata, dar um *shelf* de brilho, cortar o sub. Isso é outro instrumento
— o EQ de engenharia. Ter os dois cobre o "esculpir frequência" no
Rasgo.

E tem um gesto próprio: o `SWEEP` desloca os quatro estágios **juntos**,
como grupo — o EQ inteiro varre o espectro mantendo a forma.

## Por dentro

Quatro biquads (um bloco de filtro digital simples, com fórmulas
públicas bem estabelecidas — o *RBJ Cookbook*) em série, cada um
ajustável a um de 5 comportamentos diferentes:

- **corte** (grave ou agudo) — remove **tudo** abaixo (ou acima) de uma
  frequência, cada vez mais forte quanto mais longe dela. Não tem
  ganho ajustável — é "corta" ou "não corta", só a inclinação
  (`SLOPE`) muda quão abrupto é o corte.
- **prateleira** (*shelf*, grave ou aguda) — sobe ou desce **tudo**
  abaixo (ou acima) de uma frequência por uma quantidade **fixa** em
  dB — como inclinar um degrau: tudo naquele lado fica igualmente mais
  alto ou mais baixo, não é um pico pontual.
- **realce** (*peak*/bell) — sobe ou desce só uma **faixa estreita**
  ao redor de uma frequência específica, formando um "morro" ou um
  "vale" no espectro — é o único tipo pensado pra mirar numa frequência
  exata (tirar uma ressonância chata, por exemplo).

`Q` decide a **largura** desse morro/vale no realce (Q alto = pico
estreito e cirúrgico; Q baixo = uma região larga e suave) — a mesma
ideia de largura de banda do `RES` do `FORMANT`/`VOCODER`. `SLOPE`
decide **quantos** biquads em cascata formam um corte — cada biquad
adicional em série dobra a inclinação (12 dB/oitava com um só, 24 com
dois, 48 com quatro): mais biquads = corte mais abrupto, mais parecido
com uma "parede" do que com uma rampa suave.

`SWEEP`/`SWP` move as **quatro** frequências centrais juntas, na mesma
proporção (some em 1 V/oct — ver `CABEAMENTO.md` §4) — o formato
relativo entre os 4 estágios (as distâncias entre eles) se mantém, só a
posição do conjunto no espectro muda.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a equalizar. **Plugue aqui:** o fim de uma
  cadeia de voz, ou a saída do `MIXER` pra um EQ de barramento.
- **`SWP`** (controle, 1 V/oct) — desloca os **4 estágios juntos**.
  **Plugue aqui:** um `FUNCTION` bem lento (um "movimento" espectral de
  seção), um `ENVELOPE`.
- **`AMT`** (controle) — CV que escala a ação do EQ. **Plugue aqui:** um
  `ENVELOPE` — o EQ "abre" na nota.

### Saída

- **`OUT`** (áudio) — os 4 estágios em série, com `DRIVE`/`MIX`/`OUTPUT`
  aplicados. Vai ao `MIXER` ou ao `SPACE`.

## Os controles, um a um

**TYPE 1–4** (0–5) — o tipo de cada estágio: desligado · corte grave ·
*shelf* grave · realce (*peak*) · *shelf* agudo · corte agudo.

**FREQ 1–4** (20–20000 Hz) — a frequência central de cada estágio.

**GAIN 1–4** (±24 dB) — o ganho. Só afeta *shelves* e realce; cortes não
têm ganho.

**Q 1–4** (0,1–10) — a largura no realce; o "slope S" nas prateleiras.

**SLOPE 1–4** (12/24/48 dB/oitava) — a inclinação do corte. Só afeta os
tipos de corte.

**OUTPUT** (±24 dB) — ganho de saída, depois dos 4 estágios.

**DRIVE** (0–1) — satura suave a saída, antes do *soft-clip* de segurança.

**MIX** (0–1) — seco ↔ processado (EQ paralelo).

## Como cabear

**EQ cirúrgico no fim da voz:**
```
MATTER (OUT) → PARAMETRIC (IN) → SPACE (in) → MIXER (ch1)
```
Estágio 1 = corte grave em 40 Hz; estágio 2 = realce −4 dB numa
ressonância chata; estágio 4 = *shelf* agudo +2 dB.

**Movimento de seção:**
```
FUNCTION (bem lento) → PARAMETRIC (SWP)
```
O espectro inteiro sobe e desce ao longo de um minuto, mantendo a forma.

## Potencializar

- **Filtro em pente barato:** quatro realces (`TYPE` 3) em frequências
  regularmente espaçadas, `Q` alto — um efeito de *comb* estático.
- **Wah de grupo:** `SWEEP` de um LFO médio + `Q` altos — os 4 picos
  varrem juntos.
- **EQ que abre na nota:** `ENVELOPE.env → AMT`.
- **EQ de barramento:** na soma final (`MIXER.L+R → PARAMETRIC.IN →
  MASTER.IN`) — brilho e corpo do patch inteiro.

## Se você conhece o Eurorack

Faz o papel de um EQ paramétrico (VCV Parametra). A base são as fórmulas
do *RBJ Audio EQ Cookbook* (públicas). Distinto do `FILTER` (gestual,
auto-oscila), do `FORMANT` (banco de formantes varrido) e do `WASP`
(sujo).
