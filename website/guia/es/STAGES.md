## O que é
STAGES dibuja una curva hecha de varios tramos encadenados, de dos a ocho. Según los ajustes, esa curva se vuelve una envolvente de varias etapas, un movimiento lento que nunca se repite exactamente, una secuencia de valores en escalones, o incluso un sonido de forma extraña.

## Como pensar nele
FUNCTION hace una sola rampa, y ENVELOPE las cuatro etapas de siempre. STAGES hace formas compuestas, y usted no dibuja punto por punto: unos pocos controles moldean todo el dibujo a la vez. El control que más cambia su carácter es HOLD. En cero, los tramos son rampas y el resultado es un movimiento continuo; al máximo, cada tramo salta y se mantiene, y STAGES pasa a funcionar como un secuenciador. Con LOOP encendido, la forma se repite sola; apagado, corre una vez con cada pulso en GATE.

## Controles
- SEGS: cuántos tramos tiene la forma, de 2 a 8.
- RATE: la velocidad de una vuelta completa, cuando LOOP está encendido.
- CNTR: el dibujo de los niveles: una escalera que sube, un arco, o una escalera que baja.
- CURVE: cómo va cada tramo de un nivel al otro: rápido al principio, en línea recta, o despacio al principio.
- HOLD: en cero, rampas suaves; al máximo, escalones que saltan y se mantienen.
- TILT: hace los primeros tramos más largos que los últimos, o al revés.
- JITR: hace variar levemente niveles y duraciones en cada vuelta, siempre igual para la misma semilla.
- LOOP: encendido, la forma se repite; apagado, corre una vez por pulso.
Entradas: GATE, el pulso que dispara; RST, para volver al principio; RTM, para modular la velocidad. Salidas: OUT, la curva; EOC, un pulso al final de cada vuelta; STEP, un pulso en cada tramo.

## Experimente
1. Arme una voz: la SAW de un OSC a la entrada IN de un FILTER, LO del FILTER al MIXER. Baje CUT.
2. Conecte OUT de STAGES a la entrada FC del FILTER, con SEGS en 6. El timbre pasea siguiendo una forma que va y viene.
3. Mueva CNTR y TILT y escuche cambiar el dibujo.
4. Lleve HOLD al máximo y SEGS a 8. Conecte OUT a la entrada CV de un QUANTIZER y PTCH del QUANTIZER a 1V/O del OSC: STAGES ahora toca una melodía.
