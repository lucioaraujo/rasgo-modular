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
