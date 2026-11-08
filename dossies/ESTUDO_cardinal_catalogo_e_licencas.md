# Estudo — o Cardinal como referência: catálogo e licenças (2026-11-08)

Pedido do autor: "podemos fazer as 3 frentes" — (1) comparar o catálogo do
Cardinal com o do Modular, (2) estudar coleções específicas, (3) estudar a
arquitetura de host (aplicativo e plugin). Este documento cobre a **frente 1**
e o levantamento de licenças que as três compartilham. Nada foi copiado;
nada do Cardinal foi compilado nem executado — só leitura dos manifestos e
dos textos de licença.

O Atlas de Referência do RASGO já registra o Cardinal (§5, inspeção de
8/10/2026) e as regras de uso: estudar arquitetura, organização e conceitos,
reimplementar com desvio; não copiar código sem avaliar a licença, não tratar
todas as coleções como de mesma licença, não substituir a identidade RASGO
por um host do Cardinal.

## 1. Proveniência do material

| | |
|---|---|
| Fonte | DISTRHO Cardinal 26.02, `cardinal+deps-26.02.tar.xz` (com as dependências) |
| Arquivo | `/media/luc/2tb_interno/cardinal+deps-26.02.tar.xz`, 519.798.160 bytes |
| SHA-256 | `e318d109837a63b62d15ce2009df05ec2431074d8f2d943a0cde7dad954f621d` |
| Extração | `/media/luc/2tb_interno/cardinal-26.02`, conferida em 8/11 contra a listagem do arquivo: 33.457 arquivos com nome e tamanho iguais, 85 links simbólicos, nada faltando |
| Licença do host | GPL-3.0-or-later (`LICENSE`) |

## 2. Licenças das coleções

85 coleções com `plugin.json`, **1.422 módulos** declarados (mais 4 pastas
complementares sem manifesto: versões escuras e "helpers").

**Código** — todas as licenças declaradas são livres e, em princípio,
compatíveis com o AGPLv3 do Modular: GPL-3.0-or-later (52), MIT (13),
BSD-3-Clause (10), CC0-1.0 (4), GPL-2.0-or-later (3; o "ou posterior"
permite a v3), EUPL-1.2 (2; lista o AGPLv3 entre as compatíveis), WTFPL (1).
Mesmo assim, aproveitar código de uma coleção pede a sua licença, os créditos
e as dependências conferidos arquivo a arquivo.

**Arte, painéis e marcas** — 19 coleções licenciam a arte como Creative
Commons **não comercial** e/ou **sem derivadas**, ou guardam logo/marca:
AS (CC BY-NC-ND 4.0; logo com direitos reservados), Algoritmarte, AnimatedCircuits,
AriaModules, Befaco, Bidoo, GrandeModular, ImpromptuModular, LyraeModules,
MindMeldModular, Orbits, PathSet, RebelTech, ZZC, admiral, dBiz,
stoermelder-packone, surgext, unless_modules. Dessas, só o conceito e o código
(com a sua licença) interessam; arte e nomes ficam de fora. Cinco coleções
põem a arte em CC0 (HetrickCV, LifeFormModular, WSTD-Drums, nonlinearcircuits,
rcm-modules).

Método: leitura dos `LICENSE*`, `COPYING*`, `README*` e `res/**/LICENSE*` de
cada coleção (o texto da GPLv3 foi excluído — ele contém "noncommercially" e
gerava falso positivo). É uma triagem, não um parecer: a licença de cada
arquivo ainda se confere no momento do uso.

## 3. Catálogo: o que o Cardinal tem e o Modular cobre

Pelas tags dos manifestos (um módulo pode ter várias), cruzadas com os 60
módulos com dossiê do Modular (`00_indice.md`):

| Categoria (tags do Cardinal, nº de módulos) | No Modular |
|---|---|
| oscilador / VCO (89 / 35) | OSC, WAVETABLE, ADDITIVE, OPERATOR, PULSAR, PLL, MATTER |
| filtro / EQ (49 / 17) | FILTER, LPG, WASP, PARAMETRIC, FORMANT |
| VCA (26) | VCA, VCA4 |
| envelope / função / LFO (51 / 12 / 35) | ENVELOPE, STAGES, FUNCTION |
| aleatório / ruído / S&H (97 / 35 / 38) | NOISE, TURING, CHAOS, DRIFT, SH |
| sequenciador / clock (190 / 29 / 81 modificadores) | SEQUENCE, TRIGSEQ, PHRASE, CLOCK (euclidiano, divisões) |
| lógica / chave / múltiplo / atenuador (65 / 48 / 9 / 39) | LOGIC, DECISION, SWITCH, MATRIX, MULT, CONTROL |
| quantizador / altura (38 / 13 "tuner") | QUANTIZER, HARMONY, CHORD |
| mixer / pan (118 / 18) | MIXER, VCA4, PLANAR, MASTER |
| delay / reverb / chorus-phaser (31 / 25 / 10) | SPACE, HALL, SWIRL |
| distorção / waveshaper / ring (44 / 59 / 21) | SHAPE, CRUSH, relação de cabo RingMod |
| modelagem física / bateria (25 / 19) | STRING, RESONATOR, DRUM |
| sampler / granular / gravação (31 / 10 / 6) | SAMPLER, MEMORY (granular), LOOPER, TURNTABLE |
| vocoder / espectral / frequency shift | VOCODER, SPECTRA, SHIFTER |
| visual / entrada externa / MIDI (55 / 18 / 14) | SCOPE, SIGNAL-IN (áudio + MIDI + CV), NOTE-OUT |
| seguidor de envelope (5) | LISTEN (níveis e ataques por região) |

## 4. Lacunas reais

Conferidas no código e nos dossiês do Modular (o que parecia faltar e existe
com outro nome ficou de fora: granular = MEMORY, MIDI de entrada = SIGNAL-IN,
*ducking* = ENVELOPE/SCOPE em combinação):

1. **Dinâmica dedicada** — compressor, limitador, noise gate (≈ 25 módulos no
   Cardinal; ex.: Bogaudio PRESSOR/LMTR/NSGT, ZamAudio Compressor, CVfunk
   Pressed Duck — mixer com sidechain, ChowDSP ChowTape Compression). No
   Modular só o MASTER limita, e a dinâmica sai de combinações.
2. **Arpejador** (8; ex.: Amalgamated Harmonics Arp 3.x, Bogaudio ARP,
   ML_modules Arpeggiator). Encaixa entre CHORD/HARMONY e SEQUENCE.
3. **Áudio → altura** (pitch tracking: Cardinal "Audio To CV Pitch",
   Sapphire Env). Já previsto no roteiro do LISTEN (etapa de fundamental).
4. **Polifonia** — 615 módulos do Cardinal são polifônicos (cabos de até 16
   canais, do VCV Rack). No Modular os cabos são mono e o NOTE-OUT/SIGNAL-IN
   são last-note: é decisão de arquitetura, não um módulo a acrescentar.

As categorias "hardware clone" (78), "expander" (57), "blank" (25) e
"visual" (55) não são lacunas: o Modular reimplementa conceitos com desvio
(não clona), não tem expansores e não precisa de painéis vazios.

## 5. Próximos passos (frentes 2 e 3)

- **Frente 2 — coleções a estudar**, escolhidas pelas lacunas e pela licença
  limpa (código livre, arte livre): Bogaudio (PRESSOR, LMTR, NSGT, FOLLOW,
  ARP; GPL-3.0+), Amalgamated Harmonics (Arp; BSD-3), ZamAudio (Compressor;
  GPL-2.0+), Cardinal (Audio To CV Pitch; GPL-3.0+), CVfunk (Pressed Duck;
  MIT). Estudo do conceito e do algoritmo, com desvio RASGO; código só com
  licença, créditos e dependências conferidos.
- **Frente 3 — arquitetura de host**: como o Cardinal embrulha o Rack com o
  DPF (aplicativo e plugin AU/CLAP/LV2/VST2/VST3), a separação UI/DSP e o
  empacotamento, pensando num Modular que também rode como plugin.
