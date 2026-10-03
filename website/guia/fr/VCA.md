## O que é
VCA règle le volume d'un son au moyen d'un autre signal. Il a deux canaux identiques : le son entre sur IN, un signal de contrôle entre sur CV, et le volume de sortie monte et descend avec ce contrôle. C'est le module le plus utilisé de tout système modulaire.

## Como pensar nele
Voyez-le comme une main sur un bouton de volume, actionnée par un autre module. Avec une enveloppe sur le contrôle, chaque note reçoit un début, un milieu et une fin. Avec une onde lente, le volume ondule, un effet appelé trémolo. Il sert aussi à doser un signal de contrôle avant de l'envoyer ailleurs, et la sortie SUM additionne les deux canaux, comme un petit mélangeur. Attention : le niveau démarre à zéro, donc le canal reste muet tant que rien n'arrive sur CV ou que vous ne montez pas LVL.

## Controles
- LVL1, LVL2 : le volume de chaque canal. Le signal de contrôle s'ajoute à cette valeur.
- CV1, CV2 : à quel point le signal de contrôle agit, et dans quel sens. Négatif inverse : le volume baisse quand le contrôle monte.
- RSP1, RSP2 : la courbe de réponse. À zéro, linéaire ; au maximum, exponentielle, plus naturelle pour le volume.
- DRIFT : une petite oscillation lente des deux volumes.
Entrées : IN1 et IN2, les sons ; CV1 et CV2, les contrôles. Sorties : O1, O2 et SUM, la somme des deux.

## Experimente
1. Branchez la sortie SAW d'un OSC sur IN1, et O1 sur une entrée du MIXER. Silence : LVL1 est à zéro.
2. Montez LVL1 lentement et le son apparaît.
3. Ramenez LVL1 à zéro et branchez la sortie ENV d'un ENVELOPE sur CV1, avec le CLOCK sur l'entrée GATE de l'enveloppe. Le son pulse maintenant avec les notes.
4. Remplacez l'enveloppe par la sortie UNI d'un FUNCTION lent : le volume ondule, en trémolo.
