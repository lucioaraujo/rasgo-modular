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
- **Escuta do autor** (25/11): "funciona, mas parece mais simples que o
  estágio atual de desenvolvimento do rasgo modular juce". O som é o do
  app; falta o instrumento em volta (rack, cabos, knobs, visores, LEARN,
  REC…). Caminhos propostos: (1) demonstração de escuta no site; (2) o
  rack da seed só para ver, com as imagens de painel do site; (3) rack
  tocável em canvas, uma segunda interface. Decisão do autor.

## 5. Próximo passo

Depois da v0.1.13: AU (item 1), então protótipo no navegador (item 2).
