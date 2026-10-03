## O que é
PLANAR mélange quatre sons placés aux coins d'un carré. Un point se déplace dans le carré, et le son de sortie est le mélange des quatre, pondéré par la distance du point à chaque coin. En déplaçant le point lentement, le timbre se transforme d'un son en un autre. La technique s'appelle la synthèse vectorielle.

## Como pensar nele
Pensez à un joystick : X déplace le point de gauche à droite, Y de bas en haut. Vous pouvez le déplacer à la main, avec deux ondes lentes, ou avec un DRIFT, et le point dessine des figures. La position sort aussi par les sorties X' et Y', si bien que le même mouvement peut conduire d'autres réglages du patch. Avec une longue impulsion sur l'entrée GST, PLANAR enregistre le trajet que vous faites avec les boutons X et Y, puis le répète sans fin.

## Controles
- X, Y : la position du point dans le carré. Les signaux des entrées du même nom s'ajoutent à ces valeurs.
- CURVE : la manière de mélanger. À un bout, le mélange est linéaire, bien pour des signaux de contrôle ; à l'autre, il garde le volume constant, et le son ne se creuse pas quand le point est au milieu.
- SMTH : fait glisser le point vers sa nouvelle position au lieu de sauter.
- RATE : la vitesse de répétition du geste enregistré et de la promenade de DRIFT.
- DRIFT : fait se promener le point tout seul dans le carré.
Entrées : A, B, C et D, les quatre sons ; X et Y, pour déplacer le point ; GST, pour enregistrer et répéter un geste. Sorties : OUT, le mélange ; X' et Y', la position du point.

## Experimente
1. Branchez quatre sources différentes sur A, B, C et D : par exemple la SAW d'un OSC, OUT d'un WAVETABLE, OUT du CHORD et PNK d'un NOISE. Branchez OUT de PLANAR dans le MIXER.
2. Déplacez X et Y à la main et écoutez le son passer d'un coin à l'autre.
3. Branchez BI de deux FUNCTION lents, à des vitesses différentes, sur les entrées X et Y. Le point dessine une figure et le timbre ne cesse de changer.
4. Branchez Y' sur l'entrée FC d'un FILTER sur le chemin du son : le filtre suit le mouvement.
