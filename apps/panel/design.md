# Design — painel do Rasgo Modular (`apps/panel/`)

**Escopo:** interface do instrumento — decisões **específicas do Rasgo
Modular** (não regra RASGO comum; promoção ao comum só via G4).
**Estado:** `exploration` — wireframe funcional + concept da experiência;
não é fonte de identidade visual (essa é thread do autor, Atlas §26/§27).
**Bases lidas:** `RASGO_DOCUMENTATION/design/` — `README.md`,
`INTERFACES_E_LAYOUTS.md`, `IDENTIDADE_VISUAL.md`,
`AUDITORIA_MIGRACAO_RESPONSIVA.md`, `PRODUTO_HARDWARE_E_ELETRONICA.md`;
sibling `AQUORBIUM/WINDOW_LAYOUT_DESIGN.md`, `ANTITOTEM/docs/DESIGN.md`.
Brief: `RASGO_DOCUMENTATION/templates/BRIEF_DE_DESIGN.md`.
**Versão:** Rasgo Modular marco 2.

---

## 1. Gesto central

O Rasgo Modular **soa sozinho** (§35.4.1): não há "play", o som começa
com a janela. Tocar = **modular o patch que já soa** e **montar o patch**
(escolher e conectar módulos). O painel materializa isso: arrastar um
controle e ouvir; arrastar um módulo do catálogo pra a case.

## 2. Conceito da experiência (visão)

### 2.1 Case Eurorack que quebra em linhas

Gramática Eurorack (`INTERFACES_E_LAYOUTS §3.1`) — não imitação de um
fabricante nem de um painel físico específico, mas **na proporção real do
formato** (§3.2): a mecânica do Eurorack (3U × HP) é escalada por um
fator único pra caber bem na tela:

- cada módulo é uma caixa legível, na **ordem do fluxo de sinal**;
- **altura padrão** por módulo = o 3U real (`128,5 mm · s`); só a
  **largura** varia, `HP · 5,08 mm · s`. Módulos com poucos controles têm
  espaço vazio — é autêntico e mantém o ritmo visual;
- a "case" é uma fileira que **quebra em várias linhas** quando não cabe
  na largura da janela — melhor aproveitamento da tela (largura **e**
  altura). Rola na vertical se ultrapassar a janela;
- controles de performance junto das entradas/saídas que transformam
  (é o que o `panel()` de cada módulo já declara);
- jacks rotulados e **cabeáveis** (ver §2.5);
- visão simples por padrão; matriz/constelação/semântica são superfícies
  alternativas opcionais (`RASGO_MODULAR.md §36.2`).

**Débito da gramática:** "toda conexão exposta declara fonte, destino,
tipo de sinal e profundidade" — hoje o cabo mostra fonte/destino/tipo
(cor); falta a *profundidade* (atenuador no cabo) e a modulação
saída→parâmetro pela UI.

### 2.5 Cabeamento jack-a-jack (decisão do autor, 2026-09-02)

A ideia anterior de **descartar** o cabo jack-a-jack foi **revertida**. O
autor decidiu mantê-lo como superfície primária:

> "menos abstrato, como viés pedagógico funciona bem, além de haver já
> uma cultura entre os músicos de gostarem de cabear os módulos de
> diversas formas diferentes."

**Implementado no painel de teste:**
- puxar um cabo de um jack a outro (`SignalGraph::connect` /
  `disconnect`); solta num jack de **polaridade oposta** = conecta;
- **afordância** (pedido do autor): ao segurar uma ponta, os destinos
  **válidos acendem** com halo laranja:
  - **halo DUPLO** (dois anéis) = destino ideal — **mesmo tipo de sinal**
    (áudio→áudio ou controle→controle);
  - **halo simples** (um anel) = válido, mas **cruzando tipos** (áudio→
    controle ou vice-versa) — funciona, mas você está misturando
    domínios de sinal;
  - polaridade errada (saída↔saída, entrada↔entrada) **apaga** (fica
    quase invisível, sem rótulo).
  Assim quem não sabe onde ligar vê na hora;
- pegar a ponta de um cabo já ligado (clicar no jack de entrada) =
  desliga e re-ancora na saída de origem;
- **botão direito** num jack tira o(s) cabo(s);
- **ciclo** vira conexão de *feedback* automaticamente (o motor recusa
  ciclo sem `feedback=true`; o painel tenta de novo com a flag);
- porta de entrada já ocupada = o cabo novo **substitui** (fan-in
  continua explícito no motor, via `Sum`);
- `[espaço]` rompe/religa **todos** os cabos (cicatriz sonora, Atlas §37);
- cabo desenhado como bezier com barriga; rompido = tracejado, cor de
  aviso;
- **cor do cabo varia** — como um saco de cabos de patch: 4 tons quentes
  pra cabo de **áudio**, 4 frios pra cabo de **controle** (o tipo da
  porta de origem); a cor de cada cabo vem de um hash determinístico das
  pontas, então é estável. Dá vida à case e ainda dá uma dica do tipo de
  sinal (o débito "toda conexão declara o tipo", §2.1).

**Jacks — o soquete fêmea.** O nome: **jack** (ou "jack socket" / soquete
de 3,5 mm mono, "P2"); o plugue macho no cabo é o "P2 plug". No painel:
- **rótulo ACIMA do jack** — o cabo sai por baixo, com barriga, e
  esconderia um rótulo desenhado embaixo;
- **espaçamento de ~12 mm** entre jacks vizinhos (era ~9,5–11) — folga
  pra os plugues não se cobrirem; módulos com 5 saídas (`NOISE`, `OSC`)
  ganharam largura pra isso.

### 2.6 Mover e remover módulos (2026-09-02)

- **arrastar o corpo do módulo** (nem controle nem `[x]`) → reposiciona
  na case; a ordem de leitura sob o cursor vira a nova posição no fluxo.
  **Os cabos acompanham, nada desconecta** (reordena `shown`, refaz o
  layout; os cabos são desenhados das posições de jack a cada quadro).
- **`[x]`** no canto superior direito do módulo, **ou soltá-lo na
  paleta** (esquerda) → remove. Só os cabos que tocam esse módulo são
  desligados; os outros ficam. Simétrico com "arrastar da paleta pra
  criar".
- v1: o nó removido fica órfão no grafo (silencioso) — `SignalGraph` não
  tem `removeNode` ainda (renumeraria ids). Aceitável num painel de
  teste; anotado.

### 2.7 Modulação de parâmetro — o knob fica vivo

Desde 2026-09-04 `SignalGraph::connectToParameter` (e `followQuality`)
são **aditivos**: `param = base + offset + depth·fonte`, com `base` = o
valor do knob. Todo giro de knob / clique de toggle no painel passa por
`SignalGraph::setParameterBase()`, que atualiza a base — girar um knob
num parâmetro modulado não é mais apagado. Vale também pros params **sem**
porta de entrada correspondente (`mode`, `curve`…). Ver `TAREFAS.md`.

Ainda assim, **no painel a modulação de cutoff é um CABO de verdade**
pra a entrada `cutoff_mod` (1 V/oct) do `FILTER` — não porque o motor
exija, mas porque o músico vê o cabo: o knob CUTOFF é a base, o envelope
soma por cima.

Arrasto de knob de faixa larga tipo frequência (`hi/lo > 30`, `lo > 0`)
é **exponencial** (oitavas por pixel) — linear era inútil.

**Pendente:** atenuador/profundidade no próprio cabo; modulação
saída→parâmetro pela UI pra params sem porta; clicar no meio do cabo pra
tirar; a matriz e a constelação como *views*.

### 2.9 Displays específicos por módulo (2026-09-04)

O renderizador desenha coisas mais ricas do que o `Panel` declarativo
diz — o `Display` já mostrava um osciloscópio da saída 0 que a struct
não descreve. Estendido pra três módulos (tudo só no painel, sem mexer
em DSP nem parâmetro):

- **`MATRIX`** — os 16 knobs de célula `g<jk>` **não são desenhados**;
  no lugar, uma **grade 4×4 clicável** (linha = entrada, coluna =
  saída). Cada célula mostra o ganho como barra a partir do centro
  (cima = +, baixo = −; quente = +, frio = −). Arrasto vertical numa
  célula ajusta `g<jk>` via `setParameterBase` (mesma via dos knobs —
  a modulação aditiva do §2.7 continua valendo). Os knobs seguem na
  descrição `panel()` (renderizador ASCII, contagem de testes).
- **`SCOPE`** — clicar no `Display` alterna **onda ↔ espectro**. O
  espectro é um banco Goertzel de ~24 bandas log (60 Hz–12 kHz) sobre
  o anel de amostras, calculado no `redraw` (nunca no thread de áudio),
  escala dB. Estado por módulo, só no painel (não serializa).
- **`TRIGSEQ`** — o `Display` mostra as **4 lanes de gate** (`t1`–`t4`)
  rolando como piano-roll. O thread de áudio guarda o histórico das
  saídas 0–3 (pré-alocado — nunca aloca em RT). Read-only; um overlay
  editável exigiria uma máscara de passos no módulo (mecânica nova,
  fica pra depois).

### 2.8 Salvar o patch e gravar (2026-09-02)

O trabalho do músico precisa de um lugar. Diretório de dados do usuário:
`$XDG_DATA_HOME/rasgo-modular/` (fallback `~/.local/share/rasgo-modular/`).

- **Sessão** — `session.rmp` (formato `rasgo-modular-patch 1` do motor,
  `SignalGraph::serialize`/`deserialize` + uma linha `panel shown …` com
  a ordem dos módulos na case, ignorada pelo desserializador). **Auto-**
  **carregada no arranque** (continua de onde parou) e **auto-salva na
  saída** — inclusive num `SIGINT`/`SIGTERM` (handler pede saída limpa).
  `[Ctrl+S]` salva na hora.
- **Gravação** — `[Ctrl+R]` liga/desliga; a saída estéreo é acumulada em
  memória (reserva ~4 min; auto-para quando enche) e escrita como
  `rec-NN.wav` (16-bit, `io/WavWriter.hpp`) ao parar. Faixa vermelha
  "● GRAVANDO" na barra de status.
- Factory do painel pra desserializar: `makeModule` do catálogo + o nó
  `OUT` (sink, não é módulo de catálogo).
- **Pendente:** patches nomeados (hoje só a sessão); a gravação num
  thread escritor (o `insert` no thread de áudio é memcpy limitado —
  aceitável no teste, não RT-perfeito); gravar a partir de qualquer
  saída, não só o master; exportar o `.rmp` pra compartilhar.

### 2.2 Coluna de módulos disponíveis (catálogo)

Com o tempo o Rasgo Modular terá **muitos módulos** e cabe ao músico
escolher o que acionar. Uma coluna à esquerda lista os módulos
**agrupados por família** (`RASGO_MODULAR.md §4` — SOURCE, TIME,
DECISION, SEQUENCE, TRANSFORM, MATTER, MEMORY, SPACE, MIX…). Arrastar um
item pra a case cria um nó novo (`makeModule(type)` +
`graph.prepare()`), que fica silencioso até ser conectado.

`apps/panel/ModuleCatalog.hpp` = a factory `makeModule(type)` + o
agrupamento; serve também pra desserializar um `.rmp`
(`SignalGraph::deserialize`).

### 2.3 Ponto de partida por SEED (reescrito — gramática de portas, 2026-09-03)

**Não é um banco de arquétipos.** O `apps/panel/PatchSeed.hpp` classifica
TODA porta do rack por função musical (fonte: voz / bus / lenta / random /
gate / altura; destino: áudio / FM / mod / gate / altura) e o seed é um
**passeio aleatório PONDERADO** por esse grafo: uma matriz de
compatibilidade `W[fonte][destino]` (com bônus "experimental" escalado
pelo gene `wildness`), cabo a cabo. **Não há limitação de cabeamento** —
qualquer conexão é possível, só mais ou menos provável.

- **Espaço:** rack de 27 módulos = 80 entradas, 68 saídas, 211 parâmetros.
  Cabeamento bruto ~10^147; patches realistas **10^16 (6 cabos) a 10^44
  (20 cabos)**. Seed `uint64` = **1,8·10^19 sementes**.
- **Genes** (SplitMix64, stream por decisão): `complexity` (0 = mínimo
  ~14 cabos, 1 = teia densa ~45), `wildness` (convencional↔experimental),
  `motion` (quanto o `DRIFT` mexe a estrutura), `energy`, `space`,
  `voiceBias`, root/escala, bpm (46–156, enviesado devagar). Curva de
  `complexity`/`wildness` dá **15 % patches minimalistas e 10 % máximos**.
- **Garantias:** sempre há uma ESPINHA voz → processamento → MIXER.ch1 →
  MASTER → sink (audível); a voz é sacrossanta (o passeio não mexe nas
  entradas dela); o CLOCK sempre tica; detector de ciclo liga como
  feedback; `connectToParameter` sempre com atraso de 1 bloco.
- **Verificado (80 seeds):** 0 mudos, 0 erros, RMS 0,024–0,67, cabos
  14–45 (média 22), MIXER com 3+ canais em 75/80. Determinístico.
- **`DRIFT`** plugado em 2–4 parâmetros estruturais por seed → **todo
  patch evolui sozinho** (deriva lenta com momentum, estilo ANTITOTEM).
- **Botão `⚄ SEED`** / **`[g]`** → sorteia um seed ALEATÓRIO (não
  incrementa). `RASGO_SEED=N` / `--seed N` reproduzem um específico.
- **Banco de patches** (`§2.9`): `[Ctrl+B]` guarda o patch atual em
  `~/.local/share/rasgo-modular/patches/seed-N.rmp` (com a linha `seed N`).

### 2.3.1 Ponto de partida por SEED (histórico — v1 arquétipos, 2026-09-02)

Como o **seed do RASGO Synth Studio** (`SeedLineage`), mas de
**CABEAMENTO**: o músico escolhe um seed e o painel monta um patch
generativo coerente entre os módulos do rack — **e já toca**. É opção,
não imposição: a folha em branco continua válida.

- **Botão `⚄ SEED N`** na faixa de status (canto direito) e a tecla
  **`[g]`** — avançam pro próximo seed. O número aparece no botão e no
  título da janela.
- **`RASGO_SEED=N`** no ambiente (ou `./.run_rasgo_modular.sh --seed N`)
  — abre já naquele seed (reproduzir / render por seed).
- `apps/panel/PatchSeed.hpp` — modelado no **"fator de escolha" do RASGO
  Synth Studio** (`CompositionIdentity.hpp`, consultado, não editado):
  - `seedIdentity(seed)` extrai POUCOS **genes** perceptíveis — `Character`
    (Rhythmic/Percussive/Drone/Textural/Melodic/Harmonic), `Voice`
    (Subtractive/Physical/Chordal/Noise/Folded), `Rhythm` (Euclidean/
    Sparse/Driving/Slow), `Motion` (Still/Breathing/Turbulent/Evolving),
    `space`, `risk`, root/escala, bpm derivado do caráter;
  - **separação de streams** (SplitMix64: cada decisão tem seu id,
    acrescentar uma não desloca as outras);
  - **alinhamento por probabilidade, não presets** (um patch Rítmico
    *pode* ter voz de acorde, só é menos provável);
  - **orçamento** de modulação (um número, depois sorteio embaralhado)
    em vez de N Bernoullis independentes;
  - `seedPatch(g, seed)` lê a identidade e monta um grafo POR CARÁTER —
    Drone sustenta (sem gate), Percussive golpeia um LPG/ressoador,
    Textural manda pro MEMORY granular, etc. Os módulos ELABORAM a
    identidade; não a fabricam.
- **Sempre toca, com variedade**: 40 seeds testados → 0 mudos, RMS de
  0,014 a 0,44 (de ambiente suave a driving), 6 caráteres bem
  distribuídos. Voz física sempre ganha um fio de "arco" (ruído no `in`)
  pra não depender da densidade de golpes; ritmo esparso → notas
  sustentadas (ASR) em vez de 3 pluques em 6 s. Detector de ciclo no
  `wire` (liga como feedback se fecharia um loop).
- **Determinístico**: mesmo número → mesma identidade → mesmo patch →
  mesma música (todo módulo Rasgo é determinístico).
- `[s]` continua sendo o "adiciona um módulo do catálogo" (rotação
  determinística) — funções distintas.

**Pendências:** navegar seeds pra trás; salvar o número do seed no `.rmp`;
"linhagem" (derivar um seed do atual, como o `SeedLineage`).

### 2.4 Instâncias e módulos compostos (`INTERFACES_E_LAYOUTS §3.1`)

Duplicar um módulo cria instâncias com estado próprio; podem declarar o
que compartilham (clock, seed, energia, envelope, memória, modulação) e
organizar-se em uníssono/cardume/série/paralelo/cascata/feedback. Um
módulo composto (várias unidades com relações internas) expõe macros +
I/O claros, sem virar caixa-preta. Pendente na UI.

## 3. Layout — números (específicos do Rasgo Modular)

> **Modelo vigente:** "Eurorack proporcional" (§3.2, aprovado 2026-09-02).
> Coordenadas em **mm reais**; um fator `s` (px/mm) derivado do espaço.
> O modelo antigo (grade abstrata `1 HP = 4 unidades`, altura `34
> unidades`, `kPx = 7`) fica registrado abaixo só como histórico.

Para o monitor primário (a sessão de referência: HDMI-0 `1920×1080`) no
modelo vigente, `s ≈ 2,0 px/mm`:

| Grandeza | Valor | Nota |
|---|---|---|
| altura de todo módulo | `128,5 · s` ≈ **257 px** (constante) | é o 3U; só a largura varia |
| largura do módulo | `HP · 5,08 · s` | 8 HP ≈ 81 px · 12 HP ≈ 122 px · 16 HP ≈ 163 px |
| coluna de catálogo | `158 px` (esquerda, fixa) | rola na vertical |
| janela na 1ª abertura | `88%` da área do monitor primário, centrada | ≈ `1690×950` |
| case útil | ≈ `1500×880` (menos catálogo e faixa de status) | |
| largura da case | **enche a largura disponível** (colada na paleta, sem margem vazia) | 104 HP só como largura de referência da 1ª abertura da janela |
| **linhas visíveis** | **3** (≈ 800 px / 267) | resto rola |
| patch típico | 6–10 módulos (~60–90 HP) → **1 linha**, sem rolagem | |

A case **enche a largura** (decisão do autor 2026-09-02 — "aproveitamento
máximo da tela; cada resolução ajusta a capacidade de módulos por
linha"). Notebook `1366×768` → `s` cai a ~1,5–1,6 → ~2 linhas + rolagem
(regime compacto); abaixo disso, emergência rolável. `s` nunca passa de
`s_max` (2,6) nem cai de `s_min` (1,6) — ver §3.2.

*(histórico, modelo antigo: altura `34` unidades ≈ 260 px, largura
`hp · 4 · kPx` + pad, `kPx = 7`, ~3–4 módulos/linha quase quadrados.)*

### 3.1 O padrão Eurorack real — pesquisa e origem

**Quem criou.** O Eurorack foi **especificado em 1995 pela Doepfer
Musikelektronik** (Dieter Doepfer, engenheiro alemão); o primeiro sistema,
o **Doepfer A-100**, saiu em **1996**. A Doepfer não desenhou a mecânica
do zero: adotou o formato de placa **3U / Eurocard** que Dieter conheceu
na época da tese de física dele, já usara no VMS antigo, e reativou no
A-100. O A-100 virou o formato modular mais popular do mundo.

**A medida de origem é híbrida** (pesquisa 2026-09-02):

| Camada | Unidade | Origem |
|---|---|---|
| grade **vertical** — 1 U = 44,45 mm | **1,75 pol exatas** | rack de 19 polegadas (norma americana EIA/RS-310) — **imperial** |
| grade **horizontal** — 1 HP = 5,08 mm | **0,2 pol exatas** ("horizontal pitch") | Eurocard / DIN 41494 / IEC 60297 |
| **placa** Eurocard — 100 × 160 mm | mm redondos | europeia — **métrica nativa** |
| painel 3U — 128,5 mm (5,06 pol) | — | "3U menos folga" |

A Wikipédia registra HP como imperial-primeiro (*"One HP is 0.2 inches
(5.08 mm) wide"*) mas admite que **não dá pra afirmar com certeza** qual
foi a unidade "original" da norma Eurocard. O que é seguro: o rack de 19″
e o "U" são de raiz **imperial** (44,45 e 5,08 são conversões exatas de
polegada); a placa Eurocard é de raiz **métrica**. A Doepfer publica em
mm e arredondou as larguras reais dos painéis pra valores mm-amigáveis.

**Fontes:**
- [Wikipedia — Eurorack](https://en.wikipedia.org/wiki/Eurorack) ·
  [Wikipedia — Doepfer A-100](https://en.wikipedia.org/wiki/Doepfer_A-100) ·
  [Wikipedia — Horizontal pitch](https://en.wikipedia.org/wiki/Horizontal_pitch) ·
  [Wikipedia — Rack unit](https://en.wikipedia.org/wiki/Rack_unit)
- [Perfect Circuit — entrevista com Dieter Doepfer (2025)](https://www.perfectcircuit.com/signal/doepfer-interview-2025-1) ·
  [Perfect Circuit — Pioneering a Format: o Doepfer A-100](https://www.perfectcircuit.com/signal/doepfer-a100-eurorack-overview)
- [GreatSynthesizers — Dieter Doepfer, creator of the A-100](https://greatsynthesizers.com/en/general/interview-en/dieter-doepfer-creator-of-the-a-100-modular-system/) ·
  [Sound on Sound — Modular Profile: Dieter Doepfer](https://www.soundonsound.com/people/modular-profile-dieter-doepfer)
- [SDIY Wiki — 19-inch rack](https://sdiy.info/wiki/19-inch_rack) ·
  [Getek — EIA-310 / IEC 60297-2 / DIN 41494](https://www.getek.com/understanding-global-rack-standards-ansi-eia-rs-310-d-iec-60297-2-and-din-41494-explained/)
- [synthracks.com — Eurorack Rails DIY Guide](https://synthracks.com/blog/eurorack-rails-diy-guide)
- **ficha oficial Doepfer** — imagem local
  `/media/luc/4tb_21042021/lucio/rasgo_sdiy/eurorackdiy/dimensoes_placas/eurorack_doepfer_dimensions.png`

Consolidado na memória `[[reference_eurorack_module_dimensions]]`.

**Geometria do painel 3U:**

| Grandeza | Medida real |
|---|---|
| altura do painel | **128,5 mm** (externo); **125,5 mm** entre centros dos furos (3,0 mm de cada borda) |
| diâmetro do furo | **d = 3,2 mm** |
| furo mais à esquerda | 7,5 mm da borda esquerda; a 2ª coluna a `N · 5,08` da 1ª |
| trilho-a-trilho 3U | 133,35 mm · furo-a-furo (slot c-c) 122,5 mm |
| 1 U (referência de rack) | 44,45 mm (1,75 pol); 3U = 3 × 44,45 |

**Largura por HP** (`1 HP = 5,08 mm` calculado; a largura *real* do painel
é cortada abaixo do múltiplo pra deixar folga entre vizinhos — tabela
Doepfer, folga ~0,3–0,4 mm):

| HP | calc (mm) | real | HP | calc | real |
|---:|---:|---:|---:|---:|---:|
| 1 | 5,08 | 5,00 | 14 | 71,12 | 70,80 |
| 2 | 10,16 | 9,80 | 16 | 81,28 | 80,90 |
| 4 | 20,32 | 20,00 | 18 | 91,44 | 91,30 |
| 6 | 30,48 | 30,00 | 20 | 101,60 | 101,30 |
| 8 | 40,64 | 40,30 | 21 | 106,68 | 106,30 |
| 10 | 50,80 | 50,50 | 22 | 111,76 | 111,40 |
| 12 | 60,96 | 60,60 | 28 | 142,24 | 141,90 |
|  |  |  | 42 | 213,36 | 213,00 |

### 3.2 Decisão — "Eurorack proporcional, escalado pra caber" (aprovada 2026-09-02)

O autor aprovou: **proporção real do Eurorack, mas com um fator de escala
único** derivado do espaço da tela — não a caixa quase-quadrada antiga,
não HP físico fixo.

**Modelo de renderização.** Toda a régua passa a ser **milímetro real**,
com um fator `s` (px/mm):

- **altura de todo módulo** = `128,5 · s` px (constante — é o 3U);
- **largura** = `HP · 5,08 · s` px (largura calculada; a folga entre
  vizinhos vem do `kGap`, não de cortar o painel);
- `s` é **derivado**, não fixo:

```
s = clamp( (altura_útil_da_case / linhas_alvo − kGap) / 128,5 , s_min , s_max )
```

| Parâmetro | Valor | Porquê |
|---|---|---|
| linhas alvo | **3** | 1080p @ 88% cabe 3 fileiras confortáveis |
| `s` na referência (1920×1080 @ 88%) | **≈ 2,0 px/mm** | altura de módulo ≈ 257 px |
| `s_min` | **1,6 px/mm** | abaixo o knob (~10 mm) fica < 16 px → regime compacto: esconde rótulo secundário, **não** encolhe mais |
| `s_max` | **2,6 px/mm** | monitor grande não vira outdoor |

**Larguras a s = 2,0:** 4 HP ≈ 41 px · 8 HP ≈ 81 px · 12 HP ≈ 122 px ·
16 HP ≈ 163 px. Altura 257 px (todas). 3 linhas ≈ 800 px (a 4ª espia,
rola). Patch de 6–10 módulos (~60–90 HP) = **1 linha, sem rolagem**.

**Largura da "case" = enche a largura disponível** (revisto 2026-09-02, a
pedido do autor — a versão anterior centrava um rack de 104 HP e deixava
margens vazias num monitor largo). A case começa **colada na paleta** e
vai até a borda da janela; cada resolução ajusta quantos módulos cabem
por linha. Os **104 HP** ficam só como *largura de referência* pra
dimensionar a janela na 1ª abertura.

**Grade interna do módulo — em mm** (`Panel`/`Widget`):

| Elemento | Medida |
|---|---|
| coordenadas do `Widget` | milímetros a partir do canto sup-esq do painel |
| knob | ø ~10 mm (alvo de toque; a s=2,0 → 20 px, a s=1,6 → 16 px) |
| passo entre linhas de controle | ~13 mm |
| coluna de controle | ~1 por 8 mm de largura → 4 HP (~20 mm) ≈ 1 coluna, 8 HP ≈ 2 colunas |
| margem interna do painel | ~3 mm (borda) + faixa de nome no topo ~8 mm |
| jacks na faixa inferior | ~6 mm ø, fila a ~7 mm da base |

**Consequência assumida:** com proporção real os módulos ficam ~⅓ da
largura de antes (mesma altura). Os `panel()` dos 17 módulos precisam de
uma **passada de densidade**: menos colunas, mais empilhamento vertical
(autêntico), HP revisto pra valor realista (maioria **4–14 HP**),
PARAMETRIC e MIXER reorganizados. Regra de não-sobreposição (§7) segue —
a checagem O(n²) agora opera nas pegadas em mm.

Nada disso toca o áudio nem o motor: `Panel` continua dado puro, só muda
a unidade das coordenadas (grade abstrata → mm) e o renderizador.

### 3.3 Renderização — sem flicker, texto recortado

- **buffer fora da tela:** todo o quadro é desenhado num `Pixmap` e
  copiado pra a janela numa passada só (`XCopyArea`). Nada de desenhar
  direto na janela.
- **repintura por evento:** não há redesenho periódico. Repinta só em
  resposta a `Expose`/`ConfigureNotify`/interação. O áudio roda no seu
  thread, independente do desenho.
- **recorte por módulo:** o conteúdo de cada módulo é desenhado com um
  `clip rectangle` = a caixa do módulo (menos a borda). Rótulo que não
  cabe é **cortado na borda**, nunca invade o vizinho. O `Display`
  também tem a largura limitada à caixa. (2ª camada: encurtar/rotacionar
  o rótulo em vez de cortar.)

### 3.4 Áudio — sem xrun (o "estalo" que soa como clip)

- **`MASTER` com proteção de excelência** (`src/dsp/OutputStage.hpp`,
  padrão da família `NAVALHA`/`ANTITOTEM`): limitador com look-ahead que
  **não distorce o transiente** (o `tanh` instantâneo de antes distorcia)
  + guarda de finitude + teto −1 dBFS. O grafo em si já não clipava (pico
  ~0,66 no patch default); o que soava como clip era xrun.
- **`AlsaSink` com buffer fundo** (8 períodos de 256 = ~43 ms). O thread
  de desenho é event-driven e o `graph.prepare` (re-cabear/spawn) roda
  sob o mutex do áudio — um clique no patch é tolerável, xrun contínuo
  não. O buffer fundo absorve o atraso.
- O `AudioBlock` do motor tem teto de 256 frames → o painel processa em
  sub-blocos de ≤256 por escrita do ALSA.
- **Pendente:** mover o `graph.prepare` pra fora do caminho do áudio
  (handoff sem lock); `TruePeakDetector` 4× no `MASTER`.

## 4. Contrato responsivo (`INTERFACES_E_LAYOUTS §5.1`)

| Item | Estado |
|---|---|
| canvas de referência ≠ tamanho de janela | **sim** (parcial) |
| 1ª abertura em 85–90% da área do **monitor primário**, centrada | **sim** (`WindowPolicy.hpp`, `XRRGetMonitors` → primário; ~88%) |
| persistência/revalidação de monitor+bounds | **não** (v1) — `revalidate()` existe, falta persistir |
| completo / compacto / emergência | **parcial** — quebra em linhas + rolagem; **sem reflow semântico** ainda |
| `resized()` recalcula áreas | **parcial** — `ConfigureNotify` refaz o flow das linhas; controles não reescalam |
| DPI 100/125/150/200% | **não testado** |
| redimensionar/mover não altera áudio | **sim** — o layout não toca no grafo |
| janela recuperável após desconectar monitor | **parcial** (`revalidate()` clampa; sem persistência) |

**Débito assumido** (é `exploration`): o contrato responsivo completo —
política pura testável tipo `AQUORBIUM/WINDOW_LAYOUT_DESIGN.md`, reflow
por área semântica, persistência, matriz DPI — é do **front-end de
produção** (JUCE / web-WASM, `RASGO_MODULAR.md §36.7`), não deste teste.
Registrado pra não virar identidade por acidente.

## 5. Tokens (`IDENTIDADE_VISUAL §4`)

Cor **nomeada por função** no código (`struct Tokens`), mapeada a valores
de **estudo** (não aprovados):

```
color.background   fundo da janela
color.surface      caixa do módulo
color.recessed     display, poço de knob, coluna de catálogo
color.line         borda, trilho, traço fraco
color.text.primary rótulo forte, nome de módulo do catálogo
color.text.secondary rótulos fracos, valores, status
color.accent       ponteiro de knob, toggle ligado, handle, família do catálogo
color.warning      (reservado — clip/cicatriz)
size.control.min   knob ~18 px (alvo de toque a rever pra DPI)
space.row          ~5 unidades de grade entre linhas de controle
```

## 6. Tipografia

Fonte **de sistema** via `XFontSet` do locale (o repo não empacota
fonte — `IDENTIDADE_VISUAL §3`: sem licença/manifesto, usar sistema).
**UTF-8 obrigatório:** `setlocale(LC_ALL,"")` + `Xutf8DrawString`, com
fallback pra fonte de núcleo — **nunca conversão implícita** (regra
transversal do workspace, `PAINEL_MESTRE` 2026-08-28). Cobre acentos do
português.

## 7. Sem sobreposição acidental

Regra do autor (2026-09-02): **sobreposição só quando o conceito, a
mecânica e a definição dos objetos caminharem pra isso** — nunca por
descuido. Implementação: cada widget tem uma **pegada** (`footprint()`);
no arranque, checagem O(n²) por módulo **avisa no stderr** se duas se
cruzam. Widgets desenhados compactos pra caber no espaçamento de grade
que os `panel()` declaram. Estado: **0 sobreposições** nos módulos
exibidos.

## 8. Autonomia, MIDI/áudio, acoplamento

`RASGO_MODULAR.md §35.4.1` e `§36.8`: o painel **faz som ao carregar**,
sem exigir teclado nem entrada de áudio. `MIDI-IN`/`AUDIO-IN`/`CV-IN` e o
acoplamento a outros instrumentos / recursos de composição entram como
**módulos-adaptadores** no catálogo (família `INPUT / GESTURE`), nunca
como dependência.

## 9. Ligação com produto físico futuro

`PRODUTO_HARDWARE_E_ELETRONICA.md` (fases H0–H6): um console/módulo do
Rasgo Modular nasceria como produto próprio. A `Panel` declarativa já
separa "que controles existem e onde" de "como se parecem" — ponte pro
layout físico. Nada disso é este teste.

## 10. Pendências (roadmap da UI)

1. **cabeamento** (§2.5) — **feito** o básico; falta profundidade/
   atenuador no cabo, modulação saída→parâmetro pela UI, matriz e
   constelação (`§36.2`) como views alternativas;
1b. **displays por módulo** (§2.9) — **feito** MATRIX (grade clicável),
   SCOPE (espectro), TRIGSEQ (4 lanes); falta o overlay editável do
   TRIGSEQ (precisa de máscara de passos no módulo) e modo XY do SCOPE;
2. **ponto de partida por seed** (`§2.3`) — **feito** (botão `⚄ SEED` /
   `[g]` / `RASGO_SEED=N`); falta navegar pra trás e a "linhagem";
3. **contrato responsivo** completo (`§4`) no front-end de produção;
4. `panel()` de PARAMETRIC e MIXER em grade 2×2 pra caber na altura
   padrão;
5. persistência de janela (monitor+bounds);
6. carregar/salvar `.rmp` pela UI (patch = composição recuperável);
7. instâncias e módulos compostos (`§2.4`);
8. DPI 100/125/150/200% + matriz de resoluções (marco de interface).

## 11. Encerramento

Fonte: este documento + `apps/panel/*`. Estado: `exploration`. Não
promover a identidade nem a padrão comum sem brief próprio e decisão no
histórico global.
