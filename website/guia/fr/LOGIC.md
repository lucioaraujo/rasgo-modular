## O que é
LOGIC combine et transforme des rythmes. Il divise une impulsion pour créer des rythmes plus lents, compare deux rythmes pour en produire un troisième, et bascule un état à chaque impulsion. Ce sont des opérations simples, de celles qu'on utilise dans les circuits numériques, appliquées au temps musical.

## Como pensar nele
Un rythme intéressant est rarement une pulsation droite. C'est en général la relation entre deux : l'un qui joue à mi-vitesse, l'un qui ne joue que quand l'autre joue aussi, l'un qui joue quand l'un ou l'autre joue mais jamais les deux ensemble. LOGIC se place entre les sources d'impulsions, comme CLOCK, TURING et SEQUENCE, et les modules qu'elles déclenchent. Sans rien sur l'entrée CLK, il utilise sa propre horloge.

## Controles
- RATE : l'horloge interne, utilisée quand rien n'arrive sur CLK.
- DIV : divise l'impulsion. À 2, la sortie DIV pulse une fois toutes les deux impulsions ; à 3, une fois toutes les trois.
- MULT : ajoute des impulsions intermédiaires dans chaque période.
- GATE : combien de temps l'impulsion de la sortie DIV reste active.
- DELAY : retarde l'impulsion de la sortie DIV de jusqu'à 200 millisecondes.
Entrées : CLK, l'impulsion à diviser ; A et B, les deux rythmes à comparer ; RST, pour remettre à zéro. Sorties : DIV, l'impulsion divisée ; AND, quand A et B sont actifs ensemble ; OR, quand l'un ou l'autre l'est ; XOR, quand un seul des deux l'est ; FLIP, qui change d'état à chaque impulsion sur A.

## Experimente
1. Branchez la sortie CLK du CLOCK sur l'entrée CLK de LOGIC et DIV sur l'entrée GATE d'un DRUM relié au MIXER. Changez DIV pour entendre l'impulsion ralentir.
2. Branchez EUC du CLOCK sur A, et la sortie PLS d'un TURING sur B.
3. Passez la percussion sur la sortie XOR, puis sur AND. Écoutez comment chaque combinaison fait un rythme différent à partir des deux mêmes.
