## O que é
GENDYN est une source sonore où l’onde est un polygone de quelques points. À chaque tour, chaque point change un peu de hauteur et de distance jusqu’au suivant, au hasard. L’onde ne se répète jamais, mais ne s’échappe jamais non plus : un son qui vit.

## Como pensar nele
C’est la synthèse stochastique dynamique de Iannis Xenakis, celle de Gendy3, de 1991. Chaque point fait un pas au hasard, comme un ivrogne, et les bords le réfléchissent, pour qu’il ne s’y colle jamais. Avec BAND à 0, la hauteur est tenue et seul le dessin de l’onde respire : une note juste et vivante. Avec BAND à 1, la période erre aussi, et le son va jusqu’au bruit. DIST choisit le type de pas ; la distribution de Cauchy, la préférée de Xenakis, donne des sauts rares et grands.

## Controles
- PTS : combien de points compte le polygone.
- FREQ : la hauteur centrale.
- BAND : jusqu’où la hauteur peut s’échapper.
- AMP : le pas des amplitudes.
- DUR : le pas des durées.
- DIST : le type de pas.
- LEVEL : le niveau.
Entrées : PTCH ajoute une hauteur en volts par octave, FRZ fige l’onde tant que le gate est haut, AMP s’ajoute au pas des amplitudes. Sortie : OUT est le son.

## Experimente
1. Mettez BAND à 0,1 et branchez OUT sur un FILTER et un VCA : une basse qui respire.
2. Montez BAND et AMP à 1 : la masse de bruit de Xenakis.
3. Branchez un gate lent sur FRZ : l’onde se fige et se dégèle.
4. Branchez une séquence sur PTCH avec BAND bas : une mélodie de timbres vivants.
