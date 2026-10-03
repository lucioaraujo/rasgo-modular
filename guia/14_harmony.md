# HARMONY — movimento harmônico

**Família:** DECISION · **Módulo 14**
**Essência:** decide a sequência de centros tonais por onde a melodia
generativa passeia — por movimentos que a tradição nomeou (Coltrane,
substituição tritônica, mediante cromática…).
**Dossiê técnico:** [`../dossies/14_harmony.md`](../dossies/14_harmony.md)
· **Fonte:** `src/dsp/Harmony.hpp`

---

## A ideia

O `QUANTIZER` faz o acaso soar numa tonalidade — mas **fixa**. Música
original raramente fica num tom só; ela **modula**, e não de qualquer
jeito: por movimentos harmônicos com assinatura de intervalo própria. O
`HARMONY` é esse eixo: a cada `ADVANCE`, escolhe um novo centro tonal
segundo a técnica em `MOVE`, e manda `ROOT`/`SCALE` como CV pro
`QUANTIZER`.

## Por dentro

**O que é um "movimento harmônico com assinatura de intervalo", em
uma frase:** mudar de tom não-aleatoriamente — cada técnica move o
centro tonal por uma **distância característica**, repetidas vezes,
criando um percurso reconhecível em vez de um passeio qualquer entre
os 12 tons possíveis. `MOVEMENT` escolhe qual dessas assinaturas usar:

- **Coltrane** — sempre soma **4 semitons** (uma terça maior) a cada
  troca. Como 4 não divide 12 exatamente em passos pequenos, mas 3
  saltos de 4 semitons somam 12 (uma volta completa), o percurso forma
  um **ciclo fechado de 3 tons** igualmente espaçados — a técnica
  famosa de John Coltrane (*Giant Steps*), que soa deliberadamente
  "circular" e simétrica, nunca repousando muito tempo no mesmo lugar.
- **substituição tritônica + ii-V** — uma dupla de técnicas de jazz
  clássicas: "ii-V" é o movimento mais comum de resolução tonal
  (aproximar-se do tom seguinte por um caminho de duas etapas); a
  "substituição tritônica" troca esse caminho por um que está a
  **meia-oitava** (6 semitons) de distância, um efeito de surpresa
  harmônica clássico do jazz.
- **mediante cromática** — troca de tom por uma terça (maior ou menor),
  **sem** seguir a lógica usual de "dominante resolvendo pra tônica" —
  o efeito é uma mudança de cor tonal mais abrupta e cinematográfica
  (comum em trilhas sonoras).
- **intercâmbio modal** — troca **só** a escala (o "sabor" — maior,
  menor, um modo), mantendo a mesma tônica — o centro de gravidade
  continua o mesmo, mas o conjunto de notas disponíveis muda de
  caráter.
- **jazz modal** — o oposto de Coltrane: fica parado na mesma
  tonalidade a maior parte do tempo, com uma troca **rara** —
  imitando o estilo modal (Miles Davis, *Kind of Blue*) de deixar uma
  ideia respirar antes de mudar.
- **backdoor ii-V** — sempre soma **2 semitons**, uma variante de
  resolução menos comum que a ii-V direta.

Cada `ADVANCE` sorteia a **próxima** troca dentro da lógica da técnica
escolhida (não é livre — está restrito às distâncias que aquela técnica
permite). `SCALE_LO`/`SCALE_HI` restringem quais das 12 escalas podem
entrar no sorteio (evita, por exemplo, cair numa escala exótica demais
se você quer só maior/menor). `HOLD` é a probabilidade de o `ADVANCE`
ser **ignorado** — a troca "seria" a hora, mas o módulo decide manter o
centro tonal atual por mais um ciclo, alongando seções sem mudar a
lógica de avanço.

## Os jacks, um a um

### Entradas

- **`ADV`** (advance) (controle, disparo) — avança pro próximo centro
  tonal. Presente, substitui `RATE`. **Plugue aqui:** um `CLOCK` **muito
  lento** (via `LOGIC.div` com N grande) — marca as seções da peça;
  `SEQUENCE.eos` (a harmonia anda a cada volta da frase); `DECISION.gate`
  (troca "às vezes").
- **`RST`** (reset) (controle, disparo) — volta ao `ROOT`/escala
  inicial. **Plugue aqui:** um transporte, um `CLOCK` de compasso longo.

### Saídas

- **`ROOT`** (áudio — CV) — a nova fundamental (semitom/12). **Ligue
  em:** `QUANTIZER.ROOT` (pelo cabo).
- **`SCALE`** (áudio — CV) — o novo índice de escala (índice/11).
  **Ligue em:** `QUANTIZER.SCALE`.
- **`CHG`** (change) (controle, gate) — um pulso (~20 ms) toda vez que
  `ROOT` ou `SCALE` mudam. **Plugue em:** `DRIFT.advance` (a estrutura
  deriva a cada mudança de tom), `TRIGSEQ.fill` (uma virada na
  modulação), `DECISION.trig`.

## Os controles, um a um

**MOVE** (movement, 0–5) — a técnica de movimento harmônico. Coltrane
anda rápido e cíclico; jazz modal fica quase parado.

**RATE** (0,005–2 Hz) — o relógio interno. Usado só se `ADV` estiver
livre. Escala de dezenas de segundos a minutos.

**ROOT** (root_start, 0–11) — a nota fundamental no reset/início.

**S-LO** / **S-HI** (scale_lo/hi, 1–10) — o menor e o maior índice de
escala sorteável nas trocas.

**HOLD** (0–1) — a probabilidade de pular uma troca agendada,
mantendo o centro tonal.

## Como cabear

**Harmonia que se move sob a melodia:**
```
LOGIC (div, N alto) → HARMONY (ADV)     ou deixe o RATE interno bem lento
HARMONY (root)  → QUANTIZER (entrada ROOT, v0.1.3)
HARMONY (scale) → QUANTIZER (entrada SCL, v0.1.3)
TURING → QUANTIZER (cv) → OSC (1V/O)
```

## Potencializar

- **Forma:** `HARMONY.change → DRIFT.advance` — cada modulação de tom
  também dá um passo na estrutura do patch; a peça "vira a página"
  junto.
- **Progressão a cada frase:** `SEQUENCE.eos → ADV` — a harmonia anda
  exatamente quando o riff reinicia (sensação de "compasso").
- **Coltrane no clímax:** automatize o `MOVE` — jazz modal (parado) nas
  seções calmas, Coltrane (cíclico rápido) no ápice.
- **Acorde seguindo o tom:** `HARMONY.root` também num `CHORD.CHRD` (via
  `CONTROL`) — o acorde troca de formato com a modulação.

## Se você conhece o Eurorack

Não há equivalente direto — é o `HarmonicWanderer` do RASGO_SYNTH
portado (só a lógica de intervalos, que é fato musical). O mais próximo
seria um sequenciador de acordes (Sinfonion) tocado por probabilidade.
Distinto do `QUANTIZER` (tonalidade fixa) — o `HARMONY` é o que a move.
