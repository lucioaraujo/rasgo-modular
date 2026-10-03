## O que é
MEMORY enregistre en continu les trois dernières secondes de ce qui y entre et rejoue cette matière en petits morceaux, appelés grains. Chaque grain est un fragment bref, et beaucoup de grains ensemble forment un nuage sonore qu'on peut transposer, étaler dans le temps ou figer. La technique s'appelle la synthèse granulaire.

## Como pensar nele
Avec MEMORY, le patch se met à se souvenir de ce qu'il a joué. Une phrase qui vient de sonner revient comme texture, plus aiguë ou plus grave, étirée, brouillée. Des grains courts et clairsemés donnent un rythme râpeux ; des grains longs et nombreux, un nuage lisse. Avec HOLD allumé, l'enregistrement s'arrête et le passage gardé devient une matière fixe que les grains continuent d'explorer. LOOPER répète des passages entiers ; MEMORY les défait en particules.

## Controles
- GRAIN : la durée de chaque grain, de 5 millisecondes à une demi-seconde. Court, une texture granuleuse ; long, presque le son original.
- DENS : combien de nouveaux grains naissent par seconde. Peu donnent des points isolés ; beaucoup, un nuage continu.
- POS : de quel endroit de l'enregistrement les grains sont lus, du plus récent au plus ancien.
- SPRAY : disperse le point de lecture de chaque grain, ce qui brouille le temps.
- PITCH : transpose les grains, jusqu'à deux octaves vers le haut ou le bas.
- FBK : renvoie le nuage dans l'enregistrement, et la matière s'accumule et s'use.
- BLEND : le mélange entre le son entrant et le nuage.
- HOLD : arrête l'enregistrement et fige la matière gardée.
Entrées : IN, le son ; POS, pour déplacer le point de lecture ; PTCH, pour transposer ; FRZ, pour figer avec un signal. Sortie : OUT.

## Experimente
1. Branchez une voix qui joue une mélodie sur l'entrée IN, et OUT sur une entrée du MIXER.
2. Mettez GRAIN vers 0,1 seconde, DENS près de 30 et écoutez la mélodie devenir nuage.
3. Montez SPRAY et mettez PITCH une octave plus haut.
4. Allumez HOLD. L'enregistrement s'arrête, et le nuage continue à partir du dernier passage gardé.
