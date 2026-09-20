# Rasgo Modular — mudanças / changelog

Formato: uma seção por versão, do mais novo para o mais antigo. Datas em
ISO. Segue a mesma postura do resto do projeto: registra o que **foi
feito**, com as limitações ditas em voz alta, e não o que deveria estar
pronto.

---

## v0.1.0 — **preparada, ainda não cortada**

> **Por que ainda não é uma release.** Falta a **sessão de escuta
> documentada** (`dossies/VALIDACAO_v0.1.0.md`, Parte A — quatro estudos),
> que é o bloqueio duro da camada 2 da
> `ESTRATEGIA_DE_PUBLICACAO.md`. Nenhum trabalho de código destrava isso:
> alguém precisa ouvir e registrar o que ouviu. Estas notas ficam prontas
> para o momento em que a tag for criada.

Primeira versão publicável do instrumento. O que ela é: um ambiente
modular generativo que **soa ao abrir**, sem MIDI e sem entrada de áudio —
MIDI, áudio e acoplamento de instrumento são nós adaptadores opcionais.

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
- CI de três sistemas (`.github/workflows/package.yml`) compila, testa e
  empacota em Linux, Windows e macOS, e confere com `lipo`/`otool` que o
  `.app` é Universal 2 de verdade.

### Limitações declaradas

Estão aqui porque o gate editorial do RASGO exige que sejam ditas, não
supostas:

- **Windows e macOS nunca foram abertos.** A CI prova que **constrói e
  empacota**; não prova que abre, soa e se comporta numa máquina real. Não
  há Windows nem Mac no ambiente de desenvolvimento. A matriz completa
  está em `INSTALL.md`.
- **O workflow de CI está inerte** enquanto o projeto vive dentro do
  monorepo `rasgo-instruments` — o GitHub Actions só lê o
  `.github/workflows/` da raiz do repositório. Passa a valer na extração
  para repositório próprio.
- **Exportação PCM24/float** e o perfil de loudness dependem de um alvo de
  publicação declarado (palco / álbum / streaming). Sem esse alvo, escolher
  um perfil seria arbitrário.
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
  ceiling), **BS.1770-4 / EBU R128 metering**, and **two named recording
  taps** — `post-safety` (what you heard) and `pre-safety` (before the
  protection).
- The audio thread never waits on the UI: it takes the graph with
  `try_to_lock` and re-emits the last block with a ramp back to unity if
  the UI holds it.
- **77 automated tests**, no window server or audio device required.

**Declared limitations.** Windows and macOS have **never been opened** —
CI proves the build and the package, not the behaviour on real hardware.
The CI workflow is inert until the project is extracted to its own
repository. PCM24/float export and the loudness profile await a declared
publication target. Linux ARM is untested. The module catalogue is open by
design.

AGPL-3.0-or-later; JUCE under AGPLv3, no commercial licence.
