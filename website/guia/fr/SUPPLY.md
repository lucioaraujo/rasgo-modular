## O que é
SUPPLY est l'alimentation du patch, mise dans un module avec des câbles. Dans un circuit, quand la pile faiblit, tout change ensemble : l'horloge ralentit, le son perd du corps et les oscillateurs se désaccordent les uns des autres. SUPPLY fait entrer cette cause commune dans le patch.

## Como pensar nele
ENERG est la tension d'alimentation. La charge, le son du patch lui-même, fait céder cette tension, comme une pile à l'effort : plus le patch sonne, plus le rail s'enfonce. Ce qui reste sort sur TRILHO, à brancher où vous voulez. Les dérives D1 à D4 n'apparaissent qu'avec le rail bas et font se désaccorder les voix ; avec le rail plein, elles reviennent ensemble. Rien n'est caché : SUPPLY n'agit que là où il y a un câble.

## Controles
- ENERG : la tension d'alimentation.
- QUEDA : combien la charge fait céder le rail.
- TEMPO : le temps que met le rail à céder et à revenir, de 0,05 à 5 s.
- DERIVA : combien les voix se désaccordent avec le rail bas.
Entrées : CARGA reçoit le son du patch, ENERG déplace la tension par CV. Sorties : TRILHO est la tension qui reste, D1 à D4 quatre dérives d'accord (jusqu'à 60 cents), FALHA une impulsion à la panne et une autre au retour.

## Experimente
1. Branchez la sortie du MIXER dans CARGA et TRILHO dans le CV1 du VCA d'une voix : le patch perd du corps quand il joue fort.
2. Branchez D1 dans le 1V/O d'un OSC et D2 dans celui d'un autre. Baissez ENERG : les voix se désaccordent l'une de l'autre.
3. Remontez ENERG : elles reviennent ensemble.
4. Branchez FALHA dans le CIT de l'APOCRIFO : la panne déchire le temps.
