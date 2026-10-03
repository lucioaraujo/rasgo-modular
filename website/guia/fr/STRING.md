## O que é
STRING simule une corde : pincée, comme sur une guitare ; frappée, comme sur un piano ; ou frottée, comme sur un violon. Il recrée le comportement d'une corde qui vibre et perd peu à peu son énergie, ce qui donne à l'attaque et au corps du son un naturel, même s'ils sont synthétiques.

## Como pensar nele
MATTER est le module des objets qui résonnent, comme les cloches et les plaques ; STRING est celui des cordes. Son geste principal est POS, l'endroit où la corde est jouée : en le déplaçant, certains harmoniques disparaissent, comme quand un guitariste joue plus près du chevalet. Une impulsion sur PLK pince la corde ; un son continu sur IN la fait chanter comme sous l'archet.

## Controles
- FREQ : l'accord de la corde, de 20 à 4000 Hz.
- DECAY : combien de temps la corde sonne après avoir été pincée. Bas, c'est sec ; haut, elle ne s'arrête presque pas.
- DAMP : la brillance. Haut, la corde devient sombre, comme une vieille corde ; bas, elle garde l'éclat d'une corde neuve.
- POS : l'endroit où la corde est jouée. Près du milieu, les harmoniques pairs disparaissent et le son devient creux.
- EXCIT : combien de bruit entre dans le coup, comme le son du doigt ou du médiator.
- DRIVE : sature la corde de l'intérieur, l'emmenant du son acoustique à un son distordu.
- MIX : le mélange entre le coup brut et la corde. Au maximum, seulement la corde.
Entrées : IN, un son qui fait vibrer la corde ; PLK, une impulsion qui la pince ; 1V/O, pour l'accord ; DMP, pour changer la brillance.

## Experimente
1. Branchez OUT sur une entrée du MIXER, et la sortie EUC du CLOCK sur l'entrée PLK. La corde est pincée au rythme de l'horloge.
2. Tournez DECAY : des notes sèches à celles qui s'enchaînent.
3. Déplacez POS lentement et écoutez le timbre changer avec le point de jeu.
4. Branchez la sortie PTCH d'une SEQUENCE, menée par le même CLOCK, sur l'entrée 1V/O. La corde joue maintenant une ligne mélodique.
