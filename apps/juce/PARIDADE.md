# Paridade JUCE ↔ painel X11 — auditoria

**Data:** 2026-09-13 · **Método:** varredura feature a feature de
`apps/panel/panel_main.cpp` (3.646 linhas, o painel que estávamos usando)
contra `apps/juce/RasgoModularApp.cpp` (1.080 linhas), verificando cada
item no código dos dois lados — não por memória.

Este documento existe porque o relato do autor testando na mão
("os gráficos não estão funcionando", "as animações dos leds e
osciloscópios estão bugadas", "não há botão varia", "o cabeçalho está bem
aquém") mostrou que corrigir item a item estava escondendo o tamanho real
do buraco. O que segue é o buraco inteiro.

---

## Estado — atualizado 2026-09-18

| Item | Estado |
|---|---|
| A1 · não existe `Timer` | ✅ **corrigido** — `startTimerHz(30)`, a cadência do X11 |
| A2 · `Display` vazio | ✅ **corrigido** — as 4 vistas + alimentação pelo áudio |
| A3 · rolagem da paleta | ✅ **corrigido** — medida fora do laço + barra arrastável |
| A4 · hit-test dependente da pintura | ✅ **corrigido** — `layoutRows()` separado |
| B1 · VARIA/MUTA/EVOLUI/CRUZA/BANCO/SALVAR | ✅ **feito** — era fiação, como previsto |
| B1 · leitura, VU e flash no cabeçalho | ✅ **feito** |
| B1 · carregar `.rmp` / retomar sessão | ✅ **feito** |
| B1 · SYSTEM SCORE | ✅ **feito** — em par com o `.wav` |
| B2 · inspector, arrastar módulo, seed editável, teclado, overlays, prefs | ✅ **feito** |
| B3 · áudio in / MIDI in / REC | ✅ **feito** — pela API do JUCE, multiplataforma |

### Divergência deliberada — auditada 2026-09-20

A partir do momento em que a paridade fechou, o app passou a ganhar
recursos que o painel não tem. Isso é esperado (papéis diferentes), mas
**divergência não documentada vira surpresa** — foi a origem da maior
parte dos relatos do autor. Esta tabela existe pra ela parar de crescer em
silêncio.

| Recurso | X11 | JUCE | Decisão |
|---|---|---|---|
| Halo por audibilidade + fonte muda | ✅ | ✅ | **portado** — é comportamento do instrumento, não da interface |
| REPOR (volta ao seed atual) | ✅ `r` | ✅ `r` + botão | **portado**; no painel o `reprepare` de desenvolvimento foi pra `Shift+R` |
| Descabear tudo | ✅ `n` | ✅ `n` + botão | portado |
| Módulo novo visível na vista SAÍDA | ✅ | ✅ | portado |
| Reordenar nas vistas filtradas | ✅ | ✅ | portado |
| **Desfazer (`Ctrl+Z`)** | ❌ | ✅ | **só no JUCE** — o anel de fotografias serializadas é estrutura nova; portar exige o caminho de recarga inteiro no painel, que é ferramenta de teste |
| **ABRIR com seletor de arquivo** | ❌ | ✅ | **só no JUCE** — em X11 puro seria escrever um navegador de arquivos à mão. No painel o caminho continua `RASGO_RESUME=1` |
| **Taps de gravação (`pre-safety`)** | ❌ | ✅ | **só no JUCE** — o `MASTER` já expõe o tap nos dois; falta só a fiação do REC no painel. Portável a baixo custo se fizer falta |
| **Atalho acende o botão** | ❌ | ✅ | **só no JUCE** — o painel tem o flash no clique; no teclado ele passa direto. Portável, não portado |
| **Aviso pós-ligação no LEARN** | ❌ | ✅ | **só no JUCE** |
| **Tabela única de atalhos** (`src/ui/Shortcuts.hpp`, testada) | ❌ | ✅ | **só no JUCE** (21 set. 2026) — o painel mantém o próprio bloco de `XK_*`, com duas ações de desenvolvimento que o app não tem: `s` = sugestão de módulo, `Shift+R` = `reprepare`. Portar é mecânico; não foi feito às pressas antes da publicação |

O critério aplicado: **comportamento do instrumento** (como o som e o
cabeamento respondem) é portado; **conveniência de interface** fica onde
faz sentido pro papel de cada front-end. O painel é ferramenta de
desenvolvimento e referência de comportamento; o app é o artefato que vai
na release.

**Paridade funcional alcançada.** Daqui em diante o app JUCE tem recursos
que o painel X11 **não** tem, e isso deixa de ser dívida pra virar
diferença de papel: `ABRIR` com seletor de arquivo nativo e **desfazer**
(`Ctrl+Z`) dependem de API multiplataforma que o painel não tem. O painel
segue sendo a ferramenta de teste; o JUCE é o artefato publicável.

### Excelência de áudio — auditado 2026-09-15

Contra `RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md`.

| Requisito (§3/§5) | Estado |
|---|---|
| NaN/Inf nunca atingem o dispositivo | ✅ guarda no sink + `test_sink_guard` |
| Limitador look-ahead, teto verdadeiro | ✅ `OutputStage` no MASTER |
| Sem alocação no callback de áudio | ✅ (a fila de entrada e o `recBuf` reservam no `prepare`) |
| Automação sem clique (gain/width/mute) | ✅ corrigido — GAIN e WIDTH não tinham rampa |
| Blocos irregulares, troca de taxa | ✅ `test_output_excellence` |
| Downmix mono e correlação | ✅ `test_output_excellence` |
| Dither só em PCM fixo | ✅ TPDF no `writeWav16`, nada em float |
| Medição BS.1770 M/S/I | ✅ `src/dsp/Loudness.hpp` + fixtures EBU |
| Taps nomeados de gravação | ✅ `post-safety` e `pre-safety`, por `RASGO_REC_TAP` |
| Exportação PCM24 / float | ⬜ só PCM16 |
| Perfil de loudness declarado | ⬜ **decisão do autor** — não existe alvo único pra palco, álbum e streaming |

Os três pendentes dependem de uma decisão de publicação, não de esforço.

Efeito colateral bom da correção de A2: o `ScopeTrace` e o banco de
Goertzel saíram de `panel_main.cpp` para `src/ui/ScopeTrace.hpp`, então
**os dois front-ends desenham o mesmo gráfico a partir do mesmo código**
— e o painel X11 ficou 20 linhas menor.

**Uma pergunta que a extração levantou e é sua para decidir:** o banco de
Goertzel sempre recebeu `24000` como taxa de amostragem. Isso não é a taxa
do áudio (48 kHz) nem a do anel decimado (~9,6 kHz com bloco de 256) — ou
seja, o eixo de frequência do espectro nunca correspondeu ao conteúdo.
Preservei o valor como estava, porque corrigir muda a aparência do
espectro que você já conhece, e isso é decisão sua. O parâmetro está
explícito em `scopeSpectrum(...)`: a correção é uma linha quando quiser.

---

## A. Bugs — coisas que ESTÃO lá e estão quebradas

Estas são falhas minhas, não escopo faltando.

### A1. Nada anima — não existe `Timer` no app JUCE  ⚠️ raiz de vários relatos

O app só repinta em resposta a evento de mouse (`repaint()` aparece 13
vezes, todas dentro de handlers de entrada). **Não há
`juce::Timer`/`startTimerHz` em lugar nenhum.** O painel X11 redesenha a
cada 33 ms (`panel_main.cpp:3622`).

Consequência: tudo que se move sozinho está parado —

- osciloscópios e espectros (ver A2);
- o anel do knob quando o parâmetro está **modulado** (o JUCE já lê o
  valor modulado, `parameterValue`, em `RasgoModularApp.cpp:761`; ele
  simplesmente nunca é repintado);
- LEDs/toggles que refletem estado do motor;
- VU do MASTER, lanes do TRIGSEQ, flash de botão.

É uma causa só, e é barata: um `Timer` a 30 Hz.

### A2. `Display` desenha uma caixa vazia

`RasgoModularApp.cpp:864` pinta o retângulo recuado, a borda… e para.
O painel X11 (`panel_main.cpp:1681-1821`) desenha ali **quatro coisas
diferentes** conforme o módulo:

| Vista | Módulo | O que é |
|---|---|---|
| onda | qualquer | osciloscópio da saída 0, normalizado pelo pico |
| espectro | SCOPE (clique alterna) | banco Goertzel log de 24 bandas |
| VU + clip-latch | MASTER | pico vs. teto −1 dBFS, com indicador de clipe que decai em ~2 s, lido de `Master::gainReductionDb()` |
| piano-roll | TRIGSEQ | 4 lanes de gate (t1–t4) rolando |

Falta também toda a **alimentação**: o `ScopeTrace` (anel de 220 amostras
por módulo + 4 lanes de 128, `panel_main.cpp:136`) e o preenchimento pelo
thread de áudio (`panel_main.cpp:567-588`). Nada disso existe no JUCE.

O `ScopeTrace` e a matemática do espectro são **framework-free** — devem
sair de `panel_main.cpp` para um header compartilhado, não ser
reescritos.

### A3. Rolagem da coluna esquerda não funciona

Há `mouseWheelMove` (`RasgoModularApp.cpp:229`), mas ele é inócuo por um
erro meu: o laço de pintura faz `if (y > learnTop) break;` **antes** de
`contentH_ = y + scroll_`. Ou seja, a altura do conteúdo é medida com o
laço já interrompido na borda visível — nunca passa da viewport, então
`maxScroll` dá ~0 e a roda não tem para onde rolar. Mesma classe do bug
de rolagem do rack que corrigi ontem (medida derivada do que ela deveria
determinar).

Além disso **não há barra de rolagem visível** — só a roda, que o autor
não tinha como adivinhar.

### A4. Hit-test da paleta depende da pintura ter rodado

`rows_` é preenchido dentro do `paint()`. Funciona, mas é frágil: se um
clique chegar antes do primeiro quadro, a lista está vazia. Deve virar um
layout calculado, não um efeito colateral do desenho.

---

## B. Ausente — recursos do painel X11 que o JUCE não tem

Separado por **custo real**, que é o que importa para decidir a ordem.

### B1. Custo baixo — é fiação, o código já existe e é framework-free

Estes headers já estão em `apps/`, já estão no include path do alvo JUCE,
e **nenhum deles depende de X11 ou ALSA** (verificado):

| Recurso | Header pronto | Botão no X11 |
|---|---|---|
| VARIA (Motion Engine) | `MotionEngine.hpp`, `MotionField.hpp` | VARIA |
| MUTA / EVOLUI / CRUZA | `PatchGenetics.hpp` | MUTA, EVOLUI, CRUZA |
| Salvar / Banco de patches | `SignalGraph::serialize()` (motor) | SALVAR, BANCO |
| Carregar `.rmp` / retomar sessão | `SignalGraph::deserialize()` (motor) | — |
| SYSTEM SCORE | `ScoreRecorder.hpp` | acompanha REC |
| Gramática de seed | `SeedGrammar.hpp` | caixa de seed |

Só `WindowPolicy.hpp` é dependente de plataforma — e o JUCE não precisa
dele.

**É este bloco que o autor pediu ao dizer "não há botão varia, etc".**
Eu tinha decidido omitir esses botões pelo princípio "botão morto é pior
que botão ausente". O princípio continua certo; a leitura estava errada
— eles não precisavam ficar mortos, porque o recurso por trás **já
estava escrito**.

### B2. Custo médio — interação que existe no X11 e falta portar

| Recurso | Onde no X11 |
|---|---|
| Arrastar módulo para reposicionar | `panel_main.cpp:3389`, `:3461` |
| Soltar módulo sobre a paleta = remover | `:1558` |
| Arrastar módulo da paleta para a case | catálogo → `makeModule` |
| Inspector de cabo (ganho, condutância, relação, ruptura) | 28 ocorrências, `CableInspector` |
| Escolher companion de relação (`pickingCompanion`) | `:8 ocorrências` |
| Caixa de seed **editável** (texto, seleção, clipboard) | `seedBoxFocus`, 17 ocorrências |
| SCOPE: clicar no display alterna onda ↔ espectro | `:3377` |
| Ruptura geral `[espaço]` | `actRupture` |
| Leitura "N mód · M cabos" no cabeçalho | `:1466-1482` |
| VU do MASTER no cabeçalho | `:1455` |
| Flash de botão acionado (~160 ms) | `hdrFlash` |
| **Teclado inteiro** (~25 atalhos) | `:2903-3041` — o JUCE não tem `keyPressed` nenhum |
| Persistência de preferências (idioma, vista do rack) | `dataDir()/ui-lang`, `/rack-view` |
| Overlays TUTORIAL e SOBRE (roláveis, 4 idiomas) | `:2123-2160` |

### B3. Custo alto — entrada de sinal e gravação

| Recurso | Situação no JUCE |
|---|---|
| Entrada de áudio | 17 ocorrências no X11 via `AlsaSource.hpp`; no JUCE tem que ir por `AudioAppComponent` (é *mais* fácil e multiplataforma, mas é código novo) |
| MIDI in | 15 ocorrências via `AlsaMidi.hpp`; no JUCE, `MidiInput` — idem |
| Gravação de áudio (REC) | `recording`/`recBuf`, 10 ocorrências; escrita de arquivo é nova |

Aqui o código X11 **não** é reaproveitável: é ALSA. Mas o equivalente
JUCE é multiplataforma de verdade, o que é justamente o motivo deste
front-end existir.

---

## C. Diferente de propósito (não é bug)

- **Sem `juce::Slider`/`Button` de verdade.** O rack é immediate-mode, um
  `Component` só, igual ao `redraw()` do X11 — porque os widgets são
  descritos por dados (`Panel`/`Widget`) e são milhares.
- **Caixa LEARN silenciosa sem hover.** Decisão de projeto do painel; não
  inventei string de "dica" para preencher.
- **Disciplina de lock do áudio.** `try_to_lock` com reemissão do último
  bloco em fade — copiada de propósito, não reinventada.

---

## D. Ordem de ataque proposta

1. **`Timer` a 30 Hz** — desbloqueia sozinho todos os relatos de
   "animação bugada". Menor mudança, maior efeito.
2. **Extrair `ScopeTrace` + Goertzel para header compartilhado**,
   alimentar no `getNextAudioBlock`, desenhar as 4 vistas de `Display`.
3. **Rolagem da paleta**: medir o conteúdo fora do laço de pintura e
   desenhar barra visível.
4. **Cabeçalho completo**: VARIA/MUTA/EVOLUI/CRUZA/BANCO/SALVAR (B1, é
   fiação), mais leitura de módulos/cabos, VU e flash.
5. **Teclado** — os ~25 atalhos, de uma vez.
6. **Inspector de cabo + caixa de seed editável + arrastar módulo.**
7. **Áudio in / MIDI in / REC** pela API do JUCE.

Os passos 1–4 são o que fecha a distância que o autor está sentindo ao
abrir o app. Os passos 5–7 são o que fecha a **camada 1** do gate de
publicação (`ESTRATEGIA_DE_PUBLICACAO.md`).
