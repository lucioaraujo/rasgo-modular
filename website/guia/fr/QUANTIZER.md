## O que é
QUANTIZER accroche un signal continu aux notes d'une gamme. Une valeur qui monte et descend librement, venue d'un TURING, d'un SH ou d'un CHAOS, en ressort comme une suite de notes justes. Dans la plupart des patchs, c'est lui qui transforme le hasard en mélodie.

## Como pensar nele
Les signaux de contrôle varient de façon continue, les notes de musique non. Sans quantificateur, une mélodie née du hasard tombe entre les notes et sonne faux. QUANTIZER arrondit chaque valeur à la note la plus proche de la gamme choisie. Avec une impulsion sur l'entrée TRIG, il ne change de note qu'à l'impulsion, et la mélodie prend un rythme. La sortie GATE pulse chaque fois que la note change vraiment, ce qui sert à déclencher une enveloppe seulement sur les nouvelles notes.

## Controles
- SCALE : la gamme, parmi douze : chromatique, majeure, mineure naturelle, dorienne, phrygienne dominante, lydienne, mineure mélodique, pentatonique mineure, pentatonique majeure, hirajoshi, tons entiers et octaves seules.
- ROOT : la tonique, la note de départ de la gamme, de do à si.
- RANGE : combien d'octaves couvre le signal d'entrée, de 1 à 6. Plus d'octaves, des sauts plus grands.
- GLIDE : fait glisser la hauteur d'une note à la suivante.
- HYST : une marge de tolérance. Avec un signal instable, la note ne saute pas sans cesse entre deux voisines.
Entrées : CV, le signal à quantifier ; TRSP, pour transposer avant de quantifier ; TRIG, l'impulsion qui décide quand changer de note. Sorties : PTCH, la hauteur ; GATE, une impulsion à chaque nouvelle note ; ST, la note en demi-tons.

## Experimente
1. Branchez la sortie CV d'un TURING sur l'entrée CV du QUANTIZER, et la sortie CLK du CLOCK sur l'entrée CLK du TURING.
2. Branchez PTCH sur 1V/O d'un OSC, et la sortie SIN de l'OSC sur un ENVELOPE relié au MIXER. Branchez GATE du QUANTIZER sur GATE de l'enveloppe.
3. Changez SCALE et écoutez la même suite de valeurs devenir une autre mélodie. Changez ROOT pour transposer.
