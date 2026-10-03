## O que é
MATRIX relie quatre entrées à quatre sorties par tous les chemins possibles à la fois. Le panneau porte une grille de 16 cellules, et chacune décide quelle part d'une entrée va vers une sortie. C'est une table de distribution : au lieu d'un câble par liaison, toutes les liaisons sont là, chacune avec son niveau.

## Como pensar nele
Il est utile quand on pense par blocs : un peu de cette modulation partout, ou changer la source de tout d'un coup. Branchez quatre sources de mouvement sur les entrées et quatre destinations sur les sorties, et la grille devient un panneau de modulation. Avec de l'audio, il devient un mélangeur à quatre sorties. Avec DRIFT, les 16 cellules changent lentement d'elles-mêmes, et le réseau de liaisons évolue avec la musique.

## Controles
La grille : chaque cellule porte deux chiffres, l'entrée puis la sortie. La cellule 23, par exemple, décide quelle part de l'entrée 2 va vers la sortie 3. Faites glisser verticalement pour régler ; les valeurs négatives inversent le signal.
- LEVEL : le niveau général des quatre sorties.
- NORM : équilibre le niveau quand plusieurs cellules d'une sortie sont ouvertes, pour que la somme ne grossisse pas trop.
- RING : au lieu d'additionner les entrées sur une sortie, les multiplie, ce qui donne des timbres métalliques avec des signaux audio.
- SAT : une saturation qui tient le niveau quand vous réinjectez la matrice sur elle-même.
- DRIFT : fait se promener lentement les 16 cellules.
Entrées : IN1 à IN4. Sorties : OUT1 à OUT4.

## Experimente
1. Branchez la sortie BI d'un FUNCTION sur IN1 et la sortie A d'un DRIFT sur IN2.
2. Montez une voix avec FILTER et SPACE. Branchez OUT1 sur l'entrée FC du FILTER et OUT2 sur l'entrée FBK du SPACE.
3. Ouvrez et fermez les cellules 11, 12, 21 et 22 et écoutez chaque source passer d'une destination à l'autre.
4. Montez DRIFT et laissez la matrice se réorganiser toute seule.
