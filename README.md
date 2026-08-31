# Rasgo Modular

**Identificador arquivístico:** `ARQ-RSM-001`
**Estado:** protótipo C++17 de graph engine modular
**Licença de distribuição:** a definir antes de reutilização externa ou
publicação

## Apresentação

Rasgo Modular é o ambiente modular próprio da família RASGO: instrumento,
laboratório de síntese, composição e performance, com potencial de oferecer
infraestrutura reutilizável sem absorver a identidade dos instrumentos irmãos.
A antiga “Fábrica de Módulos” permanece como camada funcional e antecedente
conceitual, não como um projeto separado.

## Retomada e fontes locais

- [`RASGO_MODULAR.md`](RASGO_MODULAR.md) — arquitetura, taxonomia, fluxos,
  decisões e pesquisa;
- [`TAREFAS.md`](TAREFAS.md) — estado operacional, testes e próximo marco;
- `CMakeLists.txt` e `tests/test_graph_engine.cpp` — protótipo executável e
  teste do graph engine.

O protótipo separa áudio, controle, evento e descritor, possui blocos de tamanho
fixo e testa o grafo mínimo. A documentação registra build e `ctest` aprovados
para o estado atual; integração multimodal completa, scheduler stateful,
realtime safety ampliada e segundo consumidor continuam pendentes.

## Limites e promoção

Nenhum módulo é promovido ao comum apenas por semelhança. Antes de reutilização
externa, confirmar autoria, licença, dependências, contrato, testes e um
segundo consumidor plausível. O núcleo não deve receber UI, processos externos
ou integração entre instrumentos antes de estabilizar o contrato de dados.

## Próxima tarefa arquivística

Em marco estável de release, migração, entrega ou promoção de módulo, definir o
recorte de fontes/documentação e então gerar manifesto/checksum e testar
restauração. Enquanto o protótipo evolui, atualizar arquitetura e tarefas sem
criar pacote de preservação a cada experimento.
