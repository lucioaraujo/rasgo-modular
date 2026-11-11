## O que é
PRISM décompose le son en couleurs, comme un prisme décompose la lumière. Il y a six bandes, du grave à l'aigu, chacune retardée par son propre indice et placée quelque part dans la stéréo. Un coup devient un arc-en-ciel qui s'écoule dans le temps.

## Como pensar nele
Dans le verre, chaque couleur avance à sa propre vitesse. Ici DISP est l'épaisseur du verre et CURVE le sens : positive, les aigus arrivent après, comme pour la lumière ; négative, ce sont les graves, ce que le verre ne fait pas. FDBK renvoie chaque couleur dans le prisme, et les échos se séparent davantage à chaque tour.

## Controles
- DISP : le retard maximal entre les couleurs, jusqu'à 400 ms.
- CURVE : le sens et la forme de la dispersion, de −1 à 1.
- SPRD : à quel point les couleurs s'étalent dans la stéréo.
- FDBK : la part de chaque couleur qui retourne dans le prisme.
- MIX : l'équilibre entre le son d'origine et le son dispersé.
Entrées : IN reçoit le son, DISP déplace la dispersion par CV. Sorties : OUT est le mélange, WET le son dispersé seul.

## Experimente
1. Passez un DRUM dans le PRISM avec CURVE à 1, DISP à 200 ms et SPRD au maximum : chaque coup s'écoule du grave à l'aigu, d'un côté à l'autre.
2. Mettez CURVE à −1 et comparez.
3. Montez FDBK à 0,7, avec DISP à 80 ms et MIX à 0,6 : des échos colorés.
4. Branchez un LFO lent dans DISP : l'arc-en-ciel respire.
