## O que é
SHAPE enrichit un son en le déformant de façon contrôlée. Il prend une onde simple, comme un sinus, et la replie, la sature ou la multiplie par une autre, créant de nouveaux harmoniques. La technique est associée aux synthétiseurs de la côte ouest des États-Unis, qui préféraient ajouter des harmoniques plutôt que les retirer avec des filtres.

## Como pensar nele
Un filtre retire des harmoniques à un son riche ; SHAPE fait le chemin inverse, à partir d'un son pauvre. Son geste principal est FOLD : quand le signal dépasse une limite, il est replié vers l'intérieur, et chaque pli ajoute de la brillance. Avec une enveloppe sur l'entrée FCV, la note ouvre le timbre à l'attaque et le referme ensuite. RING multiplie le son par ce qui entre sur MOD, ce qui donne des timbres de cloche.

## Controles
- RING : mélange le son avec son produit par le signal de l'entrée MOD. Au maximum, vous n'entendez que le produit, métallique.
- FOLD : la quantité de repli. Plus il y en a, plus il y a d'harmoniques aigus.
- SYM : décale le centre du repli et ajoute des harmoniques pairs, ce qui rend le son plus nasillard.
- WRAP : remplace une partie du repli par une coupure franche qui réapparaît de l'autre côté, plus brutale.
- SAT : arrondit les crêtes après le repli, ce qui adoucit le résultat.
- LVL : le volume de sortie.
- DRIFT : une oscillation lente de la quantité de repli.
Entrées : IN, le son ; MOD, le second signal pour RING ; FCV, pour moduler le repli.

## Experimente
1. Branchez la sortie SIN d'un OSC sur l'entrée IN, et OUT sur une entrée du MIXER. Vous entendez un sinus propre.
2. Montez FOLD lentement et écoutez le sinus gagner en brillance, pli après pli.
3. Poussez SYM d'un côté : le son devient plus nasillard.
4. Ramenez FOLD à zéro, branchez la sortie SAW d'un autre OSC sur MOD et montez RING. Accordez les deux oscillateurs sur des notes éloignées et le son devient cloche.
