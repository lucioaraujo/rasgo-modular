## O que é
SWITCH est un sélecteur commandé par le patch. Il peut choisir une source parmi quatre et l'envoyer vers une sortie, ou prendre une seule source et l'envoyer vers l'une de quatre sorties. Le changement se fait à chaque impulsion, dans l'ordre ou au hasard, ou suit un signal de contrôle.

## Como pensar nele
Dans un patch qui se transforme, changer de matière compte autant que changer de note : la même mélodie jouée tantôt par un oscillateur, tantôt par une corde ; le filtre nourri tantôt par le bruit, tantôt par l'accord. Sans SWITCH, il faut changer les câbles à la main. Avec DEMUX éteint, il choisit entre A, B, C et D et livre sur OA. Allumé, il prend ce qui arrive sur A et l'envoie vers l'une des quatre sorties, OA, OB, OC ou OD.

## Controles
- STEP : combien de positions parcourt le sélecteur, de 2 à 4.
- MODE : comment la position change : en avant, en aller-retour, au hasard, ou seulement par le signal de l'entrée ADR.
- DEMUX : éteint, quatre entrées vers une sortie ; allumé, une entrée vers quatre sorties.
- GLID : une transition progressive entre les positions, où les deux sources se mélangent un instant. À zéro, la coupure est nette.
- SLEW : adoucit le changement pour éviter les clics. Mieux vaut toujours en laisser un peu.
Entrées : A, B, C et D, les sources ; CLK, l'impulsion qui fait avancer ; RST, pour revenir à la première position ; ADR, pour choisir la position avec un signal. Sorties : OA, OB, OC et OD ; STP, la position actuelle en signal de contrôle.

## Experimente
1. Branchez la sortie SAW d'un OSC sur A, et OUT d'un MATTER, joué par le CLOCK, sur B. Branchez OA sur une entrée du MIXER.
2. Mettez STEP à 2 et branchez la sortie DIV d'un LOGIC sur CLK, avec la sortie CLK du CLOCK sur l'entrée CLK du LOGIC et DIV à 4.
3. Toutes les quatre impulsions, le son change de source. Montez GLID pour que les changements se fondent.
