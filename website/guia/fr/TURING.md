## O que é
TURING invente des mélodies au hasard et vous laisse garder celles qui vous plaisent. Il garde une boucle de quelques valeurs qui tourne à chaque impulsion ; à chaque tour, chaque valeur peut rester ou être remplacée par une autre. Avec le réglage LOCK, vous décidez à quel point la boucle change, du hasard total à la répétition fixe.

## Como pensar nele
Ici, on compose en choisissant à l'oreille, pas en écrivant note par note. Laissez LOCK au milieu, écoutez la mélodie se transformer et, quand quelque chose de bien apparaît, tournez LOCK jusqu'au bout : la boucle se bloque et se répète. Pour la faire varier de nouveau, desserrez un peu. Le nom rend hommage au module Turing Machine de Music Thing Modular, qui a popularisé l'idée. SEQUENCE joue une phrase écrite ; TURING fait naître des phrases et les laisse se fixer.

## Controles
- RATE : l'horloge interne, utilisée quand rien n'arrive sur CLK.
- LEN : la longueur de la boucle, de 2 à 16 pas.
- LOCK : la probabilité que la boucle reste la même. À zéro, tout est tiré à nouveau ; au maximum, la phrase se bloque et se répète.
- MUT : quand une valeur change, de combien elle change. Bas, de petites variations de ce qui était là ; haut, un nouveau tirage.
- RANGE : l'étendue des valeurs de sortie, c'est-à-dire la taille des sauts de la mélodie.
- STEPS : divise la sortie en marches. À 1, les valeurs sont continues.
- OFST : le centre des valeurs, qui monte ou descend le registre de la mélodie.
Entrées : CLK, l'impulsion qui fait tourner la boucle ; LOCK, pour moduler le blocage. Sorties : CV, la mélodie ; CV2, une autre lecture de la même boucle, apparentée mais différente ; PLS, un rythme tiré de la même boucle.

## Experimente
1. Branchez la sortie CLK du CLOCK sur l'entrée CLK de TURING.
2. Branchez CV sur l'entrée CV d'un QUANTIZER, et PTCH du QUANTIZER sur 1V/O d'un OSC qui passe par un ENVELOPE jusqu'au MIXER. Branchez PLS sur l'entrée GATE de l'enveloppe.
3. Avec LOCK au milieu, écoutez la mélodie changer peu à peu.
4. Quand ce que vous entendez vous plaît, poussez LOCK au maximum. La phrase reste.
