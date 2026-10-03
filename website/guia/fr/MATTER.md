## O que é
MATTER simule un objet qui vibre quand on le frappe : une corde, une cloche, une plaque de métal, un tube. À l'intérieur, il a 24 résonances qui sonnent ensemble, comme celles d'un objet réel. Vous le frappez et vous entendez l'objet répondre, avec l'accord et la matière que vous avez choisis.

## Como pensar nele
Un oscillateur filtré sonne électronique ; MATTER sonne comme quelque chose qu'on joue. Vous ne construisez pas le timbre harmonique par harmonique : vous choisissez la matière avec STRC et l'endroit où l'objet est frappé avec POS, puis vous l'excitez. Le coup peut venir de l'intérieur, par l'entrée HIT, ou de l'extérieur, de n'importe quel son branché sur IN. Un CLOCK qui frappe HIT, c'est déjà une percussion accordée.

## Controles
- FREQ : l'accord de l'objet, de 20 à 8000 Hz.
- STRC : la matière. Au début, une corde, au son juste et doux ; à la fin, cloche ou métal, aux résonances décalées.
- BRITE : combien de résonances aiguës sonnent. Bas, c'est étouffé ; haut, brillant.
- DAMP : combien de temps l'objet continue de sonner après le coup. Bas, une touche brève ; haut, une longue résonance.
- POS : l'endroit où l'objet est frappé. En le déplaçant, certaines résonances disparaissent et d'autres apparaissent, comme sur un vrai instrument.
- EXCIT : combien de bruit entre dans le coup. Bas, une frappe nette ; haut, plus d'air dans l'attaque.
- MIX : le mélange entre le coup brut et la résonance de l'objet. Au maximum, seulement le corps de l'objet.
Entrées : IN, pour faire résonner un son extérieur ; HIT, une impulsion qui frappe ; 1V/O, pour l'accord ; STR, pour changer la matière.

## Experimente
1. Branchez OUT sur une entrée du MIXER, et la sortie CLK du CLOCK sur l'entrée HIT. Vous entendez des coups accordés.
2. Tournez STRC d'un bout à l'autre. Le coup passe de la corde à la cloche.
3. Montez DAMP pour que les coups s'enchaînent en une résonance continue.
4. Déplacez POS lentement et écoutez le timbre changer selon le point de frappe.
