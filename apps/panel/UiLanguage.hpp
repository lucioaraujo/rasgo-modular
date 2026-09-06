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

// ---- tutorial ---------------------------------------------------------
inline const L4 tutTitle {
    "HOW TO USE", "COMO USAR", "MODE D’EMPLOI", "CÓMO USAR"};
inline const L4 tutSubtitle {
    "a generative modular — it sounds on its own; you steer it",
    "um modular generativo — soa sozinho; você conduz",
    "un modulaire génératif — il sonne seul ; vous le dirigez",
    "un modular generativo — suena solo; usted lo guía"};

inline const L4 tutCableTitle {
    "1  PATCH", "1  CABEAR", "1  CÂBLER", "1  CABLEAR"};
inline const L4 tutCableBody {
    "Drag a cable from an output jack to an input jack. A double halo means the same signal type (audio or control). Right-click a jack to remove its cable. [space] cuts and restores every cable at once.",
    "Puxe um cabo de um jack de saída até um de entrada. Halo duplo = mesmo tipo de sinal (áudio ou controle). Botão direito num jack tira o cabo. [espaço] rompe e reata todos os cabos de uma vez.",
    "Tirez un câble d’une sortie vers une entrée. Un double halo indique le même type de signal (audio ou contrôle). Clic droit sur un jack pour retirer son câble. [espace] coupe et rétablit tous les câbles d’un coup.",
    "Arrastre un cable de una salida a una entrada. Doble halo = mismo tipo de señal (audio o control). Clic derecho en un jack para quitar su cable. [espacio] corta y restablece todos los cables a la vez."};

inline const L4 tutSeedTitle {
    "2  SEED", "2  SEED", "2  SEED", "2  SEED"};
inline const L4 tutSeedBody {
    "The SEED button draws a fresh patch and timbre and plays it right away — the instrument sounds with no input. The number identifies the patch; the same number reproduces the same sound.",
    "O botão SEED sorteia um cabeamento e timbre novos e já toca — o instrumento soa sem nenhuma entrada. O número identifica o patch; o mesmo número reproduz o mesmo som.",
    "Le bouton SEED tire un nouveau câblage et timbre et le joue aussitôt — l’instrument sonne sans aucune entrée. Le numéro identifie le patch ; le même numéro reproduit le même son.",
    "El botón SEED sortea un cableado y timbre nuevos y suena de inmediato — el instrumento suena sin ninguna entrada. El número identifica el patch; el mismo número reproduce el mismo sonido."};

inline const L4 tutVaryTitle {
    "3  VARY THE PATCH", "3  VARIAR O PATCH", "3  VARIER LE PATCH", "3  VARIAR EL PATCH"};
inline const L4 tutVaryBody {
    "DRIFT slowly animates the knobs. MUTATE resamples ~25% of the parameters at once; EVOLVE does it in six small steps; CROSS recombines with a fresh patch. BANK stores the current patch; SAVE writes the session.",
    "DERIVA anima os knobs devagar. MUTA reamostra ~25% dos parâmetros de uma vez; EVOLUI faz isso em seis passos pequenos; CRUZA recombina com um patch novo. BANCO guarda o patch atual; SALVA grava a sessão.",
    "DÉRIVE anime lentement les potentiomètres. MUTER rééchantillonne ~25% des paramètres d’un coup ; ÉVOLUE le fait en six petits pas ; CROISER recombine avec un patch neuf. BANQUE stocke le patch courant ; ENREG. écrit la session.",
    "DERIVA anima los knobs despacio. MUTAR remuestrea ~25% de los parámetros de una vez; EVOLUCIONA lo hace en seis pasos pequeños; CRUZAR recombina con un patch nuevo. BANCO guarda el patch actual; GUARDA escribe la sesión."};

inline const L4 tutMoveTitle {
    "4  MOVE MODULES", "4  MOVER MÓDULOS", "4  DÉPLACER LES MODULES", "4  MOVER MÓDULOS"};
inline const L4 tutMoveBody {
    "Drag a module to reposition it. The left column is the palette: drag a module from it into the rack to add one; drop a rack module back onto the palette to remove it.",
    "Arraste um módulo pra reposicionar. A coluna da esquerda é a paleta: arraste um módulo dela pra o rack pra adicionar; solte um módulo do rack sobre a paleta pra remover.",
    "Faites glisser un module pour le repositionner. La colonne de gauche est la palette : glissez-en un module vers le rack pour l’ajouter ; déposez un module du rack sur la palette pour le retirer.",
    "Arrastre un módulo para reubicarlo. La columna izquierda es la paleta: arrastre un módulo de ella al rack para añadirlo; suelte un módulo del rack sobre la paleta para quitarlo."};

inline const L4 tutZoomTitle {
    "5  ZOOM", "5  ZOOM", "5  ZOOM", "5  ZOOM"};
inline const L4 tutZoomBody {
    "Ctrl+= and Ctrl+- zoom the rack in and out; Ctrl+0 returns to 100%. Zoom out to see the first and last row together and patch a cable between them.",
    "Ctrl+= e Ctrl+- ampliam e reduzem o rack; Ctrl+0 volta a 100%. Reduza pra ver a primeira e a última fileira juntas e cabear entre elas.",
    "Ctrl+= et Ctrl+- agrandissent et réduisent le rack ; Ctrl+0 revient à 100%. Réduisez pour voir la première et la dernière rangée ensemble et câbler entre elles.",
    "Ctrl+= y Ctrl+- amplían y reducen el rack; Ctrl+0 vuelve al 100%. Reduzca para ver la primera y la última fila juntas y cablear entre ellas."};

inline const L4 tutLearnTitle {
    "6  LEARN", "6  APRENDER", "6  APPRENDRE", "6  APRENDER"};
inline const L4 tutLearnBody {
    "Hover any knob or jack: the box in the lower-left corner explains what that control does in that module, on three levels — quick, how it works, and one thing to try.",
    "Passe o mouse sobre qualquer knob ou jack: a caixa no canto inferior esquerdo explica o que aquele controle faz naquele módulo, em três níveis — rápido, como funciona, e um experimento.",
    "Survolez n’importe quel potentiomètre ou jack : la boîte en bas à gauche explique ce que fait ce contrôle dans ce module, sur trois niveaux — rapide, comment ça marche, et une chose à essayer.",
    "Pase el ratón sobre cualquier knob o jack: la caja de la esquina inferior izquierda explica qué hace ese control en ese módulo, en tres niveles — rápido, cómo funciona, y algo para probar."};

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
