## O que é
REFRACT est un rayon lumineux qui naît dans une couche de verre, en traverse trois autres et sort dans l’air. Dans chaque couche il dévie, et son angle dans chacune sort en CV sur A1 à A4.

## Como pensar nele
C’est la loi de Snell : en passant d’un milieu à un autre, le rayon change de direction selon leurs indices. Passant d’un milieu dense à un milieu moins dense sous un angle trop rasant, il ne traverse pas : il se réfléchit entièrement. C’est la réflexion totale, l’éclat du diamant et le chemin de la lumière dans la fibre optique. Chaque fois que le rayon se met à se réfléchir, une impulsion sort sur TIR.

## Controles
- N1, N2, N3, N4 : les indices des quatre couches.
- RATE : la vitesse du balayage de l’angle.
- RANGE : jusqu’où l’angle balaie.
- SHIM : le mirage, le tremblement des indices.
Entrées : ANG donne l’angle par CV, à la place du balayage, IDX s’ajoute à tous les indices. Sorties : A1 à A4 sont les angles dans les couches, TIR l’impulsion de la réflexion totale, EXIT le déplacement latéral du rayon.

## Experimente
1. Branchez TIR sur un DRUM : le balayage passe par l’angle critique deux fois par cycle, et le DRUM marque chaque passage.
2. Branchez A2 et A4 sur deux coupures de FILTER : le même rayon donne deux mouvements semblables et différents.
3. Montez SHIM avec RANGE près de l’angle critique : TIR devient un rythme de mirage.
4. Branchez un LFO sur IDX : le verre chauffe et refroidit, et le point de réflexion totale se déplace.
