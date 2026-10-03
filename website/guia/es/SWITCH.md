## O que é
SWITCH es un selector comandado por el patch. Puede elegir una de cuatro fuentes y mandarla a una salida, o tomar una sola fuente y mandarla a una de cuatro salidas. El cambio ocurre con cada pulso, en orden o al azar, o sigue una señal de control.

## Como pensar nele
En un patch que se transforma, cambiar de material importa tanto como cambiar de nota: la misma melodía tocada a veces por un oscilador, a veces por una cuerda; el filtro alimentado a veces por el ruido, a veces por el acorde. Sin SWITCH, eso exige cambiar cables a mano. Con DEMUX apagado, elige entre A, B, C y D y entrega en OA. Encendido, toma lo que llega a A y lo envía a una de las cuatro salidas, OA, OB, OC u OD.

## Controles
- STEP: cuántas posiciones recorre el selector, de 2 a 4.
- MODE: cómo cambia la posición: hacia adelante, de ida y vuelta, al azar, o solo por la señal de la entrada ADR.
- DEMUX: apagado, cuatro entradas a una salida; encendido, una entrada a cuatro salidas.
- GLID: una transición gradual entre posiciones, en la que las dos fuentes se mezclan por un instante. En cero, el corte es seco.
- SLEW: suaviza el cambio para evitar chasquidos. Conviene dejar siempre un poco.
Entradas: A, B, C y D, las fuentes; CLK, el pulso que avanza; RST, para volver a la primera posición; ADR, para elegir la posición con una señal. Salidas: OA, OB, OC y OD; STP, la posición actual como señal de control.

## Experimente
1. Conecte la salida SAW de un OSC a A, y OUT de un MATTER, tocado por el CLOCK, a B. Conecte OA a una entrada del MIXER.
2. Ponga STEP en 2 y conecte la salida DIV de un LOGIC a CLK, con la salida CLK del CLOCK en la entrada CLK del LOGIC y DIV en 4.
3. Cada cuatro pulsos, el sonido cambia de fuente. Suba GLID para que los cambios se fundan.
