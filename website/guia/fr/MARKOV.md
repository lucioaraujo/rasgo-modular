## O que é
MARKOV est une chaîne de Markov : il apprend d'où l'on va vers où dans une mélodie (après cette note est venue celle-là trois fois, cette autre une fois) et improvise en tirant la note suivante selon ce qu'il a appris. Sans rien de branché, il joue déjà, en commençant par une promenade pentatonique.

## Como pensar nele
C'est un élève qui imite le style de ce qu'il entend, pas les notes exactes. TEMP est son audace : près de 0, il répète le chemin le plus courant ; près de 2, il invente. MEM est ce dont il se souvient. La sortie SURP dit à quel point chaque note était inattendue, et cela peut aussi faire bouger le patch.

## Controles
- RATE : l'horloge interne, quand rien n'est branché sur CLK.
- GATE : la durée de chaque note.
- TEMP : l'audace du tirage, de 0 à 2.
- MEM : combien de ce qu'il a appris il garde.
- LEARN : active ou coupe l'apprentissage. Coupé, il fige son vocabulaire.
- OCT : transpose la sortie par octaves.
Entrées : CLK avance d'une note, LRN.P et LRN.G reçoivent la mélodie à apprendre (hauteur et gate), RST recommence. Sorties : PTCH est la note, GT le gate, SURP le degré de surprise.

## Experimente
1. Branchez PTCH dans un OSC et GT dans un ENVELOPE qui commande un VCA. Écoutez la promenade initiale.
2. Branchez la hauteur et le gate d'un PHRASE dans LRN.P et LRN.G : le MARKOV se met à répondre dans le style de cette voix.
3. Après lui avoir appris, coupez LEARN et montez TEMP : il improvise avec le vocabulaire qu'il a gardé.
4. Branchez SURP dans le MIX d'un SPACE : les notes inattendues prennent de l'écho.
