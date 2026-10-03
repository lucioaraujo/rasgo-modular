## O que é
NOISE produce ruido, el sonido de todas las frecuencias mezcladas al azar, en varios colores a la vez, cada uno en su salida: del blanco, siseante y brillante, al marrón, grave como el mar. También produce valores aleatorios lentos, que sirven para que otros módulos cambien solos.

## Como pensar nele
NOISE tiene dos usos muy distintos. Como sonido, es la materia del viento, la lluvia, los platillos y el chasquido de una caja. Como azar, es una fuente de variación para patches generativos: la salida S&H sortea un valor nuevo con cada pulso y lo mantiene hasta el siguiente, y SMTH se desliza de un valor a otro. Conecte una de ellas a la altura de un oscilador, pasando por un cuantizador, y tendrá una melodía que nunca se repite igual.

## Controles
- RATE: el ritmo de los sorteos de las salidas S&H y SMTH, de muy lento a 2000 por segundo. Si la entrada TRIG está conectada, manda ella en lugar de esta perilla.
- SLEW: cuánto tarda la salida SMTH en llegar a cada valor nuevo. En cero salta; al máximo se desliza despacio.
- SPRD: el tipo de sorteo. En cero, cualquier valor es igual de probable; al máximo, los valores cerca del centro salen más y los cambios se vuelven más suaves.
- POIS: cambia el ritmo regular de los sorteos por tiempos irregulares, al azar, manteniendo la misma media.
Salidas de sonido: WHT (blanco), PNK (rosa), BRN (marrón), BLU (azul), VLT (violeta) y BIT (ruido digital). Salidas de azar: S&H y SMTH. Entradas: TRIG, para disparar los sorteos, e IN.

## Experimente
1. Conecte PNK a una entrada del MIXER. Oye un siseo suave, como lluvia.
2. Cambie a BRN, más grave, y luego a WHT, más brillante.
3. Conecte S&H a la entrada CV de un QUANTIZER, y su salida PTCH a la entrada 1V/O de un OSC que esté sonando. La nota empieza a saltar al azar, siempre dentro de la escala.
4. Conecte la salida CLK del CLOCK a la entrada TRIG: los saltos ahora ocurren al ritmo del reloj.
