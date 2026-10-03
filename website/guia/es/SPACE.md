## O que é
SPACE coloca el sonido en un lugar. Repite el sonido con retardo, una o varias veces, y puede esparcir esas repeticiones hasta que se fundan en una cola continua. Con pocos ajustes, va de un eco nítido a una sala amplia.

## Como pensar nele
Eco y reverberación son los dos extremos de un mismo camino. Un solo retardo, sin nada más, es un eco. Añada repeticiones, hágalas volver a la entrada y mézclelas, y el oído deja de distinguirlas: oye una sala. SPACE recorre ese camino entero. Su lugar está cerca del final del patch, antes del MIXER o del MASTER, y varias voces que pasan por el mismo SPACE suenan como si estuvieran en el mismo ambiente.

## Controles
- TIME: el tiempo de retardo, de 2 milisegundos a 2 segundos. Girarlo mientras pasa sonido da la impresión de que el espacio cambia de tamaño.
- TAPS: cuántas repeticiones salen en cada pasada, de 1 a 8.
- SPRD: esparce esas repeticiones en el tiempo. En cero salen juntas, como un solo eco; al máximo forman un patrón rítmico.
- FBK: cuánto del sonido repetido vuelve para repetirse otra vez. Más, más repeticiones y una cola más larga.
- DIFF: mezcla las repeticiones hasta que dejan de oírse una a una. Subiendo FBK y DIFF juntos, el eco se vuelve sala.
- TONE: el brillo de las repeticiones. Bajo, cada pasada es más oscura, como en un ambiente real; alto, el brillo se mantiene.
- MOD: una leve oscilación en el tiempo de las repeticiones, que hace la cola más ancha y centelleante.
- MIX: la mezcla entre el sonido original y el procesado, en la salida OUT.
Entradas: IN, el sonido; TIME y FBK, para modular esos controles. Salidas: OUT, la mezcla; WET, solo el sonido procesado.

## Experimente
1. Conecte una voz que toque notas cortas, como un MATTER tocado por el CLOCK, a la entrada IN, y OUT a una entrada del MIXER.
2. Ponga TAPS en 1, SPRD y DIFF en cero, y FBK en un tercio. Oye un eco limpio.
3. Suba DIFF y FBK juntos, despacio. El eco se transforma en sala sin ningún salto.
4. Mueva TIME con la cola sonando y escuche el espacio cambiar de tamaño.
