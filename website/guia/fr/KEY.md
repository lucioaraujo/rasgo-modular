## O que é
KEY écoute celui qui joue, estime la tonalité et transmet la tonique et la gamme au QUANTIZER. Le patch répond alors dans la tonalité de celui qui chante ou joue par-dessus, au lieu de rester dans la gamme choisie par la seed.

## Como pensar nele
Il compte combien de temps chacune des douze notes a sonné (une note longue pèse plus qu'une note de passage) et compare ce portrait aux profils tonals de Krumhansl et Kessler. La tonalité ne change que si une autre l'emporte pendant ESTAB notes d'affilée, pour ne pas hésiter entre tonalités voisines. La sortie CERT dit quand la tonalité est ambiguë, au lieu de le cacher. L'afficheur montre les douze notes, la tonique et le nom de la tonalité.

## Controles
- MEM : combien de temps il se souvient, de 2 à 60 s. Court, il suit les modulations ; long, il reste ferme.
- ESTAB : pendant combien de notes d'affilée une nouvelle tonalité doit l'emporter pour compter.
Entrées : PTCH reçoit la hauteur, GATE marque chaque note, RST oublie tout. Sorties : ROOT est la tonique, SCL la gamme, CERT la certitude, MUD une impulsion à chaque changement de tonalité.

## Experimente
1. Branchez le SIGNAL-IN dans le SCOPE, la sortie PIT du SCOPE dans PTCH et la sortie ONS dans GATE. Branchez ROOT et SCL dans les entrées de tonique et de gamme du QUANTIZER.
2. Activez le ON du SIGNAL-IN et chantez une mélodie en ré mineur : l'afficheur montre Dm et le patch se met à jouer dans cette tonalité.
3. Branchez MUD dans l'entrée FRASE du MIXER, avec la scène en mode geste : chaque changement de tonalité déplace la scène.
4. Sans micro : un MARKOV dans un QUANTIZER, ce QUANTIZER dans KEY et KEY dans un second QUANTIZER. La seconde voix suit la tonalité que la première semble avoir.
