## O que é
FILTER laisse passer une partie des fréquences d'un son et coupe le reste. Il a trois sorties qui fonctionnent en même temps : LO, qui laisse passer les graves ; CTR, la bande du milieu ; et HI, les aigus. En tournant la coupure, vous entendez le son s'assombrir ou s'éclaircir, comme quand on met la main devant la bouche en parlant.

## Como pensar nele
C'est le deuxième module de presque tous les patchs : après une source riche en harmoniques, comme la dent de scie d'un OSC, le filtre sculpte le timbre. Sa particularité est SPRD : à zéro, les trois sorties partagent la même coupure ; en le montant, elles s'écartent et deviennent trois filtres différents, chacun sur une région du son. Avec la résonance au maximum, le filtre se met à sonner tout seul, comme un oscillateur sinusoïdal.

## Controles
- CUT : la fréquence de coupure, de 20 à 20000 Hz. C'est le réglage principal : tournez-le et écoutez le timbre s'ouvrir et se fermer.
- RESO : renforce les fréquences proches de la coupure, pour un timbre plus nasillard. Au maximum, le filtre se met à sonner seul.
- SPRD : écarte les trois sorties en fréquence. À zéro, elles suivent la même coupure ; au maximum, chacune occupe sa région du son.
- DRIVE : sature le signal avant le filtrage, ce qui épaissit le son.
Entrées : IN, le son à filtrer ; FC, pour déplacer la coupure (1 V par octave) ; Q, pour déplacer la résonance ; SPR, pour déplacer l'écart. Sorties : LO, CTR, HI et ALL, qui additionne les trois.

## Experimente
1. Branchez la sortie SAW d'un OSC sur l'entrée IN, et la sortie ALL sur une entrée du MIXER.
2. Tournez CUT lentement d'un côté à l'autre et écoutez la brillance entrer et sortir.
3. Montez RESO à mi-course et recommencez : la coupure devient marquée, presque une voix.
4. Branchez la sortie ENV d'un ENVELOPE sur l'entrée FC, avec le CLOCK sur l'entrée GATE de l'enveloppe. Le filtre s'ouvre à chaque impulsion, un son qu'on entend dans énormément de musique électronique.
