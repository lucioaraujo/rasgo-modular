#pragma once

#include "panel/LearnEntry.hpp"
#include "panel/UiLanguage.hpp"
#include <array>

namespace rasgo::panel {
enum class HeaderAction { seed, lang, rackView, standby, zoomIn, zoomOut,
    vary, mutate, evolve, cross, bank, save, tutorial, about, rec, open,
    undo, uncable, restore, startup, seedValue, varyAmount };

struct HeaderHelpRow {
    HeaderAction action;
    const char* key;
    L4 title;
    std::array<LearnEntry, 4> entries; // en, pt, fr, es
};

inline const std::array<HeaderHelpRow, 22>& headerHelpRows() {
    const auto row = [](HeaderAction action, const char* key, L4 title, L4 text) {
        return HeaderHelpRow{action, key, title, {{{text.en, "", ""}, {text.pt, "", ""},
                                                  {text.fr, "", ""}, {text.es, "", ""}}}};
    };
    using A = HeaderAction;
    namespace S = strings;
    static const std::array<HeaderHelpRow, 22> rows{{
        row(A::seed, "seed", S::hdrSeed, {
            "Draw a new seed and generate a new patch. Save the current patch first if you want to keep it.",
            "Sorteia outra seed e gera outro patch. Salve o patch atual antes se quiser guardá-lo.",
            "Tire une nouvelle graine et génère un autre patch. Enregistrez d'abord le patch actuel pour le conserver.",
            "Sortea otra semilla y genera otro patch. Guarde primero el patch actual si desea conservarlo."}),
        row(A::restore, "restore", S::hdrRestore, {
            "Rebuild the original patch from the current seed, replacing manual edits. UNDO goes back one step instead.",
            "Reconstrói o patch original da seed atual, substituindo as edições manuais. DESFAZ volta somente um passo.",
            "Reconstruit le patch initial de la graine actuelle et remplace les modifications manuelles. ANNULER revient d'un seul pas.",
            "Reconstruye el patch original de la semilla actual, reemplazando cambios manuales. DESHACER retrocede un solo paso."}),
        row(A::seedValue, "seed-value", {"SEED number", "Número da SEED", "Numéro SEED", "Número SEED"}, {
            "Type or paste up to 20 digits. Enter generates that seed; leaving without confirming restores the current number. A seed does not store your manual edits: save a patch for those.",
            "Digite ou cole até 20 dígitos. Enter gera essa seed; sair sem confirmar repõe o número atual. A seed não guarda suas edições manuais: para isso, salve o patch.",
            "Saisissez ou collez jusqu'à 20 chiffres. Entrée génère cette graine ; quitter sans confirmer rétablit le numéro actuel. Pour conserver vos modifications manuelles, enregistrez le patch.",
            "Escriba o pegue hasta 20 dígitos. Enter genera esa semilla; salir sin confirmar repone el número actual. Para conservar cambios manuales, guarde el patch."}),
        row(A::vary, "vary", S::hdrVary, {
            "Enable or pause continuous parameter movement. Cables and your own gestures still act. MIXER, MASTER, external input and LISTEN calibration are left to you.",
            "Liga ou pausa o movimento contínuo dos parâmetros. Cabos e seus gestos continuam atuando. MIXER, MASTER, entrada externa e calibração do LISTEN ficam por sua conta.",
            "Active ou suspend le mouvement continu des paramètres. Câbles et gestes manuels continuent d'agir. MIXER, MASTER, entrée externe et calibration LISTEN restent à votre main.",
            "Activa o pausa el movimiento continuo de parámetros. Cables y gestos manuales siguen actuando. MIXER, MASTER, entrada externa y calibración LISTEN quedan a su cargo."}),
        row(A::varyAmount, "vary-amount", {"VARY intensity", "Intensidade VARIA", "Intensité VARIER", "Intensidad VARIAR"}, {
            "Set movement intensity from 0 to 2. Zero freezes automatic movement; 1 is the usual range; 2 allows wider gestures. This is not a volume control.",
            "Ajusta a intensidade do movimento de 0 a 2. Zero congela o movimento automático; 1 é a faixa habitual; 2 permite gestos mais amplos. Não é controle de volume.",
            "Règle l'intensité du mouvement de 0 à 2. Zéro fige le mouvement automatique ; 1 est la plage habituelle ; 2 permet des gestes plus larges. Ce n'est pas le volume.",
            "Ajusta la intensidad del movimiento de 0 a 2. Cero congela el movimiento automático; 1 es el rango habitual; 2 permite gestos más amplios. No controla el volumen."}),
        row(A::mutate, "mutate", S::hdrChange, {
            "Change part of the connected patch's parameters once. MIXER and MASTER are protected. Compare the result and use UNDO if needed.",
            "Muda uma parte dos parâmetros do patch conectado, uma vez. MIXER e MASTER são preservados. Compare o resultado; DESFAZ permite voltar.",
            "Change une partie des paramètres du patch connecté, une fois. MIXER et MASTER sont préservés. Comparez puis utilisez ANNULER si besoin.",
            "Cambia una parte de los parámetros del patch conectado, una vez. MIXER y MASTER se conservan. Compare y use DESHACER si hace falta."}),
        row(A::evolve, "evolve", S::hdrEvolve, {
            "Apply six smaller parameter mutations in one operation. The result is immediate; this is not a timed fade between patches.",
            "Aplica seis mutações menores de parâmetros em uma operação. O resultado é imediato; não é uma transição temporizada entre patches.",
            "Applique six petites mutations de paramètres en une opération. Le résultat est immédiat, sans fondu temporisé entre patches.",
            "Aplica seis mutaciones menores de parámetros en una operación. El resultado es inmediato, sin transición temporizada entre patches."}),
        row(A::cross, "cross", S::hdrCross, {
            "Cross parameters with a donor patch generated from another seed. This does not add the new optional modules to the initial seed rack.",
            "Cruza parâmetros com um patch doador gerado por outra seed. Isso não acrescenta os módulos opcionais novos ao rack inicial da seed.",
            "Croise les paramètres avec un patch donneur issu d'une autre graine. Cela n'ajoute pas les nouveaux modules optionnels au rack initial.",
            "Cruza parámetros con un patch donante de otra semilla. No añade los módulos opcionales nuevos al rack inicial."}),
        row(A::bank, "bank", S::hdrBank, {
            "Store the patch in the local patches folder. Its name uses the seed, or the time if there is no seed. Saving the same seed again replaces its file. Use OPEN to load it.",
            "Guarda o patch na pasta local patches. O nome usa a seed ou, sem seed, o horário. Guardar a mesma seed novamente substitui seu arquivo. Use ABRIR para carregar.",
            "Conserve le patch dans le dossier local patches. Son nom reprend la graine ou, sans graine, l'heure. Enregistrer à nouveau la même graine remplace son fichier. OUVRIR le recharge.",
            "Guarda el patch en la carpeta local patches. Su nombre usa la semilla o, sin semilla, la hora. Guardar otra vez la misma semilla reemplaza su archivo. ABRIR lo carga."}),
        row(A::save, "save", S::hdrSave, {
            "Save the current patch: modules, parameters and connections. Use REC for an audio recording.",
            "Salva o patch atual: módulos, parâmetros e conexões. Para gravar áudio, use REC.",
            "Enregistre le patch actuel : modules, paramètres et connexions. Pour enregistrer l'audio, utilisez REC.",
            "Guarda el patch actual: módulos, parámetros y conexiones. Para grabar audio, use REC."}),
        row(A::open, "open", S::hdrOpen, {
            "Open a saved .rmp patch, replacing the current rack. Save your current patch first if you want to keep it.",
            "Abre um patch .rmp salvo, substituindo o rack atual. Salve antes o patch atual se quiser guardá-lo.",
            "Ouvre un patch .rmp enregistré et remplace le rack actuel. Enregistrez d'abord le patch actuel pour le conserver.",
            "Abre un patch .rmp guardado y reemplaza el rack actual. Guarde primero el patch actual si quiere conservarlo."}),
        row(A::undo, "undo", S::hdrUndo, {
            "Return to the preceding stored editing step. This is different from rebuilding the entire patch from its seed.",
            "Volta à etapa de edição anterior guardada pelo instrumento. É diferente de reconstruir o patch inteiro pela seed.",
            "Revient à l'étape d'édition précédente mémorisée. C'est différent de reconstruire tout le patch depuis sa graine.",
            "Vuelve al paso de edición anterior guardado. Es distinto de reconstruir todo el patch desde su semilla."}),
        row(A::uncable, "uncable", S::hdrUncable, {
            "Remove every cable while keeping the modules. Rebuild the connections yourself; save first or use UNDO to recover the previous patch.",
            "Remove todos os cabos e mantém os módulos. Você pode refazer as conexões; salve antes ou use DESFAZ para recuperar o patch anterior.",
            "Retire tous les câbles en gardant les modules. Refaites vos connexions ; enregistrez avant ou utilisez ANNULER pour revenir.",
            "Retira todos los cables y conserva los módulos. Reconstruya las conexiones; guarde antes o use DESHACER para volver."}),
        row(A::standby, "standby", S::hdrStandby, {
            "Mute the MASTER output without removing cables. Press again to hear the patch. The patch's internal processes continue.",
            "Silencia a saída MASTER sem remover cabos. Aperte novamente para ouvir. Os processos internos do patch continuam.",
            "Coupe la sortie MASTER sans retirer les câbles. Appuyez de nouveau pour écouter. Les processus internes continuent.",
            "Silencia la salida MASTER sin retirar cables. Pulse otra vez para escuchar. Los procesos internos continúan."}),
        row(A::zoomIn, "zoom-in", {"Zoom +", "Zoom +", "Zoom +", "Zoom +"}, {
            "Enlarge the rack modules. Scroll through the rack when the enlarged panels no longer fit the window.",
            "Aumenta os módulos do rack. Use a rolagem quando os painéis ampliados deixarem de caber na janela.",
            "Agrandit les modules du rack. Faites défiler le rack si les panneaux ne tiennent plus dans la fenêtre.",
            "Amplía los módulos del rack. Desplácese por el rack cuando los paneles ya no quepan en la ventana."}),
        row(A::zoomOut, "zoom-out", {"Zoom −", "Zoom −", "Zoom −", "Zoom −"}, {
            "Reduce the rack modules to see more at once. F2 changes LEARN reading size independently.",
            "Diminui os módulos para ver mais de uma vez. F2 muda o tamanho de leitura do LEARN separadamente.",
            "Réduit les modules pour en voir davantage. F2 règle séparément la taille de lecture du LEARN.",
            "Reduce los módulos para ver más a la vez. F2 cambia por separado el tamaño de lectura del LEARN."}),
        row(A::rackView, "rack-view", {"RACK", "RACK", "RACK", "RACK"}, {
            "Switch between all modules in the current patch and those reaching audio output. This does not load the entire catalog; add other modules from the palette. Hidden modules are not deleted.",
            "Alterna entre todos os módulos do patch atual e os que chegam à saída de áudio. Não carrega o catálogo inteiro; acrescente outros pela paleta. Os ocultos não são apagados.",
            "Alterner entre tous les modules du patch actuel et ceux reliés à la sortie audio. Le catalogue entier n'est pas chargé ; ajoutez des modules depuis la palette. Rien n'est supprimé.",
            "Alterna entre todos los módulos del patch actual y los que llegan a la salida de audio. No carga el catálogo entero; añada otros desde la paleta. Los ocultos no se borran."}),
        row(A::rec, "rec", S::hdrRec, {
            "Start or stop recording the stereo output to WAV. Patch saving and audio recording are separate actions; watch the recording indicator.",
            "Inicia ou termina a gravação da saída estéreo em WAV. Salvar patch e gravar áudio são ações diferentes; acompanhe o indicador de gravação.",
            "Démarre ou arrête l'enregistrement stéréo en WAV. Sauvegarde du patch et enregistrement audio sont deux actions distinctes ; surveillez l'indicateur.",
            "Inicia o termina la grabación estéreo en WAV. Guardar el patch y grabar audio son acciones distintas; observe el indicador de grabación."}),
        row(A::startup, "startup", {"START", "ABRE", "DÉPART", "INICIO"}, {
            "Choose how the next launch opens: a new seed, your initial patch, or the last session. Choosing the initial-patch option stores the current patch as that starting point.",
            "Escolhe como será a próxima abertura: seed nova, seu patch inicial ou última sessão. Escolher o patch inicial guarda o patch atual como esse ponto de partida.",
            "Choisit la prochaine ouverture : nouvelle graine, patch initial ou dernière session. Choisir le patch initial mémorise le patch actuel comme point de départ.",
            "Elige la próxima apertura: semilla nueva, patch inicial o última sesión. Elegir el patch inicial guarda el actual como punto de partida."}),
        row(A::lang, "language", {"Language", "Idioma", "Langue", "Idioma"}, {
            "Cycle EN, PT, FR and ES. Interface explanations follow the chosen language; technical module and control names stay the same.",
            "Alterna EN, PT, FR e ES. As explicações acompanham o idioma escolhido; nomes técnicos dos módulos e controles permanecem iguais.",
            "Parcourt EN, PT, FR et ES. Les explications suivent la langue choisie ; les noms techniques des modules et commandes restent identiques.",
            "Alterna EN, PT, FR y ES. Las explicaciones siguen el idioma elegido; los nombres técnicos de módulos y controles no cambian."}),
        row(A::tutorial, "tutorial", S::hdrTutorial, {
            "Open the built-in tutorial. Use arrows or Page Up/Down to read; Esc returns to the rack. Audio continues while you read.",
            "Abre o tutorial do instrumento. Setas ou Page Up/Down percorrem o texto; Esc retorna ao rack. O áudio continua durante a leitura.",
            "Ouvre le tutoriel intégré. Flèches ou Page Up/Down parcourent le texte ; Échap revient au rack. L'audio continue pendant la lecture.",
            "Abre el tutorial integrado. Flechas o Page Up/Down recorren el texto; Esc vuelve al rack. El audio continúa durante la lectura."}),
        row(A::about, "about", S::hdrAbout, {
            "Open version, credits, licence and live technical readings. Those readings help describe a problem when reporting it.",
            "Abre versão, créditos, licença e leituras técnicas ao vivo. Essas leituras ajudam a descrever um problema ao relatá-lo.",
            "Affiche version, crédits, licence et mesures techniques en direct. Ces mesures aident à décrire un problème.",
            "Muestra versión, créditos, licencia y lecturas técnicas en directo. Estas lecturas ayudan a describir un problema."})
    }};
    return rows;
}

inline const HeaderHelpRow* headerHelp(HeaderAction action) {
    for (const auto& row : headerHelpRows()) if (row.action == action) return &row;
    return nullptr;
}
} // namespace rasgo::panel
