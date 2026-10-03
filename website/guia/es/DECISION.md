## O que é
DECISION es la fuente de elecciones del patch. Con cada pulso decide si dispara o no una señal en la salida GATE, y sortea dos valores en las salidas X e Y. Usted controla la probabilidad de disparar, el tamaño y la forma de los sorteos, y cuánto recuerda de lo que ya hizo.

## Como pensar nele
Una onda lenta es demasiado previsible, y el ruido puro no tiene forma. DECISION está en el medio: azar con reglas. BIAS decide si algo sucede casi siempre o solo a veces. SHAPE decide si los valores se reparten por igual o se concentran cerca del centro, con saltos grandes poco frecuentes. DEJA es la memoria: en lugar de sortear de nuevo, relee los últimos pasos, y la música gana repeticiones, como un tema que vuelve.

## Controles
- RATE: el reloj interno, usado cuando no llega nada a TRIG.
- BIAS: la probabilidad de que GATE se dispare con cada pulso. En cero, nunca; al máximo, siempre.
- SPRD: el alcance de los sorteos en X e Y. Bajo, valores cercanos al centro; alto, todo el rango.
- SHAPE: en cero, todos los valores tienen la misma probabilidad; al máximo, los valores cercanos al centro salen mucho más.
- STEPS: divide los sorteos en escalones. En 1 son continuos.
- SLEW: hace que cada valor nuevo se deslice desde el anterior.
- DEJA: la probabilidad de repetir un valor de la memoria en lugar de sortear otro. Alto, la misma frase tiende a repetirse.
- LOOP: el tamaño de esa memoria, de 1 a 16 pasos.
Entradas: TRIG, el pulso de cada decisión; BIAS y SPRD, para modular esos controles. Salidas: X e Y, los valores sorteados; GATE, la decisión.

## Experimente
1. Conecte la salida EUC del CLOCK a TRIG.
2. Conecte X a la entrada CV de un QUANTIZER, PTCH a 1V/O de un OSC, y el OSC por un ENVELOPE hasta el MIXER. Conecte GATE del DECISION a GATE de la envolvente.
3. Baje BIAS y escuche cómo las notas se vuelven escasas, solo de vez en cuando.
4. Suba DEJA poco a poco. La melodía empieza a repetirse, y con DEJA al máximo queda atrapada en un bucle de LOOP pasos.
