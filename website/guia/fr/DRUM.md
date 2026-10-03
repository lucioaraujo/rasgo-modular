## O que é
DRUM est une voix de percussion prête à l'emploi : à chaque impulsion sur l'entrée GATE, il joue un coup. Avec peu de boutons, il va de la grosse caisse à la caisse claire, au tom et au charleston, et change de caractère entre le son des anciennes boîtes à rythmes et un son plus acoustique.

## Como pensar nele
Construire une grosse caisse de zéro demande plusieurs modules ; DRUM en fournit une toute prête. Il lui faut quelqu'un qui dise quand jouer : un CLOCK ou, mieux, un TRIGSEQ, qui génère des motifs rythmiques sur quatre lignes. Quatre DRUM, chacun sur une ligne du TRIGSEQ, forment une batterie. Et comme TONE accepte une hauteur par l'entrée PIT, on peut jouer une ligne de toms.

## Controles
- TONE : la hauteur du corps du coup, de 20 à 1000 Hz. Grave, c'est une grosse caisse ; médium, un tom ou une caisse claire ; aigu, quelque chose comme une clave.
- BEND : de combien la hauteur chute juste après le coup. Haut, cela donne le poids de la grosse caisse ; zéro garde le coup sur une seule hauteur.
- DECAY : la durée, d'un clic bref à un grave qui tient.
- SNAP : la quantité de claquement à l'attaque, qui donne du corps à la caisse claire et au charleston.
- MAP : le caractère, en passant par des timbres de boîtes à rythmes jusqu'à un son acoustique.
- DRIVE : sature la sortie et rend le coup plus agressif.
- ROLL : fait se déclencher le module tout seul, d'un roulement lent à un bourdonnement. À zéro, il ne joue que s'il reçoit une impulsion.
- DRIFT : varie légèrement chaque coup, pour que la répétition sonne jouée et non programmée.
Entrées : GATE, l'impulsion qui déclenche le coup ; ACC, pour accentuer ; PIT, pour la hauteur.

## Experimente
1. Branchez OUT sur une entrée du MIXER, et la sortie CLK du CLOCK sur l'entrée GATE. Vous entendez une grosse caisse qui marque le temps.
2. Montez TONE et SNAP, baissez DECAY : la grosse caisse devient une caisse claire sèche.
3. Montez ROLL sans rien sur GATE : le module joue tout seul, en roulement.
4. Branchez le CLOCK sur un TRIGSEQ et ses sorties T1 et T2 sur deux DRUM réglés différemment. Vous avez une petite batterie.
