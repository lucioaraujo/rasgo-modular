## O que é
TURNTABLE joue un enregistrement comme s'il était sur un disque vinyle. Il enregistre un passage, comme SAMPLER, mais la lecture imite une vraie platine : le disque a du poids, met du temps à prendre de la vitesse, freine peu à peu et peut être poussé en avant et en arrière, comme dans le scratch d'un DJ.

## Como pensar nele
SAMPLER saute d'un point à l'autre instantanément. TURNTABLE a de l'inertie, et c'est ce qui donne le son du vinyle : la hauteur glisse quand le disque accélère ou freine. L'entrée SCR est la main sur le disque : une onde lente à cet endroit fait aller et venir le disque en rythme. L'entrée BRK coupe le moteur, et le son descend jusqu'à s'arrêter, l'effet qu'on appelle tape stop. Il n'y a pas de synchronisation automatique du tempo : trouver le rythme fait partie du geste.

## Controles
- SPEED : la vitesse que le moteur cherche à atteindre, de la moitié au double de l'originale. Négatif fait tourner le disque à l'envers.
- TORQ : la force du moteur, c'est-à-dire la rapidité avec laquelle le disque atteint sa vitesse. Bas, le démarrage fait longuement vaciller la hauteur ; au minimum, le disque ne bouge qu'à la main, par l'entrée SCR.
- FRIC : le frottement. Il décide à quelle vitesse le disque s'arrête au freinage et à quel point il revient seul dans le rythme après un scratch.
- GRAB : la fermeté de la main, à quel point le signal sur SCR fait bouger le disque.
- START : où tombe l'aiguille à chaque impulsion sur TRIG.
- WEAR : l'usure du disque : craquements et petites irrégularités de rotation.
- LOOP : éteint, le disque arrive au bout et s'arrête ; allumé, la lecture fait le tour et recommence.
Entrées : TRIG, pour reposer l'aiguille ; IN, le son à enregistrer ; REC, le signal qui enregistre tant qu'il est actif ; SCR, la main sur le disque ; BRK, le frein. Sortie : OUT.

## Experimente
1. Enregistrez une phrase comme avec SAMPLER : la voix sur IN et la sortie DIV d'un LOGIC, avec DIV à 16, sur REC. Branchez OUT dans le MIXER et allumez LOOP.
2. Branchez la sortie BI d'un FUNCTION vers 2 Hz sur SCR et montez GRAB jusqu'au milieu. Le disque va et vient, en scratch.
3. Retirez le câble de SCR. Branchez la sortie GATE d'un DECISION avec BIAS bas sur BRK. De temps en temps, le disque freine et repart.
