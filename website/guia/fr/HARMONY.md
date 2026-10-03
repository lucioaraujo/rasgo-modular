## O que é
HARMONY décide par quelles tonalités passe la musique. De temps en temps, ou à chaque impulsion sur l'entrée ADV, il choisit un nouveau centre tonal, selon l'un des six mouvements que les musiciens de jazz et de cinéma utilisent depuis des décennies. La sortie ROOT donne la nouvelle tonique sur la même échelle de hauteur que celle que comprennent les oscillateurs.

## Como pensar nele
QUANTIZER garde la mélodie juste, mais toujours dans la même tonalité. La musique aime voyager, et chaque style voyage à sa façon. Le réglage MOVE choisit le parcours : Coltrane, qui saute d'une tierce majeure à chaque changement et boucle un cycle de trois tonalités, comme dans Giant Steps ; substitution tritonique, qui rejoint la tonalité suivante par un chemin inattendu ; médiante chromatique, des changements par tierce à la couleur de musique de film ; emprunt modal, qui garde la tonique et change le mode ; jazz modal, qui ne bouge presque jamais ; et backdoor, qui monte d'un ton entier.

Dans cette version, pour entendre les changements, on ajoute la sortie ROOT à la mélodie avant l'oscillateur. Chaque entrée n'accepte qu'un câble, donc la somme passe par un MATRIX, et toute la mélodie se transpose vers le nouveau centre. Le changement de gamme, de la sortie SCALE, n'atteint pas encore le QUANTIZER par câble.

## Controles
- MOVE : le type de parcours, parmi les six décrits plus haut.
- RATE : le rythme des changements, quand rien n'arrive sur ADV. D'un changement toutes les quelques minutes à deux par seconde.
- ROOT : la tonique de départ, vers laquelle le module revient quand il reçoit une impulsion sur RST.
- S-LO, S-HI : la plage des gammes qui peuvent être tirées au sort à chaque changement.
- HOLD : la probabilité qu'un changement soit ignoré, ce qui allonge certaines sections.
Entrées : ADV, l'impulsion qui demande un changement ; RST, pour revenir au début. Sorties : ROOT, la tonique en hauteur ; SCALE, le numéro de la gamme ; CHG, une impulsion à chaque changement.

## Experimente
1. Montez la mélodie du QUANTIZER : la sortie CV d'un TURING, mené par le CLOCK, sur l'entrée CV du QUANTIZER, et un OSC qui passe par un ENVELOPE jusqu'au MIXER.
2. Branchez PTCH du QUANTIZER sur IN1 d'un MATRIX, ROOT de HARMONY sur IN2, et OUT1 du MATRIX sur 1V/O de l'OSC. Sur le MATRIX, mettez la cellule 21 à 1 : OUT1 devient la mélodie plus la tonique.
3. Mettez MOVE sur la première position, Coltrane, et montez RATE. À chaque changement, la mélodie saute vers un autre centre, par cycles de trois.
4. Branchez CHG sur l'entrée ADV d'un DRIFT pour que le reste du patch change aussi de section à chaque changement.
