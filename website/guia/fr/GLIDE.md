## O que é
GLIDE fait glisser une note jusqu'à la suivante au lieu de sauter. Il se place sur le chemin du signal de hauteur, entre ce qui choisit les notes, comme SEQUENCE ou QUANTIZER, et l'oscillateur. Ce glissement, appelé portamento, donne leur chant aux lignes de basse acides et aux solos liés.

## Como pensar nele
Un glissement toujours actif lasse vite. L'intéressant est de choisir quand glisser, et GLIDE en décide note par note : il peut glisser toujours, seulement quand l'entrée SLIDE est haute, ou seulement quand les notes sont liées, c'est-à-dire quand la nouvelle note arrive avant que la précédente soit relâchée. Les sorties MOV et DONE indiquent quand un glissement a lieu et quand il se termine, et peuvent déclencher autre chose.

## Controles
- TIME : le temps pour monter d'une octave, de zéro, un saut net, à deux secondes.
- FALL : rend la descente plus rapide ou plus lente que la montée. Au milieu, elles sont égales.
- CURVE : la forme du glissement. À zéro, la vitesse est constante et la note arrive pile à temps ; au maximum, elle freine en approchant et met plus de temps à se poser.
- MODE : quand glisser : toujours ; seulement avec SLIDE haut, comme sur le synthétiseur de basse TB-303 ; ou seulement entre notes liées, en lisant l'entrée GATE.
Entrées : PITCH, la hauteur qui arrive ; SLIDE et GATE, qui décident quand glisser. Sorties : OUT, la hauteur conduite ; MOV, haute pendant le glissement ; DONE, une impulsion quand il se termine.

## Experimente
1. Montez une ligne : la sortie CLK du CLOCK sur l'entrée CLK d'une SEQUENCE, et la sortie SAW d'un OSC dans le MIXER.
2. Branchez PTCH de la SEQUENCE sur PITCH de GLIDE, et OUT de GLIDE sur 1V/O de l'OSC.
3. Montez TIME : les notes se mettent à glisser les unes dans les autres.
4. Mettez MODE sur la troisième position et branchez GATE de la SEQUENCE sur GATE de GLIDE. Seules les notes liées glissent maintenant.
