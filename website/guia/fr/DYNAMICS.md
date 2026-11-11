## O que é
DYNAMICS est un compresseur. Il retient les crêtes pour que le son soit plus ramassé, et il laisse un son en baisser un autre, comme la nappe qui respire quand la grosse caisse frappe. Ici il fait une troisième chose : ce qu'il entend et combien il réduit sortent en signaux de contrôle, pour faire bouger n'importe quel autre module.

## Como pensar nele
Pensez à un ingénieur du son la main sur le fader : quand le son dépasse le seuil, la main baisse ; quand le son retombe, elle rend le niveau. RATIO est la force de la main, ATK et REL sa hâte. L'entrée SIDE change ce qu'il écoute : avec une grosse caisse branchée là, c'est elle qui décide quand le reste baisse. Deux écarts Rasgo touchent au temps : BRTH fait suivre au relâchement l'intervalle entre les coups, et DRIFT fait varier attaque et relâchement à chaque coup.

## Controles
- THRES : le seuil, de −48 à 0 dB. Au-dessus, le son est baissé.
- RATIO : combien il baisse. À 4, chaque 4 dB au-dessus du seuil devient 1 ; à 20, il devient un limiteur.
- KNEE : d'un coup (0) ou peu à peu (jusqu'à 12 dB).
- ATK, REL : le temps de descendre (0,1 à 200 ms) et celui de revenir (5 ms à 2 s).
- GAIN : rend le niveau perdu, jusqu'à 24 dB.
- SIDE : le détecteur écoute l'entrée IN (0) ou l'entrée SIDE (1).
- RMS : mesure l'énergie moyenne au lieu de la crête ; le son est plus doux.
- GATE : inverse la courbe et en fait un noise gate, qui ferme ce qui reste sous le seuil.
- BRTH : le relâchement suit le tempo des coups qu'il entend.
- DRIFT : un hasard semé dans l'attaque et le relâchement, coup par coup.
Entrées : IN reçoit le son, SIDE ce que le détecteur doit entendre, THR déplace le seuil. Sorties : OUT est le son compressé, ENV le niveau qu'il entend, GR combien il réduit en ce moment.

## Experimente
1. Branchez un TRIGSEQ dans un DRUM et le DRUM dans l'entrée SIDE. Branchez un CHORD dans IN et OUT dans le MIXER.
2. Laissez SIDE à 1, RATIO à 6, ATK court et REL vers 200 ms : l'accord baisse à chaque coup de grosse caisse.
3. Montez BRTH : le relâchement se met à respirer avec le rythme.
4. Branchez la sortie ENV dans le CUT d'un FILTER sur un autre son : le filtre s'ouvre avec la force de ce que le compresseur entend.
