## O que é
PHRASE propose un motif et le joue quand des impulsions arrivent sur CLK. L'idée revient à chaque phrase, avec des changements de direction, des silences et des accents que vous réglez. Ses sorties commandent d'autres voix ; reliez un oscillateur et une enveloppe pour l'entendre.

## Como pensar nele
Utilisez CLOCK pour une pulsation régulière ou les attaques du LISTEN pour répondre à un autre son. SEED identifie le motif. VAR transforme ses passages ; à zéro, le dessin qui revient est plus facile à reconnaître. L'entrée PITCH fournit un appui, capturé au début de chaque phrase. Pour choisir une gamme, passez la sortie PITCH par QUANTIZER avec RANGE=1. PHRASE ne contient pas à lui seul un répertoire de styles ou de traditions musicales.

## Controles
- LEN : longueur de la phrase en événements, de 2 à 16 impulsions. Ce n'est pas une durée fixe en secondes.
- SPAN : étendue du dessin, de 1 à 36 demi-tons, avant quantification.
- DIR : incline le dessin vers le bas ou le haut. Au centre, conserve la direction du motif de base.
- VAR : ampleur des transformations à chaque passage. Commencez à zéro, puis augmentez en écoutant ce que vous reconnaissez encore.
- SPACE : probabilité de silences dans les pas intermédiaires. Le début et la conclusion sont préservés.
- CAD : force du retour à l'appui en fin de phrase.
- GATE : durée de l'articulation, de 5 à 400 ms. Reliez GATE à l'enveloppe de la voix pour entendre les notes articulées.
- SEED : identité numérique du motif, de 1 à 65535. La changer recrée le dessin et recommence ; VARIA laisse ce réglage inchangé.
Entrées : PITCH, appui à 1 V par octave ; CLK, un pas par front montant ; RST, reprise ; STIM, intensité de 0 à 1 ; RET, demande de conclusion au prochain pas. Sorties : PITCH, hauteur tenue ; GATE, articulation ; ACC, accent de 0 à 1 ; END, impulsion de 5 ms à la conclusion ; PHASE, position de 0 à 1 dans la phrase. Sans câble sur STIM, l'intensité vaut 1. Rouvrir le patch recommence la phrase, sans reprendre la position exacte de l'exécution précédente.

## Experimente
1. Désactivez VARIA. Reliez CLOCK.EUC à PHRASE.CLK, PHRASE.PITCH à QUANTIZER.CV (RANGE=1), puis QUANTIZER.PITCH à OSC.PITCH. Choisissez une gamme sur QUANTIZER.
2. Reliez OSC.SAW à ENVELOPE.IN, PHRASE.GATE à ENVELOPE.GATE et ENVELOPE.OUT au MIXER. Commencez avec VAR=0 et SPACE=0. Écoutez l'idée revenir ; modifiez DIR puis SPAN séparément.
3. Placez un VCA entre la voix et le MIXER : ENVELOPE.OUT vers VCA.IN1, VCA.O1 vers le MIXER, LVL1=0 et CV1=1. Reliez PHRASE.ACC à VCA.CV1 pour entendre les accents. Augmentez VAR, puis SPACE.
4. Remplacez CLOCK par LISTEN.L-HIT sur CLK et reliez LISTEN.LOW à STIM. Le son écouté appelle maintenant les notes et modifie leur intensité. Une impulsion sur RET demande une conclusion au prochain CLK ; END peut appeler une seconde voix.
