## O que é
SAMPLER enregistre un passage de ce qui y entre, jusqu'à environ huit secondes, le découpe en tranches et les joue quand il reçoit des impulsions. C'est le geste des échantillonneurs du hip-hop et de la musique électronique : prendre un enregistrement, le découper et le rejouer dans un autre ordre, plus vite, plus lentement ou à l'envers.

## Como pensar nele
Tant que l'entrée REC est active, il enregistre ce qui arrive sur IN. Quand REC s'éteint, le passage est gardé. À partir de là, chaque impulsion sur TRIG joue une tranche, et l'entrée POS choisit laquelle. Avec un TRIGSEQ qui déclenche et une SEQUENCE qui choisit les tranches, l'enregistrement se recombine en un nouveau rythme. En branchant la sortie du patch lui-même sur l'entrée, l'instrument se met à réutiliser ce qu'il vient de jouer.

## Controles
- START : où, dans la tranche, commence la lecture.
- SPEED : la vitesse de lecture, d'un quart à quatre fois l'originale. Négatif joue à l'envers. Normalement, plus vite sonne aussi plus aigu.
- SLICE : en combien de tranches égales l'enregistrement est découpé, de 1 à 16.
- REPIT : éteint, vitesse et hauteur vont ensemble, comme sur une bande ; allumé, la hauteur vient de l'entrée PIT et la tranche garde sa durée.
- WEAR : l'usure à chaque déclenchement : un départ imprécis, une perte de définition, un son granuleux.
- LOOP : éteint, chaque tranche joue une fois ; allumé, elle se répète.
Entrées : TRIG, l'impulsion qui joue ; IN, le son à enregistrer ; REC, le signal qui enregistre tant qu'il est actif ; POS, pour choisir la tranche ; PIT, pour accorder. Sortie : OUT.

## Experimente
1. Branchez une voix qui joue une phrase, par exemple une SEQUENCE qui joue un OSC, sur l'entrée IN.
2. Pour enregistrer, branchez la sortie DIV d'un LOGIC sur REC, avec la sortie CLK du CLOCK sur CLK du LOGIC et DIV à 16. Branchez OUT du SAMPLER dans le MIXER.
3. Mettez SLICE à 8, branchez T1 d'un TRIGSEQ sur TRIG et la sortie CV d'un TURING sur POS. La phrase enregistrée revient découpée et réordonnée.
4. Poussez SPEED à gauche du centre et écoutez les tranches à l'envers.
