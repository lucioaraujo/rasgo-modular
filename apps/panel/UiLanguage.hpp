#pragma once

// RASGO Modular — vocabulário de UI do painel de teste (não é do
// `rasgo_modular_core`, como o `AlsaSink`).
//
// Mesma convenção dos RASGO Synth (Studio/Performance), Navalha 2 e
// ANTITOTEM: **inglês é o padrão**, com português, francês e espanhol
// disponíveis em runtime pelo botão IDIOMA (cicla EN→PT→FR→ES).
//
// Escopo da tradução (decisão do autor, 2026-09-05): cabeçalho, tutorial,
// créditos e — em passes próprios — o LEARN. **NÃO** se traduzem rótulos
// de parâmetro nem títulos de módulo (`cutoff`, `FILTER`, `freq` — já são
// vocabulário técnico neutro).

#include <array>
#include <cstddef>
#include <string>

namespace rasgo::panel {

enum class Lang { en = 0, pt, fr, es };

// 4 traduções de um texto. Campo vazio ("") cai no inglês; se o inglês
// também estiver vazio, cai no português (base de conteúdo ainda não
// traduzido, ex.: LEARN em fases).
struct L4 {
    const char* en = "";
    const char* pt = "";
    const char* fr = "";
    const char* es = "";
};

inline int langIndex(Lang l) noexcept { return static_cast<int>(l); }

inline Lang nextLang(Lang l) noexcept {
    return static_cast<Lang>((langIndex(l) + 1) % 4);
}

inline const char* langLabel(Lang l) noexcept {
    constexpr std::array<const char*, 4> k{"EN", "PT", "FR", "ES"};
    return k[static_cast<std::size_t>(langIndex(l))];
}

inline const char* langCode(Lang l) noexcept {
    constexpr std::array<const char*, 4> k{"en", "pt", "fr", "es"};
    return k[static_cast<std::size_t>(langIndex(l))];
}

inline Lang langFromCode(const std::string& c) noexcept {
    if (c == "pt") return Lang::pt;
    if (c == "fr") return Lang::fr;
    if (c == "es") return Lang::es;
    return Lang::en;
}

inline std::string tr(const L4& s, Lang l) {
    const std::array<const char*, 4> o{s.en, s.pt, s.fr, s.es};
    const char* v = o[static_cast<std::size_t>(langIndex(l))];
    if (v && *v) return v;
    if (s.en && *s.en) return s.en;   // fallback 1: inglês
    return s.pt ? s.pt : "";          // fallback 2: português (conteúdo em fase)
}

// ---------------------------------------------------------------------------
namespace strings {

// ---- cabeçalho: botões de comando ----------------------------------------
inline const L4 hdrVary    {"VARY",    "VARIA",    "VARIER",   "VARIAR"};
inline const L4 hdrStandby {"STANDBY", "ESPERA",   "VEILLE",   "ESPERA"};
inline const L4 hdrChange  {"CHANGE",  "MUDA",     "CHANGER",  "CAMBIA"};
inline const L4 hdrEvolve  {"EVOLVE",  "EVOLUI",   "ÉVOLUE",   "EVOLUCIONA"};
inline const L4 hdrCross   {"CROSS",   "CRUZA",    "CROISER",  "CRUZAR"};
inline const L4 hdrBank    {"BANK",    "BANCO",    "BANQUE",   "BANCO"};
inline const L4 hdrSave    {"SAVE",    "SALVA",    "ENREG.",   "GUARDA"};
inline const L4 hdrRec     {"REC",     "REC",      "REC",      "REC"};
inline const L4 hdrSeed    {"SEED",    "SEED",     "SEED",     "SEED"};
inline const L4 seedCopied {"COPIED",  "COPIADO",  "COPIÉ",    "COPIADO"};

// ---- cabeçalho: navegação -----------------------------------------------
inline const L4 hdrTutorial{"TUTORIAL", "TUTORIAL", "TUTORIEL",   "TUTORIAL"};
inline const L4 hdrAbout   {"ABOUT",    "SOBRE",    "À PROPOS",   "ACERCA DE"};
inline const L4 close      {"CLOSE",    "FECHAR",   "FERMER",     "CERRAR"};

// ---- cabeçalho: leitura de estado --------------------------------------
inline const L4 rdModules  {"mod",  "mód",  "mod",  "mód"};
inline const L4 rdCables   {"cables","cabos","câbles","cables"};
inline const L4 rdPeak     {"PEAK", "PICO", "CRÊTE","PICO"};
inline const L4 rdPlaying  {"PLAY", "TOCA", "LECT.","REPR."};
inline const L4 rdMuted    {"MUTE", "MUDO", "COUPÉ","MUDO"};

// ---- tutorial (overlay do botão TUTORIAL — rolável) ------------------
inline const L4 tutTitle {
    "HOW TO USE", "COMO USAR", "MODE D’EMPLOI", "CÓMO USAR"};
inline const L4 tutSubtitle {
    "a generative modular — it sounds on its own; you steer it   ·   scroll / ↑↓ · click to close",
    "um modular generativo — soa sozinho; você conduz   ·   role / ↑↓ · clique fecha",
    "un modulaire génératif — il sonne seul ; vous le dirigez   ·   défilez / ↑↓ · clic pour fermer",
    "un modular generativo — suena solo; usted lo guía   ·   desplace / ↑↓ · clic para cerrar"};

inline const L4 tutWhatTitle {
    "WHAT THIS IS", "O QUE É ISTO", "CE QUE C’EST", "QUÉ ES ESTO"};
inline const L4 tutWhatBody {
    "RASGO Modular is a modular synth that plays by itself: it makes sound the moment it opens, with no keyboard and no audio input. You shape what you hear by drawing a random patch (SEED), letting it drift, and re-patching modules by hand. MIDI, audio-in and instrument coupling are optional adapter modules, never required.",
    "O RASGO Modular é um sintetizador modular que toca sozinho: faz som no instante em que abre, sem teclado e sem entrada de áudio. Você molda o que ouve sorteando um patch (SEED), deixando ele derivar e recabeando os módulos à mão. MIDI, entrada de áudio e acoplamento de instrumento são módulos adaptadores opcionais, nunca obrigatórios.",
    "RASGO Modular est un synthé modulaire qui joue tout seul : il produit du son dès l’ouverture, sans clavier ni entrée audio. On façonne ce qu’on entend en tirant un patch aléatoire (SEED), en le laissant dériver et en recâblant les modules à la main. MIDI, entrée audio et couplage d’instrument sont des modules adaptateurs optionnels, jamais requis.",
    "RASGO Modular es un sintetizador modular que suena solo: hace sonido en el momento en que se abre, sin teclado ni entrada de audio. Usted moldea lo que oye sorteando un patch (SEED), dejándolo derivar y recableando los módulos a mano. MIDI, entrada de audio y acoplamiento de instrumento son módulos adaptadores opcionales, nunca obligatorios."};

inline const L4 tutSeedTitle {
    "SEED — DRAW A PATCH", "SEED — SORTEAR UM PATCH", "SEED — TIRER UN PATCH", "SEED — SORTEAR UN PATCH"};
inline const L4 tutSeedBody {
    "The SEED button draws a fresh cabling and timbre and plays it at once. The number beside it identifies the patch — the same number always reproduces the same sound. Mixer pans always start centred; the master gain always starts at −24 dB and you raise it by hand.",
    "O botão SEED sorteia um cabeamento e timbre novos e já toca. O número ao lado identifica o patch — o mesmo número reproduz sempre o mesmo som. Os pans do mixer começam sempre no centro; o ganho do master começa sempre em −24 dB e você sobe à mão.",
    "Le bouton SEED tire un nouveau câblage et timbre et le joue aussitôt. Le numéro à côté identifie le patch — le même numéro reproduit toujours le même son. Les panoramiques du mixeur démarrent toujours au centre ; le gain du master démarre toujours à −24 dB, à monter à la main.",
    "El botón SEED sortea un cableado y timbre nuevos y suena de inmediato. El número al lado identifica el patch — el mismo número reproduce siempre el mismo sonido. Los panoramas del mezclador empiezan siempre centrados; la ganancia del master empieza siempre en −24 dB y usted la sube a mano."};

inline const L4 tutSeedBoxTitle {
    "THE SEED NUMBER BOX", "A CAIXA DO NÚMERO DO SEED", "LA CASE DU NUMÉRO SEED", "LA CAJA DEL NÚMERO SEED"};
inline const L4 tutSeedBoxBody {
    "The box beside SEED is a normal text field. Click to place the cursor, drag to select, double-click or Ctrl+A to select all. Backspace/Delete erase; arrows and Home/End move; Ctrl+C / Ctrl+X / Ctrl+V copy, cut, paste; the middle mouse button pastes the last selection. Type a number and press Enter to load that exact patch. The current seed also prints to the terminal on every change.",
    "A caixa ao lado de SEED é um campo de texto normal. Clique pra posicionar o cursor, arraste pra selecionar, duplo-clique ou Ctrl+A seleciona tudo. Backspace/Delete apagam; setas e Home/End movem; Ctrl+C / Ctrl+X / Ctrl+V copiam, recortam, colam; o botão do meio cola a última seleção. Digite um número e Enter pra carregar aquele patch exato. O seed atual também sai no terminal a cada troca.",
    "La case à côté de SEED est un champ de texte normal. Cliquez pour placer le curseur, glissez pour sélectionner, double-clic ou Ctrl+A tout sélectionner. Retour arrière/Suppr effacent ; flèches et Origine/Fin déplacent ; Ctrl+C / Ctrl+X / Ctrl+V copient, coupent, collent ; le bouton du milieu colle la dernière sélection. Tapez un numéro puis Entrée pour charger ce patch précis. Le seed courant s’affiche aussi dans le terminal à chaque changement.",
    "La caja junto a SEED es un campo de texto normal. Clic para situar el cursor, arrastre para seleccionar, doble clic o Ctrl+A selecciona todo. Retroceso/Supr borran; flechas y Inicio/Fin mueven; Ctrl+C / Ctrl+X / Ctrl+V copian, cortan, pegan; el botón central pega la última selección. Escriba un número y Enter para cargar ese patch exacto. El seed actual también se imprime en la terminal en cada cambio."};

inline const L4 tutVaryTitle {
    "VARY THE PATCH", "VARIAR O PATCH", "VARIER LE PATCH", "VARIAR EL PATCH"};
inline const L4 tutVaryBody {
    "VARY toggles the \"chaotic hand\": one slow motion that moves every fibre of the patch in relation, pulling back when the output gets hot. STANDBY mutes the master. CHANGE resamples about a quarter of the parameters at once; EVOLVE does the same in six small steps; CROSS recombines the patch with a freshly drawn one. A DRIFT module in the patch also animates it on its own.",
    "VARIA liga a \"mão caótica\": um movimento lento que move todas as fibras do patch em relação, recuando quando a saída esquenta. ESPERA muta o master. MUDA reamostra cerca de um quarto dos parâmetros de uma vez; EVOLUI faz o mesmo em seis passos pequenos; CRUZA recombina o patch com um recém-sorteado. Um módulo DERIVA no patch também o anima sozinho.",
    "VARIER active la \"main chaotique\" : un mouvement lent qui déplace toutes les fibres du patch en relation, en reculant quand la sortie chauffe. VEILLE coupe le master. CHANGER rééchantillonne environ un quart des paramètres d’un coup ; ÉVOLUE fait de même en six petits pas ; CROISER recombine le patch avec un patch fraîchement tiré. Un module DÉRIVE dans le patch l’anime aussi tout seul.",
    "VARIAR activa la \"mano caótica\": un movimiento lento que mueve todas las fibras del patch en relación, retrocediendo cuando la salida se calienta. ESPERA silencia el master. CAMBIA remuestrea alrededor de un cuarto de los parámetros de una vez; EVOLUCIONA hace lo mismo en seis pasos pequeños; CRUZAR recombina el patch con uno recién sorteado. Un módulo DERIVA en el patch también lo anima solo."};

inline const L4 tutStoreTitle {
    "KEEP A PATCH — BANK, SAVE", "GUARDAR UM PATCH — BANCO, SALVA", "GARDER UN PATCH — BANQUE, ENREG.", "GUARDAR UN PATCH — BANCO, GUARDA"};
inline const L4 tutStoreBody {
    "BANK stores the current patch in a rotating set you can page back through. SAVE writes the whole session to disk; it also auto-saves on exit and reloads on the next launch. Everything lives in  ~/.local/share/rasgo-modular/  (session.rmp and the banks).",
    "BANCO guarda o patch atual num conjunto rotativo que dá pra folhear de volta. SALVA grava a sessão inteira em disco; ela também é auto-salva na saída e recarregada no próximo arranque. Tudo fica em  ~/.local/share/rasgo-modular/  (session.rmp e os bancos).",
    "BANQUE stocke le patch courant dans un jeu tournant que l’on peut refeuilleter. ENREG. écrit toute la session sur le disque ; elle est aussi enregistrée automatiquement à la sortie et rechargée au lancement suivant. Tout se trouve dans  ~/.local/share/rasgo-modular/  (session.rmp et les banques).",
    "BANCO guarda el patch actual en un conjunto rotativo que se puede hojear hacia atrás. GUARDA escribe toda la sesión en disco; también se guarda sola al salir y se recarga en el siguiente arranque. Todo está en  ~/.local/share/rasgo-modular/  (session.rmp y los bancos)."};

inline const L4 tutRecTitle {
    "RECORD AUDIO — REC", "GRAVAR ÁUDIO — REC", "ENREGISTRER — REC", "GRABAR AUDIO — REC"};
inline const L4 tutRecBody {
    "REC (or Ctrl+R) starts and stops recording the output. On stop it writes a stereo 16-bit WAV to  ~/.local/share/rasgo-modular/  named rec-01.wav, rec-02.wav, … next to a rec-NN.score.txt log of the take. Recording auto-stops when the in-memory buffer (about four minutes) fills.",
    "REC (ou Ctrl+R) começa e para a gravação da saída. Ao parar, grava um WAV estéreo de 16 bits em  ~/.local/share/rasgo-modular/  com nome rec-01.wav, rec-02.wav, … ao lado de um rec-NN.score.txt com o registro da tomada. A gravação para sozinha quando o buffer em memória (cerca de quatro minutos) enche.",
    "REC (ou Ctrl+R) démarre et arrête l’enregistrement de la sortie. À l’arrêt, il écrit un WAV stéréo 16 bits dans  ~/.local/share/rasgo-modular/  nommé rec-01.wav, rec-02.wav, … à côté d’un rec-NN.score.txt journal de la prise. L’enregistrement s’arrête seul quand le tampon mémoire (environ quatre minutes) est plein.",
    "REC (o Ctrl+R) inicia y detiene la grabación de la salida. Al parar, escribe un WAV estéreo de 16 bits en  ~/.local/share/rasgo-modular/  llamado rec-01.wav, rec-02.wav, … junto a un rec-NN.score.txt con el registro de la toma. La grabación se detiene sola cuando el búfer en memoria (unos cuatro minutos) se llena."};

inline const L4 tutHdrTitle {
    "THE HEADER, LEFT TO RIGHT", "O CABEÇALHO, DA ESQUERDA PRA DIREITA", "L’EN-TÊTE, DE GAUCHE À DROITE", "LA CABECERA, DE IZQUIERDA A DERECHA"};
inline const L4 tutHdrBody {
    "VARY chaotic hand · STANDBY master mute · CHANGE resample ~25% · EVOLVE the same in 6 steps · CROSS recombine with a new patch · BANK store patch · SAVE write session · REC record WAV · SEED draw patch, with its number box · the readout shows modules / cables / output PEAK · the language button cycles EN→PT→FR→ES · TUTORIAL this screen · ABOUT version and licence.",
    "VARIA mão caótica · ESPERA muta o master · MUDA reamostra ~25% · EVOLUI o mesmo em 6 passos · CRUZA recombina com um patch novo · BANCO guarda o patch · SALVA grava a sessão · REC grava WAV · SEED sorteia o patch, com a caixa do número · a leitura mostra módulos / cabos / PICO da saída · o botão de idioma cicla EN→PT→FR→ES · TUTORIAL esta tela · SOBRE versão e licença.",
    "VARIER main chaotique · VEILLE coupe le master · CHANGER rééchantillonne ~25% · ÉVOLUE de même en 6 pas · CROISER recombine avec un nouveau patch · BANQUE stocke le patch · ENREG. écrit la session · REC enregistre un WAV · SEED tire le patch, avec sa case numéro · l’affichage montre modules / câbles / CRÊTE de sortie · le bouton de langue fait EN→PT→FR→ES · TUTORIEL cet écran · À PROPOS version et licence.",
    "VARIAR mano caótica · ESPERA silencia el master · CAMBIA remuestrea ~25% · EVOLUCIONA lo mismo en 6 pasos · CRUZAR recombina con un patch nuevo · BANCO guarda el patch · GUARDA escribe la sesión · REC graba WAV · SEED sortea el patch, con su caja de número · la lectura muestra módulos / cables / PICO de salida · el botón de idioma cicla EN→PT→FR→ES · TUTORIAL esta pantalla · ACERCA DE versión y licencia."};

inline const L4 tutCableTitle {
    "PATCH CABLES", "CABEAR", "CÂBLER", "CABLEAR"};
inline const L4 tutCableBody {
    "Drag a cable from an output jack to an input jack. A double halo means the same signal type (audio or control). Right-click a jack to remove its cable. [space] cuts and restores every cable at once. While dragging a cable, move it near the top or bottom edge to scroll the rack.",
    "Puxe um cabo de um jack de saída até um de entrada. Halo duplo = mesmo tipo de sinal (áudio ou controle). Botão direito num jack tira o cabo. [espaço] rompe e reata todos os cabos de uma vez. Enquanto arrasta um cabo, leve-o pra perto da borda de cima ou de baixo pra rolar o rack.",
    "Tirez un câble d’une sortie vers une entrée. Un double halo indique le même type de signal (audio ou contrôle). Clic droit sur un jack pour retirer son câble. [espace] coupe et rétablit tous les câbles d’un coup. En tirant un câble, approchez-le du bord haut ou bas pour faire défiler le rack.",
    "Arrastre un cable de una salida a una entrada. Doble halo = mismo tipo de señal (audio o control). Clic derecho en un jack para quitar su cable. [espacio] corta y restablece todos los cables a la vez. Mientras arrastra un cable, llévelo cerca del borde superior o inferior para desplazar el rack."};

inline const L4 tutNavTitle {
    "MOVE AROUND THE RACK", "NAVEGAR NO RACK", "SE DÉPLACER DANS LE RACK", "MOVERSE POR EL RACK"};
inline const L4 tutNavBody {
    "Mouse wheel scrolls the rack; the middle mouse button drags it up and down (it also works while you are patching a cable). Ctrl+= and Ctrl+− zoom in and out, Ctrl+0 back to 100%. Zoom out to see the first and last rows together and patch between them.",
    "A roda do mouse rola o rack; o botão do meio arrasta pra cima e pra baixo (funciona também enquanto você está cabeando). Ctrl+= e Ctrl+− ampliam e reduzem, Ctrl+0 volta a 100%. Reduza pra ver a primeira e a última fileira juntas e cabear entre elas.",
    "La molette fait défiler le rack ; le bouton du milieu le tire de haut en bas (aussi pendant le câblage). Ctrl+= et Ctrl+− zooment, Ctrl+0 revient à 100%. Dézoomez pour voir la première et la dernière rangée ensemble et câbler entre elles.",
    "La rueda del ratón desplaza el rack; el botón central lo arrastra arriba y abajo (también mientras cablea). Ctrl+= y Ctrl+− amplían y reducen, Ctrl+0 vuelve al 100%. Aleje para ver la primera y la última fila juntas y cablear entre ellas."};

inline const L4 tutModTitle {
    "ADD, MOVE, REMOVE MODULES", "ADICIONAR, MOVER, REMOVER MÓDULOS", "AJOUTER, DÉPLACER, RETIRER DES MODULES", "AÑADIR, MOVER, QUITAR MÓDULOS"};
inline const L4 tutModBody {
    "The left column is the palette — the full catalogue, grouped by family. Drag a name from it into the rack to add that module. Drag a module by its body to reposition it. To remove one: click the [x] at its top-right corner, or drag it back onto the palette. The instrument opens with one of every module already in the rack, all silent until you cable them.",
    "A coluna da esquerda é a paleta — o catálogo inteiro, agrupado por família. Arraste um nome dela pra o rack pra adicionar aquele módulo. Arraste um módulo pelo corpo pra reposicionar. Pra remover: clique no [x] no canto superior direito, ou arraste o módulo de volta pra paleta. O instrumento abre com um de cada módulo já no rack, todos mudos até você cabeá-los.",
    "La colonne de gauche est la palette — le catalogue complet, groupé par famille. Glissez-en un nom dans le rack pour ajouter ce module. Glissez un module par son corps pour le repositionner. Pour en retirer un : cliquez le [x] en haut à droite, ou glissez le module vers la palette. L’instrument s’ouvre avec un de chaque module déjà dans le rack, tous muets jusqu’à ce que vous les câbliez.",
    "La columna izquierda es la paleta — el catálogo completo, agrupado por familia. Arrastre un nombre de ella al rack para añadir ese módulo. Arrastre un módulo por su cuerpo para reubicarlo. Para quitar uno: clic en la [x] de su esquina superior derecha, o arrástrelo de vuelta a la paleta. El instrumento abre con uno de cada módulo ya en el rack, todos mudos hasta que los cablee."};

inline const L4 tutFamTitle {
    "THE EIGHT FAMILIES", "AS OITO FAMÍLIAS", "LES HUIT FAMILLES", "LAS OCHO FAMILIAS"};
inline const L4 tutFamBody {
    "SOURCE makes sound from nothing (oscillators, physical models, noise). TRANSFORM changes a signal passing through it — audio or control (filters, VCAs, waveshapers). MODULATE generates a control signal (envelopes, LFOs, chaos, sample-and-hold). TIME keeps time (clock, clock logic, sequencers). DECISION picks a value (quantise, harmonise, count). ROUTE sends signals around (switch, matrix, multiple, morph). SPACE places and remembers sound (delay, reverb, granular, looper). OUT mixes, meters and sends (mixer, master, scope, note-out).",
    "SOURCE faz som do nada (osciladores, modelos físicos, ruído). TRANSFORM muda um sinal que passa por ele — áudio ou controle (filtros, VCAs, modeladores de onda). MODULATE gera um sinal de controle (envelopes, LFOs, caos, sample-and-hold). TIME marca o tempo (clock, lógica de clock, sequenciadores). DECISION escolhe um valor (quantiza, harmoniza, conta). ROUTE encaminha sinais (chave, matriz, múltiplo, morph). SPACE espacializa e lembra o som (delay, reverb, granular, looper). OUT mistura, mede e envia (mixer, master, scope, note-out).",
    "SOURCE crée du son à partir de rien (oscillateurs, modèles physiques, bruit). TRANSFORM modifie un signal qui la traverse — audio ou contrôle (filtres, VCA, waveshapers). MODULATE génère un signal de contrôle (enveloppes, LFO, chaos, sample-and-hold). TIME marque le temps (horloge, logique d’horloge, séquenceurs). DECISION choisit une valeur (quantifier, harmoniser, compter). ROUTE achemine les signaux (commutateur, matrice, multiple, morph). SPACE spatialise et mémorise le son (delay, réverbe, granulaire, looper). OUT mixe, mesure et envoie (mixeur, master, scope, note-out).",
    "SOURCE hace sonido de la nada (osciladores, modelos físicos, ruido). TRANSFORM cambia una señal que pasa por él — audio o control (filtros, VCA, modeladores de onda). MODULATE genera una señal de control (envolventes, LFO, caos, sample-and-hold). TIME marca el tiempo (reloj, lógica de reloj, secuenciadores). DECISION elige un valor (cuantizar, armonizar, contar). ROUTE encamina señales (conmutador, matriz, múltiple, morph). SPACE espacializa y recuerda el sonido (delay, reverb, granular, looper). OUT mezcla, mide y envía (mezclador, master, scope, note-out)."};

inline const L4 tutLearnTitle {
    "LEARN — EVERY MODULE, EVERY KNOB", "LEARN — CADA MÓDULO, CADA KNOB", "LEARN — CHAQUE MODULE, CHAQUE POT.", "LEARN — CADA MÓDULO, CADA KNOB"};
inline const L4 tutLearnBody {
    "Hover a module's body and the box in the lower-left corner tells you what that module is for. Hover any knob or jack and it explains that control in that module, on three levels — quick, how it works, and one thing to try. A fuller written guide and patch recipes are coming as a companion site and PDF.",
    "Passe o mouse sobre o corpo de um módulo e a caixa no canto inferior esquerdo diz pra que aquele módulo serve. Passe sobre qualquer knob ou jack e ela explica aquele controle naquele módulo, em três níveis — rápido, como funciona, e um experimento. Um guia escrito mais completo e receitas de patch estão a caminho, como site e PDF.",
    "Survolez le corps d’un module : la boîte en bas à gauche dit à quoi sert ce module. Survolez un potentiomètre ou un jack : elle explique ce contrôle dans ce module, sur trois niveaux — rapide, comment ça marche, et une chose à essayer. Un guide écrit plus complet et des recettes de patch arrivent, sous forme de site et de PDF.",
    "Pase el ratón sobre el cuerpo de un módulo: la caja de la esquina inferior izquierda dice para qué sirve ese módulo. Pase sobre un knob o jack: explica ese control en ese módulo, en tres niveles — rápido, cómo funciona, y algo para probar. Una guía escrita más completa y recetas de patch están en camino, como sitio y PDF."};

// ---- créditos / sobre -----------------------------------------------
inline const L4 aboutBody {
    "RASGO Modular — the generative modular environment of the RASGO family. It sounds on load, with no MIDI and no audio input; MIDI, audio and instrument coupling are optional adapter nodes.\n\nLucio de Araujo — 2026\nGNU AGPLv3 or later.",
    "RASGO Modular — o ambiente modular generativo da família RASGO. Soa no arranque, sem MIDI e sem entrada de áudio; MIDI, áudio e acoplamento de instrumento são nós adaptadores opcionais.\n\nLúcio de Araújo — 2026\nGNU AGPLv3 ou posterior.",
    "RASGO Modular — l’environnement modulaire génératif de la famille RASGO. Il sonne au démarrage, sans MIDI ni entrée audio ; MIDI, audio et couplage d’instrument sont des nœuds adaptateurs optionnels.\n\nLucio de Araujo — 2026\nGNU AGPLv3 ou ultérieure.",
    "RASGO Modular — el entorno modular generativo de la familia RASGO. Suena al arrancar, sin MIDI ni entrada de audio; MIDI, audio y acoplamiento de instrumento son nodos adaptadores opcionales.\n\nLucio de Araujo — 2026\nGNU AGPLv3 o posterior."};

// frase de crédito do rodapé — padrão da família RASGO (cf. Antitotem,
// Rasgo Synth). No RASGO Modular ela vai na faixa acima da 1ª fileira de
// módulos, não no rodapé (pedido do autor 2026-09-07). O `panel_main`
// acrescenta " · <build>" (hash do git + data) ao fim.
inline const L4 footerCredit {
    "© LÚCIO DE ARAÚJO  ·  RASGO MODULAR 2026  ·  AGPLv3+ LICENSE  ·  ",
    "© LÚCIO DE ARAÚJO  ·  RASGO MODULAR 2026  ·  LICENÇA AGPLv3+  ·  ",
    "© LÚCIO DE ARAÚJO  ·  RASGO MODULAR 2026  ·  LICENCE AGPLv3+  ·  ",
    "© LÚCIO DE ARAÚJO  ·  RASGO MODULAR 2026  ·  LICENCIA AGPLv3+  ·  "};

}  // namespace strings
}  // namespace rasgo::panel
