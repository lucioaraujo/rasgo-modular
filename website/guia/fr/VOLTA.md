## O que é
VOLTA est une boucle de pas qui varie sans se perdre. Elle garde une séquence de base et la transforme par-dessus ; quand vous revenez sur le bouton, la séquence d'origine revient exactement comme elle était.

## Como pensar nele
TURING réécrit sa propre boucle, et ce qui change se perd. VOLTA ne réécrit jamais la base : chaque pas a son tour de changer, toujours dans le même ordre, et on peut donc aller et revenir. Seule une impulsion sur NEW remplace la base par une nouvelle.

## Controles
- RATE : la vitesse de l'horloge interne, quand il n'y a pas de CLK.
- LEN : combien de pas a la boucle.
- MUT : combien de pas passent à leur valeur alternative.
- ERODE : combien de pas se taisent.
- MEM : les pas protégés, qui ne changent ni ne se taisent.
- RANGE : l'amplitude de la CV.
- STEPS : en combien de paliers la CV est quantifiée.
- OFST : un décalage ajouté à la CV.
- SEED : la graine de la boucle et de l'ordre des changements.
Entrées : CLK, l'horloge ; RST, retour au premier pas ; NEW, une nouvelle base ; MUT et ERO, des signaux ajoutés aux boutons. Sorties : CV et CV2, les hauteurs ; PULSE, les gates ; POS, la position dans la boucle.

## Experimente
1. Branchez le CLK de CLOCK sur le CLK de VOLTA, la CV sur un QUANTIZER puis sur un OSC, et PULSE sur un ENVELOPE.
2. Montez MUT lentement : la mélodie varie. Ramenez MUT à zéro : elle revient entière.
3. Branchez un FUNCTION lent sur MUT : la mélodie s'éloigne et revient toute seule.
