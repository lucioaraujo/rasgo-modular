# WASP — filtro com grão

**Família:** TRANSFORM · **Módulo 32**
**Essência:** um filtro de 12 dB que distorce quando ressoa — inversores
CMOS que ceifam duro e assimétrico. O contraponto sujo do `FILTER`.
**Dossiê técnico:** [`../dossies/32_wasp.md`](../dossies/32_wasp.md)
· **Fonte:** `src/dsp/Wasp.hpp`

---

## A ideia

Nem todo filtro deve ser transparente. O EDP Wasp (1978) é procurado
pelo que faz de "errado": distorce ao ressoar, a auto-oscilação range,
empurrar o `drive` transforma o filtro num waveshaper com corte. É a
peça pra baixo agressivo, lead que corta, drone que range.

## Por dentro

O núcleo é um SVF de 2 polos, igual ao `FILTER` (#2) — a mesma ideia de
"filtro que calcula grave/faixa/agudo do mesmo estado". A diferença é o
que acontece **dentro do laço de ressonância**: em vez de um estágio de
ganho limpo (um amplificador comum), o circuito original usava um
**inversor CMOS** — um chip de lógica digital, feito pra ligar/desligar,
não pra amplificar com fidelidade — como estágio de ganho. Forçado a
operar fora do seu uso normal, esse chip tem uma curva **muito mais
abrupta** que um amplificador de verdade: some suave até certo ponto e
depois "vira a chave" quase de uma vez (um ceifador agressivo, não um
achatamento suave como o `tanh`). O `WASP` modela essa curva.

**Por que isso faz o filtro distorcer justamente quando ressoa:** a
ressonância já é, por natureza, um sinal circulando dentro de um laço
de realimentação (ver "Por dentro" do `FILTER`). Se esse laço passa por
um estágio **limpo**, a auto-oscilação sai como uma senoide pura (o que
o `FILTER` faz). Se o laço passa pelo estágio **ceifador agressivo**
descrito acima, cada volta da ressonância já sai um pouco distorcida —
então quanto mais perto de auto-oscilar, mais "sujo"/"palhetado" o som
fica, em vez de convergir pra uma senoide limpa. `GRIT` é literalmente
o **joelho** dessa curva — baixo, o ceifador só age em picos bem altos
(quase transparente); alto, ele corta cedo e duro, então mesmo sinais
moderados já saem rangendo.

Depois do filtro, um **segundo** estágio de saída ceifa de novo (o
*buzz* de palheta característico do Wasp). `BIAS` desloca esse teto de
corte de um lado mais que do outro (**assimétrico**) — cortar de forma
assimétrica quebra a simetria que normalmente cancela os harmônicos
**pares** de uma onda (a mesma lógica do `ODD` do `ADDITIVE`, #42, só
que aqui é consequência do corte, não escolhida diretamente) — daí
"harmônicos pares + bloqueio de DC" (o deslocamento constante que a
assimetria introduzia é removido à parte).

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o sinal a filtrar. **Plugue aqui:** `OSC`, `NOISE`,
  qualquer voz.
- **`FC`** (controle, 1 V/oct) — soma ao knob `CUT`. **Plugue aqui:**
  `ENVELOPE.env` (envelope de filtro, mas com grão), um LFO.
- **`Q`** (controle) — soma ao knob `RESO`.

### Saída

- **`OUT`** (áudio) — o sinal filtrado e sujo. Vai ao `MIXER` (via
  `ENVELOPE`/`VCA`).

## Os controles, um a um

**CUT** (cutoff, 20–24000 Hz) — a frequência de corte.

**RESO** (resonance, 0–1) — perto de 1 auto-oscila, mais "palhetado" que
o `FILTER`.

**MODE** (0–1) — LP ↔ BP ↔ HP.

**DRIVE** (0,1–8) — ganho de entrada; empurra o sinal pro ceifador
**antes** do filtro.

**GRIT** (0–1) — o joelho do ceifador no laço. Baixo = quase um filtro
normal; alto = range, distorce ao ressoar.

**BIAS** (−1..1) — teto assimétrico → harmônicos pares (mais *buzz*) +
bloqueio de DC.

**DRIFT** (0–1) — passeio lento e correlacionado no corte/ressonância.

## Como cabear

**Baixo acid:**
```
SEQUENCE → QUANTIZER → OSC (1V/O)
OSC (SAW) → WASP (IN) → ENVELOPE (IN) → MIXER (ch1)
ENVELOPE (env) → WASP (FC)
```
`GRIT` médio, `DRIVE` ~2, `RESO` ~0,6.

**Voz percussiva afinada:** `NOISE.pink → WASP (IN)`, `RESO` alto,
`FC` = `QUANTIZER.pitch` — o ciclo-limite sujo toca notas.

## Potencializar

- **Range controlado:** `ENVELOPE` no `Q` além do `FC` — o filtro só
  distorce no pico de cada nota.
- **Drone que respira sujo:** `DRIFT` alto + `RESO` perto de 1, nada no
  `IN` — a auto-oscilação passeia e range sozinha.
- **Antes e depois:** um sinal já filtrado pelo `FILTER` limpo passando
  pelo `WASP` só pra o estágio de saída sujar.

## Se você conhece o Eurorack

É o EDP Wasp filter — modelado de análises independentes do circuito
(René Schmitz, DIY), não do manual da Doepfer. O parente é o Schlappi
100 Grit. Distinto do `FILTER` (limpo, gestual, 3 saídas) e do `SHAPE`
(distorção sem filtro).
