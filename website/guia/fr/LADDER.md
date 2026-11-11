## O que é
LADDER est le filtre en échelle, le filtre des basses : quatre étages de 6 dB en cascade, la sortie du dernier renvoyée à l'entrée comme résonance. C'est le dessin breveté par R. Moog en 1969, ici avec un bouton qui va de l'échelle à transistors, ronde, à l'échelle à diodes, acide.

## Como pensar nele
Le FILTER est propre et stable ; le LADDER est le filtre qui se comprime et s'épaissit quand on le pousse. Trois choses le distinguent. DIODO couple les étages, et à mi-chemin il y a des échelles qui n'existent pas en circuit. Le pic reste accordé sur CUT quelle que soit la position de DIODO, et à RESO 1 l'échelle chante un sinus accordé. Et COMP rend les basses qui, dans l'échelle d'origine, disparaissent quand la résonance monte.

## Controles
- CUT : la fréquence de coupure, de 20 Hz à 20 kHz.
- RESO : la résonance. À 1, l'échelle auto-oscille ; 0,9 signifie la même proximité du seuil pour tout DIODO.
- DRIVE : pousse l'entrée contre la saturation dans la boucle ; le niveau de sortie est compensé.
- COMP : rend les basses. À 1, elles restent entières à toute résonance ; à 0, elles s'effondrent comme dans l'original.
- DIODO : de 0 (transistors, rond) à 1 (diodes, acide), en continu.
Entrées : IN reçoit le son, FC déplace la coupure à un volt par octave, Q déplace la résonance. Sorties : LP4 est la sortie à 24 dB par octave, LP2 la prise après le deuxième étage, à 12 dB par octave, plus brillante.

## Experimente
1. Branchez la sortie SAW d'un OSC dans IN, LP4 dans un VCA et le VCA dans le MIXER. Réglez CUT vers 200 Hz.
2. Montez RESO avec COMP à 1 : le pic chante et les basses restent. Baissez COMP et écoutez les basses s'effondrer.
3. Mettez DIODO à 1, RESO à 0,85 et DRIVE à 0,6, avec une enveloppe courte sur FC : la basse acide.
4. Débranchez IN, mettez RESO à 1 et branchez la hauteur d'un QUANTIZER dans FC : l'échelle joue la mélodie comme un oscillateur.
