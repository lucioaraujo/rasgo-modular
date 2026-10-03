## O que é
DRUM es una voz de percusión lista para usar: con cada pulso en la entrada GATE, toca un golpe. Con pocas perillas va del bombo a la caja, al tom y al hi-hat, y cambia de carácter entre el sonido de las cajas de ritmos antiguas y un sonido más acústico.

## Como pensar nele
Armar un bombo desde cero exige varios módulos; DRUM entrega uno listo. Necesita que alguien le diga cuándo tocar: un CLOCK o, mejor, un TRIGSEQ, que genera patrones rítmicos en cuatro líneas. Cuatro DRUM, cada uno en una línea del TRIGSEQ, forman una batería. Y como TONE acepta una altura por la entrada PIT, se puede tocar una línea de toms.

## Controles
- TONE: la altura del cuerpo del golpe, de 20 a 1000 Hz. Grave es bombo; medio, tom o caja; agudo, algo como una clave.
- BEND: cuánto cae la altura justo después del golpe. Alto da el peso del bombo; cero mantiene el golpe en una sola altura.
- DECAY: la duración, de un clic corto a un grave que se sostiene.
- SNAP: la cantidad de chasquido en el ataque, que da cuerpo a la caja y al hi-hat.
- MAP: el carácter, pasando por timbres de cajas de ritmos hasta un sonido acústico.
- DRIVE: satura la salida y vuelve el golpe más agresivo.
- ROLL: hace que el módulo se dispare solo, de un redoble lento a un zumbido. En cero solo toca cuando recibe un pulso.
- DRIFT: varía levemente cada golpe, para que la repetición suene tocada y no programada.
Entradas: GATE, el pulso que dispara el golpe; ACC, para acentuar; PIT, para la altura.

## Experimente
1. Conecte OUT a una entrada del MIXER, y la salida CLK del CLOCK a la entrada GATE. Oye un bombo marcando el tiempo.
2. Suba TONE y SNAP, baje DECAY: el bombo se vuelve una caja seca.
3. Suba ROLL sin nada en GATE: el módulo toca solo, en un redoble.
4. Conecte el CLOCK a un TRIGSEQ y sus salidas T1 y T2 a dos DRUM ajustados de forma distinta. Tiene una pequeña batería.
