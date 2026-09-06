# Dossiês do Rasgo Modular — índice

Cada módulo do Rasgo Modular tem um **dossiê** antes do código, no padrão
`AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`: problema musical e papel no
fluxo · fontes primárias e conceitos apropriados (não copiar código) ·
modelo matemático, estados e extremos · três modos obrigatórios
(autônoma / performance / híbrida) · portas, parâmetros, limites ·
alternativas descartadas · critérios técnicos e perguntas de escuta ·
integração e painel.

Visão geral e estado do projeto: [`../RASGO_MODULAR.md §36`](../RASGO_MODULAR.md);
log operacional: [`../TAREFAS.md`](../TAREFAS.md); ordem de execução e
pesquisa de módulos: [`../PESQUISA_MODULOS.md`](../PESQUISA_MODULOS.md).

| # | Dossiê | Módulo | Família | Estado |
|---|---|---|---|---|
| 1 | [`01_gerador_de_funcao.md`](01_gerador_de_funcao.md) | `FUNCTION` | SOURCE / TIME | marco 1 |
| 2 | [`02_filtro.md`](02_filtro.md) | `FILTER` | TRANSFORM | marco 1 |
| 3 | [`03_relacao_de_cabo.md`](03_relacao_de_cabo.md) | relação de `Cable` | RELATION | marco 1 |
| 4 | [`04_decisao.md`](04_decisao.md) | `DECISION` | DECISION | marco 1 |
| 5 | [`05_clock.md`](05_clock.md) | `CLOCK` (`EuclidClock`) | TIME | marco 1 |
| 6 | [`06_envelope.md`](06_envelope.md) | `ENVELOPE` | UTILITY / TIME | marco 1 |
| 7 | [`07_memory.md`](07_memory.md) | `MEMORY` | MEMORY | marco 2 |
| 8 | [`08_turing.md`](08_turing.md) | `TURING` (`TuringLoop`) | SEQUENCE | marco 2 |
| 9 | [`09_matter.md`](09_matter.md) | `MATTER` | MATTER | marco 2 |
| 10 | [`10_space.md`](10_space.md) | `SPACE` | SPACE | marco 2 |
| 11 | [`11_string.md`](11_string.md) | `STRING` (`StringVoice`) | MATTER | marco 2 |
| 12 | [`12_quantizer.md`](12_quantizer.md) | `QUANTIZER` | DECISION / PERCEPTION | marco 2 |
| 13 | [`13_parametric.md`](13_parametric.md) | `PARAMETRIC` | TRANSFORM / UTILITY | marco 2 |
| 14 | [`14_harmony.md`](14_harmony.md) | `HARMONY` | DECISION / INFERENCE | marco 2 |
| 15 | [`15_sequence.md`](15_sequence.md) | `SEQUENCE` (`StepSequencer`) | SEQUENCE | marco 2 |
| 16 | [`16_mixer.md`](16_mixer.md) | `MIXER` | MIX | marco 2 |
| 17 | [`17_master.md`](17_master.md) | `MASTER` | MIX / METER | marco 2 |
| 18 | [`18_oscilador.md`](18_oscilador.md) | `OSC` | SOURCE | marco 3 |
| 19 | [`19_ruido.md`](19_ruido.md) | `NOISE` | SOURCE / UTILITY | marco 3 |
| 20 | [`20_vca.md`](20_vca.md) | `VCA` | TRANSFORM / UTILITY | marco 3 |
| 21 | [`21_control.md`](21_control.md) | `CONTROL` | UTILITY | marco 3 |
| 22 | [`22_logic.md`](22_logic.md) | `LOGIC` | TIME / UTILITY | marco 3 |
| 23 | [`23_sample_hold.md`](23_sample_hold.md) | `SH` | UTILITY | marco 3 |
| 24 | [`24_shape.md`](24_shape.md) | `SHAPE` | TRANSFORM | marco 3 |
| 25 | [`25_lpg.md`](25_lpg.md) | `LPG` | TRANSFORM / UTILITY | marco 3 |
| 26 | [`26_chord.md`](26_chord.md) | `CHORD` | SOURCE | marco 3 |
| 27 | [`27_drift.md`](27_drift.md) | `DRIFT` | DECISION / UTILITY | marco 3 |
| 28 | [`28_switch.md`](28_switch.md) | `SWITCH` | ROUTE / UTILITY | marco 3 |
| 29 | [`29_scope.md`](29_scope.md) | `SCOPE` | METER / UTILITY | marco 3 |
| 30 | [`30_trigseq.md`](30_trigseq.md) | `TRIGSEQ` | SEQUENCE / TIME | marco 3 |
| 31 | [`31_abacus.md`](31_abacus.md) | `ABACUS` | LOGIC / UTILITY | marco 3 |
| 32 | [`32_wasp.md`](32_wasp.md) | `WASP` | TRANSFORM | marco 3 |
| 33 | [`33_matrix.md`](33_matrix.md) | `MATRIX` | ROUTE / MIX | marco 3 |
| 34 | [`34_mult.md`](34_mult.md) | `MULT` | UTILITY | marco 3 |
| 35 | [`35_audio_in.md`](35_audio_in.md) | `AUDIO-IN` | SOURCE | marco 3 |
| 36 | [`36_chaos.md`](36_chaos.md) | `CHAOS` | DECISION | marco 3 |
| 37 | [`37_pll.md`](37_pll.md) | `PLL` | SOURCE | marco 3 |
| 38 | [`38_note_out.md`](38_note_out.md) | `NOTE-OUT` | MIX | marco 3 |
| 39 | [`39_glide.md`](39_glide.md) | `GLIDE` | TRANSFORM / PITCH | Onda A (2026-09-06) |
| 40 | [`40_wavetable.md`](40_wavetable.md) | `WAVETABLE` | SOURCE | Onda A (2026-09-06) |

**40 módulos feitos.** Próxima leva (dossiê antes do código):
`PESQUISA_MODULOS.md §2.4` — Onda A `LOOPER` (`GLIDE`/`WAVETABLE` feitos),
Onda B `ADDITIVE`/`PLANAR`/`OPERATOR`/`FORMANT`, Onda C reverb FDN/`DRUM`,
Onda D `SAMPLER`/`SIGNAL-IN`.

**Fundação** (sem dossiê próprio — documentada em `RASGO_MODULAR.md §36.1-36.2`):
`SignalGraph` (grafo de áudio, `Signal`, `Cable`; modulação
saída→parâmetro **aditiva** desde 2026-09-04), `ControlSnapshot`,
`Panel`/`AsciiPanel`, `WavWriter`, `Oversampler2x` (`src/dsp/Oversampler.hpp`
— 2× meia-banda compartilhado por `SHAPE`/`WASP`); modelo de conexão de
3 camadas (matriz, constelação, semântico); serialização do patch.

**Peças** (`../examples/`, renders em `../validation-output/`):
`primeiro_fragmento` (10 s), `peca_generativa` (40 s),
`peca_generativa_2` (50 s, constelação), `peca_generativa_3` (55 s,
barramento semântico), `peca_generativa_4` (42 s, protótipo do
`MotionEngine` + `ScoreRecorder` — ver
`ESTUDO_seed_composicao_generativa.md §3.6/§5`; a `SYSTEM SCORE`
também fica salva, `peca_generativa_4.score.txt`).

**Painel gráfico de teste** (`../apps/panel/`, X11 + ALSA + Xrandr):
case Eurorack que quebra em linhas, coluna de catálogo por família,
arrastar-para-criar, sugestão de módulo por `[s]`. Decisões de design em
[`../apps/panel/design.md`](../apps/panel/design.md).

**Estudos à parte:**
[`ESTUDO_seed_composicao_generativa.md`](ESTUDO_seed_composicao_generativa.md)
— os 4 itens da conversa (Seed/composição/partitura/pedagogia): Motion
Engine (`apps/panel/MotionEngine.hpp`), Patch Genetics `MUTATE`/`EVOLVE`/
`CROSS`/`FREEZE` (`apps/panel/PatchGenetics.hpp`), `SYSTEM SCORE` +
`MUSICAL SCORE` (`apps/panel/ScoreRecorder.hpp`, `NOTE-OUT`), gramática
explícita do `Seed` (`apps/panel/SeedGrammar.hpp`) e a caixa LEARN dos 37
módulos (`apps/panel/LearnCatalog.hpp`, sempre presente no rodapé da
paleta) — **feitos**. Form Engine e a camada contextual do Learning
Engine (`WHY?`/`WHAT IF?`) continuam mapeados, não construídos;
[`ESTUDO_audio_sampling.md`](ESTUDO_audio_sampling.md) — `SAMPLER`/
`TURNTABLE`/`TAPE`.
