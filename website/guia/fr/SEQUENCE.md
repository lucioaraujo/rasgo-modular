## O que é
SEQUENCE joue une courte phrase que vous écrivez : jusqu'à huit notes, chacune avec sa hauteur, chacune active ou muette. À chaque impulsion sur l'entrée CLK, elle avance d'un pas, envoyant la hauteur sur la sortie PTCH et l'impulsion de la note sur la sortie GATE. Elle sert pour une ligne de basse, un riff, une figure qui se répète.

## Como pensar nele
TURING fait naître la phrase du hasard ; SEQUENCE part d'une phrase que vous décidez. La variation vient de la manière de lire : en avant, en arrière, en aller-retour, dans un ordre tiré au sort, ou en errant d'un pas à son voisin. La même phrase de huit notes donne beaucoup rien qu'en changeant MODE et LEN. Pour que les hauteurs tombent sur les notes d'une gamme, faites passer PTCH par un QUANTIZER avant l'oscillateur.

## Controles
- LEN : combien des huit pas entrent dans la phrase.
- MODE : l'ordre de lecture : en avant, en arrière, aller-retour, au hasard, ou en errant pas à pas.
- RATE : l'horloge interne, utilisée quand rien n'arrive sur CLK.
- GATE : combien de temps chaque note reste active dans son pas. Court, c'est détaché ; long, lié.
- GLIDE : fait glisser la hauteur d'une note à la suivante.
- RANGE : combien d'octaves couvrent les huit valeurs, jusqu'à deux.
- P1 à P8 : la hauteur de chaque pas.
- G1 à G8 : active ou rend muet chaque pas.
Entrées : CLK, l'impulsion qui fait avancer ; RST, pour revenir au premier pas. Sorties : PTCH, la hauteur ; GATE, l'impulsion de la note ; EOS, une impulsion à chaque fin de phrase.

## Experimente
1. Branchez la sortie CLK du CLOCK sur CLK de la SEQUENCE.
2. Branchez PTCH sur l'entrée CV d'un QUANTIZER, et PTCH du QUANTIZER sur 1V/O d'un OSC. Faites passer l'OSC par un ENVELOPE jusqu'au MIXER et branchez GATE de la SEQUENCE sur GATE de l'enveloppe.
3. Réglez P1 à P8 jusqu'à ce que la phrase vous plaise, et rendez quelques pas muets avec G1 à G8.
4. Changez MODE et écoutez la même phrase lue autrement.
