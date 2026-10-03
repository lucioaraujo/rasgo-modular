## O que é
CLOCK est l'horloge du patch. Il émet des impulsions régulières au tempo que vous choisissez et en tire des rythmes et des accents. Presque tous les patchs commencent par lui : ses impulsions font avancer la séquence, déclenchent les enveloppes et jouent les percussions.

## Como pensar nele
Au lieu de programmer coup par coup, vous décrivez le rythme avec deux nombres. LEN dit combien de pas compte le cycle, FILL combien d'entre eux jouent, et CLOCK répartit ces coups de la façon la plus régulière possible. Cette méthode, appelée rythme euclidien, reproduit beaucoup de motifs traditionnels du monde entier : 3 sur 8 donne le tresillo cubain, 5 sur 8 le cinquillo. La sortie CLK pulse à chaque pas ; EUC seulement sur les pas choisis ; ACC marque les accents.

## Controles
- BPM : le tempo, de 20 à 300 battements par minute.
- MULT : combien de pas tiennent dans un battement.
- LEN : la longueur du cycle, de 1 à 32 pas.
- FILL : combien de pas du cycle jouent sur la sortie EUC. Peu, le rythme est clairsemé ; près de LEN, presque continu.
- ROT : fait tourner le motif, qui commence à un autre endroit. La densité est la même, mais la sensation change.
- SWING : retarde un pas sur deux, ce qui donne du balancement.
- DRIFT : laisse le tempo varier légèrement, comme un batteur qui pousse et retient.
- GATE : combien de temps chaque impulsion reste active, à l'intérieur du pas.
- ACC-A, ACC-B : les accents tombent sur les pas multiples de ces nombres. Avec 4 et 3, par exemple, les accents forment un motif de trois contre quatre.
- AND : décide si l'accent exige les deux nombres à la fois ou si l'un des deux suffit.
- FEEL : en combien de parties se divise chaque battement : deux, trois (triolets), cinq, sept, neuf, onze, ou une division tirée au sort à chaque pas.
Entrées : EXT, pour suivre des impulsions extérieures ; RST, pour revenir au début ; BPM, pour moduler le tempo. Sorties : CLK, EUC et ACC.

## Experimente
1. Branchez EUC sur l'entrée GATE d'un DRUM, et OUT du DRUM sur une entrée du MIXER.
2. Mettez LEN à 8 et faites aller FILL de 1 à 8. À 3 et à 5, vous entendrez des motifs familiers.
3. Tournez ROT et écoutez le même motif commencer ailleurs.
4. Branchez ACC sur l'entrée ACC du DRUM et changez ACC-A et ACC-B pour entendre les accents se déplacer.
