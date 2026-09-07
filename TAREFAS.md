# RASGO Modular — tarefas e continuidade

**Atualizado:** 2026-09-04 (marco 3)
**Estado:** `prototype`

Esta é a lista operacional do projeto. A arquitetura conceitual permanece em
[`RASGO_MODULAR.md`](RASGO_MODULAR.md) (§36 = estado atual); decisões
transversais e reutilização continuam no inventário global; um dossiê por
módulo em [`dossies/`](dossies/00_indice.md).

## Estado — marco 1-2 (histórico — 2026-09-02)

*(O estado corrente é o bloco "marco 3" logo abaixo. Este bloco fica como
registro do que existia ao fim do marco 2.)*

**Feito:**
- **Fundação de áudio** `src/core/SignalGraph.hpp` — `Signal` (módulo por
  bloco), `Cable` (conexão-objeto: ruptura/cicatriz, condução
  probabilística, relação RingMod/Fold/Difference, `constellationGain`),
  feedback de 1 bloco, modulação saída→parâmetro, fan-in explícito,
  contrato RT. `ControlSnapshot<N>` (seqlock), `Panel`/`AsciiPanel`,
  `WavWriter`.
- **Modelo de conexão — 3 camadas:** matriz (`matrix*`), constelação
  (`setNodePosition`/`couplingFromDistance`/`applyConstellation`),
  semântico (`contributeQuality`/`followQuality`/`qualityValue`). **As 3
  camadas serializadas** no patch de texto (`serialize`/`deserialize`).
- **22 módulos DSP** com dossiê + testes: 1 `FUNCTION` · 2 `FILTER` ·
  3 relação de `Cable` · 4 `DECISION` · 5 `CLOCK` · 6 `ENVELOPE` ·
  7 `MEMORY` · 8 `TURING` · 9 `MATTER` · 10 `SPACE` · 11 `STRING` ·
  12 `QUANTIZER` · 13 `PARAMETRIC` (com inclinações 24/48 dB) ·
  14 `HARMONY` · 15 `SEQUENCE` · 16 `MIXER` · 17 `MASTER` (barramento
  de saída estéreo + proteção de excelência `OutputStage`) ·
  18 `OSC` (oscilador subtrativo: 5 formas antialias PolyBLEP, 1 V/oct,
  PWM, hard sync, FM linear through-zero, sub-oitava, `drift`) ·
  19 `NOISE` (branco/rosa/brown + S&H + smooth random; `spread`
  uniforme→sino) · 20 `VCA` (duplo, lin/exp, CV atenuvertida soma ao
  knob, softSat, `sum` mixer, `drift`) · 21 `CONTROL` (utilidades de CV:
  atenuversor ±, offset, retificação, slew/lag linear↔RC, soma/média) ·
  22 `LOGIC` (÷/× de clock, AND/OR/XOR, flip-flop T, gate delay, relógio
  interno). **Rack de partida completo.**
- **3 peças generativas** determinísticas (`examples/`, renders em
  `validation-output/`): `peca_generativa` (40 s), `peca_generativa_2`
  (50 s, constelação), `peca_generativa_3` (55 s, barramento semântico).
- **Painel gráfico de teste** (`apps/panel/`, X11 + ALSA + Xrandr): case
  Eurorack que quebra em linhas, coluna de catálogo por família,
  arrastar-para-criar, sugestão de módulo. Design em `apps/panel/design.md`.
- **35 alvos CTest, 100% em Debug e Release** (`-Werror`).
- **Licença:** AGPLv3-or-later (`LICENSE`, `RASGO_MODULAR.md §29`).

**Pendente (marco 3, candidatos):**
- **Módulos essenciais** pra fechar o rack de partida
  (`PESQUISA_MODULOS.md §2.1`): ~~`OSC`~~ **feito** · `NOISE` (ruído +
  S&H + aleatório suave) · `VCA` (avulso, linear/exp) · `CONTROL`
  (atenuversor/offset/soma de CV + slew/lag) · `LOGIC` (divisor de
  clock + lógica booleana de gates + gate delay).
- reverb FDN no `SPACE`; decaimento dependente de frequência na `STRING`;
  voicing vertical (`MelodyVoicingBank`); camada de patch no painel
  (cabos desenhados); sugestão de patch por seed; **front-ends JUCE +
  web/WASM**; contrato do Ensemble Bus.

O registro cronológico detalhado está nas seções "Registro da etapa" mais
abaixo.

## Estado — marco 3 (2026-09-04)

**+12 módulos** (fecha os candidatos de `PESQUISA_MODULOS.md §2.2`):
23 `SH` · 24 `SHAPE` · 25 `LPG` · 26 `CHORD` · 27 `DRIFT` · 28 `SWITCH` ·
29 `SCOPE` · 30 `TRIGSEQ` · 31 `ABACUS` · 32 `WASP` · 33 `MATRIX` ·
34 `MULT` — cada um com dossiê + testes isolados. **34 módulos DSP** no
total.

**Rodada de refinamento (2026-09-04)** — todas com default que preserva o
comportamento antigo bit-a-bit; as 4 peças de exemplo seguem
byte-idênticas:
- **motor:** `connectToParameter`/`followQuality` **aditivos** — a
  modulação soma sobre o knob (`base + offset + depth·fonte`);
  `SignalGraph::setParameterBase` / `parameterUserValue`; o painel roteia
  todo giro de knob por aí;
- **antialiasing:** helper `src/dsp/Oversampler.hpp` (`Oversampler2x` —
  2× meia-banda); `SHAPE` (2× + ADAA de 1ª ordem no folder), `WASP`
  (núcleo não-linear a 2×), `LPG` (suavização do `fc`, filtro é linear);
- **painel:** displays por módulo — `MATRIX` (grade 4×4 clicável),
  `SCOPE` (onda ↔ espectro), `TRIGSEQ` (4 lanes de gate);
- **profundidade por módulo (lista D, 8/8):** `ABACUS` `op` bit a bit
  (4–7) · `SH` `slope` (rise ≠ fall) · `MULT` `dual` (2→2+2) · `LPG`
  `bounce` (overshoot de vactrol) · `SWITCH` `dir` (demux 1→N) · `CHORD`
  `voicing` (condução de vozes) · `MATRIX` `ring` (ring-mod por coluna) ·
  `DRIFT` `anchor` (memória de topologia).

**Limpeza de painel (2026-09-04):** rótulos de widget ≤ 5 caracteres,
centrados acima dos knobs / abaixo dos toggles, fonte de legenda menor;
`tests/test_panel_layout.cpp` como gate de regressão. **35 alvos CTest**,
100% Debug + Release, 0 warnings.

**Pendente (marco 3+):** anti-aliasing 4× opcional no `SHAPE` grave;
overlay editável do `TRIGSEQ`; modo XY do `SCOPE`; grade N×M na `MATRIX`;
`CHORD`/`SWITCH` com mais entradas; estudo `SAMPLER`/`TAPE` (`dr_wav.h`);
voz de percussão dedicada; adaptadores `MIDI`/`CV`/`AUDIO-IN`; front-ends
JUCE + web/WASM.

**Estudo à parte (2026-09-04):** `dossies/ESTUDO_seed_composicao_generativa.md`
— vocabulário de patch/seed da conversa com o ChatGPT, cruzado com o
`PatchSeed.hpp` real; aprofunda a limitação apontada pelo autor (o seed
varia topologia e recebe receitas por caráter, mas não há evolução
composicional ao vivo — `drift` por módulo é cego ao contexto). Também
mapeia Patch Genetics (`MUTATE`/`EVOLVE`/`CROSS`/`FREEZE`), `RASGO Score`
(partitura/registro de eventos) e Learning Engine (hover-learn nos
widgets, precedente Antitotem/Navalha 2) — esses três **não
implementados**. O `MotionEngine` (§3 do estudo) ganhou um **protótipo
de verdade** — ver registro logo abaixo.

## Continuidade de acervo — proporcional ao desenvolvimento

- [x] **Marco 1 (2026-09-01):** ingresso `ARQ-RSM-001` atualizado
  (`RASGO_ARQUIVO/INGRESSO_ARQ-RSM-001.md`), licença registrada
  (AGPLv3-or-later, `LICENSE` + `RASGO_MODULAR.md §29`), manifesto de
  integridade criado
  (`RASGO_ARQUIVO/MANIFESTO_ARQ-RSM-001_marco-1_2026-09-01.md` —
  inventário somente leitura + SHA-256 de fonte/docs + hash dos renders),
  catálogo e `VM_STUDIO_ARCHIVE_INDEX.md` atualizados.
- [ ] **Pendente — cópia de preservação + teste de restauração:** só
  quando o estado deixar de ser móvel — commit/tag do marco 1, migração
  de diretórios ou handoff do marco 2. Aí revisar checksums e testar a
  restauração do pacote.
- Enquanto o graph engine estiver em prototipagem, registrar decisões, testes e
  próximo passo neste arquivo ou na arquitetura; não criar pacote de
  preservação a cada experimento.

## Concluído

- [x] Criar o projeto CMake C++17 puro sem dependência de JUCE.
- [x] Criar o graph engine mínimo.
- [x] Instanciar módulos com estado.
- [x] Conectar portas e avaliar grafo acíclico em ordem topológica.
- [x] Definir tipos estruturais `audio`, `control` e `event`.
- [x] Rejeitar conexões entre tipos incompatíveis.
- [x] Propagar eventos com tipo, valor e timestamp.
- [x] Definir `PortDescriptor` com nome, tipo e unidade.
- [x] Definir `Parameter` com faixa, valor padrão e unidade.
- [x] Expor alteração de parâmetros pelo graph engine.
- [x] Separar estado persistente de parâmetros editáveis.
- [x] Serializar e restaurar patches.
- [x] Testar rejeição de ciclos sem scheduler stateful.
- [x] Registrar o protótipo no inventário global e no painel mestre.

Validação atual: `cmake`, build e `ctest` passam com `1/1` teste.

## Próximo bloco — contrato de dados

- [x] Separar `PortDescriptor`, nomes e unidades das portas.
- [x] Associar `StreamDescriptor` opcional às portas e rejeitar incompatibilidade estrutural.
- [x] Definir `Parameter` com faixa, valor padrão e unidade.
- [ ] Definir política de timestamp e ordenação de eventos.
- [x] Adicionar testes de restauração de parâmetros e estado.

## Depois — áudio e tempo real

- [x] Criar um `AudioBlock` de tamanho fixo, sem alocação no processamento.
- [x] Definir sample rate, número de canais e tamanho de bloco.
- [x] Criar `StreamDescriptor` para negociação de streams multimodais.
- [x] Criar fila fixa de eventos ordenada por timestamp.
- [x] Separar o contrato inicial de processamento de áudio do graph scalar.
- [x] Integrar um graph inicial de processamento de áudio por bloco.
- [x] Criar categorias explícitas `audio`, `control`, `event` e `descriptor`.
- [x] Criar blocos fixos para controle, descritores e eventos.
- [x] Validar categoria de stream na preparação do `AudioGraph`.
- [x] Criar `MultimodalPort` com direção, categoria e descritor.
- [x] Criar payload multimodal tipado.
- [x] Criar `MultimodalGraph` de preparação topológica.
- [x] Expor manifesto multimodal versionado.
- [ ] Integrar processamento de áudio, controle e eventos no graph engine comum.
- [ ] Criar scheduler stateful para feedback e conexões cíclicas explícitas.
- [ ] Testar filas, snapshots e limites de realtime safety.

## Módulos iniciais

- [x] `SOURCE.CONSTANT`
- [x] `INPUT.CONTROL`
- [x] `TRANSFORM.GAIN`
- [x] `METER.VALUE`
- [x] `EVENT.SOURCE`
- [x] `EVENT.GATE`
- [x] `TIME.CLOCK`
- [x] `DECISION.PROBABILITY`
- [x] `ROUTE.SPLIT`
- [x] `MIX.SUM`

## Reutilização e integração

- [ ] Inventariar classes concretas do Rasgo Synth, Aquorbium, Antitotem,
  Navalha 2 e TRIOIO.
- [ ] Comparar cada candidato com os contratos do Modular antes de adaptar.
- [ ] Registrar licença, autoria, dependências e testes de cada candidato.
- [ ] Usar o protótipo em um segundo consumidor antes de promoção compartilhada.
- [ ] Manter o núcleo sem UI até o contrato de dados ficar estável.
- [x] Expor manifesto mínimo de portas e parâmetros para inspeção.

## Conexão entre instrumentos RASGO

O Modular também deve poder funcionar como uma camada de orquestração dos
instrumentos já criados, sem absorver o estado privado, a identidade ou o DSP
interno de nenhum deles.

- [ ] Rever, instrumento por instrumento, quais entradas/saídas podem ser
  expostas com segurança.
- [ ] Definir um manifesto versionado de capacidades por instrumento.
- [ ] Definir adaptadores externos ao core para Aquorbium, Antitotem, Rasgo
  Synth, Navalha 2 e demais instrumentos.
- [ ] Separar portas de áudio, evento, modulação e observação.
- [ ] Definir transporte, sample offset, latência, fan-in e feedback.
- [ ] Validar primeiro um grafo local no mesmo processo com nós fictícios.
- [ ] Conectar dois instrumentos de naturezas diferentes antes de promover o
  contrato como infraestrutura comum.
- [ ] Só depois investigar processos separados, PipeWire, OSC e performance.

Referência transversal: [`ORQUESTRACAO_INSTRUMENTOS.md`](../RASGO_DOCUMENTATION/architecture/ORQUESTRACAO_INSTRUMENTOS.md).

## Critério do próximo marco

O próximo marco estará pronto quando `Parameter`, `PortDescriptor`, estado e
eventos tiverem contrato testado e o grafo puder receber um bloco de áudio sem
alocação durante o processamento.

## Critério permanente de excelência

Cada marco técnico deve ser acompanhado por avaliação progressiva de conceito,
filosofia,
musicalidade, criatividade, experimentação, jogo, interface, código,
documentação, qualidade de áudio, estabilidade, realtime safety e potencial de
performance, além das dimensões de comunicação, webpage, tutorial,
transparência, inovação, arquivística, metodologia, genealogia, taxonomia,
arquitetura, estratégia, topologia, cartografia/fluxografia, genealogia/historiografia, ergonomia, matemática, engenharia,
hardware, textualidade, documentação, sound design, educação e publicação.
Aplicar a matriz geral em
[`METODOLOGIA_DE_DESENVOLVIMENTO.md`](../RASGO_DOCUMENTATION/METODOLOGIA_DE_DESENVOLVIMENTO.md).
Passar nos testes não encerra sozinho um marco artístico.

## Registro da etapa — separação de estado e parâmetros

**Objetivo:** impedir que parâmetros editáveis sejam confundidos com o estado
interno persistente de um módulo.

**Implementação:** o protótipo serializa `parameterState` separadamente de
`state`; `TRANSFORM.GAIN` restaura o ganho pelo primeiro contrato e mantém o
estado interno reservado para evolução futura.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitação:** o contrato ainda é experimental, textual e escalar; não cobre
automação temporal, snapshots de performance ou buffers de áudio.

**Decisão provisória de eventos:** `timestamp` representa uma posição lógica de
frame/amostra; a propagação e a restauração preservam esse valor. A comparação
entre timestamps já é determinística, mas a fila de múltiplos eventos e a
ordenação estável ainda dependem do scheduler stateful.

**Próximo passo:** definir a API de processamento por bloco e separar áudio,
controle e eventos.

## Registro da etapa — contrato inicial de áudio

**Objetivo:** estabelecer armazenamento de áudio previsível antes de conectar
buffers aos módulos do grafo.

**Implementação:** `AudioBlock` possui capacidade fixa de 256 frames por 8
canais, sample rate configurável e acesso validado por canal/frame. `clear()`
limpa apenas a região ativa e não cria memória durante o processamento.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitação:** os módulos e as portas ainda usam valores escalares; o bloco
ainda não participa da avaliação do grafo.

**Próximo passo:** integrar o processamento por bloco ao graph engine e definir
o contrato conjunto com controle e eventos.

## Registro da etapa — descritor de stream

**Objetivo:** transportar metadados de tempo, taxa, offset, dimensões, domínio
e capacidade de bloco na fase de preparação.

**Implementação:** `StreamDescriptor` compara compatibilidade estrutural sem
participar do caminho realtime; `offset` permanece metadado de latência/tempo,
não critério de incompatibilidade.

**Referência:** contrato de atributos e frames do PiPo, traduzido para uma
estrutura própria do RASGO.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitação:** ainda não há negociação entre nós nem suporte a labels,
metadados de unidades ou streams variáveis.

**Próximo passo:** associar descritores às portas e diferenciar áudio,
descritores, controle e eventos no graph multimodal.

## Registro do lote autônomo — dez tarefas

1. `StreamDescriptor` recebeu unidade e labels, com compatibilidade estrutural.
2. `AudioGraph::prepare(StreamDescriptor)` passou a preparar buffers e guardar
   o contrato do stream.
3. `EventQueue<N>` passou a ordenar eventos por timestamp sem alocação.
4. `TIME.CLOCK` produz eventos de tick com posição lógica configurável.
5. `DECISION.PROBABILITY` usa seed controlável e probabilidade editável.
6. `ROUTE.SPLIT` replica uma entrada de áudio para duas saídas.
7. `MIX.SUM` soma duas entradas de áudio.
8. Cada módulo expõe manifesto mínimo de portas e parâmetros.
9. O inventário global recebeu os contratos e referências reutilizáveis do lote.
10. A validação final compilou o projeto e executou o teste integrado.

**Limitações:** os módulos scalar ainda não formam o graph multimodal comum;
não há fan-in no `AudioGraph`, scheduler stateful, labels propagados em runtime,
automação ou segundo consumidor externo.

**Próximo lote:** associar `StreamDescriptor` às portas multimodais e testar
áudio, descritores, controle e eventos no mesmo contrato de preparação.

## Registro da etapa — descritores nas portas

**Objetivo:** fazer a compatibilidade do grafo considerar tipo de porta e
estrutura do stream, sem obrigar todo módulo scalar a possuir metadados de
áudio.

**Implementação:** `PortDescriptor` aceita `StreamDescriptor` opcional;
`Graph::connect()` rejeita descritores incompatíveis quando ambos os lados os
declaram. A preparação do `AudioGraph` também conserva seu descritor completo.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitação:** ainda não há negociação de múltiplas modalidades no mesmo
processamento nem propagação de labels/descritores durante `process()`.

**Próximo passo:** criar um contrato de porta multimodal que diferencie áudio,
controle, eventos e descritores na preparação do grafo.

## Registro do lote autônomo — dez etapas multimodais

1. Criada `StreamCategory` com `Audio`, `Control`, `Event` e `Descriptor`.
2. `StreamDescriptor` passou a carregar categoria, unidade e labels.
3. `ControlBlock` fixo foi criado sem alocação no caminho de processamento.
4. `DescriptorBlock` fixo foi criado para frames de análise.
5. `EventBlock` foi criado sobre fila fixa ordenada.
6. `AudioGraph` passou a rejeitar descritores não-audio na preparação.
7. O manifesto de módulo passou a declarar `version: 1`.
8. Testes cobrem blocos, categorias, limites e rejeição multimodal.
9. Atlas e inventário global foram atualizados com o novo contrato.
10. Build e `ctest` foram executados com sucesso.

**Limitações:** os blocos ainda são contratos de armazenamento; não há um
processador multimodal comum, fan-in de eventos/descritores, labels propagados
durante processamento ou scheduler stateful.

**Etapa concluída:** `MultimodalPort` e a preparação inicial do grafo já foram
criados; o próximo lote deve tratar o processamento desses payloads.

## Registro do lote autônomo — dez etapas de preparação multimodal

1. `MultimodalPort` foi criado com direção, categoria e `StreamDescriptor`.
2. `MultimodalPayload` foi criado para áudio, controle, descritores e eventos.
3. Compatibilidade exige direção oposta, mesma categoria e stream compatível.
4. `MultimodalGraph` foi criado para registrar portas e conexões.
5. A preparação calcula ordem topológica e rejeita ciclos.
6. O manifesto multimodal passou a expor versão e portas.
7. Testes cobrem conexão válida, payload de áudio e incompatibilidade.
8. A relação com PiPo foi registrada como inspiração de negociação de streams.
9. Atlas e inventário global receberam o contrato `MultimodalGraph`.
10. Build e `ctest` foram executados com sucesso.

**Limitações:** o grafo ainda não processa payloads; não há fan-in, adapters,
negociação de labels entre módulos ou scheduler stateful.

**Próximo lote:** criar processadores multimodais mínimos e uma preparação que
negocie capacidade sem executar trabalho no callback realtime.

## Registro do lote de 30 tarefas

### Bloco A — processamento de payloads

1. `MultimodalProcessor` define a interface de processamento tipado.
2. `AudioPayloadProcessor` usa `AudioGainProcessor` sem alocação de bloco.
3. `ControlPayloadProcessor` copia controles com capacidade preparada.
4. `DescriptorPayloadProcessor` copia frames de análise.
5. `EventPayloadProcessor` preserva a fila fixa de eventos.
6. `MultimodalPayload` e validação de categoria foram usados como resultado tipado.
7. `MultimodalChain` executa uma cadeia linear preparada.
8. Processadores rejeitam payloads da categoria errada.
9. Pipeline de áudio com dois ganhos foi testado.
10. Falhas de capacidade e categoria foram testadas.

### Bloco B — tempo, estado e segurança

11. `ProcessContext` registra frame inicial, tempo e quantidade de frames.
12. `processAt()` associa contexto temporal à execução.
13. `reset()` percorre os processadores da cadeia.
14. `MultimodalSnapshot` registra preparação, categoria e contexto.
15. `realtimeSafeBase()` explicita o escopo seguro da infraestrutura base.
16. Capacidades máximas dos blocos permanecem fixas e documentadas.
17. Fan-out continua permitido por conexões independentes.
18. Fan-in implícito foi rejeitado; mixers explícitos serão necessários.
19. Ciclos continuam rejeitados na preparação topológica.
20. Manifestos e preparação permanecem fora do callback realtime.

### Bloco C — arquivo, proveniência e validação

21. Contratos foram registrados no inventário global.
22. Tarefas e limitações foram atualizadas.
23. O mapa PiPo/IRCAM recebeu a etapa de processamento.
24. A relação com Mutable Instruments permanece registrada como pesquisa.
25. O código foi identificado como autoria própria do workspace, com licença
   ainda a definir.
26. A arquitetura viva recebeu o novo marco.
27. O handoff de continuidade foi atualizado.
28. Build C++17 com warnings tratados passou.
29. `ctest` e `git diff --check` passaram.
30. O próximo marco ficou definido: adapters multimodais e fan-in explícito.

**Limitação geral:** a cadeia multimodal já processa payloads homogêneos, mas
ainda não combina modalidades num mesmo nó, não usa scheduler stateful e não
tem consumidor externo.

## Registro da etapa — aprofundamento do Atlas: CNMAT, ossia, OpenMusic e Freesound

**Objetivo:** ampliar o diálogo do RASGO com fontes afins, verificando
potencialidades, proveniência e limites de licença.

**Documentação:** criado
`RASGO_DOCUMENTATION/architecture/CNMAT_OSSIA_FREESOUND_POTENCIALIDADES.md`;
atualizados o Atlas e `INVENTARIO_FONTES_ATLAS.md`.

**Resultados:** CNMAT foi classificado como ecossistema/genealogia; `odot` e
`libo` ficaram em pesquisa até auditoria de licença; ossia foi relacionado a
cenários, temporalidade e rede; OpenMusic a estrutura/constraints;
Freesound a corpus com ficha individual de proveniência.

**Validação:** fontes primárias consultadas em 2026-08-16; links, decisões e
próximos experimentos registrados nos documentos citados. `git diff --check`
não introduziu erro nos trechos editados; o Atlas já possuía whitespace
histórico nas linhas iniciais.

**Limitação:** ainda não foram fixados commits/licenças de cada arquivo de
`odot`/`libo`, nem criado importador Freesound ou `PayloadEnvelope`.

**Próximo passo:** auditar os repositórios CNMAT e prototipar envelope de
payload com `origin`, `timestamp`, `category` e `provenance`.

## Registro da etapa — auditoria CNMAT e contrato de envelope

**Objetivo:** substituir a anotação genérica sobre CNMAT por uma leitura
primária dos repositórios e uma hipótese de contrato autoral para o RASGO.

**Resultados:** o README do `odot` confirma payload agregado, linguagem de
expressões e primitivas de tempo/agendamento; seu `license.txt` permite uso,
cópia, modificação e distribuição sob preservação integral dos avisos. `libo`
foi confirmado como biblioteca de OSC/expressões e dependente de Flex/Bison,
mas sua licença ainda não foi consolidada.

**Decisão:** não importar código. O RASGO adotará apenas a hipótese de um
`PayloadEnvelope` próprio, com schema, origem, timestamp, categoria, payload,
descritor, latência e proveniência.

**Arquivos:** Atlas, inventário de fontes e
`CNMAT_OSSIA_FREESOUND_POTENCIALIDADES.md`.

**Validação:** leitura das páginas primárias e atualização dos registros em
2026-08-16; nenhum repositório externo foi copiado para o workspace.

**Próximo passo:** especificar o envelope em C++/JSON fora do callback realtime
e criar testes de rejeição para origem, categoria e proveniência ausentes.

## Registro da etapa — primeiro PayloadEnvelope

**Objetivo:** materializar o contrato autoral inspirado pelo Atlas para
transportar payloads multimodais sem perder contexto arquivístico.

**Implementação:** `PayloadProvenance` e `PayloadEnvelope` foram adicionados
ao núcleo. O envelope registra schema, origem, timestamp de frame, tempo lógico,
categoria, payload, descritor opcional, latência e proveniência. `valid()` rejeita
origem/proveniência incompletas, categoria incompatível, descritor inválido e
latência negativa. `toJson()` produz representação de metadados para arquivo;
não é chamado no processamento realtime.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** CMake, build C++17 com `-Wall -Wextra -Wpedantic -Werror` e
`ctest`: 1/1 teste passou em 2026-08-16.

**Limitações:** o JSON ainda não faz escape de strings e o envelope não foi
conectado ao grafo multimodal nem a um consumidor externo.

**Próximo passo:** separar um serializador arquivístico seguro e decidir onde o
envelope será anexado aos adapters multimodais e ao fan-in explícito.

## Registro da etapa — serializador arquivístico v1

**Objetivo:** transformar um `PayloadEnvelope` válido em um registro de
metadados versionado e seguro para persistência posterior.

**Implementação:** `escapeJson()` trata barras, aspas, tabulações e quebras de
linha. `PayloadEnvelope::toJson()` agora inclui latência e todos os campos de
proveniência. `ArchiveSerializer::serializeEnvelope()` valida o envelope e
produz o formato `rasgo-payload-envelope` versão 1, declarando que os bytes do
payload são externos ao manifesto.

**Limite deliberado:** a camada não faz I/O, checksum nem gravação; isso ficará
em um adapter de armazenamento arquivístico. Assim, preparação e persistência
continuam separadas do callback realtime.

**Validação:** build C++17, warnings tratados e `ctest`: 1/1 teste passou.
Também foi testado escape de aspas e rejeição de envelope inválido.

**Próximo passo:** criar o adapter de armazenamento com `manifest.json`, dados
do bloco, proveniência, licenças e checksums.

## Registro da etapa — adapter de armazenamento arquivístico

**Objetivo:** materializar um pacote mínimo recuperável a partir de um envelope
válido, sem misturar persistência com o processamento realtime.

**Implementação:** criado `src/core/ArchiveStorage.hpp`. O adapter grava
`manifest.json`, `payload.bin` e `provenance.json`; serializa áudio, controle,
descritores e eventos em formato binário de bloco; registra tamanho e checksum
FNV-1a-64 no manifesto.

**Decisão de integridade:** FNV-1a-64 é apenas marcador de integridade e não
assinatura criptográfica. Uma camada futura poderá adicionar SHA-256 ou
assinatura quando o fluxo de distribuição exigir.

**Validação:** teste cria um diretório temporário controlado, verifica os três
arquivos e remove somente esse artefato de teste. CMake, build C++17 e `ctest`:
1/1 passou em 2026-08-16.

**Limitações:** o formato binário ainda depende da arquitetura/endianness e não
possui leitor/recuperador; os arquivos existentes no diretório de destino são
substituídos pelo adapter.

**Próximo passo:** documentar o formato binário e implementar leitura com
validação de checksum antes de promover o pacote a arquivo confiável.

## Registro da etapa — leitor e verificação de integridade

**Objetivo:** permitir a leitura controlada de um pacote arquivístico e detectar
alterações no payload antes de sua aceitação.

**Implementação:** `ArchivePackage` recebe manifesto, proveniência e bytes do
payload. `ArchiveStorage::readPackage()` lê os três arquivos, extrai o checksum
FNV-1a-64 declarado no manifesto e compara-o ao valor recalculado sobre
`payload.bin`.

**Validação:** o teste leu um pacote íntegro e depois anexou um byte ao payload;
a leitura adulterada foi corretamente rejeitada. Build C++17 e `ctest`: 1/1
passou em 2026-08-16.

**Limitações:** ainda não há decodificação do payload para os tipos RASGO, nem
validação criptográfica, endianness ou versão de formato além do manifesto.

**Próximo passo:** formalizar o formato binário (ordem dos campos, endianness,
tipos e compatibilidade) e decidir se o arquivo confiável exigirá SHA-256.

## Registro da etapa — processamento inicial por bloco

**Objetivo:** testar processamento de áudio em bloco sem alocação no caminho de
execução.

**Implementação:** `AudioProcessor` define o contrato e
`AudioGainProcessor` demonstra leitura/escrita de `AudioBlock`, rejeitando
configurações incompatíveis por retorno booleano.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitação:** o `AudioGraph` ainda é paralelo ao graph scalar, aceita apenas
uma entrada por nó e não possui scheduler realtime ou fan-in.

**Próximo passo:** definir como o graph comum representará áudio, controle e
eventos sem colapsar os contratos.

## Registro da etapa — primeiro AudioGraph

**Objetivo:** conectar processadores por bloco mantendo buffers e ordem
topológica preparados fora do caminho realtime.

**Implementação:** `AudioGraph` prepara um buffer fixo por nó, calcula a ordem
acíclica em `prepare()` e executa os processadores sem alocação em `process()`.

**Arquivos:** `src/core/Graph.hpp` e `tests/test_graph_engine.cpp`.

**Validação:** build C++17 e `ctest` em 2026-08-16 — `1/1` teste passou.

**Limitações:** somente uma entrada por nó, sem fan-in, feedback, eventos ou
controle; o graph scalar continua separado.

**Próximo passo:** projetar o contrato de portas multimodais do graph comum.

## Registro da etapa — retomada 2026-09-01: build Release consertada

**Objetivo:** o teste não podia depender de `NDEBUG`.

**Problema:** `-DCMAKE_BUILD_TYPE=Release` define `NDEBUG`, `assert()` vira
no-op, e as flags `bool ...Rejected` dos blocos try/catch viravam
"set but not used" com `-Werror` → a build Release quebrava e, nas builds
onde compilava, os 63 `assert` do teste ficavam vazios. As validações
registradas em 2026-08-16 rodaram sem `CMAKE_BUILD_TYPE` (com `assert`
ativo), então não pegaram isso.

**Implementação:** `tests/test_graph_engine.cpp` agora tem um `check()`
que sempre avalia, conta falhas e faz `main()` retornar != 0; `assert` é
redefinido pra ele (diff mínimo, sem `<cassert>`). `.gitignore` do RASGO
passou a ignorar `**/build/`.

**Validação:** `cmake` + build + `ctest` em 2026-09-01 nas três
configurações (sem tipo, `Release`, `Debug`) — 1/1 teste passou nas três.

**Limitação:** continua um único alvo de teste, uma função gigante. Os
três grafos (`Graph` scalar, `AudioGraph`, `MultimodalGraph`) seguem como
ilhas paralelas que não se processam em conjunto.

**Próximo passo:** ver "avaliação da retomada" abaixo.

## Avaliação da retomada (2026-09-01) — o que trava o avanço

O protótipo acumulou **abstrações paralelas que não se conectam**:
- `Graph` (escalar, os 10 módulos, serialização, rejeição de ciclo);
- `AudioGraph` (blocos de áudio, uma entrada por nó, sem fan-in);
- `MultimodalGraph` (só topologia/preparação, não processa payload);
- `MultimodalChain` (processa, mas só cadeia linear);
- `PayloadEnvelope` / `ArchiveSerializer` / `ArchiveStorage` (arquivística).

Cada "próximo marco" dos últimos registros aponta pra **unificar o grafo**
(um só grafo que processa áudio + controle + evento em ordem topológica,
sem colapsar os tipos) - e em vez disso foram adicionadas mais ilhas.

**Decisão do autor pendente** - qual direção priorizar:
1. **Unificar o grafo** (`MultimodalGraph` passa a processar de verdade,
   com fan-in explícito, substituindo `Graph`+`AudioGraph`+`MultimodalChain`).
   É a fundação que destrava todo o resto, mas é o maior refactor.
2. **Scheduler stateful** (feedback/ciclos, fila de eventos ordenada,
   política de timestamp) - sobre a base atual, sem unificar ainda.
3. **Inventário de reaproveitamento** (varrer Rasgo Synth, Aquorbium,
   Antitotem, Navalha 2, TRIOIO por classes concretas adaptáveis) - antes
   de escrever mais core.
4. **Primeiro módulo DSP de verdade** (um oscilador, um filtro) num
   `AudioGraph` real, pra o protótipo sair do "tudo escalar" e virar som.

## Registro da etapa — 2026-09-01: fundação de áudio (SignalGraph)

**Decisão do autor:** finalidade principal = **criação de música ORIGINAL
GENERATIVA**, num **instrumento performático**, com **módulos de excelência**
codados **um a um** (cada um com a pesquisa dos conceitos/tecnologias de que
parte), **sem limite de módulos**, e onde **os fluxos proporcionam variedade
sonora** (o grafo não é encanamento neutro). Ver `RASGO_MODULAR.md §35.5`.

**Objetivo:** o caminho de áudio unificado que faltava (`CORE-GRAPH-CONNECTION`,
"prioridade arquitetural") - a base sem a qual nenhum módulo faz som.

**Implementação:**
- `src/core/ControlSnapshot.hpp` - barramento de controle por SNAPSHOT
  COERENTE (seqlock portátil, RT-safe, retry limitado de 16). Adaptado do
  PADRÃO `EnergyControlBus` de `TRIOIO/src/core/TrioioBrain` (outro projeto -
  não é import; a estrutura foi generalizada de campos fixos pra N slots e o
  seqlock passou pra variante com barreiras de thread, portátil fora de x86).
  Papel: trocar o plano do grafo na fronteira de bloco + modulação coerente.
- `src/core/SignalGraph.hpp` - o grafo:
  - `Signal` - base de módulo que processa áudio por bloco (portas tipadas,
    parâmetros, manifesto); `prepare()` fora do áudio, `process()` sem alocação.
  - `Cable` - **a conexão é um OBJETO que processa** (Atlas §9-11). Estado
    intacto/rompido; ao romper, a **cicatriz** segura o último bloco e o repete
    decaindo ~350 ms até silêncio absoluto (Atlas §37, Clouds - "romper não
    significa apagar"). Arquitetura já prevê ganho/atraso/filtro/saturação/
    probabilidade na própria conexão.
  - **Feedback** com atraso explícito de um bloco (fora da verificação de
    ciclo); ciclo sem a marca `feedback` é rejeitado em `prepare()`.
  - **Modulação saída -> parâmetro** (`connectToParameter`, profundidade+offset).
  - **Fan-in explícito** - uma conexão por porta de entrada; soma via módulo
    `Sum`; ganho por conexão (orçamento).
- `tests/test_signal_graph.cpp` - novo alvo. Cobre: snapshot coerente sob
  concorrência (escritor+leitor, 20k iterações), cadeia+fan-in, ruptura/
  cicatriz/reconexão, feedback (ponto fixo) + rejeição de ciclo, modulação
  de parâmetro.

**Validação:** `cmake` + build + `ctest` em 2026-09-01, **2/2 alvos passam**
nas três configurações (sem tipo, `Release`, `Debug`); o alvo novo rodado 8×
seguidas sem falha de corrida. `-Wall -Wextra -Wpedantic -Werror` limpo.

**Limitação:** os módulos ainda são só os de teste (constante, ganho, soma) -
nenhum DSP real, nenhum som musical ainda. `Signal` é um mundo novo, não
substitui `Graph`/`AudioGraph`/`MultimodalGraph` (essas ilhas continuam;
serão migradas/aposentadas conforme o `SignalGraph` cobrir o que elas fazem).
`setParameter` por link de modulação faz busca linear por string a cada bloco.

## Registro da etapa — 2026-09-01: pesquisa consolidada + Módulo 1

**Pesquisa de módulos - achada.** Estava em
`AQUORBIUM/aquorbium-arquitetura.md §9-12` (commit `560857c`, 9/ago):
triagem ModularGrid Top 100 (~25-30 princípios distintos depois de tirar
utilidade pura e redundância) + pesquisa EMW (hardware que o autor usa) +
varredura SchneidersLaden. Feita no contexto do Aquorbium porque o RASGO
Modular só nasceu em 16/ago. Consolidada (coluna *módulo → conceito*, sem
as apropriações Aquorbium) em `RASGO_MODULAR/PESQUISA_MODULOS.md`, que
adota o `MODULE_DEVELOPMENT_STANDARD.md` do Aquorbium (dossiê + 8 portões
+ 3 modos).

**Módulo 1 - `FUNCTION` (gerador de função).** Dossiê em
`dossies/01_gerador_de_funcao.md`. Uma rampa que é envelope/LFO/oscilador
conforme a taxa (Tides/Stages), com `drift` (random-walk com seed) como
desvio Rasgo, PolyBLEP no wrap da parte serra. Arquivos:
`src/dsp/FunctionGenerator.hpp`, `tests/test_function_generator.cpp`,
`examples/primeiro_fragmento.cpp`, `src/io/WavWriter.hpp`.

**Validação:** `ctest` 3/3 alvos nas 3 configs (sem tipo / Release /
Debug). Frequência ±2%, formas limitadas sem NaN, `drift` determinístico
e reprodutível, alias @23 kHz -47 dB / @22 kHz -41 dB (limite; polyBLAMP =
2ª camada), integração no `SignalGraph` com ruptura/cicatriz.

**Primeira música:** `validation-output/primeiro_fragmento.wav` (8 s) -
LFO+drift modula a voz ±0,9 oitava; aos 4 s o cabo rompe e a cicatriz
decai ao silêncio. Gitignore atualizado (`RASGO_MODULAR/validation-output/`).

## Registro da etapa — 2026-09-01: modelo de conexão

**Decisão do autor** sobre como os módulos se conectam (descartado o
"arrastar cabo de jack a jack" do Eurorack): **três modelos em camadas** —
(1) **matriz de roteamento** como superfície de edição (linhas=saídas,
colunas=entradas/parâmetros, célula=`Cable`); (2) **constelação/campo**
como superfície de performance (distância entre nós = intensidade da
relação, reaproveita `PER-RS-CONSTELLATION-GEOMETRY-V1`); (3) **barramento
semântico** como camada opcional depois (conectar por SIGNIFICADO -
energia/tensão/brilho, padrão `EnergyControlBus` do TRIOIO). Por baixo das
três: **cabo-como-objeto** e patch **serializado como partitura legível**.
E: **toda conexão nasce com probabilidade de condução** (Marbles/Branches
embutido).

**Feito nesta etapa:**
- `Cable::conductance` (0..1, default 1) - probabilidade de conduzir,
  re-sorteada a ~20 Hz com seed determinística por cabo; quando não
  conduz, o sinal cai em ~30 ms a silêncio e fica (estado "intermitente"
  do Atlas §24) até o próximo "conduz". Teste no `test_signal_graph.cpp`.
- `SignalGraph::serialize()` / `deserialize(text, factory)` - o patch como
  PARTITURA legível (Atlas §23): uma linha por nó/cabo/mod, salvável e
  versionável. Round-trip textual estável testado.

**Ainda a fazer no modelo de conexão:** enumeração de slots pra a view de
matriz; posições de nó + `couplingFromDistance()` pra a constelação;
camada semântica. Nenhum bloqueia os módulos DSP.

## Registro da etapa — 2026-09-01: descrição de painel + frontends

**Decisão do autor sobre frontend:** os DOIS - **JUCE** (desktop + iOS/
AUv3 + Android; e é onde os instrumentos RASGO se unem no Ensemble Bus) e
**web/WASM** (esboço rápido, patch = URL, PWA no celular). Um `.rmp` de
texto e uma `Panel` servem os dois. O core continua **framework-free**
(contrato não herda JUCE - regra da ORQUESTRACAO). Algo "mais adaptado ao
celular" fica pra depois da base. Padrão de excelência (contrato RT,
UTF-8, contrato responsivo, 8 portões) vale pro app e pros módulos.

**Feito:** `src/core/Panel.hpp` - `Widget`/`Panel` declarativos (dado
puro: tipo, label, `bind` = param id / `in:porta` / `out:porta`, posição
em grade abstrata; 1 HP = 4 unid). `Signal::panel()` virtual com
auto-layout padrão; `FunctionGenerator::panel()` próprio (12 HP: display
de forma + RATE/SLOPE/DRIFT/SYNC + 5 jacks). `src/io/AsciiPanel.hpp` -
renderizador de TEXTO só pra teste + `validatePanel()` (todo bind
resolve). Teste no `test_function_generator.cpp`. **Não é linguagem
visual** - é o esqueleto neutro; a visualidade própria do Rasgo é thread
de design à parte, autor liderando (Atlas §26/§27).

**Nota:** autor trabalhando em paralelo com o Codex no
`RASGO_SYNTH/rasgo-synth-performance` - manter o footprint desta linha
em `RASGO_MODULAR/` (edits em `INVENTARIO_GLOBAL_MODULOS.md` e no Atlas
são aditivos/mergeáveis).

## Registro da etapa — 2026-09-01: Módulo 2 (`FILTER`)

Dossiê `dossies/02_filtro.md`. Filtro das **três irmãs**: 3× SVF TPT
(Cytomic) na mesma frequência de corte, saídas `low`/`center`/`high`/`all`;
`spread` desloca as três (±2 oitavas) de "3 tomadas" a **formante** (a
relação entre as saídas é o processo, Warps/Atlas §39); `drive`
sine-shaping; **auto-oscilação** via `k` levemente negativo perto de
`resonance=1` + não-linearidade no laço (satura o estado → ciclo-limite,
não NaN). Painel próprio (12 HP). `src/dsp/Filter.hpp`,
`tests/test_filter.cpp`.

**Validação:** 4/4 alvos nas 3 configs. `spread=0` → LP/HP corretos;
`spread=1` → duas bandas separadas (formante); `resonance→1` auto-oscila
na frequência de corte ±5%; varredura de cutoff + drive sem NaN;
determinístico; integração no grafo. `examples/primeiro_fragmento.cpp`
atualizado (3 LFOs modulando voz + cutoff + spread; ruptura aos 5 s) →
`validation-output/primeiro_fragmento.wav` (10 s): timbre em movimento +
cicatriz.

**Próximo passo:** Módulo 3 - **comportamento de `Cable` = relação**
(Warps: ring-mod / waveshaping / cross-mod NA conexão, não módulo).
Depois: DECISION (SAPÈL - distribuição de probabilidade configurável),
clock (euclidiano + AND/OR de divisores), VCA/envelope. Ver
`PESQUISA_MODULOS.md §2`.

Toda etapa futura deve registrar objetivo, arquivos alterados, validação,
limitações e próximo passo neste arquivo ou no documento técnico mais próximo.

## Registro da etapa — 2026-09-01: Módulo 3 (`Cable::Relation`)

Dossiê `dossies/03_relacao_de_cabo.md`. **A relação é o processo**
(Warps, Atlas §39): a conexão passa a poder **processar** — lê um segundo
sinal (`companion`, do bloco anterior, o que evita ciclo topológico e
permite auto-relação) e o combina com o que atravessa, seco/molhado por
`amount`. Três relações: `RingMod` (`x·(1-a)+(x·y)·a`), `Fold`
(wavefolder de `x` dirigido por `|y|`, até 4 reflexões em ±1),
`Difference` (`x - a·y`). Sem estado — barato e determinístico.
Não é um nó: é configuração de `Cable` (`setRelation`, `hasRelation`,
`relation`, `companion`, `relationAmount`). Serializado na linha `cable`
do patch (`relation= companion= amount=`), só quando há relação —
patches antigos ficam idênticos.

**Arquivos:** `src/core/SignalGraph.hpp` (enum `Relation`, `setRelation`,
`applyRelation`, `relationName`/`relationFromName`, serialize/deserialize,
loop de `process()` resolve o companion de `previousOutputs_`),
`tests/test_signal_graph.cpp` (`testCableRelation`).

**Validação:** 4/4 alvos nas 3 configs. Álgebra por amostra conferida
(`RingMod` 1→0,20; `RingMod` 0,5→0,35; `Difference` 1→0,10; `Fold`
1→0,90); `amount=0` ≡ `None`; round-trip textual idêntico
(`relation=ring companion=1:0 amount=0.75`); `hasRelation`/`relation`
preservados na desserialização.

**Limitações:** troca de `Relation` em performance é dura (sem
cross-fade); `Fold` sem oversampling (alias não medido); companion tem
1 bloco de latência; só 3 das ~9 relações de Warps. Todas 2ª camada.

## Registro da etapa — 2026-09-01: Módulo 4 (`DECISION`)

Dossiê `dossies/04_decisao.md`. Aleatoriedade **domada** num módulo:
`gate` de Bernoulli (`bias`), CV bipolar `x`/`y` (dois sorteios
independentes) com distribuição **uniforme→sino** por `shape` (média de 4
uniformes, central limit — sem `log`/`cos`), `spread` escala a excursão,
`steps` quantiza em 1..32 níveis, `slew` faz S&H suave (a cola de
theremin do `RASGO_SYNTH`). **Déjà-vu** (`dejavu`, `loop_length`): buffer
circular de 16; com prob. `dejavu` relê o valor de `loop_length` passos
atrás e reescreve — em `dejavu=1` trava um laço; em 0 é sempre novo.
Avança por **evento**: `trigger` externo (borda de subida) OU clock
interno em `rate` quando `trigger` não está patchado. Determinístico
(xorshift semeado em `prepare()`).

**Arquivos:** `src/dsp/Decision.hpp`, `tests/test_decision.cpp`,
`CMakeLists.txt` (alvo `rasgo_modular_decision_tests`).

**Validação:** 5/5 alvos (Debug + Release). `bias` calibrado (0→nunca,
1→sempre, 0,5/0,8 ±5% em 4000 passos); `spread=0` → `x`≡0; distribuição
uniforme desvio ~0,577, sino claramente menor e não colapsa;
`steps=3` → `x` só em {−1,0,1}, os três ocorrem; `dejavu=1
loop_length=4` → `x` periódico-4 depois do buffer cheio, e não constante;
dois renders byte-idênticos; `slew=0,5` limita o salto por amostra a
<0,02; 5000 blocos estéreo finitos, gate ∈ {0,1}, sem alocação;
integração no grafo (`DECISION.x`→`cutoff_mod` de um `FILTER`, `y`→`in`);
painel fecha (14 HP).

**Limitações:** sem entrada de seed/`reset`; sino por soma de uniformes
(não Box-Muller); `x` e `y` sem correlação ajustável; quantização em
tensão, não em escala musical. Todas 2ª camada.

## Registro da etapa — 2026-09-01: Módulo 5 (`CLOCK` / `EuclidClock`)

Dossiê `dossies/05_clock.md`. O **tempo** do patch. Três gates:
`clock` (passo estável em `bpm·mult`), `euclid` (E(k,n) por fórmula de
Bresenham `((i+rotate)·fill) mod length < fill` — O(1), sem buffer),
`accent` (AND/OR de dois divisores → polirritmia de graça, vpme Euclidean
Circles). `swing` atrasa passos ímpares; `drift` faz random-walk no
andamento (±12%, princípio do Módulo 1). Avança por clock interno OU,
com `ext_clock` conectado, pelas bordas externas (período estimado do
intervalo entre bordas). `reset` zera contador+fase. Nome de classe
`EuclidClock` pra não colidir com o `Clock` legado de `Graph.hpp`
(`type()` continua `"CLOCK"`).

**Arquivos:** `src/dsp/EuclidClock.hpp`, `tests/test_clock.cpp`,
`CMakeLists.txt` (alvo `rasgo_modular_clock_tests`).

**Validação:** 6/6 alvos (Debug + Release). Andamento interno bate
(120 BPM×2→~4 Hz; 60 BPM×1→~1 Hz); `E(3,8)`→~3/8 dos passos (30±2 em 80);
acento OR de 4 e 3 em [0,24)=12, AND=múltiplos de 12=2; `fill=0` isola o
acento; `drift=0` byte-idêntico, `drift>0` reprodutível (mesma seed);
`ext_clock` ~10 Hz → euclid segue as bordas; `reset` reinicia o padrão;
4000 blocos estéreo (ext+reset+swing+drift) com saídas ∈ {0,1}, sem
alocação; integração no grafo (`CLOCK.euclid→DECISION.trigger→FILTER`);
painel fecha (16 HP).

**Limitações:** Bresenham ≠ Bjorklund canônico numa minoria de (k,n)
(rotação diferente, mesma densidade); sem saída de fim-de-ciclo; um só
plano euclidiano; sem sincronização a transporte global (Ensemble Bus —
G4). Todas 2ª camada.

## Registro da etapa — 2026-09-01: Módulo 6 (`ENVELOPE`) + primeira peça longa

**Módulo 6.** Dossiê `dossies/06_envelope.md`. Contorno A/D/(S)/R
disparado por gate **com VCA embutido** — gera a forma E a aplica a um
áudio. `mode` gated (ASR, segue o gate) ou trigger (AD, one-shot).
Segmentos = fasor linear moldado por `curve` (`shape(x)=x^e`, e de 4 a
0,25: convexa↔côncava; 0,5 linear). VCA:
`out = in·((1−vca_depth)+vca_depth·envUnit)` — `depth=0` passa intacto.
`time_mod` (1 V/oct) estica os três tempos; `level` escala só o `env`
out. Determinístico (sem RNG). `src/dsp/Envelope.hpp`,
`tests/test_envelope.cpp`, alvo `rasgo_modular_envelope_tests`.

Validação: 7/7 alvos (Debug + Release). AD sobe/pico≈level/decai/volta a
0; ASR segura no `sustain`; VCA `depth=1` segue o env e fecha, `depth=0`
áudio bit-a-bit; curva côncava sobe mais rápido no início; `time_mod=1`
dobra os tempos; `level=0,5` → pico do env 0,5; byte-idêntico; 4000
blocos estéreo sem NaN/alocação; grafo
(`CLOCK.euclid→ENVELOPE.gate`, `FUNCTION→in` → voz articulada); painel
12 HP.

**Primeira peça longa.** `examples/peca_generativa.cpp` →
`validation-output/peca_generativa.wav` (**40 s**, alvo
`rasgo_modular_peca_generativa`). Usa os 6 módulos + a fundação:
`CLOCK` (96 BPM ×2, E(7,16) rot 2, swing 0,22, drift 0,35, acento 4/6
OR) dispara `DECISION-altura` (steps 5, slew, déjà-vu 0,6/6 → melodia
que se lembra) e `DECISION-timbre` (via accent) e o `ENVELOPE`; a `VOZ`
(FUNCTION 110 Hz, drift) vai ao `FILTRO` por um **Cable com relação
RingMod** (companion = LFO 47 Hz, amount 0,2) e **conductance 0,9**
(dropouts); LFO 0,04 Hz modula `spread`, o `env` (bloco anterior,
feedback) abre o `cutoff`; `FILTRO.all → ENVELOPE → saída`, cabo
**rompe aos 28 s** (cicatriz) e **reconecta aos 34 s**.
**Determinística: dois renders byte-idênticos.** RMS por trecho de 5 s:
0,089 / 0,085 / 0,086 / 0,092 / 0,107 / 0,073 / 0,026 (cicatriz) / 0,094
— percurso que varia sobre a mesma estrutura, sem repetição no tempo.

**Marco 1 completo:** fundação (SignalGraph/Cable/ControlSnapshot) +
Módulos 1–6 + relação de cabo + serialização + descrição de painel +
peça. 7 alvos de teste, 2 configs, tudo verde.

**Próximo passo (marco 2, candidatos):** enumeração da matriz de conexão
no `SignalGraph` (visão matriz do modelo de conexão); acoplamento por
constelação (posições + `couplingFromDistance()`); mais módulos
(MEMORY/Clouds a partir da cicatriz, MATTER/Rings, SPACE/multitap,
Turing Machine); front-ends (JUCE + web/WASM a partir do core
framework-free); contrato do Ensemble Bus (G4: dois instrumentos de
naturezas diferentes). Ver `PESQUISA_MODULOS.md §2` e §7 (Top 100).

## Registro da etapa — 2026-09-01: licença do projeto

**Decisão do autor:** o Rasgo Modular usa **GNU AGPLv3 ou posterior** —
"a mesma que temos usado", isto é, a licença habitual da família RASGO
(MARAVI, ANTITOTEM, AQUORBIUM usam AGPLv3; RASGO Synth usa GPLv3 por
decisão específica do Studio). Perguntado entre AGPLv3 e GPLv3, o autor
confirmou **AGPLv3-or-later**.

**Feito:** `RASGO_MODULAR/LICENSE` (texto FSF da AGPL-3.0, cópia do de
MARAVI); `README.md` e `RASGO_MODULAR.md §29` atualizados com a decisão e
as consequências (código de terceiros só sob licença compatível com
AGPLv3; **GPLv3-or-later é compatível**; o alvo web/WASM ativa a cláusula
de rede — instância servida deve oferecer a fonte; front-end JUCE usa a
via AGPLv3 do JUCE). Auditoria do VCV Rack no `ATLAS §4.2` e
`PESQUISA_MODULOS.md §8` revista: código do Rack/Fundamental
(GPLv3-or-later) agora marcado **compatível** com a nossa AGPLv3 (cópia
possível como decisão consciente por arquivo); arte CC-NC/ND continua
incompatível. `INVENTARIO_GLOBAL_MODULOS.md`: 10 linhas do Rasgo Modular
agora registram `AGPL-3.0-or-later`.

**Sem cabeçalho SPDX nos fontes** — não é a convenção da família RASGO
(nenhum projeto usa; o arquivo `LICENSE` + nota no README/doc basta).

**Não commitado.**

## Registro da etapa — 2026-09-02: Módulos 7-8 + matriz + constelação (marco 2)

**Módulo 7 — `MEMORY`** (`dossies/07_memory.md`). Buffer circular de 3 s +
`freeze` + pool de 16 grãos Hann. `grain`/`density`/`position`/`spray`/
`pitch`(±24 st)/`feedback`(≤0,95)/`blend`/`freeze`. A cicatriz do `Cable`
virou módulo: reter é ter de onde tocar. Buffer alocado em `prepare()`,
`process()` sem alocação, determinístico (xorshift semeado).
`src/dsp/Memory.hpp`, `tests/test_memory.cpp` (8/8).

**Módulo 8 — `TURING`** (`dossies/08_turing.md`). Registrador de
deslocamento de `length` estágios; a cada clock o valor que reentra é o
que saiu (`lock` alto → laço travado, período = `length`) ou novo/mutado
(`lock` baixo → acaso). Saídas `cv` (quantizável por `steps`), `cv2`
(soma ponderada = expansor Volts), `pulse` (bit da frente = Pulses).
Clock externo ou interno (`rate`). `src/dsp/TuringLoop.hpp` (classe
`TuringLoop`, `type()`=`"TURING"`), `tests/test_turing.cpp` (8/8).

**Modelo de conexão — matriz + constelação** (fecha os pendentes do
"Registro da etapa — modelo de conexão"):
- **Matriz:** `SignalGraph::matrixSources()` (linhas = todas as saídas),
  `matrixSlots()` (colunas = toda entrada + todo parâmetro modulável),
  `matrixCell(src, slot)` (vazio / `Cable` / `ParameterLink`),
  `matrixToText()` (superfície de edição em terminal: `.` `X` `~`relação
  `c`condução<1 `m`modulação). É uma VISÃO — não duplica estado.
- **Constelação:** `setNodePosition(id, x, y)` / `nodePosition()`;
  `couplingFromDistance(dist, radius)` = `exp(-(dist/radius)²)` (perto =
  1, longe → 0, `radius≤0` desativa); `applyConstellation(radius)` põe o
  ganho de constelação de cada cabo pela distância entre os nós de origem
  e destino; `clearConstellation()` volta ao neutro. Novo membro
  `Cable::constellationGain` multiplicado no caminho intacto junto do
  `dropGain`. Não muda a topologia — só o ganho.
- Testes em `tests/test_signal_graph.cpp` (`testConnectionMatrix`,
  `testConstellationCoupling`). Total do alvo: sobe pra 9 alvos CTest.

**`examples/peca_generativa_2.cpp`** → `validation-output/
peca_generativa_2.wav` (**50 s**, determinística — dois renders
byte-idênticos). Os 8 módulos: `CLOCK`→`TURING` (melodia cristalizável,
`lock` modulado por um `DECISION` lento) + `DECISION`→timbre; voz→filtro
por `Cable` RingMod+conductance; `FILTER`→`ENVELOPE`→`MEMORY`→saída;
`FREEZE-LFO`→`MEMORY.freeze_gate` (seções suspensas); constelação move
`MEMORY` no campo (raio largo → respiração global de ~30%). Ruptura aos
27 s. RMS por trecho de 5 s: 0,13 / 0,10 / 0,06 / 0,13 / 0,16 / 0,09 /
0,10 / 0,11 / 0,14 / 0,12.

**Validação:** 9/9 alvos CTest em Debug e Release (`-Werror`).

**Nota de patch (não bug):** na peça 2 toda a saída passa por `MEMORY`,
então a constelação com raio largo respira a mistura inteira, não só a
"camada de memória". Uma constelação que isole camadas pede um caminho
seco paralelo (nó `Sum`) — 2ª camada.

**Próximo passo:** MATTER (Rings/Elements — ressoador modal/corda),
SPACE (multitap/reverb), sequenciador editável, camada semântica de
conexão, quantizador a escala, front-ends. Ver `PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: Módulos 9-10 (MATTER + SPACE)

**Módulo 9 — `MATTER`** (`dossies/09_matter.md`). Ressoador modal: banco
de 24 ressoadores de 2 polos em `f_i = f0·razão_i`; `structure`
harmônico→esticado (corda→sino), `brightness` (rolloff), `damping`
(escala os T60), `position` (posição de excitação → quais modos recebem
energia), `exciter` (rajada de ruído interna na borda de `strike`).
`b0 = amp_alvo·sin(w)` normalizado pelo modo mais forte; `tanh` na saída.
Coeficientes por bloco (24 modos, barato). Determinístico.
`src/dsp/Matter.hpp`, `tests/test_matter.cpp` (8/8).

**Módulo 10 — `SPACE`** (`dossies/10_space.md`). Atraso multitap de
2,2 s: `taps` (1–8) em tempos por `spread`; realimentação com filtro de
tom no laço (`tone`, `feedback` ≤ 0,97, `tanh` na escrita); difusão por
4 all-pass primos **fora do laço** (mantém o andamento dos ecos),
misturada por `diffusion`; LFO ~0,13 Hz na leitura (`mod`, chorus).
Saídas `out` e `wet`. Determinístico (sem RNG). Buffers em `prepare()`.
`src/dsp/Space.hpp`, `tests/test_space.cpp` (8/8).

**Validação:** 11/11 alvos CTest em Debug e Release (`-Werror`).
Destaques: MATTER — ring + decay, periodicidade (autocorrelação) segue
`freq`, `damping` controla sustain (3×), `damping=0` estável, structure
muda timbre, byte-idêntico. SPACE — primeira tomada em `time·sr` (±40),
ecos em múltiplos decaindo, `mix=0` byte-a-byte igual à entrada,
`feedback=0,95` sem divergir em > 5 s, `tone` afeta o brilho da cauda,
byte-idêntico.

**Notas de implementação:** o all-pass com `g=0` NÃO é bypass (é atraso
puro — inerente ao all-pass), então `diffusion` é blend `(1-d)·wet +
d·allpass⁴`; `tone` estava invertido na 1ª versão (corrigido: 1 =
brilhante). MATTER: 1ª normalização (`b0` a ganho de pico unitário)
deixava o módulo quase inaudível — trocada por `b0 ∝ amp_alvo·sin(w)`
com makeup ×3.

**Próximo passo:** corda por guia-de-onda (Karplus-Strong / Elements) —
`structure` do `MATTER` cruzando de modal a corda; reverb por FDN como
modo do `SPACE`; sequenciador editável; camada semântica de conexão;
quantizador a escala; front-ends (JUCE + web/WASM). Ver
`PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: Módulos 11-12 (STRING + QUANTIZER)

**Módulo 11 — `STRING`** (`dossies/11_string.md`). Corda por guia-de-onda
(Karplus-Strong estendido): laço de atraso `sr/f0` afinado por all-pass
fracionário (Jaffe & Smith), filtro de perda 1-polo (`damping` = brilho),
`decay` = ganho de realimentação (0,86–0,999), `position` = pente na
rajada de excitação (posição de pinça), `in` = arco. **`tanh` no laço →
o arco leva a um ciclo-limite, não à divergência** (mesmo princípio do
`FILTER` auto-oscilante). A outra face do `MATTER` (modal × guia-de-onda).
`src/dsp/StringVoice.hpp` (classe `StringVoice`, `type()`=`"STRING"`),
`tests/test_string.cpp` (7/7).

**Módulo 12 — `QUANTIZER`** (`dossies/12_quantizer.md`). CV contínua →
alturas de escala; saída `pitch` em **oitavas** (1 V/oct, casa com
`rate_mod`). 12 escalas curadas do subconjunto pesquisado em
`RASGO_SYNTH/rasgo-synth-core/src/sequencer/Scales.hpp` (só as tabelas
de intervalos — fato musical; código não incluído entre projetos).
Histerese (deadband contra tremulação), sample-and-hold por `trigger`,
glide exponencial (theremin), `gate` na mudança de nota. É o elo
"acaso → música original". `src/dsp/Quantizer.hpp`,
`tests/test_quantizer.cpp` (7/7).

**Validação:** 13/13 alvos CTest em Debug e Release (`-Werror`).
STRING: altura por autocorrelação segue `freq` (110/220/330 ±6%),
`decay` controla sustain (4×), `damping` controla brilho, arco+`decay=1`
estável (sem NaN/crescer), byte-idêntico. QUANTIZER: rampa de CV →
semitons inteiros na escala e monotônica, `root` desloca, S&H trava
entre pulsos, `glide` suaviza, byte-idêntico.

**Notas de implementação:** STRING 1ª versão divergia com arco contínuo
(`fbGain` perto de 1 + injeção sem headroom) → `tanh` sempre no laço +
`in` escalado ×0,35 + `fbGain` teto 0,999. QUANTIZER: histerese usava
`held == 0` como "não inicializado" (0 é altura válida) → flag
`initialized_`.

**Próximo passo:** movimento harmônico (`HarmonicWanderer` — escala e
tônica se movendo ao longo da peça); decaimento dependente de frequência
na `STRING`; reverb por FDN como modo do `SPACE`; sequenciador editável;
camada semântica de conexão; front-ends (JUCE + web/WASM). Ver
`PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: Módulo 13 (PARAMETRIC)

**Contexto:** o autor pediu pra codar a partir da ficha técnica do
**VCV Parametra** (`vcvrack.com/Parametra`) e perguntou se seria preciso
ver o código no GitHub. Resposta: não — Parametra é **fechado** ($30,
sem fonte pública), e mesmo se tivesse seria VERMELHO na governança. Um
EQ paramétrico é DSP de domínio público (**RBJ Audio EQ Cookbook**). A
página deu a lista de recursos; o código foi escrito do zero das
fórmulas RBJ.

**Módulo 13 — `PARAMETRIC`** (`dossies/13_parametric.md`). 4 estágios de
biquad em série, 6 tipos por estágio (Off/LowCut/LowShelf/Peak/
HighShelf/HighCut), `freq`/`gain`(dB)/`q` por estágio, coeficientes RBJ
normalizados por `a0`, Direct Form II transposta. Alpha "Q" para
peak/cortes, alpha "slope S" para shelves (forma correta do Cookbook).
Saída: `output` (dB), `drive` (`tanh` normalizado), **soft-clip de
segurança transparente até ±1** (não colore a EQ), `mix`. Desvios Rasgo:
`sweep` (1 V/oct) desloca todas as bandas como grupo; `amount` escala
todos os ganhos. Coefs por bloco, determinístico.
`src/dsp/Parametric.hpp`, `tests/test_parametric.cpp` (10/10).

**Validação:** 14/14 alvos CTest em Debug e Release (`-Werror`). Todos
Off → saída byte-a-byte igual à entrada; Peak +12 dB @ 1 kHz → **+12,0
dB** medido e ~0 dB uma década fora; LowCut −12 dB @ 80 Hz; LowShelf
+6 dB @ 200 Hz → +5,8 dB @ 50 Hz; `sweep` +1 oit move o peak de 500→1000
Hz; estabilidade com 4 estágios extremos + `sweep` senoidal; byte-
idêntico.

**Nota de implementação:** o soft-clip 1ª versão saturava em ±0,8 →
engolia o ganho do EQ nos testes (senoide unitária com +12 dB = ~4,0).
Corrigido: transparente até ±1 (é segurança, não timbre — `drive` é a
saturação intencional). Shelf 1ª versão usava a alpha de peak → curva
com "dip"; trocada pela alpha de slope do Cookbook.

**Próximo passo:** inclinações de 24/48 dB no `PARAMETRIC` (cascata de
biquads); movimento harmônico (`HarmonicWanderer`); reverb por FDN no
`SPACE`; sequenciador editável; camada semântica de conexão; front-ends.
Ver `PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: barramento semântico + peça 3

**Barramento semântico** (fecha o modelo de conexão de 3 camadas —
"concordo as três", decisão do autor). Conectar por SIGNIFICADO, não por
porta:
- `SignalGraph::Quality` = {Energy, Brightness, Density, Tension, Motion};
- `contributeQuality(nó, porta, Quality, weight, bias)` — o nó contribui:
  valor = clamp(bias + weight·|média do bloco|); várias contribuições na
  mesma qualidade → média;
- `followQuality(Quality, nó-alvo, param, depth, offset)` — o parâmetro
  segue: `param = offset + depth·qualidade`;
- `qualityValue(Quality)` — lê a qualidade resolvida.
Resolvido do **bloco anterior** (`resolveSemanticBus()` no topo de
`process()`), como a constelação/companion — sem depender da ordem
topológica. Padrão `EnergyControlBus` do TRIOIO, generalizado.
`src/core/SignalGraph.hpp`, testes em `tests/test_signal_graph.cpp`
(`testSemanticBus`). Alvo do grafo: 14 → segue 14 alvos CTest (o teste
entrou no alvo existente).

**`examples/peca_generativa_3.cpp`** → `validation-output/
peca_generativa_3.wav` (**55 s**, determinística — byte-idêntica).
Usa os módulos físicos + escala + EQ: `CLOCK` dispara `TURING`
(déjà-vu 0,7 → riff), `QUANTIZER` (Eólio, tônica ré) quantiza pra
`STRING.freq_mod` = melodia de corda; `MATTER` (66 Hz, structure 0,15) =
baixo percussivo no acento; `STRING + MATTER` → `MIX` → `PARAMETRIC`
(LowCut + 2 peaks + shelf, `sweep` de LFO) → `SPACE` → saída. **Barramento
semântico:** `TURING.cv` → qualidade *Motion* → `SPACE.feedback` (mais
movimento = mais eco); `CLOCK.euclid` → *Energy* → `PARAMETRIC.gain3`
(mais energia rítmica = mais presença). Ruptura EQ→espaço aos 31 s.
RMS por trecho de 5 s: ~0,31–0,34 (dip a 0,14 na cicatriz).

**Validação:** 14/14 alvos CTest em Debug e Release (`-Werror`).

**Estado do marco 2:** 13 módulos DSP + fundação + **modelo de conexão
completo (matriz + constelação + semântico)** + 3 peças (40 s, 50 s,
55 s). O que faltava do "concordo as três" está feito.

**Próximo passo:** inclinações 24/48 dB no `PARAMETRIC`; movimento
harmônico (`HarmonicWanderer`); reverb FDN no `SPACE`; sequenciador
editável; serialização das 3 camadas de conexão no patch de texto;
front-ends (JUCE + web/WASM). Ver `PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: serialização das 3 camadas + Módulo 14 (HARMONY)

**Serialização das 3 camadas de conexão no patch de texto** (fecha um
pendente). `serialize()`/`deserialize()` agora fazem round-trip de:
- `pos <id> <x> <y>` — posições de nó (constelação; só as não-nulas);
- `qin <nó>:<porta> <quality> weight= bias=` — contribuições semânticas;
- `qout <quality> -> <nó>:<param> depth= offset=` — followers semânticos.
`qualityFromName()`. Teste em `testSemanticBus` (round-trip idêntico +
`nodePosition` restaurada). O patch continua legível ("cabo como
partitura", Atlas §23).

**Módulo 14 — `HARMONY`** (`dossies/14_harmony.md`). O que faz a peça
**mudar de tom**. A cada fronteira de seção (trigger `advance` ou clock
interno em `rate`), avança o centro tonal (`root` 0–11, `scale` índice)
por 1 de **6 técnicas reais** de movimento harmônico (`movement`):
Coltrane/Giant Steps (+4 st, ciclo 3), sub tritônica + ii-V-I (−7 ou
±1 st), mediante cromática/Jobim (±3/±4 st), intercâmbio modal (raiz
fixa, só o modo), jazz modal/Miles (quase estático + passo "So What"),
backdoor ii-V (+2 st). Só a **lógica de intervalos** de
`RASGO_SYNTH/HarmonicWanderer.hpp` (fato musical; código não incluído).
`hold` = P(não avançar); `change` pulsa na modulação real. Saídas
normalizadas pra `connectToParameter(HARMONY, 0/1, QUANTIZER, "root"/
"scale", 12/11)`. Determinístico. `src/dsp/Harmony.hpp`,
`tests/test_harmony.cpp` (10/10).

**Validação:** 15/15 alvos CTest em Debug e Release (`-Werror`).
HARMONY: Coltrane +4 st exato (4,8,0,…); Backdoor +2 st; intercâmbio
modal → raiz constante em 40 passos, ≥3 modos; jazz modal muda a raiz em
<40% dos passos; `change` pulsa; `reset` volta a `root_start`; relógio
interno avança; byte-idêntico; integração (`HARMONY→QUANTIZER` params,
pitch quantizado sempre semitom inteiro válido).

**Nota:** `rate` do `HARMONY` tem teto de 2 Hz (uma seção a cada 0,5 s já
é rápido) — o teste usava 4 Hz e batia no clamp.

**Estado do marco 2:** 14 módulos DSP + fundação + modelo de conexão
completo e **serializado** + 3 peças. O caminho "acaso → música original
com harmonia que anda" está fechado: `CLOCK → HARMONY → QUANTIZER →
STRING/MATTER`, `TURING` como fonte de contorno.

**Próximo passo:** inclinações 24/48 dB no `PARAMETRIC`; reverb FDN no
`SPACE`; decaimento dependente de frequência na `STRING`; sequenciador
editável; **front-ends JUCE + web/WASM**; contrato do Ensemble Bus. Ver
`PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: Módulo 15 (SEQUENCE)

**Módulo 15 — `SEQUENCE`** (`dossies/15_sequence.md`). O **sequenciador
editável** (pendente do "sequenciador como família de comportamentos",
Hexen §119). Padrão de **8 passos editável** — altura (`p1..p8`, bipolar)
e gate (`g1..g8`) como parâmetros; o edit surface é o patch de texto —
tocado por **5 modos de leitura**: forward, backward, pingpong, random
(xorshift semeado), brownian (passo a passo ±1, Grids). `length`,
`glide` (portamento entre alturas), `gate_len`, `range`. Saídas `pitch`
(oitavas), `gate`, `eos` (pulso por ciclo). Clock externo (janela de
gate estimada do intervalo entre clocks) ou interno em `rate`. `reset`
volta ao passo 0. O par **escrito** do `TURING` (que é o par
**emergente**). `src/dsp/StepSequencer.hpp` (classe `StepSequencer`,
`type()`=`"SEQUENCE"`), `tests/test_sequence.cpp` (9/9).

**Validação:** 16/16 alvos CTest em Debug e Release (`-Werror`).
forward/backward/pingpong produzem a ordem de índices exata; `gate` segue
`g1..g8`; `eos` pulsa 1×/ciclo (~20 em 2 s a 30 Hz / 3 passos); `glide`
limita o salto de altura por amostra; random byte-idêntico; integração
(`CLOCK → SEQUENCE → voz`).

**Notas de teste:** com blocos de 1 frame (usados no `stepN`), o pulso
de `eos`/`gate` de 3 ms atravessa vários "passos" gravados → o teste de
`eos` passou pra blocos de 64 + clock interno; o teste de `glide`
semeava `prev` no primeiro sample e media o degrau entre blocos → mede
só a partir do 2º bloco.

**Estado do marco 2:** 15 módulos DSP + fundação + modelo de conexão
completo e serializado + 3 peças. `TURING` (emergente) + `SEQUENCE`
(escrito) cobrem a família SEQUENCE.

**Próximo passo:** inclinações 24/48 dB no `PARAMETRIC`; reverb FDN no
`SPACE`; decaimento dependente de frequência na `STRING`; ratchet/
probabilidade por passo no `SEQUENCE`; voicing vertical
(`MelodyVoicingBank`); **front-ends JUCE + web/WASM**; contrato do
Ensemble Bus. Ver `PESQUISA_MODULOS.md §2`.

**Não commitado.**

## Registro da etapa — 2026-09-02: PARAMETRIC — inclinações 24/48 dB/oct

Pendência fechada: os estágios de corte (LowCut/HighCut) do `PARAMETRIC`
ganharam `slope<s>` (1/2/3 → 12/24/48 dB/oct), implementado como **cascata
de 1/2/4 biquads idênticos** por estágio (`Stage::sections`, `z1[4]`/
`z2[4]`). Estado reset ao trocar a inclinação. 23 parâmetros.
`tests/test_parametric.cpp::testCutSlopes` (slope 48 atenua > 20 dB a
mais que slope 12 uma oitava abaixo do corte; passa transparente acima).
16/16 alvos CTest em Debug e Release. `dossies/13_parametric.md`
atualizado.

**Nota — painel gráfico:** ainda **não existe** GUI/painel gráfico
interativo. O que há: `Panel`/`Widget` (descrição declarativa neutra de
framework, um `panel()` por módulo) + `io/AsciiPanel.hpp` (renderizador
de TEXTO no terminal, pra teste). O instrumento roda **headless** — as
peças em `examples/` renderizam `.wav`. Um painel gráfico interativo +
áudio em tempo real + superfície de patch é o próximo marco grande:
front-ends JUCE (desktop/iOS/Android) e web/WASM, a partir do core
framework-free. Nada disso foi escrito ainda.

**Não commitado.**

## Registro da etapa — 2026-09-02: primeiro front-end — painel gráfico de teste

O autor pediu pra começar a testar em painel gráfico. Toolchain
disponível na máquina: **X11** (`DISPLAY=:0`, libX11) + **ALSA**
(libasound, `pw-play`/`pactl`). Sem emscripten, sem SDL/JUCE. Escolha:
um **painel nativo mínimo X11 + ALSA** — buildável e rodável agora, sem
instalar nada.

**Feito:** `apps/panel/`
- `AlsaSink.hpp` — saída ALSA bloqueante S16_LE (não é do core);
- `panel_main.cpp` (~500 linhas) — janela X11 que:
  - renderiza os painéis dos módulos **a partir da `Panel` declarativa**
    (knob = arco + ponteiro no ângulo do valor; slider; toggle; jack;
    label; display) — nada hard-coded por módulo;
  - roda o `SignalGraph` num **thread de áudio** (RT-safe, como projetado)
    e toca por ALSA em 48 kHz;
  - arrastar knob/slider (vertical) → `setParameter` ao vivo; clicar
    toggle → liga/desliga; `[espaço]` rompe/reconecta o cabo de saída;
    `[r]` reprepara; `[q]`/Esc sai;
  - mutex protege prepare/rupture vs. o áudio; pokes de parâmetro são
    corrida benigna de float (aceitável num app de teste).
- `CMakeLists.txt` — alvo `rasgo_modular_panel`, só compila se
  `find_library(X11)` e `find_library(asound)` acharem; `Threads`.
  **O `rasgo_modular_core` continua INTERFACE sem dependência.**

Patch atual (autônomo, soa ao abrir): `CLOCK` (96 BPM, E(7,16), drift) →
`ENVELOPE.gate`; `FUNCTION` (voz) → `FILTER` (três irmãs) → `ENVELOPE`
(AD) → saída; `env.env` → `filter.cutoff` (feedback de 1 bloco).

**Validação:** compila sem warning; `ldd` mostra libX11 + libasound
linkados; smoke test de 1 s sem crash. 16/16 alvos CTest seguem verdes
(o core não mudou). **Não rodei a GUI de forma prolongada** (janela +
áudio na máquina do autor — ele testa).

**Como rodar:** `cmake --build build --target rasgo_modular_panel &&
./build/rasgo_modular_panel`

**Próximo (painel):** carregar um `.rmp` (usar o `deserialize` + uma
factory dos 15 tipos) em vez do patch fixo; mais nós no patch; layout
com rolagem; depois o marco grande — JUCE e/ou web/WASM.

**Não commitado.**

## Registro da etapa — 2026-09-02: MIXER + MASTER — barramento de saída

O autor pediu "controles de saída de som: mixer, saída estéreo, master
etc". Dois módulos novos, fechando o fim de todo patch.

**Feito:**
- **`MIXER`** (`src/dsp/Mixer.hpp`, `type()`=`"MIXER"`) — 4 entradas mono
  (`ch1..ch4`); por canal `gain<c>` (−60..+12 dB, `dbToGain` zera abaixo
  de −60), `pan<c>` (−1..1, **potência constante** `t=(pan+1)·π/4`,
  L=`cos t`, R=`sin t`), `mute<c>`; `out_gain` global. Saída **estéreo**
  (grafo mono → (L+R)/2). Sem estado, sem alocação. Painel 16 HP (4 tiras
  verticais). `dossies/16_mixer.md`.
- **`MASTER`** (`src/dsp/Master.hpp`, `type()`=`"MASTER"`) — o último nó.
  Entrada `in` (estéreo), saídas `out` (estéreo) + `level` (Control, VU).
  Largura **mid/side** (`width` 0–2: 0=mono, 1=normal, 2=largo), soma
  `mono`, **bloqueio de DC** (passa-alta 1 polo ~5 Hz por canal,
  `y=x−x₁+R·y₁`, ligado por padrão), **limitador suave** de segurança
  (`tanh`, transparente até ±0,9, ligado por padrão), `gain` (−60..+12
  dB), `level` = pico com decaimento 300 ms. `dossies/17_master.md`.

**Validação:** `tests/test_mix.cpp` — 9/9 checagens (MIXER: soma, pan de
potência constante, mute, out_gain, faixa de dB; MASTER: width 0/1/2,
mono, gain, dc_block com e sem offset, limitador, level, determinismo,
integração no grafo duas vozes → MIXER → MASTER com L≠R). **17/17 alvos
CTest** em Debug e Release (`-Werror`). `test_mix` cobre os dois módulos.

**Cadeia de saída canônica:** `… → MIXER → MASTER → saída de áudio`. No
painel de teste, `MASTER.level` alimentaria um VU no topo.

**Não commitado.**

## Registro da etapa — 2026-09-02: painel — case Eurorack, catálogo, ideias de design

O autor pediu (a) ler as bases de layout em `RASGO_DOCUMENTATION/design/`,
(b) resolução/abertura na tela principal, (c) altura padrão de módulo como
Eurorack, (d) melhor aproveitamento da tela (quantas linhas de módulos),
(e) coluna à esquerda com o catálogo de módulos por família, arrastável
pra a case, (f) o Rasgo Modular podendo **sugerir um patch de partida**
(como o seed do RASGO Synth), (g) "anote essas ideias, documente".

**Bases lidas** (`RASGO_DOCUMENTATION/design/`): `README.md`,
`INTERFACES_E_LAYOUTS.md` (Eurorack como gramática §3.1, contrato
responsivo §5.1), `IDENTIDADE_VISUAL.md` (tokens por função §4,
tipografia §3), `AUDITORIA_MIGRACAO_RESPONSIVA.md`,
`PRODUTO_HARDWARE_E_ELETRONICA.md`. Siblings: `AQUORBIUM/
WINDOW_LAYOUT_DESIGN.md`, `ANTITOTEM/docs/DESIGN.md`.

**Feito no painel** (`apps/panel/`):
- **case Eurorack que quebra em linhas** — `kModH` altura padrão comum
  (só a largura varia por `Panel::hp`); os módulos fluem esquerda→direita
  e **quebram em nova linha** quando não cabem; rola na vertical;
- **coluna de catálogo à esquerda** (`kPaletteW=158`) — módulos
  agrupados por família (`ModuleCatalog.hpp` → 9 grupos SOURCE/TIME/
  DECISION/SEQUENCE/TRANSFORM/MATTER/MEMORY/SPACE/MIX); **arrastar um
  item pra a case cria o nó** (`makeModule` + `graph.prepare`);
- **`[s]` sugere um módulo** (rotação determinística — marcador da
  feature de sugestão de patch por seed, ainda não completa);
- **janela na tela primária** — `WindowPolicy.hpp` (política pura
  testável: `firstOpen` 88% da área do monitor primário via
  `XRRGetMonitors`, centrada; `revalidate` clampa se o monitor sumir);
- **sem sobreposição acidental** — cada widget tem `footprint()`,
  checagem O(n²) por módulo avisa no stderr no arranque (0 hoje);
- **UTF-8 explícito** — `setlocale` + `XFontSet` + `Xutf8DrawString`,
  fallback pra fonte de núcleo, nunca conversão implícita.

**Números (1080p a 88%, `apps/panel/design.md §3`):** altura de módulo
~260 px (34 unidades × 7 px + pad); ~3–4 módulos por linha; ~3 linhas
visíveis; patch típico de 5–9 módulos cabe em 2 linhas sem rolagem.

**Documentado:**
- `apps/panel/design.md` **reescrito** (11 seções: gesto central, case/
  catálogo/sugestão/instâncias, números específicos do Modular, contrato
  responsivo com débito assumido, tokens, tipografia, no-overlap,
  autonomia/acoplamento, produto físico, roadmap);
- `RASGO_MODULAR.md §36.7` expandido (case/catálogo/sugestão/UTF-8/
  overlap/responsivo) + "Regra de contribuição de painel";
- `src/core/Panel.hpp` — bloco "CONTRATO DE PAINEL" (altura padrão, sem
  sobreposição acidental, performance junto de I/O).

**Regra de layout registrada** (autor, 2026-09-02): sobreposição de
elementos **só quando o conceito, a mecânica e a definição dos objetos
caminharem pra isso** — nunca por descuido. Os números de linhas/altura/
largura são **específicos do Rasgo Modular** (ficam em `apps/panel/
design.md` e `RASGO_MODULAR.md`, não nas bases comuns de `design/`).

**Pendente:** camada de patch (cabos desenhados, matriz/constelação como
views); sugestão de patch por seed completa; persistência de janela;
carregar/salvar `.rmp` pela UI; instâncias e módulos compostos; DPI
100/125/150/200%; front-ends de produção JUCE + web/WASM.

**Não commitado.**

## Registro da etapa — 2026-09-02: painel — "Eurorack proporcional" (mm reais)

Pesquisa: origem do padrão Eurorack (Doepfer, 1995–96; grade de raiz
imperial — 1 HP = 0,2 pol = 5,08 mm, 1 U = 1,75 pol; placa Eurocard
métrica; painel 3U = 128,5 mm). Fontes e tabela Doepfer completa em
`apps/panel/design.md §3.1`. Decisão do autor aprovada: **proporção real
do Eurorack, escalada por um fator único `s` (px/mm)** pra caber na tela.

**Feito:**
- **`src/core/Panel.hpp`** — coordenadas do `Widget` agora em
  **milímetros** a partir do canto sup-esq; altura implícita 128,5 mm
  (3U); largura = `hp · 5,08 mm`. Contrato reescrito (grade interna em mm).
- **17 `panel()`** (os 16 módulos com painel) reescritos: layout em mm,
  empilhamento vertical (menos colunas), HP revisto pra valor realista
  (FUNCTION 12→8, FILTER 12→10, DECISION/MEMORY/MATTER/SPACE 14→12,
  QUANTIZER/HARMONY 12→10, MASTER 10→8, MIXER 16→14; CLOCK/PARAMETRIC/
  SEQUENCE mantidos).
- **`apps/panel/panel_main.cpp`** — renderizador: `s` derivado da altura
  (`(caseH/3 − pad)/128,5`, clamp 1,6–2,6 px/mm); módulo `128,5·s` de
  altura (constante) × `hp·5,08·s` de largura; case = largura de rack de
  **104 HP centrada**; pegadas (`footprintMM`) e checagem de sobreposição
  em mm; **auditoria de arranque cobre os 16 painéis do catálogo** (não
  só os exibidos).
- **`src/io/AsciiPanel.hpp`** — `renderAscii` comprime mm→char pra caber
  no terminal; `validatePanel` inalterado.

**Validação:** compila sem warning; **auditoria de pegada: 0
sobreposições** nos 16 painéis; **17/17 CTest** em Debug e Release
(`-Werror`); renders das peças **byte-idênticos** (o `panel()` não toca
o áudio). Smoke test da GUI: `timeout` saudável.

**Documentado:** `apps/panel/design.md` §2.1, §3, **§3.1** (pesquisa +
origem + fontes + tabela Doepfer), **§3.2** (modelo "Eurorack
proporcional" — fórmula do `s`, números, grade interna em mm).

**Pendente:** afinar visualmente com a janela aberta (tarefa do autor);
mostrar o valor do parâmetro no próprio painel ao arrastar (hoje só no
título); render ao vivo dos `Display` (forma/espectro/pattern).

**Não commitado.**

## Registro da etapa — 2026-09-02: OSC — oscilador subtrativo (Módulo 18)

O primeiro dos módulos essenciais que faltavam (`PESQUISA_MODULOS.md
§2.1`): a **voz "neutra"** pra `SEQUENCE → QUANTIZER → OSC → FILTER`.
Dossiê antes do código (`dossies/18_oscilador.md`).

**Feito:** `src/dsp/Oscillator.hpp` (`type()` = `"OSC"`, família SOURCE)
- **5 formas ao mesmo tempo** (saídas `sine`/`tri`/`saw`/`pulse`/`sub`);
- **antialias PolyBLEP** (Välimäki/Finke) no wrap da serra e nas duas
  transições do pulso e do sub; o triângulo (só quebra de 1ª derivada)
  fica sem BLEP — polyBLAMP é 2ª camada;
- **1 V/oct** — `f = freq · 2^(fine/1200) · 2^(pitch) · 2^(drift)`;
- **PWM** (`pw` + entrada `pwm`); **hard sync** (borda ↑ em `sync`
  reinicia as fases); **sub-oscilador** por flip-flop no wrap
  (`sub_2` → −1 ou −2 oitavas);
- **FM linear through-zero** — a entrada `fm` soma Hz, `dp` pode ficar
  negativo (a fase anda pra trás); preserva a afinação (a FM de fase
  não);
- **desvio Rasgo `drift`** — passo aleatório lento e correlacionado na
  afinação, xorshift semeado; `drift = 0` → determinístico.

`process()` sem alocação; sem RNG fora do `drift`.

**Validação:** `tests/test_oscillator.cpp` — **11/11** (afinação 1 V/oct
110/220/440 Hz + `pitch = +1` dobra; formas — seno ≈ senoidal, pulso
binível; PWM — razão cíclica segue `pw`; sub-oitava −1/−2; hard sync —
saída periódica na taxa do `sync`; TZFM — inerte em `fm_amount = 0`,
limitada com FM forte; antialias — serra a 9 kHz com menos energia de
banda baixa que a serra naïve; `drift` altera a saída mas fica < ½
semitom; determinismo byte-idêntico; grafo `OSC → FILTER`; painel
fecha). **18/18 CTest** em Debug e Release (`-Werror`). Auditoria do
painel: 0 sobreposições (painel de 12 HP incluído). Adicionado ao
catálogo (`apps/panel/ModuleCatalog.hpp`, família SOURCE).

**Pendente (candidatos, dossiê §Estado):** polyBLAMP no `tri`/`sub`;
oversampling 2× no caminho da TZFM; `spread`/super-saw como *relação
entre as saídas* (desvio Rasgo, dossiê próprio); wavefolding; formas por
wavetable morfável.

**Não commitado.**

## Registro da etapa — 2026-09-02: painel — flicker + texto fora da borda

O autor rodou a GUI e reportou: **"o gráfico tá flicando"** e "alguns
textos ultrapassam as margens e bordas dos módulos".

**Corrigido em `apps/panel/panel_main.cpp`:**
- **flicker:** (a) **buffer fora da tela** — o quadro inteiro é desenhado
  num `Pixmap` e copiado pra a janela numa passada só (`XCopyArea`);
  (b) **removido o redesenho periódico** de 40 ms — repinta só em
  resposta a `Expose`/`ConfigureNotify`/interação (o áudio já roda no
  seu próprio thread). `ConfigureNotify` só refaz o layout se o tamanho
  mudou de verdade; `Expose` só no último do lote (`count == 0`).
- **texto fora da borda:** cada módulo é desenhado com um **`clip
  rectangle`** = a caixa do módulo menos a borda; rótulo que não cabe é
  cortado na borda em vez de invadir o vizinho. Faixa de status e
  catálogo também recortados. Largura do `Display` limitada à caixa.
- fantasma do módulo arrastado agora entra no buffer (não mais desenhado
  por cima depois do flush).

Documentado em `apps/panel/design.md §3.3`. **18/18 CTest** seguem
verdes (o core não mudou); auditoria de painel 0 sobreposições; smoke
test `timeout` saudável.

**Pendente (painel):** camada de conexão (ver abaixo); encurtar rótulo
longo em vez de só cortar; render ao vivo dos `Display`.

**Não commitado.**

## Registro da etapa — 2026-09-02: cabeamento jack-a-jack no painel

**Decisão do autor (revertida):** o cabo jack-a-jack passa a ser a
superfície de conexão **primária** — "menos abstrato, viés pedagógico,
há uma cultura entre os músicos de cabear os módulos de formas variadas".
A matriz/constelação/semântica viram superfícies alternativas.
Documentado em `RASGO_MODULAR.md §36.2` e `apps/panel/design.md §2.5`.

**Motor** (`src/core/SignalGraph.hpp`):
- **`disconnect(node, port)`** — remove a conexão que chega numa porta de
  entrada; a porta volta a ficar livre (fan-in continua explícito).
  `tests/test_signal_graph.cpp::testDisconnectAndReconnect` (desliga,
  religa outra fonte, porta livre = silêncio). **18/18 CTest** Debug +
  Release.

**Painel** (`apps/panel/panel_main.cpp`):
- puxar um cabo de um jack a outro (`connect`/`disconnect` sob o mutex do
  áudio + `prepare`); solta em jack de polaridade oposta = conecta;
- **afordância pedida pelo autor:** ao segurar uma ponta, os destinos
  **válidos acendem** com halo — mesmo `PortKind` = destaque forte, tipo
  diferente = médio; polaridade errada **apaga** (quase invisível, sem
  rótulo);
- pegar a ponta de um cabo já ligado (clicar no jack de entrada) =
  desliga e re-ancora na saída de origem;
- **botão direito** num jack tira o(s) cabo(s);
- **ciclo → feedback automático** (`tryPatch`: tenta normal; em
  `std::logic_error` religa com `feedback=true`);
- entrada ocupada = o cabo novo substitui;
- `[espaço]` rompe/religa **todos** os cabos;
- cabo = bezier com barriga; rompido = tracejado + cor de aviso;
- o cabo `MASTER → sink` (sink não exibido) não é desenhado.

**Validação:** compila sem warning; 18/18 CTest Debug + Release;
auditoria de painel 0 sobreposições; smoke test `timeout` saudável (não
rodei a GUI de forma prolongada — teste do autor).

**Pendente:** atenuador/profundidade no cabo; modulação saída→parâmetro
pela UI; clicar no meio do cabo pra tirar; cor por tipo de sinal;
matriz/constelação como views.

**Não commitado.**

## Registro da etapa — 2026-09-02: excelência sonora — proteção de saída + xrun

O autor testou o painel e reportou **clipping**. Diagnóstico (probe
offline): o **grafo em si não clipa** (patch default: pico 0,66, RMS
0,13, 0 amostras estouradas). Duas causas:
1. o limitador do `MASTER` era um `tanh` **instantâneo** — distorce o
   transiente em vez de o segurar (quando o patch fica quente);
2. **xrun do ALSA** — período de 256 com só 4 períodos de buffer; o
   thread de desenho + re-prepare atrasam o áudio → estalo que soa como
   clip.

**Padrão de excelência da família verificado:** `NAVALHA2_JUCE/src/core/
OutputStage.{h,cpp}` + `LookaheadLimiter.{h,cpp}` + `TruePeakDetector` e
`ANTITOTEM/src/core/OutputStage.h` (código do autor, GPLv3/AGPLv3).
Princípio comum: *"saturação criativa é do patch; remoção de DC e
contenção de pico não são"*. Cadeia: guarda de finitude → bloqueio de DC
→ limitador com look-ahead (delay + seguidor de envelope, reduz o ganho
antes do pico) → teto suave de 1 amostra → teto −1 dBFS + telemetria.

**Feito:**
- **`src/dsp/OutputStage.hpp`** (novo, header-only zero-dep) — a cadeia
  acima, reescrita no idioma do Rasgo Modular. `process(l, r, doDc,
  doLimit)`, `gainReductionDb()`/`outputPeak()`/`nonFiniteCount()`.
  Desvio Rasgo: a telemetria de GR é pública (um módulo pode seguir a
  própria redução de ganho). Buffer de look-ahead alocado em `prepare()`.
- **`MASTER`** agora usa `OutputStage` no lugar do `tanh` + DC próprio.
  Latência de ~3 ms (o look-ahead). `dc_block`/`limit`/`gain`/`width`/
  `mono` inalterados como parâmetros.
- **Painel:** `AlsaSink` com **8 períodos** de buffer (era 4) e guarda o
  período REAL do ALSA; o thread de áudio processa em sub-blocos de
  ≤256 (limite do `AudioBlock`) por escrita. Folga contra xrun sem mexer
  no tamanho do bloco do grafo.

**Validação:** `tests/test_mix.cpp::testMasterExcellenceGuard` — seno a
+8 dB com transiente → **pico de saída ≤ 0,9, 0 amostra estoura o teto,
GR > 3 dB** (segura sem distorcer); NaN/Inf → saída finita. **18/18
CTest** Debug + Release; probe offline confirma pico 0,891 (= teto) com
12 dB de GR num sinal a +8 dB. Painel: smoke test `timeout` saudável.

**Pendente:** `TruePeakDetector` 4× polifásico (BS.1770 inter-amostra,
como na `NAVALHA`); medidor de GR no painel; RMS/LUFS; mover o
`graph.prepare` pra fora do caminho do áudio (hoje sob o mutex — um
clique no patch é tolerável, xrun contínuo não).

**Não commitado.**

## Registro da etapa — 2026-09-02: painel — mover/remover módulos, cores de cabo, knob de cutoff

Pedidos do autor (feedback rápido rodando a GUI):

- **cores de cabo variam** — "saco de cabos de patch": 4 tons quentes pra
  cabo de áudio, 4 frios pra controle (tipo da porta de origem); cor por
  hash determinístico das pontas (estável). `apps/panel/design.md §2.5`.
- **mover módulo sem desconectar** — arrastar o corpo do módulo reordena
  no fluxo da case; os cabos acompanham (reordena `shown`, refaz layout,
  cabos vêm das posições de jack). `apps/panel/design.md §2.6`.
- **remover fácil** — `[x]` no canto do módulo **ou** soltar na paleta.
  Só os cabos que tocam o módulo são desligados. v1: o nó fica órfão no
  grafo (`SignalGraph` não tem `removeNode` — renumeraria ids); anotado.
- **"o botão cutoff tá bugado"** → diagnóstico: `connectToParameter`
  **sobrescreve** o parâmetro a cada bloco (`param = offset + depth·env`)
  → o knob CUTOFF não fazia nada. **Corrigido no painel:** a modulação
  virou um **cabo de verdade** pra a entrada `cutoff_mod` (1 V/oct) do
  `FILTER` — o knob é a base, o envelope soma por cima, o cabo é visível
  e removível. `feedback=true` no cabo (filtro→env→filtro é laço).
- arrasto de knob de faixa larga tipo frequência (`hi/lo > 30`) agora é
  **exponencial** (oitavas por pixel).

**Não commitado.**

## Registro da etapa — 2026-09-02: NOISE — ruído e aleatório (Módulo 19)

Segundo dos essenciais que faltavam (`PESQUISA_MODULOS.md §2.1`): a
**fonte de acaso contínuo**. `DECISION` decide eventos; `NOISE` dá o
piso de ruído **e** as fontes de modulação aleatória. Dossiê antes do
código (`dossies/19_ruido.md`).

**Feito:** `src/dsp/Noise.hpp` (`type()` = `"NOISE"`, SOURCE/UTILITY)
- **`white`** (xorshift uniforme), **`pink`** (filtro de Paul Kellet
  "economy", 7 polos — domínio público, −3 dB/oit), **`brown`** (passeio
  com vazamento, −6 dB/oit);
- **`sh`** — sample-and-hold: segura até o pulso (de `trigger` externo ou
  relógio interno em `rate`); amostra a entrada `in` se conectada;
- **`smooth`** — tensão que passeia (Buchla 266): a cada pulso um alvo
  novo, glide até ele por `slew` (0 = degraus, 1 = deriva de segundos);
- **desvio Rasgo `spread`** — a distribuição do S&H/smooth vai de
  uniforme a sino (média de 4 uniformes), padrão `shape` do `DECISION`.
- 2 streams xorshift semeados em `prepare()` → determinístico. Sem
  alocação em `process()`.

**Validação:** `tests/test_noise.cpp` — **11/11** (branco: média ~0,
variância ~1/3, descorrelacionado, mais agudo que o rosa; rosa cai
~−3 dB/oit e brown mais forte; S&H muda ~1× por pulso e SEGURA entre
pulsos; S&H amostra `in`; smooth com `slew` alto varia devagar, com
`slew` 0 dá degraus; `spread`=1 concentra o S&H perto de 0; relógio
interno; determinismo byte-idêntico; grafo `CLOCK → NOISE.trigger`).
**19/19 CTest** Debug + Release; auditoria de painel 0 sobreposições.
Adicionado ao catálogo (família SOURCE).

**Pendente (dossiê §Estado):** ruído azul/violeta; `slew` assimétrico;
dois canais de S&H correlacionados (Marbles `X`); ruído de grão (dust).

**Faltam pra fechar o rack de partida:** `VCA`, `CONTROL`
(atenuversor/offset/slew), `LOGIC` (divisor/lógica de gates).

**Não commitado.**

## Nota de motor — `connectToParameter` aditivo (FEITO 2026-09-04)

**RESOLVIDO.** Ver "## Registro da etapa — 2026-09-04: motor —
`connectToParameter` aditivo" abaixo. O motor agora aplica
`base + offset + depth·fonte` (idem `followQuality`); `base` é o último
valor de knob e é atualizado por `SignalGraph::setParameterBase(node,
id, v)` — o painel roteia todo giro de knob / clique de toggle por aí.
`parameterUserValue()` devolve a base (não o valor instantâneo já
modulado) e é o que a serialização grava.

## Registro da etapa — 2026-09-02: VCA — amplificador duplo (Módulo 20)

Terceiro dos essenciais (`PESQUISA_MODULOS.md §2.1`). O `ENVELOPE`
embute um VCA, mas não dá pra pôr um no meio da cadeia de modulação,
fazer AM/tremolo/gate, ou somar CV. Dossiê antes do código
(`dossies/20_vca.md`).

**Feito:** `src/dsp/Vca.hpp` (`type()` = `"VCA"`, TRANSFORM/UTILITY)
- **duplo** (2 canais independentes); `in`×ganho por canal;
- **ganho = `level` (knob) + `cv_amount` · CV** — a `cv` é atenuvertida
  e **soma** ao knob. É modulação por **porta de verdade** → o knob fica
  vivo (ao contrário do `connectToParameter`);
- **`response`** (0 linear → 1 exp): `g^(1+3·resp)` — linear pra somar
  CV, exp ("dB-linear") pra volume;
- ganho suavizado (1 polo ~1,5 ms — sem zipper); **saturação suave**
  perto do teto (um VCA real tem som);
- **`sum`** = out1 + out2 (com teto) → mini-mixer de brinde;
- **desvio `drift`** — oscilação lenta correlacionada nos dois ganhos,
  xorshift semeado, ±~3 %. `drift = 0` → determinístico.
- `level1`/`level2` começam em **0** (VCA fecha por padrão, como um de
  verdade). Sem alocação em `process()`.

**Validação:** `tests/test_vca.cpp` — **11/11** (ganho `level` bate;
`level = 0` → silêncio; CV atenuvertida soma ao knob e inverte com
sinal; `response` alto encurva; suavização — degrau não estala;
`softSat` segura AM forte; `sum` = out1+out2; `drift` altera mas < 5 %;
determinismo; canais independentes; grafo `OSC → VCA` · `LFO → VCA.cv`).
**20/20 CTest** Debug + Release. Adicionado ao catálogo (TRANSFORM).

**Painel — rótulo de jack ACIMA do jack** (pedido do autor): o cabo sai
por baixo com barriga e escondia o rótulo desenhado embaixo. Ajustado o
`footprint` do jack (agora estende pra cima) e a densidade de PARAMETRIC
(pitch de estágio 17→16 mm, macros mais acima) + footprint de knob
17→15 mm. 0 sobreposições.

**Faltam pra fechar o rack de partida:** `CONTROL`
(atenuversor/offset/soma de CV + slew/lag), `LOGIC` (divisor de clock +
lógica booleana de gates).

**Não commitado.**

## Registro da etapa — 2026-09-02: painel — travamento + glitch + "sem som" + script de arranque

Reportado pelo autor: (1) *"travou somente o rasgo modular… uma vez tive
que finalizar no terminal"*, (2) *"modulos glitch"*, (3) *"iniciou sem
som"* / *"só clippado"*. Diagnóstico e correção — **bugs técnicos, sem
mudança de estética/mecânica**.

**"Sem som" — bug de locale na (de)serialização do patch (o principal).**
O painel chama `setlocale(LC_ALL, "")`; num sistema com vírgula decimal
(pt_BR do autor, fr_FR, de_DE…) o `std::stof` do `deserialize()` segue o
`LC_NUMERIC` do C e **trunca `"0.3"` em `0`**. Resultado: **todo parâmetro
fracionário caía pro mínimo a cada salvar/carregar** (envelope
attack/decay/release → 1 ms = clique; resonance/drift → 0; etc.). O
`session.rmp` (auto-carregado no arranque) degradava a cada ciclo → o
painel abria tocando cliques quase inaudíveis. Só apareceu agora porque
os smoke-tests criaram o primeiro `session.rmp` e o auto-load passou a
rodar.
- `src/core/SignalGraph.hpp` — `serialize()`/`deserialize()` agora
  `imbue(std::locale::classic())`; `std::stof`/`std::stoul` trocados por
  `parseNum`/`parseIndex` (istringstream com locale clássico). O texto do
  patch usa **sempre** ponto decimal, independente do locale do processo.
- `tests/test_signal_graph.cpp` — `testSerializationLocaleIndependent`:
  sob `fr_FR.UTF-8`, `gain=3.75` sobrevive ao round-trip (pula se o
  locale não estiver instalado). **21 verificações no alvo, 20/20 CTest.**

**"Rachado / clippando" — a saída ALSA produzia rápido demais.** Na
camada ALSA do PipeWire o `snd_pcm_writei` bloqueante **não segura o
thread** (aceita tudo e descarta) — medido **~3,7× tempo real** → xrun
contínuo, que soa como "tudo rachado". `snd_pcm_wait`/`avail_update` não
resolveu (e um buffer de 4 períodos piorou).
- `apps/panel/AlsaSink.hpp` — **ritmo por relógio de parede**: `write()`
  mede o tempo decorrido e, se está adiantado > 1 período, dorme o
  excedente. Produção fica **1,00×** qualquer que seja o comportamento do
  `writei`; o buffer de **8 períodos** só absorve jitter. `write()`
  devolve `bool`, escrita curta/recuperação em laço. Medido depois do
  fix: ratio 1,00 e **zero janelas com assinatura de xrun** na gravação
  do monitor.

**Travamento — starvation de lock.** O thread de áudio segurava o `gmx`
durante todo o `graph.process()` e só o soltava na escrita ALSA. Quando a
ALSA entrava em xrun não-recuperado, `snd_pcm_writei` voltava na hora →
o thread de áudio girava a 100 % re-adquirindo o `gmx` a cada micro-volta
→ o thread X11 (desenho/evento) nunca pegava o lock → janela morta (só o
RASGO Modular; resto do desktop ok — bate com "só o app").

**Travamento — starvation de lock.** O thread de áudio segurava o `gmx`
durante todo o `graph.process()` e só o soltava na escrita ALSA. Quando a
ALSA entrava em xrun não-recuperado, `snd_pcm_writei` voltava na hora →
o thread de áudio girava a 100 % re-adquirindo o `gmx` a cada micro-volta
→ o thread X11 (desenho/evento) nunca pegava o lock → janela morta (só o
RASGO Modular; resto do desktop ok — bate com "só o app").

Correções:
- `apps/panel/AlsaSink.hpp` — `write()` devolve `bool`; `false` = xrun
  não recuperado.
- `apps/panel/panel_main.cpp` (thread de áudio) — em falha de `write`,
  `sleep(5 ms)` **fora do lock** (fim do spin). E o bloco grande agora usa
  `try_to_lock`: se a UI está com o `gmx`, o áudio emite **um período de
  silêncio** e continua — a UI tem **prioridade**, nunca espera o áudio.
- `apps/panel/panel_main.cpp` (`redraw`) — snapshot dos osciloscópios com
  `try_lock`; a UI usa o quadro anterior se o lock está ocupado.

**Glitch — processava o rack inteiro.** `evaluationOrder()` visita
**todos** os nós; com "um de cada módulo no rack", os ~20 órfãos do
catálogo (reverb, modelos físicos, granular…) rodavam DSP a cada bloco
sem alimentar o `sink` → não fechava o tempo real → estalos.

- `src/core/SignalGraph.hpp` — **poda opcional**: `setActiveOutput(node)`
  restringe o `order_` aos nós que alcançam `node` (reverse-reachability,
  arestas de feedback incluídas). Sentinela `kEvaluateAll` (padrão) =
  comportamento inalterado. Membro `activeOutput_`.
- `apps/panel/panel_main.cpp` — chama `setActiveOutput(sink)` após o
  `prepare` inicial e após `loadPatch` (o `std::move` do grafo zera o
  alvo).
- **Renders de exemplo byte-idênticos** (`peca_generativa` 1/2/3 +
  `primeiro_fragmento`) — a poda não toca o caminho `kEvaluateAll`.
  **20/20 CTest** Debug + Release.

**`.run_rasgo_modular.sh`** (pedido do autor) — build **Release** (folga
de tempo real com o rack cheio, mesma nota do `ANTITOTEM`) + abre o
painel; `--tests` roda o ctest; `--clean` recompila do zero.

**`apps/panel/PatchSeed.hpp`** — `seedPatch(SignalGraph&, uint64_t)`:
xorshift semeado, limpa os cabos e monta CLOCK→SEQUENCE/TURING→QUANTIZER→
**OSC** (voz principal — contínua, SEMPRE soa pela envelope gatilhada)→
FILTER→ENVELOPE→[SPACE]→MIXER→MASTER→sink, + 2ª voz opcional
(MATTER/STRING no ch2), 1-2 modulações (só portas fora da cadeia), HARMONY
opcional. `E(k,16)` com `fill` ≥ 8 → nunca abre em silêncio.

**Ligado ao painel** (`avance nesta linha` → depois `ligue o botão`):
- botão **`⚄ SEED N`** na faixa de status + tecla **`[g]`** → próximo
  seed (número no botão e no título da janela);
- **`RASGO_SEED=N`** no ambiente / `./.run_rasgo_modular.sh --seed N` →
  abre já naquele seed (reproduzir / render por seed);
- verificado: 20/20 seeds produzem áudio (0 mudos); painel abrindo em
  `RASGO_SEED=3` → pico 0,89, som contínuo. Determinístico.
- Painel: 22/22 CTest, 0 sobreposições, 0 warnings.

### SH — sample & hold duplo (Módulo 23) — 2026-09-03

Primeiro candidato do §2.2 feito (`avance nos módulos novos`). Dossiê
`dossies/23_sample_hold.md` antes do código.

**Feito:** `src/dsp/SampleHold.hpp` (`type()` = `"SH"`, UTILITY) — 2
canais; segura `inN` ou o acaso interno no pulso de `trigN`/relógio
interno (`rate`); `trackN` (track & hold), `slewN` (glide Buchla 266),
`spread` (uniforme→sino), **`correlation`** −1..1 entre os acasos
internos (gêmeos↔espelho, Marbles `X`). 2 xorshift semeados,
determinístico, sem alocação. `tests/test_sample_hold.cpp` — **11
funções OK**. **23/23 CTest** Debug + Release, renders byte-idênticos,
painel 10 HP 0 sobreposições. Catálogo: TRANSFORM (junto de VCA/CONTROL).

### SHAPE — modelador de timbre (Módulo 24) — 2026-09-03

Segundo candidato do §2.2. Dossiê `dossies/24_shape.md`.

**Feito:** `src/dsp/Shape.hpp` (`type()` = `"SHAPE"`, TRANSFORM) — cadeia
ring-mod (`x·mod`, dry/wet) → wavefolder triangular fechado (`fold`,
drive 1→7×) + `symmetry` (bias DC = harmônicos pares, Buchla 259) →
`wrap` (dobra suave ↔ wrap-around seco) → `sat` (tanh progressivo,
transparente em 0) → VCA (`level`). Desvio `drift` no drive da dobra
(xorshift semeado; `drift=0` determinístico). Sem memória de amostra
além do drift. `tests/test_shape.cpp` — **11 funções OK** (ring suprime
a fundamental + bandas soma/diferença; fold → energia HF; symmetry →
2º harmônico; wrap → descontinuidades; sat → crista menor; level linear;
bypass ~transparente; drift < 6 %; determinismo; extremos finitos ≤ ±1,06;
grafo `OSC→SHAPE→FILTER`). **24/24 CTest** Debug + Release, renders
byte-idênticos, painel 10 HP 0 sobreposições. Catálogo: TRANSFORM.
**Pendência:** antialias das dobras (ADAA/oversampling).

---

### CHORD — VCO parafônico (Módulo 26) — 2026-09-03

Candidato do §2.2 (o `chord vco?` do autor). `src/dsp/Chord.hpp`,
`dossies/26_chord.md`, `tests/test_chord.cpp` (11/11).
2–4 vozes de uma base 1 V/oct; tabela de 10 formatos (`chord`/`chord_cv`,
casável com `HARMONY`), `inversion` (0–3), `detune` (coro), `wave`
(serra→pulso→tri com PolyBLEP), `drift` por voz. Soma `1/√vozes`.
**26/26 CTest** Debug + Release, renders byte-idênticos, painel 12 HP
0 sobreposições. Catálogo: SOURCE.

### seedPatch reescrito — "fator de escolha" do Studio — 2026-09-03

Pedido: *"precisamos de mais variedade"* / *"o seed pode ter um fator de
escolha similar ao do RASGO Synth Studio"* / *"studio apenas consulte"*.

Consultei (não editei) `RASGO_SYNTH/.../CompositionIdentity.hpp`,
`SeedLineage.hpp`, `GamePlan.hpp`, `MODELO_JOGO_SEED_CAMPO.md`. O modelo:
seed → POUCAS decisões perceptíveis (genes) + separação de streams +
alinhamento por probabilidade (não presets) + orçamento (não N rolls).

**`apps/panel/PatchSeed.hpp` reescrito:**
- `seedIdentity(seed)` → `SeedIdentity{ character, voice, rhythm, motion,
  space, risk, root, scale, bpm, mult, modBudget }`. `seedBits(seed,
  stream)` = SplitMix64 finalizer (`0x9E3779B97F4A7C15` etc), cada gene
  no seu stream.
- alinhamento: `switch(character)` faz nudges probabilísticos
  (Drone→voz Chordal/Subtr 70%, ritmo Slow 75%; Percussive→Physical/
  Noise; etc) — nunca hard preset.
- `seedPatch` constrói o grafo POR CARÁTER: Drone sustenta (VCA+LFO, sem
  gate); Percussive golpeia LPG (voz OSC/ruído) ou vai direto (ressoador);
  Textural → MEMORY granular; gated → ENVELOPE ASR/AD conforme o ritmo.
- **detector de ciclo** no `wire` (`reaches()`: BFS pelos cabos
  não-feedback; se `src→dst` fecharia loop, liga como feedback). Antes
  alguns seeds lançavam `logic_error` no `prepare`.
- voz física SEMPRE ganha um fio de arco (ruído no `in`) → não depende
  de densidade de golpe; ritmo esparso → ENVELOPE modo ASR + sustain
  alto (notas seguram).
- **Verificado (40 seeds):** 0 mudos, RMS 0,014–0,44 (ambiente↔driving),
  6 caráteres e 5 vozes bem distribuídos; bpm 27–144. Painel real
  `RASGO_SEED=5` → pico 0,74. Determinístico. **26/26 CTest**, painel
  0 sobreposições, exemplos byte-idênticos.

Memória: `reference_synth_studio_seed_model` (o modelo do Studio, para
consulta futura).

### LPG — low-pass gate a vactrol (Módulo 25) — 2026-09-03

Terceiro candidato do §2.2. Dossiê `dossies/25_lpg.md`.

**Feito:** `src/dsp/Lpg.hpp` (`type()` = `"LPG"`, TRANSFORM/UTILITY) — um
seguidor de envelope **não-linear assimétrico** (sobe ~2 ms; desce com
coef `∝ 0,15 + 0,85·env` → freia perto de 0 = a "cauda"/"memória" do
LDR) controla um filtro de 2 polos inline (2× 1-polo + realimentação
leve, sem `tan`/`exp` por amostra) **e** um VCA. `mode` 0..1: 0 = só
filtro, 1 = só VCA, 0.5 = os dois totalmente ativos (fecha de vez).
`response` (cauda ~30 ms–2,5 s), `offset` (abertura de repouso),
`resonance`, `strike` + `cv`. Desvio `drift`. `tests/test_lpg.cpp` —
**11 funções OK** (subida ≪ descida; `response` → cauda maior; mode 0 =
filtra sem gate / mode 1 = gate sem colorir; `offset` passa em repouso;
`cv` abre; ressonância estável; silêncio ocioso; determinismo; drift
pequeno; grafo `CLOCK→LPG.strike` · `OSC→LPG.in`). **25/25 CTest**
Debug + Release, renders byte-idênticos, painel 10 HP 0 sobreposições.
**Pendência:** o "bounce"/overshoot de vactrols reais.

---

## Registro da etapa — 2026-09-03: DRIFT (Módulo 27) + seedPatch v2 (gramática de portas)

Pedidos do autor (vários, iterando ao vivo): *"mais variedade"* /
*"não deve haver limitação de cabeamentos"* / *"chegou a calcular as
probabilidades e possibilidades de conexões?"* / *"o seed no rasgo_synth
studio... é na casa dos milhões ou bilhões"* / *"a escolha do número do
seed deve ser aleatória"* / *"o músico grava/registra o seed em seu banco
de patches"* / *"fluxos que se retroalimentam, derivas de variação (como
no antitotem)"*.

**Espaço de conexão calculado:** rack de 27 módulos = **80 entradas, 68
saídas, 211 parâmetros**. Cabeamento bruto ~10^147; patches realistas
**10^16 (6 cabos) a 10^44 (20 cabos)**. Seed `uint64` = **1,8·10^19**.

**`DRIFT` (Módulo 27)** — `src/dsp/Drift.hpp`, `dossies/27_drift.md`,
`tests/test_drift.cpp` (11/11). Campo de deriva: LFSR de 8 bits + passeio
com momentum (ANTITOTEM `CRI-DRF-001`) → 4 saídas correlacionadas (leituras
ponderadas distintas do mesmo registrador, AQUORBIUM `BiomaBrain`) + LFO
próprio; `stride`, `advance` (cadência por compasso), `event`. Escala de
minutos. **27/27 CTest** Debug + Release, painel 10 HP 0 sobreposições,
exemplos byte-idênticos. Memória: `reference_synth_studio_seed_model`,
`reference_locale_safe_text_formats` (padrão).

**`apps/panel/PatchSeed.hpp` v2 — GRAMÁTICA PROBABILÍSTICA DE PORTAS
(não mais arquétipos):**
- classifica TODA porta por função (fonte: voz/bus/lenta/random/gate/
  altura; destino: áudio/FM/mod/gate/altura);
- matriz `W[fonte][destino]` + bônus experimental escalado por `wildness`;
  o seed é um passeio aleatório ponderado por esse grafo, cabo a cabo —
  **qualquer conexão é possível, só mais/menos provável**;
- genes (SplitMix64): `complexity` (mínimo ~14 cabos ↔ teia ~45, com
  15 % minimalista / 10 % máximo), `wildness`, `motion`, energy, space,
  voiceBias, root/escala, bpm 46–156;
- garante ESPINHA voz→proc→MIXER.ch1→MASTER→sink; a voz é sacrossanta (o
  passeio não toca as entradas dela); ciclo→feedback; `connectToParameter`
  sempre com atraso de bloco (evita ciclo por link de param);
- **passeio ponderado**: `3 + complexity·25` cabos extras; vozes soltas
  → MIXER ch2/3/4; `DRIFT` plugado em 2–4 params estruturais → o patch
  EVOLUI sozinho; params de todo módulo periférico randomizados (faixa
  ampla, espinha intocada).
- **`[g]` / botão sorteiam um seed ALEATÓRIO** (não incrementa);
  `RASGO_SEED=N` / `--seed N` reproduzem.
- **Banco de patches**: `[Ctrl+B]` → `~/.local/share/rasgo-modular/
  patches/seed-N.rmp` (com a linha `seed N`).
- **Verificado (80 seeds):** 0 mudos, 0 erros, RMS 0,024–0,67, cabos
  14–45 (média 22), MIXER com 3+ canais em 75/80. Painel real (seeds
  3/17/42/71) toca contínuo. Determinístico.

`design.md §2.3` reescrito (a v1 arquétipos vira `§2.3.1` histórico).

**Não commitado.**

---

## Registro da etapa — 2026-09-03: SWITCH (Módulo 28)

Pedido do autor: *"continue"* (após o marco DRIFT + seed v2). Próximo
candidato do `PESQUISA_MODULOS.md §2.2` que o rack não tinha e que a
gramática de seed pode explorar: o **roteador controlado**.

**`SWITCH` (Módulo 28)** — `src/dsp/Switch.hpp`, `dossies/28_switch.md`,
`tests/test_switch.cpp` (11/11). Chave sequencial: 4 entradas
`a`/`b`/`c`/`d` → 1 saída `out` (mux N→1). O endereço avança no `clock`
(borda ↑), zera no `reset`, ou vem direto da CV `addr` (se conectada,
manda). `steps` 2–4; `mode` forward / pingpong / random (xorshift
semeado, anti-repetição de 1) / só-`addr`; `glide` = crossfade no ponto
de troca + slew de ~1 ms sempre (anti-clique a taxa de áudio); saída
`step` segue a posição. Sem alocação/lock/IO em `process()`,
determinístico. Catálogo do painel: família SEQUENCE (junto de
`TURING`/`SEQUENCE`), 10 HP, 0 sobreposições. Pendências: modo demux
(1→N), `hold`, 8 entradas (A-152), `spread` no `random`.

**28/28 CTest** Debug + Release. Renders de exemplo byte-idênticos
(módulo novo, não entra nas peças). Painel compila (`-Wall -Wextra`).

**Não commitado.**

---

## Registro da etapa — 2026-09-03: SCOPE (Módulo 29)

Pedido do autor: *"quais módulos a fazer?"* → listados os candidatos do
`§2.2`; *"isso, avance"* sobre o `SCOPE` (osciloscópio + analisador, o
primeiro da lista sugerida — ferramenta pra *ver* o que os seeds fazem).

**`SCOPE` (Módulo 29)** — `src/dsp/Scope.hpp`, `dossies/29_scope.md`,
`tests/test_scope.cpp` (14/14). Ferramenta de medição cujo **desvio
Rasgo** é: as medições saem como **CV patchável** (o instrumento escuta
a si mesmo). `in` → `thru` limpo (saída 0 = o que o `Display` desenha);
`trig` = comparador com histerese (`reject`) + borda (`edge`), fonte
`ext` ou `in`; `level` = seguidor de pico; `bright` = centroide
espectral **pelo diferenciador** (Parseval, sem FFT — barato,
determinístico); `pitch` (v/oct) = período entre cruzamentos de zero,
**trava só após 3 períodos consistentes** (ruído → 0); `hold` congela.
Sem RNG, `process()` sem alocação.

Colisão de nome resolvida: o `struct Scope` local do `panel_main.cpp`
(anel de amostras do Display) virou `struct ScopeTrace`.

Catálogo do painel: família MIX (junto de `MIXER`/`MASTER` — o fim da
cadeia; não há METER no catálogo). Painel 14 HP, `validatePanel` OK.

**29/29 CTest** Debug + Release, 0 warnings. Renders de exemplo
byte-idênticos (módulo novo). Gerador de seed usa o `SCOPE`
automaticamente (portas auto-classificadas).

**Pendências (painel):** espectro desenhado (barras FFT Hann lendo o
buffer da saída 0), modo XY/Lissajous.

**Não commitado.**

---

## Registro da etapa — 2026-09-03: TRIGSEQ (Módulo 30)

Pedido do autor: seguir a ordem sugerida — `TRIGSEQ` (percussão
multipista, a maior lacuna).

**`TRIGSEQ` (Módulo 30)** — `src/dsp/TrigSeq.hpp`, `dossies/30_trigseq.md`,
`tests/test_trigseq.cpp` (16/16). Grade de trigs: 4 linhas de gate on/off
(bumbo/caixa/chimbal/perc). **Gerador, não editor** (identidade RASGO
"soa ao carregar"): `map` (0–1) morfa 4 caracteres arquetípicos
(straight/broken/shuffle/sparse) interpolando o peso de cada passo;
`density1..4` = limiar sobre o peso (conceito Grids, tabelas
reescritas); `swing` (passos ímpares atrasam), `chaos` (fantasma/queda
por probabilidade, não flip), `ratchet` (rajada de 3), `fill` (entrada)
+ `fill_amt`, `drift` (passeio lento do groove). Saídas `t1..t4` +
`accent` (≥2 linhas coincidem) + `any` (OR). `length` recorta, `rate` =
relógio interno. Gate curto (≤25 ms, sempre < período/3 pras sub-hits do
ratchet aparecerem). Determinístico (xorshift semeado). `process()` sem
alocação.

Catálogo do painel: família SEQUENCE (com `TURING`/`SEQUENCE`/`SWITCH`),
16 HP, `validatePanel` OK.

**30/30 CTest** Debug + Release, 0 warnings. 4 renders de exemplo
byte-idênticos. Gerador de seed: 50 seeds (janela 10 s) → 0 mudos, 0
erros, RMS 0,09–0,72, `TRIGSEQ` cabeado em 38/50.

**Pendências:** overlay editável de toggles, 6–8 linhas, `prob` por
passo, saída de velocity.

**Não commitado.**

---

## Registro da etapa — 2026-09-04: limpeza de rótulos do painel

Pedido do autor (vários exemplos ao vivo): textos de widget se
sobrepondo — "em OSC o SUB2 sobrepõe a caixa do SYNC", "em ENVELOPE o
TRIG fica cortado na margem", "em SWITCH GLID/SLEW cruzam os rótulos B/C"
etc. Auditoria com modelo realista de largura de texto: **~98 colisões**
em quase todos os módulos.

**Causas:** (1) a fonte não escala com o zoom → no zoom mínimo um rótulo
de 6 letras ocupa mais que 2 knobs; (2) o rótulo do toggle era desenhado
**à direita** da caixa (invadia o vizinho); (3) rótulo do knob
left-aligned, esticava pra direita; (4) rótulos de 6 letras.

**`apps/panel/panel_main.cpp`:**
- fonte de LEGENDA menor (`fsCap`, 9 px) só pros rótulos de widget —
  não escala, mas 9 px em vez de 12 px já cabe;
- rótulo do knob/slider **centrado ACIMA** do widget (como os jacks —
  sugestão do autor) → não empurra pra a fileira de baixo;
- rótulo do toggle **centrado ABAIXO** da caixa (não mais à direita);
- rótulo do jack **centrado acima** (era acima-esquerda);
- `capText()` usa `Xutf8TextExtents` pra centrar de verdade;
- `XFreeFontSet(fsCap)` no encerramento.

**Módulos:** rótulos de knob/toggle ≤ 5 caracteres (regra do autor).
Encurtados: `CUTOFF`→`CUT`, `SPREAD`→`SPRD` (Filter/Noise/Space/Decision),
`STRUCT`→`STRC`, `BRIGHT`→`BRITE`, `EXCITE`→`EXCIT` (Matter/String),
`DEJAVU`→`DEJA`, `ACC A/B`→`ACC-A/B`, `SC LO/HI`→`S-LO/HI`. `CONTROL` e
`PARAMETRIC`: 1ª fileira de knobs desceu ~3 mm (o rótulo acima encostava
no Display).

**`tests/test_panel_layout.cpp`** (novo, 33º alvo): audita a pegada REAL
(widget + rótulo na posição do `panel_main`) de todos os 31 módulos no
zoom médio + a regra de ≤ 5 caracteres. Gate contra regressão.

**Verificado:** modelo no zoom médio → **0 colisões** (era ~98). Zoom
mínimo (60+ módulos na tela) ainda encosta em alguns rótulos de 5
letras em coluna de 14 mm — aceitável, o render recorta ao módulo.
**33/33 CTest** Debug + Release, 0 warnings. 4 renders byte-idênticos
(só `panel()`/render, não toca DSP).

**Não commitado.**

---

## Registro da etapa — 2026-09-04: WASP (Módulo 32)

Pedido do autor: seguir a fila — `WASP` (filtro áspero, o contraponto
sujo do `FILTER` limpo).

**`WASP` (Módulo 32)** — `src/dsp/Wasp.hpp`, `dossies/32_wasp.md`,
`tests/test_wasp.cpp` (13/13). 12 dB/oitava com GRÃO: núcleo **SVF TPT**
(2 polos, mesma base do `FILTER`) com (1) `oscPush` que leva o
amortecimento a negativo perto de `resonance`=1 → auto-oscila; (2)
ceifador `wsat` no laço muito mais duro (joelho `1−grit·0.8`, teto
ASSIMÉTRICO `1∓bias·0.42` → harmônicos pares); (3) **estágio de saída**
`wsat(y·(1+grit·2))` que ceifa DEPOIS do filtro (não re-filtrado → é o
que dá o buzz reedy, já que um SVF de 2 polos filtra os próprios
harmônicos e a auto-osc nua fica quase senoidal); (4) DC block + soft
limit. `mode` LP↔BP↔HP, `drive` (waveshaper com corte), corte a
`0.45·sr`, `drift`. Determinístico. `process()` sem alocação.

Verificação de fontes (feita no `PESQUISA §2.2`): a Doepfer não publica
esquema (service manual é cliente-only) e o VCV "Doepfer" é
proprietário — **não usados**. Base: circuito do EDP Wasp (análises
independentes de René Schmitz / DIY), teoria de inversor CMOS, SVF TPT
(já reescrito no `FILTER`).

Catálogo do painel: família TRANSFORM (com `FILTER`/`LPG`/`VCA`/…),
10 HP, `validatePanel` OK.

**32/32 CTest** Debug + Release, 0 warnings. 4 renders de exemplo
byte-idênticos. Gerador de seed: 50 seeds (janela 10 s) → 0 mudos, 0
erros, RMS 0,03–0,73, `WASP` cabeado em 14/50 (compete com o `FILTER`
pelo slot de processamento de áudio).

**Pendências:** oversampling 2×; modelo de inversor CMOS mais fiel
(auto-osc francamente palhetada); modo do `FILTER`; `fold` no laço.

**Auditoria de pegada do painel** (o `panel_main.cpp` reportou
`[sobreposicao] WASP: 'DRIFT' x 'FC'`): corrigido — `DRIFT` sobe pra
y=87, jacks descem pra y=113. Aproveitando, varredura de TODOS os
painéis com a mesma lógica `footprintMM` achou mais 2 fora do painel
(não pegos pelo audit, que só checa sobreposição): `ABACUS` (knobs
RNG/SLEW passavam de 71 mm — colunas movidas pra x 9/25/41/57) e
`DRIFT` (jack `EVT` passava de 50,8 mm — fileira de saída reespaçada
5→45). Resta `MEMORY: 'FREEZE'` (toggle largo fora do painel de 61 mm)
— pré-existente, módulo 7, não é sobreposição; deixado pro autor.

**Não commitado.**

---

## Registro da etapa — 2026-09-04: ABACUS (Módulo 31)

Pedido do autor: seguir a fila — `ABACUS` (aritmética binária de CV +
o retificador que ele pediu).

**`ABACUS` (Módulo 31)** — `src/dsp/Abacus.hpp`, `dossies/31_abacus.md`,
`tests/test_abacus.cpp` (15/15). A CV como número inteiro: `math`
(`a`⊕`b`: soma/sub/mul/**resto** na janela `range`), `quant` (fonte em
`steps` degraus + `slew`), `rect` (**retificador dedicado**: meia +/− ·
onda completa `|a|` · sinal). **Contador binário**: `clock` soma
`count_step` (pode ser negativo/2/0), `c = count mod modulus` → `p1`
(bit `bitA`), `p2` (`bitA` XOR `bitA+1`, sincopado), `carry` (overflow
→ ritmo, ideia Numeric Repetitor). **Sem `a` conectado → a fonte de
`math`/`quant` é a rampa do próprio contador** — o ABACUS sozinho toca
melodia (`quant`) + ritmo (`p1`/`p2`/`carry`). Determinístico (sem RNG,
contador exato). `process()` sem alocação (1 `exp`/bloco pro slew).

Catálogo do painel: família DECISION (com `DECISION`/`DRIFT`/
`QUANTIZER`/`HARMONY`), 14 HP, `validatePanel` OK.

**31/31 CTest** Debug + Release, 0 warnings. 4 renders de exemplo
byte-idênticos. Gerador de seed: 50 seeds (janela 10 s) → 0 mudos, 0
erros, RMS 0,06–0,63, `ABACUS` cabeado em 27/50.

**Pendências:** bit a bit real (`a AND b`) como `op` 4–7; `modulus`/
`steps` por CV; euclidiano do contador; saída do valor cru do contador.

**Não commitado.**

---

## Registro da etapa — 2026-09-04: MULT (Módulo 34)

Pedido do autor: *"avance o mult"*.

**`MULT` (Módulo 34)** — `src/dsp/Mult.hpp`, `dossies/34_mult.md`,
`tests/test_mult.cpp` (11/11). Múltiplo **processado** (um puro seria
inútil no grafo digital, onde o fan-out já é livre): 1 entrada → 4
saídas, cada uma com atenuversor (`scale` ±2, negativo = inverte) +
`offset` (±1) próprios — o mini-`CONTROL` por tomada que o `PESQUISA
§2.2` pedia. `slew` compartilhado (glide). Ocioso (sem `in`) → 4 fontes
de tensão manual (`out_k = offset_k`). Sem `drift` (utilidade de
precisão). `process()` sem alocação, sem RNG.

Catálogo do painel: família TRANSFORM (com `CONTROL`/`VCA` — utilidades
de CV). Painel 10 HP, `test_panel_layout` OK.

**35/35 CTest** Debug + Release, 0 warnings. 4 renders de exemplo
byte-idênticos. Gerador de seed: 50 seeds (janela 10 s) → 0 mudos, 0
erros, RMS 0,01–0,62, `MULT` cabeado em 23/50.

**Pendências:** modo dual (2→3+3), `slew` por tomada, saída de soma,
barras animadas no painel.

**Não commitado.**

---

## Registro da etapa — 2026-09-04: MATRIX (Módulo 33)

Pedido do autor: *"faça o matrix"*. Interpretado como **módulo
auto-contido** (a camada matriz do motor já existe, e "tudo é módulo").

**`MATRIX` (Módulo 33)** — `src/dsp/Matrix.hpp`, `dossies/33_matrix.md`,
`tests/test_matrix.cpp` (13/13). Matriz 4×4: cada cruzamento
fonte×destino é um ganho (atenuversor), `out_k = level·sat(Σ_j
in_j·g_jk)`. 16 células `g11..g44` (−1..1, **padrão identidade** =
passa-direto ao carregar); `norm` (0=soma crua, 1=nível constante por
coluna); `sat` (saturação suave opcional — matriz segura em laço);
`level`; `drift` (desvio lento senoidal dos ganhos, 1 seno/célula/bloco
— a matriz respira). Sem RNG, `process()` sem alocação. Painel 20 HP:
grade 4×4 de knobs (`11`..`44`), IN1-4 à esquerda, OUT1-4 embaixo,
macros LEVEL/NORM/SAT/DRIFT à direita.

Catálogo do painel: família MIX (é mixer N×M).

**34/34 CTest** Debug + Release, 0 warnings. `test_panel_layout` OK. 4
renders de exemplo byte-idênticos. Gerador de seed: 50 seeds (janela
10 s) → 0 mudos, 0 erros, RMS 0,03–0,79, `MATRIX` cabeado em 26/50 (as 4
entradas + 4 saídas fazem dele um hub natural).

**Pendências:** grade N×M configurável; célula ring-mod por coluna;
`slew` nos ganhos; grade clicável no painel (widget novo).

**Não commitado.**

---

**Candidatos ainda pendentes** (`PESQUISA_MODULOS.md §2.2`; ~~`SH`~~,
~~`SHAPE`~~, ~~`LPG`~~, ~~`CHORD`~~, ~~`SWITCH`~~, ~~`SCOPE`~~,
~~`TRIGSEQ`~~, ~~`ABACUS`~~, ~~`WASP`~~, ~~`MATRIX`~~, ~~`MULT`~~
feitos): voz de percussão dedicada, adaptadores `MIDI`/`CV`/`AUDIO-IN`.
Estudo à parte: `SAMPLER`/`TURNTABLE`/`TAPE`
(`ESTUDO_audio_sampling.md`).

**Estudo à parte:** `dossies/ESTUDO_audio_sampling.md` — sampler /
toca-discos / fita cassete + a decisão de biblioteca de leitura de áudio
(`dr_wav.h`, domínio público/MIT-0, header-only, preserva o zero-dep) +
o risco de determinismo (módulos de áudio-gravado ficam fora do
`seedPatch` e dos renders de exemplo).

**Não commitado.**

## Registro da etapa — 2026-09-04: motor — `connectToParameter` aditivo

Fecha a "Nota de motor" acima. Antes, cada bloco fazia
`node.setParameter(id, offset + depth·fonte)` (idem `followQuality`) e
**apagava o valor do knob**: ligar uma modulação num parâmetro zerava a
posição que o usuário tinha ajustado. Fisicamente errado num modular —
lá o knob é o piso e a modulação **soma**.

**Feito (`src/core/SignalGraph.hpp`):**
- campo `base` em `ParameterLink` e `QualityFollower`, capturado no
  `connectToParameter` / `followQuality` (lê o valor corrente do
  parâmetro no momento da ligação);
- `process()` aplica **`base + offset + depth·fonte`** (cabo) e
  **`base + offset + depth·qualidade`** (barramento semântico);
- **`SignalGraph::setParameterBase(node, id, v)`** — seta o parâmetro
  **e** atualiza `.base` de todo link/follower que aponta pra ele. É o
  caminho que o painel usa agora pra todo giro de knob e clique de
  toggle (`apps/panel/panel_main.cpp`);
- **`parameterUserValue(node, id)`** — devolve a base (o knob), não o
  valor instantâneo já modulado; `serialize()` grava isso, então o
  patch redondo mesmo depois de `process()` ter modulado.

**Migração dos exemplos** (pra manter os renders byte-idênticos): os
`connectToParameter` de `peca_generativa`, `_2` e `_3` passaram a somar
sobre o knob. Onde `offset_novo = offset_antigo − knob` dava um literal
`float` exato, usei o literal; onde não (rede de feedback do `_3`),
fixei o knob explícito (`space.feedback = 0.12`) com `offset 0`.
Resultado: **os 4 renders continuam byte-idênticos**.

**Validação:** `test_signal_graph` ganhou `testAdditiveParameterBase`
(knob sobrevive à modulação; base estável bloco a bloco;
`parameterUserValue`; round-trip textual após `process()`).
`testParameterModulation` e os 3 testes semânticos seguem passando (base
0 nesses casos). **35/35 CTest** Debug + Release, 0 warnings. Sonda de
seed (50 seeds, ~15 s): 0 mudos, 0 erros, RMS 0,015–0,61 — `seedPatch`
inalterado. Painel compila.

**Não commitado.**

## Registro da etapa — 2026-09-04: antialiasing — SHAPE / WASP / LPG (híbrido)

Segunda pendência da lista de upgrades ("B"). Os três módulos de
distorção aliasavam com os controles no talo. Solução **híbrida** (uma
técnica por ponto ideal, não um helper uniforme):

**Novo `src/dsp/Oversampler.hpp`** — helper header-only, 2×: upsample por
interpolação linear (a entrada de áudio já é ~limitada em banda) +
decimação por FIR meia-banda de 13 taps (Blackman sobre sinc, ~−45 dB de
rejeição). Fase linear, atraso de grupo ~2,5 amostras, determinístico
(FIR + aritmética, sem RNG, sem alocar). Prior art consultado:
`RASGO_SYNTH/.../DiodeShaper.hpp` faz um 2× "leve" com decimação por
MÉDIA — a meia-banda real dá ~40 dB a mais pelo mesmo custo de upsample.

**`SHAPE`** — dobra + wrap + `sat` rodam a **2× E com ADAA de 1ª ordem**
dentro do laço 2× (Parker/Esqueda/Välimäki 2016): antiderivada FECHADA
de `m(u)` — `∫folded = 8·w·|w| − 4·w`, `∫wrapped = 2·s²`, ambas contínuas
em todo `u`. O 2× sozinho deixa resíduo entre fs/2 e fs; o ADAA sozinho
não basta pra nota grave com `fold` no talo; juntos cobrem quase tudo.
Medido (vs. a matemática a 16×): erro-vs-ideal ~1,4 → ~0,55 nos casos
moderados; alias em bins não-harmônicos < 1 % da fundamental; uso limpo
(`fold`/`wrap` = 0) em **THD 0,02 %** — transparente.

**`WASP`** — o núcleo não-linear inteiro (wsat de entrada + SVF TPT com
wsat no laço + mistura de modo + wsat de saída) roda a **2×**; `g`/`a1`
com `sr·2`. DC blocker + `softLimit` no rate base. Alias em bins
não-harmônicos **0,03–0,08 % da fundamental** (−62 a −70 dB) mesmo com
`drive` 8 + `grit` 1.

**`LPG`** — o filtro é LINEAR, não aliasa; **não leva oversampling**. Só
um 1-polo de ~0,5 ms suavizando o `fc` alvo (contra zipper de modulação
rápida do corte).

**Desvio da proposta:** a tabela aprovada dizia "ADAA + 2× no ramo wrap"
pro SHAPE e "2× se sobrar" pro LPG. Na prática: SHAPE ganhou 2× em TODO
o núcleo (não só o wrap) — mais simples e melhor; LPG ficou só com a
suavização — oversamplar um filtro linear não faria nada.

**Validação:** `test_shape` e `test_wasp` ganharam `testAliasReduced`
(energia em bins não-harmônicos < 1–2 % da fundamental — gate contra
volta ao alias cru). Contagens: SHAPE 10→11, WASP 13→14 funções. **35/35
CTest** Debug + Release, 0 warnings. **4 renders byte-idênticos** (nenhum
exemplo usa SHAPE/WASP/LPG). Sonda de seed: 0 mudos, 0 erros, RMS
0,0150–0,6139 (idêntico ao de antes). Painel compila.

**Não commitado.**

## Registro da etapa — 2026-09-04: painel — displays por módulo (MATRIX/SCOPE/TRIGSEQ)

Terceira pendência da lista de upgrades ("C"). Tudo em
`apps/panel/panel_main.cpp`, **sem mexer em DSP nem em parâmetro** — o
renderizador já desenhava um osciloscópio nos `Display` que a struct
`Panel` não descreve; estendido pra três módulos:

- **`MATRIX`** — os 16 knobs `g<jk>` deixam de ser desenhados; no lugar,
  uma **grade 4×4 clicável** (linha = entrada, coluna = saída), cada
  célula com barra de ganho a partir do centro (quente = +, frio = −).
  Arrasto vertical numa célula → `setParameterBase(g<jk>)` (mesma via
  dos knobs, então a modulação aditiva continua valendo). Os widgets
  seguem no `panel()` (ASCII, contagem de testes).
- **`SCOPE`** — clicar no `Display` alterna **onda ↔ espectro**. Espectro
  = banco Goertzel de 24 bandas log (60 Hz–12 kHz) sobre o anel de
  amostras, no `redraw` (nunca no thread de áudio), escala dB. Estado
  por módulo (`scopeView`), só no painel.
- **`TRIGSEQ`** — o `Display` vira um piano-roll das **4 lanes de gate**
  (`t1`–`t4`) rolando. `ScopeTrace` ganha 4 buffers pré-alocados (2 KB/
  módulo, nunca aloca em RT); o thread de áudio preenche as saídas 0–3
  pra o TRIGSEQ. Read-only.

**Custo:** nenhuma FFT no áudio; ~5 k mul-add por SCOPE por quadro no
`redraw`. Sem novo `Widget::Kind`. **35/35 CTest** Debug + Release
(módulos intactos), 0 warnings; painel compila (`-Wall -Wextra`). Não
rodado aqui (painel gráfico — o usuário roda o `.run_rasgo_modular.sh`).

**Pendências:** overlay editável do TRIGSEQ (precisa de máscara de passos
no módulo — mecânica nova); modo XY/Lissajous do SCOPE (mono só dá
retrato de fase; stereo real precisa de infra que não existe).

**Não commitado.**

## Registro da etapa — 2026-09-04: profundidade por módulo (D) — ABACUS/SH/MULT/LPG/SWITCH

Quarta pendência da lista de upgrades ("D"). Cinco módulos ganharam
profundidade; todos com `param = default` = comportamento antigo
INALTERADO (nenhum é usado nos exemplos → **4 renders byte-idênticos**).

- **`ABACUS`** — `op` de 0–3 → **0–7**. `4-7` = bit a bit (Lunetta):
  `a`/`b` viram inteiros de 5 bits (janela ±`range` → 0..31), AND/OR/
  XOR/NAND, o resultado volta a ±`range`. `test_abacus::testBitwise`.
- **`SH`** — novo `slope` (−1..1): troca o tempo de SUBIDA pelo de
  DESCIDA do slew (`up = base·(1+slope)`, `dn = base·(1−slope)`).
  `slope = 0` → simétrico, idêntico. Painel 10→12 HP (knob SLOPE).
  `testAsymmetricSlope`.
- **`MULT`** — novo toggle `dual` + 2ª entrada `in2`: ligado, `in` →
  out1/out2 e `in2` → out3/out4 (Doepfer A-180-2). `testDual`. (o
  helper `run` do teste passou a montar 2 entradas.)
- **`LPG`** — novo `bounce` (0–1): overshoot do vactrol — senoide
  amortecida na janela ~32 ms pós-golpe somada ao env, deixa a
  condutância passar de 1 por um instante (teto 1,35). `bounce = 0` →
  idêntico. Painel: knob BNCE. `testBounce`.
- **`SWITCH`** — novo `dir` (0/1) + 3 saídas de áudio (`out_b`/`out_c`/
  `out_d`). `dir = 1` = **demux 1→N**: a entrada `a` vai pra uma das 4
  saídas conforme o passo, as outras deslizam pra 0. Painel 10→12 HP
  (toggle DEMUX, jacks OA/OB/OC/OD). `testDemux`. (helper `run` do teste
  já montava 5 saídas.)

**Validação:** cada módulo ganhou 1 teste (ABACUS 13→14, SH 11→12,
MULT 10→11, LPG 11→12, SWITCH 11→12 funções). **35/35 CTest** Debug +
Release, 0 warnings. `test_panel_layout` passa (SH e SWITCH em 12 HP,
sem sobreposição). Sonda de seed (50 seeds): 0 mudos, 0 erros, RMS
0,016–0,72 (`seedPatch` inalterado — as portas/params novos entram na
gramática sem código extra). Painel compila.

**Pendências D restantes:** `CHORD` voice-leading; `MATRIX` N×M /
ring-mod; `DRIFT` memória de topologia. *(feitos logo abaixo.)*

**Não commitado.**

## Registro da etapa — 2026-09-04: profundidade por módulo (D, parte 2) — CHORD/MATRIX/DRIFT

Fecha a lista D. Mesmo contrato: `param = default` = comportamento
antigo INALTERADO; nenhum é usado nos exemplos → **4 renders
byte-idênticos**.

- **`CHORD`** — novo `voicing` (0/1): `1` = **condução de vozes** — na
  troca de acorde cada voz vai pro tom do novo acorde MAIS PRÓXIMO do
  que ela toca (ajuste de oitava, atribuição gulosa O(nv²) só na troca)
  e DESLIZA até lá (~40 ms). `voicing = 0` = paralelo, idêntico.
  Estado `voiceSemi_[4]`/`voiceTarget_[4]`. Painel: knob VLEAD.
  `test_chord::testVoiceLeading`.
- **`MATRIX`** — novo `ring` (0–1): cruza cada coluna entre a soma
  linear e o **produto** das entradas ponderado pelo ganho (célula
  `g~0` = fator unitário/bypass, `g~±1` = ±entrada; dois ganhos em 1 =
  ring-mod de 4 quadrantes). `prod` clampado a ±4. `ring = 0` =
  idêntico. Painel: knob RING (macros passam a LEVEL/NORM/RING/SAT/
  DRIFT). `testRingMod`.
- **`DRIFT`** — novo `anchor` (0–1): **memória de topologia** — a cada
  8 tiques grava o estado (LFSR + campo) como marco; em cada tique, com
  prob ∝ `anchor²`, volta pro marco em vez de dar um passo novo → a
  deriva ORBITA paisagens em vez de vagar. `anchor = 0` → nunca grava
  nem volta, sequência de RNG idêntica (short-circuit no `&&`). Painel:
  knob ANCHR. `testAnchor` (difere de anchor=0, campo vaga menos,
  determinístico).

**Validação:** CHORD 11→12, MATRIX 12→13, DRIFT 11→12 funções de teste.
**35/35 CTest** Debug + Release, 0 warnings. `test_panel_layout` passa.
Sonda de seed (50 seeds): 0 mudos, 0 erros, RMS 0,014–0,72. Painel
compila.

**Lista D completa** (8/8): ABACUS bit a bit · SH slope · MULT dual ·
LPG bounce · SWITCH demux · CHORD voice-leading · MATRIX ring-mod ·
DRIFT anchor.

**Não commitado.**

## Registro da etapa — 2026-09-04: Motion Engine — protótipo de composição generativa

Depois de uma conversa longa do autor com o ChatGPT sobre Seed,
composição generativa, partitura e pedagogia (documentada em
`dossies/ESTUDO_seed_composicao_generativa.md`), o autor apontou o ponto
que doía de verdade: *"os patches mudam, porém as regulagens permanecem
praticamente as mesmas"* / *"não há muita variação (no sentido de
composição generativa)"*. Lendo o `PatchSeed.hpp` linha a linha
confirmou: a espinha do patch recebe receitas por caráter (varia dentro
de faixas estreitas), o resto do cabeamento sorteia uma vez no reseed, e
depois disso só resta o `drift` cego de cada módulo — nada orquestra a
peça a **ir a algum lugar** no tempo.

**Feito — o "menor passo testável" do estudo (§3.5):**
- **`apps/panel/MotionEngine.hpp`** (novo) — camada de composição FORA
  do motor (`rasgo_modular_core` continua sem saber o que é
  "composição"), como o `PatchSeed.hpp`. 3 comportamentos:
  `Walk` (deriva com alvo, rearmado por probabilidade — diferente do
  `drift` cego: tem destino), `Oscillate` (varre entre limites),
  `Attract` (persegue o valor de outro parâmetro, de outro módulo —
  "coreografia paramétrica"). `tick(graph, dt)` escreve por
  `setParameterBase()` — a mesma via que um giro de knob usa, então
  qualquer modulação por cabo já plugada no mesmo parâmetro
  (`connectToParameter`) continua somando por cima. **É o motor aditivo
  de hoje cedo (`RM-ENGINE-ADDITIVE-MOD`) que torna isto possível sem
  brigar consigo mesmo.** Cada `Binding` carrega sua própria seed
  xorshift → determinístico.
- **`examples/peca_generativa_4.cpp`** (42 s, novo) — 4 ligações:
  `WALK` em `filter.resonance`, `OSCILLATE` em `env.curve`, `ATTRACT`
  em `voice.slope` perseguindo `filter.resonance`, e `WALK` em
  `filter.spread` **por cima** da modulação já existente por cabo
  (`lfoSpread → filter.spread`) — prova viva da aditividade.
- **`tests/test_motion_engine.cpp`** (5 funções, novo) — faixa/
  visitação do `Walk`, varredura do `Oscillate`, convergência do
  `Attract`, **a aditividade testada numericamente** (base do Motion +
  modulação por cabo no mesmo parâmetro = soma exata, nenhuma apaga a
  outra), determinismo.

**Validação:** som real (RMS varia 0,10→0,20 ao longo da peça, medido em
janelas de 7 s — não é drone estático); determinístico (2 renders
byte-idênticos); **36/36 CTest** Debug + Release, 0 warnings; os 4
renders anteriores continuam byte-idênticos (nenhum usa o
`MotionEngine`); painel compila (não tocado). `peca_generativa_4.wav`
entrou em `validation-output/`.

**O que isto NÃO é:** não há `Form Engine`/seções/`tension` ainda; os 4
`Binding` são fixados à mão no C++ da peça, não gerados pelo `Seed`; não
está integrado ao painel interativo. Continua sendo prova de
arquitetura, como o estudo recomendava antes de generalizar — ver
`ESTUDO_seed_composicao_generativa.md §3.6` pro detalhe completo e os
próximos passos possíveis (generalizar via Seed, ou integrar ao painel).

**Não commitado.**

## Registro da etapa — 2026-09-04: Patch Genetics — MUTATE/EVOLVE/FREEZE (protótipo)

Segundo item da ordem sugerida em `ESTUDO_seed_composicao_generativa.md
§7` — a extensão do `PatchSeed.hpp` (que só sabe GERAR um patch do zero)
pra EDITAR um patch que já existe, preservando a topologia.

**Feito:**
- **`apps/panel/PatchGenetics.hpp`** (novo) — `mutatePatch(graph, seed,
  fraction, frozen)`: reaplica a mesma ideia do passo genérico de
  randomização do `seedPatch()` (§2.2 do estudo) — pra cada nó
  alcançado por cabo (exceto os em `frozen`), reamostra ~`fraction` dos
  parâmetros não-estruturais na faixa TOTAL, sem tocar em cabo nenhum.
  Escreve por `setParameterBase()` — aditivo, não apaga modulação por
  cabo já plugada no mesmo parâmetro (o mesmo motor de hoje cedo,
  reaproveitado pela terceira vez no dia: engine → MotionEngine →
  PatchGenetics). `evolvePatch(graph, seed, steps, fractionPerStep,
  frozen)` = várias `mutatePatch` pequenas em sequência. `FREEZE` não é
  estado guardado — é só o `unordered_set<size_t>` de nós que quem
  chama passa em `frozen`.
- **`tests/test_patch_genetics.cpp`** (6 funções, novo) — nó sem cabo
  nunca muda; nó cabeado muda dentro da faixa e poupa parâmetros
  estruturais (`bpm`/`mode`/…); `FREEZE` protege; `EVOLVE` muda algo em
  N passos; determinismo; **a aditividade testada numericamente** (base
  mutada + modulação por cabo no mesmo parâmetro = soma exata).
- **Sonda extra** (não é CTest, `$CLAUDE_JOB_DIR/tmp/mutprobe.cpp`): 30
  seeds reais do catálogo (34 módulos), 5 `MUTATE` em sequência
  (fração 0,3) em cima de cada patch já semeado.

**Limitação medida (honesta, não escondida):** **4/30 patches ficaram
mudos** na sonda — ao contrário do `seedPatch()`, `MUTATE` não tem
noção de "espinha" (ela só existe em tempo de `seedPatch`, quando a
topologia está sendo desenhada); uma sequência de mutações pode
derrubar um nível/mix perto de 0 por acaso. Documentado no cabeçalho do
`PatchGenetics.hpp` e no estudo (§4) como decisão de design, com
mitigação recomendada (`FREEZE` nos nós de saída, `fraction` pequena ao
vivo) — não implementada.

**Validação:** **37/37 CTest** Debug + Release, 0 warnings. Não muda
nenhum módulo/exemplo existente (função nova, isolada, em
`apps/panel/`) — os 5 renders seguem byte-idênticos.

**O que isto NÃO é:** não está integrado ao painel (sem botão
`MUTATE`/`EVOLVE`, sem UI de seleção pra `FREEZE`); `CROSS` continua de
fora (precisa de alinhamento entre dois grafos, não desenhado).

**Não commitado.**

## Registro da etapa — 2026-09-04: RASGO Score — SYSTEM SCORE (protótipo)

Terceiro item da ordem sugerida em `ESTUDO_seed_composicao_generativa.md
§7`. Só o nível `SYSTEM SCORE` da conversa de origem — conexões,
modulação por cabo e mudanças de parâmetro. `MUSICAL SCORE` (notas)
fica de fora: pediria que os módulos anunciassem eventos de disparo de
forma genérica, um contrato novo que não estava em jogo aqui.

**Feito:**
- **`apps/panel/ScoreRecorder.hpp`** (novo) — `RasgoEvent` com 3 tipos
  (`Connection`, `Modulation`, `ParameterChange`); `time` é **sempre**
  `amostra/sr`, o `ScoreRecorder` nunca lê relógio de parede — quem
  chama passa o tempo. `toText()` gera um formato de texto próprio (uma
  linha por evento), não MusicXML/MIDI.
- **`tests/test_score_recorder.cpp`** (4 funções, novo) — ordem
  preservada, campos no texto, determinismo do texto (mesmos eventos →
  mesmo texto byte a byte), `clear()`.
- **`examples/peca_generativa_4.cpp`** (integrado) — registra as 7
  conexões/modulação iniciais em `t=0`, e as mudanças de parâmetro do
  `MotionEngine` **acima de um limiar** (0,03 — senão seria um dump em
  taxa de controle, ~15 mil linhas por parâmetro, não um registro de
  eventos legível). Resultado: 68 eventos numa peça de 42 s, salvos em
  `peca_generativa_4.score.txt` ao lado do `.wav`.

**Validação:** o texto da partitura é **byte-idêntico entre renders**
(`validation-output/peca_generativa_4.score.txt`) — tão determinístico
quanto o áudio. **38/38 CTest** Debug + Release, 0 warnings. Os 5
renders de exemplo (incluindo o `.wav` da peça 4) seguem byte-idênticos
— gravar a partitura não muda uma amostra sequer do áudio. Painel
compila.

**O que isto NÃO é:** `MUSICAL SCORE` (notas), export MusicXML/MIDI,
gravação ao vivo no painel interativo (o gravador de WAV já existe lá,
`Ctrl+R`; um gravador de eventos seguiria o mesmo padrão mas não foi
plugado), e o caminho bidirecional (`SCORE → RASGO`) — nenhum dos
quatro implementado.

**Não commitado.**

## Registro da etapa — 2026-09-04: Learning Engine — hover-learn (protótipo)

Quarto e último item da ordem sugerida em
`ESTUDO_seed_composicao_generativa.md §7` — depois deste, os 4 itens da
conversa com o ChatGPT (Seed/composição/partitura/pedagogia) têm
protótipo. Precedente citado pelo autor: Antitotem e Navalha 2 (hover
sobre controle/jack → explicação).

**Feito:**
- **`apps/panel/LearnCatalog.hpp`** (novo) — `lookupLearn(moduleType,
  bind)` devolve `{quick, understand, explore}` ou `nullptr` (sem
  conteúdo — silencioso, não erro). Conteúdo real preenchido pra
  `FILTER` e `ENVELOPE` (2 de 34 módulos — prova o mecanismo, o resto é
  redação incremental).
- **`tests/test_learn_catalog.cpp`** (4 funções, novo).
- **`panel_main.cpp`** (integrado) — `[l]` liga/desliga o **modo
  Learn**, sinalizado na faixa de status; com o modo ligado, hover sobre
  um knob/jack desenha um tooltip com o texto `quick` (reusa o mesmo
  hit-test por `footprintPx` do clique). **Silencioso por padrão** — o
  princípio que o autor marcou como central no Antitotem/Navalha.

**Validação:** **39/39 CTest** Debug + Release, 0 warnings. Painel
compila (`-Wall -Wextra`). Os 5 renders de exemplo seguem
byte-idênticos (mudança só de desenho no painel, nenhum módulo/DSP
tocado). **Não testado visualmente** — o painel gráfico trava a máquina
do autor ao rodar por aqui; ele confirma rodando
`.run_rasgo_modular.sh`.

**O que isto NÃO é:** só `quick` é desenhado (`understand`/`explore`
ficam guardados, sem UI de 2º nível); sem `WHY?`/`WHAT IF?`
contextuais; conteúdo dos outros 32 módulos não escrito.

**Não commitado.**

## Registro da etapa — 2026-09-04: LearnCatalog — conteúdo do rack de partida

Continuação direta do registro acima — conteúdo pra mais 3 módulos:
`OSC`, `VCA`, `CLOCK` (junto com `FILTER`/`ENVELOPE`, fecha os 5
primeiros módulos que um usuário novo encontra no rack de partida).
Texto baseado nos dossiês já escritos (`18_oscilador.md`, `20_vca.md`,
`05_clock.md`), não inventado — cada linha `quick`/`understand`/
`explore` descreve o comportamento real daquele parâmetro.

`tests/test_learn_catalog.cpp` ganhou `expectAllDocumented()` (helper)
e a checagem cobre agora os 5 módulos (widgets reais tirados do
`panel()` de cada um). **39/39 CTest** Debug + Release, 0 warnings
(mesma contagem de alvos — conteúdo dentro do teste existente). 5
renders de exemplo seguem byte-idênticos. Painel compila; **não testado
visualmente** (mesma ressalva do registro anterior).

**Não commitado.**

## Registro da etapa — 2026-09-04: MASTER — volume padrão mais baixo

Correção direta pedida pelo usuário: "o rasgo modular inicia muit alto
(volume) deixe o master em 50% para não assustar". `50%` lido como
metade da amplitude linear = **−6 dB** (não 50% da faixa em dB, que
seria −24 dB — silencioso demais pra ser útil).

**Feito:**
- `src/dsp/Master.hpp` — default de `gain` mudou de `0,0` dB (unidade)
  pra **`−6,0` dB**;
- `apps/panel/PatchSeed.hpp` — a faixa aleatória de `MASTER.gain` num
  seed novo (quando o patch tem `MASTER`+`MIXER`) mudou de
  `rng(−1, 3)` pra **`rng(−8, −4)`** (mesmo spread de 4 dB, recentrado
  em −6 dB) — senão um seed novo continuava perto da unidade e a
  correção só valeria pra um `MASTER` isolado, não pro caminho normal
  de início;
- `~/.local/share/rasgo-modular/session.rmp` — a sessão salva do
  usuário (carregada automaticamente na próxima abertura, antes de
  qualquer seed) tinha `gain=1,53` — editado direto pra `gain=−6`, senão
  a mudança de código não valeria pro próximo lançamento (a sessão salva
  tem prioridade sobre o default e sobre `RASGO_SEED`).

`tests/test_mix.cpp` tinha 5 funções que assumiam o default antigo
(`0` dB) implicitamente pra isolar `width`/`mono`/`dc_block`/`limit` —
cada uma ganhou `m.setParameter("gain", 0.0f)` explícito no topo, pra
continuar testando só o que diz testar, não o valor do default.

**Validação:** probe de 50 seeds (todo o catálogo de módulos, 15 s cada)
— **0 mudos**, RMS de pico caiu pra 0,57 (fôlego bem maior antes de
bater no limitador). **39/39 CTest** Debug + Release. 5 renders de
exemplo byte-idênticos (os exemplos usam um `Out` local, não passam
pelo `Master.hpp`).

**Não commitado.**

## Registro da etapa — 2026-09-04: abertura sempre gera um seed novo

Resposta direta a: "sempre quando abro o instrumento o sequencer está com
a mesma configuração (posição) [...] precisamos criar algo mais
inteligente, que varie os timbres, os sons, as histórias, as composições,
as texturas". Causa: a sessão salva (`session.rmp`) carregava por padrão
em TODO lançamento — congelando exatamente o mesmo patch, contrariando a
identidade do instrumento (`project_rasgo_modular_identity`: soa sozinho,
diferente, desde o load — não é retomar um documento).

**Feito** (`apps/panel/panel_main.cpp`, `.run_rasgo_modular.sh`):
- lançamento simples agora sorteia um seed novo por padrão (mesmo
  caminho de `[g]`) — timbre, cabeamento e forma diferentes toda vez;
- `RASGO_SEED=N`/`--seed N` continuam reproduzindo um seed específico
  (inalterado);
- **novo**: `RASGO_RESUME=1`/`--resume` — único caminho que carrega a
  sessão salva de propósito, pra quem estava num patch feito à mão;
- `Ctrl+S` (salva sessão) e o banco (`Ctrl+B`) continuam funcionando
  como antes — só o carregamento AUTOMÁTICO no lançamento mudou.

**Validação:** painel compila; 41/41 CTest Debug + Release (a lógica de
lançamento não tem alvo de teste dedicado — é I/O de processo, testada
por leitura de código + smoke da auditoria de painel já existente).

## Registro da etapa — 2026-09-04: excelência de saída — true peak + dither TPDF

Investigação pedida pelo usuário ("ainda acho que a qualidade sonora está
bem a desejar [...] verifique a saída pra não clipar, o código do
antitotem e do navalha 2 trabalharam exaustivamente esses pontos"). Antes
de mexer em código: confirmei que a correção de volume do registro
anterior ("MASTER — volume padrão mais baixo") estava mesmo compilada e
ativa no binário que o usuário roda (`build/rasgo_modular_panel`, mtime
depois da edição; strings do binário contêm o texto novo de `[v]`) — não
era binário obsoleto. Segui pra auditoria de qualidade.

Lida `NAVALHA2_JUCE/docs/AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md` (achados
reais, medidos, com correção registrada — código do próprio autor,
GPLv3/AGPLv3, conceito reaproveitado, não copiado). Dois achados batiam
com o estado real do RASGO Modular:

**1. `src/dsp/TruePeak.hpp` (novo) — `TruePeakEstimator`.** Estudado de
`NAVALHA2_JUCE/TruePeakDetector.{h,cpp}` (FIR polifásico 4×,
Blackman-Harris, ganho DC unitário por fase) — reescrito header-only
zero-dep. Uma primeira versão tentou economizar (8 taps/fase, 32 no
total); sonda própria (senoide de amplitude conhecida, várias
frequências/fases) mediu erro de até 3,7% perto de Nyquist — PIOR que só
olhar o pico de amostra. Corrigido pra 16 taps/fase (64 no total, igual à
NAVALHA — o custo é desprezível, uma vez por amostra no estágio MASTER,
não por voz); erro medido caiu pra <4% em toda a faixa testada (200 Hz a
15 kHz), sempre do lado seguro (nunca subestima o pico real por mais de
~10%). **Não é conformidade EBU Tech 3341** — não temos as fixtures
oficiais pra validar contra elas (a NAVALHA teve HTTP 403 no pacote
oficial e usou fixtures derivadas matematicamente da especificação); é
uma estimativa medida e honesta, não uma certificação.

Integrado no `OutputStage` (`src/dsp/Master.hpp`'s dependência): o
detector do limitador agora usa o MAIOR entre pico de amostra e pico
verdadeiro. Prova construída (não hipotética): uma senoide de amplitude
real 0,95 (acima do teto de −1 dBFS = 0,891) cujo pico DE AMOSTRA (8 kHz,
fase deliberada) fica em ~0,88 — abaixo do teto, invisível pro limitador
antigo — agora aciona 0,85 dB de redução de ganho e sai a 0,85, dentro do
teto. Um sinal calmo (220 Hz, amplitude 0,5) permanece transparente (GR ≈
0) — a correção não deixa o instrumento mais comprimido em uso normal.

**2. `src/io/WavWriter.hpp` — guarda de finitude + arredondamento +
dither TPDF opcional.** Estudado de `AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md`
§3.7/P1.1 ("exportação sem dither" — achado real da NAVALHA, corrigido
lá). O escritor:
- não tinha guarda de NaN/Inf (o clamp por comparação `> 1.0f`/`< -1.0f`
  é sempre falso pra NaN — um NaN cru virava lixo indefinido no cast pra
  int16). Corrigido: `std::isfinite` antes do clamp;
- truncava (`static_cast<int16_t>`) em vez de arredondar — viés de ~meio
  LSB pra zero em toda amostra. Corrigido: `std::lround`;
- **novo parâmetro opcional** `ditherSeed` (padrão `0` = sem dither,
  bypass explícito — preserva os renders de auditoria/goldens
  byte-idênticos entre execuções, como os "WAVs dourados" da NAVALHA
  pedem `none`). Qualquer valor != 0 liga TPDF de ±1 LSB determinístico
  por esse seed (xorshift64*, mesmo padrão do resto do projeto); L e R
  nunca compartilham o mesmo par de uniformes. Ligado só na gravação ao
  vivo do painel (`[Ctrl+R]`), nunca nos renders de exemplo.

**Consequência esperada e aceita:** os 5 renders de referência em
`validation-output/` mudaram por até **±1 amostra (1 LSB)** — só o
arredondamento corrigido (truncar→arredondar), não o dither (que fica
desligado nesses renders) nem o true peak (os exemplos não passam por
`Master.hpp`). Regenerados nesta etapa; `.score.txt` (texto, não passa
pelo `WavWriter`) ficou byte-idêntico, confirmando o isolamento.

**Testes novos:** `tests/test_true_peak.cpp` (5 funções — rastreia
amplitude conhecida dentro de 4%, nunca subestima >10%, pega o caso de
estouro entre amostras que o pico de amostra sozinho não pegaria,
transparente em sinal calmo, determinismo) e `tests/test_wav_writer.cpp`
(5 funções — guarda de finitude, arredonda em vez de truncar, bypass de
dither é determinístico e byte-idêntico ao padrão anterior, dither muda
a saída mas é determinístico por seed e nunca cega o silêncio, L/R não
compartilham ruído). **41/41 CTest** Debug + Release (39→41). 5 renders
de exemplo com diff de ±1 LSB documentado (não byte-idênticos desta vez,
de propósito).

**Não verificado ainda** (ficam de fora desta etapa): "3.2 preview fora
do MASTER" (não identifiquei um caminho de preview separado no painel —
provavelmente não se aplica aqui); "3.3 medidor oculta overs" (o VU do
MASTER não tem clip-latch nem escala dBFS — melhoria de painel real,
não feita); "3.6 loudness não padronizada" (RASGO Modular não expõe
nenhuma medida de loudness hoje — não se aplica). Módulos "interessantes"
do ANTITOTEM mencionados pelo usuário: não levantados nesta etapa —
pedido separado, fica pra quando houver direção específica de quais.

**Não commitado.**

## Registro da etapa — 2026-09-04: Motion Engine — integrado ao painel ao vivo

Resposta direta ao feedback: "ainda não há variação de controles (cada
módulo fica estático com os knobs ou sliders sempre na mesma posição)".
O `MotionEngine` (§3 do estudo) só existia numa peça de exemplo; agora
está no `panel_main.cpp` interativo.

**Feito:**
- `populateMotion()` — pra cada módulo em `shown`, escolhe (hash
  determinístico tipo+id, sem depender do seed do cabeamento) **no
  máximo 1** widget `Knob`/`Slider` não-bloqueado
  (`PatchGenetics::isMutationBlocked` — mesma lista de "não estrutural"
  já usada por `MUTATE`) e cria um `Binding` `WALK` nele, faixa =
  min/max real do parâmetro, ritmo ~0,015–0,06 Hz (alvo novo a cada
  ~15–60 s, deslize proporcional — "respira", não "sacoleja"). Chamada
  em todo ponto que já chama `buildMods()` (seed novo, carregar sessão/
  banco, adicionar módulo, remover módulo) — sempre 1 fonte da verdade;
- `[v]` — liga/desliga a variação ao vivo (ligada por padrão); pausa
  sozinha enquanto o usuário está com a mão num knob (`drag.active`),
  pra não brigar com um giro manual;
- tick a cada iteração do loop principal (~30 fps, mesmo ritmo do
  redraw), fora de `gmx` — mesma via lock-free que o próprio arrasto de
  knob já usa (`setParameterBase`), motor aditivo: cabo/qualidade já
  modulando o parâmetro continua somando por cima, sem conflito.

**Validação:** sonda dedicada (todo o catálogo de módulos, 20 seeds,
~60 s simulados de variação cada, motor de áudio rodando em paralelo)
— **0/20 com problema**: sem NaN/Inf, sem silêncio, nenhum binding caiu
num parâmetro da lista bloqueada. **39/39 CTest** Debug + Release
(inalterado — a lógica vive só no painel, sem novo alvo de teste ainda).
5 renders de exemplo byte-idênticos (não passam por `panel_main.cpp`).
Painel compila; **não testado visualmente** (mesma ressalva dos
registros anteriores de painel).

Documentado no estudo (`ESTUDO_seed_composicao_generativa.md` §1.1, §7).

**Não commitado.**

## Registro da etapa — 2026-09-04: AUDIO-IN — entrada de áudio ao vivo (Módulo 35)

Resposta direta a "como podemos conectar o antitotem no rasgo modular? [...]
ou vice-versa [...] isso qualquer fonte externa [...] não altere o código
do antitotem". Investigado sem tocar no ANTITOTEM: nenhum dos dois lados
tem porta de entrada de áudio hoje (ANTITOTEM: `setAudioChannels(0,2)`;
RASGO Modular: `AlsaSink` é só playback) — o sistema já roda PipeWire
(`pw-link` instalado), mas isso só serve pra ligar SAÍDA em ENTRADA, e não
havia entrada em nenhum dos dois. Decisão: construir só o lado do RASGO
Modular (ANTITOTEM fica intocado, como pedido) — isso já resolve
ANTITOTEM→RASGO Modular (e qualquer app→RASGO Modular); o vice-versa
depende de trabalho no ANTITOTEM, fora deste escopo.

**Feito** — ver `dossies/35_audio_in.md` pro detalhe completo:
- `src/dsp/AudioIn.hpp` (zero-dep) — nó `Signal`, anel circular SPSC sem
  alocação em `process()`; sem alimentação = silêncio determinístico
  (testável sem hardware);
- `apps/panel/AlsaSource.hpp` — captura ALSA, contraparte de `AlsaSink.hpp`;
- fiação em `panel_main.cpp`: `syncAudioIn()` (abre a captura só quando o
  patch tem um `AUDIO-IN` de verdade, fecha quando sai — nunca pega o
  microfone à toa) numa thread PRÓPRIA (separada da que toca o grafo, pra
  não arriscar a temporização já delicada da reprodução);
- catálogo: família SOURCE, `moduleCatalog()`/`makeModule()`;
  `PatchSeed.hpp` não itera o catálogo às cegas, então `AUDIO-IN` nunca
  entra num seed aleatório — opcional de verdade.

**Validação:** `tests/test_audio_in.cpp` — 6 funções (silêncio sem
alimentação, ida-e-volta exata, gain aplicado, underrun não trava/suja,
estouro resincroniza, determinismo). **42/42 CTest** Debug + Release
(41→42). 5 renders de exemplo byte-idênticos. Painel compila; a fiação
ALSA/thread de captura **não pôde ser testada com hardware/PipeWire de
verdade** (não rodo o painel gráfico — combinado do projeto); só a lógica
do anel foi verificada.

**Não commitado.**

## Registro da etapa — 2026-09-05: paleta — passar o mouse destaca o módulo na case

"Pra facilitar identificar onde está o módulo no painel, ao passar o
mouse sobre o nome do módulo na coluna da esquerda, há um destaque no
painel do módulo" (feedback do autor).

**Feito** (`panel_main.cpp`, só desenho — sem estado novo persistido):
passar o mouse sobre um TIPO na paleta (coluna esquerda) — mesma faixa
de acerto já usada pelo clique-pra-adicionar — destaca a linha
(fundo + texto na cor de destaque) e todo módulo daquele tipo hoje
presente na case ganha o mesmo contorno duplo já usado pro módulo sendo
arrastado. Útil sobretudo com vários módulos do mesmo tipo ou um rack
grande/rolado, onde achar "qual é o segundo OSC" a olho é difícil.
Calculado a cada `redraw()` (~30 fps), mesmo padrão do tooltip do modo
aprender — nenhum estado guardado entre quadros.

**Validação:** 42/42 CTest Debug + Release (inalterado — é só desenho).
5 renders de exemplo byte-idênticos. Painel compila; não testado
visualmente (mesma ressalva de sempre).

**Não commitado.**

## Registro da etapa — 2026-09-05: item por item — pendências pequenas

Resposta a "quais outras coisas que não terminou ainda?" seguido de
"avance item por item" — trabalhando a lista de pendências levantada,
começando pelas menores/mais seguras. Itens de decisão arquitetural
grande (`CROSS`, contrato `NOTE` do `MUSICAL SCORE`, gramática explícita
do `Seed`) ficaram de fora de propósito — sinalizados, não decididos
sozinho.

**A. `AUDIO-IN` — seleção de dispositivo.** `RASGO_AUDIO_IN_DEVICE` no
ambiente (mesmo padrão do `RASGO_SEED`) — sem ela, `"default"`
(inalterado). `apps/panel/AlsaSource.hpp` + `panel_main.cpp`.

**B. MASTER — VU com clip-latch + leitura em dB.** Fecha o achado §3.3
da auditoria da NAVALHA ("o medidor esconde estouros") que eu mesmo
tinha listado como pendente. A saída "vu" do `MASTER` agora é uma barra
de nível real (pico da janela recente / teto de −1 dBFS) com leitura em
dB, e um indicador que acende quando `gainReductionDb()` (telemetria já
existente) mostra que o limitador teve que segurar algo — decai depois
de ~2 s sem novo estouro. Reaproveita telemetria que já existia; nenhum
estado novo no motor.

**C. RASGO Score — gravação ao vivo no painel.** `[Ctrl+R]` agora grava
`SYSTEM SCORE` junto do áudio: cabos existentes no início da tomada +
toda mudança de parâmetro feita à mão (arrastar knob) enquanto grava,
`t` relativo à tomada (amostras gravadas/sr). Escreve `rec-NN.score.txt`
ao lado do `.wav`. **Não captura ainda:** mudanças de `MUTATE`/`EVOLVE`/
Motion Engine durante a gravação, nem links de modulação por cabo (o
motor não tem enumerador público pra eles hoje) — pendências registradas,
não escondidas.

**D. `NOISE` — cores extras (azul/violeta/bit).** 3 saídas novas,
simultâneas às 5 já existentes — ver `dossies/19_ruido.md`. Painel
alargado de 12 pra 20 HP.

**E. Motion Engine — variedade de comportamento.** `populateMotion()`
escolhia sempre `WALK`; agora ~27% dos módulos recebem `OSCILLATE`
(ciclo previsível) em vez de `WALK` (alvo imprevisível), mesma faixa de
ritmo (~15–60 s), escolha determinística pelo mesmo hash tipo+id de
sempre. `ATTRACT` continua de fora (pediria relacionar DOIS módulos, não
dá pra derivar de um hash só).

**F. LearnCatalog — mais 5 módulos.** `NOISE` (incluindo as 3 saídas
novas do item D), `DRIFT`, `MIXER`, `MASTER`, `WASP` — 5 dos módulos
mais mexidos nesta sessão, cobrindo a cadeia de saída inteira
(MIXER→MASTER) além do rack de partida. **10 de 35 módulos** agora
(antes 5).

**Validação (A–F juntas):** 42/42 CTest Debug + Release (`test_noise`
ganhou 3 funções pros canais novos; `test_learn_catalog` ganhou uma
função cobrindo os 5 módulos novos). 5 renders de exemplo
byte-idênticos. Painel compila; não testado visualmente (mesma
ressalva de sempre).

**Não commitado.**

## Registro da etapa — 2026-09-05: ClockFeel — feel rítmico não-binário no CLOCK

Continuação do "avance item por item" — candidato G do levantamento
ANTITOTEM (`PESQUISA_MODULOS.md §2.3`).

**Feito** (`src/dsp/EuclidClock.hpp`): parâmetro `feel` (7 posições) —
reto/tercina/quintina/septina/nonina/undecina (razão 1/3/5/7/9/11,
multiplica em cima de `MULT`) + `glitch` (sorteia um fator de tempo por
passo, ~0,7–1,4×, em vez de razão fixa — único modo qualitativamente
novo). `swing` do Antitotem ficou de fora de propósito: o `CLOCK` já
tem um `swing` contínuo, duplicar como posição discreta só confundiria.

**Bug pego e corrigido durante a validação:** a primeira versão sorteava
o fator de glitch em TODA chamada de `advanceStep()`, mesmo com
`feel≠glitch` — isso consumia o mesmo stream de RNG que `drift` usa
pros próprios passos aleatórios, deslocando o `drift` de qualquer patch
que nunca mexeu em `feel`. Achado pelos 5 renders de exemplo terem
DIFERIDO onde antes eram byte-idênticos (2 deles usam `CLOCK.drift`).
Corrigido: só sorteia quando `feel` = glitch de verdade
(`advanceStep(bool glitch)`).

**Validação:** `tests/test_clock.cpp` ganhou 3 funções (tercina/
quintina escalam ~3x/~5x o passo reto; glitch tem variância de
intervalo bem maior que reto; determinismo do modo glitch). **42/42
CTest** Debug + Release. 5 renders de exemplo voltaram a byte-idênticos
depois da correção do RNG. `LearnCatalog` ganhou a entrada de `feel`
(CLOCK já era um dos 10 módulos documentados).

**Não commitado.**

## Registro da etapa — 2026-09-05: CHAOS — campo caótico de poço duplo (Módulo 36)

Continuação do "avance item por item" — candidato do levantamento
ANTITOTEM (`PESQUISA_MODULOS.md §2.3`), o segundo mais tratável depois
do `ClockFeel`.

**Feito:** `src/dsp/Chaos.hpp` — módulo NOVO, família DECISION. Fonte
genuinamente CAÓTICA (EDO não-linear de poço duplo), diferente de tudo
que já existia (`DECISION`/`TURING` sorteiam, `DRIFT` soma ruído
filtrado). `rate` (0,02–400 Hz, CV lenta a áudio), `drive`, `damping`,
`freeze`, `reseed` (trigger). O chute periódico aleatório é o que deixa
a trajetória cruzar de um poço pro outro — confirmado por teste, não só
por leitura do código-fonte do Antitotem.

**Validação:** `tests/test_chaos.cpp` — 7 funções (limitado e finito;
visita os dois poços; freeze segura exato; reseed diverge de uma
tomada de controle — comparação de trajetórias inteiras, não só o
salto instantâneo, que pode calhar pequeno por acaso; determinismo;
grafo `CHAOS→FILTER`; painel fecha). **43/43 CTest** Debug + Release
(42→43). 5 renders de exemplo byte-idênticos. Painel 8 HP, 0
sobreposições. **Não adicionado ao `PatchSeed.hpp`** (só disponível via
paleta, não em seed aleatório ainda — mesma decisão do `AUDIO-IN`, mas
aqui por falta de tempo de integração, não por ser opcional-nunca-
dependência).

**Não commitado.**

## Registro da etapa — 2026-09-05: proximidade por oscilador

Continuação do "avance item por item" — terceiro candidato do
levantamento ANTITOTEM (`PESQUISA_MODULOS.md §2.3`), **só metade**
implementada de propósito.

**Feito:** `src/dsp/Oscillator.hpp` ganhou `prox` — mistura as 5
saídas (sine/tri/saw/pulse/sub) com uma versão de si mesmas passada por
um passa-baixa de 1 polo bem suave (coeficiente 0,06, mesmo valor do
estudo). `prox=0` é EXATAMENTE a saída crua — mesma convenção de
`drift=0`, confirmado por teste e pelos 5 renders de exemplo
continuarem byte-idênticos.

**`órbita` (a outra metade do par proximidade/órbita do Antitotem)
ficou de fora de propósito, não por falta de tempo**: `órbita` seria um
LFO de pitch autônomo — e o `OSC` JÁ TEM `drift` fazendo exatamente
esse papel (passeio lento correlacionado na afinação). Adicionar
`órbita` duplicaria `drift`, não contribuiria nada novo — decisão
documentada, não esquecimento.

**Validação:** `tests/test_oscillator.cpp` ganhou 2 funções (`prox=0`
idêntico byte a byte à saída sem tocar no parâmetro; `prox=1` reduz a
energia de alta frequência da SAW em bem mais que 60%). **43/43 CTest**
Debug + Release (inalterado — nenhum alvo novo, conteúdo dentro de
testes existentes). 5 renders de exemplo byte-idênticos (confirma que
`prox=0` não mudou nada em nenhum patch que já existia). `LearnCatalog`
ganhou a entrada de `prox` (OSC já era um dos 10 módulos documentados).

**Não commitado.**

## Registro da etapa — 2026-09-05: PLL — oscilador de malha de fase (Módulo 37)

Fecha os 2 últimos candidatos do levantamento ANTITOTEM
(`PESQUISA_MODULOS.md §2.3` — a lista inteira de 6 candidatos está
concluída agora). Pedido explícito do autor, depois de eu perguntar
"modo dentro do OSC ou módulo dedicado?": **"faz o b, porém será um
oscilador sofisticado, com itens que o primeiro não contém ainda"** —
"acho bom haver dois osciladores, mais independência, e possibilidades
de variação".

**Feito:** `src/dsp/Pll.hpp` — módulo NOVO, família SOURCE, 16 HP. Toca
livre sozinho (FREQ/FINE próprios — é um segundo VCO de verdade, não um
efeito). Com uma referência de fase plugada (`REF`), a taxa CURVA pra
perseguir em vez de resetar duro (`OSC.sync_enable` continua sendo hard
sync, inalterado). Itens que o `OSC` não tem: `shape` (morph contínuo
seno↔tri↔serra↔quadrada), `ratio` (0,03–8×, trava além de 1:1 — desvio
Rasgo, o Antitotem só persegue 1:1), `lock_gain` exposto, rede de
feedback selecionável (6 tipos), saída `ring` (heterodino contra a
referência), saída `lock` (CV 0..1 de travamento — desvio Rasgo).

**Bug pego e corrigido durante a validação:** o sinal do erro de fase
estava invertido na primeira versão — a correção puxava a taxa pro lado
ERRADO (se afastando da referência). Achado porque a sonda de
travamento não convergia; corrigido pra bater a convenção do estudo
(`própria fase − esperada`, não o contrário).

**Achado real, documentado, não escondido:** a correção é limitada a
±0,9 (segurança) — isso dá ao `PLL` um "alcance de captura" limitado,
igual um PLL analógico de verdade: um descompasso que pediria correção
além de ±0,9 nunca trava exato, só se aproxima (caça de verdade). Medido
por sonda dedicada antes de escrever o teste final, com números
específicos no dossiê.

**Validação:** `tests/test_pll.cpp` — 9 funções (livre sem referência;
trava dentro do alcance de captura; RATIO=2 trava numa oitava acima;
LOCK sobe com referência, fica em 0 sem ela; as 6 redes de feedback
ficam limitadas/finitas; RING precisa de REF; determinismo; grafo
`OSC.saw→PLL.ref`; painel fecha). **44/44 CTest** Debug + Release
(43→44). 5 renders de exemplo byte-idênticos. Painel 16 HP, 0
sobreposições.

**Com isto, os 6 candidatos do levantamento ANTITOTEM
(`PESQUISA_MODULOS.md §2.3`) estão todos feitos.** Ficam em aberto só
os itens de decisão arquitetural grande: `CROSS`, o contrato `NOTE` do
`MUSICAL SCORE`, e a gramática explícita do `Seed` (§1.1).

**Não commitado.**

## Registro da etapa — 2026-09-05: CROSS — cruzamento de patches

"A peça mais arriscada da lista" (`ESTUDO_seed_composicao_generativa.md
§4`) — a única das 3 decisões de arquitetura grande que tinha um
caminho de implementação claro o bastante pra resolver sem inventar
nada novo. As outras duas (contrato `NOTE` do `MUSICAL SCORE`, gramática
explícita do `Seed`) continuam em aberto — pedem decisão de design, não
só engenharia.

**Alinhamento adotado** (o problema central do `CROSS`: dois grafos de
seeds diferentes quase nunca têm os mesmos nós nas mesmas posições) —
por TIPO de módulo: a k-ésima ocorrência de um tipo no ALVO casa com a
(k mod contagem-no-doador)-ésima ocorrência do MESMO tipo no DOADOR. Um
nó do alvo sem tipo correspondente no doador fica intocado — nunca caça
substituto de outro tipo. Mesma simplificação do `MUTATE`: só parâmetro,
nunca cabo/topologia (crossover de TIMBRE, não de forma).

**Feito:**
- `apps/panel/PatchGenetics.hpp::crossPatch(target, donor, seed,
  fraction, frozen)` — reaproveita a MESMA infraestrutura do `MUTATE`
  (`isMutationBlocked`, `setParameterBase`, nós tocados por cabo,
  conjunto `frozen`), só troca a FONTE do valor novo (do doador, não de
  um sorteio);
- `[c]` no painel — cruza o patch atual com um DOADOR novo (catálogo
  inteiro semeado com um seed aleatório fresco, igual a um patch de
  verdade), fração 0,5 (crossover clássico — ~metade de chance por
  parâmetro elegível), FREEZE automático de `MIXER`/`MASTER` (mesma
  mitigação do `MUTATE`/`EVOLVE`).

**Validação:** `tests/test_patch_genetics.cpp` ganhou 5 funções
(fraction=1 iguala exatamente o doador; FREEZE/não-tocado protegem
igual ao MUTATE; sem tipo correspondente no doador o nó fica intocado;
2 ocorrências do alvo casam com 1 só do doador, módulo o tamanho;
determinismo). Sonda dedicada replicando a chamada do painel (FREEZE de
MIXER/MASTER, 30 seeds, doador = seed diferente) — **0/30 mudos**, mesmo
resultado do `MUTATE`/`EVOLVE` com freeze. **44/44 CTest** Debug +
Release (inalterado — conteúdo dentro de alvos existentes). 5 renders
de exemplo byte-idênticos. Painel compila; não testado visualmente.

**Com isto, dos 3 itens de decisão arquitetural grande sinalizados
desde o início, só 2 continuam em aberto: o contrato `NOTE` do `MUSICAL
SCORE` e a gramática explícita do `Seed` (§1.1 do estudo).**

**Não commitado.**

## Registro da etapa — 2026-09-05: NOTE-OUT — contrato NOTE do MUSICAL SCORE (Módulo 38)

Segundo dos 2 itens de decisão arquitetural que sobravam
(`ESTUDO_seed_composicao_generativa.md §5`/§7). Antes de construir,
conversei com o autor sobre o desenho ("o que sugere?") — 4 pontos
combinados: módulo adaptador dedicado (não mudar os módulos
existentes), pitch em 1V/oct cru (conversão fica pra hora de exportar),
mesmo `ScoreRecorder` com um tipo de evento novo, e captura primeiro
(exportação de verdade pra depois). v1 monofônica, decidida e
documentada, não escondida.

**A reformulação que evitou o risco:** a ideia original era fazer
`ENVELOPE`/`SEQUENCE`/etc. "anunciarem" eventos de nota de forma
genérica — mudaria a interface de vários módulos, por isso o item
ficou em aberto por tanto tempo. Resolvido diferente: `NOTE-OUT`
(`src/dsp/NoteOut.hpp`, zero-dep, SEM saber o que é `ScoreRecorder`) é
um OBSERVADOR que você cabeia em GATE+PITCH onde quiser — detecta
borda de nota-liga/desliga sozinho, expõe a nota completa por
`takeCompletedNote()`. Nenhum dos outros 37 módulos mudou.

**Achado arquitetural real, verificado por sonda antes de documentar
como fato:** o motor só processa nós que chegam ao `sink` ativo
("órfãos não custam DSP por bloco") — um `NOTE-OUT` sem saída nenhuma
NUNCA seria ancestral do sink, então NUNCA teria `process()` chamado,
mesmo com gate/pitch plugados. Por isso ganhou `gate_thru`/`pitch_thru`
(cópia exata das entradas) — precisa ficar EM LINHA
(`ALGO.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate`) pra
rodar de verdade. Confirmado com uma sonda dedicada (fora de linha:
`process()` nunca roda; em linha: roda normal) antes de escrever isso
no dossiê como garantia.

**Feito:**
- `src/dsp/NoteOut.hpp` — módulo novo, família MIX;
- `apps/panel/ScoreRecorder.hpp` — `RasgoEvent::Type::Note` +
  `note(t, node, pitch, velocity, duration, accent)` + linha de texto;
- `panel_main.cpp` — lê `takeCompletedNote()` de todo `NOTE-OUT` em
  `shown`, a cada bloco, só enquanto `[Ctrl+R]` grava; `t` = início da
  nota (conclusão menos duração), relativo à tomada.

**Validação:** `tests/test_note_out.cpp` — 6 funções (sem gate nunca há
nota; nota-liga/desliga com pitch/duração corretos; velocity/accent
amostrados no instante certo; thru é cópia exata; determinismo; painel
fecha). `tests/test_score_recorder.cpp` ganhou 1 função pro evento
`Note`. **45/45 CTest** Debug + Release (44→45). 5 renders de exemplo
byte-idênticos. Painel 8 HP, 0 sobreposições.

**Com isto, resta 1 só item de decisão arquitetural grande: a
gramática explícita do `Seed` (§1.1 do estudo).**

**Não commitado.**

## Registro da etapa — 2026-09-04: MASTER — gain fixo em 30% do slider

Segunda correção de volume no mesmo dia — desta vez um número exato, não
uma leitura de "50%" minha: "preciso que o instrumento comece com o som
de saída em 30% do slider (para todos os seeds), subo na mão". `n =
(v−lo)/(hi−lo)` é como o painel calcula a posição visual do slider
(`panel_main.cpp`) — pra `gain` (−60..+12 dB), 30% = **−38,4 dB**, fixo
(não sorteado) em todo seed.

**Feito:**
- `src/dsp/Master.hpp` — default `gain` → `-38.4f`;
- `apps/panel/PatchSeed.hpp` — `setT("MASTER","gain", rng(-8,-4))` virou
  um valor FIXO `-38.4f` (antes era faixa sorteada) — "para todos os
  seeds" pede o mesmo valor sempre, não uma faixa;
- sessão salva do usuário — `gain` também ajustado direto pro mesmo
  valor (a sessão só carrega com `--resume` desde o registro de
  "abertura sempre gera um seed novo", mas ajustado por consistência).

**Efeito colateral pego e corrigido:** `tests/test_wasp.cpp::testInGraph`
dependia implicitamente do default antigo do `MASTER.gain` (roteava
OSC→WASP→`MASTER` sem fixar o gain, checando o pico da SAÍDA do
`MASTER`) — com −38,4 dB o pico caiu abaixo do limiar do teste.
Corrigido com `gain=0` explícito no teste (mesma classe de ajuste já
feita em `test_mix.cpp` no registro de volume anterior).

**Validação:** 50 seeds — 1 "mudo" limítrofe (rms=0,000377, contra o
limiar de 0,0005 da sonda) — não é um patch realmente silencioso, é o
efeito esperado de um piso 30 dB mais baixo empurrando um patch já
quieto pra perto do limiar de medição; RMS máximo caiu pra 0,022 no
total. **42/42 CTest** Debug + Release. 5 renders de exemplo
byte-idênticos.

**Não commitado.**

## Registro da etapa — 2026-09-04: Patch Genetics — MUTATE/EVOLVE no painel ao vivo

Fecha a pendência deixada em aberto desde o registro "Patch Genetics —
MUTATE/EVOLVE/FREEZE (protótipo)": `PatchGenetics.hpp` existia e tinha
teste próprio, mas nunca foi ligado ao painel interativo. Resposta a
"termine esse trabalho que não finalizou".

**Feito** (`apps/panel/panel_main.cpp`):
- `[m]` — MUTATE uma vez, fração 0,25;
- `[e]` — EVOLVE, 6 passos de 0,12 cada (a mesma distância, percorrida
  mais devagar que um MUTATE único);
- os dois congelam `MIXER`/`MASTER` automaticamente antes de mutar — a
  mitigação que o `PatchGenetics.hpp` recomendava mas nunca tinha sido
  aplicada em lugar nenhum. Sob `gmx` (deliberado — MUTATE escreve muitos
  parâmetros de uma vez, diferente do arrasto de knob que é lock-free);
- `populateMotion()` roda de novo depois — os alvos do Motion Engine
  reiniciam coerentes com os valores recém-mutados, em vez de continuar
  perseguindo uma trajetória de antes da mutação.

**Validação:** sonda dedicada replicando exatamente a chamada do painel
(FREEZE de MIXER/MASTER, 30 seeds) — **MUTATE 0/30 mudos, EVOLVE 0/30
mudos** (o baseline documentado sem freeze era 4/30). Comentário do
`PatchGenetics.hpp` atualizado pra registrar a mitigação implementada.
**42/42 CTest** Debug + Release (inalterado — lógica só no painel). 5
renders de exemplo byte-idênticos. Painel compila; não testado
visualmente (mesma ressalva de sempre).

**Não commitado.**

## Registro da etapa — 2026-09-02: CONTROL — utilidades de CV (Módulo 21)

Quarto dos essenciais (`PESQUISA_MODULOS.md §2.1`). A "gramática do
sistema" — escalar, deslocar, retificar, atrasar CV — não tinha um
módulo visível (só o `gain` do `Cable` e o `connectToParameter`).
Dossiê antes do código (`dossies/21_control.md`).

**Feito:** `src/dsp/Control.hpp` (`type()` = `"CONTROL"`, UTILITY)
- **duplo** (2 canais de CV); `in1`/`in2` → `out1`/`out2` + `sum`;
- **`scale`** (−2…+2) atenuversor: `<0` inverte, `>1` amplifica,
  **`= 0` → o canal vira fonte de tensão** (só o `offset`);
- **`offset`** (−1…+1) constante somada pós-`scale`;
- **`rectify`** (0…1) = `lerp(x, |x|, rectify)` — 0 passa, 0,5 meia-onda
  (`max(x,0)` exato), 1 onda-completa;
- **`slew`** (0…1, `s²·2 s`) + **`curve`** (0…1): slew linear (inclinação
  constante — portamento) ↔ lag exponencial (RC — seguidor de envelope).
  `rectify` + `slew` = **seguidor de envelope**;
- **`sum`** = `out1`+`out2`, com `sum_mode` soma (com teto) / média;
- desvio **`drift`** (0…1) — passeio lento ±~1,5 % somado ao offset,
  xorshift semeado. `drift = 0` → determinístico.

**Validação:** `tests/test_control.cpp` — **14 funções OK** (scale ±,
inversão, `scale=0` = fonte de tensão, offset, retificação meia/onda-
completa, slew instantâneo, rampa de slew, linear vs exp, seguidor de
envelope num seno, soma/média, drift < 3 % e determinismo, finitude,
grafo `LFO → CONTROL(slew) → OSC.pitch` = portamento). **21/21 CTest**
Debug + Release. Renders de exemplo byte-idênticos. Painel 12 HP,
**0 sobreposições**. Adicionado ao catálogo (família TRANSFORM, junto do
`VCA`).

**Falta pra fechar o rack de partida:** `LOGIC` (divisor de clock +
lógica booleana de gates + gate delay).

**Não commitado.**

## Registro da etapa — 2026-09-02: LOGIC — lógica e utilidades de clock (Módulo 22)

Quinto e ÚLTIMO dos essenciais (`PESQUISA_MODULOS.md §2.1`) — **fecha o
rack de partida**. A peça que recombina o tempo: divide/multiplica um
clock, faz AND/OR/XOR de dois gates, flip-flop T e atraso de gate.
Dossiê antes do código (`dossies/22_logic.md`).

**Feito:** `src/dsp/Logic.hpp` (`type()` = `"LOGIC"`, TIME/UTILITY)
- **`rate`** (0,1–40 Hz) — relógio interno, usado só quando `clock` está
  livre (LOGIC sozinho = gerador de ritmo → modo autônomo);
- **`divide`** (1–32) — contador módulo-N sobre as bordas;
- **`multiply`** (1–8) — mede o período e agenda M sub-bordas
  (extrapola um período à frente, estilo Pamela's);
- **`gate_len`** (0,02–0,98) — duty do pulso `div`;
- **`delay`** (0–1 → 0–200 ms) — anel de amostras 0/1 (alocado em
  `prepare()`);
- **`and`/`or`/`xor`** — saídas booleanas simultâneas de `a`,`b`
  (limiar 0,5);
- **`flip`** — flip-flop T: alterna a cada borda ↑ de `a`; `reset` (nível)
  zera contador **e** flip.
- Sem RNG (timing de precisão). Determinístico. Sem alocação em
  `process()`.

**Validação:** `tests/test_logic.cpp` — **13 funções OK** (÷2 dá metade
dos pulsos; ÷1 passa; ×2 dobra; `gate_len` controla o duty; `delay ≈ 0,5`
→ atraso ~4800 amostras; tabela-verdade AND/OR/XOR pras 4 combinações;
flip alterna e só na borda ↑; `reset` zera o flip; relógio interno gera
`div` sem entrada; finitude e {0,1}; determinismo; grafo
`CLOCK → LOGIC(÷2).clock` · `LOGIC.div → ENVELOPE.gate`). **22/22 CTest**
Debug + Release. Renders de exemplo byte-idênticos. Painel 10 HP,
**0 sobreposições**. Catálogo: família TIME, junto do `CLOCK`.

**Rack de partida COMPLETO:** `OSC` · `NOISE` · `VCA` · `CONTROL` ·
`LOGIC` (+ `FILTER` `ENVELOPE` `FUNCTION` `CLOCK` `SEQUENCE`/`TURING`
`QUANTIZER` `HARMONY` `MIXER` `MASTER` já existentes). **22 módulos DSP.**

**Não commitado.**

## Registro da etapa — 2026-09-05: SeedGrammar — a gramática de portas do Seed, nomeada

Terceira e última das três decisões arquiteturais sinalizadas nesta
etapa (`CROSS`, contrato `NOTE`, e esta). Lendo `PatchSeed.hpp::seedPatch()`
inteiro antes de mexer: a gramática **já existia** — classificação de
porta (`enum Src`/`Dst` locais) e matriz de compatibilidade fonte×destino
(`W[6][5]`), só que anônima, embutida em variáveis locais dentro de uma
função de ~500 linhas, sem nome exposto nem documentação.

**Decisão de escopo (confirmada com o autor, "sim, topo essa leitura"):**
NÃO reorganizar em torno das 6 categorias literais da conversa
(`SOURCE→TRANSFORM→CONTROL→MODULATE→FEEDBACK→OUTPUT`) porque (a) seria
uma REGRESSÃO de precisão — a classificação real é por PORTA, não por
módulo (`NOISE.smooth` e `NOISE.white` são o mesmo módulo, classes de
porta diferentes) — e (b) mudaria silenciosamente o que cada
`RASGO_SEED=N` já produz, sem rede de segurança formal (`find tests
-iname "*seed*"` → nenhum `test_patch_seed.cpp` existe). Em vez disso:
extração pura, com o objetivo explícito de **zero mudança de
comportamento**.

**Feito:** `apps/panel/SeedGrammar.hpp` (novo)
- `SeedSrc`/`SeedDst` (enums nomeados, mesmos valores/ordem dos antigos
  `S_*`/`D_*` locais — compatibilidade de índice preservada);
- `seedClassifyPorts(SignalGraph&, outSrc[6], outDst[5])` — puro, sem
  RNG, movido byte a byte do loop de classificação original;
- `seedCompatibilityMatrix(wildness, W[6][5])` — movido byte a byte;
- `seedIsGateName()`, `seedSrcName()`/`seedDstName()` (nomes legíveis,
  pra diagnóstico futuro, não usados pela lógica de sorteio);
- comentário de cabeçalho com a tabela de correspondência pro
  vocabulário `SOURCE/TRANSFORM/CONTROL/MODULATE/FEEDBACK/OUTPUT`,
  explicando por que `FEEDBACK` (propriedade do cabo, `reaches()`) e
  `OUTPUT` (`sink`/`MASTER` fixo) não são classes de porta e continuam
  em `PatchSeed.hpp`.

`apps/panel/PatchSeed.hpp` — enum/loop/matriz locais removidos,
substituídos por chamadas a `seedClassifyPorts()`/`seedCompatibilityMatrix()`;
os dois call-sites que usavam nomes curtos antigos (`D_GATE`/`D_PITCH`/
`D_AUDIO`/`S_BUS`) atualizados pros nomes qualificados novos.

**Validação — problema real identificado antes de escrever qualquer
prova:** nenhum dos 5 `examples/*.cpp` chama `seedPatch()`
(`grep -c "seedPatch" examples/*.cpp` → zero em todos) — o checkout
padrão desta sessão ("5 renders byte-idênticos") **não cobre este
código** e não pegaria uma regressão aqui. Sem teste dedicado
(`test_patch_seed.cpp` não existe). Construída uma prova própria:
- Congelada uma cópia do `seedPatch()` PRÉ-refactor (capturada verbatim
  da leitura desta mesma sessão, antes de qualquer edição), renomeada
  `seedPatchOld`/`rasgo::panel_old`, num header à parte fora do
  repositório;
- Sonda (`$CLAUDE_JOB_DIR/tmp/seedverify.cpp`) monta o grafo completo
  do catálogo (mesmo padrão de `moduleCatalog()`+`makeModule()` do
  painel), roda `seedPatch()` (novo) e `seedPatchOld()` (congelado) pros
  seeds 1–200, serializa cada grafo (`SignalGraph::serialize()` — texto
  determinístico de todo parâmetro de intenção + todo cabo) num arquivo
  por versão;
- `diff` entre as duas saídas (200 seeds, 13.163 linhas cada) —
  **byte a byte idêntico**. O refactor não mudou UMA amostra do que
  nenhum seed já produzia.

**Validação padrão:** build limpo (inclui `rasgo_modular_panel`, que
inclui `PatchSeed.hpp`+`SeedGrammar.hpp`). **45/45 CTest** Debug e
Release (nenhum módulo/teste novo — extração pura, contagem não muda).
5 renders de exemplo — isolamento de fonte confirmado (nenhum `examples/*.cpp`
inclui `PatchSeed.hpp`/`SeedGrammar.hpp`, direta ou transitivamente) +
determinismo confirmado por hash `sha256` em duas rodadas (idêntico).

**Falta:** a reformulação real de `seedPatch()` em torno das 6
categorias como estrutura de dados variável (não só nomeada) continua
fora de escopo — ver `ESTUDO_seed_composicao_generativa.md §1.1/§7`.

**Não commitado.**

## Registro da etapa — 2026-09-05: LearnCatalog — hover-learn completo pros 37 módulos

Pedido: "agora finalize o learn para todos os controles e jacks". O
protótipo (`dossies/ESTUDO_seed_composicao_generativa.md §6`) tinha
conteúdo só pra 10 dos 37 tipos do catálogo (`FILTER`, `ENVELOPE`,
`OSC`, `VCA`, `CLOCK`, `NOISE`, `DRIFT`, `MIXER`, `MASTER`, `WASP`).
Faltavam 27: `ABACUS`, `AUDIO-IN`, `CHAOS`, `CHORD`, `CONTROL`,
`DECISION`, `FUNCTION`, `HARMONY`, `LOGIC`, `LPG`, `MATRIX`, `MATTER`,
`MEMORY`, `MULT`, `NOTE-OUT`, `PARAMETRIC`, `PLL`, `QUANTIZER`, `SH`,
`SCOPE`, `SHAPE`, `SPACE`, `SEQUENCE`, `STRING`, `SWITCH`, `TRIGSEQ`,
`TURING`.

**Feito:** `apps/panel/LearnCatalog.hpp` — lido cada `src/dsp/*.hpp`
(construtor + `panel()`) pra tirar os binds exatos (nome de parâmetro,
`in:`/`out:` de porta) direto do código, não de memória; escrito
`quick` pra TODO knob e TODO jack dos 27 módulos, com `understand`/
`explore` nos mecanismos que valem explicar (ex.: por que `CLOCK.feel`
precisava de RNG isolado — já documentado no código, agora também no
tooltip; o alcance de captura do `PLL`; a assimetria de vactrol do
`LPG`; a granularidade de porta vs. módulo do `MATRIX`/`SHAPE`) e vazio
em parâmetros autoexplicativos (ex.: `MIXER.gain2`) — mesmo padrão de
"não escrever prosa por escrever" do `MODULE_DEVELOPMENT_STANDARD`.
Comentário de cabeçalho reescrito: de "protótipo, 10 de 35" pra
"catálogo completo, 37 de 37", com nota sobre a relação `Cable`
(RingMod/Fold/Difference) ficar de fora por ser propriedade do cabo, não
um tipo de módulo com painel próprio.

**Validação:** `tests/test_learn_catalog.cpp` ganhou
`testRemainingCatalogModulesHaveAtLeastQuick()` — resolve `lookupLearn`
pra cada bind dos 27 módulos novos (mesma lista tirada do `panel()` de
cada um), pega qualquer erro de digitação no nome do bind (que senão
falharia silenciosamente — `lookupLearn` devolve `nullptr` sem avisar,
por design, "sem nota ainda" não é erro). **45/45 CTest** Debug e
Release. 5 renders de exemplo — hash `sha256` idêntico ao de antes desta
mudança (confirma o esperado: `LearnCatalog.hpp` é conteúdo textual puro
de UI, nenhum exemplo/DSP o inclui).

**Falta:** `understand`/`explore` (2º/3º nível) continuam guardados na
struct mas NÃO desenhados no painel — precisa de um estado de "clique
pra expandir" ou dwell mais longo em `panel_main.cpp` (arquitetura de
exibição, não mais conteúdo). `WHY?`/`WHAT IF?` contextuais (dependem do
que já está cabeado) continuam de fora.

**Não commitado.**

## Registro da etapa — 2026-09-05: paleta — módulos em ordem alfabética dentro de cada tema

Pedido: "organize os módulos em cada item da coluna da esquerda por
ordem alfabética (os temas pode continuar como estão)".

**Cuidado tomado antes de mexer:** `moduleCatalog()`
(`ModuleCatalog.hpp`) não é só a fonte da paleta — a MESMA ordem de
iteração monta o rack inicial "um de cada módulo" (`main()`,
`byType[t] = id`), o grafo doador do `CROSS`, e a auditoria de
sobreposição. A ordem de instanciação decide o ÍNDICE de cada nó, e
`seedClassifyPorts` (`SeedGrammar.hpp`) enche `src[]`/`dst[]` na ordem
dos nós — mudar a ordem do catálogo mudaria em qual posição de cada
lista um `rnd() % lista.size()` cai, e portanto o que cada
`RASGO_SEED=N` produz (a mesma classe de regressão que o refactor da
gramática do Seed evitou por pouco nesta mesma etapa, ver o registro
"SeedGrammar" acima). `deserialize()` não depende da ordem do catálogo
(reconstrói cada nó pelo tipo salvo no próprio arquivo `.rmp`), então
`.rmp` salvos não corriam risco — mas o Seed corria.

**Feito:** `apps/panel/panel_main.cpp` — só o laço que MONTA a lista de
exibição da paleta (`palette.push_back(...)`) passou a percorrer uma
CÓPIA ordenada alfabeticamente de `g.types` (`std::sort` com comparação
de `std::string`); `ModuleCatalog.hpp::moduleCatalog()` em si **não foi
tocado** — a ordem que monta o grafo, o doador do `CROSS` e a
classificação de porta do Seed continuam exatamente as mesmas. Os temas
(famílias) continuam na ordem original, como pedido.

**Validação:** build limpo, **45/45 CTest** Debug e Release, 5 renders
de exemplo com hash `sha256` idêntico ao de antes (confirma que a
mudança é 100% cosmética, sem efeito no grafo/DSP/seed).

**Não commitado.**

## Registro da etapa — 2026-09-05: MATRIX — sobreposição de elementos no painel

Reporte do autor: "o módulo matrix tá com sobreposição de elementos".

**Causa:** `test_panel_layout` (o gate de regressão de layout) nunca
cobriu `MATRIX` — nem `MULT`, `PLL`, `CHAOS`, `NOTE-OUT`, `AUDIO-IN`,
todos adicionados ao catálogo depois do teste. E, no caso do `MATRIX`,
o teste sozinho não bastaria: os 16 `g<jk>` são declarados como `Knob`
em `Matrix.hpp` mas `panel_main.cpp` os PULA e desenha a grade 4×4 via
`matrixCellMM` (cx = 27 + k·17, cy = 35 + j·18, células de 16×17 mm) —
maiores que os knobs. A grade real chega a y≈97,5; os jacks `OUT` a
y=104 tinham o rótulo "OUT" (desenhado ACIMA do jack, ~y 97–101)
encostando na última linha da grade.

**Feito:**
- `tests/test_panel_layout.cpp` — incluídos os 6 módulos que faltavam
  (`MATRIX`, `MULT`, `PLL`, `CHAOS`, `NOTE-OUT`, `AUDIO-IN`);
  `footprints()` ganhou um caso `MATRIX` que modela a pegada REAL
  (células `matrixCellMM` no lugar dos 16 knobs + o texto-guia
  "IN↓ OUT→" em ~(50, 24)). Verificado que o teste PEGA a regressão:
  com os `OUT` em y=104 ele falha com 7 linhas
  `cell:g4x x rot:OUTx`; com o conserto, passa.
- `src/dsp/Matrix.hpp` — jacks `OUT` de y=104 → **y=110** (folga de
  ~5 mm entre o fundo da grade e o rótulo "OUT"; mesma faixa de rodapé
  que `TRIGSEQ`/`MATTER` já usam). Comentário no código explicando o
  porquê. Os outros 5 módulos recém-cobertos passaram limpos — só o
  `MATRIX` tinha o problema.

**Validação:** build limpo, **45/45 CTest** Debug e Release
(`test_panel_layout` agora cobre 37 módulos), 5 renders de exemplo com
hash `sha256` idêntico (`panel()` não está no caminho de áudio).

**Não commitado.**

## Registro da etapa — 2026-09-05: painel — NOISE mais estreito, rack na ordem da paleta, título UTF-8

Três ajustes de painel pedidos pelo autor em sequência:

**1. NOISE largo demais.** Tinha ido pra 20 HP quando ganhou as 8 saídas
(branco/rosa/brown/S&H/smooth + azul/violeta/bit) numa fileira só.
`src/dsp/Noise.hpp` — 8 jacks em **2 fileiras de 4** → volta pra **12 HP**
(a largura original). `test_panel_layout` cobre.

**2. Rack na ordem da listagem.** `apps/panel/panel_main.cpp` — a paleta
já estava em ordem alfabética por família; agora o rack inicial (`shown`)
também. Separei explicitamente as DUAS ordens:
- instanciação dos nós = ordem de `moduleCatalog()` (índice de nó
  estável — `seedClassifyPorts`, doador do CROSS dependem disso);
- exibição (`shown` + paleta) = famílias na ordem do catálogo, módulos
  alfabéticos dentro da família.
`shown` já era reordenável arrastando e salvo no `.panel`, então mexer
na ordem inicial dele é seguro.

**3. Acentuação quebrada no título da janela.** `XStoreName` grava
`WM_NAME` como STRING (Latin-1); os títulos com "—"/"●"/acento (ao
apertar `[v]`, `[m]`, `[e]`, `[c]`, salvar, etc.) apareciam como lixo na
barra do gerenciador de janelas. Adicionado `setTitle()` que grava
`_NET_WM_NAME` e `_NET_WM_ICON_NAME` como `UTF8_STRING` (o que o WM
moderno lê), mantendo `XStoreName` só de fallback. Todos os 10 pontos
que setavam título passaram a usar `setTitle()`. O texto DENTRO do
painel já era UTF-8 (`Xutf8DrawString` + fontset) — era só o título.

**Validação:** build limpo, **45/45 CTest** Debug e Release, 5 renders
de exemplo com hash `sha256` idêntico (as três mudanças são de UI, nada
toca DSP).

**Não commitado.**

## Registro da etapa — 2026-09-05: body guard — proteção contra agudo que "incomoda o corpo"

Pedido do autor, depois de ouvir frequências agressivas no painel:
"gosto de barulhos, mas há alguns que passam do limite, incomodam o
corpo do ouvinte". Rendi a voz mínima do painel e confirmei que ELA é
limpa (pico −42 dBFS, nada acima de 3 kHz) — o agudo vem de patches/
parâmetros. Proposta descrita e aprovada ("ok avance dentro da ideia b
com body guard"): transparente pra som agressivo, só age quando cruza
pro território que machuca.

**Feito:** `src/dsp/OutputStage.hpp` (usado pelo `MASTER`)
- `struct TptSvf2` — SVF TPT (Zavalishin/Cytomic) de 2 polos, estável
  perto de Nyquist, dá LP e BP de um cálculo só;
- **guarda ultrassônica** (sempre ligada, não é knob): LP Butterworth 2
  polos a ~21 kHz. −0,1 dB a 8 kHz, −0,5 dB a 19 kHz, −8 dB a 22 kHz —
  tira só o ice-pick perto de Nyquist (violeta/`bit` sustentados,
  aliasing). Filosofia do estágio: como o bloqueio de DC, nunca é
  escolha musical;
- **governador de corpo** (`bodyGuard` 0..1, parâmetro `body_guard` no
  `MASTER`, default 1,0; 0 = bypass EXATO): 2 detectores BP estreitos
  (Q 3) em 2,8/4,8/7,6 kHz → seguidores lentos (~240 ms ataque) → "gate de
  concentração" (razão banda/total — tom concentrado dispara, ruído de
  banda larga não) → high-shelf de 1 polo (corte acima de ~1,4 kHz),
  alvo `over·2,1` limitado a 0,72 (≈ −9 dB de shelf), subindo/descendo
  devagar. Telemetria `bodyGuardDb()`;
- ordem no estágio: finitude → DC → ultrassônica → corpo → pico
  verdadeiro (agora medido na SAÍDA dos guardas) → limitador → teto.
- `Master.hpp`: parâmetro `body_guard`, knob "BODY" no painel (8 HP,
  ao lado do toggle LIMIT), telemetria `bodyGuardDb()` repassada.

**Calibração (sonda `osdiag`):** senoide sustentada de 3,2 kHz —
−8 dBFS: −0,6 dB; −5 dBFS: ~2 dB; −2,5 dBFS: ~4 dB; −0,9 dBFS: ~5,5 dB.
Progressão suave, "não some o som". Ruído branco alto (−9 dBFS RMS):
< 1,5 dB (gate de concentração deixa passar). Rajadas de 3 kHz de 18 ms
a cada 380 ms (ritmo): ~0 (ataque lento). 8/12/15 kHz: transparentes
(fora da faixa).

**Validação:** `tests/test_output_stage.cpp` (novo, 9 funções — bypass
exato em 0, pega tom sustentado, escala com `bodyGuard`, ignora
transiente e ruído de banda larga, guarda ultrassônica dosada,
constante passa exata, determinismo). **46/46 CTest** Debug e Release
(era 45 — +1 alvo). `test_true_peak`/`test_mix` continuam passando
(margem fina do teste de 8 kHz preservada: guarda ultrassônica é
−0,1 dB ali). Os 5 renders de exemplo com hash `sha256` idêntico
(nenhum exemplo usa `MASTER`; a voz mínima do painel não dispara nada —
confirmado por render). README/RASGO_MODULAR.md/dossiê 17 atualizados.

**Não commitado.**

## Registro da etapa — 2026-09-05: Motion Engine janela uniforme, MASTER 50%, seed anti-viés-agudo, body guard até ~8 kHz

Rodada de correções a partir de vários reportes do autor em sequência
("o 6º slider do SEQUENCE se move muito mais", "ainda escuto frequência
hiper agudas", "elas se repetem em várias seeds", "melhore o algoritmo
da seed, ainda está engessado, com padrões de IA limitados", "mude o
master pra 50%").

**1. Motion Engine — janela de movimento uniforme.**
`apps/panel/MotionEngine.hpp` + `panel_main.cpp::populateMotion`. O bug:
a janela era o RANGE INTEIRO do parâmetro, então um `OSCILLATE` varria o
controle de ponta a ponta (o "6º slider" era o único animado do
SEQUENCE, e no modo Oscillate) enquanto um `WALK` mal saía do lugar.
Agora a janela é ±6% do range (`kMotionDepth`, IGUAL pra todo binding),
centrada no valor atual do knob (`Binding::start`, sentinela NaN =
compat com quem não seta → `examples/peca_generativa_4.cpp` e
`test_motion_engine` intocados, render byte-idêntico). Todo controle
animado "respira" a mesma fração proporcional.

**2. MASTER — default 30% → 50% do slider (−24 dB).**
`Master.hpp` + `PatchSeed.hpp` (fixo em todo seed). Era −38,4 (30%).

**3. Seed — viés de agudo e colapso de variedade.**
Sonda espectral (`seedspec`/`seedrms`) sobre 26 seeds achou: (a) a seção
"vozes soltas → mixer ch2/3/4" jogava `NOISE.white`/`.pink` CRUS direto
no MASTER a até −2 dB — o "hiper agudo que se repete em várias seeds"
(banda >10 kHz chegava a 11% da energia em vários seeds, sempre o mesmo
perfil); (b) ~15% dos seeds colapsavam num punhado de esqueletos quase
idênticos (`complexity` hard-clampado pra 0,02 → nProc=0, nCables=3).
`apps/panel/PatchSeed.hpp`:
- seção de camadas soltas reescrita: no máx 2, BEM baixas (−24..−11 dB),
  NUNCA saída de `NOISE` crua, prefere fontes já processadas
  (`SEED_SRC_BUS`);
- o passeio ponderado não usa mais `MIXER`/`MASTER` como destino (jogava
  fonte crua no barramento);
- `voiceOpts[3]` (voz de ruído) só rosa/marrom, nunca branco cru;
- rede de segurança: voz brilhante que não passou por processador nenhum
  ganha UM passa-baixa musical à força;
- `complexity` minimalista agora 0,05..0,20 (varia nProc/nCables) em vez
  de 0,02 fixo → sem colapso;
- voz OSC: ~35% em registro médio (era sempre grave), `pw` 0,25..0,75
  (não vira trem de picos), `freq` NOISE 1..500 (era 3000);
- voz FILTER auto-osc: `resonance` 0,55..0,9 (era 0,88..0,99, seno puro
  perfurante), corte-base variado e mais baixo (era travado em ~2,6 kHz);
- passe genérico de parâmetros: `resonance`/`grit`/`fold`/`drive` em
  sub-faixa (não vai pro extremo auto-oscilante), `gain` de EQ limitado
  a −9..+6, `cutoff` a 90..8000.
Resultado (26 seeds): banda >10 kHz de ~0..0,7% (era 11%+); nenhum par
de seeds idêntico; a maioria com espectro grave/médio-grave saudável.

**4. Body guard — faixa estendida a ~8 kHz.**
`OutputStage.hpp`: 3º detector (Q 3) em ~7,6 kHz (eram 2, em 2,8/4,8) —
os seeds brilhantes restantes concentram energia perto de 8 kHz, acima
da faixa antiga. `test_output_stage.cpp` ajustado (freqs de
transparência agora 1/11/14 kHz — 8 kHz agora é faixa protegida).

**Validação:** build limpo, **46/46 CTest** Debug e Release, 5 renders
de exemplo com hash `sha256` idêntico (nenhum exemplo usa `MASTER` nem
`seedPatch`; `peca_generativa_4` usa Motion Engine mas via API antiga
sem `start`). Sondas em `$CLAUDE_JOB_DIR/tmp` (não versionadas).

**Não commitado.**

## Registro da etapa — 2026-09-05: clipe nos patches/mutações + P6 do SEQUENCE ainda mexia + Motion só knob

Reportes: "em várias seeds há sons clipando", "algumas variações de
mutações e evolução também geram clipes", "ainda é o p6 no sequencer que
vai mais que os outros".

**Clipe.** Causa: o `MIXER` era soma LINEAR pura, sem teto; um patch
generativo quente (`gain1` do seed ia até +5 dB) ou uma mutação que
mexia num ganho de canal mandava +5..+10 dB pro `MASTER`, que aí
limitava demais / bombeava / distorcia.
- `src/dsp/Mixer.hpp` — joelho `tanh` no barramento: **transparente até
  1,0 (0 dBFS)** (os testes de soma linear continuam exatos), comprime
  de leve acima, teto ~1,35. Não é limitador (isso é do MASTER), é só
  não deixar a soma estourar seco;
- `apps/panel/PatchSeed.hpp` — `MIXER.gain1` de `rng(-2,+5)` pra
  `rng(-10,-2)` (mira ~−6 dBFS de barramento, com folga);
- `apps/panel/PatchGenetics.hpp::isMutationBlocked` — `gain1..4`,
  `level1`, `level2` entram na lista (mutar nível joga o barramento pra
  fora). Isso vale pro `MUTATE`/`EVOLVE`/`CROSS` **e** pra Motion Engine
  (usa a mesma função).

**Motion Engine — P6 e derivas arriscadas.**
`apps/panel/panel_main.cpp::populateMotion`:
- só `Widget::Kind::Knob` agora (era Knob+Slider). Slider no Rasgo é "a
  partitura" (passos do SEQUENCE) ou o fader do MIXER — não é regulagem
  que deva derivar sozinha. Some o "6º slider se move muito mais";
- `resonance`/`feedback`/`drive`/`grit`/`fold` fora da Motion — perto do
  extremo viram fuga/apito intermitente; derivar ali é arriscado.

**Body guard estendido a ~8 kHz** (3º detector Q3 em ~7,6 kHz) — os
seeds brilhantes restantes concentram energia perto de 8 kHz, acima da
faixa antiga (2,8/4,8 kHz). `test_output_stage.cpp` ajustado.

**Investigação de agudo residual:** sonda espectral em 26+ seeds — os
seeds mais brilhantes ainda existentes (ex.: 5, 1) têm um componente
~8 kHz, mas a −40..−47 dBFS (quase silêncio, nas passagens entre notas).
Não reproduzi um tom agudo ALTO e persistente. Isolei a cadeia de áudio
do seed 5 (`OSC.tri → PARAMETRIC → ENVELOPE`) — limpa, dominante em
125 Hz. Precisa do número do seed do autor pra fechar.

**Validação:** build limpo, **46/46 CTest** Debug e Release, 5 renders
de exemplo com hash `sha256` idêntico (nenhum exemplo usa `MIXER`/
`MASTER`/`seedPatch`/Motion via API nova).

**Não commitado.**

## Registro da etapa — 2026-09-05: MIXER + MASTER grudados no fim da família MIX

Pedido: "unir mixer e master pra ficarem próximos" → decidido (com o
autor) fazer **par grudado, sem unir os módulos** — mantém a
modularidade (MIXER sem MASTER, MASTER sozinho), zero refatoração.

`apps/panel/panel_main.cpp` — `sortFamilyForDisplay()` substitui o
`std::sort` alfabético cru nos dois pontos (rack inicial `shown` +
paleta): alfabético dentro da família, mas `MIXER` e `MASTER` recebem
chave `"~1"`/`"~2"` (o `~` vem depois de A..Z) → sempre no fim, nessa
ordem. Família MIX passa a exibir `MATRIX, NOTE-OUT, SCOPE, MIXER,
MASTER`. Só exibição — `moduleCatalog()` (índice de nó, doador do CROSS,
`seedClassifyPorts`) intocado.

**Validação:** build limpo, 46/46 CTest Debug e Release, 5 renders
`sha256` idênticos.

**Não commitado.**

## Registro da etapa — 2026-09-05: agudo = oscilador cru no mixer (seed 45932257) + botão SEED cortando

O autor identificou: "o som agudo vem do PLL, canal 2 no mixer" (seed
45932257) e "o número do seed no botão está cortando".

**Agudo — osciladores crus no barramento.** A seção "camadas soltas →
mixer" pegava qualquer fonte `SEED_SRC_BUS`/`VOICE` — inclusive
`PLL.out`, `OSC.saw`, etc. — e cabeava direto no MIXER. Com o passe
genérico sem teto de `freq`, o `PLL.freq` ia pra alguns kHz → um assobio
a −2..−14 dB no barramento. `apps/panel/PatchSeed.hpp`:
- camadas soltas agora **só de fontes JÁ PROCESSADAS**
  (`FILTER`/`WASP`/`LPG`/`SPACE`/`PARAMETRIC` — `isProcessedBus`); um
  oscilador cru direto no mixer é drone/assobio brigando com a voz
  (sugestão do próprio autor);
- passe genérico: `freq` de nó não-espinha cabeado limitado a
  40..1200 Hz; `rate` de faixa larga a ≤220 Hz;
- `QUANTIZER.range` de `rng(1, 3.5)` → `rng(1, 2)` (3,5 oitavas a partir
  de uma base de ~400 Hz levava o OSC da melodia a ~5–6 kHz);
- voz OSC "médio" de `rng(190, 520)` → `rng(175, 340)`;
- `DRIFT` targets: `QUANTIZER.range` (off 2,5 dep 2,0 → off 1,6 dep 0,7)
  e `FILTER.resonance` (off 0,4 dep 0,5 → off 0,35 dep 0,28, não deriva
  pra auto-oscilar);
- `PARAMETRIC` da cadeia: `q2` `rng(0.7, 5)` → `rng(0.6, 2.5)`, `gain2`
  `rng(-6, 10)` → `rng(-6, 6)` (Q 5 + +9 dB era pico-agulha que
  assobiava em cada nota).
Sonda `sweep` — **50 seeds espalhados: 0 ásperos** (nenhum com >4 kHz
dominando + audível), 1 mudo.

**Botão SEED.** `apps/panel/panel_main.cpp` — largura do botão agora
acompanha o rótulo (`seedButton()` mede os caracteres; 96..240 px);
número acima de 10 dígitos abrevia com "…" (`seedLabel()`). O clip da
faixa de status usa `seedButton()[0]` em vez do `-124` fixo.

**Validação:** build limpo, **46/46 CTest** Debug e Release, 5 renders
`sha256` idênticos (nenhum exemplo usa `MIXER`/`MASTER`/`seedPatch`).

**Não commitado.**

## Registro da etapa — 2026-09-05: LEARN — caixa fixa estilo terminal (modelo ANTITOTEM)

Pedido: "pro learn seguir o modelo do ANTITOTEM: uma caixa tipo terminal
no canto inferior esquerdo (coluna da esquerda) com o texto do learn …
para evitar que as caixas de texto se sobreponham ao painel". O tooltip
flutuante antigo tapava justo o módulo que você estava inspecionando.

**Feito:** `apps/panel/panel_main.cpp`
- `kLearnH` (196 px) — com o modo Learn (`[l]`) ligado, uma caixa fixa
  ocupa o rodapé da coluna esquerda; a lista de módulos (`palBottom`)
  encurta pra não sobrepor. Modo desligado = paleta ocupa a coluna toda;
- a caixa: borda + "LEARN" + regra + título (`TIPO · rótulo`) + os 3
  níveis empilhados (`quick` claro, `understand` secundário, `explore`
  com "→", cor de acento), com QUEBRA DE LINHA (`wrapText` mede com
  `Xutf8TextExtents`). Vazia → "passe o mouse sobre um knob ou jack";
- hit-test do widget sob o mouse hoisted pro topo do `redraw`
  (`learnHit`/`learnHitTitle`); o bloco do tooltip flutuante REMOVIDO;
- clamps de scroll da paleta e o hit-test de clique/roda usam `palBottom`.

**Validação:** compila limpo (`-Wall -Wextra -Werror`), **46/46 CTest**
Debug e Release, 5 renders `sha256` idênticos. Não testado visualmente
(o painel gráfico trava a máquina do autor por aqui — ele testa com
`.run_rasgo_modular.sh`).

**Não commitado.**

## Registro da etapa — 2026-09-05: caixa Learn menor + ligada por padrão; zoom do rack (Ctrl+=/-/0)

Pedidos, em sequência: "deixe a caixa do learn um pouquinho menor" · "ela
pode ficar sempre ligada" · "assim não precisa mais a mensagem no
cabeçalho 'modo aprender…'" ("isso fica pro tutorial") · "precisa ser
criado um modo redução/ampliação Ctrl+ / Ctrl- como fizemos no antitotem
— tá difícil de cabear os módulos da primeira linha com os da última".

**Caixa Learn** (`apps/panel/panel_main.cpp`)
- `kLearnH` 196 → **172 px**;
- `learnMode` agora **`true` por padrão** (como `motionOn`/`[v]`); `[l]`
  passou a ser só esconde/mostra (devolve ~170 px de paleta);
- removida a faixa de status "◆ MODO APRENDER — passe o mouse… · [l] pra
  sair" (não faz sentido com a caixa sempre presente; é assunto de
  tutorial). Legenda: "[l] aprender" → "[l] esconde/mostra a caixa
  aprender".

**Zoom de conteúdo do rack** (`apps/panel/panel_main.cpp`) — precedente
ANTITOTEM `ZoomableViewport`
- `kZoomMin/Max/Step` = 0,55 / 1,40 / 0,10; `uiZoom` multiplica `g_s`
  **depois** do `clamp[kSMin,kSMax]` do `relayout()`, com piso duro
  `kSMin·0,5` só nesse caminho (reduzir abaixo do regime compacto é o
  objetivo — ver a 1ª e a última fileira juntas pra cabear);
- `Ctrl+=` / `Ctrl++` / `Ctrl+KP_Add` ampliam · `Ctrl+-` / `Ctrl+KP_Sub`
  reduzem · `Ctrl+0` / `Ctrl+KP_0` volta a 100%;
- `scrollY` reescalado por `g_s_novo / g_s_velho` (âncora no topo);
- `setTitle("… zoom NN%")` de retorno; legenda ganhou "[Ctrl+=/-/0] zoom";
- `uiZoom == 1.0` desenha **idêntico** ao anterior (multiplicador no-op).
- Limite conhecido: o fontset X11 é de tamanho fixo — em zoom-out forte as
  legendas apertam. É vista de sobrevoo pra rotear, não de ajuste fino;
  declutter por limiar de `g_s` fica como refino aberto.

Docs: `README.md` (hover-learn), `apps/panel/design.md` §2.10 reescrita +
§3.2 nova subseção "Zoom de conteúdo",
`dossies/ESTUDO_seed_composicao_generativa.md` §6.

**Validação:** `rasgo_modular_panel` compila limpo (`-Wall -Wextra
-Werror`), **46/46 CTest** Debug e Release. Não testado visualmente (o
painel trava a máquina do autor por aqui; ele roda `.run_rasgo_modular.sh`).

**Não commitado.**

## Registro da etapa — 2026-09-05: encerramento à prova de trava (janela que "não fechava")

Sintoma relatado: "há um instrumento travado na tela / não fecha / dê o
kill por aí". Matei o processo (`SIGKILL`, `ps -C` confirmou); a sessão
tinha sido salva (`session.rmp` 22:25:56, seed 134440676, com nó
AUDIO-IN). "Não sei o que fiz."

Diagnóstico: **nenhum laço do painel é infinito** — main loop, thread de
áudio e thread de captura têm iteração limitada e `try_lock` não
bloqueante. O risco era só o **teardown de saída bloqueante**:
- `~AlsaSink` fazia `snd_pcm_drain` (espera o buffer tocar até o fim);
- `stopAudioIn()` → `audioInThread.join()` só volta quando o
  `snd_pcm_readi` do período corrente retorna — se o PipeWire parou de
  entregar, pendura;
- ambos rodam ANTES do `XDestroyWindow` → janela fica na tela.
(A máquina ainda está com o stack de vídeo nvidia cuspindo erro de EDID a
cada 10 s — GUI instável por fora do nosso código também.)

**Feito** (`apps/panel/`)
- `panel_main.cpp`: `XUnmapWindow` + `XFlush` **logo que o loop sai**,
  antes de qualquer desmonte de áudio — a janela some na hora;
- `AlsaSink.hpp` `~AlsaSink`: `snd_pcm_drain` → **`snd_pcm_drop`**;
- `AlsaSource.hpp`: novo `abort()` (`snd_pcm_drop`);
- `panel_main.cpp` `stopAudioIn()`: `audioInDev->abort()` antes do
  `join()` — corta um `read()` bloqueado.
Sem mudança de comportamento com ALSA saudável.

Docs: `apps/panel/design.md` §3.4.

**Validação:** `rasgo_modular_panel` compila limpo, **46/46 CTest**. Não
dá pra validar a saída rodando o painel aqui — precisa da verificação do
autor (abrir e fechar com `q` e pelo botão da janela).

**Não commitado.**

## Registro da etapa — 2026-09-05: caixa Learn sem toggle (tecla [l] removida)

Pedido: "não precisamos do botão learn, ele estará sempre habilitado …
a tecla l pode sumir" (no contexto do redesenho do cabeçalho — a caixa
Learn não vira botão da barra).

**Feito** (`apps/panel/panel_main.cpp`): removidos o estado `learnMode`,
o handler `XK_l`/`XK_L` e a entrada `[l]` da legenda. A caixa LEARN e o
hit-test do widget sob o mouse são incondicionais; `palBottom`/`palBot`
sempre reservam `kLearnH` (172 px) no rodapé da paleta.

Docs: `README.md`, `apps/panel/design.md` §2.10 + §roadmap 1c,
`dossies/ESTUDO_seed_composicao_generativa.md` §6.

**Validação:** compila limpo, **46/46 CTest**.

**Não commitado.**

---

## Registro da etapa — 2026-09-05: cabeçalho de linha única (modelo RASGO Synth) + i18n do painel

Pedidos, em sequência: cabeçalho seguindo o modelo dos RASGO Synth mas em
**uma linha**, altura mantida (`kCaseTop = 46`) · a legenda de texto vira
**botõezinhos** toggle/momentâneo · sem botão `LEARN` (sempre ligado, tecla
`[l]` removida) · IDIOMA/TUTORIAL/SOBRE **funcionais já** (opção B) · i18n
traduz **cabeçalho, tutorial, créditos, LEARN**; **não** rótulos de
parâmetro nem títulos de módulo · **inglês é o padrão** · LEARN traduzido
**em fases** (infra agora, textos depois, começando pelo rack de partida).

**Feito**
- `apps/panel/UiLanguage.hpp` (NOVO) — porte sem JUCE do `UiLanguage.h`
  dos Synth: `enum Lang{en,pt,fr,es}`, `L4{en;pt;fr;es}`, `tr()` com
  fallback en→pt, `nextLang`/`langLabel`/`langCode`/`langFromCode`,
  `namespace strings` (cabeçalho + 6 cartões de tutorial + créditos, 4
  línguas). `tests/test_ui_language.cpp` (NOVO): fallback, ciclo,
  completude do cabeçalho. **CMake 46→47 alvos.**
- `apps/panel/panel_main.cpp`:
  - `uiLang` (padrão `en`) carregado do pref próprio
    `~/.local/share/rasgo-modular/ui-lang` (não o patch — sobrevive a
    abrir num seed novo); `saveLangPref()` no `cycleLang`;
  - **ações hoisted** — `actSeed/actMotion/actMute/actMutate/actEvolve/`
    `actCross/actBank/actSave/actRec/actZoom(dir)/cycleLang`: uma
    implementação por comando, chamada pela tecla E pelo clique. O switch
    de `KeyPress` encolheu de ~150 linhas pra ~40;
  - **cabeçalho reescrito** (`redraw`): wordmark `RASGO MODULAR` (marca,
    não traduz) · barra de comandos em botões (`DRIFT`/`MUTE` toggles
    acendem âmbar; `MUTATE`/`EVOLVE`/`CROSS`/`BANK`/`SAVE` momentâneos
    piscam ~160 ms via `hdrFlash`; `ZOOM −/+`) · cluster da direita
    montado da borda pra dentro: `SOBRE` `TUTORIAL` `IDIOMA(EN…)`
    `● REC`(vermelho gravando) `⚄ SEED n` · pico do MASTER (barra+dB) ·
    `N mód · M cabos`. Sacrifício por largura: leitura → pico → comandos
    da direita; SEED/REC/IDIOMA/TUTORIAL/SOBRE nunca somem. `headerHits`
    (montado no `redraw`, lido no laço de evento) roteia o clique;
  - **overlay** `tutorial`/`sobre` — card sobreposto, `[Esc]` ou qualquer
    clique fecha; tutorial com 6 cartões;
  - removidos o `seedButton()` (a geometria virou parte do cluster) e a
    faixa de legenda/`● GRAVANDO` (estado agora é o próprio botão).
- Docs: `README.md`, `RASGO_MODULAR.md` (47 alvos), `apps/panel/design.md`
  §2.11 (nova).

**Escopo declarado / fase seguinte:** traduzir o `LearnCatalog.hpp` (EN
canônico + FR + ES) — hoje 100% pt, ~233 textos. `LearnEntry` vira
`L4`-por-campo quando essa tradução começar; até lá o LEARN mostra o pt.

**Validação:** `rasgo_modular_panel` + `rasgo_modular_ui_language_tests`
compilam limpos (`-Wall -Wextra -Werror`), **47/47 CTest** Debug e
Release. Não testado visualmente (o painel trava a máquina do autor por
aqui; ele roda `.run_rasgo_modular.sh`).

**Não commitado.**

## Registro da etapa — 2026-09-05: ajustes pós-cabeçalho (dwell do LEARN · MUTE isolado · mute no MASTER · AUDIO-IN mais fino)

Pedidos avulsos durante a revisão do cabeçalho:

- **Dwell da caixa LEARN** — "fica mudando o tempo todo quando mexemos o
  mouse". `panel_main.cpp`: o conteúdo só troca depois de ~2 s parado
  sobre o MESMO objeto (`learnHoverKey`=`id|bind` + `learnHoverSince`);
  fora de qualquer widget mantém o último. O laço já repinta a ~30 fps,
  o dwell resolve sozinho.
- **Botão `MUTE` isolado no cabeçalho** — "pra evitar clicar nele sem
  querer e mutar o som". Saiu do bloco de comandos; agora fica no fim da
  barra, depois do `ZOOM` + um vão de 16 px + régua vertical + 12 px.
- **`mute` no módulo MASTER** — `src/dsp/Master.hpp`: novo param `mute`
  (0/1, default 0), rampa de ~8 ms (1 polo) no fader antes da proteção de
  saída — silêncio sem estalo. Toggle `MUTE` no painel (x24 y42, os
  outros toggles desceram um passo). `tests/test_mix.cpp::testMasterMute`
  (rampa não corta seco · silencia os 2 canais em ~80 ms · volta ao
  desligar). Dossiê `17_master.md` atualizado.
- **`AUDIO-IN` 6 → 4 HP** — "muito largo". `src/dsp/AudioIn.hpp`: só tem
  1 knob e 1 jack; display 20 → 16 mm. Era o menor módulo (6); agora 4.
  `test_panel_layout` (que já cobre AUDIO-IN) passa.

**Em aberto (propostas, aguardando o autor):**
- renomear o botão `DRIFT`/`DERIVA` do cabeçalho → há um módulo `DRIFT` no
  rack; o botão é o Motion Engine (`[v]`, "variação ao vivo"). `VARIA` /
  `VARY` evita a colisão;
- o `MUTE` do cabeçalho hoje rompe TODOS os cabos (`allRuptured`, = tecla
  `[espaço]`). Podia em vez disso acionar o `mute` do(s) MASTER — mais
  limpo pra "mutar o som"; `[espaço]` fica só com o rompe-cabos;
- osciloscópio no painel do MASTER (além do VU) — `Display` de forma de
  onda ao lado do VU, ou um 2º `Display`.

**Validação:** build limpo (`-Wall -Wextra -Werror`), **47/47 CTest**
Debug e Release. Painel não testado visualmente (trava a máquina do
autor; ele roda `.run_rasgo_modular.sh`).

**Não commitado.**

## Registro da etapa — 2026-09-06: rótulos do cabeçalho, STANDBY, logo, e início do passe de ergonomia de painel

**Cabeçalho** (`apps/panel/`)
- `DERIVA` → **`VARIA`** (colidia com o módulo `DRIFT`). `MUTA` → **`MUDA`**
  ("muta = interrompe o som", não é o sentido do MUTATE) — pt/es 3ª pessoa,
  a tecla `[m]` e o título continuam "MUTATE".
- o botão `MUTE`/`MUDO` do cabeçalho virou **`STANDBY`**, isolado no fim
  da barra (vão + régua), e agora aciona o **`mute` do MASTER** (silêncio
  limpo, rampa) em vez de romper cabos. `[espaço]` continua com o
  rompe-tudo (`actRupture`). Estado compartilhado com o toggle MUTE do
  módulo (`masterMuted` lido no `redraw`).
- **toggles ligados ganham anel de destaque** — `VARIA`/`STANDBY`/`REC`
  (o `REC` em vermelho); distinto do flash momentâneo de `MUDA`/`EVOLUI`/…
  (`hdrBtnC(..., onCol)`).
- **logo RASGO** no wordmark: `apps/panel/assets/rasgo_logo_2026.svg`
  (cópia da família, de `rasgo-synth-performance/assets/`) →
  `rasgo_logo.xbm` (1-bit, `regen_logo.sh`, commitado) → `XCopyPlane` na
  cor de acento + "MODULAR" ao lado.

**Passe de ergonomia de painel — rubrica + 3 primeiros** (`src/dsp/`)
- Rubrica documentada em `apps/panel/design.md §3.2.1` (margens, display
  cheio, grade de knobs centrada por HP, jacks espaçados pelo rótulo /
  quebra em fileiras por função, HP enxuto, agrupamento didático).
- `footprintMM` do painel + `test_panel_layout` (kCharMM 2,5→2,8, folga
  0,5→0,3) agora modelam a **largura do rótulo do jack** — o modelo
  frouxo deixava "FLD/EVT", "XOR/FLIP" etc. passarem.
- **DRIFT** (10 HP): display cheio, knobs 2 col, **3 fileiras de jacks** —
  ADV/RATE in · A/B/C/D (saídas de campo correlacionadas) · FLD/EVT.
- **LOGIC** (10 HP): display cheio, saídas em **2 fileiras** —
  combinacional AND/OR/XOR · derivadas do clock DIV/FLIP.
- **FILTER** (10 HP, comentário dizia "12" mas era 10): display cheio,
  knobs 2×2 centrados, IN + 3 mod-CV numa fileira, LO/CTR/HI/ALL na de
  baixo.
- Dossiês `02_filtro.md`, `22_logic.md`, `27_drift.md` atualizados.
- **Pendentes** (o teste marca `LAYOUT (pendente)`, não falha):
  `FUNCTION` (RATE/SLOPE/SYNC), `HARMONY` (ROOT/SCALE), `MIXER` (M×1..4),
  `NOTE-OUT` (GATE/PITCH) + varredura do resto do catálogo. Aguardando OK
  do padrão pra seguir.

**Validação:** build limpo (`-Wall -Wextra -Werror`), **47/47 CTest**
Debug e Release. Painel não testado visualmente (trava a máquina do
autor; ele roda `.run_rasgo_modular.sh`).

**Não commitado.**

## Registro da etapa — 2026-09-06: logo RASGO anti-aliased (estava serrilhada)

O `.xbm` 1-bit deixava a marca serrilhada no tamanho do cabeçalho. Trocado
por um **mapa de cobertura em tons de cinza** (`assets/rasgo_logo_gray.h`,
gerado do SVG por `regen_logo.sh` com anti-aliasing) que o painel
**pré-compõe uma vez** num `Pixmap` — mistura fundo→acento por pixel — e
depois só faz `XCopyArea` a cada quadro. Sem dependência de imagem no
build (o `.h` é commitado). `rasgo_logo.xbm` removido.

**Validação:** `rasgo_modular_panel` compila limpo Debug e Release,
**47/47 CTest**.

**Não commitado.**

## Registro da etapa — 2026-09-06: variação ao vivo (VARIA) mais viva — ±20% + sliders de volta

Relato: "antes os knobs e sliders variavam mais, sem precisar clicar a
cada vez num botão". A correção do "6º slider" (2026-09-05) tinha deixado
a janela em ±6% e só-knob — ficou imperceptível.

**Feito** (`apps/panel/panel_main.cpp`, `populateMotion`)
- `kMotionDepth` 0,12 → **0,40** (±20% do range, uniforme pra todo binding);
- `b.rateHz` 0,015–0,06 → **0,03–0,12 Hz** (alvo novo a cada ~8–33 s);
- **KNOB + SLIDER** elegíveis de novo — mesma janela uniforme (o vício
  era a janela não-uniforme, não o slider). Inclui os passos do
  `SEQUENCE` (melodia deriva de leve). `MIXER gain1..4` / `MASTER gain` /
  `out_gain` seguem bloqueados (`isMutationBlocked`).

`VARIA` (ex-`[v]`) continua ligada por padrão. Doc: `ESTUDO §3.6`.

**Validação:** build limpo, **47/47 CTest**. A afinação vai pelo ouvido do
autor — se ±20% for demais/de menos, é um número.

**Não commitado.**

## Registro da etapa — 2026-09-06: "6º slider do SEQUENCE" — bug de fundo na Motion Engine

O autor apontou que o `p6` do SEQUENCE voltou a "mexer mais que os demais"
e suspeitou de bug de código. **Era bug mesmo:** `populateMotion` escolhe
QUAL controle de cada módulo anima por um hash de `tipo + id do nó`. Como
`PatchSeed` instancia o catálogo INTEIRO pra todo patch, o `id` de cada
módulo é fixo → o hash é constante → **sempre o mesmo controle anima**, em
todo seed. Pro SEQUENCE o hash caía sempre no mesmo passo, e um passo
derivando = melodia reafinada.

**Feito** (`apps/panel/panel_main.cpp`, `populateMotion`)
- o hash passa a misturar **`curSeed`** (declaração de `curSeed` subiu pra
  perto de `motionOn`) — cada patch respira por um controle diferente;
  editado à mão (`curSeed == 0`) fica estável;
- **`SEQUENCE`/`TRIGSEQ`/`TURING`/`HARMONY`/`QUANTIZER` fora da Motion
  Engine** — a camada mexe em timbre/textura, nunca nas notas;
- **de volta a só KNOB** (revertido o slider de +2026-09-06 09:xx — os
  sliders são a partitura ou fader de nível).
- guard no `XImage` da logo (fallback se `XCreateImage`/`malloc` falhar).

Doc: `ESTUDO §3.6`.

**Validação:** build limpo, **47/47 CTest**, 5 renders de exemplo geram
checksum estável (só código do painel mudou; `peca_generativa_4` usa
`MotionEngine` direto).

**Não commitado.**

## Registro da etapa — 2026-09-06: roadmap de continuidade (PESQUISA §2.4)

Pedido: "verifique quais módulos da lista de interessantes não
contemplamos ainda" + "uma boa lista para darmos continuidade".

Cruzei os 38 módulos feitos com `PESQUISA_MODULOS.md` (§2.2 aberto, §6
Polivoks, §7 Top-100 ★, §4 Aquorbium, §5 técnicas). Escrito em
**`PESQUISA_MODULOS.md §2.4`** — 4 ondas, do completador ao arriscado:

- **A** (baixo risco, fecha o rack): `GLIDE` (portamento por nota — o
  primitivo que falta pro acid), `WAVETABLE` (oscilador de tabela — EMW
  WAVE-6, hardware do autor), `LOOPER` (delay HOLD/REVERSE/fita) +
  `peca_generativa_5`;
- **B** (territórios novos): `ADDITIVE` (Odessa), `PLANAR` (morph vetorial
  XY), `OPERATOR` (FM multi-op — Akemie's), `FORMANT` (Fumana/SMR);
- **C** (espaço/caráter): reverb FDN (modo do `SPACE` ou `HALL`), `DRUM`
  (empacota MATTER+NOISE+ENVELOPE);
- **D** (grande/opt-in): `SAMPLER`/`TAPE`/`TURNTABLE`
  (`ESTUDO_audio_sampling.md`), adaptadores `MIDI`/`CV`.

Fora de onda: pulsar (modo do ADDITIVE?), LFO múltiplo (modo do FUNCTION?),
keyframes de estado (feature de painel), plataforma polimórfica (adiado).

**Antes da Onda A:** conferir wishlist/rack ModularGrid do autor (§9).
`dossies/00_indice.md` atualizado (38 feitos + ponteiro pra §2.4; texto
de estudos à parte destravado — `CROSS`/`MUSICAL SCORE`/SeedGrammar/LEARN
estão feitos).

**Docs só. Não commitado ainda.**

## Registro da etapa — 2026-09-06: Módulo 39 — GLIDE (Onda A do roadmap)

Pedido: "avance os outros itens" (o roadmap §2.4). Onda A, #39.

**`GLIDE` — portamento por nota** (`src/dsp/Glide.hpp`, `dossies/39_glide.md`,
`tests/test_glide.cpp` — 10 funções). Família TRANSFORM/PITCH.

O `CONTROL.slew` é lag RC sempre ligado — não serve pra condução de
melodia. `GLIDE` decide por nota se escorrega ou salta:
- `mode` 0 sempre · 1 slide-gated (o *slide* do TB-303: `slide` alto
  habilita) · 2 legato (`gate` sustentado desliza, borda de subida salta);
- `time` de subida (0–2 s) + `fall` (−1..1: descida = `time·6^fall`);
- `curve` linear (rate constante, MS-20/Minimoog) ↔ exponencial (RC);
- saídas `moving` (gate) e `done` (pulso ~2 ms na chegada).
Sem `drift` (é régua de afinação). Determinístico, sem alocação.

Integrado: `ModuleCatalog` (família TRANSFORM, entre SH e PARAMETRIC),
`test_panel_layout`, `LearnCatalog` (11 binds). Docs: `00_indice.md`,
`PESQUISA §2.4` (marcado feito), `RASGO_MODULAR.md §36.3`, `README.md`.
Também anotado em §2.4: `SIGNAL-IN` (evolução do `AUDIO-IN` agregando
MIDI-IN/CV-IN num só, decisão do autor) e que o autor não tem conta
ModularGrid (o §9 estava desatualizado).

**Validação:** build limpo (`-Wall -Wextra -Werror`), **48/48 CTest**
Debug e Release. Renders de exemplo estáveis (GLIDE não entra em nenhum).

**Não commitado.**

## Registro da etapa — 2026-09-06: Módulo 40 — WAVETABLE (Onda A)

`PESQUISA §2.4` Onda A, #40. Decisão do autor: opção **A** (tabelas
procedurais, sem arquivo de dados).

**`WAVETABLE` — oscilador de tabela procedural** (`src/dsp/Wavetable.hpp`,
`dossies/40_wavetable.md`, `tests/test_wavetable.cpp` — 10 funções, DFT
ponto a ponto). Família SOURCE.

- 16 quadros gerados no `prepare()` por receita espectral fixa
  (serra→quadrada→formante→seno); 10 mip-maps band-limited (maxH 512..1)
  escolhidos pela fundamental → antialiasing por construção. ~1 MB/inst.
- `pos` (+ CV + `drift`) varre a forma; `warp` = distorção de fase Casio
  CZ / "WAVE CUT" do EMW WAVE-6; `freq`/`fine`/1V-oct/`fm` linear.
- **Captura ao vivo** (resposta a "se houver fonte no AUDIO-IN…"):
  `capture` (áudio) + `grab` (trigger) → 1024 amostras (DC removido +
  normalizadas) viram o quadro do topo de `pos` (crossfade ~0,9–1,0).
  Standalone = procedural (determinístico com `drift=0`); com
  `AUDIO-IN`/`OSC`/… no `capture` = tabela viva. Quadro capturado sem
  band-limit (aliasing em afinação alta = caráter aceito, anotado).

Geração das tabelas: sinLUT indexada por `(h·s) mod kLen` + snapshot
band-limited nos `h` potência de 2 → `kFrames·kLen·kMaxH` ≈ 8 M ops no
`prepare` (não RT).

Integrado: `ModuleCatalog` (SOURCE, após OSC), `test_panel_layout`,
`LearnCatalog` (13 binds). Docs: `00_indice`, `PESQUISA §2.4`,
`RASGO_MODULAR.md §36.3`, `README.md`.

**Validação:** build limpo (`-Wall -Wextra -Werror`), **49/49 CTest**
Debug e Release. Renders de exemplo estáveis (WAVETABLE não entra em
nenhum).

Commitado (`e675fda`).

## Registro da etapa — 2026-09-06: Módulo 41 — LOOPER (Onda A — completa)

`PESQUISA §2.4` Onda A, #41 — o último da onda.

**`LOOPER` — delay de linha com HOLD / REVERSE / fita** (`src/dsp/Looper.hpp`,
`dossies/41_looper.md`, `tests/test_looper.cpp` — 8 funções). Família SPACE.

- Delay de linha (buffer circular ~2,2 s, pré-alocado no `prepare()`;
  `process()` não aloca). Distinto do `SPACE` (reverb) e do `MEMORY`
  (granular): aqui é a LINHA de atraso com os três gestos do `§6`.
- `hold` (toggle ou gate `freeze`): na borda de subida ancora a janela
  na posição de escrita e a repete infinito — para de escrever, sem
  realimentação nova. `reverse` (toggle ou gate `rev`): dois grãos Hann
  defasados meia volta em crossfade — vira o buffer sem clique.
- `age` = caráter de fita/BBD num knob: passa-baixa dentro do laço
  escurece a cada volta + wow&flutter (~0,9 e ~6,5 Hz) + `tanh` de
  compressão + chiado semeado (determinístico — reprodutível).
- `feedback` até 1,1 auto-oscila; `time` suavizada; `mix` seco↔molhado;
  saídas `out` (misturado) e `wet` (só o laço). `age=0` sem
  `hold`/`reverse` → delay digital limpo.

Bug encontrado e corrigido durante os testes: `prepare()` inicializava
`dSmooth_` com `0.3f * sr_` fixo em vez de ler o parâmetro `time` —
o tempo de atraso demorava demais a convergir. Corrigido para
`clampf(parameterValue("time") * sr_, 4, n-4)`.

Integrado: `ModuleCatalog` (SPACE, após SPACE), `test_panel_layout`,
`LearnCatalog` (12 binds). Docs: `00_indice`, `PESQUISA §2.4`,
`RASGO_MODULAR.md §36.3`, `README.md`.

**Validação:** build limpo (`-Wall -Wextra -Werror`), **50/50 CTest**
Debug e Release. Renders de exemplo estáveis (LOOPER não entra em
nenhum).

**Onda A completa** (GLIDE, WAVETABLE, LOOPER). Próxima: Onda B
(ADDITIVE / PLANAR / OPERATOR / FORMANT) — aguarda direção do autor.

## Registro da etapa — 2026-09-06: ESTUDO_audio_sampling revisto — Navalha 2 como prior art

Pergunta do autor: *"chegou a verificar no Navalha 2 o que temos já de
conceitos e códigos para o trabalho de sampler e afins?"* — não; o
`dossies/ESTUDO_audio_sampling.md` (do pedido de 2026-09-02) citava só
referências genéricas (Akai, Clouds, Space Echo) e **não mencionava o
Navalha 2**. Varredura feita agora nos dois: `RASGO/NAVALHA2_PD`
(referência Pd/web v0.28.1) e `RASGO/NAVALHA2_JUCE` (reescrita C++).
**Nenhum arquivo do Navalha 2 tocado — só leitura.**

Achados incorporados ao estudo (nova §2):

- **Licença:** GPL-3.0-or-later (Glerm Soares autorizou 07/2026),
  compatível com o default AGPLv3-or-later da família. Porte exige
  crédito nominal a Glerm Soares + Lúcio Araújo + nota do
  `G09.pitchshift.pd` (Puckette). Não é caso de "conceito→origem
  pública→desvio" — código livre, dá pra portar de fato.
- **Conceito já articulado:** "matéria gravada pra corte/fragmentação/
  recombinação, não DJ" (`CONCEPT_DECONSTRUCTION`, `DUAL_MATERIAL`);
  vocabulário GAP/STUTTER/BURST/MICROSLICE/MEMORY/MUTATION/EROSION/
  DECONSTRUCT.
- **Código C++ portável** (`NAVALHA2_JUCE/src/core/`): `SlicePlayer`
  (~240 ln — voz de sampler quase pronta: varispeed, reverse, envelope
  A/R, de-click adaptativo, stop-fade), `HeritagePitch`/`LegacyPitchChannel`
  (~117 ln — pitch-shift do G09 com interpolação `vd~` de 4 pontos),
  `SliceBank` (region/BLADE/`appendMicroSlices`), orquestração
  round-robin 2 vozes/fonte com crossfade, source mixer (level/pan/
  width mid-side/mute/solo).
- **Abstrações Pd** (`NAVALHA2_PD/core/`): `navalha_player`,
  `navalha_reverse_reader~`, `navalha_pitchshift_legacy~`,
  `navalha_voice~`, `navalha_source_mixer~`.
- **Loop de re-alimentação** (`RESAMPLE_FEEDBACK_LOOP`): performance
  gravada vira SOURCE — mesmo espírito da captura ao vivo do `WAVETABLE`.

Também revisto no estudo: §4.1 `SAMPLER` agora aponta o porte concreto
(não mais "o mesmo grão do MEMORY"); §4.3 `TAPE` marcado como
sobreposto ao `LOOPER` (#41) — só vira módulo se for além; §5 ordem
inclui o passo de portar `SlicePlayer`+`HeritagePitch`.

Ponteiros atualizados: `00_indice.md` (linha do estudo), `PESQUISA §2.2`
(linha SAMPLER/TURNTABLE/TAPE) e `PESQUISA §2.4` Onda D #48.

**Só documentação — sem código, sem build.**

## Registro da etapa — 2026-09-06: Módulo 42 — ADDITIVE (Onda B — 1/4)

`PESQUISA §2.4` Onda B, #42 — primeiro dos quatro (após "avance").

**`ADDITIVE` — oscilador aditivo / espectral** (`src/dsp/Additive.hpp`,
`tests/test_additive.cpp` — 13 funções, DFT ponto a ponto). Família
SOURCE. O timbre CONSTRUÍDO parcial a parcial — o oposto do subtrativo
do `OSC` e do eixo-de-forma do `WAVETABLE`.

- **64 parciais** somados por acumuladores de fase + LUT de seno de 2048
  (sem `std::sin` no laço). Razões/amplitudes recalculadas 1× por bloco
  (params mudam devagar, com suavização de 1 polo pra não zipar na
  borda de bloco); o laço por amostra só acumula.
- **Corte de Nyquist por parcial:** o `k`-ésimo entra só enquanto
  `f0·razão_k < 0,98·Nyquist`, com fade nos últimos 15 % pra não sumir
  com clique quando a afinação varre.
- Envelope espectral por 4 knobs (todos com CV): `tilt` (brilho,
  `a_k = k^-e`, `e` de 2,6 a 0,15), `odd` (−1..1, ímpar/par: quadrada ↔
  oco), `stretch` (−1..1, inarmonicidade `razão_k = k + s·0,004·k(k−1)`
  — linearizada de propósito pra ficar monotônica em `k` e o corte por
  `break` valer), `comb` (0–1, pente `cos` sobre o índice: até 12
  dentes).
- `drift` (0–1) = cintilância **determinística**: micro-desafino
  (±0,6 %) + respiração de amplitude (±12 %) de senóides lentas (~0,05 e
  ~0,035 Hz) defasadas por `k` no ângulo áureo. Sem RNG — soma de
  senóides incomensuráveis (percurso não-repetitivo, render
  reprodutível). `drift=0` remove o termo.
- **Segurança de saída:** seguidor de ganho de 1 polo (rápido ↓ / lento
  ↑) mira `0,9/pico_do_bloco` + `tanh` no fim. Saída sempre em
  ~[−0,95; 0,95].

Determinístico sempre. `prepare()` aloca ~8 KB; `process()` não aloca.

Integrado: `ModuleCatalog` (SOURCE, após WAVETABLE), `test_panel_layout`
(12 HP, layout espelha o `WAVETABLE`), `LearnCatalog` (13 binds). Docs:
`00_indice`, `PESQUISA §2.4`, `RASGO_MODULAR.md §36.3`, `README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**51/51 CTest** Debug e Release. Testes cobrem: série harmônica
decrescente; `tilt` achata o espectro (harmônica 8 relativa ×3+);
`odd=±1` zera pares/ímpares (<5 %); `stretch=1` move a parcial 16 pra
cima de 16,5·f0; `comb=1` cava a parcial 8 (<15 %); 1 V/oct; sem
componente acima de Nyquist audível em `freq` alto; `fm` linear;
determinismo byte a byte com `drift>0`; limites nos extremos; soa
sozinho. Renders de exemplo estáveis (ADDITIVE não entra em nenhum).

**Não commitado ainda** → commit a seguir.

**Onda B:** falta `PLANAR` (#43), `OPERATOR` (#44), `FORMANT` (#45).

## Registro da etapa — 2026-09-06: Módulo 43 — PLANAR (Onda B — 2/4)

`PESQUISA §2.4` Onda B, #43 (após "avance").

**`PLANAR` — morph vetorial XY** (`src/dsp/Planar.hpp`,
`tests/test_planar.cpp` — 12 funções). Família MIX (morph). O que o
`MIXER` faz numa linha, num plano: 4 fontes de áudio nos cantos de um
quadrado, um ponto `x`/`y` interpola por peso bilinear.

- **`curve`** linear (pesos somam 1 — morph honesto de CV) ↔ potência
  constante (`1/√Σw²` — áudio não afunda ~6 dB no centro).
- **`smooth`** = glide de 1 polo no ponto (τ de ~0 a ~0,5 s);
  **`rate`** (0,5 = 1×, `2^((r−0,5)·4)`) = velocidade do loop do gesto
  E da deriva.
- **GESTO:** gate `gesture` alto grava a trajetória do ponto (decimada
  32×, buffer ~4 s); na descida ≥ 3 quadros → toca em loop (knob
  ignorado, CV vira nudge); toque curto = limpa.
- **Desvio Rasgo:** a posição efetiva (suavizada) SAI em `x_out`/`y_out`
  como CV — o gesto desenhado num `PLANAR` de áudio dirige `cutoff`,
  `pos`, outro `PLANAR`… "a relação é o processo".
- **`drift`** = passeio 2D determinístico (Lissajous de 3 senos
  incomensuráveis, sem RNG — não fecha em < ~6 min).
- **Saída:** morph linear de fontes ≤ 1 é combinação convexa → bit-exato,
  nunca passa de |1|. Só a potência constante estoura (~2× no centro);
  aí `softclip` (`tanh` acima de |1|, assíntota ±1,5).

Determinístico sempre. `prepare()` aloca ~48 KB (2 buffers de gesto);
`process()` não aloca.

**Bugs pegos nos testes:** (1) `τ` mínimo do smooth era 1 ms — subi pra
60 µs pra `smooth=0` ser realmente instantâneo; (2) `softclip` prendia
em 1,0f exato com 4 fontes de amplitude 1 em fase — retopologizado pra
transparente até |1| e assíntota ±1,5 (o morph linear nunca precisa de
clip); (3) nudge do knob durante playback deslocava o gesto inteiro —
agora o knob é ignorado com gesto tocando, só a CV dá nudge.

Integrado: `ModuleCatalog` (MIX, após MATRIX), `test_panel_layout`
(12 HP), `LearnCatalog` (16 binds). Docs: `00_indice`, `PESQUISA §2.4`,
`RASGO_MODULAR.md §36.3`, `README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**52/52 CTest** Debug e Release. Testes: canto puro → só A; centro
`curve=0` → −6 dB, `curve=1` → ~0 dB (potência constante); varredura
A→B linear; `smooth` transforma degrau em rampa e chega; deriva move /
para com `drift`; grava um gesto e `x_out` reproduz em loop; toque curto
não deixa gesto; `x_out`/`y_out` seguem o ponto; determinismo byte a
byte com `drift` + gesto; limites; soa sozinho. Renders de exemplo
estáveis (PLANAR não entra em nenhum).

**Não commitado ainda** → commit a seguir.

**Onda B:** falta `OPERATOR` (#44), `FORMANT` (#45).

## Registro da etapa — 2026-09-06: taxonomia consolidada (18 verbos → 8 famílias)

Pergunta do autor: a classificação de famílias veio do Hexen, mas já
temos mais módulos que o Hexen — "verificar na literatura dos euroracks
como eles costumam classificar". Autor escolheu **fazer a consolidação**
(reordenar `moduleCatalog()`, atualizar §3/§4/§36.3, revalidar seeds/CROSS).

**Pesquisa** (`WebSearch` + `WebFetch` ModularGrid):
- **ModularGrid** — lista PLANA de ~60 *function tags* (1ª + 2ª por
  módulo), por tecnologia/comportamento (Oscillator/Filter/VCA/LFO/
  Sequencer/Quantizer/Logic/Switch/Multiple/Mixer/Looper/Low Pass Gate/
  Resonator/…). Sem tag "Additive" nem "Physical Modeling".
- **Patch & Tweak** (Kim Bjørn) — Sound Sources / Audio Modifiers /
  Modulation & CV / Rhythm-Sequencing / Utilities / Effects.
- **Doepfer A-100** — Sound Sources / Sound Modifiers / Modulation &
  Controllers / Utilities / Clock-Trigger / Effects.

**Diagnóstico:** a árvore de 18 verbos tinha ~5 famílias-fantasma
(DAMAGE/REPAIR/RELATION/INFERENCE/PERCEPTION, 0–1 módulo); a paleta já
tinha colapsado pra 9 grupos ad-hoc (SWITCH em SEQUENCE, MATRIX/MULT em
MIX, sem ROUTE).

**Feito:**
- `ModuleCatalog.hpp::moduleCatalog()` reordenado nas **8 famílias de
  trabalho**: SOURCE (+MATTER/STRING), TRANSFORM, MODULATE (novo —
  ENVELOPE/FUNCTION/DRIFT/CHAOS/SH), TIME (+TURING/SEQUENCE/TRIGSEQ),
  DECISION (só os que escolhem valor), ROUTE (novo — SWITCH/MATRIX/MULT/
  PLANAR), SPACE (+MEMORY), OUT (ex-MIX).
- `RASGO_MODULAR.md`: §3 (nota da revisão + as 3 fontes), §4 reescrita
  (§4.1 famílias de trabalho, §4.2 território reservado, §4.3
  correspondência com ModularGrid/Patch&Tweak/Doepfer), §31 (nota),
  §36.3 coluna Família (22 linhas).
- `dossies/00_indice.md` coluna Família (29 linhas) + ponteiro pra §4.1.
- `dossies/43_planar.md` família → ROUTE.
- `panel_main.cpp`: comentários de "não reordenar o catálogo" corrigidos.

**`tests/test_seed_patch.cpp` (NOVO):** monta o rack completo do catálogo
+ sink como o painel, roda `seedPatch` (seeds 1–24) e `crossPatch`,
confere: instancia (43 nós), sem exceção, caminho audível, saída finita
e < 12, determinismo byte a byte. **Revalidação: a reordenação do
catálogo NÃO muda o que `RASGO_SEED=N` produz** — verificado byte a byte
com a ordem antiga vs nova (o `seedPatch` cabeia a espinha por NOME de
tipo, não por posição do passeio). O comentário antigo do `panel_main`
que dizia o contrário era conservador demais.

Achado à parte (pré-existente, não regressão): ~1–3 seeds em 24 saem
quase-mudos (seed 18 = silêncio total com 22 cabos) — a espinha
voz→MASTER→sink é cabeada mas o gatilho não dispara. Fica como pendência
do `seedPatch`, não deste trabalho.

**Validação:** **53/53 CTest** Debug + Release. Build limpo.

Commitado (`918b62e`).

## Registro da etapa — 2026-09-06: Módulo 44 — OPERATOR (Onda B — 3/4)

`PESQUISA §2.4` Onda B, #44 (após "avance").

**`OPERATOR` — voz FM multi-operador** (`src/dsp/Operator.hpp`,
`tests/test_operator.cpp` — 9 funções, DFT com janela de Hann). Família
SOURCE. O `OSC` faz TZFM de um par; isto é FM de verdade.

- **4 operadores** (senóides via LUT de 2048), **8 algoritmos** (de
  A→B→C→D em série a A,B,C,D em paralelo/aditivo). Ordem A→B→C→D FIXA —
  nenhum algoritmo tem laço entre operadores, só A tem feedback → o
  cálculo é uma passada, sem topo-sort.
- **`ratio_b/c/d`** (0–9) quantizadas à tabela musical
  `{0,5;1;1,5;2;2,5;3;4;5;7;9}` — inteira = harmônico, quebrada =
  inarmônico (sino/metal). É o `QUANTIZER` da altura, mas pro timbre.
- **`index`** (0–1, +CV, mapeado ao quadrado) = profundidade global de
  modulação (bandas laterais de Bessel/Chowning). 0 = 4 senóides.
- **`feedback`** (0–1) = A modula a própria fase (média das 2 últimas
  amostras, à la DX7 — estabiliza o laço). Sozinho leva A a
  dente-de-serra.
- **`drift`** = micro-desafino lento e independente por operador,
  DETERMINÍSTICO (soma de senos, sem RNG).
- **Sem EGs por operador** nesta v1 (fica pra `OPERATOR+`); `ENVELOPE →
  index` cobre o ataque FM. Aliasing de banda lateral aceito (como no DX).
- **Saída:** soma das portadoras ÷ nº de portadoras (senóides → |·| ≤ 1)
  + `softclip` (assíntota ±1,5). Aditivo de 4 fica ~12 dB abaixo de 1
  portadora — `VCA`/`MIXER` normalizam; seguidor de ganho faria o nível
  bombear com `index`.

**Calibração de teste** (não bug do módulo): a interpolação linear da LUT
+ o vazamento espectral da DFT retangular davam centroide 1,5 pra uma
senóide pura → janela de Hann na `magAt` do teste, centroide clean → 1,00.

Integrado: `ModuleCatalog` (SOURCE, após ADDITIVE — seed-safe, ver
`test_seed_patch`), `test_panel_layout` (14 HP), `LearnCatalog`
(12 binds). Docs: `00_indice`, `PESQUISA §2.4`, `RASGO_MODULAR.md §36.3`,
`README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**54/54 CTest** Debug e Release. Testes: aditivo (razões 1/2/3 → pilha
harmônica, nada entre parciais); cadeia (índice 0 → portadora pura,
centroide 1,0; índice 0,8 → centroide 12); `index` sobe o centroide
monotônico; razão 2,5 → energia inarmônica em 2,5·f0; `feedback` → A
vira serra (centroide ×3); 1 V/oct; os 8 algoritmos finitos e < 1,5;
determinismo byte a byte com `drift`; extremos limitados. Renders de
exemplo estáveis (OPERATOR não entra em nenhum).

Commitado (`2e8ec5a`).

## Registro da etapa — 2026-09-06: Módulo 45 — FORMANT (Onda B — 4/4, COMPLETA)

`PESQUISA §2.4` Onda B, #45 — o último da onda (após "avance").

**`FORMANT` — ressoador espectral multibanda** (`src/dsp/Formant.hpp`,
`tests/test_formant.cpp` — 9 funções). Família TRANSFORM. O oposto do
`PARAMETRIC` (EQ estático em série): 5 passa-faixas em PARALELO cujas
frequências/bandas/ganhos seguem uma tabela de vogais e são varridos por
um knob — o espectro FALA.

- **5× SVF TPT** (Simper/Cytomic), não-linearidade NO laço — mesmo núcleo
  do `FILTER`/`WASP`. Saída de banda `v1` × `k` (normaliza o pico ≈ 1/k)
  × ganho da vogal, somada, `softLimit` no fim.
- **`vowel`** (0–1, +CV) varre A→E→I→O→U. 5 formantes/vogal: frequência
  interpolada em log, ganho em dB, banda linear. Dados fonéticos de voz
  de baixo (tabelas Csound `fof`/Fant 1960 — fato, `constexpr`, sem I/O).
- **`shift`** (−1..1) = `2^(shift·1,5)` sobre todas as frequências —
  comprimento do trato vocal.
- **`res`** (0–1) = `bw / (1 + res·8)` — de coloração sutil a bandas que
  cantam/apitam.
- **`mix`** (0–1) seco↔ressoado (0 = passa-direto bit-exato).
- **`drift`** = wobble determinístico por formante (senos, sem RNG).
- Sem entrada → silêncio (é TRANSFORM). Sem excitador interno nesta v1
  (pendência: modo vocoder, buzz glotal).

Integrado: `ModuleCatalog` (TRANSFORM, após FILTER — seed-safe),
`test_panel_layout` (14 HP), `LearnCatalog` (9 binds). Docs: `00_indice`,
`PESQUISA §2.4`, `RASGO_MODULAR.md §36.3`, `README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**55/55 CTest** Debug e Release. Testes: vogal A → picos em ~600/1040 Hz;
o pico de F2 sobe A(1020)→E(1600)→I(1720) monotônico (== dados fonéticos);
`shift` sobe o espectro; `res` alto estreita os picos (vale/pico cai
30 %+); `mix=0` = passa-direto bit-exato; entrada em silêncio → saída 0;
determinismo byte a byte com `drift`; extremos limitados. Renders de
exemplo estáveis (FORMANT não entra em nenhum).

**Não commitado ainda** → commit a seguir.

**Ondas A e B COMPLETAS** (7 módulos: GLIDE, WAVETABLE, LOOPER, ADDITIVE,
PLANAR, OPERATOR, FORMANT). Próximas: Onda C (reverb FDN, DRUM), Onda D
(SAMPLER — base de porte Navalha 2, SIGNAL-IN) — aguardam direção.

## Registro da etapa — 2026-09-06: Módulo 46 — HALL (Onda C — 1/2)

`PESQUISA §2.4` Onda C, #46 (após "avance").

**`HALL` — reverberação FDN** (`src/dsp/Hall.hpp`, `tests/test_hall.cpp`
— 11 funções). Família SPACE. **Decisão:** módulo NOVO, não modo do
`SPACE` — a topologia FDN não encaixa no objeto multitap+allpass do
`SPACE` sem reescrevê-lo, e o `SPACE` está entregue e testado.

- **8 linhas de atraso** (comprimentos primos entre si ~26–52 ms base) +
  **matriz de Householder** (`y_i = s_i − (2/N)·Σs` — reflexão
  ortogonal, `I − (2/N)11ᵀ` = Householder com `v = 1/√N`; sem perda,
  difusão máxima por 1 subtração/linha). Rede estável pra `g_i ≤ 1`,
  nunca cresce.
- `size` (+CV) escala as linhas (0,3×–1,7×). `decay` (+CV) = RT60
  `0,2·75^decay` (0,2 s–15 s) → `g_i = 10^(−3·d_i/RT60)`. `damp` =
  passa-baixa de 1 polo NO laço de cada linha. `mod` = modulação
  determinística do ponto de leitura (senóides ~0,5–1,4 Hz, fases
  distintas — chorus, quebra o ringing). `pre` = pré-atraso (0–120 ms).
  `mix`.
- Gate `freeze` → `g_i = 1` (cauda infinita, Householder preserva
  energia) + rampa de ~10 ms da entrada a 0.
- Estéreo: entrada mono → 8 linhas por vetor `{+,+,+,+,−,−,−,−}`; `l` =
  linhas pares com sinais alternados, `r` = ímpares → descorrelacionadas.
- `softLimit` na saída (o mesmo do `FILTER`) + flush de denormais.

Bug pego: `parameterValue("freeze")` lançava (`freeze` é PORTA, não
parâmetro) — só o gate da porta.

Integrado: `ModuleCatalog` (SPACE, após SPACE — seed-safe),
`test_panel_layout` (14 HP), `LearnCatalog` (12 binds). Docs:
`00_indice`, `PESQUISA §2.4`, `RASGO_MODULAR.md §36.3`, `README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**56/56 CTest** Debug e Release. Testes: impulso → cauda que decai
monotônica; `decay` maior → cauda 4×+ mais longa; `damp` alto → agudo
decai mais rápido (HF/total cai 40 %+); `freeze` → RMS estável por
segundos (não decai nem cresce); `mix=0` = passa-direto bit-exato;
`l`/`r` descorrelacionados (|corr| < 0,9); `pre` atrasa o molhado;
determinismo byte a byte com `mod`; extremos limitados; `decay=0,95`
sem freeze ainda decai (g < 1). Renders de exemplo estáveis.

Commitado (`bcf41ed`).

## Registro da etapa — 2026-09-06: Módulo 47 — DRUM (Onda C — 2/2, COMPLETA)

`PESQUISA §2.4` Onda C, #47 — o último da onda (após "avance").

**`DRUM` — voz de percussão** (`src/dsp/Drum.hpp`, `tests/test_drum.cpp`
— 11 funções). Família SOURCE. Um gate → um golpe. Empacota o que
`MATTER`+`NOISE`+`ENVELOPE` fariam à mão.

- **3 camadas:** CORPO (senóide com envelope de altura — o pitch-sweep
  do 808; `map` mistura com `tanh(corpo·3)` → clique do 909), ESTALO
  (ruído branco por passa-alta cujo corte sobe com `map` — 808 surdo →
  acústico brilhante — envelope próprio bem curto; `snap` é a dose),
  ENVELOPE de amplitude exponencial (`decay` ~20 ms a ~2 s).
- `tone` (20–1000 Hz, +CV 1 V/oct), `bend` (profundidade do sweep),
  `drive` (`tanh` + makeup — crunch do 909), `roll` (auto-disparo
  interno ~2–40 Hz = rufo/buzz e o modo autônomo), `drift` (humanização
  por golpe — xorshift **semeado NO disparo**, determinístico dada a
  sequência de gates). `accent` CV.
- Corpo modal via `MATTER` fica como pendência.

**Calibração de teste:** o `hfEnergy` de 1 polo do teste ainda passava
~7 % da fundamental de 200 Hz → virou 2 polos + o teste de `drive` usa
`magAt` (DFT janelada) pras harmônicas ímpares.

Integrado: `ModuleCatalog` (SOURCE, após MATTER — seed-safe),
`test_panel_layout` (14 HP), `LearnCatalog` (12 binds). Docs:
`00_indice`, `PESQUISA §2.4`, `RASGO_MODULAR.md §36.3`, `README`.

**Validação:** build limpo (`-Wall -Wextra -Wpedantic -Werror`),
**57/57 CTest** Debug e Release. Testes: gate → um golpe (silêncio
antes, decai depois); `decay` maior → cauda 5×+ mais longa; `bend` alto
→ a altura varre pra baixo (ZCR cai ao longo do golpe); `snap` → +3× de
energia de ruído; `map` sobe o brilho; `drive` → harmônicas ímpares ×4+
e saída limitada; `roll` > 0 sem gate → golpes periódicos; `accent`
alto → +50 % de nível; determinismo byte a byte com `drift`/`roll`;
extremos limitados. Renders de exemplo estáveis.

**Não commitado ainda** → commit a seguir.

**Ondas A, B e C COMPLETAS** (9 módulos: GLIDE, WAVETABLE, LOOPER,
ADDITIVE, PLANAR, OPERATOR, FORMANT, HALL, DRUM).

## Registro da etapa — 2026-09-06: Onda D — camada io/ + dr_wav (infra pro SAMPLER)

Autor: "avance a Onda D" (com o "sim" à dependência de leitura de áudio).

**dr_wav vendorizado** — `third_party/dr_wav/dr_wav.h` (v0.14.6, commit
`dfe8377`, domínio público / MIT-0 — compatível AGPLv3) +
`third_party/dr_wav/PROVENANCE.md`. Verbatim, sem alteração.

**`src/io/AudioFile.hpp` + `.cpp`** (camada `io/`, FORA do
`rasgo_modular_core` — que segue sem dependência): `struct AudioFile
{samples, channels, sampleRate}`, `loadAudioFile(path)` (WAV 8/16/24/32
+ float via dr_wav), `toMono()`. Nova biblioteca estática CMake
`rasgo_modular_io` (compilada com `-w` — o header do dr_wav tem warnings
próprios; o resto do projeto continua `-Werror`).

**`tests/test_audio_file.cpp`** — round-trip `writeWav16` (WavWriter) →
`loadAudioFile`: mono e estéreo dentro de ±1 LSB de 16 bits; `toMono`;
arquivo inexistente → `{}`. 58/58 CTest Debug e Release.

O `SAMPLER` (Módulo 48) vai receber um `std::vector<float>` — não conhece
`dr_wav`; carregar arquivo é gesto de UI (painel linka `rasgo_modular_io`).

Commitado (`c592423`).

## Registro da etapa — 2026-09-06: PitchShift portado + Módulo 48 — SAMPLER (Onda D — 1/2)

`PESQUISA §2.4` Onda D, #48 (após "avance a Onda D").

**`src/dsp/PitchShift.hpp` — `DelayPitchShifter`** (`tests/
test_pitch_shift.cpp` — 6 funções). **Porte** do `HeritagePitch`/
`LegacyPitchChannel` do `NAVALHA2_JUCE` (algoritmo = `G09.pitchshift.pd`
do Pd, Puckette, domínio público; implementação C++ = Navalha de Glerm
Soares / Navalha 2 de Lúcio Araújo, GPL-3.0-or-later). Duas leituras de
uma linha de 10 ms janeladas por `sin`, defasadas meia fase, interpolação
`vd~` de 4 pontos, passa-alta de 5 Hz. Adaptação: `setSemitones(int)` →
`setRatio(float)` contínuo (RASGO quer oitavas/cents). **Núcleo em
`double`** — o passeio de fase acumula por horas; `float` degradava a
janela e virava ruído (bug pego: `float` dava espectro espalhado, `double`
dá razão de saída linear e exata em [0,25; 4]).

**`src/dsp/Sampler.hpp` — SAMPLER** (`tests/test_sampler.cpp` — 10
funções). Família SPACE. Toca-fatias: `trig` → um golpe de um trecho.

- Gravação ao vivo (gate `rec` grava `in`, buffer de 8 s) OU arquivo
  (`setBuffer(mono, srcRate)` — só do painel, fora do RT; arquivo tem
  prioridade). Sem buffer → silêncio.
- `start`; `speed` (−1..1 → `sinal·2^(|speed|·2)` = ±0,25×–±4×, negativo
  = reverso); `slices` (1–16) + CV `pos`; `repitch` (0 varispeed / 1
  pitch-shifter, duração preservada); `wear` (jitter de início + hold +
  bit-crush, **por disparo**, xorshift **semeado no disparo** —
  determinístico); `loop`.
- De-click adaptativo (`clamp(dur·0,24, 0,5–5 ms)` — do `SlicePlayer` do
  Navalha 2). Bug pego: em `loop`, `rendered_ >= total_` matava a voz e o
  `declick` aplicava release → separado `envUp_` (só ataque) e o fim de
  fatia em loop não mexe no envelope.
- Crédito registrado em `RASGO_MODULAR.md §29.1` (nova tabela de código
  de terceiros: dr_wav VERDE, PitchShift/SlicePlayer AMARELO).

Integrado: `ModuleCatalog` (SPACE, após SH — seed-safe),
`test_panel_layout` (14 HP), `LearnCatalog` (12 binds). Docs: `00_indice`,
`PESQUISA §2.4`, `RASGO_MODULAR.md §36.3` + `§29.1`, `README`.

**Validação:** build limpo, **60/60 CTest** Debug e Release. Testes:
grava 200 Hz → toca 200 Hz; `speed=0,5` → 400 Hz; `slices=2` + `pos` →
fatia certa; `repitch=1` + oitava → 400 Hz com fatia durando o dobro;
`loop` sustenta; `wear=1` → piso de bit-crush 2×+, limitado; determinismo
byte a byte; `setBuffer` toca o arquivo; sem buffer → silêncio absoluto.

Commitado (`38b7de8`).

## Registro da etapa — 2026-09-06: Módulo 49 — SIGNAL-IN (Onda D — 2/2, roadmap FECHADO)

`PESQUISA §2.4` Onda D, #49 (após "avance a Onda D").

**`SIGNAL-IN` — o `AUDIO-IN` cresceu** (`src/dsp/SignalIn.hpp`,
`src/dsp/AudioIn.hpp` = alias, `tests/test_signal_in.cpp` — 12 funções,
ex-`test_audio_in.cpp`). Família SOURCE. Decisão do autor: áudio + MIDI
num adaptador só (não `MIDI-IN`/`CV-IN` separados).

- **Anel de áudio** = o do `AUDIO-IN`, tal e qual (resync em estouro,
  silêncio em underrun).
- **Anel de MIDI** (`pushMidi(status,d1,d2)`, 1024 eventos) → `process()`
  drena por bloco e resolve **voz monofônica last-note** (pilha de 16).
  note-on/off, note-on vel 0 = off, CC (`cc_num`), pitch-bend.
- **6 saídas:** `out` (áudio L — nome preservado pra compat de `.rmp`),
  `r`, `pitch` (1 V/oct, nota 60 = 0 V + bend × `bend` st), `gate`
  (rampa 1 ms), `vel`, `cc`.
- **Migração:** `type()` = "SIGNAL-IN"; `makeModule("AUDIO-IN")` alias;
  `using AudioIn = SignalIn`; `moduleCatalog()` lista `SIGNAL-IN`;
  `panel_main` procura `"SIGNAL-IN"` (2 pontos); `LearnCatalog` e o
  `test_learn_catalog` migrados.
- Contraparte de ENTRADA do `NOTE-OUT` (#38).

**Pendência:** a thread `snd_seq` no painel que chama `pushMidi()` (mesmo
padrão do `AlsaSource` do áudio) — o módulo está pronto, mas o MIDI ao
vivo precisa desse pedaço + um teclado pra validar. CV bruto DC-coupled
fica pra quando houver caso.

Integrado: `ModuleCatalog` (SOURCE, `AUDIO-IN`→`SIGNAL-IN` — seed-safe),
`test_panel_layout` (6 HP), `LearnCatalog` (9 binds), `panel_main`.
Docs: `00_indice` (#35 aponta pra #49), `PESQUISA §2.4`,
`RASGO_MODULAR.md §36.3`, `README`. Contagem: **48 módulos** (o rename
não conta 2×).

**Validação:** build limpo (painel incluído), **60/60 CTest** Debug e
Release. Testes: áudio herdados do `AUDIO-IN`; MIDI — gate/pitch/vel no
note-on; last-note priority (C4→G4→solta G4→volta C4); vel 0 = off;
pitch-bend ±0,5 fundo de escala × `bend`; `cc` só segue `cc_num`;
determinismo byte a byte; `type()` correto.

**Não commitado ainda** → commit a seguir.

---

## ROADMAP §2.4 FECHADO (2026-09-06)

As 4 ondas de continuidade completas — **11 módulos** nesta sessão
(GLIDE, WAVETABLE, LOOPER · ADDITIVE, PLANAR, OPERATOR, FORMANT · HALL,
DRUM · SAMPLER, SIGNAL-IN) + a taxonomia consolidada (18→8 famílias) +
a camada `io/`/`dr_wav` + `PitchShift.hpp` (porte Navalha 2). **48
módulos**, 60 alvos CTest verdes em Debug e Release.

Pendências abertas: thread ALSA-seq de MIDI no painel (#49); `TAPE`/
`TURNTABLE` (`ESTUDO_audio_sampling §4`); as reservas da taxonomia
(`§4.2`); `git push`.

## Registro da etapa — 2026-09-06: pendências (1) — bug dos seeds quase-mudos

**Diagnóstico:** seed 18 saía com peak EXATAMENTE 0,0 por > 10 s. Trace
por nó: o `CLOCK[0]` (pulso) ficava travado em 1,0 e o `CLOCK[1]`
(euclid) nunca disparava → tudo que depende do euclid (LPG.strike,
ENVELOPE.gate, MATTER.pluck) ficava mudo.

**Causa:** o passeio semeado cabeou `TRIGSEQ.any → CLOCK.reset`. O
`CLOCK.reset` é edge-triggered (`stepCounter_ = 0; phase_ = 0`); um
trigger que pulsa mais rápido que o passo do relógio zera a fase de
`phase_` a cada pulso → o relógio nunca anda. O `seedPatch` cabeia as
SAÍDAS do CLOCK explicitamente (espinha); deixar o passeio mexer nas
ENTRADAS dele é quase sempre estol.

**Fix:** `apps/panel/SeedGrammar.hpp` — `seedClassifyPorts` pula as
entradas do `CLOCK` (`if (t == "CLOCK") continue;`). As entradas nunca
entram em `outDst[]`, o passeio não as escolhe. Muda os seeds (arrays de
classificação diferentes) — deliberado, `test_seed_patch` revalida.

**Resultado:** dos ~3 seeds quase-mudos em 24 → **1** (seed 1, ~−63 dBFS
— patch legítimo baixo, não estol; peak 7e-4, som real). O `test_seed_patch`
agora renderiza 2000 blocos (~5,3 s — seeds lentos a ~46 BPM só produzem
o 1º evento do euclid depois de ~1,3 s) e o gate é `silent <= 2`.

**60/60 CTest** Debug e Release. Painel builds.

## Registro da etapa — 2026-09-06: pendências (2, 3) — thread ALSA-seq de MIDI + rename

**`apps/panel/AlsaMidi.hpp` (NOVO)** — porta de entrada MIDI via ALSA
sequencer. Cria uma porta virtual **"RASGO Modular : IN"** (`snd_seq`,
NONBLOCK); `poll(fn)` drena os eventos e chama `fn(status, d1, d2)`
(note on/off, CC, pitch-bend → 14-bit d1/d2). Header-only, fora do core.

**`panel_main.cpp`:** o `syncAudioIn` virou **`syncSignalIn`** (renomeado
nas 7 chamadas) e agora abre/fecha o `AlsaMidi` **e** o `AlsaSource`
quando um nó `SIGNAL-IN` aparece/some — cada um na sua thread. A thread
de MIDI faz `gmx.try_lock()` (como a de áudio), `midiIn->poll(...)` →
`node->pushMidi()` em cada `SIGNAL-IN`, sleep de 2 ms. `stopMidiIn()` no
encerramento.

Smoke test: `AlsaMidi` abre a porta virtual OK neste sistema, `poll()`
volta limpo (0 eventos). O end-to-end com um teclado (`aconnect <src>
"RASGO Modular"`) é do autor pra validar — não dá pra cobrir headless.

**Pendência (4) — taxonomia §4.2:** conferida; o bullet GESTO/ENTRADA
atualizado ("resolvido: o `SIGNAL-IN` #49"). O resto já estava certo.

Docs: `dossies/49_signal_in.md`, `RASGO_MODULAR.md §36.3` + `§4.2`,
`PESQUISA §2.4`. **60/60 CTest**, painel builds limpo.

**Não commitado ainda.**

Pendências restantes: `TAPE`/`TURNTABLE` (precisa de decisão do autor —
`ESTUDO_audio_sampling §4`); teste ao vivo do MIDI; `git push`.

---

## Registro da etapa — 2026-09-06: TAPE (→ `heads` no LOOPER) + Módulo 50 — TURNTABLE

Decisão do autor: **"TAPE: avance como sugeriu; TURNTABLE a"**.

**TAPE — NÃO virou módulo.** Entrou como param **`heads` (1–4)** no
`LOOPER` (#41): eco de fita multi-cabeça (Roland RE-201 / Space Echo). 1 =
eco simples; 2–4 cabeças leem frações do `time` (`{1; 0,75; 0,5;
0,25}×`), somadas (÷ nº de cabeças); a realimentação regenera todas →
Frippertronics denso com `feedback` perto de 1 + `time` longo. Só no modo
forward. Painel: knob `HEADS` em (24,54), toggles `HOLD`/`REV` movidos.
Commit `3a9eabc`.

**Módulo 50 — `TURNTABLE`** (família SPACE). O mesmo buffer do `SAMPLER`
lido por um **prato com massa**: `readPos` é a integral de uma velocidade
angular com inércia (EDO de 1ª ordem). Params: `speed` (alvo ±0,5×–±2×,
neg = reverso), `torque` (força do motor → *wow* de partida), `friction`
(coasting no `brake` + retorno pós-scratch), `grab` (firmeza da mão na CV
`scratch`), `start`, `wear` (estalos determinísticos + micro-wobble),
`loop`. `trig` põe a agulha e **liga o motor** (com `torque` baixo, a
nota nasce grave e sobe). Acoplamento AC de 1 polo na saída (prato parado
→ a amostra congelada some em ~40 ms, sem degrau de DC). Não toca
enquanto grava (igual ao `SAMPLER`). **Desvio Rasgo:** o Navalha 2
rejeita a metáfora de DJ; aqui diverge, mas com o modelo físico e SEM
quantização de BPM. `dr_wav` fica na camada `io/`, o core segue sem
dependência.

Arquivos: `src/dsp/Turntable.hpp`, `tests/test_turntable.cpp` (11
testes), `dossies/50_turntable.md`. Integração: `CMakeLists.txt`
(`rasgo_modular_turntable_tests`), `apps/panel/ModuleCatalog.hpp`,
`apps/panel/LearnCatalog.hpp` (13 binds), `tests/test_panel_layout.cpp`.
Docs: `00_indice.md` (+ contagem 49), `PESQUISA §2.4`,
`ESTUDO_audio_sampling §4.2`/§5, `RASGO_MODULAR.md §36.3`/§36.5/§29.1 +
contagens (49 módulos, 61 CTest), `README.md`.

Ajustes durante o desenvolvimento: `dragIdle = friction·0,00003` (o servo
segura a rotação — o `0,0006` inicial deixava o prato em ~0,9×); DC
blocker na saída (o vinil não tem DC — e resolve o "degrau" quando o
prato para); label do knob `TORQUE` → `TORQ` (5 chars, gate de layout).

**61/61 CTest em Debug e Release.**

Pendências restantes: teste ao vivo do MIDI (autor); `git push`.

---

## Registro da etapa — 2026-09-06: dossiê-proposta BOXCAR (#51, não implementado)

O autor achou o **AI Synthesis AI250 BXR** interessante (14 HP, Daisy;
"audio mangler / CV generator / VCO" inspirado no *boxcar averager* —
equipamento de teste nuclear, ref. Stanford Research SR200 NIM / SR-235).
Pediu o dossiê-proposta.

**`dossies/51_boxcar.md` (NOVO — proposta, sem código).** Padrão §1–§8
completo. O boxcar averager: gatilho + delay de abertura + *aperture* +
média de N capturas; *scanning* do delay reconstrói a forma de onda.
Reinterpretação musical: revelar sinal enterrado no ruído (verbo
REPAIR); S&H de janela (mede fatia, não instante); cabeça de leitura
varrendo a onda capturada; "modo oscilador" que relê o buffer que ele
mesmo montou (desvio Warps/Rings). Saída secundária `geiger` = trigger
de Poisson livre (falta no RASGO — todo ritmo aleatório hoje é preso ao
clock). Modelo por amostra, estados, extremos, testes propostos e painel
(14 HP, família DECISION, ao lado do `ABACUS`) todos esboçados.

Não é VCO novo (o RASGO já tem OSC/WAVETABLE/ADDITIVE/CHORD). Licença
OK: boxcar de bancada é domínio público (Wikipedia, manuais SR200); o
firmware do AI250 publica só o `.bin` — conceito, não código.

**Decisões do autor (2026-09-06 — tomadas):** (1) nome **`BOXCAR`** —
sem impedimento (termo genérico de DSP; o AI Synthesis usou "BXR", não
"Boxcar"; regra de licença do RASGO é sobre código, não copiado);
(2) família **DECISION**; (3) `geiger` **nos dois** — saída secundária
do `BOXCAR` **e** um modo Poisson livre novo no `NOISE`; (4) modo
oscilador **na v1** (releitura interpolada do `recon_`); (5) `in`/`out`
**Audio + Control**.

Dossiê atualizado pra "aprovado — a implementar"; `00_indice.md`
linha 51 e `PESQUISA §2.4` idem.

---

## Registro da etapa — 2026-09-07: Módulo 51 — BOXCAR + NOISE.poisson

**`src/dsp/Boxcar.hpp` (NOVO), `tests/test_boxcar.cpp` (11 testes).**
Reinterpretação musical do *boxcar averager*: `trig` trava no evento
repetitivo, `delay` posiciona uma janela no período MEDIDO, `aperture` a
largura, o conteúdo é integrado e empilhado com as capturas anteriores
(`average` = N, EMA com piso 1/min(N,hits)). `scan` varre o `delay` →
reconstrói a onda toda. `mode`: 0 follower (S&H de janela; `average` alto
= sem tremor), 1 reconstruct, 2 oscillator (relê o buffer próprio a
`rate`). Auto-trigger por `thresh` (edge trigger) quando `trig` livre.
`geiger` = trem de gates de Poisson LIVRE (`t = −ln(U)/λ`, semeado).
`blend` seco↔processado. `in` livre → piso de ruído interno −34 dB (modo
autônomo). Família DECISION, 14 HP.

**Ajustes vs. a proposta:** `mode 0` segura `recon_[bin]` (não o `m`
cru) → a média realmente ajuda o follower; a janela escreve o ARCO de
bins `[delayPos, delayPos+aperture]` (a abertura É suavização em fase, e
enche o buffer com menos capturas); `mode` default 0 (sem `scan`, 1/2 só
enchem uma fatia).

**`src/dsp/Noise.hpp`:** param `poisson` (0–1) — troca o relógio interno
periódico do S&H/smooth por o mesmo processo de Poisson. Stream xorshift
`rngPois_` próprio (poisson=0 → byte-idêntico ao anterior). Knob `POIS`,
painel do `NOISE` de 12 → 14 HP.

Integração: `CMakeLists.txt` (CTest 62), `ModuleCatalog.hpp` (DECISION),
`LearnCatalog.hpp` (BOXCAR 15 binds + NOISE.poisson), `test_panel_layout`,
`test_learn_catalog` (NOISE list). Docs: `00_indice`, `51_boxcar.md`,
`PESQUISA §2.4`, `RASGO_MODULAR.md §4.1`/§36.3/§36.5 + contagens
(50 módulos, 62 CTest), `README`.

**62/62 CTest em Debug e Release.**

---

## Registro da etapa — 2026-09-07: Motion Engine v3 — a mão caótica

O autor reportou o VARIA mexendo `OSC.pw` "freneticamente" e
`DECISION.steps` "sem nada cabeado", e pediu uma revisão de fundo:
*"instrumento de composição, não de regras prontas, de determinismos
congelados, de padrões limitados de IA. Um instrumento que busca a
excelência de composição, o inaudito, o bom gosto musical, a
experimentação."* Rejeitou duas propostas baseadas em tabela (matriz
`param→qualidade`; máquina de estados de intenções).

**Modelo aceito (`ESTUDO §3.7` reescrito):** uma **MÃO CAÓTICA**.
- **`apps/panel/MotionField.hpp` (NOVO)** — atrator de Thomas (3 vars,
  ciclicamente simétrico, `b = 0.19` → caótico). Nunca repete, nunca
  congela; semeado → reproduzível, período longo demais pra o ouvido
  pegar um ciclo. A velocidade do campo segue a `energy` do som.
  `motionReach(id)` — 4 pistas de palavra (estrutural/nível/quente/livre)
  que decidem só a AMPLITUDE.
- **`apps/panel/MotionEngine.hpp`** — o `Binding` manual-only FICA
  (`peca_generativa_4`, testes). Adicionado: `Fiber` + `inhabit(graph,
  seed, shown)` — varre o grafo, monta uma fibra por knob/slider/toggle
  (só `MIXER`/`MASTER` de fora — o músico mistura e panora na mão). Cada
  fibra: vetor de projeção semeado sobre o campo → os controles se movem
  EM RELAÇÃO (coerente por construção), cada um por um caminho seu.
  `tick(graph, dt, energy)` avança o campo, faz a ousadia (1 fibra
  estica a excursão por alguns s), e um **duck protetor** (som perto do
  teto → puxa tudo pro centro — responde ao clip que o autor reportou).
  Toggle = histerese + dwell mínimo. Estrutural = amplitude ínfima +
  quantizado. Cabo já plugado → amplitude pela metade.
- **`src/core/SignalGraph.hpp`** — `parameterIsModulated(node, id)`
  (accessor const; a Motion Engine usa pra não brigar com a fiação).
- **`apps/panel/panel_main.cpp`** — `populateMotion` (110 linhas de
  hash + lista de bloqueio) → `motion.inhabit(graph, curSeed, shown)`.
  No `tick`: `energy` = RMS do scope do `MASTER`.
- **`tests/test_motion_engine.cpp`** — 5 testes novos: a mão move e fica
  são; estrutural mal se move (quantizado); MIXER/MASTER isentos;
  determinismo do `inhabit`; quente respira, não varre.

Abertura primeiro (autor: "testar com mais abertura, depois limitar o
que não funciona") — knob + slider + toggle, tudo menos MIXER/MASTER.

**Evolução registrada:** camada de arco/`morceau` por cima (Form Engine,
`ESTUDO §3.7` "Evolução") — caminhada de Markov intro/subida/clímax/
queda/coda + gramáticas Freytag/Kishōtenketsu + accel/ritardando, prior
art no `RASGO_SYNTH/rasgo-synth-performance` (read-only).

**62/62 CTest Debug + Release.** Não commitado ainda.

Pendências abertas: confirmar de ouvido que o v3 resolve o pw/steps e o
clip (seed 625938148); teste ao vivo do MIDI; `git push`.

---

## Registro da etapa — 2026-09-07: barra de scroll da paleta + LEARN de módulo + dwell 1 s

Três pedidos do autor.

**Barra de scroll discreta** na coluna esquerda (`apps/panel/panel_main.cpp`)
— `palBar()` calcula a geometria (trilho `kCaseTop+3 .. palBottom-3` na
borda direita da coluna; cursor proporcional a `viewH/contentH`), só
desenha quando `contentH > viewH+4`. Trilho de 2 px (`T.line`), cursor de
4 px (`T.textSecondary`, `T.accent` no hover/arraste). Interação: roda do
mouse (já existia; passou a usar `palMaxScroll()`), **arraste do cursor**
(`palBarDrag`, MotionNotify mapeia `my` → `paletteScroll`), **clique no
trilho** pagina. `ButtonRelease` solta.

**LEARN — definição de módulo no hover do corpo/título**
(`apps/panel/LearnCatalog.hpp`): `moduleLearnTable()` + `lookupLearnModule(type)`
— um `LearnEntry` por tipo do `moduleCatalog()` (`quick` = o que é,
`understand` = o lugar dele / vizinhos, `explore` = uma cadeia).
`AUDIO-IN` reusa `SIGNAL-IN`. No `panel_main.cpp`: se o mouse está no
corpo do módulo mas **não** num widget bound → mostra a definição do
módulo (`rawTitle = tipo`, `rawKey = id + "|\x01mod"`).
`testEveryCatalogModuleHasBlurb` (novo, `test_learn_catalog`) é o gate.

**Dwell 2 s → 1 s** — `panel_main.cpp` `std::chrono::milliseconds(1000)`.

Docs: `ESTUDO §6`, `RASGO_MODULAR.md §36.7`. **62/62 CTest Debug +
Release.** Painel builds limpo.

---

## Registro da etapa — 2026-09-07: Motion Engine v3 afinada + carimbo de build + véu dos overlays

Depois do autor ver `pw`/`reso`/`fold` "frenéticos" no painel:
- **`MotionField.hpp`/`MotionEngine.hpp`** — quente ±9%→±4%, livre
  ±22%→±15%, estrutural ±3%→±1,2%; easing bem mais lento; campo de Thomas
  mais devagar; **`freq`/`pitch`/`tune`/`transpose`/`note`/`key` →
  estrutural** (afinação = composição — o range 8–8000 Hz do OSC estava
  varrendo demais); **janela relativa ao valor atual** (param de range
  enorme e escala log respira em torno de onde está); ousadia só nos
  livres. Commit `012e851`.

**Carimbo de build no SOBRE** (o autor não sabia qual versão rodava —
"ainda não há versão no sobre"). `CMakeLists.txt` captura
`git rev-parse --short HEAD` + data (fallback só a data) →
`-DRASGO_MODULAR_BUILD=`; o SOBRE mostra `build <hash> · <data>  ·
compilado <__DATE__ __TIME__>`. `CMAKE_CONFIGURE_DEPENDS` no
`.git/HEAD` pra o hash não ficar velho.

**Véu semi-transparente nos overlays** (tutorial e SOBRE) — o autor:
"quando abre a aba sobre desaparece os módulos no fundo". X11 puro não
tem alfa → um `dimStipple` de 8×8 (~62% coberto) sobre o back-buffer já
desenhado deixa os módulos VISÍVEIS por trás, só escurecidos.

**Frase de crédito da família** (`UiLanguage.hpp::footerCredit`, 4
idiomas — padrão Antitotem/Rasgo Synth) na **faixa vazia acima da 1ª
fileira de módulos** (não no rodapé — pedido do autor), com **ano na
frase + versão** (o carimbo de build anexado):
`© LÚCIO DE ARAÚJO · RASGO MODULAR 2026 · LICENÇA AGPLv3+ · build <hash> · <data>`.
Os módulos ficam onde estavam (`capText` na faixa `kCaseTop+11`).

Painel builds limpo, 62/62 CTest. Não commitado ainda.

Pendências: o CLIP reportado (seeds 597512815, 625938148) — render
headless de 90 s dá pico −34 dBFS, ZERO clip; falta o autor dizer se é
com VARIA on/off e se o PICO do cabeçalho acende. Confirmar de ouvido a
mão calma. `git push`.

---

## Registro da etapa — 2026-09-07: o músico continua no comando com VARIA ligado

O autor: "mesmo o VARIA ligado deve ser possível modificar os controles
na mão — faço isso e o controle volta pra posição que estava"; e "o
cabeamento também".

- **`MotionEngine::tick`** — no topo do laço de fibras, se
  `parameterUserValue(node, param)` divergiu do que a engine escreveu por
  último (> 1,2% do range), é a mão do músico → a fibra **re-ancora**:
  `center = value = valor do músico`. A mão passa a respirar em torno do
  novo valor, não volta pro anterior. (Toggle idem, zera o dwell.)
- **`MotionEngine::refreshCables(graph)`** — re-lê `parameterIsModulated`
  de todas as fibras; a fibra de um param recém-cabeado cai pra 45% de
  amplitude (não briga com o LFO/env plugado). Chamada 1×/frame no painel
  (barato) — pega qualquer via de mudança de cabo.
- **`panel_main.cpp`** — o `tick` também pausa com `cdrag.active` (arraste
  de cabo), não só `drag.active`.
- **`test_motion_engine`** — `testHandEditWins` (11ª função): com a mão
  ligada, `setParameterBase(cutoff, 5000)` → a fibra segue em torno de
  5000, não volta pros ~800 do seed.

62/62 CTest Debug + Release.

---

## Registro da etapa — 2026-09-07: revisão da lista + Onda E começou — Módulo 52 SWIRL

Autor pediu a revisão dos módulos potenciais ainda não codados.
`PESQUISA §2.5` — cruzamento dos 51 feitos contra §2.2/§3/§4/§6/§7/§8.
Tier 1: **SWIRL** (chorus/flanger/ensemble/phaser — o RASGO não tinha
NENHUM efeito de modulação), CRUSH (destruidor lo-fi — verbo DAMAGE sem
casa), STAGES (gerador de segmentos configuráveis), + PHASER (entrou no
SWIRL). Tier 2: RESONATOR, PULSAR, SWARM. Ordem sugerida da "Onda E":
SWIRL → CRUSH → PHASER(no SWIRL) → STAGES → RESONATOR → PULSAR.

**`src/dsp/Swirl.hpp` (NOVO), `tests/test_swirl.cpp` (11 testes).**
`type` (0 chorus · 1 flanger · 2 ensemble · 3 phaser): os três primeiros
= atrasos CURTOS modulados por LFO triangular (linha de ~50 ms/canal);
o phaser = **6 all-pass de 1ª ordem TPT** varridos pelo LFO. `rate`
(0,02–8 Hz, +CV), `depth`, `feedback` (−1..1; a escala do flanger é 1,08
→ auto-oscila passando da unidade), `spread` (LFO de R defasado → estéreo
+ desafino), `tone` (1 polo no molhado, <0 LP / >0 HP), `age` (**desvio
Rasgo** — companding BBD `|x|^(1∓0,25·age)` + ruído semeado + wobble; daí
a auto-oscilação sem entrada), `mix` (+CV). `mix=0` bypass exato;
`age=0` determinístico puro. Família **SPACE** (a gaveta de efeitos).

Integração: `CMakeLists.txt` (CTest 63), `ModuleCatalog.hpp` (SPACE),
`LearnCatalog.hpp` (SWIRL — 12 binds + definição de módulo),
`test_panel_layout`, `test_learn_catalog`. Docs: `00_indice`,
`52_swirl.md`, `PESQUISA §2.5`, `RASGO_MODULAR.md §4.1`/§36.3/§36.5 +
contagens (51 módulos, 63 CTest), `README`.

**63/63 CTest Debug + Release.**

---

## Registro da etapa — 2026-09-07: Onda E — Módulo 53 CRUSH (destruidor lo-fi)

**`src/dsp/Crush.hpp` (NOVO), `tests/test_crush.cpp` (10 testes).** É onde
mora o verbo **DAMAGE** (o `wear` estava espalhado em SAMPLER/TURNTABLE/
LOOPER). Cinco vetores de dano DIGITAL: `rate` (S&H interno 100 Hz–24 kHz,
+CV — SEM anti-alias, o aliasing É o som), `bits` (quantização a 2^bits),
`drive` (ganho antes da quantização), `wrap` (0 clipa · 1 enrola
`mod(x+1,2)−1` = overflow de inteiro · blend), `glitch` (probabilidade
por hold de amostra travada / dropout / repique — semeado), `jitter`
(passeio lento no clock do S&H → wow digital, semeado), `tone` (1 polo),
`mix` (+CV). Dano **reprodutível**. `in` livre + `mix=1` → S&H do próprio
ruído = fonte lo-fi. `mix=0` bypass exato. Família **TRANSFORM** (junto
de SHAPE/WASP). `process()` não aloca (sem buffer).

Integração: CMake (CTest 64), ModuleCatalog (TRANSFORM), LearnCatalog
(12 binds + def. de módulo), test_panel_layout, test_learn_catalog.
Docs: 00_indice, 53_crush.md, PESQUISA §2.5, RASGO_MODULAR.md §4.1/§36.3,
README; contagens 52 módulos / 64 alvos CTest.

**64/64 CTest Debug + Release.** Onda E: SWIRL ✓ CRUSH ✓ | STAGES,
RESONATOR, PULSAR pendentes.

---

## Registro da etapa — 2026-09-07: Onda E — Módulo 54 STAGES (segmentos configuráveis)

**`src/dsp/Stages.hpp` (NOVO), `tests/test_stages.cpp` (10 testes).** O
`FUNCTION` é UMA rampa; o `STAGES` é N segmentos reconfiguráveis cuja
FUNÇÃO EMERGE de como se encadeiam (Mutable Stages / Rossum Control
Forge / Blukač Fractalist). `segments` (2–8), `rate` (0,02–20 Hz, +CV),
`contour` (0–1 — forma dos níveis: sobe/arco/desce, blend), `curve`
(−1..1 — exp/lin/log por segmento), **`hold`** (0 desliza = rampa/env/
LFO · 1 salta e segura = degrau/S&H/sequência — o knob que faz virar
env OU seq sem trocar de tipo), `tilt` (−1..1 — distorção das durações),
`jitter` (desvio Rasgo — passeio SEMEADO nos níveis/durações), `loop`
(corre ↔ dispara). Saídas `out`/`eoc`/`step`. Desenhado por macros
(gerador, não editor). `jitter=0` → determinístico puro. `process()`
não aloca. Família **MODULATE**.

Ajuste no desenvolvimento: `curveShape` estava com exp/log trocados
(`<0` deve ser côncava p/ baixo = rápido→lento); corrigido. `hold≥1` →
`t=0` (segura `from`, salta na fronteira — degrau limpo, sem rampa de
1 sample).

Integração: CMake (CTest 65), ModuleCatalog (MODULATE), LearnCatalog
(13 binds + def. de módulo), test_panel_layout, test_learn_catalog.
Docs: 00_indice, 54_stages.md, PESQUISA §2.5, RASGO_MODULAR.md §4.1/
§36.3, README; contagens 53 módulos / 65 alvos CTest.

**65/65 CTest Debug + Release.** Onda E: SWIRL ✓ CRUSH ✓ STAGES ✓ |
RESONATOR, PULSAR pendentes.

---

## Registro da etapa — 2026-09-07: Onda E — Módulo 55 RESONATOR (ressoador modal externo)

**`src/dsp/Resonator.hpp` (NOVO), `tests/test_resonator.cpp` (11
testes).** O `MATTER` é voz AUTO-CONTIDA; o `RESONATOR` é o Rings/
Elements no modo "ressoador" — banco de ≤ 24 modos afinados que um sinal
EXTERNO bate/arqueia (DRUM, NOISE, uma voz). `freq` (20–5000 Hz, +CV
1V/oct), `structure` (harmônico↔esticado `k·√(1+B·k²)` — a mesma do
`ADDITIVE`), `partials` (1–24), `decay` (Q; pólos < 1), `damp` (agudos
decaem antes), **`tilt`** (−1..1 — inclinação espectral; **varrer cruza
`low`↔`high`** — Three Sisters), `position` (pente de pluck), `mix`
(0 = bypass). 3 saídas `low`/`mid`/`high` (1/3 grave/médio/agudo dos
parciais). `strike` = exciter embutido (rajada de ruído ~3 ms). `in`
livre → ruído interno de −36 dB (modo autônomo). Família **TRANSFORM**.

Ajuste no desenvolvimento: a normalização do ganho ressonante — `b0=amp`
+ `g_[k] = (1−r)·2·sin(w)` (1/ganho ressonante) aplicado na saída de
cada modo → `tilt` controla a amplitude direto sem o pólo perto de 1
dando um boost enorme aos graves. Ganho de banda 26/√count + `tanh`.

Integração: CMake (CTest 66), ModuleCatalog (TRANSFORM), LearnCatalog
(14 binds + def. de módulo), test_panel_layout, test_learn_catalog.
Docs: 00_indice, 55_resonator.md, PESQUISA §2.5, RASGO_MODULAR.md
§4.1/§36.3, README; contagens 54 módulos / 66 alvos CTest.

**66/66 CTest Debug + Release.** Onda E: SWIRL ✓ CRUSH ✓ STAGES ✓
RESONATOR ✓ | só PULSAR pendente.

---

## Registro da etapa — 2026-09-07: Onda E — Módulo 56 PULSAR (síntese pulsar) — Onda E COMPLETA

**`src/dsp/Pulsar.hpp` (NOVO), `tests/test_pulsar.cpp` (10 testes).**
Síntese pulsar de Curtis Roads (*Microsound*): um trem de PULSARETS —
grão curto seguido de silêncio, período total `p`. DUAS frequências
INDEPENDENTES: `freq` (20–2000 Hz, +CV 1V/oct) = 1/`p` = a ALTURA;
`formant` (0,1–8×, +CV) = 1/duração-do-pulsaret = o TIMBRE, **sem
desafinar** — o pente harmônico fica preso a múltiplos de `freq`,
`formant` só desliza o envelope espectral. `shape` (0–1 — 1 ciclo de
seno → 2–3 + harmônico agudo), `window` (0 ≈ retangular/brilhante →
0,4 Hann → 1 expodec percussivo; janela de Tukey→Hann→expodec),
`jitter` (**desvio Rasgo** SEMEADO no período/amplitude), `mask` (0–1 —
probabilidade de PULAR um pulsaret; o *masking* de Roads — padrões
rítmicos por subtração), `spread` (pulsarets alternados L/R — estéreo
por granulação), `level` (softclip `tanh`). Saídas `out`/`r` (L/R).
Pool de 4 vozes de grão; `process()` não aloca. Fonte autônoma — soa ao
carregar; `jitter=mask=0` → trem periódico determinístico. Família
**SOURCE** (depois do OPERATOR).

Ajustes no desenvolvimento: a `windowFn` virou Tukey (platô + bordas
raised-cosine curtas, SEMPRE 0 nas pontas) para w<0,4 e Hann→expodec
(ataque `1−e^{−40t}` + cauda `e^{−4t}·(1−t)`) para w≥0,4 — a versão
antiga deixava a janela em ~0,2/0,17 nas pontas, criando descontinuidade
entre pulsarets e vazando banda larga. Testes de espectro afrouxados
para o modelo real do pulsaret de 1 ciclo: o centróide sobe ~4× (não 2×)
de formante 1→5; a fundamental enfraquece com formante alto mas o pente
harmônico carrega a altura; monotonia de brilho verificada a partir de
formante 2 (em formante≈1 o pulsaret preenche o período — caso
degenerado); `mask` 0,85 → RMS < 0,55×.

Integração: CMake (CTest 67), ModuleCatalog (SOURCE, após OPERATOR),
LearnCatalog (12 binds + def. de módulo), test_panel_layout,
test_learn_catalog. Docs: 00_indice, 56_pulsar.md, PESQUISA §2.5,
RASGO_MODULAR.md §4.1/§36.3, README; contagens 56 módulos / 67 alvos
CTest.

**67/67 CTest Debug + Release.** **Onda E COMPLETA:** SWIRL ✓ CRUSH ✓
STAGES ✓ RESONATOR ✓ PULSAR ✓ (o PHASER entrou no SWIRL).

---

## Registro da etapa — 2026-09-07: investigação do clip com VARIA + teste de regressão

O autor reportou clip de áudio com o VARIA (Motion Engine v3) ligado nas
seeds 597512815 / 625938148. Investigação **headless** (`clip_probe` —
reproduz o caminho do painel: grafo completo de 1 de cada módulo + voz-base
+ `seedPatch` + `motion.inhabit` + laço de `motion.refreshCables`/
`motion.tick` a 33 ms, medindo pico/clips/RMS na saída):

- **7 seeds × MASTER a −24 / 0 / +12 dB × VARIA on/off × 60–180 s:
  ZERO clips (`|x| ≥ 1`) em todos os casos.** Com o MASTER a +12 dB (teto
  do slider) o limitador do MASTER trava o pico em ≈ 0,8913 (−1 dBFS).
- VARIA ON empurra mais energia (RMS ~20–40 % maior em alguns seeds), mas
  o limitador absorve — nunca clipa.
- `AlsaSink.hpp:80` já faz clamp em [−1, 1] antes do int16 → sem clip
  digital duro na camada ALSA.
- Hipóteses restantes (fora do alcance do headless): binário v2 velho
  (pré-duck protetor) ou **xrun do ALSA** — o comentário de `AlsaSink.hpp:72`
  diz que underrun "soa como 'tudo rachado'"; é crepitação de *timing*
  (custo do `motion.tick`), não de amplitude.
- Observação de código (NÃO alterado — mexe no Motion Engine): o
  `motion.tick` e o arrasto de knob à mão escrevem parâmetros sem pegar o
  `gmx` (o thread de áudio segura o `gmx` no `process()`; o de MIDI e o de
  áudio-in pegam `try_lock`). Escrita de float solta — no pior caso um
  valor rasgado por 1 bloco (inaudível). Fica pra decisão do autor pôr o
  tick sob `try_lock`.

**Teste de regressão novo:** `tests/test_motion_no_clip.cpp` +
`rasgo_modular_motion_no_clip_tests` (CTest 68). 3 casos: (1) +12 dB VARIA
ON, 8 seeds → zero clips + pico < 1,0; (2) +36 dB VARIA ON nas 2 seeds do
autor → zero clips; (3) −24 dB (fábrica) → pico folgado (< 0,5) e o patch
produz som. ~8 s de execução.

O teste do teclado MIDI ao vivo (hardware) continua pendente e é do autor —
a lógica do `SIGNAL-IN` (note-on/off, last-note, vel 0) já é coberta por
`test_signal_in.cpp`; o thread ALSA-seq do painel está guardado por
`gmx.try_lock()`.

**68/68 CTest Debug + Release.**

---

## Registro da etapa — 2026-09-07: cliques com VARIA — correções C + A no painel

O autor confirmou: **são cliques** (não crepitação contínua nem distorção
sustentada) com o VARIA ligado. Aprovou C + A ("se não funcionar fazemos
o B também").

**C — baratear o `motion.tick` (`panel_main.cpp`):** `motion.refreshCables()`
rodava a cada frame (33 ms) varrendo todos os cabos por fibra. A fiação
quase nunca muda entre frames (só muda via `populateMotion`, que já
reconstrói as fibras) — o único caso que o `refreshCables` do tick pega é
um desligamento por botão-direito sem `populateMotion`. Agora roda ~1×/s
(`motionCableThrottle`, 30 frames). Corta o custo de CPU do VARIA e alivia
o xrun do ALSA (o thread de áudio emite silêncio = clique quando não
acompanha).

**A — energia real pra a mão caótica (`panel_main.cpp`):** a v3 lia a
energia do anel de osciloscópio DECIMADO do MASTER (`stride = block/44`,
220 amostras), copiado por `try_lock` que defasa/falha sob carga — o duck
protetor ficava cego. Agora o thread de áudio mantém um
`std::atomic<float> gOutRms` (seguidor de RMS da saída real, τ ≈ 25 ms,
escrito por chunk) e o `motion.tick` lê esse átomo. Sem lock, sem
decimação, sem defasagem. O escalonamento `× 2,2` e o limiar do duck
(`energy > 0,82`) ficam iguais.

Ambas são correção técnica (custo de CPU + sinal de realimentação
quebrado); o COMPORTAMENTO da mão quando o som está calmo é idêntico.

**68/68 CTest Debug + Release.**

---

## Registro da etapa — 2026-09-07: B + suavização por-amostra (5 módulos) + diagnóstico final do clique

**B (`panel_main.cpp`):** o duck do Motion Engine também dobra
`Master::gainReductionDb()` na energia — se o limitador trabalha, puxa
as fibras quentes pro centro. Commit `93df255`.

**Suavização por-amostra (`PARAMETRIC`, `SHAPE`, `HALL`, `RESONATOR`,
`CRUSH`):** recalculavam coeficiente/curva por bloco a partir do param
cru; um degrau entre blocos saltava a resposta com estado não-nulo →
zíper. Agora deslizam por amostra (~5–10 ms) pros alvos, como o `FILTER`
já faz com `cutoff`. Estado inicia no valor do param → patch estático
byte-idêntico. `rate`/`bits`/`jitter` do CRUSH ficam crus (caráter).
Commit `2924c71`.

**Diagnóstico final do clique — é xrun do ALSA, não o áudio.** Probe
`wd2` reproduz o caminho EXATO do painel (grafo completo podado +
`seedPatch` + `motion.inhabit` + laço de `motion.tick` a 33 ms) no ganho
do próprio seed. Sete seeds, incluindo as 3 que o autor reportou
(944390523, 625938148, 597512815, 181161106):

| | maior \|x[n]−x[n−1]\| | saltos > 0,08 |
|---|---|---|
| VARIA OFF | ≤ 0,0139 (−37 dBFS) | **0** |
| VARIA ON  | ≤ 0,0139 | **0** |

O DSP **não produz clique** em nenhum seed, com ou sem VARIA. (As
contagens de "centenas de cliques" das medições anteriores eram bug do
probe: threshold fixo 0,08 medido com o MASTER forçado a 0 dB = +24 dB
acima do seed → a forma de onda normal cruzava o threshold.)

Logo o clique está no **caminho de tempo real**: o `snd_pcm_recover` do
`AlsaSink.hpp:96` engole xruns sem log; o comentário de `AlsaSink.hpp:72`
já descreve que xrun na camada ALSA do PipeWire "soa como 'tudo rachado'".
O VARIA acrescenta custo por frame (o `redraw()` repinta ~60 painéis a
30 fps com os knobs se movendo) → em máquina/config no limite, derruba
períodos de áudio = clique. O **C** (refreshCables 1×/s) já ataca parte.

**Próximo passo proposto (aguarda OK):** baixar a taxa de `redraw` quando
o VARIA está ligado e nada mais acontece (pintar 1 a cada 2 frames, ~15
fps) e/ou só repintar painéis cujos valores mudaram. Alívio de CPU
direto. Toggles do Motion Engine: descartado como causa (só ~1% dos
eventos; e o DSP não clica de qualquer forma).

**68/68 CTest Debug + Release.**

---

## Registro da etapa — 2026-09-07: caixa de número de seed no cabeçalho (copia/cola)

O número do seed vivia no rótulo do botão `SEED` — num painel X11 cru não
dá pra selecionar/copiar. Agora:

- Botão `SEED` = só `⚄ SEED` (sorteia, como antes).
- **Caixa rebaixada à esquerda do botão** com o número atual (ou `—` se o
  patch foi editado à mão).
- **Clique na caixa** → foca + copia o número pro clipboard do X11
  (CLIPBOARD + PRIMARY), flash "COPIADO"/"COPIED"/… (i18n `seedCopied`).
- Focada: dígitos / Backspace / **Enter** carrega o seed / **Esc**
  cancela / **Ctrl+C** copia / **Ctrl+V** cola. Atalhos de letra do
  cabeçalho (g/v/m/s/c/e) suspensos enquanto focada. Clique fora tira o
  foco.
- Protocolo de seleção do X11 implementado (`SelectionRequest` serve a
  string; `SelectionClear`; `SelectionNotify` recebe o paste) — primeira
  vez nesse painel. Átomos `CLIPBOARD`/`TARGETS`/`RASGO_SEED_PASTE` +
  `UTF8_STRING` (já existia). `#include <X11/Xatom.h>`.

Só painel — sem alvo CTest; **68/68** continua verde.

---

## Registro da etapa — 2026-09-07: seed de lançamento com entropia real + teto de cabeamento ↑

**`nextRandomSeed` (`panel_main.cpp`):** o autor reportou repetição de
números ao reiniciar. A versão antiga misturava só o `steady_clock` com
UMA rodada de xorshift — mistura fraca; entre lançamentos rápidos (delta
de poucos ms nos bits baixos) podia dar seeds correlacionados/repetidos.
Agora: `std::random_device` (/dev/urandom) ⊕ os dois relógios ⊕ seed
anterior → **splitmix64** (finalizador forte). 12 lançamentos seguidos =
12 seeds distintos e bem espalhados. `#include <random>`.

**Teto de cabeamento (`PatchSeed.hpp`):** com 42 módulos o passeio
ponderado ia a `3 + complexity·25` ≈ 28 tentativas; com 56 módulos há
muito mais porta livre. Subido pra `·40` ≈ 43. Distribuição real
(20 000 seeds): mediana 19→**22**, média 22→**26**, p90 35→**47**, teto
45→**60**. Os patches simples continuam simples (o grosso ainda entre
10–19 cabos); só o topo ficou mais denso. `id.complexity` inalterado.

**68/68 CTest Debug + Release.**

---

## Registro da etapa — 2026-09-07: Onda F — Módulo 57 SPECTRA (resíntese espectral)

**`src/dsp/Spectra.hpp` (NOVO), `tests/test_spectra.cpp` (11 testes).**
A lacuna análise → síntese (`PESQUISA §2.6` Tier 1). Nenhum módulo RASGO
ouvia um som e o re-sintetizava — o `ADDITIVE` constrói do zero, o
`MEMORY` faz grão no tempo. `SPECTRA` analisa o espectro de curto prazo
de `in` e re-oscila como um banco de senóides que **segue** o som
(Panharmonium / phase vocoder no idioma RASGO).

**Análise:** 64 passa-faixas ressonantes de 2 polos (numerador
`x−x[-2]`, pico ~unitário), log-espaçados 35 Hz–14 kHz, BW ~1/9 oitava
(sobrepostas) + seguidor de pico (atk 3 ms / rel 60 ms). A cada ~6 ms
(hop): máximos locais > −40 dB do maior, ordenados por força, os
`voices` primeiros; interpolação parabólica de `ln(env)` em log-freq
entre as 3 bandas vizinhas refina cada frequência.

**Síntese:** `voices` (2–24) senóides de fase contínua; cada uma DESLIZA
(sem zíper — a lição do zíper desta sessão) para a freq/amp do pico
herdado, constante de tempo = `blur` (0 ~5 ms ↔ 1 ~600 ms). `shift`
(±2 oct, +CV 1V/oct em `pitch`) e `stretch` (−1..1 inarmônico) transpõem
a re-síntese SEM tocar na análise; `tone` inclina; `jitter` (**desvio
Rasgo**) = wobble SEMEADO por voz; **`freeze`** (toggle + gate) para a
análise → *spectral freeze* / pad infinito de qualquer som; `mix`.
Saídas `out`/`r` (L/R, vozes ímpares/pares panoramizadas). Família
**SOURCE**.

**`in` livre → fonte autônoma:** ruído interno de −30 dB + 2 parciais
fantasma (180/430 Hz) que derivam devagar (semeados) → drone tonal que
evolui. `process()` não aloca; `jitter=0` → determinístico (com e sem
entrada).

Ajuste no desenvolvimento: a 1ª versão usava ressoador all-pole (ganho
ressonante ~500×) e Q altíssimo com 48 bandas esparsas → um parcial
entre bandas não era detectado e a saída saturava. Corrigido: BP
normalizado `b0(x−x[-2])`, 64 bandas, BW alargada pra as bandas se
sobreporem.

Integração: CMake (CTest 69), ModuleCatalog (SOURCE, após PULSAR),
LearnCatalog (12 binds + def. de módulo), test_panel_layout,
test_learn_catalog. Docs: 00_indice, 57_spectra.md, PESQUISA §2.6,
RASGO_MODULAR.md §4.1/§36.3, README; contagens 57 módulos / 69 alvos
CTest.

**69/69 CTest Debug + Release.** Onda F: SPECTRA ✓ | SHIFTER, vocoder
(mode do FORMANT), VCA 4ch pendentes.

---

## Registro da etapa — 2026-09-07: Onda F — Módulo 58 SHIFTER (deslocador de frequência)

**`src/dsp/Shifter.hpp` (NOVO), `tests/test_shifter.cpp` (13 testes).**
Deslocador de frequência SSB (`PESQUISA §2.6` Tier 1). Virou MÓDULO
próprio, não `mode` do `SHAPE` — o conceito (mover o espectro por Δf
fixo em Hz, banda única) é distinto do waveshaper. O `SHAPE` faz
ring-mod (bandas soma **e** diferença simétricas); o `SHIFTER` entrega
**uma** por saída: `up` (espectro + Δf) e `down` (espectro − Δf) —
a relação entre elas é o processo (Three Sisters). Como o deslocamento é
aditivo em Hz, os parciais deixam de ser harmônicos → metálico/sineiro.

`shift` (−2000..2000 Hz, +CV `shift_mod` ×1000 Hz), **`feedback`**
(−0,95..0,95 — a saída `up` volta pra entrada → glissando infinito, o
*barber pole* de Risset-Shepard; `tanh` no laço, `fb` negativo puxa de
`down`), `tone` (−1..1), `drift` (**desvio Rasgo** SEMEADO no Δf),
`mix`. Família **TRANSFORM**.

DSP: SSB por transformada de Hilbert — FIR de 255 taps (kernel `2/(πk)`
ímpar, janela de Blackman) para a componente imaginária + linha de
atraso casada de 127 amostras (~2,6 ms) para a real → modulação em
quadratura `re·cos ∓ im·sin`. Rejeição de imagem > 50 dB acima de
~250 Hz; abaixo tende ao ring-mod (o FIR curto não faz Hilbert no
grave — mesma limitação do hardware). softclip de segurança ±0,8 na
saída (feedback + tone altos podem passar da unidade). `process()` não
aloca; `drift=0` → determinístico.

Ajuste no desenvolvimento: a 1ª tentativa usou rede all-pass IIR
polyphase (Niemitalo 4+4) — a quadratura só ficava boa numa faixa
estreita (corr ~1,0 no grave, ~0 só perto de 1 kHz). Trocado por FIR de
255 taps, que dá quadratura perfeita e > 50 dB de rejeição na faixa
útil. E o feedback passou a realimentar SÓ a saída `up` (não up+down,
que se cancelavam) → o barber pole funciona.

Integração: CMake (CTest 70), ModuleCatalog (TRANSFORM, após SHAPE),
LearnCatalog (9 binds + def. de módulo), test_panel_layout,
test_learn_catalog. Docs: 00_indice, 58_shifter.md, PESQUISA §2.6,
RASGO_MODULAR.md §4.1/§36.3, README; contagens 58 módulos / 70 alvos
CTest.

**70/70 CTest Debug + Release.** Onda F: SPECTRA ✓ SHIFTER ✓ | vocoder
(mode do FORMANT), VCA 4ch pendentes.

---

## Registro da etapa — 2026-09-07: Onda F — vocoder como `mode` do FORMANT (#45)

**`src/dsp/Formant.hpp`, `tests/test_formant.cpp` (4 testes novos, 13 no
total).** Não é módulo novo — o `FORMANT` (5 passa-faixas paralelos de
vogal) ganhou um modo vocoder. Recomendação do autor: opção A (no
FORMANT) para o vocoder, opção B (módulo próprio `VCA4`) para o VCA.

- **Entrada `mod`** (índice 3, **apensa** — patches existentes referenciam
  índices 0–2, intactos) = o modulador.
- **Param `vocoder`** (0–1, apenso). `vocoder>0` + `mod` cabeado → +5 SVF
  de análise no `mod` nas mesmas frequências das bandas de portadora + 5
  seguidores de envelope (~12 ms); o ganho de cada banda vira
  `lerp(ganhoVogal, env·6, vocoder)`. `vowel` continua posicionando as 5
  frequências → escolhe QUAIS 5 pontos vocodar.
- **`vocoder=0` ou sem `mod`** → o bloco do vocoder é pulado inteiro →
  saída **byte-idêntica** à versão anterior (teste
  `testVocoderZeroIsClassicFormant` + `testVocoderNeedsModulator`
  comparam byte a byte).

Vocoder de 5 bandas — grosso mas vocálico (não fala inteligível de banda
larga). Um `VOCODER` de N bandas dedicado fica como pendência do `§2.6`.
Modelo: canal vocoder de Homer Dudley (Bell Labs, 1938 — domínio
público).

Integração: LearnCatalog (`vocoder` + `in:mod` binds + blurb do FORMANT
atualizado), test_panel_layout (o painel do FORMANT já era pego pelo
teste — só ganhou 1 knob + 1 jack, cabe nos 14 HP). Sem novo alvo CTest
(é `mode`). Docs: 45_formant.md §5.1, RASGO_MODULAR.md §36.3, PESQUISA
§2.6.

**70/70 CTest Debug + Release.** Onda F: SPECTRA ✓ SHIFTER ✓ vocoder ✓ |
só `VCA4` (4 canais, módulo próprio) pendente.
