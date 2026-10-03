## O que é
SH, de sample and hold, captura el valor de una señal en el instante de un pulso y lo mantiene hasta el pulso siguiente. Sin nada en la entrada, sortea un valor nuevo con cada pulso. Es la manera tradicional de convertir el azar en escalones: cada pulso, una nota nueva, un timbre nuevo. SH tiene dos canales.

## Como pensar nele
Dos sorteos independientes no tienen nada que ver entre sí, y a veces eso es lo que se busca. Pero muchas veces una melodía suena mejor cuando el timbre acompaña a la nota sin copiarla. El control CORR establece justamente esa relación entre los dos canales: de canales en espejo, donde uno sube cuando el otro baja, a independientes, y luego gemelos, que sortean el mismo valor. Los controles de deslizamiento convierten los escalones en glissandos.

## Controles
- RATE: el ritmo interno de sorteo, de 0,02 a 40 veces por segundo. Solo vale para el canal sin pulso conectado en T1 o T2.
- SLW1, SLW2: el tiempo que tarda cada canal en deslizarse hasta el valor nuevo. En cero, salta al instante.
- SLOPE: hace la subida más rápida que la bajada, o al revés.
- TRK1, TRK2: en lugar de capturar solo en el instante del pulso, el canal sigue la entrada mientras el pulso está alto y retiene el valor cuando cae.
- SPRD: cambia la manera de sortear, haciendo las variaciones pequeñas más frecuentes que las grandes.
- CORR: la relación entre los sorteos de los dos canales: en espejo, independientes o iguales. Solo vale para los canales sin nada en la entrada.
Entradas: IN1 e IN2, las señales a capturar; T1 y T2, los pulsos. Salidas: O1 y O2.

## Experimente
1. Arme una voz: la SAW de un OSC a la entrada IN de un FILTER, LO del FILTER al MIXER.
2. Conecte la salida CLK del CLOCK a T1 y a T2.
3. Conecte O1 a la entrada CV de un QUANTIZER, PTCH del QUANTIZER a 1V/O del OSC, y O2 a la entrada FC del FILTER. Cada pulso trae una nota y un timbre.
4. Gire CORR de un extremo al otro. De un lado, las notas agudas llegan oscuras; en el centro, sin relación; del otro, las notas agudas llegan brillantes.
