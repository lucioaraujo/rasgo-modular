## O que é
HARMONY decide por qué tonalidades pasa la música. De vez en cuando, o con cada pulso en la entrada ADV, elige un nuevo centro tonal, siguiendo uno de seis movimientos que los músicos de jazz y de cine usan desde hace décadas. La salida ROOT entrega la nueva tónica en la misma escala de altura que entienden los osciladores.

## Como pensar nele
QUANTIZER mantiene la melodía afinada, pero siempre en el mismo tono. La música suele viajar, y cada estilo viaja a su manera. El control MOVE elige el recorrido: Coltrane, que salta una tercera mayor en cada cambio y cierra un ciclo de tres tonos, como en Giant Steps; sustitución tritonal, que llega al tono siguiente por un camino inesperado; mediante cromática, cambios de tercera con aire de banda sonora; intercambio modal, que mantiene la tónica y cambia el modo; jazz modal, que casi nunca se mueve; y backdoor, que sube un tono entero.

Conecte la salida ROOT a la entrada ROOT del QUANTIZER y la salida SCALE a la entrada SCL. En cada cambio, la escala de la melodía cambia, y cada nota pasa a la más cercana de la escala nueva: la melodía se queda en el mismo registro, con otro color. Para oír la melodía saltar al nuevo centro, conecte también la salida ROOT a la entrada TRSP del QUANTIZER.

## Controles
- MOVE: el tipo de recorrido, entre los seis descritos arriba.
- RATE: el ritmo de los cambios, cuando no llega nada a ADV. Va de un cambio cada pocos minutos a dos por segundo.
- ROOT: la tónica de partida, a la que vuelve el módulo cuando recibe un pulso en RST.
- S-LO, S-HI: el rango de escalas que pueden sortearse en los cambios.
- HOLD: la probabilidad de que un cambio se ignore, lo que alarga algunas secciones.
Entradas: ADV, el pulso que pide un cambio; RST, para volver al principio. Salidas: ROOT, la tónica como altura; SCALE, el número de la escala; CHG, un pulso en cada cambio.

## Experimente
1. Arme la melodía del QUANTIZER: la salida CV de un TURING, movido por el CLOCK, a la entrada CV del QUANTIZER, y PTCH del QUANTIZER a 1V/O de un OSC que pase por un ENVELOPE hasta el MIXER.
2. Conecte ROOT del HARMONY a la entrada ROOT del QUANTIZER, y SCALE a la entrada SCL.
3. Ponga MOVE en la primera posición, Coltrane, y suba RATE hasta un cambio cada dos o tres segundos. En cada cambio, la melodía cambia de escala.
4. Conecte también ROOT del HARMONY a la entrada TRSP del QUANTIZER. Ahora la melodía salta de centro en centro, en ciclos de tres.
