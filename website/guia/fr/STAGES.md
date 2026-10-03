## O que é
STAGES dessine une courbe faite de plusieurs segments enchaînés, de deux à huit. Selon les réglages, cette courbe devient une enveloppe à plusieurs étapes, un mouvement lent qui ne se répète jamais exactement, une suite de valeurs en marches, ou même un son à la forme étrange.

## Como pensar nele
FUNCTION fait une seule rampe, et ENVELOPE les quatre étapes habituelles. STAGES fait des formes composées, et vous ne dessinez pas point par point : quelques réglages modèlent tout le dessin d'un coup. Le réglage qui change le plus son caractère est HOLD. À zéro, les segments sont des rampes et le résultat est un mouvement continu ; au maximum, chaque segment saute et se maintient, et STAGES se met à fonctionner comme un séquenceur. Avec LOOP allumé, la forme se répète toute seule ; éteint, elle se déroule une fois à chaque impulsion sur GATE.

## Controles
- SEGS : combien de segments a la forme, de 2 à 8.
- RATE : la vitesse d'un tour complet, quand LOOP est allumé.
- CNTR : le dessin des niveaux : un escalier qui monte, une arche, ou un escalier qui descend.
- CURVE : comment chaque segment va d'un niveau à l'autre : d'abord vite, en ligne droite, ou d'abord lentement.
- HOLD : à zéro, des rampes douces ; au maximum, des marches qui sautent et se maintiennent.
- TILT : rend les premiers segments plus longs que les derniers, ou l'inverse.
- JITR : fait varier légèrement niveaux et durées à chaque tour, toujours de la même façon pour la même graine.
- LOOP : allumé, la forme se répète ; éteint, elle se déroule une fois par impulsion.
Entrées : GATE, l'impulsion qui déclenche ; RST, pour revenir au début ; RTM, pour moduler la vitesse. Sorties : OUT, la courbe ; EOC, une impulsion à la fin de chaque tour ; STEP, une impulsion à chaque segment.

## Experimente
1. Montez une voix : la SAW d'un OSC sur l'entrée IN d'un FILTER, LO du FILTER dans le MIXER. Baissez CUT.
2. Branchez OUT de STAGES sur l'entrée FC du FILTER, avec SEGS à 6. Le timbre se promène selon une forme qui va et vient.
3. Bougez CNTR et TILT et écoutez le dessin changer.
4. Poussez HOLD au maximum et SEGS à 8. Branchez OUT sur l'entrée CV d'un QUANTIZER et PTCH du QUANTIZER sur 1V/O de l'OSC : STAGES joue maintenant une mélodie.
