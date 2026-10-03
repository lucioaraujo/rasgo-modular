## O que é
LPG, de low pass gate, es una compuerta que abre y cierra el sonido con cada golpe, bajando a la vez el volumen y el brillo. Imita un componente antiguo, el vactrol, que reacciona rápido pero suelta despacio, y por eso cada nota suena golpeada: una marimba, una kalimba, una gota.

## Como pensar nele
Se podría armar algo parecido con FILTER, VCA y ENVELOPE, pero faltaría la manera del vactrol: una subida instantánea y una caída que se va frenando cerca del final. LPG lo trae de serie. Basta un sonido continuo en IN y pulsos en STRK: cada pulso se vuelve una nota tocada. MODE elige si actúa más como filtro, más como control de volumen o como ambos.

## Controles
- MODE: hacia la izquierda, solo filtro; hacia la derecha, solo volumen; en el centro, ambos juntos, el sonido característico del LPG.
- RESP: cuánto tarda la nota en morir después del golpe, de un toque brevísimo a unos dos segundos y medio. La subida siempre es rápida.
- OFST: cuánto queda abierta la compuerta en reposo. En cero se cierra del todo entre golpes.
- RESO: la resonancia del filtro, que da un tono más marcado.
- BNCE: añade un pequeño rebote justo después de cada golpe.
- DRIFT: varía levemente, y despacio, la duración de cada nota.
Entradas: IN, el sonido; STRK, el pulso que golpea; CV, para abrir la compuerta con una señal continua.

## Experimente
1. Conecte la salida TRI de un OSC a la entrada IN, y OUT a una entrada del MIXER. Por ahora, silencio: la compuerta está cerrada.
2. Conecte la salida EUC del CLOCK a la entrada STRK. Cada pulso se vuelve una nota de timbre golpeado.
3. Gire RESP para oír las notas acortarse o alargarse.
4. Conecte la salida PTCH de una SEQUENCE, movida por el mismo CLOCK, a la entrada 1V/O del OSC. Ahora tiene una línea de marimba.
