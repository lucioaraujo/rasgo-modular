## O que é
CRUSH salit le son à la manière numérique. Il imite les défauts des premiers échantillonneurs et des jeux vidéo, et ceux d'une mauvaise connexion : moins d'échantillons par seconde, moins de paliers de volume, des nombres qui débordent et reviennent de l'autre côté, des passages qui se figent ou disparaissent. Le résultat va d'une légère rugosité à une voix de robot en morceaux.

## Como pensar nele
SHAPE déforme comme un circuit analogique, en arrondissant et en repliant l'onde. CRUSH casse le son en marches. Chaque réglage est un type de dégât différent, et vous pouvez n'en utiliser qu'un ou en cumuler plusieurs. Les accidents sont tirés au sort, mais à partir de la graine du patch, si bien que le même patch abîme le son toujours de la même façon. Avec une enveloppe sur l'entrée MXM, la note commence propre et se dégrade peu à peu.

## Controles
- RATE : combien de fois par seconde le son est lu, de 100 à 24000. Les valeurs basses donnent un son en escalier et font apparaître des notes fantômes, le grésillement métallique des machines 8 bits.
- BITS : combien de paliers de volume il reste, de 1 à 16. À 16, on ne remarque presque rien ; à 1, tout devient onde carrée.
- DRIVE : le gain avant le dégât, qui pousse le son au-delà des limites.
- WRAP : ce qui se passe quand le son dépasse la limite. À zéro, il est écrêté ; au maximum, il réapparaît du côté opposé, ce qui sonne beaucoup plus agressif, surtout avec DRIVE haut.
- GLTCH : la probabilité d'un accident : un passage qui se fige, un trou de silence, un rebond.
- JITR : rend la lecture irrégulière, et la hauteur vacille un peu, comme une bande numérique fatiguée.
- TONE : un filtre simple en sortie. À gauche il assombrit ; à droite il ne laisse que le grésillement aigu ; au milieu il ne change rien.
- MIX : le mélange entre le son propre et le son abîmé. À zéro, le son passe intact.
Entrées : IN, le son ; RTM, pour moduler RATE ; MXM, pour moduler MIX.

## Experimente
1. Branchez la sortie SAW d'un OSC sur l'entrée IN, et OUT sur une entrée du MIXER.
2. Descendez RATE vers 4000 et BITS à 8. Le son devient granuleux.
3. Montez GLTCH peu à peu et écoutez les accidents apparaître.
4. Baissez MIX à zéro et branchez la sortie ENV d'un ENVELOPE sur MXM, avec le CLOCK sur l'entrée GATE de l'enveloppe. Chaque note commence propre et se défait.
