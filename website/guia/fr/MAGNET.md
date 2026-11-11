## O que é
MAGNET est une grille de 8 sur 8 petits aimants, chacun pointant vers le haut ou vers le bas. Chaque aimant veut s'aligner sur ses voisins, et la température les agite. Une tête parcourt la grille, une case par impulsion, et l'aimant sous elle donne le gate.

## Como pensar nele
C'est le modèle d'Ising, de la physique des matériaux. Sous la température critique (vers 2,27), de grands domaines se forment et le rythme est stable. Au-dessus, le bruit l'emporte. Tout près, des domaines de toutes tailles apparaissent et disparaissent : c'est le bord du chaos, et c'est là que le module sonne le plus vivant. Un COUPL négatif fait s'opposer les aimants à leurs voisins, comme un échiquier.

## Controles
- TEMP : la température ; la transition est vers 2,27.
- COUPL : le couplage. Positif, les aimants s'alignent ; négatif, ils s'opposent.
- FIELD : un aimant extérieur qui pousse toute la grille d'un côté.
- SWEEP : combien de tentatives de retourner des aimants à chaque impulsion.
- RATE : l'horloge interne, quand rien n'est branché sur CLK.
- GATE : la durée de chaque gate.
Entrées : CLK avance d'une impulsion, QNCH chauffe tout d'un coup et la grille se réorganise, TEMP et FLD déplacent température et champ par CV. Sorties : GT est le gate de la tête, MAG l'aimantation, ENER l'énergie de la grille, FLIP la part de la grille qui a basculé à cette impulsion.

## Experimente
1. Branchez GT dans un DRUM. Avec TEMP à 1, écoutez un rythme presque figé ; à 4, un rythme de bruit.
2. Mettez TEMP vers 2,3 : des domaines apparaissent et disparaissent, et le rythme respire.
3. Branchez un déclenchement rare dans QNCH : la grille se défait et se réorganise.
4. Branchez MAG dans la coupure d'un FILTER : le timbre suit le côté vers lequel penche la grille.
