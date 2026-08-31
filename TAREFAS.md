# RASGO Modular — tarefas e continuidade

**Atualizado:** 2026-08-30 (retomada documental)
**Estado:** `prototype`

Esta é a lista operacional do projeto. A arquitetura conceitual permanece em
[`RASGO_MODULAR.md`](RASGO_MODULAR.md); decisões transversais e reutilização
continuam no inventário global.

## Continuidade de acervo — proporcional ao desenvolvimento

- [ ] Quando houver release, migração, entrega ou promoção de módulo escolhida,
  definir o recorte de fontes/documentação, gerar manifesto/checksum e testar a
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

Toda etapa futura deve registrar objetivo, arquivos alterados, validação,
limitações e próximo passo neste arquivo ou no documento técnico mais próximo.
