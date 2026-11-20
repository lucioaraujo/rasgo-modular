## O que é
POLY-IN répartit les notes d'un clavier MIDI ou d'une piste de la DAW sur jusqu'à quatre voix. Chaque voix a sa hauteur, son gate et sa vélocité, et chacune peut jouer son propre oscillateur.

## Como pensar nele
SIGNAL-IN donne une seule voix ; POLY-IN donne un accord. Le rack accepte jusqu'à quatre copies de chaque module : avec quatre OSC et quatre ENVELOPE, chaque voix de POLY-IN joue une paire, et le clavier joue maintenant des accords dans le patch. Le module démarre éteint et ne demande jamais le micro : il n'ouvre que le MIDI.

## Controles
- VOZES : combien de voix reçoivent des notes, d'une à quatre.
- MODE : comment les notes se répartissent. *Rota* passe chaque note à la voix libre suivante ; *Mesma* rend la note répétée à la voix où elle a joué ; *Altura* met les voix dans l'ordre, et la voix 1 tient toujours la note la plus grave. Sans voix libre, la nouvelle note prend la voix la plus ancienne.
- BEND : de combien de demi-tons le pitch bend fait bouger toutes les voix.
- ON : allume l'entrée MIDI.
Sorties : P1 à P4, la hauteur de chaque voix ; G1 à G4, le gate de chaque voix, ouvert tant que la touche ou la pédale tient la note ; V1 à V4, la force de chaque note.

## Experimente
1. Allumez ON. Branchez P1 sur l'entrée 1V/O d'un OSC et G1 sur l'entrée GATE d'un ENVELOPE qui reçoit le son de cet OSC.
2. Ajoutez un autre OSC et un autre ENVELOPE et branchez-y P2 et G2. Jouez deux notes ensemble : chacune sonne sur sa paire.
3. Avec quatre paires et MODE sur *Altura*, la voix 1 est toujours la basse de l'accord : donnez-lui un filtre plus fermé ou plus de niveau sur le MIXER.
