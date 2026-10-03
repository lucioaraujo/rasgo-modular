## O que é
SCOPE montre la forme d'onde d'un signal, comme un oscilloscope, et mesure ce qu'il entend : le volume, la brillance, la hauteur et le début de chaque note. Ces mesures sortent par des câbles, comme des signaux de contrôle. Le patch peut ainsi réagir à son propre son.

## Como pensar nele
Sur un oscilloscope ordinaire, l'écran est le bout du chemin. Ici, ce que mesure SCOPE peut revenir dans le patch : la brillance du mélange ouvre un filtre, la hauteur d'une voix en accorde une autre, le début de chaque note déclenche une enveloppe. Le son le traverse sans changement, par la sortie THRU, si bien qu'il peut se placer au milieu de n'importe quelle chaîne. Avec SIGNAL-IN, il peut aussi écouter un son extérieur et faire suivre au Rasgo Modular ce qu'il entend.

## Controles
- TRIG : le niveau que le signal doit franchir pour stabiliser l'image à l'écran.
- EDGE : si ce franchissement compte à la montée ou à la descente.
- REJ : une marge de tolérance, pour que le bruit ne déclenche pas l'écran plusieurs fois.
- RESP : la rapidité des mesures de volume, de brillance et de hauteur. Rapide, elles suivent chaque détail ; lent, elles montrent la tendance.
- HOLD : fige les mesures sur leur valeur actuelle.
- SENS : la sensibilité du détecteur de début de note. Haut, la moindre montée compte ; bas, seulement les attaques fortes.
Entrées : IN, le son ; EXT, un signal extérieur pour stabiliser l'écran. Sorties : THRU, le son intact ; TRIG, l'impulsion de déclenchement ; LVL, le volume ; BRT, la brillance ; PIT, la hauteur ; ONS, une impulsion au début de chaque note.

## Experimente
1. Placez SCOPE entre une voix et le MIXER : la voix sur IN, THRU vers le MIXER. Regardez la forme d'onde.
2. Branchez BRT sur l'entrée FC du FILTER d'une autre voix. Quand la première devient brillante, la seconde s'ouvre aussi.
3. Branchez ONS sur l'entrée GATE d'un DRUM. Chaque note de la première voix s'accompagne maintenant d'un coup.
