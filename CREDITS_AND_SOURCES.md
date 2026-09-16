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

O front-end de produção usa o **JUCE sob AGPLv3** (tier livre), sem
licença comercial. Como o código deste projeto já é AGPLv3-or-later, o
encaixe é direto — detalhe e conferência em
[`apps/juce/LICENSE_STATUS.md`](apps/juce/LICENSE_STATUS.md).

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
