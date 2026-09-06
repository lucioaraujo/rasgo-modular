# RASGO MODULAR

**Documento vivo de arquitetura e desenvolvimento**  
**Versão:** 0.1  
**Data:** 2026-08-16

---

## 1. Objetivo

**Rasgo Modular** será um ambiente modular próprio da família Rasgo.

O conceito incorpora a antiga proposta da **Fábrica de Módulos**, registrada em
[`0_BRAINSTORM/META_INSTRUMENTO_FABRICA_DE_MODULOS.md`](../0_BRAINSTORM/META_INSTRUMENTO_FABRICA_DE_MODULOS.md).
“Fábrica” permanece como uma camada funcional do projeto — oficina,
laboratório e construtor de instrumentos —, não como um segundo instrumento.

O projeto deve servir simultaneamente como:

- instrumento;
- laboratório de síntese e processamento;
- ambiente de composição;
- sistema de performance;
- plataforma de pesquisa;
- biblioteca de módulos reutilizáveis;
- base tecnológica para outros instrumentos Rasgo.

Um objetivo central é maximizar o **reaproveitamento de código**.

## 1.1 Critério de excelência do instrumento

O `RASGO_MODULAR`, assim como os demais instrumentos RASGO, não será avaliado
apenas por funcionar como software. Cada marco deve buscar excelência integrada
em:

- conceito e coerência poética;
- musicalidade, escuta e capacidade composicional;
- criatividade, jogo e prazer de exploração;
- experimentação e abertura a comportamentos inesperados;
- arquitetura, código, testes e manutenção;
- interface, legibilidade e qualidade da interação;
- qualidade de áudio, desempenho, estabilidade e realtime safety;
- documentação, proveniência, licença e reprodutibilidade;
- possibilidade de performance, criação de obras e conexão com outros
  instrumentos sem perder identidade.

O graph engine é uma fundação necessária, mas não suficiente. Um módulo ou
recurso só está pronto quando sua implementação técnica sustenta uma experiência
musical, criativa e jogável coerente com o instrumento.

Cada módulo deve, sempre que possível, ser concebido como um componente independente que possa ser reutilizado em:

- Rasgo Modular;
- instrumentos standalone;
- plugins;
- instalações;
- sistemas performativos;
- ferramentas experimentais;
- protótipos Pure Data/Faust/C++;
- futuros dispositivos de hardware.

---

## 2. Relação com o Atlas

O arquivo:

`ATLAS_DE_REFERENCIA_RASGO.md`

é a base de pesquisa.

Ele registra:

- projetos de referência;
- centros de pesquisa;
- algoritmos;
- licenças;
- métodos;
- conceitos;
- soluções existentes.

Este documento, `RASGO_MODULAR.md`, registra:

- decisões arquiteturais;
- taxonomia;
- fluxos;
- famílias de módulos;
- APIs;
- componentes reutilizáveis;
- roadmap;
- experimentos;
- implementação.

A lista operacional de etapas está em [`TAREFAS.md`](TAREFAS.md).

Cada etapa do desenvolvimento deve registrar objetivo, decisão, arquivos
afetados, validação executada, limitações e próximo passo. O documento vivo não
deve afirmar uma etapa como concluída sem evidência correspondente.

Antes de iniciar uma pesquisa ou módulo novo, consultar o [Inventário global
de módulos e mecanismos RASGO](../RASGO_DOCUMENTATION/architecture/INVENTARIO_GLOBAL_MODULOS.md).
Toda descoberta ou implementação com possível valor para outro projeto deve
ser registrada nele no mesmo incremento.

Regra:

```text
ATLAS
  ↓
pesquisa / referência
  ↓
RASGO MODULAR
  ↓
arquitetura / implementação
```

---

## 3. Princípio de classificação

A análise do Hexen mostrou uma ideia extremamente útil:

> classificar módulos pela **função que desempenham dentro do rack**, e não apenas pela tecnologia ou pelo tipo de circuito.

Rasgo Modular adota esse princípio, mas desenvolve uma taxonomia própria.

Pergunta principal:

> **“o que quero fazer com o fluxo agora?”**

---

## 4. Taxonomia inicial

```text
RASGO MODULE TAXONOMY
│
├── SOURCE
│
├── INPUT / GESTURE
│
├── TIME
│
├── DECISION
│
├── SEQUENCE
│
├── TRANSFORM
│
├── ROUTE
│
├── MEMORY
│
├── DAMAGE
│
├── REPAIR
│
├── RELATION
│
├── MATTER
│
├── SPACE
│
├── PERCEPTION
│
├── INFERENCE
│
├── UTILITY
│
├── MIX
│
└── METER
```

---

## 5. SOURCE

Produz ou introduz matéria inicial.

Exemplos:

- oscillator;
- noise;
- sampler;
- microphone;
- file;
- granular source;
- wavetable;
- physical model;
- corpus source;
- neural generator.

---

## 6. INPUT / GESTURE

Transforma ação humana ou externa em controle.

Exemplos:

- MIDI;
- teclado;
- pads;
- touch;
- mouse;
- sensores;
- câmera;
- movimento;
- gesto;
- OSC;
- hardware;
- controlador físico.

---

## 7. TIME

Produz estruturas temporais.

Exemplos:

- clock;
- divider;
- multiplier;
- duration;
- pulse;
- swing;
- drift;
- irregular clock;
- phase;
- temporal field.

Princípio:

> tempo não é sinônimo de sequência.

---

## 8. DECISION

Produz escolhas, condições e bifurcações.

Exemplos:

- chance;
- Bernoulli;
- comparator;
- logic;
- rule;
- threshold;
- branch;
- probabilistic gate;
- conflict;
- conditional routing.

---

## 9. SEQUENCE

Organiza eventos.

Exemplos:

- step;
- grid;
- pattern;
- roll;
- random sequence;
- chord sequence;
- physical sequence;
- generative path;
- recorded sequence;
- recursive sequence.

Princípio:

> sequenciar é uma família de comportamentos.

---

## 10. TRANSFORM

Transforma diretamente um sinal.

Exemplos:

- filter;
- distortion;
- delay;
- reverb;
- bit reduction;
- granular;
- spectral;
- modulation;
- pitch;
- dynamics.

---

## 11. ROUTE

Controla caminhos.

Exemplos:

- split;
- merge;
- multiplexer;
- switch;
- matrix;
- send;
- return;
- crosspoint;
- conditional route;
- feedback route.

---

## 12. MEMORY

Armazena, lembra ou recupera.

Exemplos:

- buffer;
- delay memory;
- freeze;
- history;
- snapshot;
- state memory;
- corpus;
- loop;
- residue;
- ghost connection.

---

## 13. DAMAGE

Introduz desgaste ou ruptura.

Exemplos:

- dropout;
- degradation;
- saturation;
- instability;
- cable fatigue;
- crack;
- erosion;
- corruption;
- interruption;
- rupture.

---

## 14. REPAIR

Reconstrói ou cicatriza.

Exemplos:

- reconnect;
- interpolation;
- inpainting;
- repair;
- recovery;
- regeneration;
- reconstruction;
- probabilistic healing.

---

## 15. RELATION

Processa a relação entre dois ou mais sinais.

Exemplos:

- cross modulation;
- ring modulation;
- contamination;
- interaction;
- morph;
- spectral transfer;
- relational waveshaping;
- feedback relationship.

Princípio:

> a conexão pode ser o próprio DSP.

---

## 16. MATTER

Transforma sinal em comportamento físico virtual.

Exemplos:

- string;
- membrane;
- plate;
- resonator;
- modal body;
- waveguide;
- friction;
- impact;
- tension;
- density.

---

## 17. SPACE

Transforma posição e campo espacial.

Exemplos:

- pan;
- trajectory;
- Ambisonics;
- binaural;
- diffusion;
- distance;
- spatial fragmentation;
- room;
- field.

---

## 18. PERCEPTION

Analisa o conteúdo do sinal.

Exemplos:

- onset;
- pitch;
- loudness;
- centroid;
- spectrum;
- rhythm;
- key;
- similarity;
- psychoacoustic descriptors.

O resultado da análise pode controlar outros módulos.

---

## 19. INFERENCE

Utiliza modelos que completam, inferem, transformam ou geram material.

Exemplos:

- neural audio;
- latent models;
- diffusion;
- source reconstruction;
- source separation;
- audio inpainting;
- generative continuation.

---

## 20. UTILITY

Operações pequenas e indispensáveis.

Verbos fundamentais:

```text
somar
subtrair
multiplicar
dividir
selecionar
copiar
inverter
segurar
limitar
quantizar
comparar
converter
mapear
normalizar
desviar
condicionar
```

Essa família deve ser extensa.

Utilities formam a **gramática** do sistema.

---

## 21. MIX

Combina fluxos.

Exemplos:

- mixer;
- crossfade;
- bus;
- matrix mixer;
- stereo mixer;
- multichannel mixer;
- summing;
- output stage.

---

## 22. METER

Observa e mede.

Exemplos:

- scope;
- spectrum;
- phase;
- level;
- correlation;
- history;
- event monitor;
- state monitor.

---

## 23. Conexão Rasgo

A conexão deve ser tratada como entidade do sistema.

Definição atual:

```text
CONEXÃO RASGO
=
SINAL
+
PROCESSO
+
ESTADO
+
HISTÓRIA
+
REGRAS
+
PERCEPÇÃO
+
INFERÊNCIA
+
MATÉRIA
+
ESPAÇO
+
GESTO
```

Ela não é apenas um fio gráfico.

Pode possuir:

- ganho;
- atraso;
- ruído;
- saturação;
- memória;
- probabilidade;
- idade;
- desgaste;
- tensão;
- posição;
- trajetória;
- comportamento;
- estado de ruptura;
- cicatriz.

---

## 24. Estados de conexão

```text
CONNECT
│
├── estável
├── instável
├── intermitente
├── degradado
├── parasita
├── contaminado
├── saturado
├── atrasado
├── memória
├── feedback
├── ruptura
├── cicatriz
└── reconstrução
```

---

## 25. Caminhos internos patcháveis

Módulos podem expor fluxos internos.

Exemplo:

```text
        ┌──── PROCESS ────┐
        │                 │
IN → MEMORY ───────────→ OUT
        ↑                 │
        └── FEEDBACK ─────┘
```

O usuário pode inserir módulos no processo interno.

Isso transforma módulos fechados em **estruturas atravessáveis**.

---

## 26. Arquitetura orientada ao reaproveitamento

Objetivo:

```text
rasgo_core/
rasgo_dsp/
rasgo_modules/
rasgo_ui/
rasgo_io/
```

Hipótese:

```text
rasgo_core/
├── graph
├── state
├── events
├── timing
├── routing
└── serialization

rasgo_dsp/
├── source
├── transform
├── memory
├── damage
├── repair
├── relation
├── matter
├── space
└── perception

rasgo_modules/
├── wrappers
├── metadata
├── ports
├── parameters
└── presets

rasgo_ui/
├── rack
├── panels
├── cables
├── meters
└── interaction

rasgo_io/
├── audio
├── midi
├── osc
├── files
└── sensors
```

---

## 27. Módulo como componente independente

Cada módulo deve tentar separar:

```text
DSP
≠
STATE
≠
UI
```

Modelo:

```text
MODULE
├── processor
├── state
├── ports
├── parameters
├── metadata
└── view
```

Isso permite reutilizar o mesmo DSP em:

- interface modular;
- plugin;
- instrumento standalone;
- render offline;
- hardware;
- servidor headless.

---

## 28. Metadados de módulo

Cada módulo deverá registrar ao menos:

```text
id:
name:
family:
description:
inputs:
outputs:
parameters:
state:
dsp_backend:
source_origin:
license:
dependencies:
reusable:
experimental:
version:
```

---

## 29. Proveniência e licença

**Licença do projeto (decisão do autor, 2026-09-01):** **GNU AGPLv3 ou
posterior** — "a mesma que temos usado", isto é, a licença habitual da
família RASGO (MARAVI, ANTITOTEM, AQUORBIUM). Arquivo `LICENSE` na raiz
do `RASGO_MODULAR/`. Consequências:

- todo código de terceiros incorporado precisa de licença livre
  compatível com AGPLv3 (MIT, BSD, ISC, Apache-2.0, LGPL, GPLv3,
  AGPLv3) — **GPLv3-or-later é compatível** (pode ser incorporado; o
  todo combinado passa a valer como AGPLv3);
- o alvo **web/WASM** ativa a cláusula de rede da AGPL: uma instância
  servida precisa oferecer o código-fonte correspondente aos usuários;
- o motor é framework-free; se um front-end usar JUCE, vale a via
  AGPLv3 do JUCE (mesma situação de AQUORBIUM / RASGO Synth Performance).

Nenhum módulo deve entrar no núcleo reutilizável sem registro de:

- origem;
- autor;
- licença;
- arquivo/repositório;
- alterações;
- dependências;
- compatibilidade.

Classificação:

```text
VERDE
licença permissiva
→ candidato real

AMARELO
copyleft
→ estudo / arquitetura / decisão consciente

VERMELHO
proprietário ou non-commercial
→ referência apenas
```

---

## 30. Fontes técnicas prioritárias

### VERDE / permissivas

- Pure Data / BSD
- DaisySP / MIT
- Mutable STM32F / MIT
- NESS / MIT
- pyfar / MIT
- SPIS / MIT
- Amphion toolkit / MIT
- PiPo / BSD
- iPlug2 / licença permissiva

### REFERÊNCIA / avaliar

- VCV Rack
- Cardinal
- Bespoke Synth
- OpenMusic
- ossia score
- IEM libraries
- RITMO tools
- Essentia
- RAVE
- Hexen

---

## 31. Princípio funcional aprendido com Hexen

A contribuição principal do Hexen para Rasgo Modular não é código.

É a organização funcional do rack.

Hexen mostra que:

> uma boa taxonomia reduz a carga cognitiva e ajuda o usuário a compreender o fluxo.

Rasgo Modular deve levar isso mais longe.

A interface de seleção de módulos deve permitir pensar em verbos:

```text
GERAR
OUVIR
MOVER
MARCAR TEMPO
DECIDIR
SEQUENCIAR
TRANSFORMAR
ROTEAR
LEMBRAR
DANIFICAR
REPARAR
RELACIONAR
MATERIALIZAR
ESPACIALIZAR
PERCEBER
INFERIR
MEDIR
MISTURAR
```

---

## 32. Prioridade inicial de implementação

Primeira camada mínima:

```text
SOURCE
TIME
DECISION
SEQUENCE
TRANSFORM
ROUTE
UTILITY
MIX
METER
```

Segunda camada — identidade Rasgo:

```text
MEMORY
DAMAGE
REPAIR
RELATION
MATTER
SPACE
PERCEPTION
```

Terceira camada:

```text
INFERENCE
GESTURE avançado
sistemas aprendíveis
modelos neurais
```

---

## 33. Estratégia de desenvolvimento

Evitar começar com dezenas de módulos complexos.

Primeiro construir:

1. graph engine;
2. modelo de portas;
3. modelo de parâmetro;
4. roteamento;
5. clock/event engine;
6. state serialization;
7. UI mínima de rack;
8. cable engine;
9. utilities;
10. primeiros módulos DSP.

Depois adicionar comportamentos Rasgo.

---

## 34. Valor estratégico

Rasgo Modular pode se tornar a **infraestrutura comum** dos futuros instrumentos Rasgo.

Em vez de cada instrumento reconstruir:

- osciladores;
- buffers;
- filtros;
- clocks;
- moduladores;
- routing;
- meters;
- MIDI;
- state;
- automação;

esses componentes poderão existir numa biblioteca comum.

Modelo:

```text
                 RASGO MODULAR CORE
                         │
        ┌────────────────┼────────────────┐
        │                │                │
   instrumento A    instrumento B    instrumento C
```

Isso reduz retrabalho e aumenta coerência entre projetos.

---

## 35. Inventário de reaproveitamento RASGO

O desenvolvimento do Rasgo Modular deve partir do trabalho já realizado nos
instrumentos da família. A fonte transversal é o
[`INVENTARIO_GLOBAL_MODULOS.md`](../RASGO_DOCUMENTATION/architecture/INVENTARIO_GLOBAL_MODULOS.md),
que deve ser consultado e atualizado durante cada avanço relevante.

### 35.1 Pesquisa e princípios disponíveis

| Origem | Material aproveitável | Relação com Rasgo Modular |
|---|---|---|
| **AQUORBIUM** | fontes de energia, organismos, ressonadores, corda física, granular, espaço multitap e módulos com modos autônomo/performance/híbrido | principais candidatos para `SOURCE`, `MATTER`, `SPACE`, `MEMORY` e `GESTURE` |
| **ANTITOTEM** | CMOS/Lunetta, clocks, divisores, scanners, portas de retorno, S&H, memória de topologia, feedback e breadboard virtual | referência central para `TIME`, `ROUTE`, `MEMORY`, `DAMAGE` e conexões atravessáveis |
| **RASGO SYNTH** | osciladores, caos, filtros, envelopes, granular, ressonadores, síntese espectral, sequenciamento, feedback e relações entre módulos | maior catálogo de DSP já implementado; deve ser triado mecanismo por mecanismo |
| **NAVALHA 2** | sources A/B, slices, vozes, sequenciamento, memória, estado, filas, gravação e render offline | referência para `SOURCE`, `SEQUENCE`, `MEMORY`, `STATE` e serviços de áudio |
| **TRIOIO** | decisão, memória, análise-resíntese, síntese energética, proveniência e despacho tipado | referência para `PERCEPTION`, `INFERENCE`, `INPUT`, `MEMORY` e contratos de decisão |

### 35.2 Candidatos técnicos iniciais

Os seguintes componentes são candidatos de investigação, não biblioteca comum
promovida:

- ressonadores modais e cordas físicas do AQUORBIUM;
- granular striker e granular cloud;
- espaço multitap e memória espacial;
- osciladores, filtros, envelopes, mixers e medidores já implementados;
- clocks, divisores, sequenciadores e utilidades de decisão;
- portas de feedback, retificação, memória e ruptura do ANTITOTEM;
- relações de cross-modulation, contaminação e influência do RASGO Synth;
- filas fixas, snapshots, serialização e contratos de realtime;
- gravação, render offline e medição de saída quando forem independentes da UI.

Cada candidato precisa ser reescrito ou adaptado para o contrato do Rasgo
Modular. O código não deve ser incluído diretamente por semelhança de nome.
Antes da promoção, verificar API, estado, unidades, sample rate, limites de
CPU/memória, licença, dependências e testes.

### 35.3 Ordem de extração

1. inventariar as classes concretas e seus testes nos projetos de origem;
2. separar DSP, estado, portas, parâmetros, metadados e interface;
3. escolher um primeiro módulo pequeno e independente;
4. adaptá-lo ao graph engine do Rasgo Modular;
5. validar comportamento e realtime safety;
6. testar o mesmo componente em um segundo instrumento;
7. somente então avaliar promoção ao núcleo compartilhado no G4.

Até o G4, o componente pode ser usado como protótipo local do Rasgo Modular,
mas não deve ser anunciado como biblioteca comum da família.

### 35.4 Limites de reaproveitamento

Devem permanecer específicos dos projetos de origem, salvo nova decisão
documentada:

- identidades visuais e metáforas de interface;
- estado musical privado de cada instrumento;
- organismos e regras ecológicas próprias do AQUORBIUM;
- relações históricas e linguagem material específicas do ANTITOTEM;
- arco composicional e mecanismos de peça específicos do RASGO Synth;
- formato de projeto e workflow editorial específicos do NAVALHA 2;
- decisões conceituais próprias do TRIOIO.

O objetivo é reaproveitar trabalho e conhecimento, não dissolver a autoria dos
instrumentos.

## 35.4.1 Autonomia e versatilidade (decisão do autor, 2026-09-02)

**O Rasgo Modular soa sozinho** — é autônomo como um sintetizador. MIDI e
entrada de áudio **podem** existir, mas como **módulos-adaptadores
opcionais** (só mais um nó `Signal`), nunca como dependência do motor.
Toda peça deve poder tocar com **zero entrada externa**.

Já é assim na prática: cada módulo tem os "três modos obrigatórios"
(autônoma / performance / híbrida — `MODULE_DEVELOPMENT_STANDARD`), e o
autônomo é obrigatório; as três peças em `examples/` rodam sem nenhuma
entrada. `CLOCK`, `DECISION`, `TURING`, `HARMONY`, `SEQUENCE`, `MATTER`,
`STRING`, `FUNCTION` têm relógio/gatilho/percurso internos.

**Implicações:**
- o front-end (JUCE/web) **faz som ao carregar** um patch, sem exigir
  teclado nem interface de áudio de entrada;
- `MIDI-IN`, `AUDIO-IN`, `CV-IN` entram como módulos `INPUT / GESTURE` —
  o patch os usa se quiser, o motor nunca os espera;
- "tocar" o instrumento é **modular o patch que já soa** (macros,
  constelação, barramento semântico), não necessariamente disparar notas
  de fora.

**Versátil em vários eixos** (é a proposta, não um efeito colateral):

| Eixo | Amplitude |
|---|---|
| escala de tempo | envelope → LFO → oscilador → áudio, no mesmo objeto (`FUNCTION`); modulação lenta ↔ taxa de áudio em todo módulo |
| papel no fluxo | 18 famílias — fonte, tempo, decisão, sequência, transformação, memória, matéria, espaço, harmonia, relação… |
| autonomia ↔ acoplamento | soa sozinho **e** acopla instrumentos RASGO, recursos de composição e gesto (§36.8) |
| alvo | headless (render `.wav`), desktop (JUCE), celular (iOS/Android/PWA web), hardware (futuro) — do mesmo core framework-free |
| uso | instrumento performático · laboratório de módulos de excelência · ambiente de composição generativa · base pra instrumentos complexos · lago de aprendizagem (§35.5) |
| conexão | **cabeamento jack-a-jack** (primária, pedagógica) + 3 superfícies alternativas — matriz (edição técnica) · constelação (campo gestual) · semântica (por significado) |
| resultado | determinístico por seed **e** nunca se repete no tempo |

## 35.5 Orientação da v1 (decisão do autor, 2026-09-01)

A pergunta em aberto desde o brainstorm da Fábrica ("a v1 serve pra aprender
eletrônica/DSP, prototipar som performaticamente ou publicar instrumentos?")
foi respondida. A v1 é, nesta ordem:

1. **Instrumento performático** - tocável ao vivo, tempo real, patch em uso.
2. **Ambiente pra codar módulos de excelência** - cada módulo levado a
   qualidade conceitual, sonora e de código, não só "compila".
3. **Pesquisa aprofundada dos módulos que já existem** - a partir dos módulos
   reais (VCV Rack, Mutable, Hexen, DaisySP, etc.), levantar **quais conceitos e
   tecnologias** cada um aplica, como se usa, como se modula com esses conceitos.
   Cada módulo do Rasgo Modular deve registrar de que módulos/artigos ele parte e
   que conceitos aplica (é o `source_origin` dos metadados, seção 28, agora
   obrigatório e substantivo, não um campo vazio).
4. **Lago de aprendizagem** - o projeto também serve pra nos atualizarmos e
   entendermos o estado da arte modular.
5. **Base pra instrumentos mais complexos** - módulos compõem instrumentos;
   o que dá pra criar a partir deles é parte do valor.

Consequência pro roadmap: o próximo marco não é UI nem arquivística - é a
**fundação que sustenta módulos de excelência num instrumento tocável**: um
contrato de módulo unificado e um grafo que processa áudio + controle + evento
por bloco, RT-safe, com fan-in explícito (resolve `CORE-GRAPH-CONNECTION`, a
"prioridade arquitetural" do inventário). Depois: o primeiro módulo DSP real
(um oscilador) feito a rigor - pesquisado, documentado, testado, e fazendo som
no grafo unificado.

## 36. Estado atual (marco 3 — 2026-09-04)

Projeto com **fundação de áudio executável, 39 módulos DSP de excelência
(inclui o barramento de saída — `MIXER` + `MASTER` estéreo — e o
oscilador subtrativo `OSC`), o modelo de conexão de três camadas
completo, três peças generativas e um painel gráfico de teste**
(`apps/panel/`, X11 + ALSA, proporção Eurorack real). Continua `prototype`:
motor C++17 header-only, **zero dependências** (sem JUCE), front-end de
produção ainda não escrito. Detalhe operacional
por etapa em [`TAREFAS.md`](TAREFAS.md); um dossiê por módulo em
[`dossies/`](dossies/00_indice.md).

**Marco 3 (2026-09-04) — rack completo + refinamento:** os 12 candidatos
(`SH`/`SHAPE`/`LPG`/`CHORD`/`DRIFT`/`SWITCH`/`SCOPE`/`TRIGSEQ`/`ABACUS`/
`WASP`/`MATRIX`/`MULT`) entraram; e uma rodada de refinamento:
(A) `connectToParameter`/`followQuality` **aditivos** — a modulação soma
sobre o knob em vez de apagá-lo (`setParameterBase`, §36.2);
(B) **antialiasing** de `SHAPE`/`WASP`/`LPG` — helper `Oversampler2x`
(2× meia-banda) + ADAA de 1ª ordem no folder do `SHAPE`;
(C) **displays por módulo** no painel — grade clicável da `MATRIX`,
espectro do `SCOPE`, 4 lanes do `TRIGSEQ` (§2.9 do design);
(D) **profundidade por módulo** — `op` bit a bit no `ABACUS`, `slope`
assimétrico no `SH`, modo `dual` no `MULT`, `bounce` no `LPG`, `dir`
demux no `SWITCH`, `voicing` (condução de vozes) no `CHORD`, `ring` no
`MATRIX`, `anchor` (memória de topologia) no `DRIFT`. Cada param novo
tem default que preserva o comportamento antigo bit-a-bit; as 4 peças de
exemplo seguem byte-idênticas.

### 36.1 Fundação de áudio (`src/core/SignalGraph.hpp`)

O caminho de áudio que faltava (resolve `CORE-GRAPH-CONNECTION`, a
"prioridade arquitetural" do inventário). Realiza as quatro decisões do
Atlas:

- **`Signal`** — base de módulo por bloco: portas tipadas
  (`audio`/`control`/`event`), parâmetros com faixa/unidade, `prepare()`
  fora do áudio, `process(inputs, outputs)` sem alocação/lock/IO,
  `panel()` e `manifest()`.
- **`Cable`** — a conexão como OBJETO que processa (Atlas §9-11, 37):
  ganho por conexão; **ruptura → cicatriz** (segura o último bloco e
  decai ~350 ms, Clouds/§37 "romper não é apagar"); **conductance**
  (probabilidade de condução por conexão, re-sorteada ~20 Hz com seed
  determinística — Marbles/Branches embutido em toda conexão);
  **relação** RingMod/Fold/Difference com um sinal companion
  (Warps/§39 "a relação é o processo"); `constellationGain`.
- **Feedback** com atraso explícito de um bloco (fora da verificação de
  ciclo). **Modulação saída→parâmetro** (`connectToParameter`, com
  profundidade e offset) — **aditiva sobre o knob**: `base + offset +
  depth·fonte`, `base` = último valor de knob, atualizado por
  `setParameterBase()`; `parameterUserValue()` lê a base. **Fan-in
  explícito** — uma conexão por porta de entrada; a soma é um nó `Sum`
  explícito, com orçamento de ganho.
- **`prepare()`** calcula ordem topológica e aloca os buffers por porta;
  `process()` só percorre armazenamento pré-alocado (contrato RT).

Acompanham: **`ControlSnapshot<N>`** (barramento de controle por snapshot
coerente — seqlock portátil RT-safe, retry limitado; derivado do padrão
`EnergyControlBus` do TRIOIO, generalizado); **`Panel`/`Widget`**
(descrição de painel declarativa, neutra de framework) + **`AsciiPanel`**
(renderizador de texto pra teste + `validatePanel()`); **`WavWriter`**
(PCM 16-bit sem deps); **`Oversampler2x`** (`src/dsp/Oversampler.hpp` —
2× compartilhado pra etapas não-lineares: upsample linear + FIR
meia-banda; usado por `WASP` e `SHAPE`).

### 36.2 Modelo de conexão — cabeamento + três superfícies

**Decisão do autor (2026-09-02): o cabeamento jack-a-jack é a superfície
primária.** A ideia anterior de descartá-lo foi revertida — cabear é
menos abstrato, tem **viés pedagógico** forte e existe **uma cultura
entre os músicos** de gostar de conectar os módulos de formas variadas.
O painel de teste já implementa: puxar um cabo de um jack a outro, com os
**destinos válidos acesos** (afordância — quem não sabe onde ligar vê;
polaridade oposta = válido, mesmo tipo de porta = destaque forte),
polaridade errada apagada; botão direito tira o cabo; ciclo vira conexão
de feedback automaticamente.

Sobre o mesmo grafo, **três superfícies alternativas** (não substituem o
cabo, complementam):

| Camada | Papel | API (`SignalGraph`) |
|---|---|---|
| **Matriz** | superfície de edição: linhas = saídas, colunas = entradas + parâmetros, célula = `Cable`/`ParameterLink`/vazio | `matrixSources()`, `matrixSlots()`, `matrixCell()`, `matrixToText()` |
| **Constelação** | superfície de performance: cada nó tem posição num campo; a distância entre nós conectados vira o ganho da conexão (gaussiana `exp(−(d/r)²)`) | `setNodePosition()`, `couplingFromDistance()`, `applyConstellation()`, `clearConstellation()` |
| **Semântico** | conectar por SIGNIFICADO: um nó CONTRIBUI para uma qualidade (Energy/Brightness/Density/Tension/Motion), um parâmetro de outro nó a SEGUE; padrão `EnergyControlBus` | `contributeQuality()`, `followQuality()`, `qualityValue()` |

Por baixo de todas: **cabo-como-objeto** (`Cable` com ruptura/cicatriz,
condução, relação), `SignalGraph::disconnect(node, port)` pra religar ao
vivo, e **patch serializado como partitura legível**

> **Resolvido (2026-09-04):** `connectToParameter`/`followQuality` são
> **aditivos** — `param = base + offset + depth·fonte`, com `base` = o
> valor do knob. `SignalGraph::setParameterBase(node, id, v)` atualiza a
> base (o painel roteia todo giro de knob / clique de toggle por aí) e
> `parameterUserValue()` a devolve (é o que `serialize()` grava). Girar
> um knob num parâmetro modulado não é mais apagado. Ver `TAREFAS.md`.

(`serialize()`/`deserialize()`, Atlas §23 — texto salvável e versionável;
formato `rasgo-modular-patch 1`).

### 36.3 Os 39 módulos DSP

Cada um com dossiê (problema musical, fontes primárias, modelo
matemático, três modos obrigatórios, critérios de escuta), testes
isolados e integração no grafo. Padrão: `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`.

| # | Módulo | Família | Essência | Parte de (estudado, não copiado) |
|---|---|---|---|---|
| 1 | `FUNCTION` | SOURCE/TIME | rampa que é envelope/LFO/oscilador conforme a taxa; `drift` | Tides/Stages, PolyBLEP |
| 2 | `FILTER` | TRANSFORM | 3× SVF TPT na mesma frequência; `spread` (relação = formante); auto-oscila | Cytomic/Simper, Three Sisters |
| 3 | relação de `Cable` | RELATION | RingMod/Fold/Difference na conexão | Warps |
| 4 | `DECISION` | DECISION | gate de Bernoulli + CV uniforme→sino + déjà-vu (loop-lock) | Branches, Marbles, Sapèl |
| 5 | `CLOCK` (`EuclidClock`) | TIME | euclidiano O(1) + acento AND/OR de divisores + drift no andamento | Toussaint/Bjorklund, vpme, Pamela's |
| 6 | `ENVELOPE` | UTILITY/TIME | A/D/(S)/R + VCA embutido; curva côncava↔convexa | Maths, Just Friends, Contour |
| 7 | `MEMORY` | MEMORY | buffer granular de 3 s + freeze (a cicatriz do `Cable` como módulo) | Clouds, arbhar, Roads |
| 8 | `TURING` (`TuringLoop`) | SEQUENCE | registrador de deslocamento; `lock` = acaso → laço travado | Music Thing Turing Machine |
| 9 | `MATTER` | MATTER | 24 modos ressonantes; `structure` corda→sino, `position` = onde bate | Rings/Elements, Cook/Smith |
| 10 | `SPACE` | SPACE | atraso multitap + difusão all-pass; de eco a cauda | Schroeder/Moorer/Dattorro, Rainmaker |
| 11 | `STRING` (`StringVoice`) | MATTER | corda por guia-de-onda (Karplus-Strong); `tanh` no laço → arco estável | KS, Jaffe & Smith, J.O. Smith |
| 12 | `QUANTIZER` | DECISION/PERCEPTION | CV → alturas de escala (12 escalas curadas); histerese; glide | RBJ… não: `RASGO_SYNTH/Scales.hpp` (só intervalos), theremin |
| 13 | `PARAMETRIC` | TRANSFORM/UTILITY | EQ paramétrico de 4 estágios (fórmulas RBJ); `sweep` move as bandas como grupo | RBJ Audio EQ Cookbook; VCV Parametra (ficha, código fechado não consultado) |
| 14 | `HARMONY` | DECISION/INFERENCE | movimento harmônico: 6 técnicas reais (Coltrane, sub tritônica, mediante cromática, intercâmbio modal, jazz modal, backdoor ii-V) dirigindo `root`/`scale` do `QUANTIZER` | `RASGO_SYNTH/HarmonicWanderer.hpp` (só a lógica de intervalos = fato musical) |
| 15 | `SEQUENCE` | SEQUENCE | sequenciador de passos: padrão de 8 passos editável (altura+gate) × 5 modos de leitura (forward/backward/pingpong/random/brownian); `glide`, `eos`. O par escrito do `TURING` | Hexen §119; René/Metropolix; Grids (browniano) |
| 16 | `MIXER` | MIX | 4 entradas mono → soma; `gain`/`pan`/`mute` por canal, pan de potência constante; saída estéreo; `out_gain` | prática de mesa; pan-law de potência constante (fato público) |
| 17 | `MASTER` | MIX/METER | barramento de saída: largura mid/side (`width` 0–2), soma `mono`, `gain`, **proteção de saída de excelência** (`src/dsp/OutputStage.hpp`: finitude + bloqueio de DC + **guarda ultrassônica** + **governador de corpo** (`body_guard`, agudo alto/sustentado/concentrado ~2,5–8 kHz → high-shelf suave) + **limitador look-ahead ~3 ms** por pico verdadeiro + teto suave, teto −1 dBFS, telemetria de GR e de body-guard), saída de VU (`level`) | matriz mid/side (Blumlein); `NAVALHA`/`ANTITOTEM` `OutputStage`+`LookaheadLimiter`+`TruePeakDetector` (código do autor) |
| 18 | `OSC` | SOURCE | oscilador subtrativo: 5 formas ao mesmo tempo (seno/tri/serra/pulso/sub) antialias PolyBLEP, 1 V/oct, PWM, hard sync, FM linear through-zero, sub-oitava; `drift` | PolyBLEP (Välimäki/Finke); hard sync clássico; TZFM (Buchla 259); sub por divisão (Juno/Moog) |
| 19 | `NOISE` | SOURCE/UTILITY | ruído branco/rosa/brown + sample-and-hold + tensão que passeia (smooth random); `spread` uniforme→sino (acaso estruturado) | Paul Kellet pink filter (domínio público); S&H clássico; Buchla 266 smooth random |
| 20 | `VCA` | TRANSFORM/UTILITY | amplificador DUPLO: `in`×ganho; CV atenuvertida SOMA ao knob (porta de verdade — knob vivo); `response` lin→exp; saturação suave; `sum` = mini-mixer; `drift` | VCA lin/exp (Doepfer A-131/132); Quad VCA como mixer; atenuverter (Maths) |
| 21 | `CONTROL` | UTILITY | utilidades de CV DUPLAS: `scale` (atenuversor −2..2), `offset`, `rectify` contínuo (`lerp(x,\|x\|)`), `slew`+`curve` (linear↔RC), saída `sum` (soma/média); `scale=0` = fonte de tensão; `rectify`+`slew` = seguidor de envelope; `drift` opt-in | Maths (atenuversor/offset/slew/somador); Serge DUSG; seguidor de envelope RC |
| 22 | `LOGIC` | TIME/UTILITY | recombina o tempo: divisor ÷1–32 + multiplicador ×1–8 (período medido); `and`/`or`/`xor` simultâneos de dois gates; flip-flop T; `gate_len` (duty) + `delay` (anel 0–200 ms); `rate` = relógio interno se `clock` livre. Fecha o rack de partida | Pamela's (÷/×); Kinks/Boolean (lógica); flip-flop T; A-160 (contador módulo-N) |
| 23 | `SH` | UTILITY | sample & hold DUPLO: cada canal segura `inN` (ou o acaso interno) no pulso de `trigN`/relógio interno; `trackN` (track & hold), `slewN` (glide Buchla 266), **`slope`** (−1..1 — subida ≠ descida do slew), `spread` (uniforme→sino), **`correlation`** −1..1 entre os acasos internos (gêmeos↔espelho). Par de CVs aleatórias relacionadas | S&H clássico (Buchla 265/266, Doepfer A-148); smooth random (266); Marbles `X`/spread; `shape` do `DECISION` |
| 24 | `SHAPE` | TRANSFORM | modelador de timbre em cadeia: ring-mod (`x·mod`) → wavefolder triangular fechado (`fold`) + `symmetry` (bias = harmônicos pares) → `wrap` (dobra suave ↔ wrap-around seco) → `sat` (tanh) → VCA (`level`); desvio `drift` no drive da dobra. Síntese por distorção da costa oeste como módulo. Antialias: núcleo a 2× + ADAA de 1ª ordem (`src/dsp/Oversampler.hpp`) | Buchla 259/258 "Timbre" (fold+symmetry); Serge Wave Multipliers; ring-mod de 4 quadrantes; dobra triangular fechada |
| 27 | `DRIFT` | DECISION/UTILITY | campo de deriva: uma fonte de CV que se move em escala de MINUTOS, com memória (momentum acumula e retroalimenta a intensidade — ANTITOTEM `CRI-DRF-001`) e correlação (LFSR compartilhado, 4 saídas = leituras ponderadas DIFERENTES dos mesmos bits + LFO próprio — AQUORBIUM `BiomaBrain`). `stride` (juntas↔separadas), `anchor` (memória de topologia — a deriva orbita marcos gravados), `advance` (cadência por compasso). Faz o patch de seed EVOLUIR sozinho | ANTITOTEM `deriveFromMemory`/`CRI-DRF-001`; AQUORBIUM `BiomaBrain::correlatedValues`; Buchla 266 smooth random; random walk limitado |
| 26 | `CHORD` | SOURCE | VCO parafônico: 2–4 vozes empilhadas de uma base 1 V/oct; tabela de 10 formatos de acorde (uníssono/oitavas/quinta/maior/menor/sus4/maj7/min7/dim/add9) por `chord` ou `chord_cv`; `inversion` (sobe as n graves uma oitava), `voicing` (condução de vozes na troca de acorde — mínimo movimento + glide), `detune` (±0,25 st = coro), `wave` (serra→pulso→tri, PolyBLEP na descontinuidade); `fm`. Soma `1/√vozes`. Desvio `drift` por voz. Casável com `HARMONY` → progressões | Plaits (modelo "chord"); Harmonaig; super-saw (JP-8000); PolyBLEP; tabelas de acorde (fato musical) |
| 25 | `LPG` | TRANSFORM/UTILITY | low-pass gate a vactrol: um seguidor não-linear assimétrico (sobe ~2 ms, desce com cauda que freia perto de 0 — a "memória" do LDR) controla um filtro de 2 polos **e** um VCA juntos. `mode` 0..1 = crossfade filtro↔VCA (0.5 = os dois totalmente ativos), `response` (tempo da cauda ~30 ms–2,5 s), `offset` (abertura de repouso), `resonance`, **`bounce`** (overshoot do vactrol pós-golpe); `strike` + `cv`. Desvio `drift`. O timbre *plucky* da costa oeste | Buchla 292 / série 200 LPG; Make Noise Optomix (crossfade); Mannequins Three Sisters (modo LPG); modelo de fotocélula (LDR) |
| 28 | `SWITCH` | ROUTE/UTILITY | chave sequencial: `dir` 0 = mux N→1 (`a`/`b`/`c`/`d` → `out`), `dir` 1 = **demux 1→N** (`a` → `out`/`out_b`/`out_c`/`out_d` conforme o passo); o endereço avança no `clock` (borda ↑), zera no `reset`, ou vem direto da CV `addr` (se conectada, manda); `steps` 2–4, `mode` (forward/pingpong/random semeado/só-`addr`), `glide` (crossfade no ponto de troca) + slew de 1 ms anti-clique; saída `step` segue a posição. O roteador controlado — faz a variação de roteamento virar parte do fluxo autônomo | Doepfer A-151/A-152 (chave sequencial/endereçada); 4ms SISM (slew na troca); multiplexador CD4051 (teoria); `mode` de leitura do `SEQUENCE` do Rasgo |
| 29 | `SCOPE` | METER/UTILITY | osciloscópio + análise cujas medições SAEM COMO CV (desvio Rasgo — num scope de hardware a tela é beco sem saída): `in`→`thru` limpo (a saída 0 = o que o painel desenha); `trig` = comparador com histerese (`reject`) contra `trigger`, borda `edge` (trigger do scope + disparador utilitário); `level` (seguidor de pico); `bright` (centroide espectral pelo diferenciador — `f_c≈(sr/2π)√(E[Δx²]/E[x²])`, sem FFT); `pitch` (v/oct, período entre cruzamentos de zero, trava após 3 períodos consistentes — ruído fica em 0); `hold` congela as leituras. O instrumento que escuta a si mesmo | osciloscópio de bancada (trigger nível/borda/histerese); Mordax DATA / ALM MUM M8 (scope de rack); centroide espectral por Parseval (resultado público); ZCR (detecção de pitch por período) |
| 30 | `TRIGSEQ` | SEQUENCE/TIME | grade de trigs de percussão — 4 linhas de gate on/off (bumbo/caixa/chimbal/perc) tocando juntas. NÃO é editor de passos: é GERADOR (identidade RASGO "soa ao carregar"). `map` (0–1) morfa entre 4 caracteres (straight/broken/shuffle/sparse) interpolando os pesos de cada passo; `density1..4` = limiar sobre o peso (à la Grids); `swing` atrasa passos ímpares; `chaos` = notas-fantasma/quedas por probabilidade (não flip cru); `ratchet` = rajada de 3 no passo; `fill` (entrada) + `fill_amt` = viradas; `drift` = passeio lento do groove. Saídas `t1..t4` + `accent` (≥2 linhas coincidem) + `any` (OR). `length` recorta, `rate` = relógio interno. Determinístico (xorshift semeado) | Mutable Grids (mapa rítmico + limiar de densidade — conceito, tabelas próprias); TR-808/909 (grade + acento derivado); Pamela's / randomRHYTHM (prob./fill); modo browniano do `SEQUENCE` |
| 31 | `ABACUS` | LOGIC/UTILITY | aritmética da CV como NÚMERO: `math` = `a` ⊕ `b` por `op` 0–7 (soma/subtração/multiplicação/**resto** · **bit a bit** AND/OR/XOR/NAND sobre inteiros de 5 bits — Lunetta); `quant` = fonte encaixada em `steps` degraus iguais, com `slew`; `rect` = retificador dedicado (meia-onda +/− · onda completa `|a|` · **sinal** `±range`/0). Contador binário: cada `clock` soma `count_step` (pode ser negativo); `c = count mod modulus` → `p1` (bit `bitA`, divisor limpo), `p2` (`bitA` XOR `bitA+1`, sincopado), `carry` (pulso no overflow — ritmo). **Sem `a` conectado → a fonte é a rampa do contador** (toca melodia + ritmo sozinho). Determinístico, sem RNG | Noise Engineering Numeric Repetitor (contador + máscara → ritmo); retificador clássico (meia/onda-completa); aritmética modular (teoria); divisor binário / Gray code (teoria) |
| 32 | `WASP` | TRANSFORM | filtro de 12 dB com GRÃO — o caráter do EDP Wasp (1978), inversores CMOS 4069 como estágios de ganho que ceifam duro e assimétrico. Núcleo SVF TPT (2 polos, igual ao `FILTER`) com ceifador muito mais agressivo no laço + estágio de saída que ceifa DEPOIS do filtro (buzz reedy). `grit` (joelho do ceifador), `bias` (teto assimétrico → harmônicos pares + bloqueador de DC), `mode` (LP↔BP↔HP), `drive` (waveshaper com corte), corte estendido a 24 kHz, `drift`. Auto-oscila perto de `resonance`=1. Núcleo não-linear a 2× (antialias). O contraponto sujo do `FILTER` limpo | circuito do EDP Wasp (análises independentes, René Schmitz/DIY — NÃO Doepfer service manual nem VCV); inversor CMOS 4069 (teoria); SVF TPT não-linear (Zavalishin/Simper-Cytomic) |
| 33 | `MATRIX` | ROUTE/MIX | matriz de roteamento 4×4 — cada cruzamento fonte×destino é um ganho (atenuversor), como as matrizes de pinos do EMS Synthi / Doepfer A-138m. `out_k = level·sat(Σ_j in_j·g_jk)`. 16 células `g11..g44` (−1..1, padrão identidade = passa-direto); `norm` (nível constante por coluna), `ring` (coluna vira produto = ring-mod de 4 quadrantes), `sat` (matriz segura em laço), `level`, `drift` (desvio lento dos ganhos — a matriz respira). No painel gráfico é uma grade clicável. Patch denso sem espaguete. Determinístico | EMS Synthi / ARP 2500 (matriz de pinos), Doepfer A-138m / Befaco / Erica matrix mixer, Serge/Buchla (norm por coluna), camada matriz do `SignalGraph` (marco 2) |
| 34 | `MULT` | UTILITY | múltiplo PROCESSADO — no grafo digital o fan-out já é livre, então cada saída tem atenuversor + offset próprios (mini-`CONTROL` por tomada). 1 entrada → 4 saídas, `out_k = slew(scale_k·in + offset_k)`; `scale` ±2 (negativo = inverte), `offset` ±1, `slew` compartilhado. **`dual`** + `in2`: out1/2 ← `in`, out3/4 ← `in2` (A-180-2). Ocioso (sem `in`) vira 4 fontes de tensão manual (`out_k = offset_k`). Sem `drift` (é utilidade de precisão). O distribuidor de CV | múltiplo bufferizado (Doepfer A-180, Intellijel Buff Mult), atenuversor+offset (Maths/Serge), voltage spreader (Frap/Doepfer) |
| 35 | `AUDIO-IN` | SOURCE | entrada de áudio ao vivo via ALSA (`AlsaSource`, `device` configurável por `RASGO_AUDIO_IN_DEVICE`) — o instrumento ouve o mundo, não só ele mesmo. `gain`, `dc_block` | ALSA PCM capture (padrão do sistema); bloqueio de DC (fato de engenharia de áudio) |
| 36 | `CHAOS` | DECISION | campo caótico de poço duplo — dois integradores perseguem uma força restauradora não-linear (`x − x³`); `drive`/`damping` decidem se assenta, oscila ou "caça"; chute periódico aleatório (`rate`) é o que deixa o sistema atravessar de um poço pro outro; `freeze`, `reseed` (trigger) | `ANTITOTEM/src/core/ChaosSources.h::ChaosField` — caos de poço duplo (Ian Fritz, 2007) |
| 37 | `PLL` | SOURCE | segundo oscilador dedicado, sofisticado: detector de fase compara contra referência externa e CURVA a própria taxa (não reseta duro); toca livre sem referência; `ratio` generaliza pra sub/super-harmônicos (0,03–8×, desvio Rasgo), `lock_gain`, alcance de captura ±0,9 medido e documentado (limitação real de PLL, não bug); `shape` (seno↔tri↔serra↔quadrada, PolyBLEP); `feedback_type` (direto/retificado/capacitivo/pulso/"transistor"/refluxo, modula só a fase lida) | `ANTITOTEM/src/core/CmosVoice.h` — OSC5 (detector de fase + `pllLockGain`), `feedbackSample()`/`FeedbackSignal` |
| 38 | `NOTE-OUT` | MIX | adaptador que captura o contrato `NOTE` do `MUSICAL SCORE` sem mudar a interface de nenhum outro módulo: detector de borda de gate + amostra de pitch, expõe `takeCompletedNote()` (chamado do laço de áudio do painel, nunca de `process()`); `gate_thru`/`pitch_thru` (pass-through, necessário pra alcançabilidade — `setActiveOutput` só processa ancestrais do sink ativo). v1 monofônico (limitação documentada) | desenho próprio — observador/adaptador sobre o contrato `NOTE` descrito em `ESTUDO_seed_composicao_generativa.md §5` |
| 39 | `GLIDE` | TRANSFORM / PITCH | portamento POR NOTA (o primitivo que faltava pro baixo acid): `mode` 0 sempre / 1 slide-gated (o *slide* do TB-303, `slide` alto habilita) / 2 legato (só se `gate` segue alto na troca); `time` de subida + `fall` (assimetria — descida = `time·6^fall`); `curve` linear (rate constante, MS-20) ↔ RC; saídas `moving` (gate) e `done` (pulso na chegada). Sem `drift` (é régua de afinação) | TB-303 slide, portamento MS-20/Minimoog vs glide RC, Bela Gliss / EMW glide processor (`PESQUISA §2.4` Onda A) |

**Desvios Rasgo recorrentes:** `drift` (deriva orgânica seeded);
não-linearidade NO laço (auto-oscilação/arco como ciclo-limite, não NaN);
a relação entre saídas/bandas como processo (`spread`/`sweep`);
determinismo por seed em tudo que usa acaso.

**Excelência sonora — padrão da família.** `src/dsp/OutputStage.hpp` traz
pra o Rasgo Modular o padrão de proteção de saída já codado em
`NAVALHA2_JUCE` (`OutputStage` + `LookaheadLimiter` + `TruePeakDetector`)
e `ANTITOTEM` (`OutputStage`): **"saturação criativa é do patch; remoção
de DC e contenção de pico NÃO são"**. Cadeia: guarda de finitude →
bloqueio de DC → **guarda ultrassônica** (LP Butterworth 2 polos ~21 kHz,
transparente até ~15 kHz, tira só o ice-pick perto de Nyquist) →
**governador de corpo** (`body_guard`, 2026-09-05 — detecta agudo alto +
sustentado + concentrado em ~2,5–8 kHz e aplica um high-shelf suave;
transiente/ritmo/ruído de banda larga passam; só o grito estável é
contido; 0 = bypass exato; telemetria `bodyGuardDb()`) → limitador com
look-ahead (~3 ms, orientado por pico verdadeiro) → teto suave de 1
amostra → teto −1 dBFS + telemetria de redução de ganho. `TruePeakEstimator`
(`src/dsp/TruePeak.hpp`) já integrado. O `MASTER` (Módulo 17) usa tudo
isso. Testes: `test_true_peak.cpp` + `test_output_stage.cpp`.
Pendente: `TruePeakDetector` 4× polifásico plenamente conforme BS.1770,
como na `NAVALHA`.

### 36.4 Peças generativas

Renders determinísticos por seed (dois renders byte-idênticos), nunca se
repetem no tempo. Em `validation-output/` (fora do git).

| Peça | Duração | O que exercita |
|---|---|---|
| `primeiro_fragmento` | 10 s | `FUNCTION` → `FILTER` → `Cable` com ruptura |
| `peca_generativa` | 40 s | os 6 módulos do marco 1 + fundação |
| `peca_generativa_2` | 50 s | os 8 módulos + matriz + **constelação** (MEMORY respira no campo) |
| `peca_generativa_3` | 55 s | módulos físicos + escala + EQ + **barramento semântico** (Motion→SPACE, Energy→PARAMETRIC) |
| `peca_generativa_4` | 42 s | protótipo do `MotionEngine` (`apps/panel/MotionEngine.hpp`) — parâmetros com comportamento `WALK`/`OSCILLATE`/`ATTRACT` no tempo, aditivo sobre a modulação por cabo já existente; grava a `SYSTEM SCORE` (`ScoreRecorder.hpp`) ao lado do `.wav`; ver `dossies/ESTUDO_seed_composicao_generativa.md §3.6/§5` |

### 36.5 Testes e build

`CMakeLists.txt` — `add_library(rasgo_modular_core INTERFACE)`,
`CMAKE_CXX_STANDARD 17`, só `find_package(Threads)`.
**48 alvos CTest**, 100% verdes em **Debug e Release**
(`-Wall -Wextra -Wpedantic -Werror`): grafo/fundação (com matriz,
constelação, barramento semântico, serialização, feedback, condução,
modulação aditiva), um alvo por módulo (`MIXER` + `MASTER` compartilham
`test_mix`), `test_panel_layout` (regressão de sobreposição de rótulos),
`test_motion_engine`/`test_patch_genetics`/`test_score_recorder`/
`test_learn_catalog` (protótipos de composição generativa e pedagogia,
`dossies/ESTUDO_seed_composicao_generativa.md` — `test_patch_genetics`
cobre `MUTATE`/`EVOLVE`/`CROSS`), `test_chaos`/`test_pll`/
`test_note_out`/`test_audio_in` (módulos 35–38), `test_true_peak`/
`test_output_stage`/`test_wav_writer` (excelência de saída: pico
verdadeiro, guarda ultrassônica, governador de corpo, dither TPDF), e o
`test_graph_engine` legado. `PatchSeed.hpp`/`SeedGrammar.hpp` não têm
alvo CTest dedicado — verificados por comparação byte a byte
(`serialize()`, seeds 1–200) contra a versão pré-refactor, ver
`TAREFAS.md` (2026-09-05). As 5 peças de exemplo renderizam
byte-idênticas às referências em `validation-output/` (checagem manual —
não é alvo CTest).

```
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

### 36.6 `src/core/Graph.hpp` — o protótipo multimodal (coexiste)

O grafo escalar + `AudioGraph` por bloco + a camada `MultimodalPort`/
`MultimodalGraph` do lote de agosto de 2026 continua em `Graph.hpp` e
tem seu próprio teste (`test_graph_engine.cpp`). O áudio real do marco
1-2 foi construído à parte em `SignalGraph`; a fusão dos dois (ou a
aposentadoria de um) é decisão de marco futuro. `Graph.hpp` também
sedia o `ArchiveStorage` (pacote `manifest.json`/`payload.bin`/
`provenance.json` com checksum FNV-1a-64).

### 36.7 Framework-free e front-ends

O motor não herda nenhum framework (regra da ORQUESTRACAO). Plano
decidido (2026-09-01): **os dois** — **JUCE** (desktop + iOS/AUv3 +
Android; onde os instrumentos RASGO se unem no Ensemble Bus) e
**web/WASM** (esboço, patch = URL, PWA). Um `.rmp` de texto e uma `Panel`
servem os dois. Os dois "grandes" ainda não escritos.

**Primeiro front-end (2026-09-02): `apps/panel/` — painel gráfico de
teste** (X11 + ALSA + Xrandr). Executável que linka X11/libasound —
**o `rasgo_modular_core` continua sem dependência**. Bases lidas:
`RASGO_DOCUMENTATION/design/` (README, INTERFACES_E_LAYOUTS,
IDENTIDADE_VISUAL, AUDITORIA_MIGRACAO_RESPONSIVA…). Detalhe e roadmap da
UI: **`apps/panel/design.md`** (`exploration`).

Concept da experiência (decisões **específicas do Rasgo Modular**):

- **case Eurorack que quebra em linhas** — módulos na ordem do fluxo,
  **altura padrão** (como 3U; a largura varia por `Panel::hp`), a fileira
  quebra em várias linhas pra aproveitar a tela (largura **e** altura),
  rola na vertical pra patches grandes;
- **coluna de módulos disponíveis** à esquerda, **por família** — com o
  tempo haverá muitos módulos e cabe ao músico escolher; arrastar um item
  pra a case cria o nó (`apps/panel/ModuleCatalog.hpp` = factory
  `makeModule(type)`, serve também pra carregar `.rmp`);
- **sugestão por seed** — como o seed do `RASGO_SYNTH` (`SeedLineage`), o
  Rasgo Modular pode **propor um patch de partida** determinístico que o
  músico modifica; a folha em branco continua válida. `[s]` hoje é um
  marcador; a sugestão completa é feature registrada;
- **camada de patch** (desenhar/criar cabos, matriz e constelação como
  views) — pendente;
- painéis renderizados **a partir da `Panel` declarativa** (nada
  hard-coded); knobs/sliders/toggles/jacks; arrastar um controle → ouve
  ao vivo (RT-safe no callback);
- **UTF-8 obrigatório** (`setlocale` + `Xutf8DrawString`, fallback);
- **sem sobreposição acidental** — cada widget com pegada explícita,
  checagem no arranque (0 avisos);
- **contrato responsivo** (`INTERFACES_E_LAYOUTS §5.1`): 1ª abertura em
  ~88% da área do **monitor primário**, centrada (`WindowPolicy.hpp` +
  `XRRGetMonitors`); persistência de bounds e reflow semântico completo
  = débito assumido do front-end de produção.

O motor **é autônomo** (§35.4.1): o front-end **faz som ao carregar** um
patch, sem exigir teclado nem entrada de áudio. `MIDI-IN`/`AUDIO-IN` e o
acoplamento a outros instrumentos / recursos de composição (§36.8) entram
como módulos-adaptadores no catálogo, nunca como dependência.

**Regra de contribuição de painel:** todo módulo do Rasgo Modular
descreve seu `panel()` cabendo na **altura padrão**, sem widgets
sobrepostos, e seguindo a **rubrica de layout** de `apps/panel/design.md
§3.2.1` — margens fixas, display a largura interna cheia, grade de knobs
centrada por HP, fileiras de jack espaçadas pelo rótulo (quebra em
fileiras por função quando não cabe), HP enxuto. `tests/test_panel_layout.cpp`
(modela a largura do rótulo do jack) é o gate contra regressão.

**Cabeçalho + i18n (2026-09-05/06):** cabeçalho de linha única no modelo
dos RASGO Synth — logo RASGO (anti-aliased) + barra de comandos em botões
(toggles com anel de destaque) + pico da saída + `SEED`/`REC`/`STANDBY` +
`IDIOMA`/`TUTORIAL`/`SOBRE`. `apps/panel/UiLanguage.hpp` — EN padrão,
PT/FR/ES no botão; cabeçalho/tutorial/créditos traduzidos, LEARN em
fases, rótulos de módulo não. Passe de ergonomia módulo-a-módulo dos 38
painéis (larguras, displays, colisões de rótulo) — ver `TAREFAS.md`
2026-09-06.

### 36.8 Acoplamento — instrumentos e recursos de composição

Autônomo **não** quer dizer fechado. Além de soar sozinho (§35.4.1), o
`RASGO_MODULAR` tem as conexões pra **acoplar**:

- **outros instrumentos RASGO** — antitotem, rasgo-synth-performance,
  navalha, trioio, aquorbium… por **adaptadores e manifestos**, nunca
  por acesso ao estado privado. Referência:
  [`ORQUESTRACAO_INSTRUMENTOS.md`](../RASGO_DOCUMENTATION/architecture/ORQUESTRACAO_INSTRUMENTOS.md)
  (`RASGO Ensemble Bus`, separa áudio/evento/modulação/observação);
- **recursos de composição** — motores de arco/estrutura, engines
  harmônicas, partituras/eventos externos, geradores de forma, DAW/OSC,
  etc. (ex.: `HarmonicWanderer`/`GenerativeArc` do `RASGO_SYNTH` já
  entraram *como conceito* nos Módulos 12/14; a versão "engine viva
  acoplada" é a evolução);
- **entrada de gesto** — `MIDI-IN`, `AUDIO-IN`, `CV-IN`, sensores, como
  módulos `INPUT / GESTURE` opcionais.

Todos entram do mesmo jeito: **só mais um nó `Signal`** (um
`InstrumentAdapter` / `ResourceAdapter` / `InputAdapter`), com o motor
nunca dependendo deles. Portão G4: promover o contrato comum só depois
de dois acoplamentos de naturezas diferentes.

### 36.9 Próximo

- inclinações de 24/48 dB no `PARAMETRIC` (cascata de biquads);
- movimento harmônico (`HarmonicWanderer` — escala/tônica se movendo);
- reverb por FDN como modo do `SPACE`; decaimento dependente de
  frequência na `STRING`;
- sequenciador editável (família de comportamentos, Hexen §119);
- serialização das três camadas de conexão no patch de texto;
- **front-ends JUCE + web/WASM** a partir do core framework-free;
- contrato do Ensemble Bus (após dois instrumentos).

### 36.10 Histórico do protótipo (agosto 2026)

O núcleo inicial era `CMakeLists.txt`, `src/core/Graph.hpp` e
`tests/test_graph_engine.cpp` — C++17 puro, sem JUCE, sem GUI. A primeira
fatia (2026-08-16) verificava:

- instanciação de módulos, portas escalares tipadas (`audio`/`control`/
  `event`) com descritor, avaliação topológica de grafo acíclico;
- estado mínimo de `SOURCE.CONSTANT`/`INPUT.CONTROL`/`TRANSFORM.GAIN`/
  `METER.VALUE`; parâmetros com faixa/default/unidade separados do estado
  persistente; serialização/restauração de patch;
- rejeição de conexões entre tipos incompatíveis e de ciclos sem
  scheduler stateful.

Em seguida (agosto de 2026) vieram, em `Graph.hpp`: `AudioBlock` fixo e
`AudioGraph` por bloco sem alocação; `StreamDescriptor`/`EventQueue`/
manifesto de módulo; a camada multimodal (`MultimodalPort`,
`MultimodalGraph`, processadores homogêneos, `ProcessContext`, snapshot);
e `PayloadEnvelope`/`ArchiveSerializer`/`ArchiveStorage` (pacote
`manifest.json`+`payload.bin`+`provenance.json` com checksum FNV-1a-64,
sem I/O no callback; autoria RASGO, inspirado no odot, sem copiar código).
Tudo com `ctest` 1/1 na época.

A partir de 2026-09-01 o desenvolvimento seguiu no `SignalGraph` (§36.1)
— o caminho de áudio real. `Graph.hpp` permanece como está; a
convergência dos dois grafos é decisão de marco futuro.
