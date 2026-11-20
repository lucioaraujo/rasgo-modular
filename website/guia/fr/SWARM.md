## O que é
SWARM est un banc de six poissons, et chaque poisson est une horloge avec son propre tempo. Ils s’écoutent et se tirent les uns les autres. Quatre d’entre eux sortent sur G1 à G4, et la pulsation du banc entier sur ALL.

## Como pensar nele
C’est le modèle de Kuramoto, celui des grenouilles qui se synchronisent dans le chœur de phases de l’Aquorbium et des lucioles qui clignotent ensemble. Un seul réglage, COHES, va du dispersé au soudé : à 0, chaque poisson joue dans son temps, comme une hétérophonie ; à 1, tous sur la même pulsation, comme une homorythmie ; entre les deux, le banc se forme et se défait. SYNC mesure à quel point ils sont ensemble.

## Controles
- RATE : le tempo moyen.
- SPRD : combien les tempos naturels diffèrent.
- COHES : la cohésion du banc.
- GATE : la durée de chaque gate.
Entrées : SCAR est la frayeur qui disperse le banc, LEAD un meneur externe que tous suivent, COH s’ajoute à COHES. Sorties : G1 à G4 sont les pulsations de quatre poissons, ALL celle du banc, SYNC la cohésion mesurée.

## Experimente
1. Branchez G1 à G4 sur quatre DRUM avec COHES à 0 : quatre tempos dispersés. Montez COHES lentement et écoutez le banc se former.
2. Branchez un déclencheur rare sur SCAR : le banc se disperse et se regroupe.
3. Branchez un CLOCK sur LEAD : le banc se met à suivre le meneur.
4. Branchez SYNC sur le niveau d’un VCA : le son grandit quand le banc se resserre.
