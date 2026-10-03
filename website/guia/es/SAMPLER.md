## O que é
SAMPLER graba un tramo de lo que entra en él, hasta unos ocho segundos, lo divide en rebanadas y las toca cuando recibe pulsos. Es el gesto de los samplers del hip-hop y de la música electrónica: tomar una grabación, trocearla y tocarla en otro orden, más rápida, más lenta o al revés.

## Como pensar nele
Mientras la entrada REC está encendida, graba lo que llega a IN. Cuando REC se apaga, el tramo queda guardado. A partir de ahí, cada pulso en TRIG toca una rebanada, y la entrada POS elige cuál. Con un TRIGSEQ disparando y una SEQUENCE eligiendo las rebanadas, la grabación se recombina en un ritmo nuevo. Conectando la salida del propio patch a la entrada, el instrumento empieza a reutilizar lo que acaba de tocar.

## Controles
- START: dónde, dentro de la rebanada, empieza la lectura.
- SPEED: la velocidad de lectura, de un cuarto a cuatro veces la original. Negativo toca al revés. Normalmente, más rápido también suena más agudo.
- SLICE: en cuántas rebanadas iguales se divide la grabación, de 1 a 16.
- REPIT: apagado, velocidad y altura van juntas, como en una cinta; encendido, la altura viene de la entrada PIT y la rebanada conserva su duración.
- WEAR: el desgaste en cada disparo: un inicio impreciso, pérdida de definición, sonido granulado.
- LOOP: apagado, cada rebanada suena una vez; encendido, se repite.
Entradas: TRIG, el pulso que toca; IN, el sonido a grabar; REC, la señal que graba mientras está encendida; POS, para elegir la rebanada; PIT, para afinar. Salida: OUT.

## Experimente
1. Conecte una voz que toque una frase, por ejemplo una SEQUENCE tocando un OSC, a la entrada IN.
2. Para grabar, conecte la salida DIV de un LOGIC a REC, con la salida CLK del CLOCK en CLK del LOGIC y DIV en 16. Conecte OUT del SAMPLER al MIXER.
3. Ponga SLICE en 8, conecte T1 de un TRIGSEQ a TRIG y la salida CV de un TURING a POS. La frase grabada vuelve troceada y reordenada.
4. Lleve SPEED a la izquierda del centro y escuche las rebanadas al revés.
