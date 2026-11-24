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
   casca). Loja do MOD: revisão deles; conferir a política sobre código
   feito com IA antes de pedir.
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

## 4. Próximo passo

Depois da v0.1.13: AU (item 1), então protótipo no navegador (item 2).
