## O que é
CONTROL ajuste des signaux de contrôle avant qu'ils n'arrivent à destination. Il peut réduire, agrandir ou inverser un signal, le décaler vers le haut ou le bas, lisser ses marches et additionner deux signaux. Ce sont des opérations modestes, mais on les retrouve dans presque tous les patchs qui vont au-delà de l'essentiel.

## Como pensar nele
Parfois une modulation a la bonne forme mais la mauvaise taille, ou le mauvais sens : une enveloppe qui devrait fermer le filtre au lieu de l'ouvrir, une onde qui ne devrait osciller qu'au-dessus de zéro. Le gain d'un câble peut réduire un signal, mais pas l'inverser ni le décaler. CONTROL fait tout cela sur un panneau, avec deux canaux identiques et une sortie qui les réunit. Avec un vrai son à l'entrée, il peut aussi suivre le volume de ce son et le transformer en contrôle, ce qu'on appelle un suiveur d'enveloppe.

## Controles
- SCALE : la taille du signal, sur chaque canal. À 1, il passe tel quel ; à 0,5, de moitié ; négatif, inversé. À zéro, il ne reste que le décalage de OFF, et le canal devient un contrôle manuel.
- OFF : une valeur ajoutée au signal, qui le décale vers le haut ou le bas.
- RECT : replie vers le haut la partie négative du signal. Au milieu, la partie négative disparaît ; au maximum, elle est mise en miroir.
- SLEW : lisse les changements brusques, sur jusqu'à deux secondes. Les marches deviennent des rampes.
- CRV : la forme de ce lissage, d'une rampe droite à une courbe qui ralentit.
- SUM : si la sortie SUM additionne les deux canaux ou en fait la moyenne.
- DRIFT : une lente promenade des décalages, qui ôte sa raideur à une valeur fixe.
Entrées : IN1 et IN2. Sorties : O1, O2 et SUM.

## Experimente
1. Montez une voix : la SAW d'un OSC sur l'entrée IN d'un FILTER, et LO du FILTER dans le MIXER. Baissez CUT.
2. Branchez la sortie ENV d'un ENVELOPE joué par le CLOCK, sur IN1 de CONTROL, et O1 sur l'entrée FC du FILTER. Chaque note ouvre le filtre.
3. Mettez SCALE du premier canal à −1 et montez CUT. Chaque note ferme maintenant le filtre.
4. Branchez la sortie BI d'un FUNCTION sur IN2 et écoutez la sortie SUM à la place de O1 : les deux modulations agissent ensemble.
