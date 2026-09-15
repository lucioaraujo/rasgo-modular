# Guia do Rasgo Modular

Um guia **para quem toca** o Rasgo Modular. O objetivo não é só listar
botões — é ajudar você a **pensar** o instrumento: como ter uma ideia
sonora, como cabear pra ela acontecer, e como crescer essa ideia até
virar uma peça.

Pra isso o guia tem quatro partes: a **mentalidade** (como este
instrumento quer ser tocado), o **funcionamento** (como o todo se
encaixa e como os cabos trabalham), os **módulos um a um** (o que cada
um faz, cada jack, como cabear), e as **receitas** (montagens completas
que você refaz e modifica).

É a base do site (multilíngue, futuro) e de uma publicação em PDF.
**Só português por enquanto** — a estrutura já está pronta pra tradução.

Onde os outros documentos moram: arquitetura e decisões em
[`../RASGO_MODULAR.md`](../RASGO_MODULAR.md); um **dossiê técnico** por
módulo (problema, fontes, modelo matemático, testes) em
[`../dossies/`](../dossies/00_indice.md), com o mesmo número de cada
página deste guia.

---

## Parte 1 — Como pensar o instrumento

- [`COMO_PENSAR.md`](COMO_PENSAR.md) — a mentalidade. O instrumento
  **soa ao carregar**; o cabo é um **objeto**, não um fio; a **relação
  entre as saídas** costuma ser o gesto; o `drift` faz o patch respirar;
  o **seed** é uma hipótese, não um preset; as **8 famílias** são
  verbos. E como ter e crescer uma ideia.

## Parte 2 — Como o instrumento funciona

- [`CABEAMENTO.md`](CABEAMENTO.md) — **leia primeiro.** Áudio × controle,
  o caminho do som até os alto-falantes (o `MIXER` e o `MASTER`), o que
  cada tipo de entrada espera, como somar sinais, como **retroalimentar**
  um módulo, e o que fazer quando uma página diz "cabeie um LFO aqui".
- [`RELACAO_DE_CABO.md`](RELACAO_DE_CABO.md) — o cabo que **processa**:
  `RingMod`/`Fold`/`Difference` na própria conexão, ruptura + cicatriz,
  condução probabilística. Com receitas — clique no corpo de um cabo no
  painel pra usar (editar o `.rmp` à mão vira alternativa avançada).

## Parte 3 — Os módulos, um a um

Cada página segue o mesmo gabarito — ver ["Como ler cada
página"](#como-ler-cada-página) abaixo. Índice por família mais adiante.

## Parte 4 — Receitas

- [`RECEITAS.md`](RECEITAS.md) — montagens passo a passo, com o que
  ouvir e o que mexer. *Potencializar uma ideia*: você tem X, agora
  experimente Y. (Em construção.)

## Apêndice

- [`APENDICE_equivalencias.md`](APENDICE_equivalencias.md) — "se você
  conhece o módulo X do Eurorack, no Rasgo é o Y", e o que o Rasgo faz
  de diferente.

---

## Como ler cada página

Toda página de módulo tem:

1. **Cabeçalho** — nome, família, uma frase de essência, links pro
   dossiê técnico e pro arquivo-fonte.
2. **A ideia** — o conceito: que problema musical o módulo resolve, de
   onde vem, o que ele é *de verdade* (uma corda? um banco de
   ressonadores? um relógio?).
3. **Por dentro** — como funciona tecnicamente, o suficiente pra você
   prever o que cada controle vai fazer. Sem a matemática — essa está
   no dossiê.
4. **Os jacks, um a um** — cada entrada e cada saída: o que é (áudio ou
   controle), o que plugar nela, de onde costuma vir / pra onde costuma
   ir, e se vai ao `MIXER`.
5. **Os controles, um a um** — o que cada knob e chave faz *neste
   módulo*, a faixa útil, os extremos.
6. **Como cabear** — as ligações canônicas (autônomo / performance /
   híbrido) e as cadeias em que o módulo costuma aparecer.
7. **Potencializar** — dois ou três caminhos pra levar a ideia adiante:
   modular tal jack, cruzar com tal módulo, o que muda de caráter.
8. **Se você conhece o Eurorack** — os equivalentes e a diferença
   deliberada do Rasgo.

A densidade varia com o módulo — um `MULT` precisa de menos que um
`SPECTRA`. O guia não escreve prosa por escrever.

---

## As 8 famílias

A paleta do painel agrupa todo módulo numa de 8 famílias de trabalho. A
ordem abaixo é a do fluxo do sinal — de gerar a misturar.

| Família | Verbo | O que faz |
|---|---|---|
| **SOURCE** | gerar | produz som ou tensão do zero (inclui as vozes de modelagem física) |
| **TRANSFORM** | transformar | modifica um sinal que passa — áudio ou CV |
| **MODULATE** | mover | gera um sinal de controle (envelope, LFO, aleatório) |
| **TIME** | marcar tempo | clock, lógica de clock, sequenciadores |
| **DECISION** | decidir | escolhe um valor — quantiza, harmoniza, calcula, compara |
| **ROUTE** | rotear | chave, matriz, múltiplo, morph vetorial |
| **SPACE** | espacializar / lembrar | delay, reverb, granular |
| **OUT** | misturar / medir / enviar | mixer, saída, osciloscópio, MIDI |

---

## Índice por família

Estado: **✓** pronto · **·** a fazer.

### SOURCE — gerar

| # | Módulo | Página | Estado |
|---|---|---|---|
| 09 | `MATTER` | [`09_matter.md`](09_matter.md) | ✓ |
| 11 | `STRING` | [`11_string.md`](11_string.md) | ✓ |
| 18 | `OSC` | [`18_oscilador.md`](18_oscilador.md) | ✓ |
| 19 | `NOISE` | [`19_ruido.md`](19_ruido.md) | ✓ |
| 26 | `CHORD` | [`26_chord.md`](26_chord.md) | ✓ |
| 37 | `PLL` | [`37_pll.md`](37_pll.md) | ✓ |
| 40 | `WAVETABLE` | [`40_wavetable.md`](40_wavetable.md) | ✓ |
| 42 | `ADDITIVE` | [`42_additive.md`](42_additive.md) | ✓ |
| 44 | `OPERATOR` | [`44_operator.md`](44_operator.md) | ✓ |
| 47 | `DRUM` | [`47_drum.md`](47_drum.md) | ✓ |
| 49 | `SIGNAL-IN` | [`49_signal_in.md`](49_signal_in.md) | ✓ |
| 56 | `PULSAR` | [`56_pulsar.md`](56_pulsar.md) | ✓ |
| 57 | `SPECTRA` | [`57_spectra.md`](57_spectra.md) | ✓ |

### TRANSFORM — transformar

| # | Módulo | Página | Estado |
|---|---|---|---|
| 02 | `FILTER` | [`02_filtro.md`](02_filtro.md) | ✓ |
| 13 | `PARAMETRIC` | [`13_parametric.md`](13_parametric.md) | ✓ |
| 20 | `VCA` | [`20_vca.md`](20_vca.md) | ✓ |
| 21 | `CONTROL` | [`21_control.md`](21_control.md) | ✓ |
| 24 | `SHAPE` | [`24_shape.md`](24_shape.md) | ✓ |
| 25 | `LPG` | [`25_lpg.md`](25_lpg.md) | ✓ |
| 32 | `WASP` | [`32_wasp.md`](32_wasp.md) | ✓ |
| 39 | `GLIDE` | [`39_glide.md`](39_glide.md) | ✓ |
| 45 | `FORMANT` | [`45_formant.md`](45_formant.md) | ✓ |
| 53 | `CRUSH` | [`53_crush.md`](53_crush.md) | ✓ |
| 55 | `RESONATOR` | [`55_resonator.md`](55_resonator.md) | ✓ |
| 58 | `SHIFTER` | [`58_shifter.md`](58_shifter.md) | ✓ |
| 59 | `VCA4` | [`59_vca4.md`](59_vca4.md) | ✓ |
| 60 | `VOCODER` | [`60_vocoder.md`](60_vocoder.md) | ✓ |

### MODULATE — mover

| # | Módulo | Página | Estado |
|---|---|---|---|
| 01 | `FUNCTION` | [`01_gerador_de_funcao.md`](01_gerador_de_funcao.md) | ✓ |
| 06 | `ENVELOPE` | [`06_envelope.md`](06_envelope.md) | ✓ |
| 23 | `SH` | [`23_sample_hold.md`](23_sample_hold.md) | ✓ |
| 27 | `DRIFT` | [`27_drift.md`](27_drift.md) | ✓ |
| 36 | `CHAOS` | [`36_chaos.md`](36_chaos.md) | ✓ |
| 54 | `STAGES` | [`54_stages.md`](54_stages.md) | ✓ |

### TIME — marcar tempo

| # | Módulo | Página | Estado |
|---|---|---|---|
| 05 | `CLOCK` | [`05_clock.md`](05_clock.md) | ✓ |
| 08 | `TURING` | [`08_turing.md`](08_turing.md) | ✓ |
| 15 | `SEQUENCE` | [`15_sequence.md`](15_sequence.md) | ✓ |
| 22 | `LOGIC` | [`22_logic.md`](22_logic.md) | ✓ |
| 30 | `TRIGSEQ` | [`30_trigseq.md`](30_trigseq.md) | ✓ |

### DECISION — decidir

| # | Módulo | Página | Estado |
|---|---|---|---|
| 04 | `DECISION` | [`04_decisao.md`](04_decisao.md) | ✓ |
| 12 | `QUANTIZER` | [`12_quantizer.md`](12_quantizer.md) | ✓ |
| 14 | `HARMONY` | [`14_harmony.md`](14_harmony.md) | ✓ |
| 31 | `ABACUS` | [`31_abacus.md`](31_abacus.md) | ✓ |
| 51 | `BOXCAR` | [`51_boxcar.md`](51_boxcar.md) | ✓ |

### ROUTE — rotear

| # | Módulo | Página | Estado |
|---|---|---|---|
| 28 | `SWITCH` | [`28_switch.md`](28_switch.md) | ✓ |
| 33 | `MATRIX` | [`33_matrix.md`](33_matrix.md) | ✓ |
| 34 | `MULT` | [`34_mult.md`](34_mult.md) | ✓ |
| 43 | `PLANAR` | [`43_planar.md`](43_planar.md) | ✓ |

### SPACE — espacializar / lembrar

| # | Módulo | Página | Estado |
|---|---|---|---|
| 07 | `MEMORY` | [`07_memory.md`](07_memory.md) | ✓ |
| 10 | `SPACE` | [`10_space.md`](10_space.md) | ✓ |
| 41 | `LOOPER` | [`41_looper.md`](41_looper.md) | ✓ |
| 46 | `HALL` | [`46_hall.md`](46_hall.md) | ✓ |
| 48 | `SAMPLER` | [`48_sampler.md`](48_sampler.md) | ✓ |
| 50 | `TURNTABLE` | [`50_turntable.md`](50_turntable.md) | ✓ |
| 52 | `SWIRL` | [`52_swirl.md`](52_swirl.md) | ✓ |

### OUT — misturar / medir / enviar

| # | Módulo | Página | Estado |
|---|---|---|---|
| 16 | `MIXER` | [`16_mixer.md`](16_mixer.md) | ✓ |
| 17 | `MASTER` | [`17_master.md`](17_master.md) | ✓ |
| 29 | `SCOPE` | [`29_scope.md`](29_scope.md) | ✓ |
| 38 | `NOTE-OUT` | [`38_note_out.md`](38_note_out.md) | ✓ |

### Dois dossiês sem página de módulo — mas com página própria

- **#03 — a relação de `Cable`** (`RingMod` / `Fold` / `Difference` +
  ruptura/cicatriz + condução probabilística) não é um módulo: é uma
  propriedade do *cabo*. Página dedicada:
  [`RELACAO_DE_CABO.md`](RELACAO_DE_CABO.md) — as fórmulas, receitas, e
  **como usar no painel** (clique no corpo de um cabo — 2026-09-12).
- **#35 — `AUDIO-IN`** virou o **`SIGNAL-IN`** (#49). Uma página só.

---

## Progresso

- **Andaime + primer:** `00_indice.md`, `CABEAMENTO.md` (áudio×CV,
  caminho do som, feedback, fan-in), `COMO_PENSAR.md`,
  `APENDICE_equivalencias.md`.
- **Todas as 8 famílias completas — 58/58 páginas de módulo**, no
  gabarito de 8 partes, jack a jack:
  SOURCE (13) · TRANSFORM (14) · MODULATE (6) · TIME (5) · DECISION (5) ·
  ROUTE (4) · SPACE (7) · OUT (4).
- Conferidas contra o construtor `Signal(...)` de cada `.hpp` (nomes e
  faixas reais de porta/parâmetro), o `LearnCatalog` e o dossiê §1 de
  cada módulo.
- **Aprofundamento didático completo (2026-09-12):** todas as 58
  páginas, mais `COMO_PENSAR.md`/`CABEAMENTO.md`/
  `APENDICE_equivalencias.md`/`RELACAO_DE_CABO.md`, ganharam o conceito
  por trás de cada parâmetro explicado desde o início — não só a
  fórmula/valor.
- **`RECEITAS.md` completo (2026-09-12):** 10 montagens passo a passo,
  da voz subtrativa mínima ao vocoder falado.
- **Falta:** só a revisão da tradução, quando o site for construído.

**Todo módulo novo do Rasgo Modular ganha a página de guia junto com o
dossiê** — o catálogo segue sempre aberto.
