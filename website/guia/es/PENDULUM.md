## O que é
PENDULUM es un péndulo doble: un péndulo con otro colgado de su punta. Con poca energía, oscila casi como un reloj; con mucha, el brazo de abajo da vueltas y el movimiento nunca se repite. Las posiciones de las dos puntas salen como CV.

## Como pensar nele
Es el ejemplo clásico del caos: una ley simple, un resultado imprevisible. ENRGY es el botón del caos. Abajo, orden; arriba, vueltas. CROSS marca el ritmo de la oscilación, y FLIP cada vuelta completa, el instante en que aparece el caos.

## Controles
- ENRGY: la energía de la oscilación; por encima de la mitad, aparece el caos.
- RATIO: la razón entre las longitudes de los dos brazos.
- MASS: la razón entre las masas.
- SPEED: la velocidad del tiempo del péndulo.
- DAMP: el rozamiento; con él, el péndulo va perdiendo fuerza.
- AUTO: da un empujón cuando el péndulo casi se detiene.
Entradas: KICK da un empujón, SPD mueve la velocidad por CV. Salidas: X1 e Y1 son la punta del primer brazo, X2 e Y2 la del segundo, CROSS un gate en cada paso por el centro, FLIP un gate en cada vuelta completa.

## Experimente
1. Conecte X2 a un QUANTIZER y el QUANTIZER a un OSC, y CROSS a un ENVELOPE que controla el VCA de la voz: una melodía que se balancea.
2. Suba ENRGY despacio y escuche la melodía pasar de regular a imprevisible.
3. Conecte FLIP al ADV del HARMONY: el acorde cambia cuando aparece el caos.
4. Conecte X1 al pan de un canal del MIXER: el sonido gira en el espacio.
