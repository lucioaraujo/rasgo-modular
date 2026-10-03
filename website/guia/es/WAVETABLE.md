## O que é
WAVETABLE es un oscilador que guarda una secuencia de 16 formas de onda distintas, como los fotogramas de una animación, de la más brillante (una sierra) a la más suave (una senoidal). La perilla POS elige en qué punto de esa secuencia está usted. Al mover POS, el timbre cambia de forma continua, sin necesidad de filtro.

## Como pensar nele
Mientras OSC da un sonido fijo que se esculpe después, WAVETABLE nace en movimiento: basta con que algo mueva POS. Un LFO lento hace una textura que respira; una envolvente hace que cada nota empiece brillante y termine oscura. También sabe escuchar: conectado a SIGNAL-IN, captura un ciclo del sonido que entra y lo usa como forma de onda.

## Controles
- FREQ: la altura de la nota, de 8 a 8000 Hz.
- FINE: afinación fina, hasta un semitono hacia cada lado.
- POS: la posición en la tabla. Al principio, el sonido está lleno de armónicos; al final, es casi una senoidal. La entrada POS se suma a esta perilla, y es por ella que el timbre empieza a moverse solo.
- WARP: deforma la lectura de cada ciclo y añade armónicos con acento digital, sin cambiar de fotograma.
- FM: cuánto mueve la altura la señal de la entrada FM.
- DRIFT: una pequeña oscilación lenta en la afinación.
Entradas: 1V/O para la altura, POS para mover la posición, FM, CAP para el sonido a capturar y GRAB, un pulso que ordena capturar un ciclo nuevo.

## Experimente
1. Conecte OUT a una entrada del MIXER y gire POS de punta a punta. Escuche el timbre pasar de áspero a liso.
2. Conecte la salida de un FUNCTION lento a la entrada POS. Ahora el sonido cambia solo, en ciclos.
3. Para capturar: conecte SIGNAL-IN a la entrada CAP y un pulso del CLOCK a GRAB. Con cada pulso, el oscilador empieza a tocar un trozo del sonido que entra.
