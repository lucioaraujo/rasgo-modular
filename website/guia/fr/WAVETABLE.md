## O que é
WAVETABLE est un oscillateur qui garde une suite de 16 formes d'onde différentes, comme les images d'une animation, de la plus brillante (une dent de scie) à la plus douce (un sinus). Le bouton POS choisit où vous vous trouvez dans cette suite. En déplaçant POS, le timbre change de façon continue, sans filtre.

## Como pensar nele
Là où OSC donne un son fixe que l'on sculpte ensuite, WAVETABLE naît en mouvement : il suffit que quelque chose fasse bouger POS. Un LFO lent donne une nappe qui respire ; une enveloppe fait commencer chaque note brillante et finir sombre. Il sait aussi écouter : branché sur SIGNAL-IN, il capture un cycle du son entrant et l'utilise comme forme d'onde.

## Controles
- FREQ : la hauteur de la note, de 8 à 8000 Hz.
- FINE : l'accord fin, jusqu'à un demi-ton de chaque côté.
- POS : la position dans la table. Au début, le son est plein d'harmoniques ; à la fin, c'est presque un sinus. L'entrée POS s'ajoute à ce bouton, et c'est par elle que le timbre se met à bouger tout seul.
- WARP : déforme la lecture de chaque cycle et ajoute des harmoniques à l'accent numérique, sans changer d'image.
- FM : à quel point le signal sur l'entrée FM fait bouger la hauteur.
- DRIFT : une petite oscillation lente de l'accord.
Entrées : 1V/O pour la hauteur, POS pour déplacer la position, FM, CAP pour le son à capturer et GRAB, une impulsion qui déclenche la capture d'un nouveau cycle.

## Experimente
1. Branchez OUT sur une entrée du MIXER et tournez POS d'un bout à l'autre. Le timbre passe de rugueux à lisse.
2. Branchez la sortie d'un FUNCTION lent sur l'entrée POS. Le son change maintenant tout seul, par cycles.
3. Pour capturer : branchez SIGNAL-IN sur l'entrée CAP et une impulsion du CLOCK sur GRAB. À chaque impulsion, l'oscillateur se met à jouer un fragment du son entrant.
