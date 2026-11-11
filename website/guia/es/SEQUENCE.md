## O que é
SEQUENCE toca una frase corta que usted escribe: hasta ocho notas, cada una con su altura, cada una encendida o en silencio. Con cada pulso en la entrada CLK avanza un paso, entregando la altura en la salida PTCH y el pulso de la nota en la salida GATE. Sirve para una línea de bajo, un riff, una figura que se repite.

## Como pensar nele
TURING hace nacer la frase del azar; SEQUENCE parte de una frase que usted decide. La variación viene de la manera de leer: hacia adelante, hacia atrás, de ida y vuelta, en orden sorteado, o vagando de un paso a su vecino. La misma frase de ocho notas da mucho de sí cambiando solo MODE y LEN. Para que las alturas caigan en las notas de una escala, pase PTCH por un QUANTIZER antes del oscilador.

## Controles
- LEN: cuántos de los ocho pasos entran en la frase.
- MODE: el orden de lectura: hacia adelante, hacia atrás, ida y vuelta, al azar, vagando paso a paso, o el cursor, un cuerpo que avanza por los pasos empujado por el reloj y atraído o repelido por un objetivo.
- RATE: el reloj interno, usado cuando no llega nada a CLK.
- GATE: cuánto tiempo queda encendida cada nota dentro del paso. Corto suena separado; largo, ligado.
- GLIDE: hace que la altura se deslice de una nota a la siguiente.
- RANGE: cuántas octavas cubren los ocho valores, hasta dos.
- P1 a P8: la altura de cada paso.
- G1 a G8: enciende o silencia cada paso.
- INERC: en el modo cursor, cuánto conserva el cursor su velocidad de un pulso al siguiente.
- FORCA: en el modo cursor, la fuerza hacia el paso ALVO: positiva atrae, y el cursor se demora cerca de él; negativa repele.
- ALVO: el paso que atrae o repele al cursor.
- FADIG: la fatiga. Cada paso que suena se cansa y, demasiado cansado, calla esa vez: la repetición se gasta. Vale en cualquier modo.
Entradas: CLK, el pulso que avanza; RST, para volver al primer paso. Salidas: PTCH, la altura; GATE, el pulso de la nota; EOS, un pulso al final de cada frase.

## Experimente
1. Conecte la salida CLK del CLOCK a CLK de la SEQUENCE.
2. Conecte PTCH a la entrada CV de un QUANTIZER, y PTCH del QUANTIZER a 1V/O de un OSC. Haga pasar el OSC por un ENVELOPE hasta el MIXER y conecte GATE de la SEQUENCE a GATE de la envolvente.
3. Ajuste P1 a P8 hasta que la frase le guste y silencie algunos pasos con G1 a G8.
4. Cambie MODE y escuche la misma frase leída de otras maneras.
