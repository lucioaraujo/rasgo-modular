# Rasgo Modular — mudanças / changelog

Formato: uma seção por versão, do mais novo para o mais antigo. Datas em
ISO. Segue a mesma postura do resto do projeto: registra o que **foi
feito**, com as limitações ditas em voz alta, e não o que deveria estar
pronto.

---

## v0.1.3 — em preparação

- **Auditoria de idiomas** (pedido do autor, 3 out. 2026): o LEARN segue
  o idioma escolhido em todas as chamadas (as quatro passam o idioma; a
  troca chega à paleta, ao rack, ao rodapé, ao cabeçalho e ao tutorial),
  os 803 verbetes de controle e os 58 de módulo têm os campos completos
  nos quatro idiomas, e um auditor de palavras não achou texto num idioma
  dentro de outro. Corrigido o que apareceu:
  - **primeira abertura no idioma do sistema** (pt, fr ou es; qualquer
    outro, inglês). Antes abria em português para todo mundo; quem já
    escolheu um idioma continua com a escolha;
  - a tela SOBRE dizia "blocos perdidos" e "alvo" em português em
    qualquer idioma;
  - o LEARN de TURING e SEQUENCE mandava usar "MUTATE", botão que não
    existe em nenhum idioma (TURING: o knob MUT; SEQUENCE: MUDA/EVOLUI,
    CHANGE/EVOLVE, CHANGER/ÉVOLUE, CAMBIA/EVOLUCIONA); o do MASTER em
    português chamava o botão ESPERA de STANDBY;
  - o cartão TECLADO dizia "m mutate / muter / muta" com o botão chamado
    CHANGE / CHANGER / CAMBIA;
  - o contador do cabeçalho cortava a última letra ("13 câble").
  Verificado: ctest 80/80; o app aberto com o sistema em francês e sem
  preferência salva abre em francês (cabeçalho, LEARN, contador).

---

## v0.1.2 — 2026-10-03

> **English summary.** Fixes a patch going silent for good after UNCABLE,
> and audible clicks caused by dropped audio blocks (106 per minute before,
> 0 after, measured on the same seed). Clicking a cable now hits that cable
> (a swapped-argument bug since 15 Sep.), the cable under the mouse lights
> up, and a click on a cable never moves a module. The cable box sits in
> the lower-right corner with both ends named, GAIN, an always-visible COND,
> a live conduction light and UNPLUG. The tutorial is laid out for reading,
> scrolls properly and links to the new module guide on the website, which
> now explains all 58 modules for beginners in four languages. The
> recording's score file gets a header. Windows and macOS are still built
> and tested by continuous integration only. Details below, in Portuguese.

- **Tutorial com link para o guia dos módulos.** O cartão LEARN dizia que
  um guia escrito estava "a caminho"; agora convida para o guia do site e
  traz logo abaixo o endereço da página de módulos no idioma do app,
  clicável (abre no navegador). O guia foi reescrito para quem está
  começando: os 58 módulos em português, inglês, francês e espanhol, cada
  um com o que faz, como pensar nele, o que muda em cada controle e um
  exercício. Os nomes de controles e portas citados são conferidos contra
  o código por `website/checar_guia.py`.
- **Corrigido: depois de DESCABEIA o patch ficava mudo para sempre.** A
  saída de som é um nó que não aparece no rack, ligado ao MASTER por um
  cabo invisível; o DESCABEIA (e o clique direito na saída do MASTER)
  tirava também esse cabo, e não havia como refazê-lo — só SEED ou
  DESFAZ traziam o som de volta, e o "construir do zero" do tutorial não
  funcionava. Agora essa ligação final nunca é tirada, e um MASTER
  re-adicionado pela paleta vai direto à saída.
- **Corrigido: estalos por blocos de áudio perdidos.** O áudio desistia
  do bloco na primeira tentativa se a interface estivesse com o patch
  travado, e a interface trava o patch a cada quadro por frações de
  milissegundo (o VARIA mexe nos parâmetros sob a trava). Num patch comum
  isso perdia ~2 blocos por segundo; cada perda é uma queda de ~6 ms no
  som, e muitas por minuto soavam como estalos (relato do autor no seed
  4303935450909092226: 2247 blocos perdidos numa sessão, gravação limpa —
  o bloco perdido não entra na gravação). Agora o áudio espera até 1,5 ms
  antes de desistir. Medido no mesmo seed, 60 s: **106 perdidos antes, 0
  depois** da abertura (os 14 da abertura são a medição de volume do seed,
  antes de o som começar). `RASGO_DIAG=1` imprime a contagem no terminal.
- **Partitura com cabeçalho:** data e hora, arquivo de áudio com duração,
  versão do instrumento, autoria, licença e contato.
- **Clique no cabo acerta o cabo.** Desde 15 set. o front-end JUCE passava
  o ponto do mouse em último lugar à função que mede a distância até a
  curva, que o espera em primeiro: o teste era outro, e o clique só
  acertava por acaso — o "fico tentando várias vezes" do autor. Nova
  função `nearestCable`, testada no ctest, que também escolhe o cabo mais
  próximo quando dois passam perto.
- **Destaque do cabo sob o mouse** (mais grosso e mais claro) e cursor de
  mão; o cabo inspecionado fica destacado enquanto a caixa está aberta.
- **Módulo não foge mais no clique** (regra do autor): clique sobre um cabo
  é sempre do cabo; o módulo só se move se o clique começar fora de cabo e
  o arrasto passar de 6 px.
- **Caixa do cabo fixa no canto inferior direito** da área visível, com
  **título completo** — módulo e porta das duas pontas (`CLOCK · euclid`
  / `→ QUANTIZER · trigger`), sugestão do autor — e mais controles do que
  o motor já sabia fazer: **GAIN** (0 a 2, neutro no meio; existia e não
  tinha controle), **COND sempre visível** (só aparecia depois de escolher
  uma relação, embora funcione sem ela), **luz de condução** ao vivo,
  marca de **realimentação**, o **companion pelo nome** (clicar volta a
  escolher) e **DESPLUGAR** (era "REMOVER", que lia como tirar o módulo).
- **Desplugar na vista RACK · SAÍDA não some com o módulo.** Quem chegava ao
  som e deixou de chegar com o corte fica à vista por exceção, com borda
  tracejada, como o módulo recém-adicionado — vale para o DESPLUGAR, o
  clique direito no jack e pegar a ponta do cabo. Antes, o módulo sumia
  da tela no instante do corte (continuava no patch, mas para quem toca
  era igual a ter sido apagado). `RASGO_INSPECIONAR=n` abre a caixa do n-ésimo
  cabo ao iniciar, para capturas de tela.
- **Tutorial diagramado para leitura:** coluna de no máximo 640 px (em vez
  de linhas de ~120 letras), listas do cabeçalho e do teclado um item por
  linha, textos longos em parágrafos, régua e espaço entre os cartões, e a
  roda do mouse rolando ~70 px por clique (eram ~8 px — "a rolagem não
  responde"). Dois fatos corrigidos no texto, nos 4 idiomas: a gravação é
  de 24 bits (dizia 16) e o volume do MASTER é ajustado por seed (dizia que
  começava sempre em −24 dB).
- **A caixa LEARN segue o idioma:** ao trocar de idioma ela guardava o
  verbete no idioma anterior até o próximo hover; agora volta à dica.
- **Tutorial: barra de rolagem de verdade** — trilha visível, polegar
  arrastável, clique na trilha pula uma página. Antes era só um traço
  desenhado e, como qualquer clique fechava o tutorial, tentar arrastá-la
  fechava a janela; agora só fecham o FECHAR, um clique fora do cartão e
  `Esc`. Saem os asteriscos de "negrito" que apareciam literais no texto.
  `RASGO_TUTORIAL=1` abre o tutorial ao iniciar, para capturas.
- **Cabeçalho em grupos**, separados por régua: [seed · SEED · REPOR] |
  [VARIA · MUDA · EVOLUI · CRUZA] | [BANCO · SALVA · ABRIR] | [DESFAZ ·
  DESCABEIA · ESPERA] | [ZOOM · RACK]. O RACK saiu do canto direito.
- **Ícone: o monograma** (a letra `r` do wordmark, o favicon do site) em
  todos os tamanhos e sistemas. No Linux o menu e a barra mostravam o
  wordmark master preto, quase invisível no painel escuro; o `.desktop`
  ganhou `StartupWMClass` para a barra associar a janela ao atalho.

---

## v0.1.1 — 2026-10-02

> **English summary.** Fixes a crash on Windows (and a freeze on Linux and
> macOS) when picking up the end of a cable from an input that was already
> patched — the most common way to re-patch. The window now always opens
> maximised on the primary monitor. Each module shows its family as a thin
> colour stripe at the top, with the matching colour as a legend in the
> module palette. The header shows the full 20-digit seed, the VARY slider
> is wider, STANDBY sits next to UNCABLE, and REC turns red while
> recording. Windows and macOS are still built and tested by continuous
> integration only. Details below, in Portuguese.

- **Faixa de cor por família** no topo de cada módulo, e o mesmo tom num
  quadrado ao lado do nome da família na paleta, que serve de legenda.
  Sugestão de um usuário do r/modular (não se via de relance a família de
  um módulo dentro do rack); faixa escolhida pelo autor entre faixa e
  fundo tingido, por capturas do mesmo patch. Tons de saturação baixa,
  para não competir com a cor dos cabos (quentes = áudio, frios =
  controle); o par mais próximo, TRANSFORM e SPACE, fica a ΔE 17,8. Está
  na camada fixa do módulo: custo zero por quadro.
- Sai a régua entre DESCABEIA e ESPERA.
- **REC fica vermelho enquanto grava**, em vez do laranja de "ligado" dos
  outros botões.
- **Cabeçalho** (pedidos do autor, 2 out. 2026): a caixa do seed mede a
  própria largura pela fonte e mostra os 20 dígitos inteiros (cortava o
  primeiro em seeds longos); o slider VARIA passou de 54 para 84 px, para
  ajuste fino; ESPERA fica logo à direita do DESCABEIA;
  o rótulo ZOOM deixa de ficar sozinho na primeira fileira quando os
  comandos descem para a segunda.
- **Corrigido: o app fechava (Windows) ou congelava (Linux/macOS) ao
  pegar a ponta de um cabo numa entrada já cabeada** — o gesto de
  repatchear clicando na entrada. O código travava o mutex do grafo e,
  ainda com ele travado, chamava uma função que o travava de novo;
  `std::mutex` não é reentrante. No Linux isso congela a thread (comprovado
  com teste mínimo); no Windows a STL da MSVC lança exceção, compatível com
  o relato de um usuário do Windows 10 no r/modular (1 out. 2026). Defeito
  presente desde o commit `ab316bf`, portanto na v0.1.0. Uma varredura do
  arquivo por outras chamadas a funções que travam o mesmo mutex dentro de
  um lock aberto não achou outro caso.
- **A janela abre sempre maximizada no monitor principal** (pedido do
  autor, 1 out. 2026). Antes abria com ~88% do monitor, e num sistema com
  dois monitores o Cinnamon a punha no monitor onde estava o mouse. Agora
  ela é colocada no monitor principal (o primário do XRandR) depois de
  aparecer e só então maximizada — maximizada, não tela cheia: barra de
  título e painel do sistema continuam à vista, e restaurar volta ao
  tamanho anterior. Conferido no Linux Mint/Cinnamon com dois monitores
  (`xprop` e `xwininfo`, três aberturas). **Limitação:** por ~0,3 s a
  janela pode aparecer no monitor do mouse antes de ir para o principal;
  Windows e macOS não foram abertos pelo autor.
- Desempenho do desenho: duas tentativas medidas **sem ganho** (repintura
  por região; chrome `RGB` opaco), não integradas. Registro e números em
  `TAREFAS.md` → "Tarefa aberta — v0.1.1".

---

## v0.1.0 — 2026-09-29

> Publicada em 29 set. 2026: tag `v0.1.0`, release com os três
> instaladores (`.deb`, `.exe`, `.dmg`) gerados pela CI, repositório
> aberto e site no ar. A sessão de escuta documentada foi feita em 23–24
> set. (`dossies/VALIDACAO_v0.1.0.md`). O `.deb` foi instalado pelo autor
> no Linux Mint a partir da release; Windows e macOS continuam sem teste
> em máquina real.

Primeira versão publicável do instrumento. O que ela é: um ambiente
modular generativo que **nunca abre em branco** — ele apresenta um patch
já montado e soando, como **ponto de partida**, e quem toca é o músico.
Não precisa de MIDI nem de entrada de áudio; MIDI, áudio e acoplamento de
instrumento são nós adaptadores opcionais.

### O instrumento

- **58 módulos DSP**, cada um com dossiê escrito e testes. Oito famílias:
  SOURCE, TRANSFORM, MODULATE, TIME, DECISION, ROUTE, SPACE, OUT.
- **O cabo como objeto**, que é a ideia que separa este instrumento de um
  modular comum: cada ligação tem ganho, condutância probabilística,
  relação (RingMod / Fold / Difference) e **ruptura com cicatriz** — o
  corte decai, não estala.
- **Modelo de conexão em 3 camadas** — matriz, constelação (acoplamento
  por distância) e semântico (qualidades) — todas serializadas no patch.
- **Composição generativa**: seed reproduzível, VARIA (variação ao vivo) e
  as operações genéticas MUDA / EVOLUI / CRUZA.

### Front-end

- **App JUCE multiplataforma** (`apps/juce/`) — o artefato da release.
  Rack em milímetros, cabeamento jack-a-jack, inspector de cabo, LEARN em
  três níveis, tutorial rolável, quatro idiomas (pt / en / fr / es).
- **Desfazer** com anel de 24 fotografias serializadas, cobrindo **cada
  cabo** ligado ou cortado.
- **Painel X11** (`apps/panel/`) segue como ferramenta de teste e
  referência de comportamento — não é o artefato publicado. As
  divergências deliberadas entre os dois estão tabeladas em
  `apps/juce/PARIDADE.md`.

### Saída de áudio

- **Guarda de segurança no sink** (`apps/panel/SinkOut.hpp`): sanitiza
  não-finitos e aplica teto de ±0.891251. Um módulo cabeado direto no OUT,
  por fora do MASTER, recorta de forma feia e **audível** — isso é
  proposital, é o aviso de que falta um MASTER — mas nunca estoura sem
  limite nem produz estalo de NaN.
- **Medição BS.1770-4 / EBU R128** (`src/dsp/Loudness.hpp`): momentânea,
  curta e integrada com os dois gates, com a ponderação K derivada dos
  protótipos analógicos por taxa de amostragem. A leitura fica no cartão
  SOBRE.
- **Dois taps de gravação nomeados**: `post-safety` (o que se ouviu) e
  `pre-safety` (antes da proteção). `RASGO_REC_TAP=both` grava os dois,
  com o tap no nome do arquivo — dois arquivos da mesma tomada soando
  diferente sem explicação seria uma armadilha.
- **Gravação em PCM 24 bits**, sem dither. A tomada não é o arquivo
  final, e o MASTER abre em −24 dB de propósito: em 16 bits isso jogaria
  fora resolução que não volta. Em 24 bits o degrau de quantização fica
  muito abaixo do ruído do material, então dither só somaria ruído.
- **Medição de true-peak** (pico entre amostras), sobreamostrando 4×.
  O pico de amostra mente: um sinal que marca 0 dBFS pode passar de
  +3 dBTP depois do conversor ou de um codificador com perdas — e é o
  true-peak que estoura na recodificação.
- **Alvo de publicação declarado: streaming** — −14 LUFS integrado, teto
  de −1 dBTP. O cartão SOBRE mostra a distância até o alvo em LU, o
  true-peak medido e a **taxa de amostragem real** do dispositivo (o app
  adota a do sistema, não impõe uma). Nada disso normaliza o sinal: o
  medidor não o toca em lugar nenhum.
- **Rampas por amostra** em GAIN e WIDTH do MASTER, que antes clicavam.
- Cada tomada sai com um `.score.txt` que registra o patch e as ligações.

### Disciplina de tempo real

O thread de áudio **nunca espera pela interface**: pega o grafo com
`try_to_lock` e, se a UI estiver editando, reemite o último bloco com
rampa de volta à unidade. A UI, do lado dela, desenha de fotografias
sem-trava. Foi o que acabou com os cliques.

### Verificação

- **77 testes automatizados**, sem dependência de janela ou dispositivo de
  áudio — rodam em servidor ou container.
- **Auditoria de teclado executável** (`tests/test_shortcuts.cpp`): as 17
  combinações, cada uma nas quatro formas em que o sistema pode entregar a
  tecla, mais a garantia de que o tutorial não diverge do código.
- **CI de três sistemas verde** (21 set. 2026): compila, roda os 77
  testes e empacota em Linux (`.deb`), Windows (`.exe`, NSIS) e macOS
  (`.dmg`). No macOS a verificação com `file`/`lipo`/`otool` confirmou
  Universal 2 real — as duas fatias (x86_64 e arm64) e `minos 10.15`, o
  alvo do projeto e não o da máquina que compilou (um runner macOS 26).

  Vale o registro de que esse verde custou cinco correções, porque o
  workflow **nunca tinha rodado** antes da extração para repositório
  próprio: `std::filesystem` contra um deployment target de 10.13 no
  macOS; `M_PI` indefinido no MSVC em 83 lugares; testes gravando em
  `/tmp` fixo, que não existe no Windows; um OOM por paralelismo sem
  limite; e uma captura de lambda que GCC, Clang e MSVC tratam de três
  jeitos. Dois desses defeitos impediam a compilação por completo.

### Limitações declaradas

Estão aqui porque o gate editorial do RASGO exige que sejam ditas, não
supostas:

- **Windows e macOS nunca foram abertos.** A CI prova que **constrói e
  empacota**; não prova que abre, soa e se comporta numa máquina real. Não
  há Windows nem Mac no ambiente de desenvolvimento. **O autor decidiu
  publicar nessa condição** (21 set. 2026), com a limitação declarada —
  mesmo precedente do Antitotem. A matriz completa está em `INSTALL.md`.
- O instrumento vive agora em **repositório próprio**
  (`lucioaraujo/rasgo-modular`, privado até a publicação), extraído do
  monorepo com o histórico preservado. Foi o que tirou a **CI de três
  sistemas da inércia**: enquanto o projeto era um subdiretório, o GitHub
  Actions não lia o workflow.
- **Custo de CPU alto: ~70% de um núcleo** num patch comum. Medido com
  `perf`: ~38% é desenho da interface e ~11% é DSP — o desenho custa mais
  que o som, porque a view inteira é repintada 30 vezes por segundo
  embora só os osciloscópios e LEDs mudem. Numa máquina modesta o áudio
  pode falhar; aumentar o buffer de saída ajuda. Correção prevista para a
  v0.1.1, deixada de fora desta versão por decisão explícita — mexer no
  caminho de desenho antes de publicar trocaria um problema medido por um
  risco desconhecido.
- **Linux ARM** não foi construído nem testado.
- O catálogo de módulos **não é fechado** por desenho: a fase didática
  corre em paralelo com módulos novos.

### Licença

AGPL-3.0-or-later. O JUCE é usado sob AGPLv3, sem licença comercial — ver
`apps/juce/LICENSE_STATUS.md`.

---

## English

### v0.1.0 — prepared, not yet tagged

Held back by the documented listening session (`dossies/VALIDACAO_v0.1.0.md`,
Part A), which is the hard gate for publication layer 2 and cannot be
closed by code.

First publishable version of the instrument: a generative modular
environment that **sounds on load**, with no MIDI and no audio input.

- **58 DSP modules** across eight families, each with a written dossier
  and tests.
- **The cable as an object** — gain, probabilistic conductance, relation
  (RingMod / Fold / Difference) and rupture that leaves a scar: a cut
  decays instead of clicking.
- **Three-layer connection model** (matrix, constellation, semantic), all
  serialised into the patch.
- **Generative composition**: reproducible seed, live variation, and the
  MUTATE / EVOLVE / CROSS genetic operations.
- **Cross-platform JUCE app** as the release artefact, in four languages,
  with a 24-step undo ring that covers every cable made or cut. The X11
  panel remains a development tool; deliberate divergences are tabulated
  in `apps/juce/PARIDADE.md`.
- **Output safety stage** (non-finite sanitising plus a ±0.891251
  ceiling) and **two named recording taps** — `post-safety` (what you
  heard) and `pre-safety` (before the protection).
- **BS.1770-4 / EBU R128 metering with true-peak**, and a declared
  **streaming target** (−14 LUFS integrated, −1 dBTP ceiling). Recording
  is **24-bit PCM**; the sample rate is whatever the device offers, and
  the ABOUT card shows it alongside the distance to the target. None of
  this normalises the signal — the meter never touches it.
- The audio thread never waits on the UI: it takes the graph with
  `try_to_lock` and re-emits the last block with a ramp back to unity if
  the UI holds it.
- **77 automated tests**, no window server or audio device required.

**Declared limitations.** Windows and macOS have **never been opened** —
CI proves the build and the package, not the behaviour on real hardware;
the author decided to publish under that condition, stated openly. The project now lives in its own
repository (private until publication), which is what made the
three-platform CI actually run.
Linux ARM is untested. The module catalogue is open by design.

AGPL-3.0-or-later; JUCE under AGPLv3, no commercial licence.
