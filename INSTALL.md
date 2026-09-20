# Rasgo Modular — instalação / installation

> **Estado (18 set. 2026):** candidato publicável em preparação. Ainda
> **não há release** nem pacote assinado. Este documento descreve como
> construir e rodar a partir do código — que é o caminho reproduzível
> hoje, e é o que a `ESTRATEGIA_DE_PUBLICACAO.md` chama de release de
> pesquisa: válida desde que isso seja dito com todas as letras.

---

## Matriz de plataformas

Cada combinação está marcada com o que foi **realmente feito**, não com o
que deveria funcionar.

| Plataforma | Front-end | Build | Execução | Áudio verificado |
|---|---|---|---|---|
| Linux x86-64 | JUCE (`RasgoModularApp`) | ✅ local | ✅ local | ✅ em hardware real |
| Linux x86-64 | painel X11 (teste) | ✅ local | ✅ local | ✅ em hardware real |
| Windows x86-64 | JUCE | ⚠️ só na CI | ❌ nunca aberto | ❌ |
| macOS (Universal 2) | JUCE | ⚠️ só na CI | ❌ nunca aberto | ❌ |
| Linux ARM | — | ❌ | ❌ | ❌ |

**O que "só na CI" quer dizer.** O workflow de três sistemas
(`.github/workflows/package.yml`) compila o motor, roda os 76 testes,
compila o app e gera o instalador em cada um — e, no macOS, verifica com
`lipo` e `otool` que o `.app` é Universal 2 de verdade e carrega o
deployment target do projeto. Isso prova que **constrói e empacota**. Não
prova que abre, soa e se comporta bem numa máquina real, porque não há
Windows nem Mac aqui. O Antitotem foi publicado nessa mesma condição, com
a decisão registrada explicitamente; o mesmo vale ser dito aqui em vez de
deixar o leitor supor.

**Decisão do autor (21 set. 2026):** publicar a `v0.1.0` **nessa
condição**, com Windows e macOS verificados apenas pela CI — o mesmo
precedente do Antitotem. A decisão fica registrada aqui, e a página
editorial dirá o mesmo: o gate do RASGO exige que a limitação seja dita,
não suposta.

**Ressalva adicional:** o workflow está **inerte** enquanto o projeto vive
dentro do monorepo `rasgo-instruments` — o GitHub Actions só lê
`.github/workflows/` da raiz do repositório. Ele passa a valer no momento
da extração pra repositório próprio.

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
alto-falantes. Com `RASGO_REC_TAP=both` a tomada sai nos dois arquivos,
com o tap no nome (`rec-….post-safety.wav` e `rec-….pre-safety.wav`) —
dois arquivos da mesma tomada soando diferente sem explicação seria uma
armadilha. Ambos em PCM 24 bits.

### Formato de saída e de gravação

| | |
|---|---|
| **Taxa de amostragem** | **a do dispositivo** — o app não impõe uma. Ele abre a saída e adota o que o sistema oferecer (44,1 · 48 · 96 kHz…). A taxa em uso aparece no cartão **SOBRE**, porque a única resposta honesta a "em que taxa estou?" vem da tela, não do código. |
| **Gravação (REC)** | **WAV PCM 24 bits**, sem dither. A tomada não é o arquivo final: vai ser comparada com o tap `pre-safety` e possivelmente masterizada depois, e 16 bits jogariam fora resolução que não volta — ainda mais porque o MASTER abre em −24 dB de propósito. Em 24 bits o degrau de quantização fica muito abaixo do ruído do material, então dither só somaria ruído. |
| **Renders de auditoria/CI** | seguem em PCM 16 com dither TPDF desligado, para continuarem byte-idênticos entre execuções (são *goldens*, não obra). |

**Alvo de publicação declarado (21 set. 2026): streaming / plataformas.**
Daí saem **−14 LUFS integrado** e teto de **−1 dBTP**. O cartão SOBRE
mostra a distância até o alvo em LU e o true-peak medido, e marca em cor
de aviso quando o true-peak passa do teto — é essa a condição que estoura
na recodificação com perdas, e ela **não aparece** no pico de amostra.
Nada disso normaliza a saída: o medidor não toca no sinal em lugar
nenhum. O número é para quem está ouvindo decidir.

### Onde o instrumento guarda as coisas

- **Estado e preferências:** `~/.local/share/rasgo-modular/`
  (`session.rmp`, `patches/`, `ui-lang`, `rack-view`). No macOS e no
  Windows, o diretório de dados nativo do sistema.
- **Gravações:** `~/Music/RasgoModular/` — `.wav` mais o `.score.txt` da
  tomada. São obra, não estado do app, por isso ficam separadas.

### Problemas comuns

- **Abre mudo.** O instrumento soa ao abrir por desenho. Se não sair som,
  confira o dispositivo de saída do sistema e o `gain` do módulo MASTER,
  que começa em −24 dB de propósito.
- **`RASGO_MODULAR_JUCE_PATH` não definido.** O alvo do app é pulado e só
  o motor é construído; é intencional, pra um checkout limpo compilar sem
  o JUCE à mão.
- **Sem entrada de MIDI/áudio.** Elas só são abertas quando o patch tem um
  módulo `SIGNAL-IN` — é decisão de projeto, não falta: o instrumento toca
  sozinho e os adaptadores são opcionais.

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
