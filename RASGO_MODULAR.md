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

## 36. Estado atual

Projeto formalmente iniciado como conceito, arquitetura e primeiro protótipo
executável.

### Protótipo atual

`CMakeLists.txt`, `src/core/Graph.hpp` e `tests/test_graph_engine.cpp` formam um
núcleo C++17 puro, sem JUCE e sem interface gráfica. A primeira fatia verifica:

- instanciação de módulos;
- portas escalares de entrada e saída;
- portas tipadas como `audio`, `control` e `event`;
- descritores de porta com nome, tipo e unidade;
- conexão e avaliação topológica de um grafo acíclico;
- estado mínimo de `SOURCE.CONSTANT`, `INPUT.CONTROL`, `TRANSFORM.GAIN` e
  `METER.VALUE`;
- serialização e restauração de patch, com estado persistente separado de
  parâmetros editáveis;
- parâmetros com faixa, valor padrão e unidade;
- rejeição de conexões entre tipos incompatíveis;
- rejeição explícita de ciclos que ainda exigem scheduler stateful.

Estado: `prototype`. Ainda não é biblioteca compartilhada nem contrato final.
Os valores de áudio ainda são escalares e não há buffers de áudio; eventos já
podem ser propagados entre módulos compatíveis. Ainda não há scheduler
stateful, automação de parâmetros, metadados YAML ou integração com outro
instrumento.

Validação executada em 2026-08-16:

```text
cmake -S RASGO/RASGO_MODULAR -B /tmp/rasgo-modular-build -DBUILD_TESTING=ON
cmake --build /tmp/rasgo-modular-build --parallel 2
ctest --test-dir /tmp/rasgo-modular-build --output-on-failure
1/1 test passed (inclui validação de tipos, conexão incompatível e restauração
separada de estado e parâmetros)
```

### Orquestração entre instrumentos

O `RASGO_MODULAR` deve prever uma segunda função além de ser um instrumento e
laboratório: atuar como camada capaz de conectar instrumentos RASGO já criados.
Essa conexão será feita por adaptadores e manifestos de capacidade, nunca por
acesso direto ao estado privado ou aos cabeçalhos internos de outro instrumento.

O contrato ainda precisa ser investigado instrumento por instrumento. A
referência transversal é
[`ORQUESTRACAO_INSTRUMENTOS.md`](../RASGO_DOCUMENTATION/architecture/ORQUESTRACAO_INSTRUMENTOS.md),
que propõe o `RASGO Ensemble Bus` como nome de trabalho e separa áudio, evento,
modulação e observação.

Primeiro laboratório previsto: grafo local no mesmo processo com nós fictícios,
depois dois instrumentos de naturezas diferentes. Processos separados, PipeWire
e OSC ficam para uma fase posterior.

Registro da etapa mais recente:

- o contrato de persistência agora distingue `state` de `parameterState`;
- `TRANSFORM.GAIN` demonstra a separação sem fingir que já possui memória DSP;
- eventos preservam o `timestamp` como posição lógica de frame/amostra e já
  possuem comparação determinística, sem ainda formar uma fila;
- `AudioBlock` fixo já define sample rate, canais e frames sem alocação durante
  a limpeza/processamento do bloco;
- `AudioProcessor` e `AudioGainProcessor` já demonstram processamento de bloco
  separado do graph scalar, com rejeição não-excepcional de configurações
  incompatíveis;
- `AudioGraph` já conecta dois processadores em ordem topológica, preparando
  buffers fora de `process()` e executando o caminho sem alocação dinâmica;
- `StreamDescriptor`, `EventQueue` e manifesto mínimo de módulo já existem
  como contratos de preparação/inspeção, sem serem confundidos com o scheduler
  final;
- `StreamCategory`, `ControlBlock`, `DescriptorBlock` e `EventBlock` ampliam a
  fundação para streams multimodais sem alterar ainda o graph scalar;
- os módulos iniciais agora incluem clock, probabilidade, split e soma;
- a validação permanece limitada a valores escalares e a um teste integrado.

Marco de continuidade em 2026-08-16:

- `PayloadProvenance` e `PayloadEnvelope` agora registram schema, origem,
  timestamp, tempo lógico, categoria, payload, descritor opcional, latência e
  proveniência;
- `PayloadEnvelope::valid()` rejeita categoria incompatível, origem ausente,
  proveniência incompleta, descritor inválido e latência negativa;
- `toJson()` é uma representação de metadados para arquivo e permanece fora do
  caminho realtime;
- `ArchiveSerializer` produz um manifesto versionado com escape JSON seguro,
  sem realizar I/O nem incluir bytes do payload no callback;
- `ArchiveStorage` grava o pacote mínimo `manifest.json`, `payload.bin` e
  `provenance.json`, incluindo tamanho e checksum FNV-1a-64;
- `ArchiveStorage::readPackage()` lê o pacote e rejeita `payload.bin` quando o
  checksum não coincide com o manifesto;
- o envelope é autoria do RASGO, inspirado conceitualmente pelo odot, sem
  copiar código externo;
- build C++17, warnings tratados e `ctest` passaram com 1/1 teste.

Próximas investigações:

- substituir os valores escalares por buffers de áudio e eventos tipados;
- transformar a política provisória de timestamp em fila ordenada quando o
  scheduler stateful for introduzido;
- definir a API de processamento por bloco e separar áudio, controle e eventos;
- integrar um nó de áudio por bloco ao grafo sem misturar os tipos `control` e
  `event`;
- projetar o contrato de portas multimodais do graph comum, preservando as
  diferenças entre áudio, controle e eventos;
- associar `StreamDescriptor` às portas e negociar áudio, descritores,
  controle e eventos na preparação do grafo;
- criar um contrato de porta multimodal que diferencie áudio, controle,
  eventos e descritores durante a preparação;
- criar `MultimodalPort` e um grafo de preparação que negocie esses contratos
  sem misturar seus valores;
- criar processadores multimodais mínimos, mantendo preparação e execução
  realtime em fases distintas;
- criar adapters multimodais e fan-in explícito para combinar resultados sem
  esconder a origem dos streams;
- separar manifesto, estado e parâmetros em formatos versionados;
- separar estado, parâmetros e metadados do processamento em uma API estável;
- definir o formato de metadados de módulo;
- decidir o scheduler para conexões stateful e feedback;
- inventário completo dos módulos Hexen;
- comparação Hexen × VCV × Cardinal × Bespoke;
- levantamento de módulos já desenvolvidos nos instrumentos Rasgo;
- identificação de código reaproveitável;
- definição de API de módulo;
- definição de portas e tipos de sinal;
- primeiro protótipo do graph engine.

Próximo marco efetivo: formalizar o formato binário, decidir sobre SHA-256 e
depois integrar o armazenamento aos adapters multimodais e ao fan-in explícito,
preservando origem, latência e proveniência.
