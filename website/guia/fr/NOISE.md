## O que é
NOISE produit du bruit, le son de toutes les fréquences mêlées au hasard, en plusieurs couleurs à la fois, chacune sur sa sortie : du blanc, sifflant et brillant, au brun, grave comme la mer. Il produit aussi des valeurs aléatoires lentes, qui servent à faire changer d'autres modules tout seuls.

## Como pensar nele
NOISE a deux usages très différents. Comme son, c'est la matière du vent, de la pluie, des cymbales et du claquement d'une caisse claire. Comme hasard, c'est une source de variation pour les patchs génératifs : la sortie S&H tire une nouvelle valeur à chaque impulsion et la garde jusqu'à la suivante, et SMTH glisse d'une valeur à l'autre. Branchez l'une d'elles sur la hauteur d'un oscillateur, en passant par un quantificateur, et vous avez une mélodie qui ne se répète jamais à l'identique.

## Controles
- RATE : le rythme des tirages des sorties S&H et SMTH, de très lent à 2000 par seconde. Si l'entrée TRIG est branchée, c'est elle qui commande à la place de ce bouton.
- SLEW : le temps que met la sortie SMTH pour atteindre chaque nouvelle valeur. À zéro, elle saute ; au maximum, elle glisse lentement.
- SPRD : le type de tirage. À zéro, toutes les valeurs sont également probables ; au maximum, les valeurs proches du milieu reviennent plus souvent et les changements sont plus doux.
- POIS : remplace le rythme régulier des tirages par des moments irréguliers, au hasard, avec la même moyenne.
Sorties de son : WHT (blanc), PNK (rose), BRN (brun), BLU (bleu), VLT (violet) et BIT (bruit numérique). Sorties de hasard : S&H et SMTH. Entrées : TRIG, pour déclencher les tirages, et IN.

## Experimente
1. Branchez PNK sur une entrée du MIXER. Vous entendez un souffle doux, comme la pluie.
2. Passez à BRN, plus grave, puis à WHT, plus brillant.
3. Branchez S&H sur l'entrée CV d'un QUANTIZER, et sa sortie PTCH sur l'entrée 1V/O d'un OSC qui sonne. La note se met à sauter au hasard, toujours dans la gamme.
4. Branchez la sortie CLK du CLOCK sur l'entrée TRIG : les sauts se font maintenant au rythme de l'horloge.
