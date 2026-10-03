## O que é
CRUSH ensucia el sonido a la manera digital. Imita los defectos de los primeros samplers y videojuegos, y los de una mala conexión: menos muestras por segundo, menos escalones de volumen, números que se desbordan y vuelven por el otro lado, tramos que se traban o desaparecen. El resultado va de una leve aspereza a una voz de robot en pedazos.

## Como pensar nele
SHAPE deforma como un circuito analógico, redondeando y plegando la onda. CRUSH rompe el sonido en escalones. Cada control es un tipo distinto de daño, y puede usar uno solo o sumar varios. Los fallos se sortean, pero a partir de la semilla del patch, así que el mismo patch estropea el sonido siempre de la misma manera. Con una envolvente en la entrada MXM, la nota empieza limpia y se va degradando.

## Controles
- RATE: cuántas veces por segundo se lee el sonido, de 100 a 24000. Los valores bajos dejan el sonido en escalera y traen notas fantasma, el chisporroteo metálico de las máquinas de 8 bits.
- BITS: cuántos escalones de volumen quedan, de 1 a 16. En 16 casi no se nota; en 1, todo se vuelve onda cuadrada.
- DRIVE: la ganancia antes del daño, que empuja el sonido más allá de los límites.
- WRAP: qué pasa cuando el sonido supera el límite. En cero se recorta; al máximo reaparece por el lado opuesto, lo que suena mucho más agresivo, sobre todo con DRIVE alto.
- GLTCH: la probabilidad de un fallo: un tramo que se traba, un hueco de silencio, un rebote.
- JITR: vuelve irregular la lectura, y la afinación vacila un poco, como una cinta digital cansada.
- TONE: un filtro simple en la salida. A la izquierda oscurece; a la derecha deja solo el chisporroteo agudo; en el centro no cambia nada.
- MIX: la mezcla entre el sonido limpio y el estropeado. En cero, el sonido pasa intacto.
Entradas: IN, el sonido; RTM, para modular RATE; MXM, para modular MIX.

## Experimente
1. Conecte la salida SAW de un OSC a la entrada IN, y OUT a una entrada del MIXER.
2. Baje RATE a unos 4000 y BITS a 8. El sonido se vuelve granulado.
3. Suba GLTCH poco a poco y escuche aparecer los fallos.
4. Baje MIX a cero y conecte la salida ENV de un ENVELOPE a MXM, con el CLOCK en la entrada GATE de la envolvente. Cada nota empieza limpia y se deshace.
