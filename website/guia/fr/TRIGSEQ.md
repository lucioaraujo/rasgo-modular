## O que é
TRIGSEQ génère des rythmes de batterie sur quatre lignes à la fois, pensées pour grosse caisse, caisse claire, charleston et une percussion supplémentaire. Vous ne placez pas les coups un par un : vous choisissez un style, réglez la densité de chaque ligne, et il crée le motif. Dès son chargement, il joue.

## Como pensar nele
Quatre CLOCK, un par instrument, produiraient des lignes qui ne se parlent pas. Dans TRIGSEQ, les quatre naissent du même style, et elles s'emboîtent donc comme une batterie jouée par quelqu'un. Le réglage MAP parcourt quatre caractères : droit, comme dans le rock et la house ; cassé, comme dans le breakbeat ; balancé, comme dans le hip-hop ; et clairsemé, comme dans le dub. Tourné lentement, le groove change de personnalité sans perdre la pulsation.

## Controles
- LEN : combien des 16 pas entrent dans le cycle.
- RATE : l'horloge interne, utilisée quand rien n'arrive sur CLK.
- MAP : le style, du droit au clairsemé.
- DNS1, DNS2, DNS3, DNS4 : la densité de chaque ligne. Plus haut, plus de coups.
- SWING : retarde un pas sur deux, ce qui donne du balancement.
- CHAOS : la probabilité de coups fantômes ou de coups prévus qui manquent, ce qui rend le rythme moins mécanique.
- RATCH : la probabilité qu'un coup devienne une rafale rapide de répétitions, un roulement.
- FILL : de combien l'entrée FILL augmente les densités, pour les breaks.
- DRIFT : laisse le style et les densités varier peu à peu, et le groove évolue tout seul.
Entrées : CLK, l'impulsion ; RST, pour revenir au début ; FILL, un signal qui déclenche le break ; MAP, pour moduler le style. Sorties : T1 à T4, les quatre lignes ; ACC, les accents ; ANY, une impulsion chaque fois qu'une ligne joue.

## Experimente
1. Branchez la sortie CLK du CLOCK sur l'entrée CLK de TRIGSEQ.
2. Placez trois DRUM dans le rack. Branchez T1, T2 et T3 sur l'entrée GATE de chacun, et leurs sorties OUT dans le MIXER.
3. Réglez TONE et DECAY de chaque DRUM pour qu'ils sonnent comme grosse caisse, caisse claire et charleston.
4. Tournez MAP lentement et écoutez le groove changer de style. Puis jouez avec les densités.
