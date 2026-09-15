# VOCODER — vocoder de N bandas

**Família:** TRANSFORM · **Módulo 60**
**Essência:** a energia de uma voz por banda de frequência controla a
mesma banda numa portadora rica — a portadora "fala". 4 a 20 bandas.
**Dossiê técnico:** [`../dossies/60_vocoder.md`](../dossies/60_vocoder.md)
· **Fonte:** `src/dsp/Vocoder.hpp`

---

## A ideia

O vocoder é um dos efeitos mais reconhecíveis da música eletrônica
(Kraftwerk, Stevie Wonder, Daft Punk, ELO). O `FORMANT` tem um modo
vocoder de 5 bandas — bom pra o "coro que fala vogais", ruim pra fala
inteligível. O `VOCODER` cobre o caso **banda larga**: 16–20 bandas =
fala clara; 6 = o vocoder dos anos 70.

## Por dentro

**A ideia central de um vocoder, em uma frase:** pegar a **forma
espectral** de um som (quais frequências estão fortes, agora — a
"assinatura" que faz uma vogal soar como aquela vogal) e **impor** essa
forma em cima de outro som completamente diferente. É "vestir" o som B
com a forma do som A.

**Como isso é feito, passo a passo (Homer Dudley, 1938):** o
**modulador** (a voz/fala) passa por um banco de `BANDS` filtros
passa-faixa, cada um cobrindo uma fatia de frequência — e, pra cada
banda, um **seguidor de envelope** mede continuamente "quanta energia
tem aqui agora" (sobe rápido quando a energia chega, desce mais devagar
quando ela vai embora — os tempos são `ATTACK`/`RELEASE`). Isso dá uma
"foto contínua" da forma espectral do modulador, banda a banda. A
**portadora** (o som rico que vai "falar") passa pelo **mesmo** banco de
filtros — e cada banda da portadora é **multiplicada** pela medida de
energia correspondente do modulador: onde o modulador tinha energia
forte naquela faixa, a portadora passa forte ali também; onde não
tinha, a portadora é abafada ali. O resultado tem o **timbre/textura**
da portadora, mas a **forma espectral** (a "fala") do modulador.

**Por que `SIBILANCE` existe:** consoantes como "s", "f", "ch"
(fricativas) concentram energia em frequências **bem altas** e sem
estrutura tonal — um banco de filtros de banda larga (poucas bandas,
cada uma cobrindo uma faixa generosa) não captura bem esses sons
rápidos e agudos, então sem ajuda a fala vocodada perde inteligibilidade
justamente nas consoantes. `SIBILANCE` deixa o agudo do modulador (acima
de ~3,5 kHz) passar **direto** pra saída, sem passar pelo processo de
banda — devolvendo essas consoantes que a análise por banda deixaria
escapar.

`ATTACK`/`RELEASE` controlam **quão rápido** o seguidor de envelope
reage: `ATTACK` curto captura o início abrupto de uma consoante (mais
"cortado", mais inteligível); `RELEASE` curto deixa a energia cair
rápido entre sílabas (mais *staccato*); `RELEASE` longo faz a energia
"escorrer" de uma sílaba pra próxima, borrando a fala num pad contínuo.
`FREEZE` simplesmente **para de atualizar** essas medidas — as bandas
ficam paradas na última "foto" da forma espectral, e a portadora
continua tocando aquele instante congelado indefinidamente (a mesma
ideia do `FREEZE` do `SPECTRA`, #57, aplicada por banda em vez de por
parcial).

## Os jacks, um a um

### Entradas

- **`CAR`** (áudio) — a **portadora**: o que vai "falar". **Plugue
  aqui:** `OSC.saw`, `CHORD`, um pad (`HALL`/`ADDITIVE`), `NOISE`.
  Livre → uma serra interna afinável por `PIT`.
- **`MOD`** (áudio) — o **modulador**: a voz/fala (ou `DRUM`, ou
  qualquer som rítmico) cuja envoltória espectral molda a portadora.
  **Plugue aqui:** `SIGNAL-IN`, um `SAMPLER`, um `DRUM`.
- **`PIT`** (controle, 1 V/oct) — CV pra a serra interna (só quando
  `CAR` está livre). **Plugue aqui:** `SEQUENCE.pitch` — você toca a
  altura da voz robô.

### Saída

- **`OUT`** (áudio) — a portadora moldada pelas bandas do modulador +
  sibilância, com *softclip*. Vai ao `MIXER`.

## Os controles, um a um

**BANDS** (4–20) — quantas bandas de frequência. Poucas = "robô" grosso;
muitas (16–20) = fala inteligível.

**SHIFT** (−1..1) — desloca as frequências da **síntese** em relação à
análise (*formant shift*): pra cima = voz pequena; pra baixo = voz
grande — sem mudar a fala.

**ATTACK** (0–1, ~1–60 ms) — o ataque dos seguidores de envelope. Curto
= as consoantes cortam secas; longo = tudo amolece.

**RELEASE** (0–1, ~20–600 ms) — curto = *staccato* / inteligível; longo
= as sílabas borram (pad falado).

**SIBILANCE** (0–1) — quanto do agudo do modulador passa direto. 0 =
ceceio; alto = fala nítida.

**FREEZE** (0–1) — congela as envoltórias das bandas — a portadora fica
"falando a última sílaba" para sempre. Tire o `MOD` depois de congelar e
o pad segura.

**MIX** (0–1) — portadora seca ↔ vocodada. 0 = bypass.

## Como cabear

**Voz robô clássica:**
```
OSC (SAW) → VOCODER (CAR)
SIGNAL-IN (L) → VOCODER (MOD)      BANDS = 16
VOCODER (OUT) → MIXER (ch1)
```

**Pad que fala:**
```
HALL (out) → VOCODER (CAR)
SAMPLER (voz) → VOCODER (MOD)      RELEASE alto
```

**Percussão tonal:** `DRUM → VOCODER.MOD`, serra no `CAR` — cada batida
"abre" a portadora no seu espectro → um pad ritmado pela bateria.

## Potencializar

- **Congelar uma palavra:** `FREEZE` no pico de uma sílaba → um drone
  com aquela cor de vogal; some com o vivo depois.
- **Robô que toca melodia:** `CAR` livre, `SEQUENCE.pitch → PIT`,
  `SIGNAL-IN` no `MOD` — você fala o ritmo e o sequenciador dá as notas.
- **Sibilância como gesto:** um `ENVELOPE` no `SIBILANCE` (via
  `CONTROL`) — as fricativas entram e saem.
- **Cruze com o `SHIFTER`:** `VOCODER → SHIFTER` — a voz robô metaliza.

## Se você conhece o Eurorack

Faz o papel de um vocoder de rack (o modo do Frap Fumana em banda larga,
Bastl, mutantes de vocoder). A base é Dudley (1938, domínio público),
o banco de filtros de Q constante + seguidor RC, e o *voiced/unvoiced
passthrough* (EMS 5000 / Roland VP-330). Distinto do modo vocoder do
`FORMANT` (5 bandas de vogal) e do `SPECTRA` (re-síntese que segue a
altura).
