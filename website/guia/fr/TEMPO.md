## O que é
TEMPO fait respirer le tempo. Il donne à CLOCK un tempo qui bouge : il accélère vers le climax, s'élargit à la fin de chaque phrase, emprunte et rend du temps, tient une fermata et ralentit jusqu'à l'arrêt à la fin.

## Como pensar nele
Une horloge fixe sonne comme une machine ; une personne qui joue ne garde jamais le tempo immobile. TEMPO sort en RATE, qui va à l'entrée RATE de CLOCK. Seul, il respire déjà dans les phrases ; relié à FORM, il suit la même forme que la pièce.

## Controles
- ACCEL : de combien le tempo accélère avec la tension.
- FINAL : le tempo d'arrivée à la fin, en fraction.
- SHAPE : la courbe du ralentissement final.
- RIT : combien de temps dure le ralentissement final, en secondes.
- MODE : strict, avec arche de phrase, ou élastique, avec le rubato aussi.
- PHR : la longueur de la phrase en temps.
- ARCH : l'arche de la phrase — plus rapide au milieu.
- RUBATO : le temps emprunté et rendu dans la phrase.
- MOLA : la vitesse à laquelle le tempo suit les changements, toujours sans saut.
- FERMT : combien de temps dure la fermata.
- SEED : la graine des sauts et du rubato.
Entrées : TENS, la tension ; SECT, les changements de section ; CODA, le début de la fin ; FERM, une fermata ; CLK, l'horloge qui compte les temps. Sorties : RATE, pour CLOCK ; ARCH, l'arche de phrase comme signal ; BEAT, le temps déjà déformé ; END, la fin.

## Experimente
1. Branchez le RATE de TEMPO sur le RATE de CLOCK et le CLK de CLOCK sur le CLK de TEMPO. Écoutez le tempo respirer dans les phrases.
2. Branchez TENS, SECT et END de FORM sur TENS, SECT et CODA de TEMPO : la pièce accélère au climax et ralentit jusqu'à l'arrêt à la fin.
3. Branchez une impulsion rare (d'un DECISION, par exemple) sur FERM : de temps en temps, le temps s'arrête et repart.
