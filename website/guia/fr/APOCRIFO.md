## O que é
APOCRIFO fait entrer le passé lointain dans le présent, comme une rupture. Le LOOPER répète le maintenant ; l'APOCRIFO cite ce qui a sonné il y a au moins IDADE secondes. Il enregistre l'entrée, jamais sa propre sortie : l'apocryphe ne se cite pas lui-même.

## Como pensar nele
Il y a trois façons de citer. Rongée : le passage revient à l'envers et rongé, le présent baissé en dessous, comme une mémoire corrompue. Coupe : une coupe de montage, le passage revient net une fois et les raccords tombent sur les passages par zéro, comme la lame du monteur de bande. Parenthèse : le maintenant bégaie un instant tandis que le patch continue en dessous, et revient exactement là où le patch est déjà. La citation vient d'un déclenchement sur CIT ou d'elle-même, par CHANCE.

## Controles
- MODO : rongée (0), coupe (1) ou parenthèse (2).
- IDADE : de combien loin vient la citation, de 5 à 55 s.
- DUR : la durée du passage cité.
- CHANCE : la fréquence des citations spontanées ; à 0, seulement par déclenchement.
- PROF : la profondeur de la citation sur le présent.
- SEED : la graine qui choisit les passages.
Entrées : IN reçoit le son, CIT déclenche une citation. Sorties : OUT est le son avec les citations, ATIVO un gate pendant que la citation sonne, IDADE de combien loin elle vient.

## Experimente
1. Branchez la sortie du MIXER dans IN et OUT dans le MASTER.
2. Mettez CHANCE à 0,3, MODO sur rongée et IDADE à 30 : de temps en temps, la pièce d'il y a une demi-minute déchire le milieu.
3. Passez en mode coupe et branchez le COL du COHERENCE dans CIT : quand la ligne devient trop prévisible, la pièce coupe vers le passé.
4. Branchez la sortie IDADE dans la coupure d'un FILTER : plus la citation est ancienne, plus elle est sombre.
