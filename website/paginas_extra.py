#!/usr/bin/env python3
"""Tutorial e Instalação do site do Rasgo Modular (4 línguas) e o menu de
páginas em todas as páginas do site.

    python3 gerar_modulos.py && python3 paginas_extra.py && python3 seo.py

Nivelamento dos sites da família RASGO (autor, 19/11/2026: "nivele pelos
mais completos"; métodos em RASGO_DOCUMENTATION/design/WEBSITES.md §8):

- Tutorial: os cartões do TUTORIAL do app (apps/panel/UiLanguage.hpp,
  chaves tut*Title/tut*Body, na ordem de RasgoModularApp.cpp), lidos do
  código para o site e a janela dizerem o mesmo;
- Instalação: os passos do INSTALL.md (que tem português e inglês), aqui
  nas 4 línguas, com os avisos do Windows e do macOS;
- Módulos (gerar_modulos.py) faz o papel de Aprender;
- menu Conceito · Tutorial · Módulos · Instalação · Baixar em todas as
  páginas; Baixar leva à seção dos pacotes da página inicial (id="baixar").

A casca de cada página nova (faixa RASGO, cabeçalho, rodapé, contato) vem
da página Módulos da mesma língua; o bloco SEO é escrito depois pelo seo.py.
"""

import html
import pathlib
import re

AQUI = pathlib.Path(__file__).parent
FONTE = AQUI.parent / "apps" / "panel" / "UiLanguage.hpp"
LANGS = ("pt", "en", "fr", "es")
ORDEM = {"en": 0, "pt": 1, "fr": 2, "es": 3}   # struct L4: en, pt, fr, es
SUFIXO = {"pt": "", "en": "-en", "fr": "-fr", "es": "-es"}
CARTOES = ("What", "Seed", "SeedBox", "Vary", "Store", "Rec", "Hdr", "Cable", "Nav", "Keys",
           "Mod", "Scratch", "Fam", "Learn")


def arquivo(tipo, lang):
    if tipo == "conceito":
        return "index.html" if lang == "pt" else lang + ".html"
    return tipo + SUFIXO[lang] + ".html"


MENU = {
    "pt": ("Seções", [("conceito", "Conceito"), ("tutorial", "Tutorial"), ("modulos", "Módulos"), ("instalacao", "Instalação"), ("baixar", "Baixar")]),
    "en": ("Sections", [("conceito", "Concept"), ("tutorial", "Tutorial"), ("modulos", "Modules"), ("instalacao", "Installation"), ("baixar", "Download")]),
    "fr": ("Sections", [("conceito", "Concept"), ("tutorial", "Tutoriel"), ("modulos", "Modules"), ("instalacao", "Installation"), ("baixar", "Télécharger")]),
    "es": ("Secciones", [("conceito", "Concepto"), ("tutorial", "Tutorial"), ("modulos", "Módulos"), ("instalacao", "Instalación"), ("baixar", "Descargar")]),
}

TUT = {
    "pt": ("Tutorial — Rasgo Modular", "Tutorial",
           "Os cartões do tutorial que o Rasgo Modular traz no botão TUTORIAL: o que ele é, a seed, variar, guardar, gravar, os cabos, a navegação, o teclado e os módulos.",
           "Tutorial do Rasgo Modular: a seed, variar, guardar e gravar um patch, os cabos, a navegação pelo rack, os atalhos e os módulos.", "Sumário"),
    "en": ("Tutorial — Rasgo Modular", "Tutorial",
           "The tutorial cards Rasgo Modular carries under its TUTORIAL button: what it is, the seed, varying, storing, recording, the cables, navigation, the keyboard and the modules.",
           "Rasgo Modular tutorial: the seed, varying, storing and recording a patch, the cables, rack navigation, shortcuts and the modules.", "Contents"),
    "fr": ("Tutoriel — Rasgo Modular", "Tutoriel",
           "Les cartes du tutoriel que Rasgo Modular porte sous son bouton TUTORIAL : ce qu'il est, la seed, varier, garder, enregistrer, les câbles, la navigation, le clavier et les modules.",
           "Tutoriel de Rasgo Modular : la seed, varier, garder et enregistrer un patch, les câbles, la navigation dans le rack, les raccourcis et les modules.", "Sommaire"),
    "es": ("Tutorial — Rasgo Modular", "Tutorial",
           "Las tarjetas del tutorial que Rasgo Modular lleva en su botón TUTORIAL: qué es, la seed, variar, guardar, grabar, los cables, la navegación, el teclado y los módulos.",
           "Tutorial de Rasgo Modular: la seed, variar, guardar y grabar un patch, los cables, la navegación por el rack, los atajos y los módulos.", "Sumario"),
}

# Marcação: **negrito** e `código`. Blocos: ("p", t), ("ol"|"ul", [..]), ("tab", [(a, b)..]), ("code", t)
INST = {
    "pt": {
        "title": "Instalação — Rasgo Modular", "h1": "Instalação",
        "lead": "Como instalar o Rasgo Modular no Windows, no macOS e no Linux, o que ele pede do computador e onde guarda as coisas.",
        "desc": "Instalar o Rasgo Modular no Windows, macOS e Linux, passo a passo, com os avisos de segurança, os requisitos e onde ficam os dados.",
        "secoes": [
            ("Antes de começar", [("p", "Os pacotes estão em **Baixar**, na página inicial, e na página de releases do GitHub. Os avisos de segurança do Windows e do macOS são esperados e aparecem só na primeira vez.")]),
            ("Windows", [
                ("ol", ["Baixe `rasgo-modular-<versão>-windows-x64.exe` e abra.",
                        "O Windows mostra **\"O Windows protegeu o computador\"**: clique em **\"Mais informações\"** e depois em **\"Executar assim mesmo\"**.",
                        "Se pedir permissão de administrador, clique em **\"Sim\"**.",
                        "Clique em **\"Avançar\"** até o fim. O Rasgo Modular fica no Menu Iniciar e na área de trabalho; cada versão nova substitui a anterior."]),
                ("p", "**Sem instalar:** descompacte o `.zip` e abra `Rasgo Modular.exe`. Se ele não abrir, aperte Win + R, digite `%APPDATA%\\rasgo-modular` e envie o `arranque.log` numa issue do GitHub: a última linha diz onde a abertura parou."),
            ]),
            ("macOS", [
                ("ol", ["Baixe `rasgo-modular-<versão>-macos-universal.dmg`. Ele serve para Mac Intel e Apple Silicon.",
                        "Abra o `.dmg` e arraste o Rasgo Modular para **Aplicativos**.",
                        "Abra o app. Na primeira vez, o macOS o bloqueia, porque ele tem assinatura ad hoc, sem o Developer ID pago da Apple. Clique em **\"OK\"**.",
                        "Libere o app uma vez: em **Ajustes do Sistema → Privacidade e Segurança**, clique em **\"Abrir Mesmo Assim\"** e confirme. No macOS 14 ou anterior: botão direito no app, no Finder → **Abrir → Abrir**.",
                        "Ao ligar o botão **ON** do módulo SIGNAL-IN, o macOS pede o microfone: permita, se quiser tocar com entrada de áudio."]),
                ("p", "Se o botão não aparecer, use o Terminal. O comando tira a marca de quarentena que o macOS põe em todo arquivo baixado:"),
                ("code", 'xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"'),
            ]),
            ("Linux", [("ul", ["**Ubuntu, Debian e Mint:** dois cliques no `.deb`, ou `sudo apt install ./rasgo-modular-<versão>-linux-x86_64.deb`.",
                               "**Outras distribuições, AppImage:** torne-o executável (Propriedades → permitir executar, ou `chmod +x`) e abra.",
                               "**Outras distribuições, `.tar.gz`:** descompacte e rode `./install.sh`, que instala para o seu usuário, sem root."])]),
            ("Requisitos", [("tab", [("**Windows**", "Windows 10 (1607 ou posterior) ou 11, 64 bits"),
                                     ("**macOS**", "10.15 ou posterior, Intel e Apple Silicon"),
                                     ("**Linux**", "x86-64 com glibc 2.35 ou mais nova; testado na abertura em Debian 12, Ubuntu 24.04, Fedora e Arch"),
                                     ("**Processador**", "64 bits, dois núcleos ou mais; medido num Intel Core i5-6500: o app usa de um terço a metade de um núcleo"),
                                     ("**Memória e disco**", "cerca de 55 MB em uso e 20 MB no disco"),
                                     ("**Tela**", "1280 × 760 ou maior"),
                                     ("**Áudio**", "qualquer saída do sistema; microfone e teclado MIDI são opcionais (módulo SIGNAL-IN)")])]),
            ('Com outros programas', [('p', 'O Rasgo Modular é um aplicativo próprio e conversa com DAWs, outros sintetizadores e máquinas pelo áudio e pelo MIDI do sistema. A porta de entrada é o módulo SIGNAL-IN: ligue o **ON** e cabeie uma saída dele. A partir daí ele recebe o som, as notas e o andamento; o NOTE-OUT manda as notas para fora.'), ('ul', ['**Som de outro programa no Rasgo:** no Linux, ligue a saída do programa à entrada do Rasgo Modular no PipeWire ou no JACK (qpwgraph, Helvum ou Carla); no macOS, use um dispositivo virtual como o BlackHole; no Windows, o VB-Cable. O som chega nas saídas L e R do SIGNAL-IN.', '**Som do Rasgo numa DAW:** o mesmo caminho ao contrário. A saída do Rasgo Modular vira a entrada de uma faixa da DAW (Ardour, Reaper, Bitwig, Live, Infinite e outras) para gravar ou processar.', '**Notas:** um teclado MIDI ou uma faixa MIDI da DAW chega no SIGNAL-IN (1V/O, GATE, VEL e CC). O NOTE-OUT manda as notas do patch para outro instrumento.', '**O andamento da DAW (MIDI clock):** na DAW, ligue o envio de MIDI clock para a porta que o Rasgo Modular escuta. Cabeie o **CLK** do SIGNAL-IN no **EXT** do CLOCK e o **RST** no **RST** do CLOCK: o patch segue o andamento e o "um" da DAW e para no STOP. O **DIV** escolhe a divisão; a colcheia, o padrão, casa com o MULT 2 do CLOCK. O **RUN** fica alto enquanto a DAW toca.', '**Portas MIDI entre programas:** no Linux, as portas aparecem no próprio sistema (ALSA); no macOS, use o driver IAC; no Windows, um cabo MIDI virtual como o loopMIDI. O Rasgo Modular escuta as entradas MIDI que existem quando o SIGNAL-IN é ligado: abra a DAW antes.'])]),
            ("Onde ficam as coisas", [("ul", ["**Estado e preferências:** `~/.config/rasgo-modular/` no Linux (sessão, patches, idioma); no macOS e no Windows, a pasta de dados de aplicativo do sistema.",
                                              "**Gravações:** `~/Music/RasgoModular/`, o `.wav` e a partitura da tomada, com a versão de entrega para streaming ao lado."])]),
            ("Se não sair som", [("p", "O Rasgo Modular soa ao abrir. Sem som, confira a saída de áudio do sistema, o botão ESPERA (que cala o master) e o `gain` do módulo MASTER. Para outra coisa, escreva para **rasgo.instruments@gmail.com** com o sistema, a versão e o que apareceu.")]),
        ],
    },
    "en": {
        "title": "Installation — Rasgo Modular", "h1": "Installation",
        "lead": "How to install Rasgo Modular on Windows, macOS and Linux, what it needs from the computer and where it keeps things.",
        "desc": "Install Rasgo Modular on Windows, macOS and Linux, step by step, with the security warnings, the requirements and where the data lives.",
        "secoes": [
            ("Before you start", [("p", "The packages are under **Download**, on the home page, and on the GitHub releases page. The Windows and macOS security warnings are expected and appear only the first time.")]),
            ("Windows", [
                ("ol", ["Download `rasgo-modular-<version>-windows-x64.exe` and open it.",
                        "Windows shows **\"Windows protected your PC\"**: click **\"More info\"**, then **\"Run anyway\"**.",
                        "If Windows asks for administrator permission, click **\"Yes\"**.",
                        "Click **\"Next\"** to the end. Rasgo Modular is then in the Start menu and on the desktop; each new version replaces the previous one."]),
                ("p", "**Without installing:** unzip the `.zip` and open `Rasgo Modular.exe`. If it does not open, press Win + R, type `%APPDATA%\\rasgo-modular` and send `arranque.log` in a GitHub issue: its last line says where startup stopped."),
            ]),
            ("macOS", [
                ("ol", ["Download `rasgo-modular-<version>-macos-universal.dmg`. It works on Intel and Apple Silicon Macs.",
                        "Open the `.dmg` and drag Rasgo Modular into **Applications**.",
                        "Open the app. The first time, macOS blocks it, because it is signed ad hoc, without a paid Apple Developer ID. Click **\"OK\"**.",
                        "Allow the app once: in **System Settings → Privacy & Security**, click **\"Open Anyway\"** and confirm. On macOS 14 or earlier: right-click the app in Finder → **Open → Open**.",
                        "When you turn on the **ON** button of the SIGNAL-IN module, macOS asks for the microphone: allow it if you want to play with audio input."]),
                ("p", "If the button does not appear, use Terminal. The command removes the quarantine mark macOS puts on every downloaded file:"),
                ("code", 'xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"'),
            ]),
            ("Linux", [("ul", ["**Ubuntu, Debian and Mint:** double-click the `.deb`, or `sudo apt install ./rasgo-modular-<version>-linux-x86_64.deb`.",
                               "**Other distributions, AppImage:** make it executable (Properties → allow executing, or `chmod +x`) and open it.",
                               "**Other distributions, `.tar.gz`:** unpack it and run `./install.sh`, which installs for your user, without root."])]),
            ("Requirements", [("tab", [("**Windows**", "Windows 10 (1607 or later) or 11, 64-bit"),
                                       ("**macOS**", "10.15 or later, Intel and Apple Silicon"),
                                       ("**Linux**", "x86-64 with glibc 2.35 or newer; launch-tested on Debian 12, Ubuntu 24.04, Fedora and Arch"),
                                       ("**Processor**", "64-bit, two cores or more; measured on an Intel Core i5-6500: the app uses a third to half of one core"),
                                       ("**Memory and disk**", "about 55 MB in use and 20 MB on disk"),
                                       ("**Screen**", "1280 × 760 or larger"),
                                       ("**Audio**", "any system output; microphone and MIDI keyboard are optional (SIGNAL-IN module)")])]),
            ('With other programs', [('p', "Rasgo Modular is a standalone application that talks to DAWs, other synthesizers and machines through the system's audio and MIDI. The way in is the SIGNAL-IN module: switch it **ON** and patch one of its outputs. From then on it receives sound, notes and tempo; NOTE-OUT sends notes out."), ('ul', ["**Sound from another program into Rasgo:** on Linux, connect the program's output to Rasgo Modular's input in PipeWire or JACK (qpwgraph, Helvum or Carla); on macOS, use a virtual device such as BlackHole; on Windows, VB-Cable. The sound arrives at SIGNAL-IN's L and R outputs.", "**Rasgo's sound in a DAW:** the same path the other way round. Rasgo Modular's output becomes the input of a DAW track (Ardour, Reaper, Bitwig, Live, Infinite and others) to record or process.", "**Notes:** a MIDI keyboard or a DAW MIDI track arrives at SIGNAL-IN (1V/O, GATE, VEL and CC). NOTE-OUT sends the patch's notes to another instrument.", '**The DAW\'s tempo (MIDI clock):** in the DAW, turn on MIDI clock output to the port Rasgo Modular listens to. Patch SIGNAL-IN\'s **CLK** into the CLOCK\'s **EXT** and **RST** into the CLOCK\'s **RST**: the patch follows the DAW\'s tempo and "one" and stops on STOP. **DIV** picks the division; the eighth note, the default, matches the CLOCK\'s MULT 2. **RUN** stays high while the DAW plays.', '**MIDI ports between programs:** on Linux, the ports appear in the system itself (ALSA); on macOS, use the IAC driver; on Windows, a virtual MIDI cable such as loopMIDI. Rasgo Modular listens to the MIDI inputs that exist when SIGNAL-IN is switched on: open the DAW first.'])]),
            ("Where things live", [("ul", ["**State and preferences:** `~/.config/rasgo-modular/` on Linux (session, patches, language); on macOS and Windows, the system's application data folder.",
                                           "**Recordings:** `~/Music/RasgoModular/`, the `.wav` and the take's score, with the streaming delivery version beside them."])]),
            ("If there is no sound", [("p", "Rasgo Modular sounds when it opens. With no sound, check the system audio output, the STANDBY button (which mutes the master) and the `gain` of the MASTER module. For anything else, write to **rasgo.instruments@gmail.com** with your system, the version and what you saw.")]),
        ],
    },
    "fr": {
        "title": "Installation — Rasgo Modular", "h1": "Installation",
        "lead": "Comment installer Rasgo Modular sous Windows, macOS et Linux, ce qu'il demande à l'ordinateur et où il range les choses.",
        "desc": "Installer Rasgo Modular sous Windows, macOS et Linux, pas à pas, avec les avertissements de sécurité, la configuration requise et l'emplacement des données.",
        "secoes": [
            ("Avant de commencer", [("p", "Les paquets sont dans **Télécharger**, sur la page d'accueil, et sur la page des releases GitHub. Les avertissements de sécurité de Windows et de macOS sont attendus et n'apparaissent que la première fois.")]),
            ("Windows", [
                ("ol", ["Téléchargez `rasgo-modular-<version>-windows-x64.exe` et ouvrez-le.",
                        "Windows affiche **« Windows a protégé votre ordinateur »** : cliquez sur **« Informations complémentaires »**, puis sur **« Exécuter quand même »**.",
                        "Si Windows demande l'autorisation d'administrateur, cliquez sur **« Oui »**.",
                        "Cliquez sur **« Suivant »** jusqu'à la fin. Rasgo Modular est alors dans le menu Démarrer et sur le bureau ; chaque nouvelle version remplace la précédente."]),
                ("p", "**Sans installer :** décompressez le `.zip` et ouvrez `Rasgo Modular.exe`. S'il ne s'ouvre pas, appuyez sur Win + R, tapez `%APPDATA%\\rasgo-modular` et envoyez `arranque.log` dans une issue GitHub : sa dernière ligne dit où le démarrage s'est arrêté."),
            ]),
            ("macOS", [
                ("ol", ["Téléchargez `rasgo-modular-<version>-macos-universal.dmg`. Il fonctionne sur les Mac Intel et Apple Silicon.",
                        "Ouvrez le `.dmg` et glissez Rasgo Modular dans **Applications**.",
                        "Ouvrez l'app. La première fois, macOS la bloque, car elle a une signature ad hoc, sans Developer ID Apple payant. Cliquez sur **« OK »**.",
                        "Autorisez l'app une fois : dans **Réglages Système → Confidentialité et sécurité**, cliquez sur **« Ouvrir quand même »** et confirmez. Sous macOS 14 ou antérieur : clic droit sur l'app dans le Finder → **Ouvrir → Ouvrir**.",
                        "Quand vous allumez le bouton **ON** du module SIGNAL-IN, macOS demande le microphone : autorisez-le pour jouer avec une entrée audio."]),
                ("p", "Si le bouton n'apparaît pas, utilisez le Terminal. La commande retire la marque de quarantaine que macOS pose sur chaque fichier téléchargé :"),
                ("code", 'xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"'),
            ]),
            ("Linux", [("ul", ["**Ubuntu, Debian et Mint :** double-cliquez sur le `.deb`, ou `sudo apt install ./rasgo-modular-<version>-linux-x86_64.deb`.",
                               "**Autres distributions, AppImage :** rendez-le exécutable (Propriétés → autoriser l'exécution, ou `chmod +x`) et ouvrez-le.",
                               "**Autres distributions, `.tar.gz` :** décompressez et lancez `./install.sh`, qui installe pour votre utilisateur, sans root."])]),
            ("Configuration requise", [("tab", [("**Windows**", "Windows 10 (1607 ou ultérieur) ou 11, 64 bits"),
                                                ("**macOS**", "10.15 ou ultérieur, Intel et Apple Silicon"),
                                                ("**Linux**", "x86-64 avec glibc 2.35 ou plus récente ; testé au lancement sous Debian 12, Ubuntu 24.04, Fedora et Arch"),
                                                ("**Processeur**", "64 bits, deux cœurs ou plus ; mesuré sur un Intel Core i5-6500 : l'app utilise d'un tiers à la moitié d'un cœur"),
                                                ("**Mémoire et disque**", "environ 55 Mo en usage et 20 Mo sur le disque"),
                                                ("**Écran**", "1280 × 760 ou plus grand"),
                                                ("**Audio**", "toute sortie du système ; microphone et clavier MIDI sont facultatifs (module SIGNAL-IN)")])]),
            ('Avec d’autres logiciels', [('p', 'Rasgo Modular est une application autonome qui dialogue avec les DAW, d’autres synthétiseurs et des machines par l’audio et le MIDI du système. La porte d’entrée est le module SIGNAL-IN : allumez **ON** et câblez une de ses sorties. Il reçoit alors le son, les notes et le tempo ; NOTE-OUT envoie les notes vers l’extérieur.'), ('ul', ['**Le son d’un autre logiciel dans Rasgo :** sous Linux, reliez la sortie du logiciel à l’entrée de Rasgo Modular dans PipeWire ou JACK (qpwgraph, Helvum ou Carla) ; sous macOS, utilisez un périphérique virtuel comme BlackHole ; sous Windows, VB-Cable. Le son arrive sur les sorties L et R de SIGNAL-IN.', '**Le son de Rasgo dans une DAW :** le même chemin en sens inverse. La sortie de Rasgo Modular devient l’entrée d’une piste de la DAW (Ardour, Reaper, Bitwig, Live, Infinite et d’autres) pour enregistrer ou traiter.', '**Les notes :** un clavier MIDI ou une piste MIDI de la DAW arrive sur SIGNAL-IN (1V/O, GATE, VEL et CC). NOTE-OUT envoie les notes du patch vers un autre instrument.', '**Le tempo de la DAW (horloge MIDI) :** dans la DAW, activez l’envoi de l’horloge MIDI vers le port qu’écoute Rasgo Modular. Câblez le **CLK** de SIGNAL-IN sur l’**EXT** du CLOCK et le **RST** sur le **RST** du CLOCK : le patch suit le tempo et le « un » de la DAW et s’arrête au STOP. **DIV** choisit la division ; la croche, par défaut, s’accorde au MULT 2 du CLOCK. **RUN** reste haut tant que la DAW joue.', '**Ports MIDI entre logiciels :** sous Linux, les ports apparaissent dans le système (ALSA) ; sous macOS, utilisez le pilote IAC ; sous Windows, un câble MIDI virtuel comme loopMIDI. Rasgo Modular écoute les entrées MIDI présentes quand SIGNAL-IN est allumé : ouvrez d’abord la DAW.'])]),
            ("Où vivent les choses", [("ul", ["**État et préférences :** `~/.config/rasgo-modular/` sous Linux (session, patches, langue) ; sous macOS et Windows, le dossier de données d'application du système.",
                                              "**Enregistrements :** `~/Music/RasgoModular/`, le `.wav` et la partition de la prise, avec la version de livraison pour le streaming à côté."])]),
            ("S'il n'y a pas de son", [("p", "Rasgo Modular sonne dès l'ouverture. Sans son, vérifiez la sortie audio du système, le bouton ATTENTE (qui coupe le master) et le `gain` du module MASTER. Pour autre chose, écrivez à **rasgo.instruments@gmail.com** avec votre système, la version et ce qui s'est affiché.")]),
        ],
    },
    "es": {
        "title": "Instalación — Rasgo Modular", "h1": "Instalación",
        "lead": "Cómo instalar Rasgo Modular en Windows, macOS y Linux, qué pide del ordenador y dónde guarda las cosas.",
        "desc": "Instalar Rasgo Modular en Windows, macOS y Linux, paso a paso, con los avisos de seguridad, los requisitos y dónde quedan los datos.",
        "secoes": [
            ("Antes de empezar", [("p", "Los paquetes están en **Descargar**, en la página de inicio, y en la página de releases de GitHub. Los avisos de seguridad de Windows y macOS son esperables y aparecen solo la primera vez.")]),
            ("Windows", [
                ("ol", ["Descarga `rasgo-modular-<versión>-windows-x64.exe` y ábrelo.",
                        "Windows muestra **\"Windows protegió su PC\"**: haz clic en **\"Más información\"** y luego en **\"Ejecutar de todas formas\"**.",
                        "Si Windows pide permiso de administrador, haz clic en **\"Sí\"**.",
                        "Haz clic en **\"Siguiente\"** hasta el final. Rasgo Modular queda en el menú Inicio y en el escritorio; cada versión nueva reemplaza a la anterior."]),
                ("p", "**Sin instalar:** descomprime el `.zip` y abre `Rasgo Modular.exe`. Si no abre, pulsa Win + R, escribe `%APPDATA%\\rasgo-modular` y envía el `arranque.log` en una issue de GitHub: su última línea dice dónde se detuvo el arranque."),
            ]),
            ("macOS", [
                ("ol", ["Descarga `rasgo-modular-<versión>-macos-universal.dmg`. Sirve para Mac Intel y Apple Silicon.",
                        "Abre el `.dmg` y arrastra Rasgo Modular a **Aplicaciones**.",
                        "Abre la app. La primera vez, macOS la bloquea, porque tiene firma ad hoc, sin el Developer ID de pago de Apple. Haz clic en **\"OK\"**.",
                        "Autoriza la app una vez: en **Ajustes del Sistema → Privacidad y seguridad**, haz clic en **\"Abrir igualmente\"** y confirma. En macOS 14 o anterior: clic derecho en la app en el Finder → **Abrir → Abrir**.",
                        "Al encender el botón **ON** del módulo SIGNAL-IN, macOS pide el micrófono: autorízalo si quieres tocar con entrada de audio."]),
                ("p", "Si el botón no aparece, usa la Terminal. El comando quita la marca de cuarentena que macOS pone a todo archivo descargado:"),
                ("code", 'xattr -dr com.apple.quarantine "/Applications/Rasgo Modular.app"'),
            ]),
            ("Linux", [("ul", ["**Ubuntu, Debian y Mint:** doble clic en el `.deb`, o `sudo apt install ./rasgo-modular-<versión>-linux-x86_64.deb`.",
                               "**Otras distribuciones, AppImage:** hazlo ejecutable (Propiedades → permitir ejecutar, o `chmod +x`) y ábrelo.",
                               "**Otras distribuciones, `.tar.gz`:** descomprímelo y ejecuta `./install.sh`, que instala para tu usuario, sin root."])]),
            ("Requisitos", [("tab", [("**Windows**", "Windows 10 (1607 o posterior) u 11, 64 bits"),
                                     ("**macOS**", "10.15 o posterior, Intel y Apple Silicon"),
                                     ("**Linux**", "x86-64 con glibc 2.35 o más reciente; probado al abrir en Debian 12, Ubuntu 24.04, Fedora y Arch"),
                                     ("**Procesador**", "64 bits, dos núcleos o más; medido en un Intel Core i5-6500: la app usa de un tercio a la mitad de un núcleo"),
                                     ("**Memoria y disco**", "unos 55 MB en uso y 20 MB en el disco"),
                                     ("**Pantalla**", "1280 × 760 o mayor"),
                                     ("**Audio**", "cualquier salida del sistema; micrófono y teclado MIDI son opcionales (módulo SIGNAL-IN)")])]),
            ('Con otros programas', [('p', 'Rasgo Modular es una aplicación propia que conversa con DAW, otros sintetizadores y máquinas por el audio y el MIDI del sistema. La puerta de entrada es el módulo SIGNAL-IN: encienda **ON** y conecte una de sus salidas. Desde ahí recibe el sonido, las notas y el tempo; NOTE-OUT manda las notas hacia fuera.'), ('ul', ['**Sonido de otro programa en Rasgo:** en Linux, conecte la salida del programa a la entrada de Rasgo Modular en PipeWire o JACK (qpwgraph, Helvum o Carla); en macOS, use un dispositivo virtual como BlackHole; en Windows, VB-Cable. El sonido llega a las salidas L y R de SIGNAL-IN.', '**El sonido de Rasgo en una DAW:** el mismo camino al revés. La salida de Rasgo Modular se vuelve la entrada de una pista de la DAW (Ardour, Reaper, Bitwig, Live, Infinite y otras) para grabar o procesar.', '**Notas:** un teclado MIDI o una pista MIDI de la DAW llega a SIGNAL-IN (1V/O, GATE, VEL y CC). NOTE-OUT manda las notas del patch a otro instrumento.', '**El tempo de la DAW (reloj MIDI):** en la DAW, active el envío de reloj MIDI al puerto que escucha Rasgo Modular. Conecte el **CLK** de SIGNAL-IN al **EXT** del CLOCK y el **RST** al **RST** del CLOCK: el patch sigue el tempo y el «uno» de la DAW y se detiene con STOP. **DIV** elige la división; la corchea, por defecto, casa con el MULT 2 del CLOCK. **RUN** queda alto mientras la DAW toca.', '**Puertos MIDI entre programas:** en Linux, los puertos aparecen en el propio sistema (ALSA); en macOS, use el driver IAC; en Windows, un cable MIDI virtual como loopMIDI. Rasgo Modular escucha las entradas MIDI que existen cuando SIGNAL-IN se enciende: abra la DAW antes.'])]),
            ("Dónde quedan las cosas", [("ul", ["**Estado y preferencias:** `~/.config/rasgo-modular/` en Linux (sesión, patches, idioma); en macOS y Windows, la carpeta de datos de aplicación del sistema.",
                                                "**Grabaciones:** `~/Music/RasgoModular/`, el `.wav` y la partitura de la toma, con la versión de entrega para streaming al lado."])]),
            ("Si no sale sonido", [("p", "Rasgo Modular suena al abrir. Sin sonido, revisa la salida de audio del sistema, el botón ESPERA (que silencia el máster) y el `gain` del módulo MASTER. Para otra cosa, escribe a **rasgo.instruments@gmail.com** con tu sistema, la versión y lo que apareció.")]),
        ],
    },
}


def ler_l4(nome):
    fonte = FONTE.read_text(encoding="utf-8")
    m = re.search(r"inline const L4 %s\s*\{(.*?)\};" % re.escape(nome), fonte, re.S)
    if not m:
        raise ValueError("UiLanguage.hpp: falta " + nome)
    partes, atual, anterior = [], None, False
    for literal, virgula in re.findall(r'"((?:[^"\\]|\\.)*)"|(,)', m.group(1)):
        if virgula:
            anterior = False
            continue
        texto = bytes(literal, "utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8")
        if anterior:
            partes[-1] += texto
        else:
            partes.append(texto)
        anterior = True
    if len(partes) != 4:
        raise ValueError("%s: esperava 4 línguas, achei %d" % (nome, len(partes)))
    return partes


def inline(texto):
    e = html.escape(texto, quote=False)
    e = re.sub(r"\*\*(.+?)\*\*", r"<strong>\1</strong>", e)
    return re.sub(r"`(.+?)`", r"<code>\1</code>", e)


def paragrafos(texto):
    out = []
    for par in [p for p in texto.split("\n\n") if p.strip()]:
        linhas = [l for l in par.split("\n") if l.strip()]
        if all(l.lstrip().startswith(("• ", "- ")) for l in linhas):
            out.append('  <ul class="tut-list">\n%s\n  </ul>' % "\n".join(
                "    <li>%s</li>" % html.escape(l.lstrip()[2:], quote=False) for l in linhas))
        else:
            out.append("  <p>%s</p>" % "<br />".join(html.escape(l, quote=False) for l in linhas))
    return "\n".join(out)


def blocos(lista):
    out = []
    for tipo, c in lista:
        if tipo == "p":
            out.append("  <p>%s</p>" % inline(c))
        elif tipo in ("ul", "ol"):
            out.append('  <%s class="tut-list">\n%s\n  </%s>' % (tipo, "\n".join("    <li>%s</li>" % inline(i) for i in c), tipo))
        elif tipo == "tab":
            out.append('  <table class="ref">\n%s\n  </table>' % "\n".join(
                "    <tr><th scope=\"row\">%s</th><td>%s</td></tr>" % (inline(a), inline(b)) for a, b in c))
        elif tipo == "code":
            out.append('  <pre class="tut-code"><code>%s</code></pre>' % html.escape(c, quote=False))
    return "\n".join(out)


NAV_RE = re.compile(r'<nav class="page-nav".*?</nav>', re.S)


def menu(lang, atual):
    rotulo, itens = MENU[lang]
    links = []
    for tipo, nome in itens:
        href = arquivo("conceito", lang) + "#baixar" if tipo == "baixar" else arquivo(tipo, lang)
        cur = ' aria-current="page"' if tipo == atual else ""
        links.append('      <a href="%s"%s>%s</a>' % (href, cur, nome))
    return '<nav class="page-nav" aria-label="%s">\n%s\n    </nav>' % (rotulo, "\n".join(links))


def linguas(lang, atual):
    rotulo = {"pt": "Idioma", "en": "Language", "fr": "Langue", "es": "Idioma"}[lang]
    links = "\n".join('      <a href="%s"%s>%s</a>' % (arquivo(atual, L), ' aria-current="true"' if L == lang else "", L.upper())
                      for L in LANGS)
    return '<nav class="lang-switch" aria-label="%s">\n%s\n    </nav>' % (rotulo, links)


def pagina(lang, tipo, titulo, descricao, h1, lead, corpo):
    casca = (AQUI / arquivo("modulos", lang)).read_text(encoding="utf-8")
    cabeca, resto = casca.split('<main id="conteudo">', 1)
    rodape = resto.split("</main>", 1)[1]
    cabeca = re.sub(r"<title>.*?</title>", lambda _: "<title>%s</title>" % html.escape(titulo, quote=False), cabeca, count=1)
    cabeca = re.sub(r'<meta name="description" content="[^"]*"', lambda _: '<meta name="description" content="%s"' % html.escape(descricao, quote=True), cabeca, count=1)
    cabeca = NAV_RE.sub(lambda _: menu(lang, tipo), cabeca, count=1)
    cabeca = re.sub(r'<nav class="lang-switch".*?</nav>', lambda _: linguas(lang, tipo), cabeca, count=1, flags=re.S)
    hero = '<section class="hero" id="top">\n  <h1>%s</h1>\n  <p class="lead">%s</p>\n</section>' % (
        html.escape(h1, quote=False), html.escape(lead, quote=False))
    return cabeca + '<main id="conteudo">\n' + hero + "\n\n" + corpo + "\n</main>" + rodape


def main():
    cartoes = [(ler_l4("tut%sTitle" % c), ler_l4("tut%sBody" % c)) for c in CARTOES]
    for lang in LANGS:
        i = ORDEM[lang]
        titulo, h1, lead, desc, rotulo_sumario = TUT[lang]
        partes = ['<section class="section section-dark">\n  <h2>%s</h2>\n  <ol class="tut-toc">\n%s\n  </ol>\n</section>' % (
            rotulo_sumario, "\n".join('    <li><a href="#cartao-%d">%s</a></li>' % (n + 1, html.escape(t[i].capitalize(), quote=False))
                                      for n, (t, _) in enumerate(cartoes)))]
        for n, (t, b) in enumerate(cartoes):
            classe = "section" + (" section-dark" if n % 2 else "")
            partes.append('<section class="%s" id="cartao-%d">\n  <h2>%d · %s</h2>\n%s\n</section>' % (
                classe, n + 1, n + 1, html.escape(t[i], quote=False), paragrafos(b[i])))
        (AQUI / arquivo("tutorial", lang)).write_text(pagina(lang, "tutorial", titulo, desc, h1, lead, "\n\n".join(partes)), encoding="utf-8")
        x = INST[lang]
        partes = []
        for n, (h2, lista) in enumerate(x["secoes"]):
            classe = "section" + (" section-dark" if n % 2 else "")
            partes.append('<section class="%s">\n  <h2>%s</h2>\n%s\n</section>' % (classe, html.escape(h2, quote=False), blocos(lista)))
        (AQUI / arquivo("instalacao", lang)).write_text(pagina(lang, "instalacao", x["title"], x["desc"], x["h1"], x["lead"], "\n\n".join(partes)), encoding="utf-8")
        for tipo in ("conceito", "modulos"):
            caminho = AQUI / arquivo(tipo, lang)
            texto = caminho.read_text(encoding="utf-8")
            texto = NAV_RE.sub(lambda _: menu(lang, tipo), texto, count=1)
            if tipo == "conceito" and 'id="baixar"' not in texto:
                texto = re.sub(r'<section class="section">(\s*<h2>[^<]*</h2>\s*<!-- ESTADO:INICIO -->)',
                               r'<section class="section" id="baixar">\1', texto, count=1)
            caminho.write_text(texto, encoding="utf-8")
    print("paginas_extra.py: 8 páginas novas (tutorial, instalacao), menu em 16")


if __name__ == "__main__":
    main()
