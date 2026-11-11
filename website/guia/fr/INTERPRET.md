## O que é
INTERPRET joue les notes qui arrivent comme le ferait un interprète. Il n'invente pas de notes : il reçoit la hauteur et le gate de n'importe quel séquenceur et rend la note jouée, avec un glissé sur l'intervalle, un léger dépassement dans les sauts, un vibrato qui grandit sur la note longue, une articulation selon le contexte, du souffle et de la brillance.

## Como pensar nele
La partition dit la note ; l'interprète dit comment. Le module regarde les notes récentes : il marque le saut, soutient le sommet de la phrase, raccourcit deux notes rapides, lie le reste. Le souffle s'use au fil de la phrase et revient dans une pause. Trois axes de caractère (ENERG, SUAV, BRILHO) donnent la personnalité, et DERIVA la fait changer lentement. Avec EXPR à 0, la note sort brute.

## Controles
- EXPR : la part d'interprétation ; à 0, la note brute.
- GLIDE : le temps de glisser entre les notes, jusqu'à 0,3 s.
- VIB : la profondeur du vibrato sur les notes longues.
- ENERG : le caractère énergique, qui marque les sauts.
- SUAV : le caractère doux, qui lie les notes.
- BRILHO : le caractère brillant, qui ouvre le timbre au sommet.
- DERIVA : combien le caractère change avec le temps.
Entrées : PTCH reçoit la hauteur, GATE le gate de chaque note, CAR déplace le caractère par CV. Sorties : PTCH est la hauteur jouée, AMP la dynamique, BRT la brillance, FRASE la position dans la phrase, GT le gate articulé.

## Experimente
1. Branchez la hauteur et le gate d'un QUANTIZER dans PTCH et GATE. Branchez la sortie PTCH dans le 1V/O d'un OSC et GT dans l'enveloppe de la voix.
2. Mettez EXPR à 0 puis à 0,8 : comparez la note brute et la note jouée.
3. Branchez AMP dans le CV1 du VCA de la voix et BRT dans la coupure du filtre : la ligne gagne dynamique et couleur.
4. Montez VIB et GLIDE sur une ligne lente : elle se met à chanter.
