## O que é
RUNGLER a deux oscillateurs et un registre de 8 bits. À chaque tour de l’oscillateur A, le registre avance d’une case et garde l’état de l’oscillateur B à cet instant. Les trois derniers bits deviennent une tension de 8 marches, qui sort sur RUNG et revient moduler les deux oscillateurs.

## Como pensar nele
C’est le cœur du Benjolin, de Rob Hordijk. La boucle se ferme : avec peu de modulation, un motif qui se répète ; avec beaucoup, le circuit trébuche entre se verrouiller sur un rythme et devenir bruit. LOOP est l’interrupteur de Hordijk : au maximum, le bit qui sort de la fin revient au début et le motif de 8 pas se répète pour toujours ; entre les deux, un nouveau bit entre de temps en temps et le motif se transforme.

## Controles
- FRQA : la fréquence de l’oscillateur A, l’horloge.
- FRQB : la fréquence de l’oscillateur B.
- LOOP : l’interrupteur de boucle.
- MODA : combien le rungler module A.
- MODB : combien le rungler module B.
- LEVEL : le niveau.
Entrées : PTA et PTB ajoutent des hauteurs en volts par octave, LOOP s’ajoute à l’interrupteur de boucle. Sorties : TRIA et TRIB sont les triangles, PWM le comparateur, RUNG les 8 marches.

## Experimente
1. Mettez FRQA à 8 Hz et branchez RUNG sur un QUANTIZER et un OSC : une mélodie qui pense toute seule.
2. Branchez PWM sur un FILTER et un VCA avec les deux fréquences en audio : le son rauque du Benjolin.
3. Montez LOOP jusqu’au bout : le motif se fige. Baissez un peu : il commence à changer.
4. Montez MODA et MODB : le motif trébuche jusqu’au chaos.
