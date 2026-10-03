## O que é
OSC est un oscillateur : il produit une note continue et stable, la matière première de presque tous les sons de synthétiseur. Il fournit cinq formes d'onde à la fois, chacune sur sa propre sortie : sinus, triangle, dent de scie, impulsion, et une voix une ou deux octaves plus bas (le sub). Vous choisissez celle que vous voulez, ou vous en utilisez plusieurs.

## Como pensar nele
Voyez OSC comme une matière brute. Seul, il sonne nu, et c'est voulu : le caractère vient de ce que vous branchez ensuite, comme un filtre qui retire de la brillance ou une enveloppe qui donne forme à la note. Si vous cherchez une voix qui a déjà sa personnalité, STRING et MATTER en ont une ; si vous voulez un timbre qui se transforme tout seul, essayez WAVETABLE. OSC est la base prévisible, et c'est pour cela qu'il est le meilleur endroit pour apprendre.

## Controles
- FREQ : la hauteur de la note, de 8 à 8000 Hz. Les valeurs basses deviennent une vibration lente ; la zone musicale se situe à peu près entre 50 et 1000 Hz.
- FINE : l'accord fin, jusqu'à un demi-ton vers le haut ou vers le bas. Pour s'accorder sur une autre voix, ou se désaccorder légèrement et épaissir le son.
- PW : la largeur d'impulsion, qui n'agit que sur la sortie PLS. Au milieu, l'onde est carrée et sonne creux ; vers les extrémités, elle devient fine et nasillarde.
- FM : à quel point le signal branché sur l'entrée FM fait bouger la hauteur. Avec un autre oscillateur à cet endroit et ce bouton haut, le timbre devient métallique, comme une cloche.
- DRIFT : une petite oscillation lente de l'accord, comme celle d'un oscillateur analogique. À zéro, la note reste parfaitement immobile.
- SUB2 : place la sortie SUB une octave (éteint) ou deux octaves (allumé) sous la note.
- SYNC : active l'entrée SYNC. Avec elle, un autre oscillateur plus lent force celui-ci à recommencer son cycle, et tourner FREQ change alors le timbre au lieu de la hauteur.
- PROX : mélange chaque sortie avec une version plus sombre d'elle-même. Une façon d'adoucir le son sans y consacrer un filtre.
Entrées : 1V/O reçoit la hauteur venant d'un séquenceur ou d'un quantificateur, FM et PWM acceptent une modulation, et SYNC reçoit l'oscillateur qui commande la synchronisation.

## Experimente
1. Branchez la sortie SAW d'OSC sur une entrée du MIXER et écoutez un bourdonnement continu.
2. Tournez FREQ lentement : la note monte et descend.
3. Déplacez le câble sur la sortie PLS et faites aller PW d'un côté à l'autre. Le timbre passe de creux à fin sans que la hauteur change.
4. Revenez à SAW et faites passer le son par un FILTER avant le MIXER. Vous faites maintenant de la synthèse soustractive : partir d'un son riche et retirer ce qui est en trop.
