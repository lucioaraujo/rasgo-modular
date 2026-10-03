## O que é
SIGNAL-IN trae el mundo de fuera al patch. Recibe el sonido de la entrada de audio del ordenador, como un micrófono u otro instrumento, y las notas de un teclado o controlador MIDI, y lo convierte todo en señales que los demás módulos entienden.

## Como pensar nele
Con él, alguien de fuera puede tocar Rasgo Modular: la altura de la tecla va a un oscilador, la pulsación de la tecla abre una envolvente y la rueda de modulación del teclado puede abrir un filtro. También deja que el patch procese sonido de fuera, pasando una voz o una guitarra por filtros y espacios. El módulo empieza apagado: la aplicación solo abre el micrófono y el MIDI después de que usted encienda ON y conecte alguna salida.

## Controles
- GAIN: el volumen del audio que entra, de cero al doble.
- BEND: cuántos semitonos mueve la altura la rueda de pitch bend del teclado, de cero a dos octavas.
- CC#: qué controlador del teclado sigue la salida CC. El número 1 suele ser la rueda de modulación.
- ON: enciende la entrada de fuera. Apagado, que es como empieza el módulo, no entra nada al patch.
Salidas: L y R, el audio que entra; 1V/O, la altura de la tecla tocada; GATE, abierto mientras la tecla está pulsada; VEL, la fuerza con que se tocó; CC, el valor del controlador elegido.

## Experimente
1. Con un micrófono o instrumento conectado al ordenador, conecte L a una entrada del MIXER, encienda ON y ajuste GAIN hasta oír el sonido.
2. Haga pasar ese sonido por un FILTER o por SPACE antes del MIXER para transformarlo.
3. Con un teclado MIDI y ON encendido, conecte 1V/O a la entrada 1V/O de un OSC, y GATE a la entrada GATE de un ENVELOPE que tenga el sonido del OSC en su entrada IN. El teclado ahora toca el patch.
