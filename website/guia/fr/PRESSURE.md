## O que é
PRESSURE est une boîte de gaz. Les particules avancent en ligne droite et rebondissent sur les parois. La paroi de droite est un piston, et chaque choc sur elle donne un gate sur HIT. La force moyenne des chocs est la pression, qui sort sur PRES.

## Como pensar nele
C’est la théorie cinétique des gaz, de Maxwell et Boltzmann. Plus de chaleur, les particules courent plus vite et frappent davantage. Moins de volume, elles frappent plus souvent, car la paroi est plus proche : la pression monte, comme dans la loi de Boyle. Avec ATTR positif, le gaz devient un gaz réel : les particules s’attirent et, fortes, se condensent en goutte, et la pression baisse.

## Controles
- N : combien de particules il y a dans la boîte.
- TEMP : la température, la vitesse moyenne.
- VOL : le volume ; la position du piston.
- ATTR : attraction (positif) ou répulsion (négatif) entre les particules.
- GATE : la durée de chaque gate.
Entrées : HEAT donne un éclat de chaleur à toutes, VOL et TEMP bougent volume et température par CV. Sorties : HIT est le gate des chocs sur le piston, PRES la pression, TEMP la température mesurée, X et Y la position d’une particule.

## Experimente
1. Branchez HIT sur un DRUM avec N à 3 : un rythme clairsemé et irrégulier. Montez N jusqu’à 20 : cela devient un grésillement de chocs.
2. Branchez un LFO lent sur VOL : quand le piston comprime, le rythme accélère et PRES monte.
3. Branchez X et Y sur le PAN et la coupure d’un FILTER : la particule dessine le timbre.
4. Montez ATTR lentement : le gaz se condense, les chocs se raréfient et la pression baisse.
