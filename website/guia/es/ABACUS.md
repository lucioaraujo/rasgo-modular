## O que é
ABACUS trata las señales como números. Hace cuentas entre dos entradas, cuenta pulsos y convierte la cuenta en ritmos y escaleras, y corta o refleja la parte negativa de una señal. Incluso sin nada en las entradas A y B, su contador interno genera patrones que sirven de ritmo y de melodía.

## Como pensar nele
Contar es la manera más simple de crear un patrón. Un contador que va de 0 a 7 y vuelve a empezar, mirado bit a bit, produce ritmos sincopados sin ningún secuenciador: cada bit se enciende y se apaga a una velocidad distinta. La salida QNT convierte la cuenta en una escalera de valores, buena para enviar a un QUANTIZER. Y la salida RCT, el rectificador, es útil por sí sola siempre que haga falta la mitad positiva de una señal o su valor absoluto.

## Controles
- OP: la cuenta entre A y B que sale por MTH: suma, resta, multiplicación, resto de la división, o cuatro operaciones bit a bit.
- MOD: hasta cuánto cuenta el contador antes de volver a empezar, de 2 a 32.
- STEP: en cuántos escalones se divide la salida QNT.
- RNG: el tamaño de la ventana de valores que usan las salidas MTH, QNT y RCT.
- RECT: el modo del rectificador: solo la parte positiva, solo la negativa, las dos reflejadas hacia arriba, o solo el signo, más o menos.
- CNT: cuánto avanza el contador con cada pulso. Negativo cuenta hacia atrás.
- PAT: qué bit del contador se vuelve la salida P1. Los bits bajos cambian rápido; los altos, despacio.
- SLEW: suaviza las salidas MTH y QNT.
- RATE: el reloj interno, usado cuando no llega nada a CLK.
Entradas: A y B, los números de la cuenta; CLK, el pulso que hace contar; RST, para poner a cero. Salidas: MTH, la cuenta; QNT, la escalera; RCT, el rectificador; P1 y P2, dos ritmos sacados del contador; CRY, un pulso cada vez que la cuenta vuelve a empezar.

## Experimente
1. Conecte la salida CLK del CLOCK a la entrada CLK de ABACUS, con MOD en 8.
2. Conecte P1, P2 y CRY a las entradas GATE de tres DRUM conectados al MIXER. Oye un ritmo sincopado que se cierra cada ocho pulsos.
3. Gire PAT y escuche P1 volverse más rápido o más lento.
4. Conecte QNT a la entrada CV de un QUANTIZER, y PTCH a 1V/O de un OSC conectado al MIXER. La cuenta se vuelve una melodía en escalera.
