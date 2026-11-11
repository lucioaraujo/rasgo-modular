## O que é
ORBIT est un petit système solaire : un soleil au centre et deux ou trois planètes en gravité. Les positions des planètes sortent en CV, et leurs rencontres sortent en impulsions.

## Como pensar nele
Ce sont les lois de Newton et de Kepler qui jouent le patch. Les orbites plus larges prennent plus de temps, donc les planètes avancent selon des rythmes qui s'alignent presque puis se séparent. MASS fait s'attirer les planètes entre elles : faible, les orbites sont régulières ; forte, elles se défont en chaos. Une planète qui s'échappe revient sur son orbite, donc le système ne se perd jamais. LIGHT retarde l'impulsion de conjonction selon la distance, comme la lumière qui met du temps à arriver.

## Controles
- PLNTS : combien de planètes, 2 ou 3.
- RATIO : à quel point les orbites s'écartent les unes des autres.
- ECC : l'aplatissement des ellipses.
- MASS : combien les planètes s'attirent entre elles.
- SPEED : la vitesse du temps du système.
- LIGHT : le retard de la conjonction selon la distance, jusqu'à environ 1 s.
Entrées : NUDGE pousse la première planète et change la forme de son orbite, SPD déplace la vitesse par CV. Sorties : X1 et Y1 sont la position de la première planète, X2 et Y2 celle de la seconde, DIST la distance entre elles, CONJ une impulsion à la conjonction, PERI une impulsion quand la première planète passe au plus près du soleil.

## Experimente
1. Branchez CONJ dans un DRUM et PERI dans un autre : deux rythmes lents qui se répètent presque.
2. Branchez X1 dans un QUANTIZER et le QUANTIZER dans un OSC : une mélodie en orbite.
3. Montez MASS lentement : les orbites se défont et le rythme devient imprévisible.
4. Branchez DIST dans le MIX d'un SPACE : quand les planètes s'éloignent, l'espace s'ouvre.
