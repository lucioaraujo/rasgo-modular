## O que é
SH, pour sample and hold, capture la valeur d'un signal au moment d'une impulsion et la garde jusqu'à l'impulsion suivante. Sans rien à l'entrée, il tire une nouvelle valeur au hasard à chaque impulsion. C'est la façon traditionnelle de transformer le hasard en marches : à chaque impulsion, une nouvelle note, un nouveau timbre. SH a deux canaux.

## Como pensar nele
Deux tirages indépendants n'ont rien à voir l'un avec l'autre, et c'est parfois ce qu'on veut. Mais une mélodie sonne souvent mieux quand le timbre accompagne la note sans la copier. Le réglage CORR établit justement cette relation entre les deux canaux : des canaux en miroir, où l'un monte quand l'autre descend, à des canaux indépendants, puis jumeaux, qui tirent la même valeur. Les réglages de glissement transforment les marches en glissandos.

## Controles
- RATE : le rythme interne des tirages, de 0,02 à 40 fois par seconde. Ne vaut que pour un canal sans impulsion sur T1 ou T2.
- SLW1, SLW2 : le temps que met chaque canal à glisser jusqu'à la nouvelle valeur. À zéro, il saute aussitôt.
- SLOPE : rend la montée plus rapide que la descente, ou l'inverse.
- TRK1, TRK2 : au lieu de capturer seulement à l'instant de l'impulsion, le canal suit l'entrée tant que l'impulsion est haute et garde la valeur quand elle retombe.
- SPRD : change la façon de tirer, rendant les petites variations plus fréquentes que les grandes.
- CORR : la relation entre les tirages des deux canaux : en miroir, indépendants ou identiques. Ne vaut que pour les canaux sans rien à l'entrée.
Entrées : IN1 et IN2, les signaux à capturer ; T1 et T2, les impulsions. Sorties : O1 et O2.

## Experimente
1. Montez une voix : la SAW d'un OSC sur l'entrée IN d'un FILTER, LO du FILTER dans le MIXER.
2. Branchez la sortie CLK du CLOCK sur T1 et sur T2.
3. Branchez O1 sur l'entrée CV d'un QUANTIZER, PTCH du QUANTIZER sur 1V/O de l'OSC, et O2 sur l'entrée FC du FILTER. Chaque impulsion apporte une note et un timbre.
4. Tournez CORR d'un bout à l'autre. D'un côté, les notes aiguës arrivent sombres ; au milieu, aucune relation ; de l'autre, les notes aiguës arrivent brillantes.
