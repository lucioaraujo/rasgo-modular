## O que é
PLL est un oscillateur qui sait en suivre un autre. Seul, il fonctionne comme n'importe quel oscillateur, avec sa propre hauteur et une forme d'onde que vous choisissez. Quand il reçoit un second signal sur l'entrée REF, il ajuste peu à peu sa vitesse jusqu'à marcher avec ce signal, comme un musicien qui se cale sur un autre.

## Como pensar nele
OSC peut aussi s'accrocher à un autre oscillateur, mais d'un seul coup (c'est la synchro). PLL le fait progressivement, et ce chemin vers l'accrochage s'entend : un glissement de hauteur. Avec RATIO, il s'accroche une octave au-dessus, une en dessous, ou sur des divisions plus profondes de la référence. La sortie LOCK indique à quel point il est accroché, et peut faire réagir d'autres modules.

## Controles
- FREQ : la hauteur sans référence, et le point de départ quand il y en a une.
- FINE : l'accord fin, jusqu'à un demi-ton de chaque côté.
- SHAPE : la forme d'onde, du sinus au triangle, à la dent de scie et au carré.
- RATIO : le rapport à la référence sur lequel il s'accroche. À 1, la même note ; à 2, une octave au-dessus ; près du minimum, très en dessous.
- LOCK : la force avec laquelle il poursuit la référence. Bas, il glisse lentement jusqu'à arriver ; haut, il s'accroche vite, mais devient plus nerveux.
- FM : à quel point le signal sur l'entrée FM fait bouger la hauteur.
- FBK : quelle part de son propre signal revient dans le circuit, ajoutant de la texture sans désaccorder.
- FTYP : choisit entre six types de retour, chacun avec sa couleur.
Entrées : 1V/O pour la hauteur, FM et REF, la référence à suivre. Sorties : OUT, RING (le PLL multiplié par la référence) et LOCK, un signal de contrôle qui indique à quel point il est accroché.

## Experimente
1. Branchez OUT sur une entrée du MIXER. Sans rien sur REF, c'est un oscillateur ordinaire ; tournez SHAPE pour entendre les formes.
2. Branchez la sortie SAW d'un OSC sur l'entrée REF. Le PLL cherche la hauteur de l'OSC et s'y accroche.
3. Baissez LOCK. Il met maintenant du temps à arriver, et vous entendez la hauteur glisser.
4. Mettez RATIO à 2. Il s'accroche une octave au-dessus de l'OSC.
