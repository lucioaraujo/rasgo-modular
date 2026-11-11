## O que é
ARP est un arpégiateur : il joue un accord note par note, et ainsi un accord devient une mélodie. Il choisit les notes (l'une des dix formes d'accord du CHORD, avec renversement et octaves) et le chemin qui les parcourt. Sans rien de branché, il joue sur sa propre horloge.

## Como pensar nele
Séparez deux questions : quelles notes, et dans quel ordre. CHORD, INV et OCT répondent à la première ; MODE à la seconde (en montant, en descendant, aller-retour, au hasard, mélangé, du centre vers l'extérieur, de l'extérieur vers le centre). SKIP ouvre des trous dans la ligne, et les trous sont toujours les mêmes pour le même patch : la ligne sonne jouée et se répète à l'identique.

## Controles
- CHORD : la forme de l'accord, de 0 à 9.
- INV : le renversement, de 0 à 3.
- OCT : sur combien d'octaves l'arpège s'étend, de 1 à 4.
- MODE : le chemin à travers les notes, de 0 à 7.
- SKIP : la probabilité qu'un pas reste silencieux.
- RATE : l'horloge interne, utilisée quand rien n'est branché sur CLK.
- GATE : la durée de chaque note, de 5 à 95 % du pas.
Entrées : CLK avance d'un pas par impulsion, RST revient au début, ROOT reçoit la tonique (un volt par octave), CHD change la forme de l'accord. Sorties : PTCH est la note, GT le gate de chaque note, EOS une impulsion à chaque tour complet.

## Experimente
1. Branchez PTCH dans le 1V/O d'un OSC, la sortie de l'OSC dans un VCA et le VCA dans le MIXER. Branchez GT dans un ENVELOPE et l'ENVELOPE dans le VCA.
2. Tournez MODE et écoutez le même accord prendre d'autres dessins.
3. Mettez MODE à 5, SKIP à 0,3 et OCT à 3 : on dirait une ligne improvisée.
4. Branchez la sortie root du HARMONY dans ROOT : l'arpège avance avec la progression.
