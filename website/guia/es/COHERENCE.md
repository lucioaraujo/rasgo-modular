## O que é
COHERENCE mide cuán previsible es una melodía y rompe cuando se vuelve demasiado ordenada. Guarda las últimas notas y calcula su entropía de Shannon: una nota repetida da orden 1, las doce notas por igual dan 0. Cuando el orden se mantiene por encima del umbral el tiempo suficiente, dispara un pulso de ruptura.

## Como pensar nele
Se suele pensar que la ruptura nace del caos. Aquí es al revés: el orden extremo también rompe. La presión sube con cada nota previsible; tras PAC notas seguidas, rompe. La ruptura deja cicatriz: la ventana olvida su mitad más antigua y, durante CIC notas, la presión no vuelve a subir. El módulo no suena: decide cuándo, y el cable decide qué.

## Controles
- JAN: la ventana, cuántas notas recuerda, de 4 a 32.
- LIM: el umbral de orden por encima del cual sube la presión.
- PAC: la paciencia, cuántas notas seguidas por encima del umbral hasta romper.
- CIC: la cicatriz, cuántas notas tras la ruptura sin nueva presión.
Entradas: PTCH recibe la altura, GATE marca cada nota (sin él, cuenta cada cambio de altura), RST lo olvida todo. Salidas: ORD es el orden medido, PRES la presión acumulada, COL el pulso de ruptura, SURP la sorpresa de cada nota.

## Experimente
1. Conecte la altura y el gate de un QUANTIZER que toca un SEQUENCE corto a PTCH y GATE.
2. Conecte COL al RST de un TURING que alimenta otra voz: cuando la primera línea se repite demasiado, la otra se rehace.
3. Conecte PRES al corte de un FILTER: el timbre se abre mientras crece la tensión y se cierra en la ruptura.
4. Conecte SURP al nivel de un VCA: la nota rara suena más fuerte.
