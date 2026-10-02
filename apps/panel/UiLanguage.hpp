#pragma once

// RASGO Modular — vocabulário de UI do painel de teste (não é do
// `rasgo_modular_core`, como o `AlsaSink`).
//
// Mesma convenção dos RASGO Synth (Studio/Performance), Navalha 2 e
// ANTITOTEM: inglês é o padrão, com português, francês e espanhol
// disponíveis em runtime pelo botão IDIOMA (cicla EN→PT→FR→ES).
//
// Escopo da tradução (decisão do autor, 2026-09-05): cabeçalho, tutorial,
// créditos e — em passes próprios — o LEARN. NÃO se traduzem rótulos
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
inline const L4 hdrOpen    {"OPEN",    "ABRIR",    "OUVRIR",   "ABRIR"};
inline const L4 hdrUndo    {"UNDO",    "DESFAZ",   "ANNULER",  "DESHACER"};
inline const L4 hdrUncable {"UNPATCH", "DESCABEIA","DÉCÂBLER", "DESCABLEA"};
// Volta o patch ao estado original do seed atual, desfazendo TODA edição
// manual de uma vez. Diferente do desfazer, que anda um passo.
inline const L4 hdrRestore {"RESTORE", "REPOR",    "RÉTABLIR", "REPONER"};

// Aviso momentâneo na caixa LEARN, depois de ligar um cabo num caminho
// que ainda não alcança a saída. Descreve ESTADO, não erro: construir uma
// cadeia longe da saída e ligá-la ao som por último é legítimo.
inline const L4 cableNotAudible {
    "Patched — but this path does not reach the sound yet. Cable it onward to the MASTER to hear it.",
    "Ligado — mas este caminho ainda não chega ao som. Siga cabeando até o MASTER pra ouvir.",
    "Câblé — mais ce chemin n’atteint pas encore le son. Continuez à câbler jusqu’au MASTER pour l’entendre.",
    "Conectado — pero este camino aún no llega al sonido. Siga cableando hasta el MASTER para oírlo."};
inline const L4 hdrRec     {"REC",     "REC",      "REC",      "REC"};
inline const L4 hdrSeed    {"SEED",    "SEED",     "SEED",     "SEED"};
inline const L4 seedCopied {"COPIED",  "COPIADO",  "COPIÉ",    "COPIADO"};

// ---- cabeçalho: vista do rack (botão RACK, alterna 2 estados) ---------
// TODOS os módulos  ↔  só os que chegam à saída (os que de fato soam).
inline const L4 hdrRackAll {"ALL",    "TODOS", "TOUS",   "TODOS"};
inline const L4 hdrRackOut {"OUTPUT", "SAÍDA", "SORTIE", "SALIDA"};
inline const L4 rackViewEmpty {
    "no module reaches the output yet — patch toward the MASTER, or RACK: ALL",
    "nenhum módulo chega à saída ainda — cabeie até o MASTER, ou RACK: TODOS",
    "aucun module n’atteint la sortie — câblez vers le MASTER, ou RACK : TOUS",
    "ningún módulo llega a la salida — cablee hacia el MASTER, o RACK: TODOS"};

// ---- cabeçalho: navegação -----------------------------------------------
inline const L4 hdrTutorial{"TUTORIAL", "TUTORIAL", "TUTORIEL",   "TUTORIAL"};
inline const L4 hdrAbout   {"ABOUT",    "SOBRE",    "À PROPOS",   "ACERCA DE"};
inline const L4 close      {"CLOSE",    "FECHAR",   "FERMER",     "CERRAR"};

// ---- inspector de cabo e diálogos ------------------------------------
// Verbos e prosa SE traduzem. Rótulos técnicos não (`RING`, `FOLD`,
// `DIFF`, `AMT`, `COND`, `ZOOM`, `LEARN`) — são vocabulário neutro, mesma
// regra dos nomes de parâmetro e de módulo declarada no topo deste
// arquivo.
inline const L4 inspRupture   {"BREAK",     "ROMPER",     "ROMPRE",      "ROMPER"};
inline const L4 inspReconnect {"RECONNECT", "RECONECTAR", "RECONNECTER", "RECONECTAR"};
// caixa do cabo, grupo 1 (2 out. 2026): tirar o cabo pela própria caixa
// ("REMOVER" lia como tirar o MÓDULO — trocado a pedido do autor), e
// a marca de cabo de realimentação (atrasado um bloco para fechar um laço)
inline const L4 learnCableHead {"CABLE  \xc2\xb7  click to open", "CABO  \xc2\xb7  clique para abrir",
                                 "CÂBLE  \xc2\xb7  cliquer pour ouvrir", "CABLE  \xc2\xb7  clic para abrir"};
inline const L4 inspRemove    {"UNPLUG",    "DESPLUGAR",  "DÉBRANCHER",  "DESENCHUFAR"};
inline const L4 inspFeedback  {"FEEDBACK",  "REALIMENTA", "RÉTROACTION", "REALIMENTA"};
inline const L4 openPatch     {"Open patch","Abrir patch","Ouvrir un patch","Abrir patch"};
inline const L4 builtOn       {"built",     "compilado",  "compilé",     "compilado"};

// ---- cabeçalho: leitura de estado --------------------------------------
inline const L4 rdModules  {"mod",  "mód",  "mod",  "mód"};
inline const L4 rdCables   {"cables","cabos","câbles","cables"};
inline const L4 rdPeak     {"PEAK", "PICO", "CRÊTE","PICO"};
inline const L4 rdPlaying  {"PLAY", "TOCA", "LECT.","REPR."};
inline const L4 rdMuted    {"MUTE", "MUDO", "COUPÉ","MUDO"};

// ---- tutorial (overlay do botão TUTORIAL — rolável) ------------------
inline const L4 tutTitle {
    "HOW TO USE", "COMO USAR", "MODE D’EMPLOI", "CÓMO USAR"};
// Correção de moldura pedida pelo autor (28 set. 2026): o instrumento NÃO
// é "um modular que toca sozinho". Quem toca é o músico; o que o
// instrumento faz é APRESENTAR um patch como ponto de partida, em vez de
// abrir em branco. A diferença não é de estilo — a primeira versão dava ao
// programa o papel do músico, e era o oposto do que o instrumento é.
inline const L4 tutSubtitle {
    "patches as a starting point — you play it   ·   scroll / ↑↓",
    "patches como ponto de partida — quem toca é você   ·   role / ↑↓",
    "des patches comme point de départ — c’est vous qui jouez   ·   défilez / ↑↓",
    "patches como punto de partida — quien toca es usted   ·   desplace / ↑↓"};

inline const L4 tutWhatTitle {
    "WHAT THIS IS", "O QUE É ISTO", "CE QUE C’EST", "QUÉ ES ESTO"};
inline const L4 tutWhatBody {
    "RASGO Modular is a modular synth that never opens blank: it offers a patch already built and sounding, as a starting point. You are the one who plays it — drawing another patch (SEED), adjusting, letting it drift, re-patching modules by hand, or taking it all apart and building from scratch. No keyboard and no audio input are needed; MIDI, audio-in and instrument coupling are optional adapter modules, never required.",
    "O RASGO Modular é um sintetizador modular que nunca abre em branco: ele apresenta um patch já montado e soando, como ponto de partida. Quem toca é você — sorteando outro patch (SEED), ajustando, deixando derivar, recabeando os módulos à mão, ou desmontando tudo e construindo do zero. Não precisa de teclado nem de entrada de áudio; MIDI, entrada de áudio e acoplamento de instrumento são módulos adaptadores opcionais, nunca obrigatórios.",
    "RASGO Modular est un synthé modulaire qui ne s’ouvre jamais sur une page blanche : il propose un patch déjà monté et sonnant, comme point de départ. C’est vous qui jouez — en tirant un autre patch (SEED), en ajustant, en laissant dériver, en recâblant les modules à la main, ou en démontant tout pour construire depuis zéro. Ni clavier ni entrée audio ne sont nécessaires ; MIDI, entrée audio et couplage d’instrument sont des modules adaptateurs optionnels, jamais requis.",
    "RASGO Modular es un sintetizador modular que nunca abre en blanco: ofrece un patch ya montado y sonando, como punto de partida. Quien toca es usted — sorteando otro patch (SEED), ajustando, dejando derivar, recableando los módulos a mano, o desmontándolo todo y construyendo desde cero. No hace falta teclado ni entrada de audio; MIDI, entrada de audio y acoplamiento de instrumento son módulos adaptadores opcionales, nunca obligatorios."};

inline const L4 tutSeedTitle {
    "SEED — DRAW A PATCH", "SEED — SORTEAR UM PATCH", "SEED — TIRER UN PATCH", "SEED — SORTEAR UN PATCH"};
inline const L4 tutSeedBody {
    "The SEED button draws a fresh cabling and timbre and plays it at once. The number beside it identifies the patch — the same number always reproduces the same sound. Mixer pans always start centred; the master gain is set for each seed so that every patch opens at a similar loudness — raise or lower it by hand from there.",
    "O botão SEED sorteia um cabeamento e timbre novos e já toca. O número ao lado identifica o patch — o mesmo número reproduz sempre o mesmo som. Os pans do mixer começam sempre no centro; o ganho do master é ajustado para cada seed, para todo patch abrir num volume parecido — a partir daí, suba ou baixe à mão.",
    "Le bouton SEED tire un nouveau câblage et timbre et le joue aussitôt. Le numéro à côté identifie le patch — le même numéro reproduit toujours le même son. Les panoramiques du mixeur démarrent toujours au centre ; le gain du master est réglé pour chaque seed, afin que chaque patch s’ouvre à un volume semblable — ensuite, montez-le ou baissez-le à la main.",
    "El botón SEED sortea un cableado y timbre nuevos y suena de inmediato. El número al lado identifica el patch — el mismo número reproduce siempre el mismo sonido. Los panoramas del mezclador empiezan siempre centrados; la ganancia del master se ajusta para cada seed, para que todo patch abra a un volumen parecido — a partir de ahí, súbala o bájela a mano."};

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
    "VARY is a SLIDER, not a button: at zero the hand lets go of the instrument entirely, at the centre mark it is the normal amount, and to the right it grows bolder. It moves every fibre of the patch in relation, pulling back when the output gets hot. The lower the slider, the rarer it also is for the hand to touch the switches — with VARY gentle the STRUCTURE stays put and only what sweeps moves. STANDBY mutes the master. CHANGE resamples about a quarter of the parameters at once; EVOLVE does the same in six small steps; CROSS recombines the patch with a freshly drawn one. A DRIFT module in the patch also animates it on its own.",
    "VARIA é um SLIDER, não um botão: em zero a mão solta o instrumento por completo, no meio (a marca do centro) é a intensidade normal, e à direita ela fica mais ousada. Ela move todas as fibras do patch em relação, recuando quando a saída esquenta. Quanto mais baixo o slider, mais raro também é ela mexer nos interruptores — em VARIA suave a ESTRUTURA fica quieta e só o que varre se move. ESPERA muta o master. MUDA reamostra cerca de um quarto dos parâmetros de uma vez; EVOLUI faz o mesmo em seis passos pequenos; CRUZA recombina o patch com um recém-sorteado. Um módulo DERIVA no patch também o anima sozinho.",
    "VARIER est un CURSEUR, pas un bouton : à zéro la main lâche complètement l’instrument, au repère central c’est l’intensité normale, et vers la droite elle devient plus audacieuse. Elle déplace toutes les fibres du patch en relation, en reculant quand la sortie chauffe. Plus le curseur est bas, plus il est rare qu’elle touche aux interrupteurs — en VARIER doux la STRUCTURE reste tranquille et seul ce qui balaie bouge. VEILLE coupe le master. CHANGER rééchantillonne environ un quart des paramètres d’un coup ; ÉVOLUE fait de même en six petits pas ; CROISER recombine le patch avec un patch fraîchement tiré. Un module DÉRIVE dans le patch l’anime aussi tout seul.",
    "VARIAR es un DESLIZADOR, no un botón: en cero la mano suelta el instrumento por completo, en la marca central es la intensidad normal, y hacia la derecha se vuelve más audaz. Mueve todas las fibras del patch en relación, retrocediendo cuando la salida se calienta. Cuanto más bajo el deslizador, más raro es también que toque los interruptores — con VARIAR suave la ESTRUCTURA se queda quieta y sólo se mueve lo que barre. ESPERA silencia el master. CAMBIA remuestrea alrededor de un cuarto de los parámetros de una vez; EVOLUCIONA hace lo mismo en seis pasos pequeños; CRUZAR recombina el patch con uno recién sorteado. Un módulo DERIVA en el patch también lo anima solo."};

inline const L4 tutStoreTitle {
    "KEEP A PATCH — BANK, SAVE", "GUARDAR UM PATCH — BANCO, SALVA", "GARDER UN PATCH — BANQUE, ENREG.", "GUARDAR UN PATCH — BANCO, GUARDA"};
inline const L4 tutStoreBody {
    "SAVE (Ctrl+S) writes the whole session to a .rmp — the graph, the cabling, every parameter, the seed. It is the instrument, not the sound. BANK (Ctrl+B) files the current patch under its own name instead of overwriting the session. OPEN (Ctrl+O) brings any .rmp back, starting in the bank folder. All of it lives in  ~/.local/share/rasgo-modular/  (session.rmp and patches/). REC is a different thing: it records the AUDIO — see RECORD AUDIO.",
    "SALVA (Ctrl+S) grava a sessão inteira num .rmp — o grafo, o cabeamento, cada parâmetro, o seed. É o instrumento, não o som. BANCO (Ctrl+B) arquiva o patch atual com nome próprio, em vez de sobrescrever a sessão. ABRIR (Ctrl+O) traz qualquer .rmp de volta, já começando na pasta do banco. Tudo fica em  ~/.local/share/rasgo-modular/  (session.rmp e patches/). REC é outra coisa: grava o ÁUDIO — veja GRAVAR ÁUDIO.",
    "ENREG. (Ctrl+S) écrit toute la session dans un .rmp — le graphe, le câblage, chaque paramètre, le seed. C’est l’instrument, pas le son. BANQUE (Ctrl+B) classe le patch courant sous son propre nom au lieu d’écraser la session. OUVRIR (Ctrl+O) rouvre n’importe quel .rmp, en démarrant dans le dossier de la banque. Tout est dans  ~/.local/share/rasgo-modular/  (session.rmp et patches/). REC est autre chose : il enregistre l’AUDIO — voir ENREGISTRER.",
    "GUARDA (Ctrl+S) escribe la sesión entera en un .rmp — el grafo, el cableado, cada parámetro, el seed. Es el instrumento, no el sonido. BANCO (Ctrl+B) archiva el patch actual con nombre propio en vez de sobrescribir la sesión. ABRIR (Ctrl+O) trae de vuelta cualquier .rmp, empezando en la carpeta del banco. Todo está en  ~/.local/share/rasgo-modular/  (session.rmp y patches/). REC es otra cosa: graba el AUDIO — vea GRABAR AUDIO."};

inline const L4 tutRecTitle {
    "RECORD AUDIO — REC", "GRAVAR ÁUDIO — REC", "ENREGISTRER — REC", "GRABAR AUDIO — REC"};
inline const L4 tutRecBody {
    "REC (or Ctrl+R) starts and stops recording the output. On stop it writes a stereo 24-bit WAV to  ~/Music/RasgoModular/  named by date and time — rec-20260911-143052.wav — next to a matching .score.txt log of the take (so it never overwrites). Recording auto-stops when the in-memory buffer (about four minutes) fills. Set RASGO_REC_DIR to change the folder.",
    "REC (ou Ctrl+R) começa e para a gravação da saída. Ao parar, grava um WAV estéreo de 24 bits em  ~/Music/RasgoModular/  com nome por data e hora — rec-20260911-143052.wav — ao lado de um .score.txt com o mesmo nome (nunca sobrescreve). A gravação para sozinha quando o buffer em memória (cerca de quatro minutos) enche. RASGO_REC_DIR no ambiente muda a pasta.",
    "REC (ou Ctrl+R) démarre et arrête l’enregistrement de la sortie. À l’arrêt, il écrit un WAV stéréo 24 bits dans  ~/Music/RasgoModular/  nommé par date et heure — rec-20260911-143052.wav — à côté d’un .score.txt du même nom (n’écrase jamais). L’enregistrement s’arrête seul quand le tampon mémoire (environ quatre minutes) est plein. RASGO_REC_DIR change le dossier.",
    "REC (o Ctrl+R) inicia y detiene la grabación de la salida. Al parar, escribe un WAV estéreo de 24 bits en  ~/Music/RasgoModular/  con nombre por fecha y hora — rec-20260911-143052.wav — junto a un .score.txt del mismo nombre (nunca sobrescribe). La grabación se detiene sola cuando el búfer en memoria (unos cuatro minutos) se llena. RASGO_REC_DIR cambia la carpeta."};

inline const L4 tutHdrTitle {
    "THE HEADER, LEFT TO RIGHT", "O CABEÇALHO, DA ESQUERDA PRA DIREITA", "L’EN-TÊTE, DE GAUCHE À DROITE", "LA CABECERA, DE IZQUIERDA A DERECHA"};
inline const L4 tutHdrBody {
    "VARY chaotic hand (a SLIDER: 0 = off) · STANDBY master mute · CHANGE resample ~25% · EVOLVE the same in 6 steps · CROSS recombine with a new patch · BANK store patch · SAVE write session · REC record WAV · SEED draw patch, with its number box · the MASTER meter shows the recent peak against the −1 dBFS ceiling: the mark on the RIGHT lights when the limiter had to hold something, the one on the LEFT when BODY — the guard against piercing sound — is acting. Both are rare, and that is the point. The readout shows modules / cables / output PEAK · the language button cycles EN→PT→FR→ES · RACK toggles the rack view: ALL modules ↔ only those reaching the OUTPUT · TUTORIAL this screen · ABOUT version and licence.",
    "VARIA mão caótica (é um SLIDER: 0 = desligado) · ESPERA muta o master · MUDA reamostra ~25% · EVOLUI o mesmo em 6 passos · CRUZA recombina com um patch novo · BANCO guarda o patch · SALVA grava a sessão · REC grava WAV · SEED sorteia o patch, com a caixa do número · o medidor do MASTER mostra o pico recente contra o teto de −1 dBFS: a marca da DIREITA acende quando o limitador teve que segurar algo, e a da ESQUERDA quando o BODY — a guarda contra som que fura — está agindo. As duas são raras, e é isso que se espera. A leitura mostra módulos / cabos / PICO da saída · o botão de idioma cicla EN→PT→FR→ES · RACK alterna a vista do rack: TODOS os módulos ↔ só os que chegam à SAÍDA · TUTORIAL esta tela · SOBRE versão e licença.",
    "VARIER main chaotique (un CURSEUR : 0 = éteint) · VEILLE coupe le master · CHANGER rééchantillonne ~25% · ÉVOLUE de même en 6 pas · CROISER recombine avec un nouveau patch · BANQUE stocke le patch · ENREG. écrit la session · REC enregistre un WAV · SEED tire le patch, avec sa case numéro · le vumètre du MASTER montre la crête récente face au plafond de −1 dBFS : le repère de DROITE s’allume quand le limiteur a dû retenir quelque chose, celui de GAUCHE quand BODY — la garde contre le son qui perce — agit. Les deux sont rares, et c’est voulu. L’affichage montre modules / câbles / CRÊTE de sortie · le bouton de langue fait EN→PT→FR→ES · RACK bascule la vue du rack : TOUS les modules ↔ seulement ceux qui atteignent la SORTIE · TUTORIEL cet écran · À PROPOS version et licence.",
    "VARIAR mano caótica (un DESLIZADOR: 0 = apagado) · ESPERA silencia el master · CAMBIA remuestrea ~25% · EVOLUCIONA lo mismo en 6 pasos · CRUZAR recombina con un patch nuevo · BANCO guarda el patch · GUARDA escribe la sesión · REC graba WAV · SEED sortea el patch, con su caja de número · el medidor del MASTER muestra el pico reciente frente al techo de −1 dBFS: la marca de la DERECHA se enciende cuando el limitador tuvo que retener algo, y la de la IZQUIERDA cuando BODY — la guarda contra el sonido que perfora — está actuando. Ambas son raras, y eso es lo esperado. La lectura muestra módulos / cables / PICO de salida · el botón de idioma cicla EN→PT→FR→ES · RACK alterna la vista del rack: TODOS los módulos ↔ solo los que llegan a la SALIDA · TUTORIAL esta pantalla · ACERCA DE versión y licencia."};

inline const L4 tutCableTitle {
    "PATCH CABLES", "CABEAR", "CÂBLER", "CABLEAR"};
inline const L4 tutCableBody {
    "Drag a cable from an output jack to an input jack. A double halo means the same signal type (audio or control). A SOLID halo with a lit jack means patching there sounds right away; a DASHED halo means the connection is valid but that path does not reach the sound yet — building away from the output and connecting it last is legitimate. If the cable you are dragging looks greyed, it is the SOURCE that is silent.\n\nCLICK ON A CABLE to open its box: a cable here is an OBJECT, not a wire. The cable under the pointer lights up — that is the one a click opens, and a click on a cable never moves a module. The box opens in the bottom-right corner, titled with both ends (module and port), and the cable stays lit while it is open.\n\nGAIN scales the signal (0 to 2, neutral in the middle). COND is the conductance — the chance of letting through at each moment, where the living intermittence comes from; the light in the title blinks with it.\n\nRELATION combines what crosses the cable with a second signal, the companion: NONE just lets it through, RING multiplies the two, FOLD folds the signal driven by the companion, DIFF subtracts the companion; AMT sets how much, and the companion's name is shown (click it to pick another output).\n\nA cable can also be BROKEN without being removed — the sound decays into a scar instead of cutting dead.\n\nUNPLUG, or a right-click on a jack, takes the cable out; in the RACK · OUTPUT view, whatever stops reaching the sound stays on screen with a dashed border.\n\n[space] cuts and restores every cable at once. While dragging a cable, move it near the top or bottom edge to scroll the rack.",
    "Puxe um cabo de um jack de saída até um de entrada. Halo duplo = mesmo tipo de sinal (áudio ou controle). Halo CONTÍNUO e jack aceso = ligar ali soa agora; halo TRACEJADO = a ligação vale, mas o caminho ainda não chega ao som — construir longe da saída e ligar ao som por último é legítimo. Se o cabo que você puxa sai acinzentado, a FONTE é que está muda no momento.\n\nCLIQUE SOBRE UM CABO para abrir a caixa dele: aqui o cabo é um OBJETO, não um fio. O cabo sob o ponteiro se acende — é ele que o clique abre, e clique sobre cabo nunca move módulo. A caixa abre no canto inferior direito, com as duas pontas no título (módulo e porta), e o cabo fica aceso enquanto ela está aberta.\n\nGAIN dosa o sinal (0 a 2, neutro no meio). COND é a condutância — a chance de deixar passar a cada instante, de onde vem a intermitência viva; a luz do título pisca com ela.\n\nA RELAÇÃO combina o que atravessa o cabo com um segundo sinal, o companion: NONE só deixa passar, RING multiplica os dois, FOLD dobra o sinal comandado pelo companion, DIFF subtrai o companion; AMT diz quanto, e o nome do companion aparece (clique nele para escolher outra saída).\n\nUm cabo também pode ser ROMPIDO sem ser removido — o som decai numa cicatriz em vez de cortar seco.\n\nDESPLUGAR, ou o botão direito num jack, tira o cabo; na vista RACK · SAÍDA, o que deixa de chegar ao som fica na tela com borda tracejada.\n\n[espaço] rompe e reata todos os cabos de uma vez. Enquanto arrasta um cabo, leve-o pra perto da borda de cima ou de baixo pra rolar o rack.",
    "Tirez un câble d’une sortie vers une entrée. Un double halo indique le même type de signal (audio ou contrôle). Halo CONTINU et jack allumé : câbler là sonne tout de suite ; halo TIRETÉ : la liaison est valide mais ce chemin n’atteint pas encore le son. Si le câble tiré paraît grisé, c’est la SOURCE qui est muette.\n\nCLIQUEZ SUR UN CÂBLE pour ouvrir sa fenêtre : ici le câble est un OBJET, pas un fil. Le câble sous le pointeur s’allume — c’est lui que le clic ouvre, et un clic sur un câble ne déplace jamais un module. La fenêtre s’ouvre dans le coin inférieur droit, avec les deux extrémités dans le titre (module et port), et le câble reste allumé tant qu’elle est ouverte.\n\nGAIN dose le signal (0 à 2, neutre au milieu). COND est la conductance — la chance de laisser passer à chaque instant, d’où vient l’intermittence vivante ; le voyant du titre clignote avec elle.\n\nLa RELATION combine ce qui traverse le câble avec un second signal, le compagnon : NONE laisse simplement passer, RING multiplie les deux, FOLD replie le signal piloté par le compagnon, DIFF soustrait le compagnon ; AMT dit combien, et le nom du compagnon s’affiche (cliquez dessus pour choisir une autre sortie).\n\nUn câble peut aussi être ROMPU sans être retiré — le son décroît en cicatrice au lieu de couper net.\n\nDÉBRANCHER, ou un clic droit sur un jack, retire le câble ; dans la vue RACK · SORTIE, ce qui cesse d’atteindre le son reste à l’écran avec une bordure tiretée.\n\n[espace] coupe et rétablit tous les câbles d’un coup. En tirant un câble, approchez-le du bord haut ou bas pour faire défiler le rack.",
    "Arrastre un cable de una salida a una entrada. Doble halo = mismo tipo de señal (audio o control). Halo CONTINUO y jack encendido: cablear ahí suena de inmediato; halo DISCONTINUO: la conexión es válida pero ese camino aún no llega al sonido. Si el cable que arrastra se ve grisáceo, es la FUENTE la que está muda.\n\nHAGA CLIC SOBRE UN CABLE para abrir su caja: aquí el cable es un OBJETO, no un hilo. El cable bajo el puntero se ilumina — es el que el clic abre, y un clic sobre un cable nunca mueve un módulo. La caja se abre en la esquina inferior derecha, con los dos extremos en el título (módulo y puerto), y el cable sigue iluminado mientras está abierta.\n\nGAIN dosifica la señal (0 a 2, neutro en el centro). COND es la conductancia — la probabilidad de dejar pasar en cada instante, de donde viene la intermitencia viva; la luz del título parpadea con ella.\n\nLa RELACIÓN combina lo que atraviesa el cable con una segunda señal, el compañero: NONE solo deja pasar, RING multiplica las dos, FOLD pliega la señal guiada por el compañero, DIFF resta el compañero; AMT dice cuánto, y el nombre del compañero aparece (haga clic en él para elegir otra salida).\n\nUn cable también puede ser ROTO sin ser retirado — el sonido decae en una cicatriz en vez de cortarse seco.\n\nDESENCHUFAR, o un clic derecho en un jack, quita el cable; en la vista RACK · SALIDA, lo que deja de llegar al sonido queda en pantalla con borde discontinuo.\n\n[espacio] corta y restablece todos los cables a la vez. Mientras arrastra un cable, llévelo cerca del borde superior o inferior para desplazar el rack."};

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
    "The left column is the palette — the full catalogue, grouped by family. Drag a name from it into the rack to add that module. Drag a module by its body — from a spot with no cable over it — to reposition it; it only moves after a short drag, so a click never shifts it. To remove one: click the [x] at its top-right corner, or drag it back onto the palette. The rack holds one of every module. The instrument does not open silent: it draws a seed and plays at once — see BUILD FROM SCRATCH for the other way in.",
    "A coluna da esquerda é a paleta — o catálogo inteiro, agrupado por família. Arraste um nome dela pra o rack pra adicionar aquele módulo. Arraste um módulo pelo corpo — num ponto onde não passe cabo — pra reposicionar; ele só se move depois de um arrasto curto, então um clique nunca o desloca. Pra remover: clique no [x] no canto superior direito, ou arraste o módulo de volta pra paleta. O rack tem um de cada módulo. O instrumento não abre mudo: ele sorteia um seed e já toca — veja CONSTRUIR DO ZERO pro outro caminho.",
    "La colonne de gauche est la palette — le catalogue complet, groupé par famille. Glissez-en un nom dans le rack pour ajouter ce module. Glissez un module par son corps — à un endroit sans câble dessus — pour le repositionner ; il ne bouge qu’après un court glissement, donc un clic ne le déplace jamais. Pour en retirer un : cliquez le [x] en haut à droite, ou glissez le module vers la palette. Le rack contient un de chaque module. L’instrument ne s’ouvre pas muet : il tire un seed et joue aussitôt — voir CONSTRUIRE DE ZÉRO pour l’autre voie.",
    "La columna izquierda es la paleta — el catálogo completo, agrupado por familia. Arrastre un nombre de ella al rack para añadir ese módulo. Arrastre un módulo por su cuerpo — desde un punto sin cable encima — para reubicarlo; solo se mueve tras un arrastre corto, así que un clic nunca lo desplaza. Para quitar uno: clic en la [x] de su esquina superior derecha, o arrástrelo de vuelta a la paleta. El rack tiene uno de cada módulo. El instrumento no abre mudo: sortea un seed y suena de inmediato — vea CONSTRUIR DESDE CERO para la otra vía."};

inline const L4 tutFamTitle {
    "THE EIGHT FAMILIES", "AS OITO FAMÍLIAS", "LES HUIT FAMILLES", "LAS OCHO FAMILIAS"};
inline const L4 tutFamBody {
    "SOURCE makes sound from nothing (oscillators, physical models, noise).\nTRANSFORM changes a signal passing through it — audio or control (filters, VCAs, waveshapers).\nMODULATE generates a control signal (envelopes, LFOs, chaos, sample-and-hold).\nTIME keeps time (clock, clock logic, sequencers).\nDECISION picks a value (quantise, harmonise, count).\nROUTE sends signals around (switch, matrix, multiple, morph).\nSPACE places and remembers sound (delay, reverb, granular, looper).\nOUT mixes, meters and sends (mixer, master, scope, note-out).\n\nEach module carries its family colour as a thin stripe along its top edge; the square beside each family name in the palette is the legend.",
    "SOURCE faz som do nada (osciladores, modelos físicos, ruído).\nTRANSFORM muda um sinal que passa por ele — áudio ou controle (filtros, VCAs, modeladores de onda).\nMODULATE gera um sinal de controle (envelopes, LFOs, caos, sample-and-hold).\nTIME marca o tempo (clock, lógica de clock, sequenciadores).\nDECISION escolhe um valor (quantiza, harmoniza, conta).\nROUTE encaminha sinais (chave, matriz, múltiplo, morph).\nSPACE espacializa e lembra o som (delay, reverb, granular, looper).\nOUT mistura, mede e envia (mixer, master, scope, note-out).\n\nCada módulo leva a cor da sua família numa faixa fina no topo; o quadradinho ao lado do nome de cada família, na paleta, é a legenda.",
    "SOURCE crée du son à partir de rien (oscillateurs, modèles physiques, bruit).\nTRANSFORM modifie un signal qui la traverse — audio ou contrôle (filtres, VCA, waveshapers).\nMODULATE génère un signal de contrôle (enveloppes, LFO, chaos, sample-and-hold).\nTIME marque le temps (horloge, logique d’horloge, séquenceurs).\nDECISION choisit une valeur (quantifier, harmoniser, compter).\nROUTE achemine les signaux (commutateur, matrice, multiple, morph).\nSPACE spatialise et mémorise le son (delay, réverbe, granulaire, looper).\nOUT mixe, mesure et envoie (mixeur, master, scope, note-out).\n\nChaque module porte la couleur de sa famille en une fine bande sur son bord supérieur ; le petit carré à côté de chaque nom de famille, dans la palette, en est la légende.",
    "SOURCE hace sonido de la nada (osciladores, modelos físicos, ruido).\nTRANSFORM cambia una señal que pasa por él — audio o control (filtros, VCA, modeladores de onda).\nMODULATE genera una señal de control (envolventes, LFO, caos, sample-and-hold).\nTIME marca el tiempo (reloj, lógica de reloj, secuenciadores).\nDECISION elige un valor (cuantizar, armonizar, contar).\nROUTE encamina señales (conmutador, matriz, múltiple, morph).\nSPACE espacializa y recuerda el sonido (delay, reverb, granular, looper).\nOUT mezcla, mide y envía (mezclador, master, scope, note-out).\n\nCada módulo lleva el color de su familia en una franja fina en su borde superior; el cuadradito junto al nombre de cada familia, en la paleta, es la leyenda."};

// Cartão de ATALHOS. Os atalhos estavam espalhados pelos outros cartões
// (seed, REC, navegação) e as teclas de uma letra — g, v, m, e, c — não
// apareciam em lugar nenhum: quem só lesse o tutorial não descobria que
// existiam. Reunidos aqui, sem tirar as menções em contexto.
// Os dois caminhos de uso. Por omissão o instrumento já APRESENTA um
// patch, mas construir o patch à mão é caminho de primeira classe, não
// gambiarra —
// e faltava dizer isso em algum lugar (o autor perguntou se a opção
// existia; existia só como dezenas de cliques, por isso veio o [n]).
inline const L4 tutScratchTitle {
    "BUILD FROM SCRATCH", "CONSTRUIR DO ZERO",
    "CONSTRUIRE DE ZÉRO", "CONSTRUIR DESDE CERO"};
inline const L4 tutScratchBody {
    "There are two ways in. One: press SEED and steer what comes out. Two: press n to pull every cable at once — the modules stay, the sound stops — and build the piece connection by connection. The rack is the set of modules available; the patch is the cabling. While building, keep RACK on ALL so you can see everything; switch it to OUTPUT to show only the modules that actually reach the sound, which is also how you thin a crowded rack without removing anything. Ctrl+Z steps back one action, cable by cable.",
    "Há dois caminhos. Um: aperte SEED e conduza o que sair. Dois: aperte n pra tirar todos os cabos de uma vez — os módulos ficam, o som para — e construa a peça ligação por ligação. O rack é o conjunto de módulos disponíveis; o patch é o cabeamento. Enquanto constrói, deixe RACK em TODOS pra enxergar tudo; passe pra SAÍDA pra ver só os módulos que de fato chegam ao som, que é também como enxugar um rack cheio sem remover nada. Ctrl+Z volta uma ação, cabo a cabo.",
    "Il y a deux voies. Une : appuyez sur SEED et dirigez ce qui sort. Deux : appuyez sur n pour retirer tous les câbles d’un coup — les modules restent, le son s’arrête — et construisez la pièce liaison par liaison. Le rack est l’ensemble des modules disponibles ; le patch est le câblage. Pendant la construction, gardez RACK sur TOUS ; passez à SORTIE pour ne voir que les modules qui atteignent le son, ce qui allège aussi un rack chargé sans rien retirer. Ctrl+Z revient d’une action, câble par câble.",
    "Hay dos vías. Una: pulse SEED y guíe lo que salga. Dos: pulse n para quitar todos los cables de una vez — los módulos quedan, el sonido para — y construya la pieza conexión por conexión. El rack es el conjunto de módulos disponibles; el patch es el cableado. Mientras construye, deje RACK en TODOS; pase a SALIDA para ver solo los módulos que llegan al sonido, que es también cómo aligerar un rack cargado sin quitar nada. Ctrl+Z retrocede una acción, cable a cable."};

inline const L4 tutKeysTitle {
    "KEYBOARD", "TECLADO", "CLAVIER", "TECLADO"};
inline const L4 tutKeysBody {
    "g new seed  ·  v live variation on/off  ·  m mutate  ·  e evolve (six small steps)  ·  c cross with a fresh donor patch  ·  space breaks and restores every cable at once  ·  Ctrl+S save the session  ·  Ctrl+B file the patch in the bank  ·  Ctrl+R record  ·  Ctrl+Z undo (one action back, cable by cable)  ·  Ctrl+O open a patch  ·  Esc cancel the gesture in progress  ·  n pull every cable  ·  r restore the current seed (undo every hand edit)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  arrows scroll the rack  ·  q quit. Every shortcut that has a button lights that button for a moment, so you can see what you just did. Inside TUTORIAL and ABOUT: arrows, Page Up/Down, Home/End scroll, Esc closes.",
    "g sorteia um seed  ·  v liga/desliga a variação ao vivo  ·  m muda  ·  e evolui (seis passos pequenos)  ·  c cruza com um doador novo  ·  [espaço] rompe e reata todos os cabos de uma vez  ·  Ctrl+S salva a sessão  ·  Ctrl+B arquiva o patch no banco  ·  Ctrl+R grava  ·  Ctrl+Z desfaz (uma ação, cabo a cabo)  ·  Ctrl+O abre um patch  ·  Esc cancela o gesto em curso  ·  n tira todos os cabos  ·  r repõe o seed atual (desfaz toda edição à mão)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  setas rolam o rack  ·  q sai. Todo atalho que tem botão ACENDE esse botão por um instante — dá pra ver o que você acabou de fazer. Dentro de TUTORIAL e SOBRE: setas, Page Up/Down, Home/End rolam, Esc fecha.",
    "g nouveau seed  ·  v variation en direct on/off  ·  m muter  ·  e évoluer (six petits pas)  ·  c croiser avec un donneur neuf  ·  espace rompt et rétablit tous les câbles d’un coup  ·  Ctrl+S enregistre la session  ·  Ctrl+B classe le patch dans la banque  ·  Ctrl+R enregistre  ·  Ctrl+Z annule (une action, câble par câble)  ·  Ctrl+O ouvre un patch  ·  Échap annule le geste en cours  ·  n retire tous les câbles  ·  r rétablit le seed courant (annule toute édition manuelle)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  flèches font défiler le rack  ·  q quitte. Chaque raccourci qui a un bouton ALLUME ce bouton un instant. Dans TUTORIEL et À PROPOS : flèches, Page Haut/Bas, Origine/Fin défilent, Échap ferme.",
    "g sortea un seed  ·  v activa/desactiva la variación en vivo  ·  m muta  ·  e evoluciona (seis pasos pequeños)  ·  c cruza con un donante nuevo  ·  espacio rompe y restablece todos los cables a la vez  ·  Ctrl+S guarda la sesión  ·  Ctrl+B archiva el patch en el banco  ·  Ctrl+R graba  ·  Ctrl+Z deshace (una acción, cable a cable)  ·  Ctrl+O abre un patch  ·  Esc cancela el gesto en curso  ·  n quita todos los cables  ·  r repone el seed actual (deshace toda edición manual)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  flechas desplazan el rack  ·  q sale. Cada atajo que tiene botón ENCIENDE ese botón un instante. Dentro de TUTORIAL y ACERCA DE: flechas, Re/Av Pág, Inicio/Fin desplazan, Esc cierra."};

inline const L4 tutLearnTitle {
    "LEARN — EVERY MODULE, EVERY KNOB", "LEARN — CADA MÓDULO, CADA KNOB", "LEARN — CHAQUE MODULE, CHAQUE POT.", "LEARN — CADA MÓDULO, CADA KNOB"};
inline const L4 tutLearnBody {
    "Hover a module's body and the box in the lower-left corner tells you what that module is for. Hover any knob or jack and it explains that control in that module, on three levels — quick, how it works, and one thing to try. A fuller written guide and patch recipes are coming as a companion site and PDF.",
    "Passe o mouse sobre o corpo de um módulo e a caixa no canto inferior esquerdo diz pra que aquele módulo serve. Passe sobre qualquer knob ou jack e ela explica aquele controle naquele módulo, em três níveis — rápido, como funciona, e um experimento. Um guia escrito mais completo e receitas de patch estão a caminho, como site e PDF.",
    "Survolez le corps d’un module : la boîte en bas à gauche dit à quoi sert ce module. Survolez un potentiomètre ou un jack : elle explique ce contrôle dans ce module, sur trois niveaux — rapide, comment ça marche, et une chose à essayer. Un guide écrit plus complet et des recettes de patch arrivent, sous forme de site et de PDF.",
    "Pase el ratón sobre el cuerpo de un módulo: la caja de la esquina inferior izquierda dice para qué sirve ese módulo. Pase sobre un knob o jack: explica ese control en ese módulo, en tres niveles — rápido, cómo funciona, y algo para probar. Una guía escrita más completa y recetas de patch están en camino, como sitio y PDF."};

// Caixa LEARN sem nada sob o mouse. Existia como literal fixo em
// português dentro do `panel_main.cpp` — os outros três idiomas viam uma
// frase em português. Trazida pra cá em 2026-09-14, quando o app JUCE
// precisou da mesma dica.
inline const L4 learnIdle {
    "hover a knob or jack of a module in the rack.",
    "passe o mouse sobre um knob ou jack de um módulo do rack.",
    "survolez un potentiomètre ou un jack d’un module du rack.",
    "pase el ratón sobre un knob o jack de un módulo del rack."};

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
