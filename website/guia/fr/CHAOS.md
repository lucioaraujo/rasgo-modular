## O que é
CHAOS génère un mouvement imprévisible à partir d'un système physique simulé : quelque chose comme une bille qui roule entre deux vallées, tantôt s'installant dans l'une, tantôt sautant dans l'autre. On ne peut pas prévoir quand elle va sauter, mais avec la même graine tout se répète à l'identique.

## Como pensar nele
D'autres modules tirent des valeurs au sort, comme TURING et DECISION, ou se promènent lentement, comme DRIFT. CHAOS a un comportement à lui : il tourne un moment autour d'une région puis passe soudain à une autre. Sur un filtre, cela donne un timbre qui hésite puis bondit ; sur une mélodie, des phrases qui restent dans un registre puis en changent brusquement. À haute vitesse, il devient lui-même une texture sonore.

## Controles
- RATE : la vitesse du mouvement, de 0,02 à 400 cycles par seconde. Lent pour faire bouger des réglages ; rapide pour l'écouter directement.
- DRIVE : la force qui pousse la bille entre les vallées. Plus haut, des sauts plus grands et plus fréquents.
- DAMP : le frottement. Haut, la bille se pose dans une vallée et s'arrête presque ; bas, elle oscille largement et saute d'un côté à l'autre.
- FRZ : fige le mouvement sur sa valeur actuelle.
Entrées : RSD, une impulsion qui donne une nouvelle poussée ; RTM, pour moduler la vitesse. Sortie : OUT.

## Experimente
1. Montez une voix : la SAW d'un OSC sur l'entrée IN d'un FILTER, LO du FILTER dans le MIXER.
2. Branchez OUT de CHAOS sur l'entrée FC du FILTER. Mettez RATE vers 0,3, DRIVE haut et DAMP bas.
3. Écoutez le timbre rester un moment dans une région puis bondir vers une autre.
4. Branchez aussi OUT de CHAOS sur l'entrée CV d'un QUANTIZER, et son PTCH sur 1V/O de l'OSC. La mélodie change maintenant de registre en même temps que le timbre.
