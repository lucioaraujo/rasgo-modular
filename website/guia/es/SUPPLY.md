## O que é
SUPPLY es la alimentación del patch, puesta en un módulo con cables. En un circuito, cuando la batería se debilita, todo cambia junto: el reloj se ralentiza, el sonido pierde cuerpo y los osciladores se desafinan unos de otros. SUPPLY trae esa causa común al patch.

## Como pensar nele
ENERG es la tensión de alimentación. La carga, el propio sonido del patch, hace ceder esa tensión, como una batería bajo esfuerzo: cuanto más suena el patch, más se hunde el raíl. Lo que queda sale en TRILHO, para conectarlo donde usted quiera. Las derivas D1 a D4 solo aparecen con el raíl bajo y hacen que las voces se desencuentren; con el raíl lleno, vuelven juntas. Nada es invisible: SUPPLY solo actúa donde hay cable.

## Controles
- ENERG: la tensión de alimentación.
- QUEDA: cuánto hace ceder la carga al raíl.
- TEMPO: lo que tarda el raíl en ceder y volver, de 0,05 a 5 s.
- DERIVA: cuánto se desencuentran las voces con el raíl bajo.
Entradas: CARGA recibe el sonido del patch, ENERG mueve la tensión por CV. Salidas: TRILHO es la tensión que queda, D1 a D4 cuatro derivas de afinación (hasta 60 cents), FALHA un pulso en el apagón y otro en la vuelta.

## Experimente
1. Conecte la salida del MIXER a CARGA y TRILHO al CV1 del VCA de una voz: el patch pierde cuerpo cuando suena fuerte.
2. Conecte D1 al 1V/O de un OSC y D2 al de otro. Baje ENERG: las voces se desafinan una de otra.
3. Suba ENERG de nuevo: vuelven juntas.
4. Conecte FALHA al CIT del APOCRIFO: el apagón rasga el tiempo.
