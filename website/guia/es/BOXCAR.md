## O que é
BOXCAR mide una señal en ventanas cortas y promedia muchas mediciones seguidas. La técnica viene de instrumentos de laboratorio usados para encontrar una señal débil escondida en el ruido. En el patch, sirve como seguidor de volumen muy estable, como reconstructor de formas de onda y como oscilador hecho de lo que midió.

## Como pensar nele
SH captura un instante; BOXCAR captura una porción de tiempo y la promedia. Con cada pulso en TRIG, abre una ventana en un punto del ciclo, mide y suma esa medición a las anteriores. Lo que se repite igual en cada ciclo se va afirmando, y lo que es azar se cancela. MODE elige qué hacer con el resultado: entregar solo el promedio, como una señal de control que sigue al sonido despacio; reproducir el ciclo reconstruido; o releer ese ciclo por su cuenta, como un oscilador. Tiene además una salida aparte, GEIG, con pulsos irregulares como los de un contador Geiger.

## Controles
- DLY: en qué punto del ciclo se abre la ventana.
- APER: el ancho de la ventana. Estrecha, mide casi un instante; ancha, promedia un tramo.
- AVG: cuántas mediciones entran en el promedio, de 1 a 64. Más mediciones, menos ruido y una respuesta más lenta.
- SCAN: hace que la ventana recorra el ciclo sola, en un sentido o en el otro. En cero queda quieta.
- MODE: seguidor, reconstrucción u oscilador.
- RATE: el reloj interno, cuando no llega nada a TRIG, y la velocidad de relectura en modo oscilador.
- THRSH: el nivel que la señal debe cruzar para disparar una medición, cuando no llega nada a TRIG.
- GEI: la densidad de los pulsos en la salida GEIG. En cero queda muda.
- BLEND: la mezcla entre la señal original y la procesada.
Entradas: IN, la señal a medir; TRIG, el pulso de cada medición; SWP, para mover la ventana; THR, para modular el umbral. Salidas: OUT y GEIG.

## Experimente
1. Conecte GEIG a la entrada GATE de un DRUM conectado al MIXER y suba GEI. Oye golpes en momentos imprevisibles.
2. Ahora conecte una voz que toque notas, como un OSC que pase por un ENVELOPE, a la entrada IN, con MODE en la primera posición y APER ancha.
3. Conecte OUT a la entrada CV1 de un VCA que controle otro sonido. El segundo sonido sigue ahora el volumen de la voz, sin temblar.
