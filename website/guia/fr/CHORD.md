## O que é
CHORD transforme une note en accord. Vous lui donnez une hauteur et il joue de deux à quatre voix accordées au-dessus, dans des formes qui vont de l'unisson aux accords avec neuvième. Dans un modulaire, un accord demande normalement plusieurs oscillateurs et quantificateurs ; ici, un seul module suffit.

## Como pensar nele
Voyez-le comme une main sur un clavier qui connaît déjà la forme de l'accord. Le bouton CHORD choisit la forme, VOX le nombre de notes et INV leur disposition. Une onde lente sur l'entrée CHRD fait changer la forme toute seule ; la sortie ROOT de HARMONY sur l'entrée PITCH fait suivre à la fondamentale les changements de tonalité. Avec VLEAD, chaque voix va vers la note la plus proche de l'accord suivant, en glissant, et l'enchaînement sonne doux comme un chœur.

## Controles
- FREQ : la fondamentale, de 16 à 4000 Hz.
- CHORD : la forme de l'accord, parmi dix tables, de l'unisson aux accords avec neuvième.
- VOX : combien de voix sonnent, de deux à quatre.
- INV : monte d'une octave les notes les plus graves. Change l'allure de l'accord sans changer les notes.
- VLEAD : au changement d'accord, fait aller chaque voix vers la note la plus proche, en glissant. À zéro, toutes sautent en même temps.
- DTUNE : désaccorde les voix entre elles. Un peu épaissit le son ; beaucoup devient un mur.
- WAVE : la forme d'onde des voix, de la dent de scie à l'impulsion et au triangle.
- DRIFT : laisse chaque voix osciller légèrement dans l'accord, lentement.
Entrées : PITCH pour la fondamentale, CHRD pour changer la forme, et FM.

## Experimente
1. Branchez OUT sur une entrée du MIXER et tournez CHORD lentement. Écoutez défiler les formes.
2. Passez VOX de 2 à 4 et l'accord se remplit.
3. Montez un peu DTUNE : les voix s'écartent et le son s'élargit.
4. Branchez la sortie CLK du CLOCK sur l'entrée CLK d'une SEQUENCE, et la sortie PTCH de la SEQUENCE sur l'entrée PITCH du CHORD. L'accord avance maintenant avec la séquence.
