// Os textos que só a página tem (25/11/2026, F4b do rack no navegador). Os
// textos do app — cabeçalho, avisos, TUTORIAL, SOBRE, banco de escalas,
// inspetor — vêm do próprio motor (`rw_textos`, de apps/panel/UiLanguage.hpp),
// para não haver duas cópias. Aqui ficam:
//   - o que o app não tem (TOCAR, VOLUME, o navegador guardando a sessão…);
//   - os cartões do TUTORIAL que no navegador são outros: o texto do app com
//     o que não vale aqui trocado (pastas do disco, terminal, `q` para sair,
//     botão do meio), nas mesmas quatro línguas.
// Ordem das línguas: en, pt, fr, es (`Lang` do app).

export const LINGUAS = ['en', 'pt', 'fr', 'es'];
export const ROTULO_LINGUA = ['EN', 'PT', 'FR', 'ES'];

const T = {
  tocar: ['PLAY', 'TOCAR', 'JOUER', 'TOCAR'],
  volume: ['VOLUME', 'VOLUME', 'VOLUME', 'VOLUMEN'],
  compacto: ['COMPACT', 'COMPACTO', 'COMPACT', 'COMPACTO'],
  pronto: ['Click anywhere (or PLAY) to hear it — the browser only lets it sound after a click.',
           'Clique em qualquer lugar (ou em TOCAR) para ouvir — o navegador só deixa soar depois de um clique.',
           'Cliquez n’importe où (ou sur JOUER) pour l’entendre — le navigateur ne laisse sonner qu’après un clic.',
           'Haga clic en cualquier lugar (o en TOCAR) para oírlo — el navegador solo deja sonar después de un clic.'],
  montando: ['Building the patch…', 'Montando o patch…', 'Montage du patch…', 'Montando el patch…'],
  semWasm: ['Could not load rasgo.wasm (open it through an http server).',
            'Não consegui carregar rasgo.wasm (abra por um servidor http).',
            'Impossible de charger rasgo.wasm (ouvrez-le via un serveur http).',
            'No pude cargar rasgo.wasm (ábralo desde un servidor http).'],
  salvoAqui: ['Session saved in this browser.', 'Sessão salva neste navegador.',
              'Session enregistrée dans ce navigateur.', 'Sesión guardada en este navegador.'],
  noBanco: ['Patch filed in the bank: {nome} (in your downloads folder).',
            'Patch no banco: {nome} (na pasta de downloads).',
            'Patch classé dans la banque : {nome} (dans le dossier des téléchargements).',
            'Patch en el banco: {nome} (en la carpeta de descargas).'],
  nadaDesfazer: ['Nothing to undo.', 'Nada para desfazer.', 'Rien à annuler.', 'Nada que deshacer.'],
  abrirFalhou: ['Could not open that file: it does not look like a RASGO Modular patch (.rmp).',
                'Não consegui abrir esse arquivo: não parece um patch do Rasgo Modular (.rmp).',
                'Impossible d’ouvrir ce fichier : il ne ressemble pas à un patch RASGO Modular (.rmp).',
                'No pude abrir ese archivo: no parece un patch de RASGO Modular (.rmp).'],
  grande: ['File too large for a patch.', 'Arquivo grande demais para um patch.',
           'Fichier trop grand pour un patch.', 'Archivo demasiado grande para un patch.'],
  seedInvalida: ['The seed is a positive whole number (up to 20 digits).',
                 'A seed é um número inteiro positivo (até 20 dígitos).',
                 'Le seed est un nombre entier positif (jusqu’à 20 chiffres).',
                 'El seed es un número entero positivo (hasta 20 dígitos).'],
  tipoDesconhecido: [': unknown module type.', ': tipo desconhecido.', ' : type de module inconnu.', ': tipo de módulo desconocido.'],
  // o ABRE: o app abre "já tocando"; aqui, depois do TOCAR
  abreSeed: ['Next time it opens with a new patch (seed), after PLAY.',
             'Na próxima vez, abre com um patch novo (seed), depois do TOCAR.',
             'La prochaine fois, il s’ouvre avec un nouveau patch (seed), après JOUER.',
             'La próxima vez abre con un patch nuevo (seed), después de TOCAR.'],
  abreUltima: ['Next time it opens with the session as it is when you close the page.',
               'Na próxima vez, abre com a sessão como estiver ao fechar a página.',
               'La prochaine fois, il s’ouvre sur la session telle qu’elle est à la fermeture de la page.',
               'La próxima vez abre con la sesión tal como esté al cerrar la página.'],
  // as duas notas da vista RACK (no app, escritas em `onRackView`)
  vistaTodos: ['ALL: modules in this patch; more in the palette: ', 'TODOS: módulos deste patch; demais na paleta: ',
               'TOUS : modules de ce patch ; autres dans la palette : ', 'TODOS: módulos de este patch; otros en la paleta: '],
  vistaSaida: ['OUTPUT: modules reaching audio: ', 'SAÍDA: módulos que chegam ao áudio: ',
               'SORTIE : modules reliés à l’audio : ', 'SALIDA: módulos que llegan al audio: '],
  // a leitura do LEARN (no app, `showReadingMenu`)
  leitura: ['LEARN reading', 'Leitura do LEARN', 'Lecture du LEARN', 'Lectura del LEARN'],
  leitura0: ['Standard — 12 px', 'Padrão — 12 px', 'Standard — 12 px', 'Normal — 12 px'],
  leitura1: ['Large — 14 px', 'Grande — 14 px', 'Grand — 14 px', 'Grande — 14 px'],
  leitura2: ['Larger — 16 px', 'Maior — 16 px', 'Plus grand — 16 px', 'Mayor — 16 px'],
  leituraBranco: ['White reading text', 'Texto de leitura branco', 'Texte de lecture blanc', 'Texto de lectura blanco'],
  leituraPadrao: ['Restore reading defaults', 'Restaurar padrão de leitura', 'Rétablir la lecture par défaut', 'Restaurar lectura predeterminada'],
  learnRole: ['scroll / F1: read', 'role / F1: ler', 'défiler / F1: lire', 'desplaza / F1: leer'],
  escalas1: ['scale', 'escala', 'gamme', 'escala'],
  escalasN: ['scales', 'escalas', 'gammes', 'escalas'],
  // o SOBRE: a linha que no app diz a taxa e os blocos perdidos
  taxa: ['Hz   ·   WebAssembly, in this browser', 'Hz   ·   WebAssembly, neste navegador',
         'Hz   ·   WebAssembly, dans ce navigateur', 'Hz   ·   WebAssembly, en este navegador'],

  // REC: os arquivos
  recMontando: ['Preparing the recording files…', 'Preparando os arquivos da gravação…',
                'Préparation des fichiers de l’enregistrement…', 'Preparando los archivos de la grabación…'],
  recPronto: ['Recording downloaded: {arquivos}.', 'Gravação baixada: {arquivos}.',
              'Enregistrement téléchargé : {arquivos}.', 'Grabación descargada: {arquivos}.'],
  // as ajudas do cabeçalho (LEARN) que no navegador são outras
  ajuda_bank: ['Download the current patch as a .rmp named after its seed (or the time, without a seed), to keep or to open in the RASGO Modular app. Use OPEN to load it back.',
               'Baixa o patch atual num .rmp com o nome da seed (ou o horário, sem seed), para guardar ou abrir no app RASGO Modular. Use ABRIR para carregá-lo de volta.',
               'Télécharge le patch actuel en .rmp au nom de son seed (ou l’heure, sans seed), à garder ou à ouvrir dans l’application RASGO Modular. Utilisez OUVRIR pour le recharger.',
               'Descarga el patch actual como .rmp con el nombre de su seed (o la hora, sin seed), para guardar o abrir en la aplicación RASGO Modular. Use ABRIR para cargarlo de nuevo.'],
  ajuda_save: ['Keep the current session in this browser: modules, parameters and connections. OPENS · LAST brings it back on the next visit. To record audio, use REC.',
               'Guarda a sessão atual neste navegador: módulos, parâmetros e conexões. ABRE · ÚLTIMA a traz de volta na próxima visita. Para gravar áudio, use REC.',
               'Garde la session actuelle dans ce navigateur : modules, paramètres et connexions. OUVRE · DERNIÈRE la ramène à la prochaine visite. Pour enregistrer l’audio, utilisez REC.',
               'Guarda la sesión actual en este navegador: módulos, parámetros y conexiones. ABRE · ÚLTIMA la recupera en la próxima visita. Para grabar audio, use REC.'],
  ajuda_rec: ['Start or stop recording the stereo output (up to 4 min; the button shows the time). On stop the browser downloads rec-date-time-seedN.wav (24-bit), its -14 LUFS delivery version, the .score.txt and, with a NOTE-OUT in line, a .mid. Saving a patch and recording audio are different actions.',
              'Inicia ou termina a gravação da saída estéreo (até 4 min; o botão mostra o tempo). Ao parar, o navegador baixa rec-data-hora-seedN.wav (24 bits), a versão de entrega a -14 LUFS, o .score.txt e, com um NOTE-OUT no caminho, um .mid. Salvar patch e gravar áudio são ações diferentes.',
              'Démarre ou arrête l’enregistrement de la sortie stéréo (jusqu’à 4 min ; le bouton affiche le temps). À l’arrêt, le navigateur télécharge rec-date-heure-seedN.wav (24 bits), sa version de diffusion à -14 LUFS, le .score.txt et, avec un NOTE-OUT sur le trajet, un .mid. Enregistrer un patch et l’audio sont deux actions différentes.',
              'Inicia o detiene la grabación de la salida estéreo (hasta 4 min; el botón muestra el tiempo). Al parar, el navegador descarga rec-fecha-hora-seedN.wav (24 bits), su versión de entrega a -14 LUFS, el .score.txt y, con un NOTE-OUT en el camino, un .mid. Guardar un patch y grabar audio son acciones distintas.'],

  // ---- cartões do TUTORIAL no navegador ---------------------------------
  tutSeedBoxBody: [
    'The box beside SEED is a normal text field. Click to place the cursor, drag to select, double-click or Ctrl+A to select all. Backspace/Delete erase; arrows and Home/End move; Ctrl+C / Ctrl+X / Ctrl+V copy, cut, paste. Type a number and press Enter to load that exact patch.',
    'A caixa ao lado de SEED é um campo de texto normal. Clique pra posicionar o cursor, arraste pra selecionar, duplo-clique ou Ctrl+A seleciona tudo. Backspace/Delete apagam; setas e Home/End movem; Ctrl+C / Ctrl+X / Ctrl+V copiam, recortam, colam. Digite um número e Enter pra carregar aquele patch exato.',
    'La case à côté de SEED est un champ de texte normal. Cliquez pour placer le curseur, glissez pour sélectionner, double-clic ou Ctrl+A tout sélectionner. Retour arrière/Suppr effacent ; flèches et Origine/Fin déplacent ; Ctrl+C / Ctrl+X / Ctrl+V copient, coupent, collent. Tapez un numéro puis Entrée pour charger ce patch précis.',
    'La caja junto a SEED es un campo de texto normal. Clic para situar el cursor, arrastre para seleccionar, doble clic o Ctrl+A selecciona todo. Retroceso/Supr borran; flechas y Inicio/Fin mueven; Ctrl+C / Ctrl+X / Ctrl+V copian, cortan, pegan. Escriba un número y Enter para cargar ese patch exacto.'],
  tutStoreBody: [
    'SAVE (Ctrl+S) keeps the whole session in this browser — the graph, the cabling, every parameter, the seed. It is the instrument, not the sound. BANK (Ctrl+B) downloads the current patch as a .rmp named after its seed (seed-…rmp), to keep or to open in the RASGO Modular app. OPEN (Ctrl+O) brings any .rmp back, from this page or from the app. REC is a different thing: it records the AUDIO — see RECORD AUDIO.',
    'SALVA (Ctrl+S) guarda a sessão inteira neste navegador — o grafo, o cabeamento, cada parâmetro, o seed. É o instrumento, não o som. BANCO (Ctrl+B) baixa o patch atual num .rmp com o nome do seed (seed-…rmp), para guardar ou abrir no app RASGO Modular. ABRIR (Ctrl+O) traz qualquer .rmp de volta, desta página ou do app. REC é outra coisa: grava o ÁUDIO — veja GRAVAR ÁUDIO.',
    'ENREG. (Ctrl+S) garde toute la session dans ce navigateur — le graphe, le câblage, chaque paramètre, le seed. C’est l’instrument, pas le son. BANQUE (Ctrl+B) télécharge le patch courant en .rmp au nom de son seed (seed-…rmp), à garder ou à ouvrir dans l’application RASGO Modular. OUVRIR (Ctrl+O) rouvre n’importe quel .rmp, de cette page ou de l’application. REC est autre chose : il enregistre l’AUDIO — voir ENREGISTRER.',
    'GUARDA (Ctrl+S) guarda la sesión entera en este navegador — el grafo, el cableado, cada parámetro, el seed. Es el instrumento, no el sonido. BANCO (Ctrl+B) descarga el patch actual como .rmp con el nombre de su seed (seed-…rmp), para guardar o abrir en la aplicación RASGO Modular. ABRIR (Ctrl+O) trae de vuelta cualquier .rmp, de esta página o de la aplicación. REC es otra cosa: graba el AUDIO — vea GRABAR AUDIO.'],
  tutRecBody: [
    'REC (or Ctrl+R) starts and stops recording the output. On stop the browser downloads a stereo 24-bit WAV named by date and time — rec-20260911-143052.wav — with a matching .score.txt log of the take; if the patch has a NOTE-OUT in line, the notes also come out as a .mid of the same name, to open in a DAW or a score editor. Recording stops by itself at four minutes. Nothing is sent anywhere: the files are made in this page.',
    'REC (ou Ctrl+R) começa e para a gravação da saída. Ao parar, o navegador baixa um WAV estéreo de 24 bits com nome por data e hora — rec-20260911-143052.wav — com um .score.txt de mesmo nome; se o patch tem um NOTE-OUT no caminho, as notas saem também num .mid com o mesmo nome, para abrir numa DAW ou num editor de partitura. A gravação para sozinha aos quatro minutos. Nada é enviado a lugar nenhum: os arquivos são feitos nesta página.',
    'REC (ou Ctrl+R) démarre et arrête l’enregistrement de la sortie. À l’arrêt, le navigateur télécharge un WAV stéréo 24 bits nommé par date et heure — rec-20260911-143052.wav — avec un .score.txt du même nom ; si le patch a un NOTE-OUT sur le trajet, les notes sortent aussi dans un .mid du même nom, à ouvrir dans une DAW ou un éditeur de partition. L’enregistrement s’arrête seul à quatre minutes. Rien n’est envoyé nulle part : les fichiers sont faits dans cette page.',
    'REC (o Ctrl+R) inicia y detiene la grabación de la salida. Al parar, el navegador descarga un WAV estéreo de 24 bits con nombre por fecha y hora — rec-20260911-143052.wav — con un .score.txt del mismo nombre; si el patch tiene un NOTE-OUT en el camino, las notas salen también en un .mid del mismo nombre, para abrir en una DAW o un editor de partituras. La grabación se detiene sola a los cuatro minutos. Nada se envía a ningún lugar: los archivos se hacen en esta página.'],
  tutNavBody: [
    'Mouse wheel scrolls the rack (over a knob it turns the knob). Ctrl+= and Ctrl+− zoom in and out, Ctrl+0 back to 100%. Zoom out to see the first and last rows together and patch between them; while you drag a cable, take it near the top or bottom edge to scroll.',
    'A roda do mouse rola o rack (sobre um knob, gira o knob). Ctrl+= e Ctrl+− ampliam e reduzem, Ctrl+0 volta a 100%. Reduza pra ver a primeira e a última fileira juntas e cabear entre elas; enquanto arrasta um cabo, leve-o pra perto da borda de cima ou de baixo pra rolar.',
    'La molette fait défiler le rack (sur un potentiomètre, elle le tourne). Ctrl+= et Ctrl+− zooment, Ctrl+0 revient à 100%. Dézoomez pour voir la première et la dernière rangée ensemble et câbler entre elles ; en tirant un câble, approchez-le du bord haut ou bas pour défiler.',
    'La rueda del ratón desplaza el rack (sobre un knob, lo gira). Ctrl+= y Ctrl+− amplían y reducen, Ctrl+0 vuelve al 100%. Aleje para ver la primera y la última fila juntas y cablear entre ellas; mientras arrastra un cable, llévelo cerca del borde superior o inferior para desplazar.'],
  tutKeysBody: [
    'g new seed  ·  v live variation on/off  ·  m change  ·  e evolve (six small steps)  ·  c cross with a fresh donor patch  ·  space breaks and restores every cable at once  ·  Ctrl+S save the session  ·  Ctrl+B file the patch in the bank  ·  Ctrl+R record  ·  Ctrl+Z undo (one action back, cable by cable)  ·  Ctrl+O open a patch  ·  Esc cancel the gesture in progress  ·  n pull every cable  ·  r restore the current seed (undo every hand edit)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  arrows scroll the rack  ·  F1 read the LEARN box  ·  F2 reading size. Every shortcut that has a button lights that button for a moment, so you can see what you just did. Inside TUTORIAL and ABOUT: arrows, Page Up/Down, Home/End scroll, Esc closes.',
    'g sorteia um seed  ·  v liga/desliga a variação ao vivo  ·  m muda  ·  e evolui (seis passos pequenos)  ·  c cruza com um doador novo  ·  [espaço] rompe e reata todos os cabos de uma vez  ·  Ctrl+S salva a sessão  ·  Ctrl+B arquiva o patch no banco  ·  Ctrl+R grava  ·  Ctrl+Z desfaz (uma ação, cabo a cabo)  ·  Ctrl+O abre um patch  ·  Esc cancela o gesto em curso  ·  n tira todos os cabos  ·  r repõe o seed atual (desfaz toda edição à mão)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  setas rolam o rack  ·  F1 lê a caixa LEARN  ·  F2 tamanho da leitura. Todo atalho que tem botão ACENDE esse botão por um instante — dá pra ver o que você acabou de fazer. Dentro de TUTORIAL e SOBRE: setas, Page Up/Down, Home/End rolam, Esc fecha.',
    'g nouveau seed  ·  v variation en direct on/off  ·  m changer  ·  e évoluer (six petits pas)  ·  c croiser avec un donneur neuf  ·  espace rompt et rétablit tous les câbles d’un coup  ·  Ctrl+S enregistre la session  ·  Ctrl+B classe le patch dans la banque  ·  Ctrl+R enregistre  ·  Ctrl+Z annule (une action, câble par câble)  ·  Ctrl+O ouvre un patch  ·  Échap annule le geste en cours  ·  n retire tous les câbles  ·  r rétablit le seed courant (annule toute édition manuelle)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  flèches font défiler le rack  ·  F1 lit la case LEARN  ·  F2 taille de lecture. Chaque raccourci qui a un bouton ALLUME ce bouton un instant. Dans TUTORIEL et À PROPOS : flèches, Page Haut/Bas, Origine/Fin défilent, Échap ferme.',
    'g sortea un seed  ·  v activa/desactiva la variación en vivo  ·  m cambia  ·  e evoluciona (seis pasos pequeños)  ·  c cruza con un donante nuevo  ·  espacio rompe y restablece todos los cables a la vez  ·  Ctrl+S guarda la sesión  ·  Ctrl+B archiva el patch en el banco  ·  Ctrl+R graba  ·  Ctrl+Z deshace (una acción, cable a cable)  ·  Ctrl+O abre un patch  ·  Esc cancela el gesto en curso  ·  n quita todos los cables  ·  r repone el seed actual (deshace toda edición manual)  ·  Ctrl+= / Ctrl+− / Ctrl+0 zoom  ·  flechas desplazan el rack  ·  F1 lee la caja LEARN  ·  F2 tamaño de lectura. Cada atajo que tiene botón ENCIENDE ese botón un instante. Dentro de TUTORIAL y ACERCA DE: flechas, Re/Av Pág, Inicio/Fin desplazan, Esc cierra.'],
};

// a língua inicial: a escolhida antes neste navegador, senão a do navegador
export function linguaInicial(guardada) {
  const g = Number(guardada);
  if (guardada !== null && g >= 0 && g <= 3) return g;
  const n = (navigator.language || 'en').slice(0, 2).toLowerCase();
  const i = LINGUAS.indexOf(n);
  return i >= 0 ? i : 0;
}

// o texto da página na língua `l`, com {nome} etc. preenchidos
export function web(chave, l, vars = {}) {
  const t = T[chave];
  let s = t ? (t[l] || t[0]) : chave;
  for (const [k, v] of Object.entries(vars)) s = s.replace('{' + k + '}', v);
  return s;
}
export const temWeb = (chave) => chave in T;
