# Rasgo Modular — instalação / installation

> **Estado (3 out. 2026):** a versão atual é a **v0.1.2**, com
> instaladores para Linux, Windows e macOS na
> [página da release](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.2)
> (a v0.1.1 saiu em 2 out. e a v0.1.0 em 29 set. 2026). Este documento descreve também como
> construir e rodar a partir do código. *English:* the current version is
> v0.1.2 — installers on the release page above; the English section is
> further down.

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

### Requisitos

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

**Limitações do `.deb`** (inspecionado no pacote que a CI gerou):

- **exige Ubuntu 24.04+ / Debian 13+.** A dependência é `libasound2t64`,
  nome que vem da transição do `time_t` para 64 bits; no Ubuntu 22.04
  (suporte até 2027) o pacote ainda se chama `libasound2` e a instalação
  falha. Isso não foi escolhido: vem de a CI usar `ubuntu-latest`. Fixar
  o runner em `ubuntu-22.04` ampliaria o alcance, ao custo de compilar
  contra bibliotecas mais antigas;
- **só cobre a família Debian.** Fedora, Arch e openSUSE precisam
  compilar do código — um AppImage cobriria todos de uma vez, se vier a
  fazer falta.

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

### Requirements

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
