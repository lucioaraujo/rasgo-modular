# Rasgo Modular

**Identificador arquivístico:** `ARQ-RSM-001` — ingresso e manifesto em
[`../RASGO_ARQUIVO/`](../RASGO_ARQUIVO/INGRESSO_ARQ-RSM-001.md)
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
resolvidas). **Rack de partida completo**; 38 módulos DSP, 47 alvos CTest
verdes (Debug + Release), 5 peças de exemplo byte-idênticas. Custodiante:
Lúcio de Araújo.
**Licença do código:** GNU AGPLv3 ou posterior — ver [`LICENSE`](LICENSE)
(decisão do autor, 2026-09-01: "a mesma que temos usado" → a licença
habitual da família RASGO). Código de terceiros só entra sob licença
livre compatível com AGPLv3 (MIT/BSD/ISC/Apache-2.0/LGPL/GPL/AGPL) e com
proveniência registrada.

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
- `src/core/SignalGraph.hpp` = grafo de áudio; `src/dsp/*` = os 38 módulos
  (+ `Oversampler.hpp`, helper 2× compartilhado);
  `examples/*` = as 5 peças; `apps/panel/` = painel gráfico de teste
  (+ `MotionEngine.hpp`, protótipo de composição — ver
  `dossies/ESTUDO_seed_composicao_generativa.md`);
  `CMakeLists.txt` + `tests/` = 47 alvos CTest.

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
[`../RASGO_ARQUIVO/MANIFESTO_ARQ-RSM-001_marco-1_2026-09-01.md`](../RASGO_ARQUIVO/MANIFESTO_ARQ-RSM-001_marco-1_2026-09-01.md)
(inventário somente leitura + SHA-256).

**Pendente (só quando o estado deixar de ser móvel):** cópia de
preservação congelada + teste de restauração — atrelar a um commit/tag do
marco, à migração de diretórios ou ao handoff do marco 2. Enquanto o
protótipo evolui, basta atualizar `TAREFAS.md`, este README e o ingresso;
não se cria pacote de preservação a cada etapa (governança
`VM_STUDIO_ARCHIVE.md §167-181`).
