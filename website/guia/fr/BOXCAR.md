## O que é
BOXCAR mesure un signal dans de courtes fenêtres et fait la moyenne de nombreuses mesures successives. La technique vient d'instruments de laboratoire servant à retrouver un signal faible enfoui dans le bruit. Dans un patch, il sert de suiveur de volume très stable, de reconstructeur de formes d'onde et d'oscillateur fait de ce qu'il a mesuré.

## Como pensar nele
SH capture un instant ; BOXCAR capture une tranche de temps et en fait la moyenne. À chaque impulsion sur TRIG, il ouvre une fenêtre à un point du cycle, mesure, et ajoute cette mesure aux précédentes. Ce qui se répète à l'identique à chaque cycle se renforce, et ce qui relève du hasard s'annule. MODE choisit quoi faire du résultat : ne donner que la moyenne, comme un signal de contrôle qui suit le son lentement ; rejouer le cycle reconstruit ; ou relire ce cycle tout seul, comme un oscillateur. Il a en plus une sortie à part, GEIG, aux impulsions irrégulières comme celles d'un compteur Geiger.

## Controles
- DLY : à quel point du cycle s'ouvre la fenêtre.
- APER : la largeur de la fenêtre. Étroite, elle mesure presque un instant ; large, elle fait la moyenne d'un passage.
- AVG : combien de mesures entrent dans la moyenne, de 1 à 64. Plus de mesures, moins de bruit et une réponse plus lente.
- SCAN : fait parcourir le cycle à la fenêtre toute seule, dans un sens ou dans l'autre. À zéro, elle reste immobile.
- MODE : suiveur, reconstruction ou oscillateur.
- RATE : l'horloge interne, quand rien n'arrive sur TRIG, et la vitesse de relecture en mode oscillateur.
- THRSH : le niveau que le signal doit franchir pour déclencher une mesure, quand rien n'arrive sur TRIG.
- GEI : la densité des impulsions sur la sortie GEIG. À zéro, elle reste muette.
- BLEND : le mélange entre le signal original et le signal traité.
Entrées : IN, le signal à mesurer ; TRIG, l'impulsion de chaque mesure ; SWP, pour déplacer la fenêtre ; THR, pour moduler le seuil. Sorties : OUT et GEIG.

## Experimente
1. Branchez GEIG sur l'entrée GATE d'un DRUM relié au MIXER et montez GEI. Vous entendez des coups à des moments imprévisibles.
2. Branchez maintenant une voix qui joue des notes, comme un OSC passant par un ENVELOPE, sur l'entrée IN, avec MODE sur la première position et APER large.
3. Branchez OUT sur l'entrée CV1 d'un VCA qui règle un autre son. Le second son suit maintenant le volume de la voix, sans trembler.
