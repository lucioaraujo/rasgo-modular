## O que é
SIGNAL-IN fait entrer le monde extérieur dans le patch. Il reçoit le son de l'entrée audio de l'ordinateur, comme un micro ou un autre instrument, et les notes d'un clavier ou d'un contrôleur MIDI, et transforme tout cela en signaux que les autres modules comprennent.

## Como pensar nele
Grâce à lui, quelqu'un de l'extérieur peut jouer du Rasgo Modular : la hauteur de la touche va vers un oscillateur, l'appui sur la touche ouvre une enveloppe, et la molette de modulation du clavier peut ouvrir un filtre. Il permet aussi au patch de traiter un son extérieur, en faisant passer une voix ou une guitare par des filtres et des espaces. L'application n'ouvre l'entrée audio que s'il y a un SIGNAL-IN dans le patch.

## Controles
- GAIN : le volume de l'audio entrant, de zéro au double.
- BEND : de combien de demi-tons le pitch bend du clavier fait bouger la hauteur, de zéro à deux octaves.
- CC# : quel contrôleur du clavier la sortie CC suit. Le numéro 1 est en général la molette de modulation.
Sorties : L et R, l'audio entrant ; 1V/O, la hauteur de la touche jouée ; GATE, ouvert tant que la touche est enfoncée ; VEL, la force avec laquelle elle a été jouée ; CC, la valeur du contrôleur choisi.

## Experimente
1. Avec un micro ou un instrument relié à l'ordinateur, branchez L sur une entrée du MIXER et réglez GAIN jusqu'à entendre le son.
2. Faites passer ce son par un FILTER ou par SPACE avant le MIXER pour le transformer.
3. Avec un clavier MIDI, branchez 1V/O sur l'entrée 1V/O d'un OSC, et GATE sur l'entrée GATE d'un ENVELOPE qui a le son de l'OSC sur son entrée IN. Le clavier joue maintenant le patch.
