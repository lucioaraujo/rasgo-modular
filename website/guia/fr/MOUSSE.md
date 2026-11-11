## O que é
MOUSSE est une mousse. Appuyez : une partie s'enfonce aussitôt, une partie s'écoule lentement ; relâchez : elle revient sans hâte. La sortie DEPTH dit combien elle est enfoncée : une enveloppe faite non de droites, mais de matière.

## Como pensar nele
Une enveloppe ordinaire monte et descend selon des lignes. La mousse a de la mémoire : des coups répétés l'enfoncent davantage, et avec FATG elle s'amollit à l'usage. STIFF est sa dureté, SLOW sa fluidité, RET combien de fois le retour est plus lent que la descente. Sans rien sur HIT, elle est jouée seule au rythme de RATE.

## Controles
- STIFF : la dureté. Dure, elle s'enfonce peu et revient vite.
- VISC : la viscosité, le temps d'écoulement, de 0,05 à 8 s.
- SLOW : la part de l'enfoncement qui est lente plutôt qu'immédiate.
- RET : combien de fois le retour est plus lent, de 1 à 8.
- FORCE : la force de chaque coup.
- FATG : la fatigue ; des coups répétés amollissent la mousse.
- RATE : le rythme des coups internes, quand rien n'est branché sur HIT.
Entrées : HIT donne un coup, PRESS appuie avec une force continue. Sorties : DEPTH est l'enfoncement, SPEED sa vitesse, REST un gate quand la mousse se repose.

## Experimente
1. Branchez un DRUM dans HIT et DEPTH dans le CUT d'un FILTER, avec le câble atténué et inversé : chaque coup assombrit le son et la brillance revient lentement.
2. Montez FATG et jouez des coups répétés : la mousse cède peu à peu.
3. Branchez la sortie ENV du DYNAMICS dans PRESS : le mélange lui-même pousse la mousse.
4. Branchez REST dans le gate d'un ENVELOPE : un son seulement quand la mousse se repose.
