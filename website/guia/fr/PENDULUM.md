## O que é
PENDULUM est un double pendule : un pendule avec un autre suspendu à son extrémité. Avec peu d'énergie, il se balance presque comme une horloge ; avec beaucoup, le bras du bas fait des tours et le mouvement ne se répète jamais. Les positions des deux extrémités sortent en CV.

## Como pensar nele
C'est l'exemple classique du chaos : une loi simple, un résultat imprévisible. ENRGY est le bouton du chaos. En bas, l'ordre ; en haut, des tours. CROSS marque le rythme du balancement, et FLIP chaque tour complet, l'instant où le chaos apparaît.

## Controles
- ENRGY : l'énergie du balancement ; au-delà de la moitié, le chaos apparaît.
- RATIO : le rapport entre les longueurs des deux bras.
- MASS : le rapport entre les masses.
- SPEED : la vitesse du temps du pendule.
- DAMP : le frottement ; avec lui, le pendule perd peu à peu sa force.
- AUTO : donne une poussée quand le pendule s'arrête presque.
Entrées : KICK donne une poussée, SPD déplace la vitesse par CV. Sorties : X1 et Y1 sont l'extrémité du premier bras, X2 et Y2 celle du second, CROSS un gate à chaque passage par le centre, FLIP un gate à chaque tour complet.

## Experimente
1. Branchez X2 dans un QUANTIZER et le QUANTIZER dans un OSC, et CROSS dans un ENVELOPE qui commande le VCA de la voix : une mélodie qui se balance.
2. Montez ENRGY lentement et écoutez la mélodie passer du régulier à l'imprévisible.
3. Branchez FLIP dans l'ADV du HARMONY : l'accord change quand le chaos apparaît.
4. Branchez X1 dans le pan d'une voie du MIXER : le son tourne dans l'espace.
