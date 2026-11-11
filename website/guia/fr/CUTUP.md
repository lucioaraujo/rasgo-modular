## O que é
CUTUP découpe le son du patch dans le tempo. Il garde les quatre dernières secondes et, quand on l'appelle, fait un geste de montage sur les derniers temps de l'horloge : un bégaiement, une rafale mélangée, une répétition microscopique ou un silence.

## Como pensar nele
C'est la technique du cut-up de William Burroughs et du collage sur bande, mise sur le bus. Les gestes arrivent par gate, chacun sur son entrée, ou d'eux-mêmes par CHANCE. Chaque fragment a des bords doux de 3 ms, et le geste entier aussi, pour que la coupe ne claque pas.

## Controles
- CHANCE : la fréquence des gestes spontanés ; à 0, seulement par gate.
- MICRO : combien de répétitions a le geste microscopique, de 2 à 8.
- MIX : l'équilibre entre le son d'origine et le son découpé.
- SEED : la graine qui choisit gestes et tranches.
Entrées : IN reçoit le son, CLK l'horloge du patch, STUT déclenche le bégaiement, BURST la rafale de tranches mélangées, MICRO la répétition microscopique, GAP un temps de silence. Sorties : OUT est le son découpé, ATIVO un gate pendant qu'un geste sonne.

## Experimente
1. Branchez la sortie du MIXER dans IN, OUT dans le MASTER et le CLOCK dans CLK.
2. Mettez CHANCE à 0,15 : de temps en temps, le patch se découpe pile dans le temps.
3. Branchez la sortie euclidienne du CLOCK dans STUT : le bégaiement tombe sur les temps du rythme.
4. Branchez l'EOS d'un SEQUENCE dans GAP : chaque phrase finit sur un silence coupé.
