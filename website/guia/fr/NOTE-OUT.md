## O que é
NOTE-OUT note les notes d'une voix. Placé sur le chemin des signaux de hauteur et de déclenchement, il consigne chaque note qui passe, avec hauteur, durée, intensité et accent, dans la partition que l'application enregistre avec l'audio. Il n'a pas de réglages et ne change pas le son.

## Como pensar nele
Dans un patch génératif, les notes apparaissent et disparaissent. NOTE-OUT en garde une trace pour qui voudra étudier, transcrire ou retravailler la pièce plus tard. Il se place au milieu du chemin : la hauteur entre sur PITCH et ressort inchangée sur PTHR ; l'impulsion entre sur GATE et ressort inchangée sur GTHR. Quand vous enregistrez avec REC, le fichier texte qui accompagne l'audio contient ces notes.

## Controles
NOTE-OUT n'a pas de réglages.
Entrées : GATE, l'impulsion de chaque note ; PITCH, la hauteur ; VEL, l'intensité ; ACC, l'accent. Sorties : GTHR et PTHR, les mêmes signaux, pour continuer jusqu'à la voix.

## Experimente
1. Dans une mélodie faite avec QUANTIZER, OSC et ENVELOPE, branchez PTCH du QUANTIZER sur PITCH de NOTE-OUT, et PTHR sur 1V/O de l'OSC.
2. Branchez l'impulsion qui déclenche l'enveloppe sur GATE de NOTE-OUT, et GTHR sur GATE de l'enveloppe. Le son reste le même.
3. Enregistrez un passage avec REC. À côté du fichier audio, le fichier texte de la partition liste chaque note jouée.
