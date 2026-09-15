# Estudo à parte — Seed, composição generativa, partitura e pedagogia

**Estado:** estudo / mapa de arquitetura. **Os 5 itens da conversa
ganharam protótipo (2026-09-04/05)**, cada um com lacunas documentadas na
sua seção — nenhum está "pronto", mas nenhum é só papel:
Motion Engine (§3.5/§3.6 — `apps/panel/MotionEngine.hpp` +
`examples/peca_generativa_4.cpp` + `tests/test_motion_engine.cpp`;
**integrado ao painel interativo em 2026-09-04**, `[v]`),
Patch Genetics `MUTATE`/`EVOLVE`/`CROSS`/`FREEZE` (§4 —
`apps/panel/PatchGenetics.hpp` + `tests/test_patch_genetics.cpp`;
**integrado ao painel interativo**, `[m]`/`[e]`/`[c]` — `CROSS` desde
2026-09-05),
RASGO Score — `SYSTEM SCORE` (§5 — `apps/panel/ScoreRecorder.hpp` +
`tests/test_score_recorder.cpp`, integrado em `peca_generativa_4.cpp` e
gravação ao vivo `[Ctrl+R]`) + `MUSICAL SCORE` (captura de notas via
`NOTE-OUT`, Módulo 38, desde 2026-09-05),
Learning Engine (§6 — `apps/panel/LearnCatalog.hpp`, caixa sempre
presente no rodapé da paleta, os 37 módulos preenchidos),
Gramática do Seed nomeada e explícita (§1.1/§7 —
`apps/panel/SeedGrammar.hpp`, desde 2026-09-05).
**Origem (2026-09-04):** conversa do autor com o ChatGPT sobre o rumo do
RASGO Modular — colada na íntegra na sessão. Este documento sintetiza o
vocabulário proposto, cruza com o código real (`apps/panel/PatchSeed.hpp`,
`src/core/SignalGraph.hpp`) e aprofunda o ponto que motivou a conversa.

**Pedido do autor:** *"documente a conversa, para não nos perdermos nesse
assunto do seed, composição, variação de controles"* — e, ao ler o
cruzamento com o código: *"isso é uma limitação atual, os patches mudam,
porém as regulagens permanecem praticamente as mesmas"* / *"também não
há muita variação (no sentido de composição generativa), podemos ir a
fundo nesse item"*.

---

## 0. Por que este documento existe

A conversa cobre quatro frentes bem diferentes em tamanho e risco —
vocabulário de patch/seed, uma camada de composição generativa, um
sistema de partitura/registro de eventos, e um sistema pedagógico de
hover-learn. Nenhuma foi implementada. Este documento existe pra:

1. preservar o vocabulário e a arquitetura conceitual da conversa;
2. cruzar cada ideia com o que **já existe** no motor (pra não redescobrir
   o que já foi resolvido, nem prometer o que já está feito);
3. aprofundar, com números do código real, a limitação que o autor
   apontou — item 3 abaixo é o mais desenvolvido de propósito.

---

## 1. O vocabulário proposto (adotado)

Do ChatGPT, em português quando fizer sentido usar no projeto:

- **patch** = a configuração resultante das conexões (+ knobs, switches,
  sequências); **patching** = o ato/processo de conectar;
  **patch design** = a concepção deliberada; **routing** = o caminho
  técnico do sinal; **patch as composition** = o patch como estrutura
  composicional, não só cabeamento.
- **Seed**: `Study Seed` (didático, legível), `Exploration Seed`
  (combinações menos óbvias, cross-mod, feedback), `Composition Seed`
  (já com comportamento temporal — eventos, ciclos, densidade).
  `Seed → algoritmo → patch → resultado`, reproduzível por número.
- **Operações sobre um patch já existente** (não regerar do zero):
  `MUTATE`, `EVOLVE`, `CROSS`, `SIMPLIFY`, `COMPLEXIFY`, `VARIATION`,
  `FREEZE`, `RESEED`.
- Distinção: **Random Patch** (fortuito) vs. **Seeded Patch**
  (determinístico, reproduzível) vs. **Generative Patch System**
  (regras que constroem patches) vs. **Patch Evolution** (transformação
  no tempo).
- Pra composição: **Motion Engine** (comportamento de parâmetro no
  tempo), **Event Engine** (notas/triggers), **Form Engine** (seções,
  evolução estrutural), **Mutation Engine** (transforma o próprio
  sistema), regidos por um **Composer** com tendências globais
  (`density`, `tension`, `energy`, `complexity`, `stability`, `novelty`).
  Comportamentos de parâmetro nomeados: `STATIC`, `DRIFT`, `WALK`,
  `OSCILLATE`, `CHASE`, `ATTRACT`, `REPEL`, `PULSE`, `STEP`, `RAMP`,
  `ORBIT`, `SYNC`, `CHAOS`, `PROBABILISTIC`.
- Pra registro: **RASGO Event** (nota / mudança de parâmetro / conexão de
  patch, com timestamp), três níveis de partitura — `MUSICAL SCORE`
  (convencional, exportável MusicXML/MIDI), `SYSTEM SCORE` (patch +
  modulação + mutação), `RASGO SCORE` (os dois + forma).
- Pra pedagogia: `Guided Patch`, `Study Seed`, `Patch Challenge`,
  `Incomplete Patch`, `Debug Patch`, `Explain This Patch`,
  `Variation Exercise`; níveis de hover `QUICK`/`UNDERSTAND`/`EXPLORE`;
  botões `WHY?` / `WHAT IF?`; modo Learn silencioso por padrão.

### 1.1 Seed como hipótese, não como `Randomize` (adição, 2026-09-04)

Distinção que vale registrar à parte, porque muda a intenção por trás do
`[g]`: um `Randomize` diz "embaralhe os parâmetros"; um **Seed** poderia
dizer **"crie uma hipótese de patch"** — obedecendo uma gramática de
papéis, não só sorteio cego:

```
SOURCE → TRANSFORM → CONTROL → MODULATE → FEEDBACK → OUTPUT
```

com probabilidades, restrições e regras de compatibilidade por papel
(um `SOURCE` liga num `TRANSFORM`, não direto num `FEEDBACK`, etc.).
Dois exemplos de "hipótese" que a gramática deveria conseguir produzir
(formas bem diferentes, ambas válidas):

```
CLOCK → SEQUENCER → OSC.pitch
ENVELOPE → VCA
OSC → WAVEFOLDER → FILTER ← RANDOM CV
```

```
LFO A → OSC B.fm → FILTER ← DELAY ← OSC B
                     └──── feedback ────┘
```

**Cruzamento com o código real:** `PatchSeed.hpp` já tem uma noção
implícita de papel — a "espinha" (`spine[n]`, §2 abaixo) sempre percorre
fonte→processamento→saída, e cada `setT(...)` é uma regra de
compatibilidade escrita à mão (ex.: só liga em `FILTER.cutoff` se o tipo
character combina). A gramática **explícita e nomeada** ganhou protótipo
em 2026-09-05: `apps/panel/SeedGrammar.hpp` — extraiu a classificação de
porta (`enum Src`/`Dst` locais → `SeedSrc`/`SeedDst` nomeados,
`seedClassifyPorts()`) e a matriz de compatibilidade (`seedCompatibilityMatrix()`)
de dentro de `seedPatch()` pra um arquivo próprio, documentado, com a
tabela de correspondência pro vocabulário `SOURCE/TRANSFORM/CONTROL/
MODULATE/FEEDBACK/OUTPUT` acima. **Refactor comprovado, não reescrita:**
a classificação continua por PORTA, não por módulo inteiro (reorganizar
pras 6 categorias por módulo seria uma REGRESSÃO de precisão — ex.
`NOISE.smooth` e `NOISE.white` são o mesmo módulo mas classes de porta
diferentes) — verificado byte a byte: `seedPatch()` produz exatamente o
mesmo grafo serializado (`SignalGraph::serialize()`) pros seeds 1–200,
antes e depois do refactor. `FEEDBACK` continua sendo propriedade do
cabo (`reaches()`), não classe de porta; `OUTPUT` continua fixo
(`sink`/`MASTER`). A "caminhada" que sorteia cabo a cabo (consome RNG)
não mudou de lugar nem de lógica — só passou a LER a classificação e a
matriz de um arquivo nomeado em vez de tê-las embutidas anônimas.

**Patch como objeto pesquisável — genealogia.** A mesma conversa propõe
guardar não só o patch final, mas a **origem generativa**: um seed pode
ter variações e evoluções, cada uma rastreável até a origem —

```
Seed 1042
  ├── Variation A
  │     └── Mutation A1
  ├── Variation B
  └── Evolution 01
        ├── Gen 1
        ├── Gen 2
        └── Gen 3
```

`SEED`/`MUTATE`/`EVOLVE`/`FREEZE` já têm protótipo (§3, §4) — cada
`mutatePatch`/`evolvePatch` já é determinístico por seed, então a
**trajetória** já existe tecnicamente (dado o seed original + a
sequência de chamadas, o caminho é reproduzível). O que não existe é o
**registro da árvore em si** — hoje, rodar `MUTATE` duas vezes não deixa
rastro de que a segunda veio da primeira; isso é `RASGO Score` (§5)
aplicado a operações de patch, não só a eventos de áudio. `CROSS`
ganhou protótipo em 2026-09-05 (§4) — `SIMPLIFY`, `COMPLEXIFY`,
`VARIATION` (como operação nomeada e reconhecível, distinta de um
`MUTATE` qualquer) continuam de fora, nenhuma tem protótipo.

**Atração/repulsão como família de regras do Seed.** A conversa liga
isto ao que já existe: em vez de cabear ao acaso, módulos/parâmetros
poderiam **atrair, repelir, orbitar, sincronizar, agrupar ou se
separar**, e o patch nascer dessas relações. `MotionEngine::Attract`
(§3) já implementa a metade "atrair" — um parâmetro persegue outro. As
outras (`REPEL`, `ORBIT`, `SYNC`, agrupar/separar) estão só nomeadas em
§1, sem `Behavior` correspondente ainda.

---

## 2. Estado real — o que já é isso, e o que falta

### 2.1 O Seed já existe

`apps/panel/PatchSeed.hpp::seedPatch(graph, seed)` **é** o "Seeded
Patch" da conversa, não uma proposta: seed `uint64` determinístico
(SplitMix64), gramática probabilística de portas (matriz de
compatibilidade fonte×destino — não arquétipos fixos, ver
`design.md §2.3`), genes (`complexity`, `wildness`, `motion`, `energy`,
`space`, `voiceBias`, root/escala, `bpm`), `[g]`/botão sorteia,
`RASGO_SEED=N`/`--seed N` reproduz, `Ctrl+B` grava banco
(`patches/seed-N.rmp`). O **v1** (histórico, substituído) foi modelado
direto no `CompositionIdentity`/`SeedLineage`/`GamePlan` do RASGO Synth
Studio ([[reference_synth_studio_seed_model]] — consultado, não copiado;
RASGO_SYNTH é read-only).

`MUTATE`/`EVOLVE`/`CROSS`/`FREEZE` **existem** desde 2026-09-04/05 (ver
§4) — `SIMPLIFY`/`COMPLEXIFY`/`VARIATION` (como operação nomeada) ainda
não. `MUTATE`/`EVOLVE`/`CROSS` são casos de "editar o patch como está",
não substitutos do "regenerar tudo do zero" (reseed completo), que
continua sendo o `[g]`.

### 2.2 A limitação verificada: por que as regulagens parecem sempre as mesmas

Lendo `seedPatch()` linha a linha, o quadro é mais preciso do que
"quase nada muda", mas confirma a observação do autor:

- **A espinha (`spine[n]`) fica INTOCADA pelo passo genérico de
  randomização** (`if (!touched[n] || spine[n]) continue;` — a espinha é
  o caminho garantido voz→…→MASTER→sink). Os módulos da espinha recebem
  valores **específicos por caráter** (o `switch` sobre `id.character`,
  linhas ~296–420: `Drone`/`Percussive`/`Textural`/`Melodic`/… cada um
  com sua faixa de `setP` própria) — não são aleatórios livres, são
  *receitas por caráter*. Dentro de um caráter, patch a patch, essas
  receitas escolhem novos números a cada seed (são `rng(lo, hi)`), então
  a espinha **varia**, mas dentro de faixas estreitas e sempre pela
  mesma receita — o "jeito" de um Drone continua sempre um Drone.
- **Módulos fora da espinha, mas alcançados por algum cabo**, recebem um
  passe genérico (§564-593): pra cada parâmetro não-estrutural
  (`bpm`/`mult`/`length`/`scale`/`root`/`mode`/`voices`/`gain`/… ficam de
  fora), ~45% de chance (`f01() > 0.55f` pula) de sortear um valor **na
  faixa TOTAL** do parâmetro (`rng(lo, hi)`) — isso sim é bem aberto.
- **Módulos do catálogo que não entram no cabeamento daquele seed
  específico** (a maioria, num catálogo de 34 módulos e um patch de
  dezenas de cabos) **nunca são tocados** — ficam no valor **default de
  fábrica** do construtor C++ do módulo, pra sempre, seed após seed.
  Como o painel adiciona TODOS os tipos do catálogo como nós (mesmo os
  não cabeados — ver a sonda de seed em `TAREFAS.md`), boa parte do rack
  visível carrega sempre os mesmos números quando não participa daquele
  desenho de cabos.
- **E o ponto mais importante para a segunda observação do autor: tudo
  isso acontece UMA VEZ, no instante do reseed.** Depois disso, o que
  muda ao vivo é só:
  1. o `drift` interno de cada módulo — um passeio pequeno, limitado,
     **cego ao contexto** (não sabe se a peça está "subindo" ou
     "descendo", não se correlaciona com outro módulo a não ser onde um
     `DRIFT` dedicado foi explicitamente cabeado);
  2. a modulação por cabo já plugada nesse seed (fixa desde o reseed —
     não aparece nem desaparece cabo sozinho);
  3. o `DRIFT` module com `anchor` (feito hoje, 2026-09-04) — memória de
     topologia, o mais perto que o motor chega hoje de "orbitar" em vez
     de vagar, mas ainda é só 4 saídas de CV, sem saber nada sobre
     "seção" ou "tensão" da peça.

**Correções pontuais (2026-09-05)** — a partir de reportes do autor
("ainda soa engessado, com padrões de IA limitados"; "frequências
agudas que se repetem em várias seeds"). Não é a reformulação grande da
§1.1, são consertos de bug/viés medidos por sonda espectral
(`seedspec`/`seedrms`, 26 seeds):
- a seção "camadas soltas → mixer" cabeava fonte CRUA (`NOISE.white`,
  `PLL.out`, `OSC.saw`…) direto no barramento a até −2 dB → assobio/
  drone brigando com a voz (o autor: "o agudo vem do PLL, canal 2 no
  mixer", seed 45932257). Reescrita: no máx 2 camadas, −24..−11 dB, e
  **só de fonte JÁ PROCESSADA** (`FILTER`/`WASP`/`LPG`/`SPACE`/
  `PARAMETRIC`) — nunca um oscilador cru;
- `QUANTIZER.range` `1..3,5` → `1..2` oitavas + voz OSC médio até 340 Hz
  + passe genérico limitando `freq` a 40..1200 Hz + `DRIFT` não deriva
  mais `QUANTIZER.range`/`FILTER.resonance` pro extremo + `PARAMETRIC`
  da cadeia com Q ≤ 2,5 (era 5) — tudo pra a melodia/EQ não "assobiar"
  a 5–6 kHz. Sonda `sweep`: 50 seeds, 0 ásperos;
- o passeio ponderado não usa mais `MIXER`/`MASTER` como destino;
- `complexity` minimalista era hard-clampado pra 0,02 → ~15% dos seeds
  colapsavam em poucos esqueletos idênticos. Agora 0,05..0,20 (nProc e
  nCables ainda variam) → sem colapso;
- voz de ruído só rosa/marrom; voz OSC ganha registro médio às vezes
  (era sempre grave), `pw` contido; voz FILTER auto-osc menos "seno
  puro" (resonance 0,55..0,9, corte-base variado);
- rede de segurança: voz brilhante sem processador ganha um LP musical;
- passe genérico de parâmetros tira `resonance`/`grit`/`fold`/`drive`
  do extremo perigoso, limita ganho de EQ e cutoff.
Resultado: banda >10 kHz caiu de ~11% (em vários seeds) pra ~0; nenhum
par de seeds idêntico; espectro grave/médio-grave saudável na maioria.
Ver `TAREFAS.md`, registro "Motion Engine janela uniforme, MASTER 50%,
seed anti-viés-agudo".

**Passeio restrito ao subgrafo audível (2026-09-10)** — a nova vista
`RACK · SAÍDA` do painel (2026-09-09) expôs que **~62 % dos cabos** de um
seed tinham destino que não chegava à saída: fiação de exploração pelo
rack inteiro, inaudível. O autor: *"não gostaria que os cabos ficassem
sem função sonora."* O passeio ponderado e a fiação da `DRIFT` passaram a
só aceitar destino em módulo que **já chega à saída**
(`SignalGraph::nodesFeeding(sink)`, recalculado a cada cabo — o conjunto
CRESCE conforme o passeio liga uma fonte de áudio numa entrada audível),
mais uma poda final dos cabos de montagem órfãos. Menos cabos totais, mas
**mais módulos de fato soando** (complexity alta: ~16 → ~25). O stream de
RNG diverge → todo seed anterior vira um patch diferente.
`test_seed_patch` ganhou a asserção "sem cabo morto". Ver `TAREFAS.md`,
registro "seed — passeio restrito ao subgrafo audível".

**Não existe nada que orquestre uma evolução da peça no tempo** —
nenhuma seção, nenhuma curva de tensão, nenhuma decisão de "agora a
densidade sobe". As 3 peças de exemplo determinísticas
(`examples/peca_generativa*.cpp`) têm uma forma (ruptura de cabo aos
28s, reconecta aos 34s, respiração da MEMORY por posição no campo) —
mas essa forma é **hardcoded em C++** (`if (b == ruptureBlock) ...`),
escrita à mão peça por peça, não um motor generativo reutilizável.
**É exatamente essa lacuna que a §3 ataca.**

---

## 3. Aprofundando — Motion Engine / Generative Composition Layer

*(Item pedido "a fundo" pelo autor — tratamento mais longo de propósito.)*

### 3.1 Por que o `drift` por módulo não é suficiente

Cada `drift` (há em `OSC`, `NOISE`, `FILTER.spread`, `MATRIX`, `SHAPE`,
`CHORD`, `SH`, `LPG`, `MULT`... quase todo módulo tem o seu) é:
- **independente** — não sabe do `drift` do módulo vizinho, a não ser
  que algo explicitamente os cabeie juntos;
- **limitado** — passeio pequeno em torno do valor atual, não uma
  trajetória com intenção (não "sobe", não "desce", só treme);
- **cego ao tempo composicional** — não tem noção de seção, fase,
  compasso, ou de uma curva de tensão global. Um `drift` de 3 minutos
  atrás se parece exatamente com um `drift` de agora.

Isso dá **vida** (nada soa estático), mas não dá **direção** — não
existe hoje nenhum mecanismo que faça "a peça" ir a algum lugar.

### 3.2 O que a conversa propõe, mapeado contra o motor real

| Comportamento (ChatGPT) | Existe? | Onde |
|---|---|---|
| `STATIC` | trivial — é o padrão sem `drift` | qualquer módulo, `drift=0` |
| `DRIFT` | **sim** | `drift` desvio, quase todo módulo |
| `WALK` | **sim**, dentro de `DRIFT` | `DRIFT.field` — passeio com momentum |
| `ORBIT` / memória de topologia | **sim, feito hoje** (2026-09-04) | `DRIFT.anchor` — volta a marcos gravados |
| `OSCILLATE` | parcial, só por exemplo | `connectToParameter` com um `FunctionGenerator` a rate baixa (ex.: `lfoSpread` em `peca_generativa.cpp`) — funciona, mas é fiação **manual, por peça**, não "atribua OSCILLATE a qualquer parâmetro" |
| `CHASE` / `ATTRACT` / `REPEL` **entre dois parâmetros quaisquer** | **não** | não existe mecanismo genérico; o mais perto é o barramento semântico (`contributeQuality`/`followQuality`), que é 1 qualidade → N parâmetros, não par-a-par |
| `SYNC`, `PULSE`, `STEP`, `RAMP` | fragmentado | pedaços bespoke em módulos específicos (`SWITCH.mode`, `TRIGSEQ.swing/ratchet`, `ABACUS.count_step`) — nada reutilizável por qualquer parâmetro |
| `PROBABILISTIC`, `CHAOS` | fragmentado | idem — `DECISION`, `TRIGSEQ.chaos`, `SWITCH` modo random |

O barramento semântico do motor (`SignalGraph::Quality` — `Energy`,
`Brightness`, `Density`, **`Tension`**, **`Motion`**;
`src/core/SignalGraph.hpp:701`) já tem os NOMES que a conversa pede pra
`tensão` e `movimento` — mas hoje `resolveSemanticBus()` só resolve
essas qualidades **de baixo pra cima**: um nó contribui com a média do
seu bloco de áudio (`contributeQuality`), nunca são setadas **de cima
pra baixo** por uma curva composicional. Não existe hoje um jeito de
dizer "a Tensão está subindo" sem ter algum sinal de áudio real
empurrando ela — é uma peça de arquitetura que já espera por isso, mas
falta o lado que a alimenta de propósito.

### 3.3 Onde isso viveria — respeitando as convenções do motor

`rasgo_modular_core` é **zero-dep e não sabe o que é "composição"** —
essa é uma decisão de arquitetura estabelecida (o motor não deve saber
de painel, de forma musical, de nada além de grafo+DSP). Então um Motion
Engine **não entra no `SignalGraph.hpp`** — entra em `apps/panel/`,
igual o `PatchSeed.hpp`: um consumidor da API pública do grafo.

A peça que destrava isso de verdade é o que fizemos hoje cedo (Task A,
`RM-ENGINE-ADDITIVE-MOD`): `graph.setParameterBase(node, id, valor)`
agora **atualiza a base sob a modulação existente**, em vez de brigar
com ela. Um Motion Engine que mexe num parâmetro via
`setParameterBase()` se comporta **exatamente como um giro de knob** —
qualquer cabo/modulação já plugado nesse parâmetro continua somando por
cima, sem conflito. Sem essa aditividade (o estado de ontem), um Motion
Engine ligado num parâmetro já modulado por cabo teria apagado a
modulação a cada tique — a base ficaria brigando com o cabo.

Esboço de forma (não implementado):

```
struct MotionBinding {
    node, paramId
    behavior   // STATIC/DRIFT/WALK/OSCILLATE/ATTRACT/...
    range[lo,hi]  // dentro dos limites do próprio ParameterDescriptor
    rateHz
    state...   // por tipo de comportamento
};

class MotionEngine {
    std::vector<MotionBinding> bindings;
    void tick(SignalGraph& g, float dt);  // roda em taxa baixa (UI ou timer),
                                            // nunca em process() — sem custo RT
        // pra cada binding: calcula o próximo valor, chama
        // g.setParameterBase(b.node, b.paramId, valor)
};
```

`ATTRACT`/`REPEL`/`CHASE` entre dois parâmetros só precisam que o
`MotionBinding` de um leia o `parameterUserValue()` do outro — já
exposto pela aditividade de hoje.

### 3.4 Form Engine — seções e tensão

Um `Composer` acima do `MotionEngine`: uma máquina de estados de seções
(`INTRO → DEVELOP → EXPAND → ...`, com transições opcionalmente
probabilísticas, como a conversa desenha), e uma curva de `tension`
(0..1) que cada seção define como alvo. A tensão **não seta parâmetros
diretamente** — ela é uma "intenção" que o `MotionEngine`/o barramento
semântico traduzem: `range`/`rate` de cada `MotionBinding` pode ser
função de `tension` (mais tensão → faixas maiores, `rate` mais alto,
mais chance de `PROBABILISTIC` disparar), e/ou `tension` alimenta
`Quality::Tension` do motor via um "nó fantasma" que contribui um valor
fixo por bloco (usar `contributeQuality` como está hoje, sem mudar o
motor) — caminho que **não pede nenhuma mudança no `SignalGraph.hpp`**.

### 3.5 Menor passo testável, se/quando formos implementar

Recomendação (não decisão — é só o caminho de menor risco): prototipar
um `MotionEngine` com **2-3 comportamentos** (`WALK`, `OSCILLATE`,
`ATTRACT`) sobre um punhado de parâmetros escolhidos à mão, numa 4ª
peça de exemplo (`peca_generativa_4.cpp`?) — validar a forma da API
(`setParameterBase`-based) antes de generalizar pra "qualquer parâmetro,
qualquer comportamento, configurável pelo `Seed`". Isso segue o mesmo
padrão incremental que o resto do projeto usou (cada módulo: dossiê →
código → teste isolado → integração).

### 3.6 Protótipo feito (2026-09-04)

O passo da §3.5 foi construído, exatamente na forma esboçada:

- **`apps/panel/MotionEngine.hpp`** — `MotionEngine::Binding` com 3
  comportamentos (`Walk`, `Oscillate`, `Attract`); `tick(graph, dt)`
  escreve via `SignalGraph::setParameterBase()`. Cada `Binding` carrega
  sua própria seed xorshift → determinístico. Fica em `apps/panel/`
  (como o `PatchSeed.hpp`) — o core continua sem saber o que é
  "composição".
- **`examples/peca_generativa_4.cpp`** (42 s) — 4 ligações: `WALK` em
  `filter.resonance` (deriva com alvo, ao contrário do `drift` cego de
  módulo); `OSCILLATE` em `env.curve` (a forma do envelope "respira");
  `ATTRACT` em `voice.slope` perseguindo `filter.resonance` (dois
  parâmetros de módulos DIFERENTES coreografados); `WALK` em
  `filter.spread` **por cima** da modulação já existente por cabo
  (`lfoSpread → filter.spread`, `connectToParameter`) — a prova viva da
  aditividade.
- **`tests/test_motion_engine.cpp`** (5 funções) — `WALK` fica na faixa
  e visita valores diferentes; `OSCILLATE` varre entre os limites;
  `ATTRACT` converge pro alvo (outro módulo); **a aditividade** (uma
  base do `MotionEngine` + uma modulação por cabo no MESMO parâmetro
  somam sem se apagar — testado numericamente, não só por escuta);
  determinismo.

**Verificado:** som real (RMS varia 0,10→0,20 ao longo da peça, medido
em janelas de 7 s — não é um drone estático), determinístico (2 renders
byte-idênticos), **36/36 CTest** Debug + Release, 0 warnings, os 4
renders anteriores continuam byte-idênticos (nenhum exemplo antigo usa
o `MotionEngine`). `peca_generativa_4.wav` entrou em `validation-output/`
como referência.

**O que isto NÃO é ainda:** não há `Form Engine`/seções/`tension` (§3.4);
os 4 `Binding` da peça de exemplo são fixados à mão no código C++, não
gerados pelo `Seed` — isso continua em aberto. **Atualização
(2026-09-04): passou a estar integrado ao painel interativo também**
(`panel_main.cpp`, `[v]`) — `populateMotion()` escolhe 1 parâmetro por
módulo mostrado (hash determinístico tipo+id, não à mão como na peça de
exemplo), então a prova de arquitetura da peça JÁ generalizou pro caso
"qualquer patch aberto no painel", só não pro caso "o `Seed` decide os
`Binding`s" (§3.5, ainda em aberto).

**Correção (2026-09-05):** a janela de movimento no painel era o RANGE
INTEIRO do parâmetro — um `OSCILLATE` varria o controle de ponta a
ponta, um `WALK` mal saía do lugar (o autor: "o 6º slider do SEQUENCE se
move muito mais que os outros"). Passou a ser uma fração IGUAL do range,
centrada no valor atual (`Binding::start`) — o vício era a janela
**não-uniforme**, não o slider em si.

**Ajuste (2026-09-06):** a ±6% ficou imperceptível ("os knobs e sliders
variavam mais, sem precisar clicar num botão"). Agora **±20%**
(`kMotionDepth = 0.40`), ritmo ~0,03–0,12 Hz (alvo novo a cada ~8–33 s).

**Bug de fundo, mesmo dia:** a escolha de QUAL controle anima por módulo
era um hash de `tipo + id` — e como o catálogo é instanciado inteiro pra
todo patch, o `id` de cada módulo é fixo → **sempre o mesmo controle**
animava, em todo seed. Pro `SEQUENCE` isso era o "6º slider": o hash
caía sempre no mesmo passo e um passo derivando reafina a melodia.
Correções:
- o hash passa a misturar **`curSeed`** — cada patch generativo respira
  por um controle diferente, nenhum vira "o que sempre mexe" (editado à
  mão, `curSeed == 0` → estável);
- **módulos de partitura fora da Motion Engine**: `SEQUENCE`, `TRIGSEQ`,
  `TURING`, `HARMONY`, `QUANTIZER` — a camada mexe no timbre/textura,
  nunca nas notas. `MUTATE`/`EVOLVE` continuam reembaralhando a partitura
  sob demanda;
- **volta a ser só KNOB** (os sliders são a partitura ou fader de nível).

`test_motion_engine` e `peca_generativa_4` intocados (usam `MotionEngine`
direto, não `populateMotion` → compat, renders byte-idênticos). Ver
`TAREFAS.md`.

### 3.7 Motion Engine v3 — a mão caótica que toca o patch (spec, 2026-09-07)

**Motivação (feedback do autor):** o `populateMotion` de v2 (§3.6) anima
**1 knob por módulo**, sorteado por hash, com janela fixa de ±20%. É
conservador demais (a maioria dos knobs fica congelada pra sempre) *e*
quando sorteia mal cai num `pw` (PWM largo demais → "frenético") ou num
`DECISION.steps` (estrutural → "se move sem nada cabeado"). O autor
quer: **versátil** ("a princípio pode ser qualquer controle"), que
**saiba o que está fazendo**, com **uma razão pra alterar o que altera**
— "desde que seja musical".

**Direção do autor (2026-09-07), depois de rejeitar duas versões
baseadas em tabela:** *"é um instrumento de composição, não de regras
prontas, de determinismos congelados, de padrões limitados de IA. Um
instrumento que busca a excelência de composição, o inaudito, o bom
gosto musical, a experimentação. Sei que alguns itens são subjetivos,
mas é necessário encontrar um caminho."*

Ou seja: **nada de lista de intenções + matriz de qualidades + camadas
de substring.** Isso é rulebook. O caminho é um instrumento que
**explora** — determinístico a partir do seed (o render tem que ser
reproduzível) mas de trajetória **longa e não-óbvia**, que nunca vira um
loop audível nem congela.

#### 3.7.1 O modelo — a "mão" caótica que toca o patch

**Estado:** um sistema caótico pequeno de baixa dimensão (a matemática do
`CHAOS` — integrador de 1ª ordem tipo Rössler/Thomas, 3–4 variáveis
acopladas). Ele NUNCA repete, NUNCA para, é levemente imprevisível.
Semeado → reproduzível; mas o período é longo demais pra o ouvido pegar
um ciclo.

**Acoplamento aos knobs:** cada parâmetro animável ganha um **vetor de
projeção** próprio (pesos semeados) sobre as variáveis do estado caótico
→ `alvo(param) = centro + Σ wᵢ·estadoᵢ · amplitude`. Fonte compartilhada
→ os knobs se movem em **relação** (sai coerente por construção); projeção
distinta → cada um traça um caminho seu. Não há "agora está clareando"
previsível — há um gesto contínuo que vai a algum lugar interessante.

**Realimentação (o "ouve o que sai"):** a velocidade global do estado
caótico é modulada pela **energia/tensão medida do som** (o barramento
semântico `Quality`, já bottom-up hoje — `Energy`, `Tension`, `Motion`).
Música intensifica → a mão se move mais; acalma → recua. É o loop
fechado: o *inaudito* vem daí — do patch reagindo à própria
não-linearidade, não de um script.

**Gosto = amplitude (a única "regra", e é mínima).** 4 pistas de palavra
no `id` decidem só QUANTO cada knob pode andar:
- `step`/`length`/`slice`/`scale`/`root`/`bpm`/`mult`/`ratio`/`voices`/
  `pattern`/`heads` → **estrutural**: passo pequeno, quantizado ao
  descriptor, e RARO (a projeção quase não o toca);
- `gain`/`level`/`output`/`master` → **nível**: ±3%, clamp duro;
- `res`/`feedback`/`drive`/`fold`/`grit`/`crush`/`fm_amount`/`pw`/
  `index` → **quente**: ±5–8% (respira, não varre);
- resto → **livre**: ±15–25%.
Nenhum é proibido. `id` desconhecido → livre com metade da amplitude.
A janela é sempre em torno do valor ATUAL do knob (o que o seed/o músico
deixou); as bordas do `ParameterDescriptor` são limite absoluto.

**Únicos módulos isentos (decisão do autor, 2026-09-07): `MIXER` e
`MASTER`.** São o estágio de mistura/saída — o músico controla balanço e
pans na mão. Todo o resto entra. Além disso: só **KNOB** é animado —
sliders são partitura (notas do `SEQUENCE`) ou fader de nível, nunca uma
regulagem que derive sozinha.

**Ousadia (a experimentação):** de tempos em tempos (Poisson semeado,
raro) a projeção de UM parâmetro é amplificada por alguns segundos — um
alcance maior, uma tentativa — e depois relaxa. Sem avaliação
automática de "ficou bom" na v1 (isso é subjetivo demais); a aposta é
que a mão caótica + a realimentação já produzem material que vale ouvir,
e o músico corta com `[v]` ou com `MUTATE` se não gostar.

#### 3.7.2 Sabe do patch

- **param já com cabo/modulação entrando** → amplitude pela metade (não
  briga com a fiação do seed/do músico; não apaga — `setParameterBase` é
  aditivo);
- **módulo que não chega ao `Out` ativo** → amplitude zero (não adianta
  mexer no que não soa) — se a API de alcançabilidade não existir ainda,
  fica como pendência e anima tudo por ora.

#### 3.7.3 Musical / seguro por construção

- **coerência:** um estado caótico → todos os knobs em relação; o ouvido
  lê um gesto, não 30 tremores;
- **sem deriva acumulada:** a janela é sempre em torno do valor do seed,
  com limite do descriptor — não "foge" com o tempo;
- **estrutura protegida:** `steps`/`scale`/`bpm` mal se movem, e
  quantizados; `MUTATE`/`EVOLVE` seguem sendo o jeito deliberado de
  reembaralhar a partitura;
- **determinístico:** o estado caótico + as projeções + o RNG da ousadia
  são semeados; `dt` fixo do chamador → 2 sessões = a mesma trajetória
  (renders de exemplo byte-idênticos);
- **light-touch** (`feedback_generative_design_light_touch`): ~4 pistas
  de palavra e um integrador de 3–4 linhas. O resto é a dinâmica.

#### 3.7.4 Forma / escopo

`apps/panel/MotionEngine.hpp` — o `Binding` manual-only FICA
(`peca_generativa_4`, `test_motion_engine` usam direto). Adiciona-se:
`MotionField` (o estado caótico + realimentação) e
`MotionEngine::inhabit(graph, seed, shown)` — varre o grafo, monta uma
projeção por knob com a amplitude da pista de palavra. `tick()` avança o
`MotionField` e escreve os alvos. `populateMotion` do `panel_main.cpp`
vira `motion.inhabit(...)`. Pista-de-palavra + a montagem da projeção
num header testável (`apps/panel/MotionField.hpp`?).
`tests/test_motion_engine.cpp` cresce: a trajetória não repete numa
janela longa; estrutural mal se move; não briga com cabo; 2 renders
byte-idênticos. `rasgo_modular_core` intocado.

**Evolução — o `morceau` (autor, 2026-09-07):** *"boa parte dos
instrumentos RASGO já pensa num formato final de música (morceau) — com
início, desenvolvimento (várias etapas) e coda. O VARIA poderia
encontrar seu sentido original nisso."* O caminho é uma **camada de arco
por cima da mão caótica** (o Form Engine da §3.4), na linha do que o
`RASGO_SYNTH/rasgo-synth-performance` já coda:

- **caminhada de Markov sobre papéis estruturais** — `intro / subida /
  clímax / queda / coda`, a `coda` como estado ABSORVENTE (a peça só
  termina nela); a duração total é consequência da caminhada, não um
  valor a priori (`RASGO_SYNTH/docs/pt/_catalogo_mecanismos.md`,
  "Tempo discreto com ritardando real na coda"; Doc. de Referência §4.2);
- **gramáticas plugáveis** — Dramatic (pirâmide de Freytag) e
  Kishōtenketsu;
- **andamento** — saltos discretos (salto-e-espera) nas transições de
  arco; **accelerando** exponencial real rumo ao 1º clímax, **ritardando**
  exponencial real na coda (sinalizar chegada/fechamento);
- **cortes secos** em transições específicas (sem interpolação).

No RASGO_MODULAR isso vira: o papel do arco define a INTENÇÃO macro
(subida → a mão puxa brilho/tensão/densidade pra cima e acelera; queda →
o oposto; coda → `ASSENTAR` + ritardando de qualquer `CLOCK`/`rate`), e a
mão caótica segue sendo o detalhe/textura por baixo. O `RASGO_MODULAR`
**não precisa** da camada de álbum (conjunto de morceaux) — só do
morceau. Prior art a estudar (não copiar código; `RASGO_SYNTH` é
read-only): a caminhada de Markov + as duas gramáticas + o
accelerando/ritardando exponencial.

**Outras evoluções:** avaliação do material ("ficou bom?") via as
`Quality` medidas — manter a excursão que melhorou a direção pretendida,
reverter a que não; `ATTRACT`/`REPEL` entre params.

---

## 4. Patch Genetics — `MUTATE` / `EVOLVE` / `CROSS` / `FREEZE`

**Prototipado em 2026-09-04, `CROSS` em 2026-09-05** —
`apps/panel/PatchGenetics.hpp` + `tests/test_patch_genetics.cpp` (11
funções).

- **`mutatePatch(graph, seed, fraction, frozen)`** — reaplica a mesma
  ideia do passo genérico de randomização do `seedPatch()` (§2.2): pra
  cada nó alcançado por cabo (exceto os em `frozen`), reamostra
  ~`fraction` dos parâmetros não-estruturais na faixa TOTAL. Não toca em
  topologia — nenhum cabo muda. Escreve por `setParameterBase()`
  (aditivo — não apaga modulação por cabo já plugada no mesmo
  parâmetro, testado numericamente). Determinístico por seed.
- **`evolvePatch(graph, seed, steps, fractionPerStep, frozen)`** —
  várias `mutatePatch` pequenas em sequência, cada passo com sua própria
  derivação do seed (reproduzível passo a passo).
- **`FREEZE`** — não é estado guardado em lugar nenhum: é só o
  `std::unordered_set<std::size_t>` de nós que quem chama passa em
  `frozen`. O chamador decide o que proteger.
- **`crossPatch(target, donor, seed, fraction, frozen)`** (2026-09-05) —
  a peça que era "a mais arriscada da lista" porque precisava de uma
  noção de alinhamento entre dois grafos. Resolvido por TIPO de módulo,
  não por índice: a k-ésima ocorrência de um tipo no ALVO casa com a
  (k mod contagem-no-doador)-ésima ocorrência do MESMO tipo no DOADOR —
  um `FILTER` só no doador ainda cruza com os 3 `FILTER` do alvo, se for
  o caso. Um nó do alvo sem tipo correspondente no doador fica intocado
  (nunca caça substituto de outro tipo). Mesma simplificação de
  `MUTATE`: só parâmetro, nunca cabo/topologia — crossover de
  TIMBRE/CARÁTER, não de forma.

**Limitação medida (sonda, 30 seeds × 5 `MUTATE` em sequência, fração
0,3):** **4/30 patches ficaram mudos** — ao contrário do `seedPatch()`,
`MUTATE` não protege uma "espinha"; reamostra qualquer parâmetro
não-estrutural sem saber que é crítico pro som sair, então uma sequência
de mutações pode derrubar um nível/mix perto de 0 por acaso. Não é bug —
é o preço de MUTATE não ter espinha (decisão de design: a espinha só
existe em tempo de `seedPatch`, quando a topologia está sendo desenhada;
recriar essa noção pra um grafo já em uso ao vivo exigiria rastrear o
caminho ativo até o sink a cada chamada). Mitigação recomendada:
`FREEZE` os nós de saída (`MIXER`/`MASTER`) e/ou usar `fraction` pequena
em uso ao vivo — **implementada** (ver abaixo).

**Atualização (2026-09-04): integrado ao painel interativo.** `[m]`
(MUTATE, fração 0,25) e `[e]` (EVOLVE, 6×0,12) em `panel_main.cpp`, os
dois com FREEZE automático de `MIXER`/`MASTER` — a mitigação acima,
aplicada sem exigir nada do usuário. Sonda com o freeze: **0/30 mudos**
(30 seeds, mesma metodologia da sonda original que mediu 4/30 sem
freeze). Não tem UI de seleção manual pra `FREEZE` (o congelamento é
automático e fixo em `MIXER`/`MASTER`, não escolhido pelo usuário nó a
nó) — isso continua em aberto, se algum dia for útil.

**Atualização (2026-09-05): `CROSS` integrado ao painel também.** `[c]`
em `panel_main.cpp` — cruza o patch atual com um DOADOR novo (catálogo
inteiro semeado com um seed aleatório fresco), fração 0,5 (crossover
clássico), mesmo FREEZE automático de `MIXER`/`MASTER`. Sonda com o
freeze, mesma metodologia de `MUTATE`/`EVOLVE`: **0/30 mudos**.

---

## 5. RASGO Score — partitura / registro de eventos

Três níveis, como a conversa propõe: `MUSICAL SCORE` (notas/ritmo/
dinâmica, exportável), `SYSTEM SCORE` (patch + modulação + mutação),
`RASGO SCORE` (os dois juntos). Paralelo conceitual ao que o
RASGO_SYNTH_STUDIO já fez com o RASGO Musical Event — RASGO_SYNTH é
**read-only**: consultei o conceito, não copiei código.

**`SYSTEM SCORE` prototipado em 2026-09-04** —
`apps/panel/ScoreRecorder.hpp` + `tests/test_score_recorder.cpp` (4
funções) + integrado em `examples/peca_generativa_4.cpp`:

- `RasgoEvent` com 3 tipos: `Connection` (cabo), `Modulation`
  (`connectToParameter`), `ParameterChange` (giro de knob/`MotionEngine`/
  `PatchGenetics`). `time` é **sempre** `amostra/sr` — quem chama passa,
  o `ScoreRecorder` nunca lê relógio de parede;
- `toText()` — formato de texto próprio, uma linha por evento
  (`t=0.000000 connection 0:1 -> 1:0`, `param 5.resonance 0.30 -> 0.40`,
  e agora `note 9 pitch=0.5 velocity=0.8 duration=0.333 accent=1`),
  não MusicXML/MIDI ainda (ver "o que ainda falta" abaixo);
- na peça 4: as 7 conexões/modulação iniciais em `t=0`, mais as
  mudanças de parâmetro do `MotionEngine` **acima de um limiar** (0,03 —
  senão seria um dump em taxa de controle, ~15 mil linhas por
  parâmetro, não um registro de eventos legível). Resultado: 68 eventos
  numa peça de 42 s. **O texto da partitura é byte-idêntico entre
  renders** (`validation-output/peca_generativa_4.score.txt`) — tão
  determinístico quanto o áudio.

**Atualização (2026-09-05): gravação ao vivo + `MUSICAL SCORE` (captura)
feitas.**
- **gravação AO VIVO no painel** — **feito**: `[Ctrl+R]` grava
  `SYSTEM SCORE` de verdade (cabos existentes no início da tomada +
  toda mudança de parâmetro feita à mão enquanto grava), escreve
  `rec-NN.score.txt` ao lado do `.wav`;
- **`MUSICAL SCORE` (notas) — captura feita**, sem mudar a interface de
  NENHUM módulo existente: `NOTE-OUT` (`src/dsp/NoteOut.hpp`, Módulo
  38) é um observador que você cabeia em GATE+PITCH onde quiser — detecta
  nota-liga/desliga sozinho, `panel_main.cpp` lê a nota completa a cada
  bloco e grava no `ScoreRecorder` (`RasgoEvent::Type::Note`, novo). A
  ideia original ("mudaria a interface dos módulos") era desnecessária —
  um adaptador observador resolve sem tocar em `ENVELOPE`/`SEQUENCE`/etc.
  Precisa ficar EM LINHA no patch pra rodar (confirmado por sonda: fora
  do caminho do sink, o motor nunca chama seu `process()`) — mesma
  exigência de qualquer utilidade do catálogo.

**O que ainda falta:**
- **export MusicXML/MIDI** — só a captura está feita; converter
  `pitch` (1V/oct cru) pra nome de nota/MIDI e escrever um arquivo de
  verdade continua em aberto;
- **polifonia** — `NOTE-OUT` v1 é monofônico (1 nó, 1 voz por vez);
  várias notas simultâneas (tipo `CHORD`) pediria vários `NOTE-OUT` ou
  uma versão nova;
- **bidirecional (`SCORE → RASGO`)** — a parte mais distante: precisa
  reconstruir/comparar contra um grafo a partir do texto — nem
  desenhado.

---

## 6. Learning Engine — hover-learn no painel

Precedente citado pelo autor: **Antitotem** e **Navalha 2** (hover sobre
controles/jacks → explicação). Camada de UI pura, não toca no motor.

**Protótipo feito em 2026-09-04** —
`apps/panel/LearnCatalog.hpp` + `tests/test_learn_catalog.cpp` (4
funções) + integrado em `panel_main.cpp`:

- `lookupLearn(moduleType, bind)` — tabela `moduleType → bind →
  LearnEntry{quick, understand, explore}` (mapeando os
  `short`/`basic`/`listening`/`experiment`/`concepts` da conversa em
  3 campos); `nullptr` quando não há conteúdo — silencioso, não erro;
- a **caixa Learn estilo terminal** fica no rodapé da coluna esquerda
  (a lista de módulos encurta pra caber) e mostra `quick` +
  `understand` + `explore` do knob/jack sob o mouse, com quebra de
  linha; parada, mostra só uma linha-guia. Modelo do ANTITOTEM
  (`learnEditor`), pedido do autor 2026-09-05: "uma caixa tipo terminal
  no canto inferior esquerdo … para evitar que as caixas de texto se
  sobreponham ao painel" — o tooltip flutuante antigo tapava justo o
  módulo que você inspecionava. Hit-test por `footprintPx`, o mesmo do
  clique;
- **sempre presente** (autor 2026-09-05: "ela pode ficar sempre ligada …
  a tecla l pode sumir") — sem toggle `[l]`, sem estado `learnMode`; a
  paleta só reserva os ~170 px do rodapé. Continua **silenciosa** — nunca
  fala sozinha, texto só ao passar o mouse — que era o princípio do
  Antitotem/Navalha (não atrapalhar quem já conhece o instrumento); a
  antiga faixa "MODO APRENDER — passe o mouse…" saiu, isso é assunto de
  tutorial, não de cabeçalho;
- **conteúdo completo desde 2026-09-05** — os 37 tipos de módulo do
  `moduleCatalog()` têm entrada, todo knob e todo jack de todo módulo
  instanciável no painel (confirmado por teste dedicado,
  `testRemainingCatalogModulesHaveAtLeastQuick`, que resolve cada bind
  exato tirado do `panel()` de cada `src/dsp/*.hpp`). `quick` está
  preenchido em tudo; `understand`/`explore` são mais densos nos
  mecanismos que valem explicar (ex.: por que o `feel` do `CLOCK`
  precisa de RNG isolado, o alcance de captura do `PLL`) e ficam vazios
  em parâmetros autoexplicativos — o mesmo padrão de "não escrever
  prosa por escrever" do `MODULE_DEVELOPMENT_STANDARD`.

**Definição de MÓDULO no hover do corpo/título (2026-09-07):** passar o
mouse sobre o corpo de um módulo do rack (não sobre um knob) mostra na
caixa LEARN o que o MÓDULO É — `lookupLearnModule(type)` →
`LearnEntry{quick = o que é, understand = o lugar dele / como difere dos
vizinhos, explore = uma cadeia pra experimentar}`. Um por tipo do
`moduleCatalog()` (`AUDIO-IN` reusa `SIGNAL-IN`); gate de regressão em
`testEveryCatalogModuleHasBlurb`. **Dwell encurtado de 2 s → 1 s** (mesmo
pedido).

**O que ainda falta:** `WHY?`/`WHAT IF?` contextuais (precisam saber o
que já está cabeado, não só o parâmetro isolado) — os 3 níveis
(`quick`/`understand`/`explore`) já aparecem na caixa. Conteúdo e
exibição completos; o resto é a camada contextual.

**Não pude testar visualmente** (o painel gráfico trava a máquina do
autor ao rodar por aqui — ele testa com `.run_rasgo_modular.sh`); a
verificação foi: compila limpo (`-Wall -Wextra`), o teste do catálogo
passa, e os 5 renders de exemplo continuam byte-idênticos (a mudança é
só de desenho, nenhum módulo/DSP foi tocado).

---

## 7. Estado — ordem sugerida (não decidida)

1. **Motion Engine** (§3) — **protótipo feito** (§3.6, 2026-09-04),
   **integrado ao painel interativo** (2026-09-04, `[v]` liga/desliga):
   cada módulo mostrado ganha até 1 parâmetro em `WALK` lento — resposta
   direta a "cada módulo fica estático" (feedback do autor). Continua
   faltando generalizar (o `Seed` decidir os `Binding`s, não uma escolha
   fixa de 1-por-módulo) e as famílias `REPEL`/`ORBIT`/`SYNC` (§1.1).
2. **Patch Genetics** `MUTATE`/`FREEZE`/`EVOLVE`/`CROSS` (§4) —
   **protótipo feito** (2026-09-04, `CROSS` 2026-09-05) e **integrado ao
   painel interativo** (`[m]` MUTATE / `[e]` EVOLVE / `[c]` CROSS), com
   FREEZE automático de `MIXER`/`MASTER` — a mitigação que reduziu a
   limitação medida (mutação sem espinha emudecia ~13% dos casos sem
   freeze) pra **0/30 numa sonda igual, com freeze** (mesmo resultado pro
   `CROSS`). `SIMPLIFY`/`COMPLEXIFY`/`VARIATION` continuam de fora.
3. **RASGO Score** (§5) — **`SYSTEM SCORE` prototipado** (2026-09-04,
   integrado ao painel — `[Ctrl+R]` grava de verdade) + **`MUSICAL
   SCORE` (captura) feita** (2026-09-05, `NOTE-OUT`, Módulo 38 — sem
   mudar a interface de nenhum outro módulo, um observador adaptador
   resolveu). Export MusicXML/MIDI, polifonia e o caminho bidirecional
   continuam de fora.
4. **Learning Engine** (§6) — **protótipo feito** (2026-09-04),
   **conteúdo completo + exibição** (2026-09-05): caixa terminal sempre
   presente no rodapé da paleta, os 37 módulos do catálogo, os 3 níveis
   (`quick`/`understand`/`explore`) desenhados. O que resta é só a camada
   contextual (`WHY?`/`WHAT IF?`), não mais redação nem layout.
5. **Gramática do Seed nomeada e explícita** (§1.1) — **feita**
   (2026-09-05, `apps/panel/SeedGrammar.hpp`): extração/documentação da
   classificação de porta e da matriz de compatibilidade que já existiam
   dentro de `seedPatch()`, sem reorganizar pras 6 categorias por módulo
   (regressão de precisão, ver §1.1) — comportamento idêntico verificado
   (`serialize()` byte a byte, seeds 1–200). A reformulação real de
   `seedPatch()` em torno de `SOURCE→TRANSFORM→CONTROL→MODULATE→
   FEEDBACK→OUTPUT` como estrutura de dados variável (não só nomeada)
   continua de fora — é maior que este passo, decisão de escopo em
   aberto.

**Os 4 itens da lista têm protótipo, mais a gramática do Seed nomeada.**
Nenhum está "pronto" — cada um tem lacunas documentadas na seção
correspondente (§3.6, §4, §5, §6) — mas a arquitetura de cada peça está
validada por teste, não só por papel.

Nenhuma ordem aqui é compromisso — é só a leitura de dependências.
Aesthetics/mecânica nova sempre passa por descrição + proposta +
aprovação antes de virar código (convenção do projeto).
