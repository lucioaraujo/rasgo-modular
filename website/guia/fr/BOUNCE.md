## O que é
BOUNCE lâche des balles qui rebondissent, chaque fois plus bas et plus vite, jusqu'à s'arrêter. Il y a jusqu'à trois balles. Chaque rebond est un gate, la force du rebond est un accent, et la hauteur de chaque balle est une CV qui monte et descend.

## Como pensar nele
C'est un rythme qui accélère tout seul, comme une balle de ping-pong lâchée sur la table. ELAST décide combien la balle rend à chaque rebond, GRAV est sa hâte. Avec AUTO, les balles arrêtées sont relancées, et WIND les pousse doucement : le module joue sans fin, sans se répéter.

## Controles
- BALLS : combien de balles, de 1 à 3.
- GRAV : la gravité. Plus haute, des rebonds plus rapides.
- ELAST : combien chaque rebond rend. Près de 1, la balle met longtemps à s'arrêter.
- SPRD : à quel point les balles sont lâchées de hauteurs différentes.
- WIND : un vent léger qui empêche les répétitions exactes.
- AUTO : relance les balles qui se sont arrêtées.
Entrées : DROP relâche les balles, GRAV déplace la gravité par CV. Sorties : GT est le gate de chaque rebond, ACC la force du rebond, H1, H2 et H3 la hauteur de chaque balle.

## Experimente
1. Branchez GT dans un DRUM et le DRUM dans le MIXER : écoutez le rebond accélérer jusqu'à s'arrêter.
2. Branchez ACC dans le niveau du DRUM : les premiers rebonds sonnent plus fort.
3. Branchez H1 dans un QUANTIZER et le QUANTIZER dans un OSC : une mélodie qui tombe.
4. Branchez GT dans le CLK d'un ARP : l'arpège accélère avec la balle.
