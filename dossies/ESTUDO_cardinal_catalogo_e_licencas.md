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

## 4b. Ideias de módulo que o Modular não tem (passada por conceito)

A comparação por categoria (§3) conta "sequenciador" como coberto se houver
algum sequenciador. A passada fina procura ideias distintas nas descrições
dos 1.422 módulos e confere só na tabela de módulos de `RASGO_MODULAR.md`.
Faltam, além das lacunas do §4:

| Ideia | Exemplos no Cardinal (arte livre) |
|---|---|
| autômato celular como sequenciador | voxglitch Glitch Sequencer |
| cadeia de Markov | ZetaCarinae Rosenchance, GuildensTurn |
| física — bolas, gravidade, cordas elásticas | JW Bouncy Balls, Sapphire Elastika, voxglitch Hazumi |
| reverb de mola | Autinn Coil |
| rede neural pequena | ChowDSP ChowRNN, sonusmodular Neurosc |
| analisador de espectro | Bogaudio ANALYZER / -XL / RANALYZER |
| sequências matemáticas (Collatz, Fibonacci) | CVfunk Collatz, alefsbits fibb |
| microtonal / escalas Scala | h4n4 Xen Quantizer (já pendente no `TAREFAS.md`) |

Limites: busca por palavras nas descrições — pode faltar ideia descrita com
outros termos, e um "já tem" pode ser parcial (ex.: o mapa contínuo de
padrões de bateria do Topograph diante de SEQUENCE/TRIGSEQ não foi conferido).

## 5. Frente 2 — as coleções que cobrem as lacunas (2026-11-08)

Leitura do código (sem copiar) dos módulos de licença limpa — código livre e
arte livre. Para cada lacuna: o conceito, a origem pública a citar, e o que o
Modular já tem perto. As ideias de módulo são **opções para o autor decidir**
(mecânica e som passam por ele); nada foi implementado.

### 5.1 Dinâmica — compressor, gate, limitador, *duck*

- **Bogaudio PRESSOR** (GPL-3.0+, arte livre; `src/Pressor.cpp`,
  `src/dsp/signal.cpp`): *feed-forward*. O detector lê L+R ou uma mistura
  com a entrada de **sidechain** (*crossfade* DETECTOR MIX), em **RMS ou
  pico**; balística assimétrica (ataque até 500 ms, release até 2 s, curso
  do botão ao quadrado); curva em dB com **joelho suave** de 3 dB; o mesmo
  motor vira **noise gate** (expansor para baixo, joelho de 6 dB); saída do
  envelope do detector. Bogaudio LMTR e NSGT são o mesmo motor com outros
  controles.
- **ZamAudio ZamComp** (GPL-2.0+, de Damien Zammit): compressor de estúdio —
  ataque 0,1–100 ms, release 1–500 ms, joelho até 8 dB, ganho de
  compensação, sidechain, luz de redução de ganho.
- **CVfunk Pressed Duck** (MIT, Cody Geary): mixer de 6 canais com compressão
  no barramento ("press") e **duck** — os canais baixam conforme a entrada de
  sidechain; filtro anti-DC na saída.
- **Origem pública a citar:** a formulação de manual do compressor digital
  (detector, balística, curva com joelho) — D. Giannoulis, M. Massberg,
  J. D. Reiss, "Digital Dynamic Range Compressor Design — A Tutorial and
  Analysis", *JAES* 60(6), 2012.
- **Perto no Modular:** o MASTER já tem limitador (só na saída); o *ducking*
  sai de ENVELOPE/SCOPE em combinação; o LISTEN mede níveis e ataques.
- **Opções para o autor:** (a) um módulo de dinâmica (compressor/gate com
  sidechain e saída de envelope — o envelope vira fonte de modulação, no
  espírito RASGO de que tudo é sinal); (b) um *duck* dentro do MIXER, por
  canal, alimentado por qualquer sinal; (c) nada novo — documentar a receita
  com os módulos que existem (fase didática).

### 5.2 Arpejador

- **Bogaudio ARP** (GPL-3.0+): entrada polifônica de notas; modos UP, DOWN,
  UP-DOWN, UP-DOWN com repetição das pontas, NA ORDEM tocada, ALEATÓRIO e
  EMBARALHADO (sem repetir até esgotar); duração do gate; HOLD/latch.
- **Amalgamated Harmonics Arp 3.x** (BSD-3): padrões que **divergem e
  convergem** a partir de um centro, em passos de escala (maior/menor),
  com comprimento, deslocamento e repetição; o 3.1 arpeja sobre um acorde.
- **Origem pública:** o arpejador de sintetizador (Roland Jupiter-4, 1978;
  ARP 2500/Odyssey) e os modos clássicos; não há algoritmo proprietário.
- **Perto no Modular:** CHORD e HARMONY geram as notas, SEQUENCE/TRIGSEQ o
  tempo, PHRASE o motivo — mas não há quem percorra as notas de um acorde.
- **Opções para o autor:** (a) um ARP que lê o CHORD/HARMONY e percorre as
  notas no clock (modos clássicos + divergir/convergir); (b) um modo "arpejo"
  dentro do CHORD; (c) só a receita didática com SEQUENCE.

### 5.3 Áudio → altura (*pitch tracking*)

- **Cardinal Audio To CV Pitch** (GPL-3.0+, Bram Giesen e Filipe Coelho):
  usa a biblioteca **aubio** (Paul Brossier, GPL-3.0) com o algoritmo
  **YIN** ("yinfast"); limiar de confiança, tolerância, silêncio em −30 dB,
  ajuste de oitava; mantém a última altura confiável.
- **Origem pública:** A. de Cheveigné, H. Kawahara, "YIN, a fundamental
  frequency estimator for speech and music", *JASA* 111(4), 2002.
- **Perto no Modular:** o LISTEN — o roteiro dele já prevê, depois das três
  regiões, a fundamental do grave e a altura predominante (`TAREFAS.md`,
  "Novo módulo de escuta"). O caminho natural é YIN reescrito dentro do
  LISTEN (uma dependência externa como a aubio contraria a regra de
  instrumentos autônomos e pediria vendorizar a biblioteca inteira).

### 5.4 Polifonia

615 módulos do Cardinal são polifônicos (cabos de até 16 canais, herança do
VCV Rack 2). No Modular os cabos são mono e NOTE-OUT/SIGNAL-IN são
last-note. Não é um módulo a acrescentar: é decisão de arquitetura (o
`SignalGraph` precisaria de cabos com N canais e cada módulo, de motores por
canal — como o `Engine` por canal do Bogaudio). Fica registrada como
pergunta ao autor, não como lacuna a preencher.

## 6. Frente 3 — a arquitetura de host (2026-11-08)

Leitura de `README.md`, `docs/OVERVIEW.md` e da árvore `src/` do Cardinal.

**Como o Cardinal é montado:**

- **Um binário só, tudo dentro.** Rack, as 85 coleções e os utilitários são
  compilados juntos e estaticamente; o Cardinal **não carrega módulos de
  fora** nem fala com a loja do VCV. É o mesmo princípio do Modular (um
  instrumento autocontido, sem plugins de terceiros em tempo de execução).
- **O Rack é usado, não bifurcado.** O código original entra como está;
  cabeçalhos em `include/` são incluídos antes dos do Rack para trocar
  detalhes de implementação, e `deps/` refaz o build das dependências de
  forma estática e portável.
- **DPF faz os formatos de plugin.** O DISTRHO Plugin Framework (ISC)
  cuida de LV2, VST2, VST3, AU e CLAP (este ainda em andamento) e do
  aplicativo autônomo. Formato novo se acrescenta no DPF, não no Cardinal.
- **Os módulos "Core" do Rack foram trocados por módulos do host:**
  Host Audio, Host MIDI (notas, CC, gate, mapa), Host CV, Host Parameters
  (24 parâmetros de automação do host viram CV), Host Time (transporte do
  host). Ou seja: o mundo de fora entra no patch **como módulos** — a mesma
  ideia da identidade do Modular ("MIDI, áudio e CV são nós adaptadores
  opcionais", `RASGO_MODULAR.md §35.4.1`).
- **Variantes pelo papel no host.** "main" (8 entradas/saídas de áudio e 10
  de CV; fora de AU/VST2, que não têm CV), **Synth** (instrumento, 2
  saídas) e **FX** (efeito, 2 entradas e 2 saídas) — o mesmo plugin, porque
  muitos hosts exigem que instrumento e efeito sejam produtos separados.
  Há ainda Mini (para a web/recursos baixos), Native (autônomo) e Remote
  (controle por OSC).
- **Outros hosts dentro do patch:** os módulos Carla e Ildaeil hospedam
  plugins (LV2/VST) dentro do Cardinal — o caminho inverso.
- **Licenças:** só coleções compatíveis com a GPLv3 entram; a marca VCV foi
  retirada para evitar problema de marca registrada.

**O que isso sugere para o Modular** (opções para o autor):

1. **Um alvo de plugin com o JUCE, não com o DPF.** O Modular já tem o
   núcleo sem framework e o aplicativo em JUCE (`apps/juce/`); o JUCE faz
   VST3, AU e LV2 (CLAP por extensão) com o mesmo código de aplicativo, sob
   a mesma via AGPLv3 já usada. O DPF só valeria se o JUCE pesasse demais
   para um plugin.
2. **Os nós de fora no papel de "módulos do host":** no modo plugin,
   SIGNAL-IN recebe o áudio e o MIDI do host, NOTE-OUT manda MIDI de volta e
   o MASTER entrega a saída — sem mudar o motor (o Modular continua soando
   sozinho, como instrumento autônomo).
3. **Duas variantes desde o começo, Synth e FX**, como o Cardinal, pela
   mesma razão (os hosts separam instrumento e efeito).
4. **Parâmetros do host como fonte de modulação** (como o Host Parameters) e
   **o transporte do host** sincronizando o CLOCK (como o Host Time).
5. Nada disso pede código do Cardinal: é arquitetura, e o JUCE já cobre os
   formatos.

Para seguir: o autor decide se o Modular ganha um alvo de plugin e, se
ganhar, em que ordem (Synth primeiro, por ser o caso de uso de um
instrumento que soa sozinho).
