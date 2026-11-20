## O que é
ECLIPSE est la Lune avec ses deux mois. L’un est celui des phases, de nouvelle lune à nouvelle lune. L’autre est le retour aux nœuds, les deux points où l’orbite de la Lune croise le plan de la Terre. Il n’y a éclipse que quand les deux coïncident, et chaque éclipse donne un gate sur SUN ou sur MOON.

## Como pensar nele
Les deux mois s’emboîtent presque, mais pas tout à fait. C’est pourquoi les éclipses viennent par saisons, à peu près tous les six mois, et la séquence entière ne revient qu’après 223 mois : le Saros, déjà connu des Babyloniens. C’est un polyrythme astronomique, rare et prévisible. Les nombres sont ceux du ciel de l’Asterismus. SYN et DRAC changent les mois et donnent d’autres ciels.

## Controles
- SPEED : combien de jours par seconde.
- LIMIT : la limite d’éclipse.
- GATE : la durée du gate.
- SYN : le mois des phases.
- DRAC : le mois des nœuds.
Entrées : CLK fait de chaque impulsion un jour, RST revient au jour 0. Sorties : SUN et MOON sont les éclipses, NEW chaque nouvelle lune, NODE chaque passage par un nœud, ALIGN l’arrivée de la saison, PHASE la phase de la Lune.

## Experimente
1. Branchez NEW sur un DRUM et SUN sur un accord : la pulsation du mois, et de temps en temps un événement rare.
2. Branchez ALIGN sur le niveau d’une voix : elle grandit à l’approche de la saison des éclipses.
3. Branchez PHASE sur la coupure d’un FILTER : un LFO d’un mois.
4. Rapprochez DRAC de SYN : les éclipses deviennent fréquentes ; éloignez-le : elles deviennent rarissimes.
