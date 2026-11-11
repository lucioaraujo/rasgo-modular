## O que é
TAP donne au patch le tempo de celui qui joue. Tapez trois fois ou plus sur l'entrée TAP (un bouton, une touche, des claquements de mains par le SIGNAL-IN) : il mesure l'intervalle et se met à donner une impulsion à chaque temps, tout seul, à votre tempo. Le premier coup est le temps fort, le un.

## Como pensar nele
C'est le décompte avant que le groupe n'entre. TAP prend la médiane des coups, donc un coup de travers ne gâche pas le compte. Si les coups sont trop irréguliers, il n'invente pas de tempo : il attend un meilleur décompte. Une pause et un nouveau décompte changent le tempo.

## Controles
- COMP : de combien de temps est la mesure, de 1 à 16. La sortie UM revient tous les COMP temps.
- DISP : combien d'irrégularité il accepte dans le décompte.
Entrées : TAP reçoit les coups, RST oublie le tempo. Sorties : CLK est l'impulsion à chaque temps, UM le temps fort, BPM le tempo en CV, TRAV un gate tant qu'il y a un tempo.

## Experimente
1. Branchez le SIGNAL-IN dans le SCOPE et la sortie ONS du SCOPE dans TAP. Activez le ON du SIGNAL-IN.
2. Branchez CLK dans l'entrée d'horloge externe du CLOCK : tout le patch suit votre pulsation.
3. Tapez un, deux, trois, quatre et commencez à jouer : le patch est déjà à votre tempo.
4. Mettez COMP à 3 et branchez UM dans le RST d'un SEQUENCE : la phrase recommence à chaque mesure de valse.
