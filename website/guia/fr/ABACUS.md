## O que é
ABACUS traite les signaux comme des nombres. Il fait des calculs entre deux entrées, compte des impulsions et transforme le compte en rythmes et en escaliers, et coupe ou retourne la partie négative d'un signal. Même sans rien sur les entrées A et B, son compteur interne génère des motifs qui servent de rythme et de mélodie.

## Como pensar nele
Compter est la façon la plus simple de créer un motif. Un compteur qui va de 0 à 7 et recommence, regardé bit par bit, produit des rythmes syncopés sans aucun séquenceur : chaque bit s'allume et s'éteint à une vitesse différente. La sortie QNT transforme le compte en escalier de valeurs, bon à envoyer vers un QUANTIZER. Et la sortie RCT, le redresseur, est utile à elle seule chaque fois qu'il faut la moitié positive d'un signal ou sa valeur absolue.

## Controles
- OP : le calcul entre A et B qui sort sur MTH : addition, soustraction, multiplication, reste de la division, ou quatre opérations bit à bit.
- MOD : jusqu'où compte le compteur avant de recommencer, de 2 à 32.
- STEP : en combien de marches est divisée la sortie QNT.
- RNG : la taille de la fenêtre de valeurs utilisée par les sorties MTH, QNT et RCT.
- RECT : le mode du redresseur : seulement la partie positive, seulement la négative, les deux retournées vers le haut, ou seulement le signe, plus ou moins.
- CNT : de combien avance le compteur à chaque impulsion. Négatif compte à rebours.
- PAT : quel bit du compteur devient la sortie P1. Les bits bas changent vite ; les bits hauts, lentement.
- SLEW : lisse les sorties MTH et QNT.
- RATE : l'horloge interne, utilisée quand rien n'arrive sur CLK.
Entrées : A et B, les nombres du calcul ; CLK, l'impulsion qui fait compter ; RST, pour remettre à zéro. Sorties : MTH, le calcul ; QNT, l'escalier ; RCT, le redresseur ; P1 et P2, deux rythmes tirés du compteur ; CRY, une impulsion chaque fois que le compte recommence.

## Experimente
1. Branchez la sortie CLK du CLOCK sur l'entrée CLK d'ABACUS, avec MOD à 8.
2. Branchez P1, P2 et CRY sur les entrées GATE de trois DRUM reliés au MIXER. Vous entendez un rythme syncopé qui se referme toutes les huit impulsions.
3. Tournez PAT et écoutez P1 accélérer ou ralentir.
4. Branchez QNT sur l'entrée CV d'un QUANTIZER, et PTCH sur 1V/O d'un OSC relié au MIXER. Le compte devient une mélodie en escalier.
