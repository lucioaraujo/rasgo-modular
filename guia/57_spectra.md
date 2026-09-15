# SPECTRA — resíntese espectral

**Família:** SOURCE · **Módulo 57**
**Essência:** ouve um som, acha os parciais mais fortes e re-oscila como
um banco de senoides que **segue** o som. A ponte análise → síntese.
**Dossiê técnico:** [`../dossies/57_spectra.md`](../dossies/57_spectra.md)
· **Fonte:** `src/dsp/Spectra.hpp`

---

## A ideia

O Rasgo tem muitas formas de **fazer** espectro (`OSC`, `ADDITIVE`,
`OPERATOR`, `WAVETABLE`, `PULSAR`) e de **medir** som (`SCOPE` → CV). Não
tinha como usar **o espectro de um som como partitura de outro**. É a
técnica por trás de meia dúzia dos módulos mais cobiçados fora do Rasgo
(Panharmonium, Rainmaker espectral, Clouds spectral).

O `SPECTRA` fica em SOURCE porque a saída é um banco de osciladores —
mas quase sempre tem algo no `IN`: uma voz, um `DRUM`, uma gravação, o
`SIGNAL-IN`. Ele "toca de novo" o que ouviu, e você transpõe, estica,
congela e borra essa re-execução.

## Por dentro

**Análise, o que significa de fato:** 64 filtros passa-faixa (cada um
deixa passar só uma fatia estreita de frequência, espalhados em log de
35 Hz a 14 kHz — mais densos nos graves, como o ouvido) escutam o `IN`
ao mesmo tempo; um seguidor de pico mede **quanta energia** passa por
cada um. A cada ~6 ms, o módulo olha esse retrato e escolhe os
`VOICES` filtros com **mais energia** naquele instante — essas são as
frequências que, no som que entrou, mais se destacam agora (os
"parciais mais fortes").

**Síntese, o que significa de fato:** em vez de tocar o som analisado
de volta, o `SPECTRA` **recria** cada um desses parciais com uma
senoide própria, gerada do zero — daí "re-síntese": o som que sai não é
uma cópia, é uma nova voz **imitando** o espectro que a análise achou.
Cada senoide é **de fase contínua**: quando o parcial que ela está
seguindo muda de frequência (a cada novo retrato de análise, ~6 ms), a
senoide **desliza** suavemente até a frequência nova em vez de saltar —
sem esse deslize, cada atualização criaria um clique audível (o
"zíper" clássico de re-síntese espectral malfeita, uma sucessão de
saltos abruptos que soa como um ruído metálico granulado). `BLUR`
controla **a velocidade** desse deslize: em 0, a senoide já chega quase
instantaneamente no parcial novo (segue o som de perto, inclusive
transientes); em 1, o deslize demora ~0,6 s (a re-síntese "arrasta"
atrás do som real, criando um rastro/coro fantasma — o som "derrete").

`SHIFT`/`STRETCH` mexem **só na síntese**, depois da análise já ter
achado os parciais — por isso não afetam o que é detectado, só onde as
senoides recriadas tocam. `STRETCH` empurra os parciais **pra longe uns
dos outros** (ou pra perto) em torno do centro: como os parciais
deixam de estar em razões inteiras exatas entre si, o resultado ganha o
mesmo caráter inarmônico de um sino/metal (a mesma ideia do
`structure` do `MATTER`, #9) — sem mudar a nota fundamental percebida.

`FREEZE` simplesmente **para de atualizar** o retrato da análise — as
senoides continuam tocando pra sempre o último conjunto de parciais que
foi capturado, um instante do som virando um pad sustentado
indefinidamente.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio) — o som a **analisar**. **Plugue aqui:** uma voz
  (`OSC`, `CHORD`, `MATTER`), um `DRUM`, um `SAMPLER`, `SIGNAL-IN`.
  Sem cabo, um ruído interno + 2 parciais fantasma viram um drone
  autônomo que respira.
- **`PIT`** (controle, altura) — 1 V/oct somada a `SHIFT` — transpõe a
  re-síntese. **Plugue aqui:** um LFO (o espectro sobe e desce inteiro),
  `SEQUENCE.pitch` (tocar a re-síntese numa melodia), `QUANTIZER`.
- **`FRZ`** (controle, gate) — congela a análise enquanto o gate está
  alto (ou use a chave `FREEZE`). **Plugue aqui:** `CLOCK.euclid`,
  `DECISION.gate`, um pedal via `SIGNAL-IN.CC` — o espectro alterna
  entre congelado e vivo.

### Saídas

- **`L`** / **`R`** (áudio) — vozes ímpares e pares panoramizadas. Vão
  para dois canais do `MIXER`, ou `L` para um filtro e `R` para um
  espaço.

## Os controles, um a um

**VOICES** (2–24) — quantas senoides o banco usa. Poucas = caricatura do
espectro; muitas = re-síntese fiel (quase um clone).

**BLUR** (0–1) — quão rápido cada voz persegue o parcial que herdou. 0 =
trava no som (transientes passam); 1 = arrasta ~0,6 s (borra, coro
fantasma, o som "derrete").

**SHIFT** (−2..2 oitavas) — transpõe a re-síntese sem tocar na análise —
*pitch-shift* de espectro, formante junto. A entrada `PIT` soma aqui.

**STRETCH** (−1..1) — afasta / junta os parciais em torno do meio →
inarmônico (sino, metal) sem mudar a altura percebida.

**TONE** (−1..1) — a inclinação espectral da saída: <0 abafa os agudos
das vozes, >0 realça. 0 = fiel à análise.

**JITTER** (0–1) — *wobble* lento e semeado na frequência de cada voz
(±3%) — a re-síntese respira, nunca idêntica. Determinístico. 0 =
estático.

**FREEZE** (chave / gate `FRZ`) — para a análise, o banco continua
oscilando no último espectro. Pad infinito de qualquer instante.

**MIX** (0–1) — seco (o `IN` cru) ↔ re-síntese. Em 1 (padrão) você ouve
só a re-síntese.

## Como cabear

**Sombra harmônica de uma voz:**
```
OSC (SAW) → SPECTRA (IN)
SPECTRA (L) → MIXER (ch2)     (VOICES baixo + STRETCH = uma sombra inarmônica da melodia)
```

**Pad de qualquer coisa:**
```
CHORD (OUT) → SPECTRA (IN)
CLOCK (euclid) → SPECTRA (FRZ)     (o acorde congela e descongela no compasso)
SPECTRA (L)/(R) → MIXER (ch1)/(ch2)
```

## Potencializar

- **Coro que derrete:** `BLUR` alto num vocal ou pad no `IN` — as vozes
  se arrastam entre os parciais e formam um halo.
- **Espectro tocável:** `SEQUENCE.pitch → PIT` com `MIX` = 1 — você toca
  uma melodia *feita do espectro* de outra fonte.
- **Ritmo virando sino:** `DRUM → IN`, `VOICES` baixo, `STRETCH` alto —
  uma sombra sineira do ritmo.
- **Autônomo como leito:** sem `IN`, `JITTER` pequeno, `TONE` levemente
  negativo — um drone que respira, bom fundo pra um patch de seed.

## Se você conhece o Eurorack

Faz o papel do Rossum Panharmonium (conceito, não código). A base é o
*phase vocoder* (Flanagan & Golden 1966), o modelo sinusoidal
McAulay–Quatieri (1986) e a SMS de Xavier Serra (1989). Distinto do
`ADDITIVE` (constrói o espectro do zero) e do `MEMORY` (grão no tempo,
não no espectro).
