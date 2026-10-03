## O que é
PULSAR produce un tren de pequeños pulsos sonoros, cada uno seguido de un silencio. Cuando los pulsos se repiten rápido, se oye una nota; cuando se vuelven lentos, un ritmo. La técnica viene del compositor Curtis Roads y tiene una propiedad curiosa: la altura y el timbre se controlan por separado, así que se puede cambiar el color del sonido sin desafinarlo.

## Como pensar nele
FREQ decide cuántas veces por segundo se repite el pulso, y por lo tanto la nota. FRMT decide qué pasa dentro de cada pulso, y por lo tanto el timbre: bajo suena hueco, alto suena nasal. Baje FREQ a unas 30 repeticiones por segundo y cada pulso se vuelve un evento que se oye por separado; con MASK y JITR, esos eventos se vuelven irregulares, como una nube.

## Controles
- FREQ: la tasa de repetición de los pulsos, de 20 a 2000 Hz. Es la altura de la nota.
- FRMT: la frecuencia dentro de cada pulso, de 0,1 a 8 veces FREQ. Es el timbre: bajo es hueco, alto es brillante.
- SHAPE: la forma de cada pulso, de una senoidal simple a un pulso más estrecho y rico en armónicos.
- WIND: el contorno de cada pulso. Hacia la izquierda los bordes son duros y el sonido brillante; en el centro es limpio; hacia la derecha cada pulso ataca rápido y decae, como una percusión.
- JITR: vuelve irregulares el momento y el volumen de cada pulso. En cero, el tren es rígido.
- MASK: la probabilidad de que cada pulso se salte. Abre huecos y crea patrones rítmicos sin cambiar la nota.
- SPRD: reparte los pulsos alternos entre izquierda y derecha, ensanchando el sonido en estéreo.
- LEVEL: el volumen de salida.
Entradas: PIT para la altura y FQM para modular el timbre. Salidas L y R, para el estéreo.

## Experimente
1. Conecte L a una entrada del MIXER y gire FRMT despacio. El timbre cambia, pero la nota se queda en su lugar.
2. Lleve FREQ cerca del mínimo. La nota se deshace en pulsos que se oyen uno a uno.
3. Con FREQ bajo, suba MASK y JITR. Los pulsos se vuelven escasos e irregulares, como una lluvia.
4. Conecte la salida BI de un FUNCTION lento a la entrada FQM. El timbre pasea solo sin desafinar.
