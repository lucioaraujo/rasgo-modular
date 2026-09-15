# QUANTIZER — quantizador de escala

**Família:** DECISION · **Módulo 12**
**Essência:** o que faz o acaso soar **numa tonalidade** — pega uma CV
contínua e prende às notas de uma escala. O elo entre "gerar variedade"
e "gerar música".
**Dossiê técnico:** [`../dossies/12_quantizer.md`](../dossies/12_quantizer.md)
· **Fonte:** `src/dsp/Quantizer.hpp`

---

## A ideia

O `TURING` e o `DECISION` produzem CV em degraus — mas degraus iguais em
tensão não são degraus iguais em música. Sem um quantizador, a "melodia"
generativa é atonal por acidente. O `QUANTIZER` prende qualquer CV às
notas de uma das 12 escalas curadas — e, com `ROOT`/`SCALE` modulados
(pelo `HARMONY`), é o que faz a peça ter harmonia que se move.

## Por dentro

**O que "quantizar" significa, literalmente:** uma CV contínua pode
assumir **qualquer** valor — inclusive os que caem "entre" duas notas
(o equivalente a tocar levemente desafinado, nem sustenido nem
natural). Quantizar é **arredondar** esse valor pro grau **mais
próximo** que de fato pertence à escala escolhida (`SCALE`) na tônica
escolhida (`ROOT`) — não importa quão "torto" o valor de entrada seja,
a saída **sempre** cai exatamente numa nota válida daquela escala.
É esse arredondamento que transforma qualquer fonte de CV — mesmo uma
completamente aleatória — em algo que soa "correto" musicalmente.

**Por que existe uma zona-morta (`HYSTERESIS`) em vez de arredondar
sempre pro mais próximo, sem mais:** se uma CV instável (com um
tremor pequeno, de ruído ou de imprecisão) estiver bem em cima da
fronteira entre duas notas vizinhas, um arredondamento ingênuo faria a
saída **tremular** entre as duas a cada minúscula oscilação — audível
como um trinado indesejado. `HYSTERESIS` exige que a CV se afaste **um
pouco mais** da nota atual antes de trocar pra vizinha (a fronteira de
saída de uma nota fica um pouco além da fronteira de entrada) — um
pequeno tremor perto do limite já não é mais suficiente pra cruzar
essa margem extra, e a nota permanece estável.

**Por que `TRIGGER` importa pra virar "melodia ritmada":** sem um pulso
conectado, a saída acompanha a CV de entrada **em tempo real,
continuamente** — qualquer mudança na fonte (o `TURING` deslocando, o
`DECISION` sorteando de novo) já aparece imediatamente na saída,
inclusive fora de tempo com o resto do patch. Conectar um `TRIGGER`
transforma isso num sample-and-hold: a nota só é **lida e atualizada**
no instante do pulso, e fica parada até o próximo — assim, mudanças de
altura só acontecem exatamente no tempo do clock, dando ritmo a uma
fonte de CV que sozinha não teria nenhum.

## Os jacks, um a um

### Entradas

- **`CV`** (áudio — CV) — a CV contínua a quantizar. **Plugue aqui:**
  `TURING.cv`, `DECISION.x`, `NOISE.sh`, um LFO, `CHAOS.out`.
- **`TRSP`** (transpose) (controle, 1 V/oct) — CV que soma **antes** de
  quantizar — transpõe a escala inteira. **Plugue aqui:** `HARMONY` não
  (esse vai no `ROOT`); um `SEQUENCE` de CV pra mudar de oitava, um
  pedal.
- **`TRIG`** (trigger) (controle, disparo) — S&H: a nota só atualiza no
  pulso. **Plugue aqui:** `CLOCK.euclid` — alinha a melodia ao ritmo.

### Saídas

- **`PTCH`** (pitch) (áudio — CV, 1 V/oct) — a CV quantizada, pronta pra
  `OSC.1V/O`, `MATTER.1V/O`, qualquer entrada de altura.
- **`GATE`** (controle, gate) — um pulso (~5 ms) toda vez que a nota
  **muda de verdade**. **Plugue em:** `ENVELOPE.gate` — só rearticula
  quando a altura muda (legato natural).
- **`ST`** (semitone) (áudio — CV) — a mesma nota normalizada pra ±1 em
  vez de oitavas. Útil como CV de controle (não de altura).

## Os controles, um a um

**SCALE** (0–11) — a escala: cromática, maior, os modos, pentatônicas,
tons inteiros, oitava. A CV do `HARMONY.scale` entra aqui (via cabo).

**ROOT** (0–11) — a tônica (0 = C … 11 = B). A CV do `HARMONY.root`
entra aqui.

**RANGE** (1–6 oitavas) — quantas oitavas a CV de entrada cobre antes de
quantizar.

**GLIDE** (0–1) — portamento entre uma nota e a próxima.

**HYST** (hysteresis, 0–1) — a zona-morta antes de trocar de grau —
evita tremular com CV instável.

## Como cabear

**A melodia generativa (o uso central):**
```
CLOCK (euclid) → QUANTIZER (TRIG)
TURING (cv) → QUANTIZER (CV)
QUANTIZER (pitch) → OSC (1V/O)
QUANTIZER (gate) → ENVELOPE (gate)
```

**Com harmonia que se move:**
```
HARMONY (root)  → QUANTIZER (ROOT via cabo)
HARMONY (scale) → QUANTIZER (SCALE via cabo)
CLOCK (bem lento) → HARMONY (advance)
```

## Potencializar

- **Legato de verdade:** use o `QUANTIZER.gate` (dispara só na troca de
  nota) em vez do `CLOCK` no `ENVELOPE` — notas repetidas ficam ligadas.
- **Pentatônica pra segurança:** `SCALE` numa pentatônica — qualquer
  acaso soa "certo".
- **Transposição por seção:** um `SEQUENCE` de CV bem lento no `TRSP` —
  a melodia sobe/desce de oitava a cada N compassos.
- **Duas vozes, uma escala:** o mesmo `HARMONY` em dois `QUANTIZER`, com
  CVs diferentes no `CV` — as duas vozes trocam de tom juntas.

## Se você conhece o Eurorack

Faz o papel de um quantizador (Doepfer A-156, uO_C, Scales). A base são
as tabelas de escala do `RASGO_SYNTH/Scales.hpp` (só intervalos — fato
musical) e a histerese de um theremin digital. O par que move a
tonalidade é o `HARMONY` (#14).
