## O que é
PREDATOR est un petit écosystème : des proies qui croissent seules et des prédateurs qui croissent en mangeant. Les deux populations montent et descendent en cycle, le prédateur toujours un peu en retard, et sortent en CV sur PREY et PRED.

## Como pensar nele
Ce sont les équations de Lotka et Volterra, des années 1920, la même physique que l’écosystème de l’Aquorbium. Quand les proies abondent, le prédateur croît ; quand les prédateurs abondent, la proie chute ; alors le prédateur a faim, et tout recommence. Les captures donnent des impulsions sur CTCH, et le pic de la proie, la fuite, donne une impulsion sur FLEE. CANON répète la ligne de la proie avec un retard qui rétrécit quand le prédateur s’approche : un canon qui se resserre jusqu’à la strette.

## Controles
- PERIOD : la durée d’un cycle.
- SIZE : la taille de l’orbite.
- HUNT : combien le prédateur chasse.
- WILD : le bruit de l’environnement.
- DELAY : le retard maximal du canon.
Entrées : CTCH est une capture venue de l’extérieur, RATE bouge la vitesse en volts par octave. Sorties : PREY et PRED sont les populations, CTCH et FLEE les impulsions de la chasse et de la fuite, CANON le canon de la proie.

## Experimente
1. Branchez PREY sur la coupure d’un FILTER et PRED sur le niveau d’un VCA : le timbre et la dynamique se chassent.
2. Branchez CTCH sur un DRUM avec HUNT haut : les captures deviennent des coups dans les moments d’abondance.
3. Branchez PREY et CANON sur les hauteurs de deux voix : la seconde imite la première de plus en plus près.
4. Montez WILD : les cycles cessent d’être pareils, comme dans un vrai écosystème.
