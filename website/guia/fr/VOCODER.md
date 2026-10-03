## O que é
VOCODER fait parler un son avec la voix d'un autre. Il écoute un son, en général une voix, et mesure l'énergie présente dans chaque bande de fréquences ; puis il applique ce dessin à un second son, riche en harmoniques. Le résultat, c'est le second son qui porte la parole du premier : l'effet de voix robotique de tant de musique électronique.

## Como pensar nele
Il a besoin de deux entrées : CAR, la porteuse, le son qui va parler (une dent de scie, un accord), et MOD, celui qui parle (une voix via SIGNAL-IN, un tambour, un enregistrement). Avec peu de bandes, le résultat est grossier et robotique ; avec beaucoup, la parole devient claire. Rien sur CAR ? Il utilise une dent de scie interne, accordable par l'entrée PIT.

## Controles
- BANDS : combien de bandes de fréquences, de 4 à 20. Peu sonnent robotiques ; beaucoup rendent la parole intelligible.
- SHIFT : change la taille de la voix obtenue, de grande à petite, sans changer ce qui est dit.
- ATK : la rapidité de réaction de chaque bande. Court, les consonnes sortent sèches ; long, tout s'adoucit.
- REL : le temps que met chaque bande à relâcher. Court, c'est net ; long, les syllabes se fondent en nappe.
- SIBIL : quelle part des aigus de la voix passe directement, pour que les S et les T restent clairs.
- FRZ : fige le dessin actuel. La porteuse continue de dire la dernière syllabe indéfiniment.
- MIX : le mélange entre la porteuse originale et la porteuse qui parle.
Entrées : CAR, la porteuse ; MOD, la voix qui parle ; PIT, la hauteur de la dent de scie interne quand il n'y a rien sur CAR.

## Experimente
1. Branchez la sortie SAW d'un OSC sur CAR, et la sortie L de SIGNAL-IN, avec un micro, sur MOD. Branchez OUT sur une entrée du MIXER et parlez.
2. Descendez BANDS à 6 et écoutez la voix devenir robotique ; montez à 20 et elle devient claire.
3. Montez REL : les mots se fondent en un son continu.
4. Sans micro, branchez la sortie OUT d'un DRUM sur MOD. Chaque coup ouvre la porteuse dans son propre timbre.
