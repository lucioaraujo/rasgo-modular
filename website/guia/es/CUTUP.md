## O que é
CUTUP recorta el propio sonido del patch a tempo. Guarda los últimos cuatro segundos y, cuando se lo llama, hace un gesto de montaje sobre los últimos tiempos del reloj: un tartamudeo, una ráfaga barajada, una repetición microscópica o un silencio.

## Como pensar nele
Es la técnica de recorte de William Burroughs y del collage de cinta, puesta en el bus. Los gestos llegan por gate, cada uno en su entrada, o solos con CHANCE. Cada fragmento tiene bordes suaves de 3 ms, y el gesto entero también, para que el corte no chasquee.

## Controles
- CHANCE: la frecuencia de los gestos espontáneos; en 0, solo por gate.
- MICRO: cuántas repeticiones tiene el gesto microscópico, de 2 a 8.
- MIX: la proporción entre el sonido original y el recortado.
- SEED: la semilla que elige gestos y rebanadas.
Entradas: IN recibe el sonido, CLK el reloj del patch, STUT dispara el tartamudeo, BURST la ráfaga de rebanadas barajadas, MICRO la repetición microscópica, GAP un tiempo de silencio. Salidas: OUT es el sonido recortado, ATIVO un gate mientras suena un gesto.

## Experimente
1. Conecte la salida del MIXER a IN, OUT al MASTER y el CLOCK a CLK.
2. Ponga CHANCE en 0,15: de vez en cuando, el patch se recorta justo a tiempo.
3. Conecte la salida euclidiana del CLOCK a STUT: el tartamudeo cae en los tiempos del ritmo.
4. Conecte el EOS de un SEQUENCE a GAP: cada frase termina con un silencio cortado.
