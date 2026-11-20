## O que é
INTERFERENCE es una pantalla iluminada por dos o tres fuentes de luz coherente. Donde las ondas llegan juntas, la luz se suma y la franja es clara; donde llegan opuestas, se cancela y la franja es oscura. Un punto recorre la pantalla, y cada franja clara da un pulso en FRNG, cada oscura en DARK.

## Como pensar nele
Es el experimento de Young, de 1801, la prueba de que la luz es onda. Con dos fuentes, las franjas son regulares, una tras otra. Con la tercera, las franjas se agrupan y se apagan en ciclos lentos, el moiré, que sale en MOIRE. Despacio es ritmo; con SPEED alto, INT se vuelve un sonido.

## Controles
- SPEED: cuántas franjas por segundo.
- RATIO: la diferencia de camino de la tercera fuente.
- THIRD: la fuerza de la tercera fuente.
- THR: el umbral de las franjas.
- COHER: la coherencia de la luz, del láser a la lámpara.
Entradas: SPD mueve la velocidad en voltios por octava, RATIO se suma a RATIO, RST vuelve al centro de la pantalla. Salidas: INT es la intensidad, FRNG y DARK los pulsos de las franjas, MOIRE la envolvente lenta.

## Experimente
1. Conecte FRNG a un DRUM con THIRD en 0: un pulso regular. Suba THIRD: los pulsos se agrupan y desaparecen en ciclos.
2. Conecte MOIRE al nivel de un VCA: el sonido respira con el moiré.
3. Baje COHER: el patrón nítido se vuelve niebla y el ritmo se deshace.
4. Ponga SPEED en 220 y conecte INT al MIXER: las franjas se vuelven timbre.
