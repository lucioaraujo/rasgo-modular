## O que é
COMPARADOR mélange deux signaux avec polarité et décide quand le mélange franchit un seuil. Les boutons A et B ont le zéro au centre : vers la droite l'entrée s'ajoute, vers la gauche elle entre inversée. La sortie COMP est un gate qui reste haut tant que le mélange est au-dessus du seuil.

## Como pensar nele
C'est une balance. Avec A à droite et B à gauche, le mélange est la différence entre les deux signaux ; avec le même signal des deux côtés, il s'annule. La décision vient ensuite : le gate s'ouvre quand la balance penche d'un côté. L'hystérésis l'empêche de trembler quand le signal est bruité près du seuil.

## Controles
- A, B : combien chaque entrée entre, de −1 (inversée) à 1.
- OFS : décale tout le mélange vers le haut ou vers le bas.
- LIM : le seuil de la décision.
- HIST : l'hystérésis, une marge autour du seuil.
Entrées : A et B reçoivent les signaux, LIM déplace le seuil par CV. Sorties : MIX est le mélange, INV le mélange à l'envers, COMP le gate de la décision, CRUZ une impulsion à chaque changement de côté.

## Experimente
1. Branchez deux FUNCTION lents, à des vitesses différentes, dans A et B. Mettez A à droite et B à gauche.
2. Branchez COMP dans le gate d'un ENVELOPE et CRUZ dans un DRUM : deux mouvements lents qui se croisent deviennent rythme.
3. Branchez le même signal dans A et B avec A à 1 et B à −1 : le mélange disparaît.
4. Branchez le LVL d'un SCOPE dans A et mettez LIM au milieu : quand celui qui joue dépasse un certain volume, une autre voix entre.
