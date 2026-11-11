## O que é
BLEFE est un jeu de confiance fait de signaux. Chaque déclenchement devient un signal, qui sonne toujours pareil, vérité ou bluff. Une fenêtre plus tard, la vérité se confirme et le bluff non. À chaque résolution, la confiance bouge : elle monte avec les vérités, baisse avec les bluffs. Quand la confiance construite s'effondre, c'est la trahison, et la réponse refuse jusqu'au pardon.

## Como pensar nele
Un nombre aléatoire n'a pas d'intention ; un bluff en a une. Le module ne tire pas des notes, il crée une relation entre des voix : une voix qui répond tant qu'elle a confiance, hésite quand elle doute et se tait une fois trahie. VERD est le taux de vérité, MEM la vitesse à laquelle la confiance change, LIM le point de trahison. L'entrée RST est le pardon. SEED fixe la partie : la même graine donne les mêmes vérités et les mêmes mensonges.

## Controles
- VERD : le taux de vérité, de 0 à 1.
- JAN : la fenêtre entre le signal et la confirmation, de 0,05 à 4 s.
- MEM : la vitesse à laquelle la confiance bouge.
- LIM : sous ce niveau, la confiance construite devient trahison.
- SEED : la graine de la partie.
Entrées : IN reçoit les déclenchements, RST pardonne et recommence. Sorties : SIG est le signal, toujours le même ; VERD livre aussitôt la vérité cachée, pour qui veut tricher ; CONF pulse quand la vérité se confirme ; CNF est le niveau de confiance ; RESP est la réponse, qui se tait après la trahison ; TRAI pulse à l'instant de la trahison.

## Experimente
1. Branchez le CLOCK dans IN, SIG dans le gate d'un ENVELOPE et CONF dans un DRUM : le signal et la confirmation sonnent comme deux sons.
2. Branchez RESP dans le CV1 du VCA d'une autre voix : elle répond tant qu'elle a confiance.
3. Commencez avec VERD à 0,9 puis tournez-le vers 0,2. La voix qui répond grandit, hésite et se tait.
4. Branchez l'EOS d'un SEQUENCE dans RST : à chaque phrase, la relation recommence.
