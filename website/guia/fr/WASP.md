## O que é
WASP est un filtre qui distord. Inspiré d'un synthétiseur de la fin des années 1970 connu pour son son râpeux, il coupe les fréquences comme FILTER, mais salit le son quand la résonance monte. C'est le filtre des basses agressives, des solos tranchants et des bourdons qui grincent.

## Como pensar nele
FILTER est propre et précis ; WASP en est le pendant sale. Trois boutons dosent la saleté : DRIVE pousse le son vers la distorsion avant le filtrage, GRIT décide à quel point le filtre grince en résonnant, et BIAS déséquilibre la distorsion, qui devient plus bourdonnante. Avec une résonance haute, il sonne tout seul, et ce son-là est râpeux aussi.

## Controles
- CUT : la fréquence de coupure, de 20 à 24000 Hz.
- RESO : la résonance. Près du maximum, le filtre sonne tout seul.
- MODE : le type de filtre, du passe-bas au passe-bande et au passe-haut.
- DRIVE : le gain d'entrée, qui pousse le son vers la distorsion avant le filtrage.
- GRIT : à quel point le filtre distord en résonnant. Bas, presque propre ; haut, il grince.
- BIAS : déséquilibre la distorsion, en ajoutant des harmoniques pairs et un bourdonnement.
- DRIFT : une oscillation lente de la coupure et de la résonance.
Entrées : IN, le son à filtrer ; FC, pour déplacer la coupure ; Q, pour la résonance.

## Experimente
1. Branchez la sortie SAW d'un OSC sur l'entrée IN, et OUT sur une entrée du MIXER.
2. Montez RESO vers les deux tiers et tournez CUT : écoutez le filtre mordre.
3. Montez GRIT et DRIVE. Le son devient râpeux et agressif.
4. Branchez la sortie ENV d'un ENVELOPE sur FC, avec le CLOCK sur l'entrée GATE de l'enveloppe. Vous avez une basse acide.
