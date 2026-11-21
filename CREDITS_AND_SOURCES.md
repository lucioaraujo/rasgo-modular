# Rasgo Modular — créditos, fontes e licenças

Este é o documento **voltado à publicação**: curto, verificável, e o que
alguém precisa ler antes de usar, redistribuir ou derivar do Rasgo
Modular. O registro arquivístico completo — módulo a módulo, com a fonte
de cada ideia — mora em [`RASGO_MODULAR.md §29`](RASGO_MODULAR.md) e na
tabela de módulos do mesmo documento. Onde os dois divergirem, o §29 é a
fonte de verdade; este aqui é o resumo, e um resumo que se afasta do
original é um erro a corrigir, não uma versão alternativa.

## Autoria e licença

**Lúcio de Araújo**, 2026. Código sob **GNU AGPL-3.0-or-later** — ver
[`LICENSE`](LICENSE). A escolha é a licença habitual da família RASGO
(decisão do autor, 1 set. 2026).

**Os módulos de DSP** (`src/dsp/`, `src/core/`) são oferecidos **também**
sob **GPL-3.0-or-later**, à escolha de quem os recebe, para o VCV Rack e o
Cardinal (decisão do autor, 20/11/2026). O instrumento continua AGPL. O que
vale para cada parte, por que isso é seguro (autoria conferida; o código do
Navalha 2 de Glerm Soares já é GPL-3.0-or-later) e o que muda na prática:
[`LICENSING.md`](LICENSING.md). Texto da GPL: [`LICENSE.GPL-3.0`](LICENSE.GPL-3.0).

O front-end de produção usa o **JUCE sob AGPLv3** (tier livre), sem
licença comercial. Como o código deste projeto já é AGPLv3-or-later, o
encaixe é direto — detalhe e conferência em
[`apps/juce/LICENSE_STATUS.md`](apps/juce/LICENSE_STATUS.md).

## Escuta e fraseado — implementação própria

Os módulos locais LISTEN e PHRASE, de 17/10/2026, são implementações
originais sob AGPL-3.0-or-later. LISTEN retoma como referência conceitual a
escuta por regiões do Trioio, commit050637f8baf6ff1545965b7e6b0769247d8c201b;
seu divisor offline não foi copiado. PHRASE não incorpora o arquivo de
escalas ou código do Trioio. Contratos e proveniência nos dossiês
[61](dossies/61_listen.md) e [62](dossies/62_phrase.md).

## Dinâmica — implementação própria

O módulo DYNAMICS, de 8/11/2026, é implementação original sob
AGPL-3.0-or-later, na formulação pública de D. Giannoulis, M. Massberg e
J. D. Reiss, "Digital Dynamic Range Compressor Design — A Tutorial and
Analysis", *JAES* 60(6), 2012. O conceito foi lido, sem cópia de código, no
Bogaudio PRESSOR (GPL-3.0-or-later), no ZamComp de Damien Zammit
(GPL-2.0-or-later) e no Pressed Duck de Cody Geary / CVfunk (MIT), dentro do
DISTRHO Cardinal 26.02. Estudo e licenças em
[`dossies/ESTUDO_cardinal_catalogo_e_licencas.md`](dossies/ESTUDO_cardinal_catalogo_e_licencas.md);
contrato em [63](dossies/63_dynamics.md).

## Arpejador — implementação própria

O módulo ARP, de 8/11/2026, é implementação original sob AGPL-3.0-or-later.
A ideia é pública — os arpejadores de sintetizador dos anos 1970 (Roland
Jupiter-4, ARP Odyssey) e os seus modos clássicos; o conceito foi lido, sem
cópia de código, no Bogaudio ARP (GPL-3.0-or-later) e no Arp 3.x da
Amalgamated Harmonics (BSD-3-Clause), dentro do DISTRHO Cardinal 26.02. A
tabela de formatos é a do CHORD (#26). Contrato em [64](dossies/64_arp.md).

## CELL, MARKOV, BOUNCE, SPRING — implementações próprias

Os módulos 65–68, de 9/11/2026, são implementação original sob
AGPL-3.0-or-later, a partir de ideias públicas; os conceitos foram lidos, sem
cópia de código, em coleções de licença livre dentro do DISTRHO Cardinal 26.02:

- **CELL** — autômatos celulares elementares de S. Wolfram (*Rev. Mod. Phys.*
  55, 1983); voxglitch Glitch Sequencer (GPL-3.0-or-later).
- **MARKOV** — a composição por cadeias de Markov de Hiller e Isaacson (*Illiac
  Suite*, 1957) e de Xenakis (*Analogique A*); ZetaCarinae Rosenchance e
  GuildensTurn (GPL-3.0-or-later).
- **BOUNCE** — mecânica de livro-texto (queda livre, restituição); JW-Modules
  Bouncy Balls (BSD-3-Clause) e voxglitch Hazumi (GPL-3.0-or-later).
- **SPRING** — V. Välimäki, J. Parker, J. S. Abel, "Parametric Spring
  Reverberation Effect", *JAES* 58(7/8), 2010; Autinn Coil (GPL-3.0-or-later).

Os módulos 69–71 (MOUSSE, PENDULUM, PRISM), de 9/11/2026, partem de ideias do
autor e de física pública — viscoelasticidade (sólido linear padrão de Zener),
mecânica de Lagrange do pêndulo duplo, dispersão óptica; o SVF do PRISM segue
V. Zavalishin, *The Art of VA Filter Design* (2012). Nenhum tem equivalente no
Cardinal; implementação própria sob AGPL-3.0-or-later.

Contratos em [65](dossies/65_cell.md), [66](dossies/66_markov.md),
[67](dossies/67_bounce.md) e [68](dossies/68_spring.md).

## FORM, TEMPO, VOLTA e a máquina de escalas (21/11/2026)

- **FORM:** a forma do rasgo-synth / Rasgo Synth Studio (caminhada de
  Markov sobre papéis, coda absorvente; Freytag, Kishōtenketsu) e o Trioio
  (arco de tensão, dinâmica nunca acima da base), do mesmo autor; a medida de
  11 dB entre vale e clímax é do Studio (`docs/COMPOSICAO.md`).
- **TEMPO:** N. P. McAngus Todd, "The dynamics of dynamics" (JASA, 1992) — o
  arco de frase; A. Friberg & J. Sundberg, "Does music performance allude to
  locomotion?" (JASA 105, 1999) — o ritardando final; a mola de andamento e o
  rubato de Fourier do Rasgo Synth Asterismus (mesmo autor).
- **VOLTA:** a "transformação reversível" do Navalha 2 (Glerm Soares / Lúcio
  Araújo), como conceito; o TURING (Music Thing Turing Machine, Tom
  Whitwell) como ponto de partida. Nenhum código copiado.
- **QUANTIZER, máquina de escalas:** a interpolação entre escalas do Trioio
  (`ScaleMachine`, mesmo autor), como conceito; implementação própria.

## POLY-IN (#103, 20/11/2026)

- A alocação de vozes dos sintetizadores polifônicos (rodízio, a mesma
  voz, ordem de altura, roubo da mais antiga) — prática pública; o MIDI-CV
  polifônico do VCV Rack (GPL) lido como conceito, nenhum código.

## SIGNAL-IN: MIDI clock (20/11/2026)

- **Fons Adriaensen**, "Using a DLL to filter time" (Linux Audio
  Conference, 2005): o filtro de tempo de 2ª ordem que o JACK usa. Conceito
  público, implementação própria em `src/dsp/SignalIn.hpp` (banda variável
  e atraso de entrega medido são do Rasgo).
- **MIDI 1.0** (MIDI Association): mensagens de tempo real (clock, start,
  continue, stop) e song position pointer.

## Módulos 91–102 — física, óptica, fita e a família (20/11/2026)

Aprovados pelo autor em 9/11/2026 ("aprovo todos") e criados em 20/11/2026.
Implementação sob AGPL-3.0-or-later; nenhum código de terceiros.

- **PRESSURE** — teoria cinética dos gases (Maxwell, Boltzmann); conceito lido
  no Sapphire Nucleus (GPL-3.0-or-later), sem cópia de código.
- **ELASTIC** — corda discreta de massas e molas; conceito lido no Sapphire
  Elastika (GPL-3.0-or-later), sem cópia de código.
- **REFRACT**, **INTERFERENCE**, **LENS** — óptica pública (Ibn Sahl, Snell,
  Descartes; Young, 1801; profundidade de campo), sem equivalente no Cardinal.
  O LENS usa o SVF de V. Zavalishin e os passa-tudo de M. R. Schroeder (1962).
- **TAPE** — a saturação vem do `TapeSaturator.hpp` do Rasgo Synth Studio
  (mesmo autor, AGPL), adaptada; o ChowTape (GPL-3.0) foi lido como conceito,
  sem cópia de código.
- **PREDATOR** — Lotka (1925) e Volterra (1926); a integração em logaritmos e
  a captura vêm do `PopulationCycles` do **Aquorbium** (mesmo autor, AGPL),
  copiadas e adaptadas.
- **SWARM** — Kuramoto (1975); o coro de fases do Aquorbium (`ChorusPhase`,
  mesmo autor) e o cardume de T. Blackwell e P. Bentley (2002).
- **ECLIPSE** — os meses sinódico e dracônico, o limite de eclipse e o Saros:
  dados astronômicos públicos, com os valores e a validação do céu do **Rasgo
  Synth Asterismus** (mesmo autor); nenhum código copiado.
- **SEASON** — a fórmula pública da altura do Sol; a ideia do relógio de dia,
  noite e estação vem do Aquorbium (mesmo autor).
- **GENDYN** — síntese estocástica dinâmica de I. Xenakis (*Formalized Music*;
  GENDY3, 1991); copiado e adaptado do `GendynOscillator.hpp` do Rasgo Synth
  Studio (mesmo autor, AGPL).
- **RUNGLER** — o Rungler de Rob Hordijk (Benjolin, Blippoo Box), pelos textos
  públicos dele; o Rasgo Synth Studio tem uma versão (mesmo autor) que serviu
  de ponto de partida. Nenhum código de clone.

Contratos em `dossies/91_pressure.md` a `dossies/102_rungler.md`.

## Código de terceiros incorporado

Só três, e nenhum no núcleo de DSP:

| O quê | Autor / origem | Licença | Onde |
|---|---|---|---|
| **dr_wav** v0.14.6 | David Reid (dr_libs) | domínio público ou MIT-0 | `third_party/dr_wav/` — implementação restrita a `src/io/AudioFile.cpp` |
| **DelayPitchShifter** | algoritmo `G09.pitchshift.pd` (Miller Puckette, domínio público); implementação C++ de `NAVALHA2_JUCE/HeritagePitch.cpp` — Navalha de **Glerm Soares**, reescrita por Lúcio Araújo | GPL-3.0-or-later | `src/dsp/PitchShift.hpp` (usado pelo `SAMPLER`) |
| **SlicePlayer** (conceito e trechos) | Glerm Soares / Lúcio Araújo (`NAVALHA2_JUCE`) | GPL-3.0-or-later | `src/dsp/Sampler.hpp` |

As duas entradas GPL-3.0-or-later são **decisão consciente**, não
descuido: GPLv3 é compatível com AGPLv3-or-later na direção em que este
projeto a usa, e o crédito a Glerm Soares está registrado em cada ponto
de uso. Ver `RASGO_MODULAR.md §29.1` e
`dossies/ESTUDO_audio_sampling.md §2`.

O **JUCE não é versionado** neste repositório — nem submodule, nem
FetchContent. É apontado por `-DRASGO_MODULAR_JUCE_PATH`. Política da
família RASGO; ver [`INSTALL.md`](INSTALL.md).

## Regra de pesquisa

O projeto classifica toda fonte por cor antes de usar:

- **VERDE** — domínio público, licença livre compatível, teoria publicada,
  ou código do próprio autor. Pode ser incorporado.
- **AMARELO** — copyleft compatível. Entra com crédito explícito e
  decisão registrada.
- **VERMELHO** — proprietário ou non-commercial. Serve **só como
  referência**: lê-se o comportamento, não o código, e a implementação é
  escrita do zero no idioma do Rasgo.

Módulos inspirados em instrumentos comerciais fechados (Mutable, Xaoc,
Intellijel, Make Noise, 4ms, Frap Tools, entre outros) estão na faixa
VERMELHA: o que foi tomado é o **conceito musical** — o que o módulo faz
e por quê —, nunca circuito ou código. Cada um desses casos está marcado
com ★ na tabela de módulos e rastreado em `PESQUISA_MODULOS.md §7`.

## Teoria e domínio público

O grosso do DSP vem de literatura pública, e está citado por módulo na
tabela de `RASGO_MODULAR.md`. Os nomes que reaparecem com mais peso:

- **Homer Dudley**, *The Vocoder* (Bell Labs, 1938) — modo vocoder do `FORMANT`
- **Gunnar Fant**, *Acoustic Theory of Speech Production* (1960) — formantes
- **Miller Puckette** / Pure Data — pitch-shift por atraso
- **Paul Kellet** — filtro de ruído rosa (domínio público)
- **Vadim Zavalishin** — filtros TPT / zero-delay-feedback
- **Udo Zölzer**, *DAFX* — all-pass, modulação, efeitos
- **Karplus & Strong** (1983) — corda pinçada
- **Jot / Householder** — redes de atraso realimentadas (reverb)
- **de Cheveigné & Kawahara**, YIN (2002) — detecção de altura
- **ITU-R BS.1770-4 / EBU R128** — medição de loudness (`src/dsp/Loudness.hpp`)

## Instrumentos irmãos

O Rasgo Modular herda e devolve para a família:

- **ANTITOTEM** — CMOS/Lunetta, clocks, divisores, memória de topologia,
  feedback; e o molde de empacotamento e CI multiplataforma
- **NAVALHA 2** (Glerm Soares / Lúcio Araújo) — sampling, slices, o
  `OutputStage` que virou a proteção de saída de excelência
- **RASGO SYNTH** — o cabeçalho de linha única, i18n e a disciplina de
  gravação/score
- **TRIOIO** — decisão, memória, proveniência

A proteção de saída (`src/dsp/OutputStage.hpp`) é **estudada de** código
do próprio autor em `NAVALHA2_JUCE` e `ANTITOTEM` (GPLv3/AGPLv3,
compatível), reescrita no idioma header-only e sem dependências do
Modular.

## Tipografia e marca

A marca RASGO usada no cabeçalho dos dois front-ends vem de
`apps/panel/assets/rasgo_logo_gray.h`, mapa de cobertura gerado do SVG
master do projeto — autoria própria. Qualquer fonte usada em páginas web
do projeto deve ser **livre e auto-hospedada** via `@font-face`, nunca
uma família nomeada do sistema (regra da família RASGO).

## O que este repositório NÃO contém

Verificado antes da publicação, conforme o gate editorial:

- nenhuma gravação de áudio privada, tomada de teste ou material de
  brainstorm — as gravações do instrumento vão pra a pasta de música do
  usuário, fora do repositório (`.gitignore`);
- nenhum binário de terceiro (o JUCE é apontado, não copiado);
- nenhum arquivo de áudio com origem não declarada.
