## O que é
OPERATOR est un synthétiseur FM à quatre oscillateurs sinusoïdaux, appelés opérateurs. En FM, un opérateur ne sonne pas seul : il pousse la fréquence d'un autre en avant et en arrière très vite, et de cela naissent de nouveaux harmoniques. C'est ainsi qu'on fait pianos électriques, cloches, basses claquantes et timbres métalliques, comme sur les synthétiseurs numériques des années 1980.

## Como pensar nele
Trois décisions font le timbre : qui module qui (ALGO), le rapport de fréquence entre les opérateurs (RB, RC, RD) et la force de la modulation (INDEX). Commencez avec INDEX bas et montez peu à peu : le son va d'un sinus propre à quelque chose de brillant et âpre. Le geste le plus utile est de brancher une enveloppe sur l'entrée IDX, pour que chaque note commence brillante et s'assombrisse, comme une touche de piano électrique.

## Controles
- FREQ : la hauteur de la note, de 8 à 8000 Hz.
- FINE : l'accord fin, jusqu'à un demi-ton de chaque côté.
- ALGO : choisit l'un des huit agencements de qui module qui. Au début, les opérateurs sont en chaîne et le son est plus âpre ; à la fin, ils sonnent côte à côte, plus près d'un orgue.
- RB, RC, RD : le rapport de fréquence de chaque opérateur avec le premier. Les nombres entiers sonnent juste ; les valeurs fractionnaires, comme 2,5, sonnent comme une cloche ; les valeurs hautes, comme du métal.
- INDEX : la force de la modulation. À zéro, vous n'entendez que des sinus purs ; au maximum, un timbre très brillant.
- FBK : fait se moduler le premier opérateur lui-même. À lui seul, il transforme déjà le sinus en quelque chose de proche d'une dent de scie.
- DRIFT : désaccorde chaque opérateur un tout petit peu, lentement. À zéro, tout reste stable.
Entrées : 1V/O pour la hauteur et IDX pour moduler la force de la FM.

## Experimente
1. Branchez OUT sur une entrée du MIXER avec INDEX à zéro : un simple sinus.
2. Montez INDEX lentement et écoutez les harmoniques apparaître.
3. Mettez RB sur une valeur fractionnaire et le son devient cloche.
4. Branchez la sortie ENV d'un ENVELOPE sur l'entrée IDX, et la sortie CLK du CLOCK sur l'entrée GATE de l'ENVELOPE. Chaque note s'ouvre brillante et s'éteint, comme un piano électrique.
