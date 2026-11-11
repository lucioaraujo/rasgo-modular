## O que é
CELL est un automate cellulaire : une rangée de cases allumées ou éteintes qui se réécrit à chaque impulsion. Chaque case regarde elle-même et ses deux voisines, et une règle (un nombre de 0 à 255) décide si elle reste allumée. Le module lit ce dessin avec une tête : la case sous elle donne le gate, et les trois suivantes choisissent la note.

## Como pensar nele
C'est une partition qui s'écrit seule selon une loi très simple. Certaines règles donnent des dessins qui ne se répètent jamais (30, 110), d'autres des triangles (90) ou des formes qui se déplacent (184). Vous ne composez pas les notes, vous composez la loi. MUT ajoute un petit hasard, pour que la ligne ne meure pas ni ne se bloque.

## Controles
- RATE : l'horloge interne, quand rien n'est branché sur CLK.
- RULE : la règle, de 0 à 255. Commencez par 30, 90, 110 et 184.
- CELLS : combien de cases a la rangée, de 8 à 24.
- HEAD : où se trouve la tête de lecture.
- RANGE : l'étendue des notes, jusqu'à 2 octaves.
- MUT : la probabilité qu'une case bascule au hasard.
- GATE : la durée de chaque note.
Entrées : CLK avance d'une génération, RST recommence la rangée, RULE change la règle par CV. Sorties : PTCH est la note, GT le gate, DENS la proportion de cases allumées, EOS une impulsion quand la rangée renaît.

## Experimente
1. Branchez GT dans un DRUM et le DRUM dans le MIXER. Avec RULE à 30, écoutez un rythme qui ne se répète jamais pareil.
2. Branchez PTCH dans un OSC et GT dans un ENVELOPE qui commande un VCA après l'OSC. Tournez RULE lentement.
3. Branchez DENS dans le CUT d'un FILTER : plus la rangée est vivante, plus le son est brillant.
4. Branchez EOS dans l'ADV du HARMONY : l'accord change quand la rangée renaît.
