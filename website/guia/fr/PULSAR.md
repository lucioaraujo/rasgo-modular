## O que é
PULSAR produit un train de petites impulsions sonores, chacune suivie d'un silence. Quand les impulsions se répètent vite, vous entendez une note ; quand elles ralentissent, un rythme. La technique vient du compositeur Curtis Roads et a une propriété curieuse : hauteur et timbre se règlent séparément, si bien qu'on peut changer la couleur du son sans le désaccorder.

## Como pensar nele
FREQ décide combien de fois par seconde l'impulsion se répète, donc la note. FRMT décide de ce qui se passe à l'intérieur de chaque impulsion, donc le timbre : bas, il sonne creux ; haut, nasillard. Descendez FREQ vers une trentaine de répétitions par seconde et chaque impulsion devient un événement qu'on entend séparément ; avec MASK et JITR, ces événements deviennent irréguliers, comme un nuage.

## Controles
- FREQ : le taux de répétition des impulsions, de 20 à 2000 Hz. C'est la hauteur de la note.
- FRMT : la fréquence à l'intérieur de chaque impulsion, de 0,1 à 8 fois FREQ. C'est le timbre : bas, creux ; haut, brillant.
- SHAPE : la forme de chaque impulsion, d'un simple sinus à une impulsion plus étroite et riche en harmoniques.
- WIND : le contour de chaque impulsion. Vers la gauche, les bords sont durs et le son brillant ; au milieu, il est propre ; vers la droite, chaque impulsion attaque vite et décroît, comme une percussion.
- JITR : rend irréguliers le moment et le volume de chaque impulsion. À zéro, le train est rigide.
- MASK : la probabilité que chaque impulsion soit sautée. Ouvre des trous et crée des motifs rythmiques sans changer la note.
- SPRD : répartit les impulsions alternées entre gauche et droite, ce qui élargit le son en stéréo.
- LEVEL : le volume de sortie.
Entrées : PIT pour la hauteur et FQM pour moduler le timbre. Sorties L et R, pour la stéréo.

## Experimente
1. Branchez L sur une entrée du MIXER et tournez FRMT lentement. Le timbre change, mais la note reste à sa place.
2. Amenez FREQ près du minimum. La note se défait en impulsions qu'on entend une à une.
3. Avec FREQ bas, montez MASK et JITR. Les impulsions deviennent clairsemées et irrégulières, comme une pluie.
4. Branchez la sortie BI d'un FUNCTION lent sur l'entrée FQM. Le timbre se promène tout seul sans se désaccorder.
