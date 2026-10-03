## O que é
QUANTIZER ata una señal continua a las notas de una escala. Un valor que sube y baja libremente, venido de un TURING, de un SH o de un CHAOS, sale de él como una sucesión de notas afinadas. En la mayoría de los patches, es él quien convierte el azar en melodía.

## Como pensar nele
Las señales de control varían de forma continua, y las notas musicales no. Sin un cuantizador, una melodía generada al azar cae entre las notas y suena desafinada. QUANTIZER redondea cada valor a la nota más cercana de la escala elegida. Con un pulso en la entrada TRIG, solo cambia de nota en el pulso, y la melodía gana ritmo. La salida GATE pulsa cada vez que la nota cambia de verdad, lo que sirve para disparar una envolvente solo en las notas nuevas.

## Controles
- SCALE: la escala, entre doce: cromática, mayor, menor natural, dórica, frigia dominante, lidia, menor melódica, pentatónica menor, pentatónica mayor, hirajoshi, tonos enteros y solo octavas.
- ROOT: la tónica, la nota de partida de la escala, de do a si.
- RANGE: cuántas octavas cubre la señal de entrada, de 1 a 6. Más octavas, saltos más grandes.
- GLIDE: hace que la altura se deslice de una nota a la siguiente.
- HYST: un margen de tolerancia. Con una señal inestable, la nota no salta sin parar entre dos vecinas.
Entradas: CV, la señal a cuantizar; TRSP, para transponer antes de cuantizar; TRIG, el pulso que decide cuándo cambiar de nota. Salidas: PTCH, la altura; GATE, un pulso en cada nota nueva; ST, la nota en semitonos.

## Experimente
1. Conecte la salida CV de un TURING a la entrada CV del QUANTIZER, y la salida CLK del CLOCK a la entrada CLK del TURING.
2. Conecte PTCH a 1V/O de un OSC, y la salida SIN del OSC a un ENVELOPE conectado al MIXER. Conecte GATE del QUANTIZER a GATE de la envolvente.
3. Cambie SCALE y escuche la misma sucesión de valores volverse otra melodía. Cambie ROOT para transponer.
