## O que é
PLANAR mezcla cuatro sonidos colocados en las esquinas de un cuadrado. Un punto se mueve dentro del cuadrado, y el sonido que sale es la mezcla de los cuatro, ponderada por la distancia del punto a cada esquina. Al mover el punto despacio, el timbre se transforma de un sonido en otro. La técnica se llama síntesis vectorial.

## Como pensar nele
Piense en un joystick: X mueve el punto de izquierda a derecha, Y de abajo arriba. Puede moverlo a mano, con dos ondas lentas o con un DRIFT, y el punto dibuja figuras. La posición también sale por las salidas X' e Y', lo que permite que el mismo movimiento conduzca otros controles del patch. Con un pulso largo en la entrada GST, PLANAR graba el recorrido que usted haga con las perillas X e Y y luego lo repite sin parar.

## Controles
- X, Y: la posición del punto en el cuadrado. Las señales de las entradas del mismo nombre se suman a estos valores.
- CURVE: la manera de mezclar. En un extremo, la mezcla es lineal, buena para señales de control; en el otro, mantiene el volumen constante, y el sonido no se hunde cuando el punto está en el centro.
- SMTH: hace que el punto se deslice a la nueva posición en lugar de saltar.
- RATE: la velocidad de repetición del gesto grabado y del paseo de DRIFT.
- DRIFT: hace que el punto pasee solo por el cuadrado.
Entradas: A, B, C y D, los cuatro sonidos; X e Y, para mover el punto; GST, para grabar y repetir un gesto. Salidas: OUT, la mezcla; X' e Y', la posición del punto.

## Experimente
1. Conecte cuatro fuentes distintas a A, B, C y D: por ejemplo la SAW de un OSC, OUT de un WAVETABLE, OUT del CHORD y PNK de un NOISE. Conecte OUT de PLANAR al MIXER.
2. Mueva X e Y a mano y escuche el sonido pasar de una esquina a otra.
3. Conecte BI de dos FUNCTION lentos, a velocidades distintas, a las entradas X e Y. El punto dibuja una figura y el timbre no deja de cambiar.
4. Conecte Y' a la entrada FC de un FILTER en el camino del sonido: el filtro sigue el movimiento.
