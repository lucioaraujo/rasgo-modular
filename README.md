# Rasgo Modular

![Rasgo Modular running: a three-row rack of modules, each with a thin stripe in its family colour, crossed by dozens of orange and blue cables; one cable is lit and its box is open in the bottom-right corner](screenshots/rack-completo-2026-10-03.png)

**Website:** [lucioaraujo.github.io/rasgo-modular](https://lucioaraujo.github.io/rasgo-modular/) ·
**Download:** [v0.1.1 release](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.1) ·
**Contact:** **rasgo.instruments@gmail.com**

Languages:

- [English](#english)
- [Português](#português)
- [Français](#français)
- [Español](#español)

---

## English

**Patches as a starting point.** Rasgo Modular does not open on a blank
page: it offers a patch already built and sounding, for you to play from —
re-patching by hand, adjusting, letting it drift, or taking it all apart and
building from scratch.

**Authorship:** Lúcio Araújo · **Family:** [RASGO](https://rasgosound.arquiviagem.net/) ·
**Version:** v0.1.1 · **License:** GNU AGPL-3.0-or-later (see [`LICENSE`](LICENSE)
and [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md))

### What it is

A generative modular environment with **58 modules** in eight families
(SOURCE, TRANSFORM, MODULATE, TIME, DECISION, ROUTE, SPACE, OUT), written in
C++17 with no dependencies in the core. Modules are described as data — each
declares its own panel in millimetres — and one engine drives two front-ends:
a cross-platform JUCE app and an X11 test panel.

It is not a plugin and needs no DAW. It opens, sounds, records. MIDI and
audio input exist as optional adapter modules, opened only when a patch
contains one.

### The cable is an object, not a wire

- **Conductance** — a cable can conduct probabilistically, passing
  sometimes rather than always.
- **Relation** — a cable can combine what crosses it with a second signal:
  ring modulation, wavefolding or difference (RING, FOLD, DIFF in the cable
  inspector).
- **Rupture with a scar** — breaking a cable does not cut to silence: the
  scar holds the last block and repeats it, decaying.

### How you use it

Two ways in, and neither is the "advanced" one. **One:** press SEED and
steer what comes out — the same number always reproduces the same patch;
VARY moves parameters live, CHANGE, EVOLVE and CROSS take the patch
elsewhere. **Two:** press `n` to pull every cable at once and build the piece
connection by connection; `Ctrl+Z` steps back one action.

Hovering any knob, jack or module explains it in the LEARN box — in
Portuguese, English, French or Spanish. The [module guide](https://lucioaraujo.github.io/rasgo-modular/modulos-en.html)
has the same text for all 58 modules.

A recording comes out as a pair: the `.wav` of what you heard and a
`.score.txt` logging the topology and every gesture of the take. Patches are
saved as `.rmp`.

### Download

[**v0.1.1 release**](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.1) —
Linux `.deb`, Windows `.exe`, macOS `.dmg`, all built and packaged by
continuous integration.

| Platform | Built | Run | Audio verified |
|---|---|---|---|
| Linux x86-64 | yes | yes | yes, on real hardware |
| Windows x86-64 | continuous integration only | no | no |
| macOS (Universal 2) | continuous integration only | no | no |

Said in full before you download: the Windows and macOS packages have
never been opened by the author, and the `.dmg` is ad-hoc signed, without
Developer ID or notarisation — Gatekeeper will warn about an unidentified
developer on first launch.

### Build from source

Requirements, build steps, environment variables and troubleshooting are
in [`INSTALL.md`](INSTALL.md) (English and Portuguese). With JUCE available,
`./run` builds when needed and opens the app. What changed between versions
is in [`CHANGELOG.md`](CHANGELOG.md); development notes, in Portuguese, are
in [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md).

---

## Português

**Patches como ponto de partida.** O Rasgo Modular não abre em branco: ele
apresenta um patch já montado e soando, para você tocar a partir dele —
recabeando à mão, ajustando, deixando derivar, ou desmontando tudo e
construindo do zero.

**Autoria:** Lúcio Araújo · **Família:** [RASGO](https://rasgosound.arquiviagem.net/) ·
**Versão:** v0.1.1 · **Licença:** GNU AGPL-3.0-or-later (ver [`LICENSE`](LICENSE)
e [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md))

### O que ele é

Um ambiente modular generativo com **58 módulos** em oito famílias
(SOURCE, TRANSFORM, MODULATE, TIME, DECISION, ROUTE, SPACE, OUT), escrito em
C++17 sem dependências no núcleo. Os módulos são descritos por dados — cada
um declara seu painel em milímetros — e o mesmo motor alimenta dois
front-ends: um app multiplataforma em JUCE e um painel de teste em X11.

Ele não é plugin e não depende de uma DAW. Abre, soa, grava. MIDI e entrada
de áudio existem como módulos adaptadores opcionais, abertos só quando o
patch tem um deles.

### O cabo é um objeto, não um fio

- **Condutância** — o cabo pode conduzir de forma probabilística, passando
  às vezes, não sempre.
- **Relação** — o cabo pode combinar o que o atravessa com um segundo
  sinal: modulação em anel, dobra de onda ou diferença (RING, FOLD, DIFF no
  inspetor de cabo).
- **Ruptura com cicatriz** — romper um cabo não corta para o silêncio: a
  cicatriz segura o último bloco e o repete decaindo.

### Como se usa

Há dois caminhos, e nenhum é o "avançado". **Um:** aperte SEED e conduza o
que sair — o mesmo número reproduz sempre o mesmo patch; VARIA mexe nos
parâmetros ao vivo, MUDA, EVOLUI e CRUZA levam o patch a outro lugar.
**Dois:** aperte `n` para tirar todos os cabos de uma vez e construa a peça
ligação por ligação; `Ctrl+Z` volta uma ação.

Passar o mouse sobre qualquer knob, jack ou módulo explica-o na caixa
LEARN — em português, inglês, francês ou espanhol. O [guia dos módulos](https://lucioaraujo.github.io/rasgo-modular/modulos.html)
traz o mesmo texto para os 58 módulos.

A gravação sai em par: o `.wav` do que se ouviu e um `.score.txt` com a
topologia e cada gesto da tomada. Os patches são salvos em `.rmp`.

### Download

[**Release v0.1.1**](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.1) —
`.deb` para Linux, `.exe` para Windows, `.dmg` para macOS, todos construídos
e empacotados pela integração contínua.

| Plataforma | Construído | Executado | Áudio verificado |
|---|---|---|---|
| Linux x86-64 | sim | sim | sim, em hardware real |
| Windows x86-64 | só na integração contínua | não | não |
| macOS (Universal 2) | só na integração contínua | não | não |

Dito por inteiro antes de baixar: os pacotes de Windows e macOS nunca foram
abertos pelo autor, e o `.dmg` tem assinatura ad-hoc, sem Developer ID nem
notarização — o Gatekeeper mostra aviso de desenvolvedor não identificado
na primeira abertura.

### Compilar a partir do código

Requisitos, passos de build, variáveis de ambiente e problemas comuns estão
no [`INSTALL.md`](INSTALL.md) (português e inglês). Com o JUCE disponível,
`./run` compila quando precisa e abre o app. O que mudou entre versões está
no [`CHANGELOG.md`](CHANGELOG.md); as notas de desenvolvimento, no
[`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md).

---

## Français

**Des patches comme point de départ.** Le Rasgo Modular ne s'ouvre pas sur
une page blanche : il propose un patch déjà monté et sonnant, pour que vous
jouiez à partir de lui — en recâblant à la main, en ajustant, en le laissant
dériver, ou en démontant tout pour construire depuis zéro.

**Auteur :** Lúcio Araújo · **Famille :** [RASGO](https://rasgosound.arquiviagem.net/) ·
**Version :** v0.1.1 · **Licence :** GNU AGPL-3.0-or-later (voir [`LICENSE`](LICENSE)
et [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md))

### Ce que c'est

Un environnement modulaire génératif de **58 modules** en huit familles
(SOURCE, TRANSFORM, MODULATE, TIME, DECISION, ROUTE, SPACE, OUT), écrit en
C++17 sans dépendances dans le noyau. Les modules sont décrits comme des
données — chacun déclare son propre panneau en millimètres — et un seul
moteur alimente deux interfaces : une application JUCE multiplateforme et un
panneau de test X11.

Ce n'est pas un plugin et il ne dépend d'aucune STAN. Il s'ouvre, il sonne,
il enregistre. Le MIDI et l'entrée audio existent comme modules adaptateurs
optionnels, ouverts seulement quand un patch en contient un.

### Le câble est un objet, pas un fil

- **Conductance** — un câble peut conduire de façon probabiliste, passer
  parfois plutôt que toujours.
- **Relation** — le câble peut combiner ce qui le traverse avec un second
  signal : modulation en anneau, repliement d'onde ou différence (RING,
  FOLD, DIFF dans l'inspecteur de câble).
- **Rupture avec cicatrice** — rompre un câble ne coupe pas vers le
  silence : la cicatrice retient le dernier bloc et le répète en
  décroissant.

### Comment on s'en sert

Deux voies, et aucune n'est la voie « avancée ». **Une :** appuyez sur SEED
et dirigez ce qui sort — le même numéro reproduit toujours le même patch ;
VARIER déplace les paramètres en direct, CHANGER, ÉVOLUE et CROISER
emmènent le patch ailleurs. **Deux :** appuyez sur `n` pour retirer tous
les câbles d'un coup et construisez la pièce liaison par liaison ; `Ctrl+Z`
revient d'une action.

Survoler n'importe quel potentiomètre, jack ou module l'explique dans la
case LEARN — en portugais, anglais, français ou espagnol. Le [guide des modules](https://lucioaraujo.github.io/rasgo-modular/modulos-fr.html)
donne le même texte pour les 58 modules.

Un enregistrement sort par paire : le `.wav` de ce qu'on a entendu et un
`.score.txt` avec la topologie et chaque geste de la prise. Les patches sont
enregistrés en `.rmp`.

### Téléchargement

[**Version v0.1.1**](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.1) —
`.deb` pour Linux, `.exe` pour Windows, `.dmg` pour macOS, tous construits et
empaquetés par l'intégration continue.

| Plateforme | Construit | Lancé | Audio vérifié |
|---|---|---|---|
| Linux x86-64 | oui | oui | oui, sur matériel réel |
| Windows x86-64 | intégration continue uniquement | non | non |
| macOS (Universal 2) | intégration continue uniquement | non | non |

Dit en entier avant de télécharger : les paquets Windows et macOS n'ont
jamais été ouverts par l'auteur, et le `.dmg` est signé en ad-hoc, sans
Developer ID ni notarisation — Gatekeeper affichera un avertissement de
développeur non identifié au premier lancement.

### Compiler depuis les sources

Prérequis, étapes de compilation, variables d'environnement et problèmes
courants sont dans [`INSTALL.md`](INSTALL.md) (en anglais et en portugais).
Avec JUCE disponible, `./run` compile si nécessaire et ouvre l'application.
Les changements entre versions sont dans [`CHANGELOG.md`](CHANGELOG.md) ; les
notes de développement, en portugais, dans [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md).

---

## Español

**Patches como punto de partida.** El Rasgo Modular no abre en blanco:
ofrece un patch ya montado y sonando, para que usted toque a partir de él —
recableando a mano, ajustando, dejándolo derivar, o desmontándolo todo y
construyendo desde cero.

**Autoría:** Lúcio Araújo · **Familia:** [RASGO](https://rasgosound.arquiviagem.net/) ·
**Versión:** v0.1.1 · **Licencia:** GNU AGPL-3.0-or-later (ver [`LICENSE`](LICENSE)
y [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md))

### Qué es

Un entorno modular generativo con **58 módulos** en ocho familias (SOURCE,
TRANSFORM, MODULATE, TIME, DECISION, ROUTE, SPACE, OUT), escrito en C++17 sin
dependencias en el núcleo. Los módulos se describen como datos — cada uno
declara su propio panel en milímetros — y un mismo motor alimenta dos
interfaces: una aplicación JUCE multiplataforma y un panel de prueba X11.

No es un plugin y no depende de un DAW. Abre, suena, graba. El MIDI y la
entrada de audio existen como módulos adaptadores opcionales, abiertos solo
cuando el patch contiene uno.

### El cable es un objeto, no un hilo

- **Conductancia** — un cable puede conducir de forma probabilística,
  pasando a veces y no siempre.
- **Relación** — el cable puede combinar lo que lo atraviesa con una
  segunda señal: modulación en anillo, plegado de onda o diferencia (RING,
  FOLD, DIFF en el inspector de cable).
- **Ruptura con cicatriz** — romper un cable no corta al silencio: la
  cicatriz retiene el último bloque y lo repite decayendo.

### Cómo se usa

Dos caminos, y ninguno es el "avanzado". **Uno:** pulse SEED y conduzca lo
que salga — el mismo número reproduce siempre el mismo patch; VARIAR mueve
los parámetros en vivo, CAMBIA, EVOLUCIONA y CRUZAR llevan el patch a otro
lugar. **Dos:** pulse `n` para quitar todos los cables de una vez y
construya la pieza conexión a conexión; `Ctrl+Z` deshace una acción.

Pasar el ratón sobre cualquier perilla, jack o módulo lo explica en la caja
LEARN — en portugués, inglés, francés o español. La [guía de módulos](https://lucioaraujo.github.io/rasgo-modular/modulos-es.html)
trae el mismo texto para los 58 módulos.

Una grabación sale en par: el `.wav` de lo que se escuchó y un `.score.txt`
con la topología y cada gesto de la toma. Los patches se guardan en `.rmp`.

### Descarga

[**Versión v0.1.1**](https://github.com/lucioaraujo/rasgo-modular/releases/tag/v0.1.1) —
`.deb` para Linux, `.exe` para Windows, `.dmg` para macOS, todos construidos
y empaquetados por la integración continua.

| Plataforma | Construido | Ejecutado | Audio verificado |
|---|---|---|---|
| Linux x86-64 | sí | sí | sí, en hardware real |
| Windows x86-64 | solo en integración continua | no | no |
| macOS (Universal 2) | solo en integración continua | no | no |

Dicho por completo antes de descargar: los paquetes de Windows y macOS
nunca fueron abiertos por el autor, y el `.dmg` tiene firma ad-hoc, sin
Developer ID ni notarización — Gatekeeper mostrará un aviso de desarrollador
no identificado en la primera apertura.

### Compilar desde el código

Requisitos, pasos de compilación, variables de entorno y problemas comunes
están en [`INSTALL.md`](INSTALL.md) (en inglés y portugués). Con JUCE
disponible, `./run` compila cuando hace falta y abre la aplicación. Los
cambios entre versiones están en [`CHANGELOG.md`](CHANGELOG.md); las notas
de desarrollo, en portugués, en [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md).
