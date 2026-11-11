## O que é
ACCENT da fraseo a una línea. Un intérprete no acentúa solo con el volumen: la nota acentuada es más fuerte, más brillante y más corta, y hay muchas razones para acentuarla. ACCENT reúne esas causas, cada una con su peso, en un énfasis de 0 a 1, y entrega ese énfasis a varias salidas a la vez: nivel, brillo, duración, barrido.

## Como pensar nele
Hay seis causas de acento: el tiempo fuerte (METR), el tamaño del salto (SALTO), la nota más aguda de la frase reciente (APICE), el acento que viene de fuera (EXT), la nota tras una nota larga (AGOG) y un acento de fuera en un tiempo débil (SINC). Tres controles moldean el conjunto: HER deja una cola en la nota siguiente, SUPR impide que todo se vuelva acento, ROT hace que el tiempo fuerte migre despacio. PROF es cuánto de todo esto llega a las salidas.

## Controles
- METR: el peso del tiempo fuerte de cada ciclo.
- SALTO: el peso del tamaño del intervalo desde la nota anterior.
- APICE: el peso de la nota más aguda de las últimas ocho.
- EXT: el peso del acento que llega por la entrada ACC.
- AGOG: el peso de la nota que sigue a una nota larga.
- SINC: el peso de un acento de fuera en un tiempo débil.
- CICLO: de cuántas notas es el ciclo métrico, de 2 a 16.
- ROT: la velocidad con que el tiempo fuerte cambia de lugar.
- HER: la cola que un acento deja en la nota siguiente.
- SUPR: tras un acento fuerte, el siguiente cuesta más.
- PROF: la profundidad con que el énfasis llega a las salidas.
Entradas: GATE marca cada nota, PTCH recibe la altura, ACC el acento de fuera, RST reinicia el ciclo. Salidas: ENF es el énfasis, VCA el nivel, FC el brillo para un filtro, DEC la duración para una envolvente, SWP un barrido que sube con acentos seguidos, GT un gate solo en las notas acentuadas, NIV el énfasis en cuatro escalones (0, un tercio, dos tercios, 1).

## Experimente
1. Toque una línea de SEQUENCE a través de un QUANTIZER en un OSC. Conecte la altura y el gate del QUANTIZER a PTCH y GATE.
2. Conecte la salida VCA al CV1 del VCA de la voz: la línea empieza a respirar como una frase.
3. Pase el OSC por un LADDER y conecte SWP a su FC: los acentos seguidos suben el barrido, como en un bajo ácido.
4. Ponga CICLO en 5 y ROT en 0,5: el tiempo fuerte cambia de lugar y las notas siguen siendo las mismas.
