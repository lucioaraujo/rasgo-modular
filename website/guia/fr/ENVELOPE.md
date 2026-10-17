## O que é
ENVELOPE donne forme à une note dans le temps : comment elle commence, comment elle retombe, combien elle se maintient et comment elle s'éteint. Il reçoit une impulsion sur l'entrée GATE et dessine une courbe à chaque impulsion. Il a déjà un contrôle de volume intégré, si bien qu'il suffit d'y faire passer un son pour entendre des notes à la place d'un son continu.

## Como pensar nele
Les sources sonnent tout le temps ; ENVELOPE transforme ce flux en phrases. Il a deux usages qui peuvent coexister. Par l'entrée IN et la sortie OUT, il articule le son qui passe. Par la sortie ENV, il fournit la courbe elle-même, que vous pouvez envoyer ouvrir un filtre, changer un timbre ou doser un effet. Les quatre étapes de la courbe ont des noms anglais qu'on trouve sur tous les synthétiseurs : attack, decay, sustain et release, d'où ADSR.

## Controles
Dans l'interface en développement 0.1.7-dev, ATK, DEC, SUS et REL sont des curseurs verticaux : tirez vers le haut pour augmenter. Les trois durées suivent une échelle logarithmique de 1 ms à 10 s ; SUS reste un niveau linéaire. CURVE, VCA et LVL se trouvent en dessous. Les paramètres et les patches enregistrés conservent les valeurs de l'interface précédente.

- ATK : l'attaque, le temps que met la note pour atteindre son maximum après l'impulsion. Court, c'est percussif ; long, comme un archet qui entre doucement.
- DEC : la décroissance, le temps de la chute après le maximum jusqu'au niveau de SUS.
- SUS : le niveau auquel la note se maintient tant que l'impulsion reste haute. À zéro, pas de maintien et la note sonne pincée.
- REL : le temps que met la note à s'éteindre une fois l'impulsion terminée.
- CURVE : la forme des courbes, de plus douce à plus incisive, avec une attaque perçue plus tôt.
- TRIG : dans une position, la note se maintient tant que dure l'impulsion ; dans l'autre, chaque impulsion déclenche la courbe entière, sans attendre.
- VCA : à quel point le volume intégré agit sur le son qui passe. À zéro, le son traverse sans changer et seule la sortie ENV compte.
- LVL : le volume de sortie.
Entrées : IN, le son ; GATE, l'impulsion qui déclenche la note ; TIME, pour accélérer ou ralentir toutes les étapes. Sorties : OUT, le son articulé ; ENV, la courbe.

## Experimente
1. Branchez la sortie SAW d'un OSC sur l'entrée IN, OUT sur une entrée du MIXER, et la sortie EUC du CLOCK sur GATE. Vous entendez des notes.
2. Montez ATK et écoutez chaque note entrer doucement. Revenez et jouez avec DEC pour rendre les notes courtes ou longues.
3. Placez un FILTER entre l'OSC et l'ENVELOPE, avec CUT bas, et branchez ENV sur l'entrée FC du filtre. Chaque note ouvre maintenant aussi le timbre.
