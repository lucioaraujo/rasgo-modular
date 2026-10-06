# Rasgo Modular — instalação / installation

> **Estado (6 out. 2026):** a versão atual é a **v0.1.4**, com pacotes
> para Linux, Windows e macOS na
> [página da release](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.4)
> (antes: v0.1.3 em 4 out., v0.1.2 em 3 out., v0.1.1 em 2 out., v0.1.0 em
> 29 set. 2026). Este documento descreve também como
> construir e rodar a partir do código.
>
> **Para instalar, comece por ["Instalar, passo a passo"](#instalar-passo-a-passo)**:
> lá estão os avisos do Windows e do macOS e o que fazer em cada um.
> *English:* the current version is v0.1.4. Start with
> ["Installing, step by step"](#installing-step-by-step).

---

## Matriz de plataformas

Cada combinação está marcada com o que foi **realmente feito**, não com o
que deveria funcionar.

| Plataforma | Front-end | Build | Execução | Áudio verificado |
|---|---|---|---|---|
| Linux x86-64 | JUCE (`RasgoModularApp`) | ✅ local | ✅ local | ✅ em hardware real |
| Linux x86-64 | painel X11 (teste) | ✅ local | ✅ local | ✅ em hardware real |
| Windows x86-64 | JUCE | ✅ na CI (verde em 21 set. 2026) | ❌ nunca aberto | ❌ |
| macOS (Universal 2) | JUCE | ✅ na CI (verde em 21 set. 2026) | ❌ nunca aberto | ❌ |
| Linux ARM | — | ❌ | ❌ | ❌ |

**O que "na CI" quer dizer, e o que NÃO quer.** O workflow de três
sistemas (`.github/workflows/package.yml`) compila o motor, roda os 77
testes, compila o app e gera o instalador em cada sistema. No macOS ele
ainda confere com `file`/`lipo`/`otool` que o `.app` é Universal 2 de
verdade — e a verificação passou com o binário trazendo as duas fatias
(x86_64 e arm64) e `minos 10.15`, o alvo do projeto, e **não** o do
runner, que roda macOS 26.

Isso prova que **constrói, passa nos testes e empacota**. Não prova que
abre, soa e se comporta bem numa máquina real, porque não há Windows nem
Mac neste ambiente. O Antitotem foi publicado nessa mesma condição, com a
decisão registrada explicitamente; o mesmo vale ser dito aqui em vez de
deixar o leitor supor.

**Correção de uma afirmação anterior.** Até 21 set. 2026 este documento
dizia que a CI "prova que constrói e empacota" nos três sistemas. **Não
provava**: o workflow nunca tinha rodado uma única vez, porque o GitHub
Actions só lê `.github/workflows/` da raiz do repositório e o projeto
vivia dentro do monorepo. Quando finalmente rodou, os três jobs falharam,
e foi preciso corrigir cinco defeitos reais — dois deles impediam a
compilação por completo (o `std::filesystem` contra o deployment target
de 10.13 no macOS; o `M_PI`, que o MSVC não define, em 83 lugares do
código). A frase acima só passou a ser verdadeira depois disso, e fica
registrado que antes não era.

**Decisão do autor (21 set. 2026):** publicar a `v0.1.0` **nessa
condição**, com Windows e macOS verificados apenas pela CI — o mesmo
precedente do Antitotem. A decisão fica registrada aqui, e a página
editorial dirá o mesmo: o gate do RASGO exige que a limitação seja dita,
não suposta.

**Histórico dessa ressalva:** até 21 set. 2026 o workflow estava
**inerte**, porque o projeto vivia dentro do monorepo `rasgo-instruments`
e o GitHub Actions só lê `.github/workflows/` da raiz do repositório. Com
a extração para [`lucioaraujo/rasgo-modular`](https://github.com/lucioaraujo/rasgo-modular)
— preservando o histórico — ele passou a rodar de fato.

---

## Português

### Instalar, passo a passo

Os avisos de segurança abaixo **são esperados**: o Rasgo Modular é software
livre, publicado sem certificado pago de assinatura digital. Eles não
indicam defeito nem vírus. Aparecem só na primeira vez.

#### Windows

1. Na [página da release](https://github.com/lucioaraujo/rasgo-modular/releases/latest),
   baixe `rasgo-modular-<versão>-windows-x64.exe`.
2. Abra o arquivo. O Windows mostra uma janela azul **"O Windows protegeu o
   computador"**. Clique em **"Mais informações"** e depois em
   **"Executar assim mesmo"**.
3. Se o Windows pedir permissão de administrador, clique em **"Sim"**. O
   programa é instalado em Arquivos de Programas.
4. Clique em **"Avançar"** até o fim. A partir da v0.1.4, a última tela já
   traz **"Executar o Rasgo Modular"** marcado.
5. Depois disso, o Rasgo Modular fica no **Menu Iniciar** e na área de
   trabalho.

**Sem instalar** (a partir da v0.1.4): baixe o `.zip`, descompacte-o e abra
`Rasgo Modular.exe`. O aviso do passo 2 aparece do mesmo jeito na primeira
vez.

**Se você instalou a v0.1.3 ou anterior e nada acontece ao abrir**, ou
aparece um erro sobre `VCRUNTIME140.dll` ou `MSVCP140.dll`: falta no seu
Windows o pacote "Microsoft Visual C++ Redistributable", que essas versões
exigiam sem avisar. Instale-o pelo link da Microsoft
(<https://aka.ms/vs/17/release/vc_redist.x64.exe>) e abra o Rasgo Modular
de novo. A partir da v0.1.4 isso não é mais necessário, porque tudo vai
dentro do `.exe`.

**Se ainda assim não abrir:** aperte **Win + R**, digite
`%APPDATA%\rasgo-modular` e dê Enter. Envie o conteúdo de `arranque.log` e
de `crash.log` (se existir) [numa issue](https://github.com/lucioaraujo/rasgo-modular/issues).
A última linha do `arranque.log` diz em que ponto a abertura parou.

#### macOS

1. Na [página da release](https://github.com/lucioaraujo/rasgo-modular/releases/latest),
   baixe `rasgo-modular-<versão>-macos-universal.dmg`. O mesmo arquivo
   serve para Mac Intel e Apple Silicon.
2. Abra o `.dmg` com dois cliques e **arraste "Rasgo Modular" para a pasta
   Aplicativos**, que aparece como atalho na própria janela.
3. Abra o Rasgo Modular em Aplicativos. Na primeira vez, o macOS **bloqueia**
   o app e mostra uma mensagem dizendo que **a Apple não pôde confirmar que
   ele está livre de software malicioso**. Num Mac em francês, por exemplo:
   *« Rasgo Modular ne peut pas être ouvert. Apple n'a pas pu confirmer que
   Rasgo Modular ne contenait pas de logiciel malveillant. »*
   - Isso **não indica defeito nem vírus**. O macOS faz isso com todo app
     baixado da internet que não passou pela notarização da Apple, que
     exige uma conta paga.
   - Clique em **"OK"** ou **"Concluído"**. **Não** clique em "Mover para o
     Lixo".
4. Libere o app de um destes dois jeitos. Basta fazer uma vez.

   **Pelos Ajustes (sem Terminal):** abra **Ajustes do Sistema →
   Privacidade e Segurança** e desça até a parte de segurança. Lá aparece
   a linha "Rasgo Modular foi bloqueado…". Clique em **"Abrir Mesmo
   Assim"**, confirme com sua senha ou Touch ID e clique em **"Abrir"**.
   - **No macOS 14 ou anterior há um atalho:** no Finder, clique no app com
     o botão direito (ou Control + clique), escolha **"Abrir"** e depois
     **"Abrir"** de novo.

   **Pelo Terminal**, se o botão não aparecer ou o bloqueio continuar. Foi
   assim que o autor abriu a v0.1.4 num Mac real, em 6 out. 2026:
   1. Abra o **Terminal**. Ele fica em Aplicativos → Utilitários, ou é só
      apertar Cmd + Espaço e digitar "Terminal".
   2. Copie a linha abaixo, cole no Terminal e aperte **Enter**:

      ```sh
      xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"
      ```

      Se nada aparecer depois do Enter, deu certo: o comando não responde
      quando funciona.
   3. Feche o Terminal e abra o Rasgo Modular normalmente.

   O comando só retira a "marca de quarentena" que o macOS põe em todo
   arquivo baixado. Ele não altera o app.
5. Quando você ligar o botão **ON** do módulo SIGNAL-IN, o macOS pede
   permissão para usar o microfone. Permita, se quiser tocar com entrada de
   áudio.

**Se aparecer "Rasgo Modular está danificado e não pode ser aberto"**
(v0.1.3 ou anterior, em Mac com chip Apple): o arquivo não está danificado.
Essas versões não tinham o pacote assinado por inteiro. O mesmo comando do
Terminal, do passo 4, resolve.

**Por que esse aviso existe e quando vai sumir:** ele só desaparece com a
assinatura **Developer ID** e a **notarização** da Apple. As duas exigem o
Apple Developer Program, que é pago (anual). Essa é uma decisão ainda em
aberto. Até lá, o caminho é o deste passo a passo.

#### Linux

- **Ubuntu, Debian e Mint:** dê dois cliques no `.deb` ou use
  `sudo apt install ./rasgo-modular-<versão>-linux-x86_64.deb`.
- **Outras distribuições** (a partir da v0.1.4): use o **AppImage**. Marque
  o arquivo como executável (propriedades → permitir executar, ou
  `chmod +x`) e abra. Ou use o **`.tar.gz`**: descompacte-o e rode
  `./install.sh`, que instala no seu usuário, sem root.

### Requisitos para usar

| | |
|---|---|
| **Windows** | Windows 10 (versão 1607 ou posterior) ou Windows 11, 64 bits (x86-64) |
| **macOS** | macOS 10.15 Catalina ou posterior, Intel ou Apple Silicon (Universal 2) |
| **Linux** | x86-64 com glibc 2.35 ou mais nova: Ubuntu 22.04+, Debian 12+, Mint 21+ (o `.deb`); outras distribuições pelo AppImage ou pelo `.tar.gz` (testados na abertura em Debian 12, Ubuntu 24.04, Fedora e Arch) |
| **Processador** | 64 bits, dois núcleos ou mais. Medido num Intel Core i5-6500 (2015, 4 núcleos, 3,2 GHz): o app inteiro usa de um terço a metade de um núcleo; o som sozinho, de 7% a 15% |
| **Memória** | ~55 MB em uso (medido); qualquer computador com 4 GB basta |
| **Disco** | ~20 MB |
| **Tela** | 1280 × 760 ou maior |
| **Áudio** | qualquer saída de áudio do sistema; microfone e teclado MIDI são opcionais (módulo SIGNAL-IN, botão ON) |

O mínimo de sistema vem do build (macOS 10.15 é o alvo declarado no
`CMakeLists.txt`; Windows 10 1607 é o mínimo do JUCE; glibc 2.35 é a do
Ubuntu 22.04, onde o `.deb` é compilado). O de processador e memória é o
que foi **medido** na máquina do autor; máquinas mais fracas não foram
testadas.

### Requisitos para compilar

**Motor e testes** (não precisam de GUI nem de JUCE):

- CMake ≥ 3.22
- compilador C++17 (GCC 11+, Clang 14+, MSVC 2022)

**App JUCE** (front-end de produção), além do acima:

- um checkout do [JUCE](https://github.com/juce-framework/JUCE) — **não é
  versionado aqui**, por política da família RASGO (nem submodule, nem
  FetchContent)
- Linux: `libasound2-dev`, `libfreetype6-dev`, `libfontconfig1-dev`,
  `libx11-dev`, `libxcomposite-dev`, `libxcursor-dev`, `libxext-dev`,
  `libxinerama-dev`, `libxrandr-dev`, `libxrender-dev`, `libgtk-3-dev`,
  `libglu1-mesa-dev`
- Windows: Visual Studio 2022; NSIS se quiser gerar o instalador
- macOS: Xcode command line tools

**Painel X11** (ferramenta de teste, só Linux): `libX11`, `libXrandr`,
`libasound2-dev`. O alvo é pulado automaticamente se faltarem.

### Só o motor e os testes

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

76 testes. Nenhuma dependência de áudio ou de janela — é o caminho para
verificar o instrumento num servidor ou num container.

### App JUCE

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DRASGO_MODULAR_JUCE_PATH=/caminho/para/JUCE
cmake --build build --target RasgoModularApp -j
```

O executável fica em `build/apps/juce/RasgoModularApp_artefacts/Release/`.

### Pacote instalável

**O `.deb`** é compilado no Ubuntu 22.04 desde a v0.1.2. Antes, a
dependência saía `libasound2t64` e o pacote não instalava no 22.04. Agora
ele instala no Ubuntu 22.04+, no Debian 12+ e no Mint 21+. Para outras
distribuições, a partir da v0.1.4, há AppImage e `.tar.gz`, gerados por
`packaging/linux/empacotar.sh`.

O pacote instala `/usr/bin/rasgo-modular`, uma entrada `.desktop` com as
categorias de áudio e um ícone SVG escalável.

```sh
cd build
cpack -G DEB          # Linux
cpack -G NSIS         # Windows (precisa do NSIS)
cpack -G DragNDrop    # macOS
```

### Variáveis de ambiente

| Variável | Efeito |
|---|---|
| `RASGO_SEED=N` | abre reproduzindo o patch do seed `N` (render determinístico) |
| `RASGO_RESUME=1` | retoma a sessão salva em vez de sortear um patch novo |
| `RASGO_REC_DIR` | pasta das gravações (padrão: `~/Music/RasgoModular/`) |
| `RASGO_REC_TAP` | de onde o REC grava: `post` (padrão) · `pre` · `both` |

### Taps de gravação

O REC pode gravar de dois pontos da cadeia de saída:

- **`post-safety`** (padrão) — o que se ouviu: depois do limitador, do
  teto e de toda a proteção;
- **`pre-safety`** — o mesmo sinal ANTES da proteção de saída.

Gravar só o primeiro faz o limitador esconder justamente a dinâmica que
se queria examinar; gravar só o segundo mente sobre o que saiu pelos
alto-falantes.

**Os dois nem sempre se comparam como se espera, e saber disso evita
diagnosticar defeito onde não há.** O `pre-safety` sai do MASTER, antes
da proteção; o `post-safety` é a saída FINAL, já somada. Então o
`pre-safety` só aparece mais alto e mais dinâmico quando duas condições
valem ao mesmo tempo: o sinal está quente o bastante para a proteção
agir, e o MASTER é o único caminho até a saída. Num patch em que outros
módulos chegam ao OUT por fora do MASTER — o que é comum e legítimo — o
`post-safety` pode sair mais alto, simplesmente porque contém som que o
`pre` nunca viu. Com sinal quente e o MASTER no caminho, a diferença é
inequívoca: pico em +17,9 dB no pre contra −1,0 dB no post (o teto), com
11,9 dB de crista contra 6,2 dB. Com `RASGO_REC_TAP=both` a tomada sai nos dois arquivos,
com o tap no nome (`rec-….post-safety.wav` e `rec-….pre-safety.wav`) —
dois arquivos da mesma tomada soando diferente sem explicação seria uma
armadilha. Ambos em PCM 24 bits.

### Formato de saída e de gravação

| | |
|---|---|
| **Taxa de amostragem** | **a do dispositivo** — o app não impõe uma. Ele abre a saída e adota o que o sistema oferecer (44,1 · 48 · 96 kHz…). A taxa em uso aparece no cartão **SOBRE**, porque a única resposta honesta a "em que taxa estou?" vem da tela, não do código. |
| **Gravação (REC)** | **WAV PCM 24 bits**, sem dither. A tomada não é o arquivo final: vai ser comparada com o tap `pre-safety` e possivelmente masterizada depois, e 16 bits jogariam fora resolução que não volta — ainda mais porque o ganho do MASTER abre com folga abaixo do teto (ajustado por seed para todo patch abrir num volume parecido). Em 24 bits o degrau de quantização fica muito abaixo do ruído do material, então dither só somaria ruído. |
| **Renders de auditoria/CI** | seguem em PCM 16 com dither TPDF desligado, para continuarem byte-idênticos entre execuções (são *goldens*, não obra). |

**Alvo de publicação declarado (21 set. 2026): streaming / plataformas.**
Daí saem **−14 LUFS integrado** e teto de **−1 dBTP**. O cartão SOBRE
mostra a distância até o alvo em LU e o true-peak medido, e marca em cor
de aviso quando o true-peak passa do teto — é essa a condição que estoura
na recodificação com perdas, e ela **não aparece** no pico de amostra.
Nada disso normaliza a saída: o medidor não toca no sinal em lugar
nenhum. O número é para quem está ouvindo decidir.

### Custo de CPU — declarado, medido, e com correção prevista

O app consome **cerca de 70% de um núcleo** num patch comum (medido num
Intel de mesa, 44,1 kHz, bloco de 256). A repartição, por perfil de
execução com `perf`:

| | |
|---|---|
| Desenho da interface | **~38%** |
| DSP (o instrumento em si) | ~11% |
| Medição de loudness (BS.1770 + true-peak) | 0,55% |

**O desenho custa mais que o som**, e a causa está localizada: a
interface repinta a view inteira do rack 30 vezes por segundo, embora só
os osciloscópios, os LEDs e o VU mudem entre um quadro e o outro. O
motor, medido isoladamente sem janela, custa de 6,6% a 14,5% conforme o
patch.

**O que isso significa na prática:** numa máquina de desempenho modesto o
áudio pode falhar. Se acontecer, aumentar o tamanho do buffer no
dispositivo de saída dá mais folga.

Na v0.1.1 foram testadas duas saídas, repintar só as regiões que mudam e
desenhar a moldura opaca, e as duas foram medidas **sem ganho**; ficaram
fora do código, guardadas em branches (`TAREFAS.md`). Na v0.1.2, os
estalos por blocos de áudio perdidos foram corrigidos (de 106 por minuto
para 0, medido no mesmo seed); a tela SOBRE mostra a contagem de blocos
perdidos, que deve ficar em zero.

### Onde o instrumento guarda as coisas

- **Estado e preferências:** `~/.config/rasgo-modular/` no Linux
  (`session.rmp`, `patches/`, `ui-lang`, `rack-view`). No macOS e no
  Windows, o diretório de dados de aplicativo do sistema. (O
  `~/.local/share/rasgo-modular/` que pode existir é do painel de teste
  X11 antigo.) A partir da v0.1.3, apagar o `ui-lang` faz o app voltar a abrir no idioma do
  sistema.
- **Gravações:** `~/Music/RasgoModular/` — `.wav` mais o `.score.txt` da
  tomada. São obra, não estado do app, por isso ficam separadas.

### Problemas comuns

- **Abre mudo.** O instrumento soa ao abrir por desenho. Se não sair som,
  confira o dispositivo de saída do sistema, o botão ESPERA (que cala o
  master) e o `gain` do módulo MASTER, que cada seed ajusta para abrir num
  volume parecido.
- **`RASGO_MODULAR_JUCE_PATH` não definido.** O alvo do app é pulado e só
  o motor é construído; é intencional, pra um checkout limpo compilar sem
  o JUCE à mão.
- **Sem entrada de MIDI/áudio.** Elas só são abertas quando o patch tem um
  módulo `SIGNAL-IN`, por decisão de projeto: o instrumento abre soando
  sem precisar deles, e os adaptadores são opcionais.

### Licença

AGPL-3.0-or-later (`LICENSE`). O JUCE é usado sob AGPLv3, sem licença
comercial — ver [`apps/juce/LICENSE_STATUS.md`](apps/juce/LICENSE_STATUS.md).

---

## English

### Installing, step by step

The security warnings below **are expected**: Rasgo Modular is free
software published without a paid code-signing certificate. They do not
mean anything is wrong, and they only appear the first time.

#### Windows

1. From the [release page](https://github.com/lucioaraujo/rasgo-modular/releases/latest),
   download `rasgo-modular-<version>-windows-x64.exe`.
2. Open it. Windows shows a blue **"Windows protected your PC"** box. Click
   **"More info"**, then **"Run anyway"**.
3. If Windows asks for administrator permission, click **"Yes"**.
4. Click **"Next"** to the end. From v0.1.4 on, the last page has
   **"Run Rasgo Modular"** already ticked.
5. Rasgo Modular is then in the **Start menu** and on the desktop.

**Without installing** (from v0.1.4): unzip the `.zip` and open
`Rasgo Modular.exe`.

**v0.1.3 or earlier and nothing happens**, or an error about
`VCRUNTIME140.dll` or `MSVCP140.dll` appears: your Windows lacks the
"Microsoft Visual C++ Redistributable", which those versions silently
required. Install it from
<https://aka.ms/vs/17/release/vc_redist.x64.exe> and open the app again.
From v0.1.4 on, everything is inside the `.exe`.

**Still not opening?** Press **Win + R**, type `%APPDATA%\rasgo-modular`,
and send `arranque.log` and `crash.log` (if present) in
[an issue](https://github.com/lucioaraujo/rasgo-modular/issues).

#### macOS

1. Download `rasgo-modular-<version>-macos-universal.dmg`. It works on both
   Intel and Apple Silicon Macs.
2. Open it and **drag "Rasgo Modular" into Applications**.
3. Open it. The first time, macOS **blocks** the app with a message saying
   **Apple could not confirm it is free of malicious software**. On a French
   Mac, for example: *« Rasgo Modular ne peut pas être ouvert. Apple n'a pas
   pu confirmer que Rasgo Modular ne contenait pas de logiciel
   malveillant. »*
   - This is **not a defect or a virus**. macOS does it for every downloaded
     app that has not been notarised by Apple, which requires a paid account.
   - Click **"OK"** or **"Done"**, **not** "Move to Trash".
4. Allow the app in one of two ways. You only need to do this once.

   **In System Settings (no Terminal):** go to **System Settings → Privacy
   & Security**, scroll to the security section, click **"Open Anyway"**
   next to "Rasgo Modular was blocked…", confirm with your password or
   Touch ID, and click **"Open"**.
   - **On macOS 14 or earlier there is a shortcut:** right-click (or
     Control-click) the app in Finder, then **Open → Open**.

   **In Terminal**, if the button does not appear or the block remains. This
   is how the author opened v0.1.4 on a real Mac on 6 Oct. 2026.
   1. Open **Terminal**. It is in Applications → Utilities, or press
      Cmd + Space and type "Terminal".
   2. Copy this line, paste it into Terminal and press **Enter**:

      ```sh
      xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"
      ```

      If nothing is printed after Enter, it worked.
   3. Close Terminal and open Rasgo Modular normally.

   The command only removes the "quarantine mark" macOS puts on every
   downloaded file. It does not change the app.
5. Turning on the SIGNAL-IN **ON** switch makes macOS ask for microphone
   access. Allow it if you want audio input.

**"Rasgo Modular is damaged and can't be opened"** (v0.1.3 or earlier, on
Apple Silicon Macs): the file is not damaged. Those versions were not fully
signed. The same Terminal command from step 4 fixes it.

**Why the warning exists, and when it will go away:** it only goes away
with Apple's **Developer ID** signing and **notarisation**. Both require the
paid (yearly) Apple Developer Program, which is still an open decision.
Until then, follow the steps above.

#### Linux

- **Ubuntu, Debian and Mint:** double-click the `.deb`, or run
  `sudo apt install ./<file>.deb`.
- **Other distributions** (from v0.1.4): use the **AppImage** (make it
  executable, then open it), or the **`.tar.gz`** (unpack it and run
  `./install.sh`, which installs for your user only, without root).

### Requirements to run

- **Windows:** Windows 10 (version 1607 or later) or Windows 11, 64-bit.
- **macOS:** macOS 10.15 Catalina or later, Intel or Apple Silicon.
- **Linux:** x86-64 with glibc 2.35 or newer — Ubuntu 22.04+, Debian 12+,
  Mint 21+ for the `.deb`; other distributions via the AppImage or the
  `.tar.gz` (launch-tested on Debian 12, Ubuntu 24.04, Fedora and Arch).
- **CPU:** 64-bit, two cores or more. Measured on an Intel Core i5-6500
  (2015, 4 cores, 3.2 GHz): the whole app uses a third to half of one core;
  the sound alone, 7% to 15%.
- **Memory:** ~55 MB in use (measured). **Disk:** ~20 MB. **Screen:**
  1280 × 760 or larger.
- **Audio:** any system audio output; microphone and MIDI keyboard are
  optional (SIGNAL-IN module, ON switch).

The OS minimums come from the build; the CPU and memory figures were
measured on the author's machine, and weaker machines have not been tested.

### Requirements to build

**Engine and tests** (no GUI, no JUCE): CMake ≥ 3.22 and a C++17 compiler
(GCC 11+, Clang 14+, MSVC 2022).

**JUCE app:** additionally a local [JUCE](https://github.com/juce-framework/JUCE)
checkout — deliberately **not vendored** here — plus the platform
development packages listed in the Portuguese section above.

### Engine and tests only

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

76 tests, no audio device or window server required.

### JUCE app

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DRASGO_MODULAR_JUCE_PATH=/path/to/JUCE
cmake --build build --target RasgoModularApp -j
cd build && cpack -G DEB    # or NSIS / DragNDrop
```

### Environment variables

`RASGO_SEED=N` reproduces a specific patch; `RASGO_RESUME=1` restores the
saved session instead of drawing a new one; `RASGO_REC_DIR` changes the
recording folder; `RASGO_REC_TAP` picks where REC captures from —
`post` (default, what you heard), `pre` (before the safety stage) or
`both`, which writes one file per tap with the tap in its name.

### Where state lives

Session, patch bank and preferences in the platform's application data
directory under `rasgo-modular/`. Recordings (`.wav` + `.score.txt`) go to
the user's music folder — they are work, not application state.

### License

AGPL-3.0-or-later. JUCE is used under AGPLv3, with no commercial licence;
see [`apps/juce/LICENSE_STATUS.md`](apps/juce/LICENSE_STATUS.md).
