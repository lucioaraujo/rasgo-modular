## O que é
PLL es un oscilador que sabe seguir a otro. Solo funciona como cualquier oscilador, con su propia altura y una forma de onda que usted elige. Cuando recibe una segunda señal en la entrada REF, ajusta poco a poco su velocidad hasta andar junto a esa señal, como un músico que acompasa su paso con el de otro.

## Como pensar nele
OSC también puede engancharse a otro oscilador, pero de golpe (eso es la sincronía). PLL lo hace poco a poco, y ese camino hasta engancharse se oye: un deslizamiento de altura. Con RATIO se engancha una octava arriba, una abajo o en divisiones más profundas de la referencia. La salida LOCK indica cuánto está enganchado, y sirve para que otras cosas reaccionen a eso.

## Controles
- FREQ: la altura cuando no hay referencia, y el punto de partida cuando la hay.
- FINE: afinación fina, hasta un semitono hacia cada lado.
- SHAPE: la forma de onda, pasando de senoidal a triangular, diente de sierra y cuadrada.
- RATIO: en qué relación con la referencia se engancha. En 1, la misma nota; en 2, una octava arriba; cerca del mínimo, muy abajo.
- LOCK: la fuerza con que persigue la referencia. Bajo, se desliza despacio hasta llegar; alto, se engancha enseguida pero se vuelve más nervioso.
- FM: cuánto mueve la altura la señal de la entrada FM.
- FBK: cuánto de su propia señal vuelve al circuito, añadiendo textura sin desafinar.
- FTYP: elige entre seis tipos de realimentación, cada uno con un color distinto.
Entradas: 1V/O para la altura, FM y REF, la referencia a seguir. Salidas: OUT, RING (el PLL multiplicado por la referencia) y LOCK, una señal de control que indica cuánto está enganchado.

## Experimente
1. Conecte OUT a una entrada del MIXER. Sin nada en REF es un oscilador común; gire SHAPE para oír las formas.
2. Conecte la salida SAW de un OSC a la entrada REF. El PLL busca la altura del OSC y se engancha a ella.
3. Baje LOCK. Ahora tarda en llegar, y se oye la altura deslizándose.
4. Ponga RATIO en 2. Se engancha una octava por encima del OSC.
