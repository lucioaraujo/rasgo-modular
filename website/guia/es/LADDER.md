## O que é
LADDER es el filtro en escalera, el filtro del grave: cuatro etapas de 6 dB en cascada, con la salida de la última devuelta a la entrada como resonancia. Es el diseño que R. Moog patentó en 1969, aquí con un mando que va de la escalera de transistores, redonda, a la escalera de diodos, ácida.

## Como pensar nele
El FILTER es limpio y estable; el LADDER es el filtro que se comprime y engorda cuando se lo empuja. Tres cosas lo distinguen. DIODO acopla las etapas, y a medio camino hay escaleras que no existen como circuito. El pico queda afinado en CUT en cualquier posición de DIODO, y con RESO en 1 la escalera canta un seno afinado. Y COMP devuelve el grave que, en la escalera original, desaparece cuando sube la resonancia.

## Controles
- CUT: la frecuencia de corte, de 20 Hz a 20 kHz.
- RESO: la resonancia. En 1, la escalera autooscila; 0,9 significa la misma cercanía al umbral en cualquier DIODO.
- DRIVE: empuja la entrada contra la saturación dentro del lazo; el nivel de salida se compensa.
- COMP: devuelve el grave. En 1, el grave queda entero con cualquier resonancia; en 0, se hunde como en el original.
- DIODO: de 0 (transistores, redondo) a 1 (diodos, ácido), de forma continua.
Entradas: IN recibe el sonido, FC mueve el corte a un voltio por octava, Q mueve la resonancia. Salidas: LP4 es la salida de 24 dB por octava, LP2 la toma de la segunda etapa, de 12 dB por octava, más brillante.

## Experimente
1. Conecte la salida SAW de un OSC a IN, LP4 a un VCA y el VCA al MIXER. Ponga CUT cerca de 200 Hz.
2. Suba RESO con COMP en 1: el pico canta y el grave se queda. Baje COMP y escuche el grave hundirse.
3. Ponga DIODO en 1, RESO en 0,85 y DRIVE en 0,6, con una envolvente corta en FC: el bajo ácido.
4. Desconecte IN, ponga RESO en 1 y conecte la altura de un QUANTIZER a FC: la escalera toca la melodía como un oscilador.
