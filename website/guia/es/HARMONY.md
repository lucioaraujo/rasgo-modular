## O que é
HARMONY decide por qué tonalidades pasa la música. De vez en cuando, o con cada pulso en la entrada ADV, elige un nuevo centro tonal, siguiendo uno de seis movimientos que los músicos de jazz y de cine usan desde hace décadas. La salida ROOT entrega la nueva tónica en la misma escala de altura que entienden los osciladores.

## Como pensar nele
QUANTIZER mantiene la melodía afinada, pero siempre en el mismo tono. La música suele viajar, y cada estilo viaja a su manera. El control MOVE elige el recorrido: Coltrane, que salta una tercera mayor en cada cambio y cierra un ciclo de tres tonos, como en Giant Steps; sustitución tritonal, que llega al tono siguiente por un camino inesperado; mediante cromática, cambios de tercera con aire de banda sonora; intercambio modal, que mantiene la tónica y cambia el modo; jazz modal, que casi nunca se mueve; y backdoor, que sube un tono entero.

En esta versión, la manera de oír los cambios es sumar la salida ROOT a la melodía antes del oscilador. Cada entrada acepta un solo cable, así que la suma pasa por un MATRIX, y toda la melodía se transpone al nuevo centro. El cambio de escala, de la salida SCALE, todavía no llega al QUANTIZER por cable.

## Controles
- MOVE: el tipo de recorrido, entre los seis descritos arriba.
- RATE: el ritmo de los cambios, cuando no llega nada a ADV. Va de un cambio cada pocos minutos a dos por segundo.
- ROOT: la tónica de partida, a la que vuelve el módulo cuando recibe un pulso en RST.
- S-LO, S-HI: el rango de escalas que pueden sortearse en los cambios.
- HOLD: la probabilidad de que un cambio se ignore, lo que alarga algunas secciones.
Entradas: ADV, el pulso que pide un cambio; RST, para volver al principio. Salidas: ROOT, la tónica como altura; SCALE, el número de la escala; CHG, un pulso en cada cambio.

## Experimente
1. Arme la melodía del QUANTIZER: la salida CV de un TURING, movido por el CLOCK, a la entrada CV del QUANTIZER, y un OSC que pase por un ENVELOPE hasta el MIXER.
2. Conecte PTCH del QUANTIZER a IN1 de un MATRIX, ROOT del HARMONY a IN2, y OUT1 del MATRIX a 1V/O del OSC. En el MATRIX, lleve la celda 21 a 1: OUT1 pasa a ser la melodía más la tónica.
3. Ponga MOVE en la primera posición, Coltrane, y suba RATE. En cada cambio, la melodía salta a otro centro, en ciclos de tres.
4. Conecte CHG a la entrada ADV de un DRIFT para que el resto del patch también cambie de sección en cada cambio.
