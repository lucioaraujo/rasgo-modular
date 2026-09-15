# NOTE-OUT — detector de nota

**Família:** OUT · **Módulo 38**
**Essência:** observa uma voz (gate + altura) e **registra cada nota
completa** na partitura — sem mudar o som. A contraparte de saída do
`SIGNAL-IN`.
**Dossiê técnico:** [`../dossies/38_note_out.md`](../dossies/38_note_out.md)
· **Fonte:** `src/dsp/NoteOut.hpp`

---

## A ideia

O `SIGNAL-IN` deixa alguém de fora tocar o Rasgo. O `NOTE-OUT` é o
observador do outro lado: você o põe no caminho de uma voz, e ele
**captura** cada nota (a altura no instante em que o gate liga, a
duração até o gate desligar, a velocity, o acento) e a escreve na
**MUSICAL SCORE** — o registro de notas da peça. É um adaptador: não tem
knob, não muda o áudio.

## Por dentro

Um detector de borda de gate + amostra de pitch. Quando o `GATE` sobe,
ele lê `PITCH`/`VELOCITY`/`ACCENT`; quando o `GATE` desce, fecha a nota
(altura + duração + dinâmica) e a entrega. O painel drena essas notas
completas (fora do laço de áudio) pra a `MUSICAL SCORE` e pode emiti-las.

**Por que encadear pelas saídas `GATE_THRU`/`PITCH_THRU` é obrigatório,
não só uma boa prática:** o motor só processa os módulos que de fato
**alimentam a saída** — se o `NOTE-OUT` só recebesse `GATE`/`PITCH` sem
que nada dependesse das saídas dele, ele ficaria fora do caminho de
áudio ativo e simplesmente não rodaria (a mesma lógica por trás da
vista "RACK · SAÍDA" do painel: só o que chega ao `MASTER`, transitivamente,
é avaliado). Como `GTHR`/`PTHR` são cópias exatas do que entra, cabear
por elas garante que o resto da voz continua **dependendo** do
`NOTE-OUT` — ele fica "no meio do caminho" de verdade, mesmo não
mudando nada do som.

## Os jacks, um a um

### Entradas (controle)

- **`GATE`** (disparo) — o gate da voz a capturar. A borda de descida
  fecha a nota. **Plugue aqui:** o mesmo gate que dispara o `ENVELOPE`
  da voz (`CLOCK.euclid`, `SEQUENCE.gate`…).
- **`PITCH`** (1 V/oct) — a altura da voz, capturada quando o gate liga.
  **Plugue aqui:** a mesma CV que vai pro `OSC.1V/O`
  (`QUANTIZER.pitch`).
- **`VEL`** (velocity) — a velocity (0–1), capturada no gate. Sem cabo,
  1,0. **Plugue aqui:** `DECISION.x`, um `SEQUENCE` de CV.
- **`ACC`** (accent) — o acento, capturado no gate. **Plugue aqui:**
  `CLOCK.accent`, `TRIGSEQ.accent`.

### Saídas (controle)

- **`GTHR`** (gate_thru) (disparo) — cópia exata de `GATE`. **Cabeie
  daqui pro `ENVELOPE.gate`** (não direto do clock) — senão o `NOTE-OUT`
  fica órfão e nunca roda.
- **`PTHR`** (pitch_thru) (1 V/oct) — cópia exata de `PITCH`. Cabeie
  daqui pro `OSC.1V/O`.

## Como cabear

**Registrar a melodia principal:**
```
QUANTIZER (pitch) → NOTE-OUT (PITCH)
CLOCK (euclid) → NOTE-OUT (GATE)
CLOCK (accent) → NOTE-OUT (ACC)

NOTE-OUT (PTHR) → OSC (1V/O)          (a voz continua igual…)
NOTE-OUT (GTHR) → ENVELOPE (gate)     (…e agora fica registrada na MUSICAL SCORE)
```

O `.score.txt` da gravação (`Ctrl+R`) inclui essas notas — ver
[`CABEAMENTO.md`](CABEAMENTO.md) e o guia de gravação.

## Potencializar

- **Duas vozes, uma partitura:** um `NOTE-OUT` em cada voz — a peça
  gravada tem a linha melódica e o baixo transcritos.
- **Acento como dinâmica escrita:** cabeie `TRIGSEQ.accent` no `ACC` —
  a partitura registra quais notas foram enfatizadas.
- **Não tira o som:** como as saídas são pass-through exato, você pode
  inserir o `NOTE-OUT` em qualquer voz existente sem risco.

## Se você conhece o Eurorack

Faz o papel de um conversor CV→MIDI / gravador de sequência (Expert
Sleepers, Hermod no modo record) — mas focado em **transcrever** a parte
generativa numa partitura legível. É a contraparte de entrada do
`SIGNAL-IN` (#49). Desenho próprio (observador sobre o contrato `NOTE`
da MUSICAL SCORE).
