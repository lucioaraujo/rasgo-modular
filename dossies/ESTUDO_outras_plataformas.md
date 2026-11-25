# Rasgo Modular em outras plataformas — ideias de ampliação

Aberto em 24/11/2026, a pedido do autor: "anote essas ideias de ampliação do
rasgo modular para essas outras disponibilidades" e "vamos avançar nelas
também, porém algumas delas não tenho como testar".

## 1. O que já existe (v0.1.13)

| Forma | Onde | Teste |
|---|---|---|
| Aplicativo | Windows, macOS, Linux (pacotes da release) | autor no Linux; CI nos três; autor em Windows 10 e Mac reais em 6/10 |
| Plugin VST3 / LV2 | DAWs com VST3; LV2 no Linux (download próprio desde a v0.1.13) | autor no Ardour 8.4 (Linux); CI carrega e confere som e estado nos três |
| Plugin do VCV Rack 2 | `rasgo-modular-vcv`, `.vcvplugin` para win-x64, mac-x64, mac-arm64, lin-x64 | autor no VCV Rack Free (Linux) |
| Áudio e MIDI do sistema | SIGNAL-IN / NOTE-OUT com qualquer programa (DAW, Pd, Max, TouchDesigner, JACK) | autor no Linux |
| Linha de comando | `rasgo_modular_cli`: seed em WAV sem placa de som | local |
| Maestruc | orquestra da família RASGO | local |

O Cardinal recusou a inclusão (#953, política contra código feito com IA);
decisão do autor: sem fork. Ver `ESTUDO_port_vcv_cardinal.md`.

## 2. Ideias, na ordem recomendada

1. **Audio Unit (AU)** — Logic e GarageBand (e MainStage) não usam VST3. O
   JUCE gera AU com uma linha no `juce_add_plugin` (`FORMATS VST3 AU`).
   Custo baixo; o `auval` da Apple roda na CI macOS. **Teste:** só CI (o
   autor não tem Mac à mão); pedir a um usuário de Logic.
2. **Navegador (WebAssembly)** — os módulos tocando na própria página do
   site e do tutorial. O motor (`src/core`, `src/dsp`) não depende de
   framework; um AudioWorklet chama o grafo. Interface própria em canvas,
   leve. **Teste:** o autor testa no navegador (Linux). Divulgação forte.
3. **Hardware LV2: MOD Dwarf/Duo X, Zynthian** — pedaleiras e sintetizadores
   Linux ARM que carregam LV2. O Rasgo como caixa generativa. Pede
   compilação aarch64, um LV2 sem a janela JUCE (os parâmetros viram os
   controles do aparelho) e, na MOD, o pacote para a loja deles.
   **Teste:** o autor não tem o aparelho; a MOD tem um emulador
   (`mod-host` + `mod-ui`) que roda no Linux e cobre parte.
   **Detalhe (24/11, conversa do autor):** já existe um LV2 (JUCE, Linux
   x86-64, na v0.1.13). Para o MOD faltam: (a) uma casca LV2 **sem JUCE**
   (o MOD não abre janela; a interface é o modgui na página web dele),
   gerada como a do VCV; (b) compilação aarch64 pelo `mod-plugin-builder`;
   (c) a forma — recomendação: **cada módulo como um plugin LV2** (cadeias
   de pedal, leve para o ARM do Dwarf), começando por um lote de 5–10.
   Teste sem hardware: **MOD Desktop** (gratuito, Linux x86, roda a mesma
   casca). Loja do MOD: revisão deles. **Decisão do autor (24/11): por ora, não
   enviar nada ao fórum do MOD** (nem pedido à loja); os plugins LV2, se
   feitos, ficam publicados por conta própria, como o `.vcvplugin`.
4. **Raspberry Pi / mini computador** — caixa generativa autônoma, sem
   tela, para instalação: a linha de comando tocando ao vivo (seed ou
   patch, VARIA ligado) na saída de áudio. Compilação aarch64.
   **Teste:** precisa de uma placa (barata); dá para simular com QEMU.
5. **Externals para Pure Data e Max** — cada módulo vira um objeto (`rasgo.osc~`),
   com a mesma ideia da casca do VCV (`Casca.hpp`). Pd é livre e roda no
   Linux: **o autor testa**. Max só com licença; o SDK do Max é gratuito.
6. **Celular (iOS AUv3, Android)** — o JUCE cobre os dois, mas a interface
   precisa ser repensada para o toque. Maior custo; por último.

## 3. Princípios

- Cada forma nova usa o **mesmo DSP** (o motor do Modular), copiado ou
  ligado por uma casca, como no VCV — nada de reescrever módulos.
- O aplicativo continua sendo o instrumento completo (seeds, VARIA, cabos
  como processo); as outras formas levam parte dele para outros lugares.
- Publicar só o que tem algum teste: do autor quando ele puder, senão da CI
  (que carrega, toca e confere), e dizer no site qual foi.

## 4. Feito depois da v0.1.13 (24/11, autor: "avance nas ideias novas")

- **AU no macOS:** formato ligado no CMake; a CI valida com `auval` e põe o
  `.component` no zip do plugin. Sem Mac para testar: só a CI.
- **LV2 por módulo, sem JUCE** (`apps/lv2/rasgo_lv2.cpp`): 100 plugins num
  bundle `rasgo-modular.lv2`; teste próprio no CTest; o lilv valida os 100.
  É a base do MOD, do Raspberry Pi e do Zynthian. Falta: teste do autor no
  Ardour e no MOD Desktop; compilar para aarch64 (CI com máquina ARM).
- **WebAssembly:** Emscripten 4.0.10 instalado (autor: "pode instalar o
  emscipten"; `third_party/emsdk`, 1,4 GB, proveniência lá). Protótipo em
  `apps/web/` (`build.sh`): o motor inteiro em 2,3 MB de wasm autônomo,
  tocando uma seed num AudioWorklet; página de teste com TOCAR, SEED NOVA,
  PARAR, número da seed e volume. Conferido no Node: a seed
  4303935450909092226 monta os mesmos 63 cabos e soa igual ao app nativo
  (RMS −33,5 dBFS, pico −22,0 dBFS), a 5× o tempo real; montar a seed leva
  ~1,8 s (o equilíbrio de volume).
- **VARIA no navegador** (25/11): a mão caótica (`MotionEngine`) no motor
  wasm, no caminho do app JUCE — um passo a cada ~1/30 s, energia da saída
  (RMS × 2,2 ou redução de ganho do MASTER × 0,18), LUFS de curto prazo
  guiando chão e teto, cabos relidos a cada 30 passos. Ligado por padrão,
  como no app; botão VARIA e intensidade 0–2 na página. Conferido no Node,
  30 s por seed (4303935450909092226, 597512815, 42): 4–11× o tempo real
  com o VARIA, pico sempre abaixo de 0 dBFS (−19 a −7 dBFS), nível e brilho
  mudando segundo a segundo em relação ao VARIA desligado.
- **Escuta do autor** (25/11, Firefox e Chrome no Linux; VARIA e volume funcionam): "funciona, mas parece mais simples que o
  estágio atual de desenvolvimento do rasgo modular juce". O som é o do
  app; falta o instrumento em volta (rack, cabos, knobs, visores, LEARN,
  REC…). Caminhos propostos: (1) demonstração de escuta no site; (2) o
  rack da seed só para ver, com as imagens de painel do site; (3) rack
  tocável em canvas, uma segunda interface. Decisão do autor: **o 3**, "rack
  tocável completo". No Opera o som pareceu "mais estranho" — sem saber se
  eram as seeds; o motor soa igual em 44,1 e 48 kHz (conferido no Node), e a
  página passou a mostrar a taxa do navegador. Teste pendente: a mesma seed
  no Opera e no Chrome.

## 6. O rack tocável no navegador (desde 25/11)

Plano em quatro lotes, cada um com imagens para o autor antes do seguinte:

1. **F1 — o rack aparece** (feito, 25/11): os módulos da seed desenhados
   como no app, com os cabos, e os controles se mexendo com o VARIA.
2. **F2 — a mão no instrumento** (feito, 25/11; falta o teste do autor):
   arrastar knob/slider (220 px = a faixa; exponencial nas frequências),
   roda do mouse (2 %, Shift 0,5 %, seletores um passo), clicar toggle;
   puxar cabo de um jack, com os halos do app (duplo = mesmo tipo de sinal,
   contínuo = chega ao som, tracejado = ainda não); puxar de uma entrada
   ocupada leva o cabo dali; botão direito num jack corta; um laço vira
   realimentação (o `prepare` dentro do try, como `tryPatch`); clique num
   cabo abre o inspetor (NONE/RING/FOLD/DIFF, companion escolhido num jack
   de saída, AMT, GAIN, COND, ROMPER/RECONECTAR, DESPLUGAR); cabo sob o
   mouse em destaque; selo da relação no meio do cabo; na vista SAÍDA quem
   deixa de chegar ao som fica tracejado; o VARIA espera enquanto a mão
   está num controle. Conferido no motor pelo Node (`teste_f2.mjs`: girar,
   cortar, religar, laço, cortar saída, GAIN, COND, RING, NONE, ROMPER) e
   por captura (`?insp=N&rel`). A interação com o mouse não foi testada
   por mim: sem entrada sintética na máquina do autor. Ainda sem: DESFAZ,
   mover módulos pelo título, o [x] de remover (vão com a paleta, F3).
3. **F3 — o que falta do painel:** os visores (SCOPE, KEY, VU…), a MATRIX,
   o cabeçalho do app (seed, banco, VARIA, volume, loudness), a paleta para
   acrescentar e tirar módulos. Em lotes:
   - **F3a, visores e MATRIX** (feito, 25/11; falta o teste do autor): a
     telemetria de `Rack::feedScopes` portada para o motor web
     (`rw_visor`: o anel da onda, as 4 lanes e extras por tipo — histograma
     do KEY e do COHERENCE, pesos do PALCO, redução de ganho do MASTER), com
     o bloco de 512 do app emulado sobre o quantum de 128, para o visor
     cobrir o mesmo tempo. O desenho de `paintDisplay`, tipo a tipo, em
     `apps/web/visores.js`; o motor só manda os visores que estão na tela.
     SCOPE: clique no visor alterna onda ↔ espectro. MATRIX: as 16 células
     de `matrixCellMM`, arrasto vertical em cada uma. Conferido no Node
     (`teste_visor.mjs`: 100 visores, sinal nos do caminho da seed) e por
     captura (`?previa&rolar=fim`). Ainda sem: o navegador do banco de
     escalas do QUANTIZER (o clique no visor; vai com o cabeçalho e o
     banco). O custo do quadro com os visores não foi medido: a captura
     sem janela roda em tempo virtual.
   - **F3b, o cabeçalho** (feito, 25/11; falta o teste do autor): a
     `HeaderBar` do app em HTML, com os mesmos grupos e réguas — marca ·
     seed SEED REPOR | VARIA (o slider, 0 = desligado) MUDA EVOLUI CRUZA |
     BANCO SALVA ABRIR | DESFAZ DESCABEIA ESPERA | ZOOM − + RACK ·
     TODOS/SAÍDA, e à direita a leitura (módulos · cabos), o VU do MASTER e
     o ABRE —, o realce de 160 ms em cada ação e os atalhos do cartão
     ATALHOS (g v m e c n r, espaço, Ctrl+Z/S/B/O, Ctrl+=/−/0). No motor
     web: a pilha do DESFAZ (24 fotos em texto, como `Rack::snapshot`; cada
     cabo ligado/cortado e cada gesto do inspetor entram), `mutatePatch`/
     `evolvePatch`/`crossPatch` com MIXER e MASTER congelados, DESCABEIA
     (fica o cabo da saída), ESPERA (o `mute` do MASTER), REPOR, e o `.rmp`
     do app para salvar e abrir (mesmo formato: um patch do navegador abre
     no app e vice-versa). Conferido no Node (`teste_f3b.mjs`). O que é do
     navegador, e por isso escolha minha para o autor revisar: o TOCAR
     (o navegador só deixa soar depois de um clique; some depois), o
     VOLUME da página, SALVA guardando a sessão no próprio navegador
     (localStorage), BANCO baixando `seed-N.rmp`, ABRIR pelo seletor de
     arquivo, e o ABRE valendo no primeiro TOCAR (ÚLTIMA guarda a sessão a
     cada 5 s). Fora deste lote: REC, idioma, TUTORIAL e SOBRE (F4; botão
     sem função não entra, como no app) e o navegador do banco de escalas
     do QUANTIZER (próximo lote, com a paleta).
4. **F4 — o resto do app:** tutorial, LEARN, REC (WAV e .mid para baixar),
   4 idiomas, o lugar no site. REC: teto de 4 minutos, como no app (autor,
   25/11: "20 min é muito", "no app são 4 minutos"); o WAV montado na
   página e baixado, nada vai a servidor.

**Como é feito.** O painel de cada módulo não é copiado: o motor web
entrega o `Panel` do próprio módulo (`Signal::panel()`, posições em mm),
os parâmetros com faixa e os cabos (`rw_descrever`, `rw_valores`,
`rw_cabos_estado`); a página (`apps/web/rack.js`) traduz o desenho do app
JUCE — `relayout`, `paintChrome`, `paintWidget`, `paintCables` — com as
mesmas medidas, cores, regras de rótulo e o wordmark (`RasgoWordmark.h`).
A ordem dos módulos e as cópias de voz seguem `Rack::build` +
`syncSeedCopies`; a vista SAÍDA usa o mesmo `nodesFeeding` (seed
4303935450909092226: 34 de 105, os mesmos do app).

**Custo de desenho** (lição do Pong): camada fixa por módulo rasterizada uma
vez; densidade de pixel até 1,5; sem sombra borrada; canvas do tamanho da
janela, só os módulos à vista; controles móveis juntados por cor em poucos
traços; cabos de condução 1 numa camada refeita só ao rolar. Medido num
Chrome sem janela (desenho por software, CPU limitada a 2 núcleos): de
9,4 ms para 0,3 ms por quadro na vista SAÍDA.

**Testes:** `?previa[=s]` roda o motor na página, sem som, e desenha o
estado (capturas num navegador sem saída de áudio); `?medir` mede o quadro;
`?saida` abre na vista SAÍDA; `?auto` toca sem clique.

**Ainda diferente do app na F1:** visores vazios (só a moldura), MATRIX sem
as células, sem selo de relação nos cabos, sem cabeçalho/paleta/LEARN do
app; fonte do sistema no lugar da fonte do JUCE.

## 5. Próximo passo

Depois da v0.1.13: AU (item 1), então protótipo no navegador (item 2).
