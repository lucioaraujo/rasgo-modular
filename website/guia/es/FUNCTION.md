## O que é
FUNCTION genera una rampa que sube y baja sin parar. Lenta, sirve para mover otros controles, lo que se llama LFO, oscilador de baja frecuencia; rápida, se vuelve un sonido audible; disparada por un pulso, funciona como una envolvente simple. Es la fuente de movimiento más usada de Rasgo Modular.

## Como pensar nele
Envolvente, LFO y oscilador son lo mismo a velocidades distintas, y FUNCTION los cubre todos, de un ciclo cada cien segundos a miles por segundo. Cuando una guía dice que conecte un LFO en algún lugar, casi siempre se trata de un FUNCTION lento. La forma va de una rampa que baja a una que sube, pasando por el triángulo, y las dos salidas entregan la misma forma en escalas distintas: UNI solo por encima de cero, BI por encima y por debajo.

## Controles
- RATE: la velocidad, de 0,01 a 12000 ciclos por segundo.
- SLOPE: la forma. En un extremo, sube de golpe y baja despacio; en el centro, triángulo; en el otro extremo, sube despacio y cae de golpe.
- DRIFT: varía la velocidad poco a poco, para que el movimiento no suene a metrónomo.
- SYNC: activa la entrada SYNC, para que cada pulso reinicie la rampa.
Entradas: RATE y SLOPE, para modular esos controles; SYNC, el pulso que reinicia. Salidas: UNI y BI.

## Experimente
1. Arme una voz: la SAW de un OSC a la entrada IN de un FILTER, LO del FILTER al MIXER. Baje CUT.
2. Conecte BI del FUNCTION a la entrada FC del FILTER, con RATE bajo. El timbre se abre y se cierra despacio.
3. Suba RATE a unos 5 Hz y conecte BI a la entrada FM del OSC, con el FM del OSC muy bajo. La altura tiembla, en un vibrato.
4. Suba RATE hasta la zona audible y conecte BI directamente al MIXER: el propio FUNCTION se vuelve sonido.
