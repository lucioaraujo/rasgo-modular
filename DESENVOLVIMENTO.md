# Rasgo Modular — notas de desenvolvimento

> Este era o `README.md` até 2 out. 2026. Passou a se chamar
> `DESENVOLVIMENTO.md` (com `git mv`, histórico preservado) quando o
> `README.md` virou a apresentação pública em quatro idiomas, como pede o
> portão editorial da `ESTRATEGIA_DE_PUBLICACAO.md`. O conteúdo abaixo é
> o registro de trabalho, em português, e não mudou.

**Identificador arquivístico:** `ARQ-RSM-001` — ingresso e manifesto em
`RASGO_ARQUIVO/INGRESSO_ARQ-RSM-001.md`, no acervo local do autor (fora
deste repositório)
**Estado:** protótipo C++17 — **marco 1** (2026-09-01): fundação
`SignalGraph` + 6 módulos DSP + peça de 40 s. **marco 2**
(2026-09-02): + `MEMORY` (granular/freeze) + `TURING` (registrador) +
`MATTER` (modal) + `SPACE` (multitap) + `STRING` (corda/guia-de-onda) +
`QUANTIZER` (escala) + `PARAMETRIC` (EQ, RBJ) + `HARMONY` (movimento
harmônico — 6 técnicas reais) + `SEQUENCE` (sequenciador de passos
editável, 5 modos de leitura) + `MIXER` (4 canais, pan de potência
constante) + `MASTER` (largura mid/side, bloqueio de DC, limitador, VU) +
**modelo de conexão de 3 camadas** (matriz + constelação + semântico,
serializado no patch) + 3 peças (40/50/55 s) + **painel gráfico de teste**
(`apps/panel/`, X11 + ALSA) + `OSC` (oscilador subtrativo antialias, 1
V/oct, 5 formas, sync, TZFM) + `NOISE` (branco/rosa/brown + S&H) + `VCA`
(duplo, linear/exp, atenuverter) + `CONTROL` (atenuversor/offset/slew/
retificação/soma de CV) + `LOGIC` (÷/× de clock, AND/OR/XOR, flip-flop,
gate delay). **marco 3** (2026-09-04): + `SH` (S&H duplo) + `SHAPE`
(wavefolder/ring-mod) + `LPG` (low-pass gate a vactrol) + `CHORD` (VCO
parafônico) + `DRIFT` (campo de deriva) + `SWITCH` (chave/mux-demux) +
`SCOPE` (medidor/osciloscópio) + `TRIGSEQ` (grade de gates) + `ABACUS`
(aritmética/lógica de CV) + `WASP` (filtro áspero) + `MATRIX` (matriz de
roteamento 4×4) + `MULT` (múltiplo processado); **`connectToParameter`
aditivo** (a modulação soma sobre o knob) + **antialiasing** (helper
`Oversampler2x` 2× + ADAA) + **displays por módulo no painel** (grade
clicável da MATRIX, espectro do SCOPE, lanes do TRIGSEQ) + profundidade
por módulo (op bit a bit, slew assimétrico, voice-leading, ring-mod,
memória de topologia…) + **`MotionEngine`** (`apps/panel/`, protótipo de
composição generativa — parâmetros com comportamento `WALK`/
`OSCILLATE`/`ATTRACT` no tempo, aditivo sobre o patch) + **Patch
Genetics** (`MUTATE`/`EVOLVE`/`CROSS`/`FREEZE`,
`apps/panel/PatchGenetics.hpp` — edita um patch já existente preservando
topologia; `CROSS` alinha por tipo de módulo entre dois patches) +
**`ScoreRecorder`** (`SYSTEM SCORE` — registro determinístico de
conexões e mudanças de parâmetro, texto byte-idêntico entre renders —
+ `MUSICAL SCORE`, captura de notas via `NOTE-OUT`) + **hover-learn**
(`apps/panel/LearnCatalog.hpp` — caixa estilo terminal sempre presente
no rodapé da coluna esquerda, silenciosa; os 37 módulos com texto de 3
níveis; ver `dossies/ESTUDO_seed_composicao_generativa.md`).
**(2026-09-05):**
+ `AUDIO-IN` (entrada de áudio ao vivo, ALSA) + `CHAOS` (campo caótico
de poço duplo) + `PLL` (segundo oscilador, malha de fase + rede de
feedback selecionável) + `NOTE-OUT` (adaptador que captura o contrato
`NOTE` do `MUSICAL SCORE`) + gramática do `Seed` nomeada e explícita
(`apps/panel/SeedGrammar.hpp`, refactor comprovado byte a byte —
ver `dossies/ESTUDO_seed_composicao_generativa.md §1.1`)
+ **body guard** na saída (`OutputStage.hpp` — governador de agudo
áspero) + `MASTER.mute` (rampa). **(2026-09-06):**
+ **cabeçalho de linha única** modelo RASGO Synth (logo anti-aliased +
barra de comandos em botões, toggles com anel · pico da saída ·
`SEED`/`REC`/`STANDBY` · `IDIOMA`/`TUTORIAL`/`SOBRE`)
+ **i18n do painel** (`apps/panel/UiLanguage.hpp` — EN padrão, PT/FR/ES
no botão `IDIOMA`; cabeçalho/tutorial/créditos traduzidos; rótulos de
módulo não; LEARN em fases)
+ **passe de ergonomia** dos 38 painéis (`design.md §3.2.1` — larguras
enxutas, displays cheios e mais altos, colisões de rótulo de jack
resolvidas)
+ roadmap de continuidade (`PESQUISA_MODULOS.md §2.4`, 4 ondas) e a
Onda A (completa): `GLIDE` (Módulo 39 — portamento por nota: slide 303,
legato, `fall` assimétrico) + `WAVETABLE` (Módulo 40 — oscilador de
tabela procedural, `warp` tipo WAVE CUT, captura de ciclo ao vivo do
`AUDIO-IN`) + `LOOPER` (Módulo 41 — delay de linha com HOLD/REVERSE
sem clique e caráter de fita/BBD num knob `age`). Onda B (completa):
`ADDITIVE` (Módulo 42 — oscilador aditivo, 64 parciais) + `PLANAR`
(Módulo 43 — morph vetorial XY, gesto gravável) + `OPERATOR` (Módulo 44
— FM de 4 operadores, 8 algoritmos, feedback DX7) + `FORMANT` (Módulo 45
— 5 passa-faixas paralelos, morph de vogais A→E→I→O→U). Onda C (completa):
`HALL` (Módulo 46 — reverb FDN de 8 linhas + matriz de Householder,
`freeze`, estéreo) + `DRUM` (Módulo 47 — voz de percussão, corpo com
pitch-sweep + estalo, mapa 808↔909↔acústico, `roll`). Onda D (completa):
`SAMPLER` (Módulo 48 — toca-fatias com varispeed/reverse/repitch/wear;
camada `io/` + `dr_wav`) + `SIGNAL-IN` (Módulo 49 — o `AUDIO-IN` cresceu:
áudio + MIDI num adaptador, voz mono last-note, saídas pitch/gate/vel/cc)
+ `TURNTABLE` (Módulo 50 — o buffer do `SAMPLER` lido por um prato com
inércia: torque de motor, atrito, mão na CV `scratch`, `wear`
determinístico; o `TAPE` virou `heads` no `LOOPER`).
**As 4 ondas do roadmap `§2.4` fechadas.** Pós-roadmap: `BOXCAR` (Módulo
51 — *boxcar averager* / integrador de porta: janela + delay + média de N
capturas, `scan` reconstrói a onda, `geiger` = trem de Poisson livre;
inspirado no AI Synthesis AI250 BXR) + o `NOISE` ganhou o modo `poisson`.
**Onda E** (`§2.5`) completa: `SWIRL` (Módulo 52 — chorus/flanger/ensemble/
phaser, a família de MODULAÇÃO que faltava) + `CRUSH` (Módulo 53 —
destruidor lo-fi: redução de taxa/bits, wrap de inteiro, glitch, jitter;
o verbo DAMAGE) + `STAGES` (Módulo 54 — gerador de N segmentos
configuráveis: envelope, LFO ou sequência conforme a fiação; Mutable
Stages / Rossum Control Forge) + `RESONATOR` (Módulo 55 — banco de modos
afinados excitado por sinal externo, saídas low/mid/high que se cruzam;
Rings/Elements + Three Sisters) + `PULSAR` (Módulo 56 — síntese pulsar de
Curtis Roads: trem de pulsarets com altura e timbre em duas frequências
independentes; masking e jitter semeados).
**Onda F** (`§2.6`, análise → síntese) começou: `SPECTRA` (Módulo 57 —
resíntese espectral: ouve um som, acha os parciais e re-oscila como um
banco de senóides que o segue; `blur`, `freeze` = *spectral freeze*,
`shift`/`stretch` transpõem a re-síntese; Panharmonium / phase vocoder;
autônomo sem entrada) + `SHIFTER` (Módulo 58 — deslocador de frequência
SSB: move o espectro inteiro por um Δf fixo em Hz → inarmônico; saídas
`up`/`down`, `feedback` = barber pole de Risset; Bode/Moog) + `VCA4`
(Módulo 59 — banco de 4 VCAs + mixer somado; Veils/Quad VCA) + `VOCODER`
(Módulo 60 — vocoder DEDICADO de até 20 bandas: a portadora fala o
modulador; `sibilance` pra as fricativas, `freeze` = pad falado; o
`FORMANT` já tinha um `mode` vocoder de 5 bandas). **Onda F completa
(Dudley 1938 pra os dois).**
+ **painel — navegação e tutorial (2026-09-08):** caixa do número do seed
vira **campo de texto padrão** (selecionar/copiar/colar/apagar, botão do
meio cola; clipboard ICCCM completo; o seed também sai no terminal);
**rolar o rack ao cabear** (arrastar o cabo pra borda) e **botão do meio
paneia**; **TUTORIAL reescrito e rolável** — cobre o cabeçalho botão a
botão, gravar (as gravações vão pra `~/Music/RasgoModular/`, nome por
data/hora — desde 2026-09-11), adicionar/remover módulos e **as 8
famílias**, nas 4 línguas; pans do MIXER sempre no centro em todo seed.
+ **painel — vista do rack (2026-09-09):** botão `RACK` no cabeçalho
alterna **TODOS** os módulos ↔ só os que **chegam à saída**
(`SignalGraph::nodesFeeding`); é só uma vista, persistida, com reflow ao
vivo ao cabear.
+ **fase didática completa (2026-09-09 → 12):**
[`guia/`](guia/00_indice.md) — guia para quem toca, em 4 partes:
mentalidade (`COMO_PENSAR.md`), funcionamento (`CABEAMENTO.md` +
`RELACAO_DE_CABO.md`), **os 58 módulos um a um** (jack a jack, "como
cabear", "potencializar"), o apêndice de equivalências Eurorack e o
caderno de **10 receitas** de patch passo a passo — todos com um passe
de aprofundamento didático (o conceito por trás de cada parâmetro, não
só a fórmula). Falta só a tradução, quando o site for construído.
+ **taxonomia consolidada** (18 verbos → 8 famílias). **Rack
de partida completo**;
58 módulos DSP,
80 alvos CTest
verdes (Debug + Release), 5 peças de exemplo byte-idênticas. Custodiante:
Lúcio de Araújo.
**Licença do código:** GNU AGPLv3 ou posterior — ver [`LICENSE`](LICENSE)
(decisão do autor, 2026-09-01: "a mesma que temos usado" → a licença
habitual da família RASGO). Código de terceiros só entra sob licença
livre compatível com AGPLv3 (MIT/BSD/ISC/Apache-2.0/LGPL/GPL/AGPL) e com
proveniência registrada.

## Build e plataformas

Duas interfaces, papéis diferentes:

| | `apps/juce/` — **produção** | `apps/panel/` — **teste** |
|---|---|---|
| Plataformas | Linux · Windows · macOS | Linux (X11 + ALSA) |
| Papel | o que vai em release | ferramenta de desenvolvimento |
| Estado | paridade funcional alcançada (`apps/juce/PARIDADE.md`) | completo |

O app JUCE tem dois recursos que o painel X11 não tem, porque dependem de
API multiplataforma: **ABRIR** um `.rmp` por seletor de arquivo nativo, e
**desfazer** (`Ctrl+Z`) — um anel de fotografias serializadas do grafo que
cobre cada cabo ligado ou cortado.

**Motor + testes** (sem dependência de GUI, os três sistemas):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
```

**Front-end JUCE.** O JUCE não é versionado aqui (política da família
RASGO: nem submodule, nem FetchContent) — aponte um checkout local:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRASGO_MODULAR_JUCE_PATH=/caminho/para/JUCE
cmake --build build --target RasgoModularApp -j
cd build && cpack -G DEB      # ou NSIS (Windows) / DragNDrop (macOS)
```

Instruções completas, matriz de plataformas e variáveis de ambiente:
[`INSTALL.md`](INSTALL.md). Licenciamento do JUCE:
[`apps/juce/LICENSE_STATUS.md`](apps/juce/LICENSE_STATUS.md); créditos e
fontes: [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md); estado de
prontidão para publicação e procedimento de correção/retirada:
[`PUBLICACAO.md`](PUBLICACAO.md).

**Excelência de saída.** O instrumento segue a arquitetura comum da
família (`RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md`):
proteção de excelência no `MASTER` (finitude, DC, guarda ultrassônica,
governador de corpo, limitador look-ahead por pico verdadeiro, teto
−1 dBFS) e uma **guarda de segurança no sink**, que não pode ser
contornada nem desligada — o MASTER é um módulo, e dá pra cabear por fora
dele. Medição BS.1770-4 (momentary, short-term, integrated com as duas
portas) em `src/dsp/Loudness.hpp`, com fixtures de conformidade EBU no
`ctest`. Taps de gravação nomeados: o REC captura de `post-safety` (padrão) ou
`pre-safety`, por `RASGO_REC_TAP` — gravar só depois do limitador faz ele
esconder a dinâmica que se queria examinar. **Ainda fora:** exportação
PCM24 e float, que depende de um alvo de publicação declarado.

**Painel de teste (só Linux):** `./.run_rasgo_modular.sh` — precisa de
`libX11` e `libasound`; o alvo é pulado automaticamente se faltarem.

Matriz de plataformas: Linux é a única **verificada em hardware real**
hoje. Windows e macOS são construídos e empacotados pela CI
(`.github/workflows/package.yml`, com checagem de Universal 2 e
deployment target no macOS), ainda **não abertos em máquina real** — a
mesma condição em que o Antitotem foi publicado, e uma decisão a
registrar explicitamente antes de qualquer release.

## Apresentação

Rasgo Modular é o ambiente modular próprio da família RASGO: instrumento,
laboratório de síntese, composição e performance, com potencial de oferecer
infraestrutura reutilizável sem absorver a identidade dos instrumentos irmãos.
A antiga “Fábrica de Módulos” permanece como camada funcional e antecedente
conceitual, não como um projeto separado.

## Retomada e fontes locais

- [`RASGO_MODULAR.md`](RASGO_MODULAR.md) — arquitetura, taxonomia, fluxos,
  decisões e pesquisa; §29 = proveniência e licença;
- [`TAREFAS.md`](TAREFAS.md) — estado operacional, testes e próximo marco
  ("Registro da etapa — 2026-09-01");
- [`PESQUISA_MODULOS.md`](PESQUISA_MODULOS.md) — pesquisa de módulos e ordem
  de execução (§2);
- [`dossies/`](dossies/) — um dossiê por módulo (problema, fontes,
  modelo, testes);
- [`guia/`](guia/00_indice.md) — guia de referência **para quem toca**:
  uma página em prosa por módulo (fase didática, `PESQUISA_MODULOS.md
  §2.7`; iniciada 2026-09-09, piloto `OSC`/`MULT`/`SPACE`);
- `src/core/SignalGraph.hpp` = grafo de áudio; `src/dsp/*` = os 58 módulos de rack (mais alguns auxiliares que não aparecem no catálogo: medição de loudness e true-peak, oversampler, pitch-shifter, estágio de saída)
  (+ `Oversampler.hpp`, helper 2× compartilhado);
  `examples/*` = as 5 peças; `apps/panel/` = painel gráfico de teste
  (+ `MotionEngine.hpp`, protótipo de composição — ver
  `dossies/ESTUDO_seed_composicao_generativa.md`);
  `CMakeLists.txt` + `tests/` = 72 alvos CTest.

O `src/core/Graph.hpp` (grafo escalar/multimodal do lote de agosto) segue
existindo; o áudio real do marco 1 foi construído à parte em `SignalGraph`.
Scheduler stateful, realtime safety ampliada, front-ends e segundo
consumidor continuam pendentes (marco 2).

## Limites e promoção

Nenhum módulo é promovido ao comum apenas por semelhança. Antes de reutilização
externa, confirmar autoria, licença, dependências, contrato, testes e um
segundo consumidor plausível. O núcleo não deve receber UI, processos externos
ou integração entre instrumentos antes de estabilizar o contrato de dados.

## Próxima tarefa arquivística

**Feito no marco 1 (2026-09-01):** ingresso `ARQ-RSM-001` atualizado,
licença registrada (AGPLv3-or-later), manifesto de integridade em
`RASGO_ARQUIVO/MANIFESTO_ARQ-RSM-001_marco-1_2026-09-01.md`, no acervo
local do autor (inventário somente leitura + SHA-256).

**Pendente (só quando o estado deixar de ser móvel):** cópia de
preservação congelada + teste de restauração — atrelar a um commit/tag do
marco, à migração de diretórios ou ao handoff do marco 2. Enquanto o
protótipo evolui, basta atualizar `TAREFAS.md`, este README e o ingresso;
não se cria pacote de preservação a cada etapa (governança
`VM_STUDIO_ARCHIVE.md §167-181`).
