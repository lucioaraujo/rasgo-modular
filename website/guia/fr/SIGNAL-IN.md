## O que é
SIGNAL-IN fait entrer le monde extérieur dans le patch. Il reçoit le son de l'entrée audio de l'ordinateur, comme un micro ou un autre instrument, et les notes d'un clavier ou d'un contrôleur MIDI, et transforme tout cela en signaux que les autres modules comprennent.

## Como pensar nele
Grâce à lui, quelqu'un de l'extérieur peut jouer du Rasgo Modular : la hauteur de la touche va vers un oscillateur, l'appui sur la touche ouvre une enveloppe, et la molette de modulation du clavier peut ouvrir un filtre. Il permet aussi au patch de traiter un son extérieur, en faisant passer une voix ou une guitare par des filtres et des espaces. Le module démarre éteint : l'application n'ouvre le micro et le MIDI qu'après que vous avez allumé ON et câblé une de ses sorties.

## Controles
- GAIN : le volume de l'audio entrant, de zéro au double.
- BEND : de combien de demi-tons le pitch bend du clavier fait bouger la hauteur, de zéro à deux octaves.
- CC# : quel contrôleur du clavier la sortie CC suit. Le numéro 1 est en général la molette de modulation.
- ON : allume l'entrée extérieure. Éteint, ce qui est l'état de départ, rien n'entre dans le patch.
- DIV : la division de l'horloge MIDI sur la sortie CLK : noire, croche (la valeur de départ, accordée au MULT 2 du CLOCK), double croche ou les 24 impulsions par noire.
Sorties : L et R, l'audio entrant ; 1V/O, la hauteur de la touche jouée ; GATE, ouvert tant que la touche est enfoncée ; VEL, la force avec laquelle elle a été jouée ; CC, la valeur du contrôleur choisi ; CLK, des impulsions au tempo de l'horloge MIDI qui vient d'un logiciel d'enregistrement ou d'une machine ; RUN, ouvert tant que ce logiciel joue ; RST, une impulsion sur le premier temps après le start.

## Experimente
1. Avec un micro ou un instrument relié à l'ordinateur, branchez L sur une entrée du MIXER, allumez ON et réglez GAIN jusqu'à entendre le son.
2. Faites passer ce son par un FILTER ou par SPACE avant le MIXER pour le transformer.
3. Avec un clavier MIDI et ON allumé, branchez 1V/O sur l'entrée 1V/O d'un OSC, et GATE sur l'entrée GATE d'un ENVELOPE qui a le son de l'OSC sur son entrée IN. Le clavier joue maintenant le patch.
4. Dans un logiciel d'enregistrement, activez l'envoi de l'horloge MIDI vers Rasgo Modular. Branchez CLK sur l'entrée EXT du CLOCK et RST sur l'entrée RST du CLOCK, puis lancez la lecture : le patch avance à son tempo et commence sur le même temps.
