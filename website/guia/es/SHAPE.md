## O que é
SHAPE enriquece un sonido deformándolo de manera controlada. Toma una onda simple, como una senoidal, y la pliega, la satura o la multiplica por otra, creando armónicos nuevos. La técnica está asociada a los sintetizadores de la costa oeste de Estados Unidos, que preferían añadir armónicos antes que quitarlos con filtros.

## Como pensar nele
Un filtro quita armónicos a un sonido rico; SHAPE hace el camino inverso, a partir de uno pobre. Su gesto principal es FOLD: cuando la señal pasa de un límite, se pliega hacia dentro, y cada pliegue añade brillo. Con una envolvente en la entrada FCV, la nota abre el timbre en el ataque y lo cierra después. RING multiplica el sonido por lo que entra en MOD, lo que da timbres de campana.

## Controles
- RING: mezcla el sonido con su producto por la señal de la entrada MOD. Al máximo solo se oye el producto, metálico.
- FOLD: la cantidad de pliegue. Cuanto más, más armónicos agudos.
- SYM: desplaza el centro del pliegue y añade armónicos pares, lo que vuelve el sonido más nasal.
- WRAP: cambia parte del pliegue por un corte seco que reaparece del otro lado, más brutal.
- SAT: redondea los picos después del pliegue, suavizando el resultado.
- LVL: el volumen de salida.
- DRIFT: una oscilación lenta en la cantidad de pliegue.
Entradas: IN, el sonido; MOD, la segunda señal para RING; FCV, para modular el pliegue.

## Experimente
1. Conecte la salida SIN de un OSC a la entrada IN, y OUT a una entrada del MIXER. Oye una senoidal limpia.
2. Suba FOLD despacio y escuche la senoidal ganar brillo, pliegue a pliegue.
3. Lleve SYM hacia un lado: el sonido se vuelve más nasal.
4. Vuelva FOLD a cero, conecte la salida SAW de otro OSC a MOD y suba RING. Afine los dos osciladores en notas alejadas y el sonido se vuelve campana.
