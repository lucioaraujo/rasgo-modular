## O que é
ACCENT donne son phrasé à une ligne. Un interprète n'accentue pas qu'avec le volume : la note accentuée est plus forte, plus brillante et plus courte, et il y a bien des raisons de l'accentuer. ACCENT réunit ces causes, chacune avec son poids, en une emphase de 0 à 1, et envoie cette emphase à plusieurs sorties à la fois : niveau, brillance, durée, balayage.

## Como pensar nele
Il y a six causes d'accent : le temps fort (METR), la taille du saut (SALTO), la note la plus aiguë de la phrase récente (APICE), l'accent venu de l'extérieur (EXT), la note après une note longue (AGOG) et un accent extérieur sur un temps faible (SINC). Trois réglages façonnent l'ensemble : HER laisse une traîne sur la note suivante, SUPR empêche que tout devienne accent, ROT fait migrer lentement le temps fort. PROF est la part de tout cela qui atteint les sorties.

## Controles
- METR : le poids du temps fort de chaque cycle.
- SALTO : le poids de la taille de l'intervalle depuis la note précédente.
- APICE : le poids de la note la plus aiguë des huit dernières.
- EXT : le poids de l'accent qui arrive sur l'entrée ACC.
- AGOG : le poids de la note qui suit une note longue.
- SINC : le poids d'un accent extérieur sur un temps faible.
- CICLO : de combien de notes est le cycle métrique, de 2 à 16.
- ROT : la vitesse à laquelle le temps fort se déplace.
- HER : la traîne qu'un accent laisse sur la note suivante.
- SUPR : après un accent fort, le suivant coûte plus.
- PROF : la profondeur avec laquelle l'emphase atteint les sorties.
Entrées : GATE marque chaque note, PTCH reçoit la hauteur, ACC l'accent extérieur, RST recommence le cycle. Sorties : ENF est l'emphase, VCA le niveau, FC la brillance pour un filtre, DEC la durée pour une enveloppe, SWP un balayage qui monte avec des accents successifs, GT un gate sur les seules notes accentuées, NIV l'emphase en quatre paliers (0, un tiers, deux tiers, 1).

## Experimente
1. Jouez une ligne de SEQUENCE à travers un QUANTIZER dans un OSC. Branchez la hauteur et le gate du QUANTIZER dans PTCH et GATE.
2. Branchez la sortie VCA dans le CV1 du VCA de la voix : la ligne se met à respirer comme une phrase.
3. Passez l'OSC dans un LADDER et branchez SWP dans son FC : les accents successifs font monter le balayage, comme dans une basse acide.
4. Mettez CICLO à 5 et ROT à 0,5 : le temps fort change de place et les notes restent les mêmes.
