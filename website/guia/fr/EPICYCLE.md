## O que é
EPICYCLE dessine avec des cercles : jusqu'à huit, chacun tournant sur le bord du précédent. La pointe du dernier trace une figure, et ses coordonnées sortent en signaux. C'est une série de Fourier dessinée, la même idée que les épicycles de Ptolémée.

## Como pensar nele
La même machine sert à deux échelles. Avec peu de cercles et un RATE lent, c'est du geste et du rythme : une figure qui revient. Avec beaucoup de cercles et la hauteur d'une note sur PTCH, elle tourne à la vitesse du son et devient timbre. MODO choisit la famille de figures, et DESV désaccorde les vitesses pour que la figure ne se referme jamais.

## Controles
- RATE : la vitesse du premier cercle, de 0,01 Hz à 2 kHz.
- CIRC : combien de cercles, de 1 à 8.
- MODO : carrée (vitesses impaires), dent de scie (toutes les vitesses), rayons de Fibonacci ou vitesses de Fibonacci.
- DESV : l'écart des vitesses ; au-dessus de 0, la figure ne se referme jamais.
- FASE : la phase de départ des cercles.
Entrées : PTCH reçoit la hauteur à un volt par octave, pour le mode timbre ; RST recommence la figure. Sorties : X et Y sont les coordonnées de la pointe, R la distance au centre, ANG l'angle, GT une impulsion quand Y passe par zéro en montant.

## Experimente
1. Avec RATE à 0,2 Hz et CIRC à 3, branchez X dans la coupure d'un FILTER et Y dans le pan d'une voie du MIXER : le son dessine une figure dans l'espace.
2. Branchez GT dans un ENVELOPE : la figure devient rythme.
3. Branchez X dans le MIXER, mettez CIRC à 8 et branchez la hauteur d'un QUANTIZER dans PTCH : la figure est maintenant un timbre.
4. Changez MODO et montez DESV lentement : le timbre s'ouvre et ne revient jamais au même endroit.
