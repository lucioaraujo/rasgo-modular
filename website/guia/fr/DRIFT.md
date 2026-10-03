## O que é
DRIFT produit des mouvements très lents, à l'échelle des minutes. Branché sur quelques réglages d'un patch, il fait évoluer la musique peu à peu d'elle-même : le timbre à trente secondes n'est plus celui de la quatrième minute, sans sauts et sans revenir au même point.

## Como pensar nele
Un patch que personne ne touche finit par tourner en rond. DRIFT est la couche du développement lent. À l'intérieur, une seule valeur se promène, et les quatre sorties A, B, C et D sont des versions de cette même promenade, chacune avec son poids. Elles racontent donc la même histoire sous des angles différents : quand l'une monte, les autres ont tendance à monter aussi. La promenade a de l'inertie : quand elle prend une direction, elle a tendance à la garder un moment.

## Controles
- RATE : le rythme de la promenade, d'un pas par seconde à un toutes les huit minutes environ.
- DEPTH : la portée, à quelle distance la promenade peut s'éloigner de son point de repos.
- MOMT : l'inertie. Haut, le chemin est doux et insiste dans une direction ; bas, il change de cap sans cesse.
- STRD : à zéro, les quatre sorties avancent presque ensemble ; au maximum, chacune part de son côté.
- ANCHR : la mémoire. À zéro, la promenade ne revient jamais ; haut, elle tend à repasser par des endroits déjà visités, ce qui donne à la pièce quelque chose comme des thèmes qui reviennent.
- BIAS : déplace le point de repos vers le haut ou vers le bas.
Entrées : ADV, une impulsion qui force un pas ; RATE, pour moduler la vitesse. Sorties : A, B, C et D, les quatre aspects de la promenade ; FLD, la promenade sans pondération ; EVT, une impulsion à chaque pas.

## Experimente
1. Montez une voix quelconque qui passe par un FILTER et un SPACE avant le MIXER.
2. Branchez A sur l'entrée FC du FILTER et B sur l'entrée FBK du SPACE. Montez DEPTH.
3. Laissez jouer quelques minutes. Le timbre et l'espace changent ensemble, sans hâte.
4. Pour suivre la forme de la musique, branchez la sortie EOS d'une SEQUENCE sur ADV : la promenade fait un pas à chaque fin de phrase.
