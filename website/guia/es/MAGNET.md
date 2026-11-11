## O que é
MAGNET es una rejilla de 8 por 8 pequeños imanes, cada uno apuntando hacia arriba o hacia abajo. Cada imán quiere alinearse con sus vecinos, y la temperatura los agita. Un cabezal recorre la rejilla, una casilla por pulso, y el imán bajo él da el gate.

## Como pensar nele
Es el modelo de Ising, de la física de los materiales. Por debajo de la temperatura crítica (cerca de 2,27), se forman dominios grandes y el ritmo queda estable. Por encima, gana el ruido. Cerca de ella, aparecen y desaparecen dominios de todos los tamaños: es el borde del caos, y es ahí donde el módulo suena más vivo. Un COUPL negativo hace que los imanes se opongan a sus vecinos, como un tablero de ajedrez.

## Controles
- TEMP: la temperatura; la transición está cerca de 2,27.
- COUPL: el acoplamiento. Positivo, los imanes se alinean; negativo, se oponen.
- FIELD: un imán externo que empuja toda la rejilla hacia un lado.
- SWEEP: cuántos intentos de girar imanes hay en cada pulso.
- RATE: el reloj interno, cuando no hay nada conectado en CLK.
- GATE: la duración de cada gate.
Entradas: CLK avanza un pulso, QNCH calienta todo de golpe y la rejilla vuelve a organizarse, TEMP y FLD mueven temperatura y campo por CV. Salidas: GT es el gate del cabezal, MAG la magnetización, ENER la energía de la rejilla, FLIP cuánto de la rejilla giró en este pulso.

## Experimente
1. Conecte GT a un DRUM. Con TEMP en 1, escuche un ritmo casi quieto; en 4, un ritmo de ruido.
2. Ponga TEMP cerca de 2,3: aparecen y desaparecen dominios, y el ritmo respira.
3. Conecte un disparo poco frecuente a QNCH: la rejilla se deshace y se reorganiza.
4. Conecte MAG al corte de un FILTER: el timbre sigue hacia qué lado se inclina la rejilla.
