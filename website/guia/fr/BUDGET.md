## O que é
BUDGET donne aux voix un budget d'événements. Quatre gates entrent et sortent, mais par cycle seuls TETO événements passent au total. Quand le budget est épuisé, les gates suivants restent dehors jusqu'au prochain cycle.

## Como pensar nele
La densité n'est pas la qualité. BUDGET oblige le patch à choisir : avec peu de budget, chaque note vaut plus. Il y a deux économies. Concurrence : le premier arrivé prend. Coopération : chaque voix qui joue a sa part, et ne la dépasse que si les autres n'ont pas encore dépensé la leur.

## Controles
- TETO : combien d'événements tiennent dans chaque cycle, de 1 à 32.
- COOP : coupé, les voix sont en concurrence ; activé, elles coopèrent.
- CICLO : la durée du cycle, de 0,25 à 16 s, quand rien n'est branché sur CLK.
Entrées : G1 à G4 reçoivent les gates des voix, CLK recommence le cycle à chaque impulsion. Sorties : O1 à O4 sont les gates qui sont passés, SALDO ce qui reste du budget, FIM une impulsion quand il est épuisé.

## Experimente
1. Branchez les gates de deux voix dans G1 et G2, et O1 et O2 dans leurs enveloppes.
2. Avec TETO à 6, écoutez les voix se disputer le cycle.
3. Activez COOP : les deux se mettent à partager le budget.
4. Branchez SALDO dans la coupure d'un FILTER : le son s'assombrit à mesure que le budget se dépense.
