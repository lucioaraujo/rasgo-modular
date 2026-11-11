## O que é
VIA est le chemin du signal traité comme une matière. Il y a sept portes par où le son peut passer : résistance, diode, condensateur, impulsion, transistor, reflux et haut-parleur. Celles qui sont ouvertes se combinent par leur moyenne. Chacune laisse sur le son la marque d'un composant.

## Como pensar nele
Dans un circuit, le fil n'est jamais neutre : la diode coupe, le condensateur retient et relâche, le haut-parleur a un corps et sature. VIA met ces comportements entre deux modules quelconques, sur un câble audio ou de contrôle. Les six premières portes viennent de l'Antitotem ; FALANTE est un corps résonant accordé par CORPO. Sans aucune porte ouverte, le chemin reste silencieux.

## Controles
- RES : la résistance : le signal, atténué.
- DIODO : redresse ; seul le haut de l'onde passe, avec sa direction.
- CAP : le condensateur, qui retient et relâche lentement.
- PULSO : seulement le front ; le signal devient un seuil, presque une marche.
- TRANS : la courbe asymétrique du transistor, avec le condensateur.
- REFLX : le reflux, la différence entre le signal et la mémoire du condensateur : un courant qui revient altéré.
- FALANTE : le corps d'un haut-parleur, qui résonne et sature.
- MEM : le temps du condensateur, de 1 ms à 2 s.
- CORPO : la résonance du haut-parleur, de 40 à 400 Hz.
- NIVEL : le niveau de sortie.
Entrées : IN reçoit le signal, MEM déplace le temps du condensateur. Sorties : OUT est le signal transformé, CAP l'état du condensateur en CV.

## Experimente
1. Passez une voix dans la VIA avec seulement RES ouvert : le son passe tel quel.
2. Ouvrez FALANTE et tournez CORPO : la voix prend le corps d'un petit ou d'un grand baffle.
3. Fermez tout et n'ouvrez que DIODO et CAP : un timbre rongé, avec mémoire.
4. Branchez un LFO dans IN et utilisez la sortie CAP pour déplacer un filtre : la VIA traite aussi les signaux de contrôle.
