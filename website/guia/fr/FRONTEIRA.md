## O que é
FRONTEIRA écoute une mélodie et marque où commence une idée nouvelle. Nous entendons une frontière là où quelque chose change : un saut après des degrés conjoints, une note longue après des courtes, une pause. Le module mesure ces trois changements note par note et pulse quand leur force dépasse le seuil.

## Como pensar nele
C'est le modèle de Cambouropoulos, d'après les règles de groupement de Lerdahl et Jackendoff. Pour savoir si une note est une frontière, il doit entendre la suivante, donc l'impulsion arrive une note plus tard. Un long silence ferme la phrase aussitôt. Le PHRASE génère un motif ; la FRONTEIRA écoute.

## Controles
- LIM : le seuil de force au-dessus duquel un changement devient frontière.
- MEM : combien de notes il prend comme référence de ce qui est normal.
- MIN : le minimum de notes entre deux frontières.
- SIL : combien de silence ferme la phrase, de 0,3 à 4 s.
Entrées : PTCH reçoit la hauteur, GATE marque chaque note. Sorties : FRONT pulse au début de chaque cellule, FORCA est la force de la dernière frontière, NOTAS combien de notes avait la cellule, FIM pulse quand le silence ferme la phrase.

## Experimente
1. Branchez le SIGNAL-IN dans le SCOPE, la sortie PIT du SCOPE dans PTCH et la sortie ONS dans GATE. Activez le ON du SIGNAL-IN.
2. Branchez FRONT dans l'entrée FRASE du MIXER, avec la scène en mode geste : chaque idée nouvelle de celui qui joue déplace la scène.
3. Branchez FIM dans le RST d'un SEQUENCE : quand celui qui joue s'arrête, l'accompagnement recommence avec la phrase suivante.
4. Branchez NOTAS dans la durée d'un ENVELOPE : les phrases longues appellent des notes plus longues.
