## O que é
SUM réunit des signaux. Quatre entrées, chacune avec son atténuverteur, deviennent une seule sortie : la somme, la somme inversée et la moyenne.

## Como pensar nele
Dans Rasgo Modular chaque entrée reçoit un seul câble. Pour qu'un filtre bouge avec deux mouvements à la fois, les deux passent d'abord par SUM. MULT copie un signal vers plusieurs endroits ; SUM réunit plusieurs signaux en un seul endroit.

## Controles
- A1 à A4 : quelle part de chaque entrée entre dans la somme, de −2 à +2. Au milieu, l'entrée reste de côté ; sous le milieu, elle entre inversée.
- OFS : une valeur fixe ajoutée au résultat.
- SAT : allumé, la somme sature doucement et ne dépasse jamais ±1, bien pour sommer de l'audio.
Entrées : IN1 à IN4. Sorties : SUM, la somme ; INV, la somme inversée ; AVG, la moyenne des entrées branchées.

## Experimente
1. Branchez le BI d'un FUNCTION sur IN1 et une sortie d'un DRIFT sur IN2. Branchez SUM sur le cutoff d'un FILTER : le filtre bouge avec les deux.
2. Tournez A2 sous le milieu : le DRIFT pousse maintenant le filtre dans l'autre sens.
3. Branchez deux oscillateurs sur IN1 et IN2, allumez SAT et envoyez SUM au MIXER : les deux jouent ensemble sans écrêter.
