## O que é
COHERENCE mesure à quel point une mélodie est prévisible et rompt quand elle devient trop ordonnée. Il garde les dernières notes et calcule leur entropie de Shannon : une note répétée donne un ordre de 1, les douze notes à parts égales donnent 0. Quand l'ordre reste assez longtemps au-dessus du seuil, il lance une impulsion de rupture.

## Como pensar nele
On croit volontiers que la rupture naît du chaos. Ici c'est l'inverse : l'ordre extrême rompt aussi. La pression monte à chaque note prévisible ; après PAC notes d'affilée, elle rompt. La rupture laisse une cicatrice : la fenêtre oublie sa moitié la plus ancienne et, pendant CIC notes, la pression ne remonte pas. Le module ne sonne pas : il décide quand, et le câble décide quoi.

## Controles
- JAN : la fenêtre, combien de notes il retient, de 4 à 32.
- LIM : le seuil d'ordre au-dessus duquel la pression monte.
- PAC : la patience, combien de notes d'affilée au-dessus du seuil avant de rompre.
- CIC : la cicatrice, combien de notes après une rupture sans nouvelle pression.
Entrées : PTCH reçoit la hauteur, GATE marque chaque note (sans lui, chaque changement de hauteur compte), RST oublie tout. Sorties : ORD est l'ordre mesuré, PRES la pression accumulée, COL l'impulsion de rupture, SURP la surprise de chaque note.

## Experimente
1. Branchez la hauteur et le gate d'un QUANTIZER qui joue un SEQUENCE court dans PTCH et GATE.
2. Branchez COL dans le RST d'un TURING qui nourrit une autre voix : quand la première ligne se répète trop, l'autre se refait.
3. Branchez PRES dans la coupure d'un FILTER : le timbre s'ouvre tant que la tension monte et se ferme à la rupture.
4. Branchez SURP dans le niveau d'un VCA : la note rare sonne plus fort.
