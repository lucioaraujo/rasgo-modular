## O que é
MULT prend un signal de contrôle et le distribue vers quatre sorties, chacune avec son propre réglage de taille et de décalage. Sans rien à l'entrée, il devient une banque de quatre valeurs fixes que vous réglez à la main.

## Como pensar nele
Dans le Rasgo Modular, une sortie peut déjà aller vers plusieurs destinations avec des câbles ordinaires. MULT sert quand chaque destination a besoin d'une dose différente : la même enveloppe qui ouvre un filtre en grand, baisse un peu le volume d'une autre voix, effleure un effet. Une entrée, quatre versions ajustées. Avec DUAL allumé, il se divise en deux distributeurs de deux sorties chacun.

## Controles
- DUAL : éteint, IN va vers les quatre sorties ; allumé, IN va vers O1 et O2, et IN2 vers O3 et O4.
- SCL1, SCL2, SCL3, SCL4 : la taille du signal sur chaque sortie. À 1, il passe tel quel ; en dessous de 1, réduit ; au-dessus, agrandi ; négatif, inversé. À zéro, il ne reste que le décalage.
- OFF1, OFF2, OFF3, OFF4 : une valeur ajoutée à chaque sortie. Sans rien à l'entrée, c'est la valeur de la sortie elle-même.
- SLEW : lisse les quatre sorties ensemble.
Entrées : IN et IN2. Sorties : O1 à O4.

## Experimente
1. Montez deux voix, chacune avec son FILTER, qui passent par le MIXER.
2. Branchez la sortie ENV d'un ENVELOPE joué par le CLOCK sur IN du MULT. Branchez O1 sur FC du premier FILTER et O2 sur FC du second.
3. Laissez SCL1 à 1 et mettez SCL2 sur une valeur négative. À chaque note, un filtre s'ouvre et l'autre se ferme.
