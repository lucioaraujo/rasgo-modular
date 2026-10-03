## O que é
DECISION est la source de choix du patch. À chaque impulsion, il décide s'il envoie ou non un signal sur la sortie GATE, et tire deux valeurs sur les sorties X et Y. Vous réglez la probabilité de déclencher, la taille et la forme des tirages, et à quel point il se souvient de ce qu'il a fait.

## Como pensar nele
Une onde lente est trop prévisible, et le bruit pur n'a pas de forme. DECISION se tient entre les deux : du hasard avec des règles. BIAS décide si quelque chose arrive presque toujours ou seulement de temps en temps. SHAPE décide si les valeurs se répartissent également ou se concentrent près du centre, les grands sauts devenant rares. DEJA est la mémoire : au lieu de tirer à nouveau, il relit les derniers pas, et la musique gagne des répétitions, comme un thème qui revient.

## Controles
- RATE : l'horloge interne, utilisée quand rien n'arrive sur TRIG.
- BIAS : la probabilité que GATE se déclenche à chaque impulsion. À zéro, jamais ; au maximum, toujours.
- SPRD : la portée des tirages sur X et Y. Bas, des valeurs proches du centre ; haut, toute l'étendue.
- SHAPE : à zéro, toutes les valeurs ont la même chance ; au maximum, les valeurs proches du centre reviennent bien plus souvent.
- STEPS : divise les tirages en marches. À 1, ils sont continus.
- SLEW : fait glisser chaque nouvelle valeur depuis la précédente.
- DEJA : la probabilité de répéter une valeur de la mémoire au lieu d'en tirer une autre. Haut, la même phrase tend à se répéter.
- LOOP : la taille de cette mémoire, de 1 à 16 pas.
Entrées : TRIG, l'impulsion de chaque décision ; BIAS et SPRD, pour moduler ces réglages. Sorties : X et Y, les valeurs tirées ; GATE, la décision.

## Experimente
1. Branchez la sortie EUC du CLOCK sur TRIG.
2. Branchez X sur l'entrée CV d'un QUANTIZER, PTCH sur 1V/O d'un OSC, et l'OSC par un ENVELOPE jusqu'au MIXER. Branchez GATE de DECISION sur GATE de l'enveloppe.
3. Baissez BIAS et écoutez les notes se raréfier, seulement de temps en temps.
4. Montez DEJA peu à peu. La mélodie commence à se répéter, et avec DEJA au maximum elle reste prise dans une boucle de LOOP pas.
