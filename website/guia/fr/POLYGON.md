## O que é
POLYGON est la polymétrie dessinée comme une figure. Il y a un cycle commun et jusqu'à quatre couches ; chaque couche est un polygone de N sommets inscrit dans ce cycle, et chaque sommet est une impulsion. Un triangle et un carré dans le même cycle donnent trois contre quatre.

## Como pensar nele
Pensez à une horloge avec plusieurs aiguilles invisibles, chacune partageant le cycle en parts égales différentes. Les impulsions de chaque couche sont régulières ; l'intérêt est là où elles se rencontrent. La sortie COINC marque ces rencontres, et c'est là que la musique demande souvent un accent. F fait tourner chaque polygone, jusqu'à un côté, pour décaler la couche sans changer sa division.

## Controles
- N1, N2, N3, N4 : combien de sommets a chaque couche, de 0 (coupée) à 16.
- F1, F2, F3, F4 : la rotation de chaque couche, jusqu'à un côté du polygone.
- CICLO : la durée du cycle, de 0,25 à 32 s, quand rien n'est branché sur CLK.
- PASSOS : avec une horloge sur CLK, combien d'impulsions dure le cycle.
Entrées : CLK mène le cycle par impulsions, RST recommence. Sorties : G1 à G4 sont les impulsions de chaque couche, COINC pulse quand deux sommets ou plus tombent ensemble, EOS à la fin de chaque cycle, FASE est la position dans le cycle.

## Experimente
1. Avec N1 à 3 et N2 à 4, branchez G1 dans un DRUM et G2 dans un autre : trois contre quatre.
2. Branchez COINC dans un troisième son : seulement les rencontres.
3. Mettez N3 à 5 et tournez F3 lentement : la couche de cinq glisse sur les autres.
4. Branchez le CLOCK dans CLK avec PASSOS à 16 : la polymétrie suit le tempo du patch.
