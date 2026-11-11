## O que é
CELL es un autómata celular: una fila de casillas encendidas o apagadas que se reescribe en cada pulso. Cada casilla se mira a sí misma y a sus dos vecinas, y una regla (un número de 0 a 255) decide si sigue encendida. El módulo lee ese dibujo con un cabezal: la casilla bajo él da el gate, y las tres siguientes eligen la nota.

## Como pensar nele
Es una partitura que se escribe sola con una ley muy simple. Algunas reglas dan dibujos que nunca se repiten (30, 110), otras dan triángulos (90) o cosas que se desplazan (184). Usted no compone las notas, compone la ley. MUT añade un pequeño azar, para que la línea no muera ni se trabe.

## Controles
- RATE: el reloj interno, cuando no hay nada conectado en CLK.
- RULE: la regla, de 0 a 255. Empiece por 30, 90, 110 y 184.
- CELLS: cuántas casillas tiene la fila, de 8 a 24.
- HEAD: dónde está el cabezal de lectura.
- RANGE: el alcance de las notas, hasta 2 octavas.
- MUT: la probabilidad de que una casilla cambie al azar.
- GATE: la duración de cada nota.
Entradas: CLK avanza una generación, RST reinicia la fila, RULE cambia la regla por CV. Salidas: PTCH es la nota, GT el gate, DENS la proporción de casillas encendidas, EOS un pulso cuando la fila renace.

## Experimente
1. Conecte GT a un DRUM y el DRUM al MIXER. Con RULE en 30, escuche un ritmo que nunca se repite igual.
2. Conecte PTCH a un OSC y GT a un ENVELOPE que controla un VCA después del OSC. Gire RULE despacio.
3. Conecte DENS al CUT de un FILTER: cuanta más vida en la fila, más brillo.
4. Conecte EOS al ADV del HARMONY: el acorde cambia cuando la fila renace.
