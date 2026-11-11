## O que é
DYNAMICS es un compresor. Sujeta los picos para que el sonido quede más junto, y deja que un sonido baje a otro, como el pad que respira cuando entra el bombo. Aquí hace una tercera cosa: lo que oye y cuánto reduce salen como señales de control, para mover cualquier otro módulo.

## Como pensar nele
Piense en un técnico de sonido con la mano en el fader: cuando el sonido pasa del umbral, baja; cuando el sonido cae, devuelve el nivel. RATIO es la fuerza de la mano, ATK y REL su prisa. La entrada SIDE cambia lo que escucha: con un bombo ahí, es el bombo el que decide cuándo baja el resto. Dos desvíos Rasgo tocan el tiempo: BRTH hace que la vuelta siga el intervalo entre los golpes, y DRIFT varía ataque y vuelta en cada golpe.

## Controles
- THRES: el umbral, de −48 a 0 dB. Por encima, el sonido se baja.
- RATIO: cuánto baja. Con 4, cada 4 dB por encima del umbral se vuelven 1; con 20, se convierte en limitador.
- KNEE: de golpe (0) o poco a poco (hasta 12 dB).
- ATK, REL: el tiempo de bajar (0,1 a 200 ms) y el de volver (5 ms a 2 s).
- GAIN: devuelve el nivel perdido, hasta 24 dB.
- SIDE: el detector escucha la entrada IN (0) o la entrada SIDE (1).
- RMS: mide la energía media en lugar del pico; suena más suave.
- GATE: invierte la curva y lo convierte en una puerta de ruido, que cierra lo que queda por debajo del umbral.
- BRTH: la vuelta sigue el tempo de los golpes que oye.
- DRIFT: un azar sembrado en el ataque y la vuelta, golpe a golpe.
Entradas: IN recibe el sonido, SIDE lo que el detector debe oír, THR mueve el umbral. Salidas: OUT es el sonido comprimido, ENV el nivel que oye, GR cuánto reduce ahora.

## Experimente
1. Conecte un TRIGSEQ a un DRUM y el DRUM a la entrada SIDE. Conecte un CHORD a IN y OUT al MIXER.
2. Deje SIDE en 1, RATIO en 6, ATK corto y REL cerca de 200 ms: el acorde baja con cada bombo.
3. Suba BRTH: la vuelta empieza a respirar con el ritmo.
4. Conecte la salida ENV al CUT de un FILTER en otro sonido: el filtro se abre con la fuerza de lo que el compresor oye.
