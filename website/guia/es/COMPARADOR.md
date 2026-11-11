## O que é
COMPARADOR mezcla dos señales con polaridad y decide cuándo la mezcla pasa de un umbral. Los mandos A y B tienen el cero en el centro: hacia la derecha la entrada suma, hacia la izquierda entra invertida. La salida COMP es un gate que queda alto mientras la mezcla está por encima del umbral.

## Como pensar nele
Es una balanza. Con A a la derecha y B a la izquierda, la mezcla es la diferencia entre las dos señales; con la misma señal en las dos, se anula. La decisión viene después: el gate se abre cuando la balanza se inclina hacia un lado. La histéresis impide que tiemble cuando la señal tiene ruido cerca del umbral.

## Controles
- A, B: cuánto entra cada entrada, de −1 (invertida) a 1.
- OFS: desplaza toda la mezcla hacia arriba o hacia abajo.
- LIM: el umbral de la decisión.
- HIST: la histéresis, un margen alrededor del umbral.
Entradas: A y B reciben las señales, LIM mueve el umbral por CV. Salidas: MIX es la mezcla, INV la mezcla al revés, COMP el gate de la decisión, CRUZ un pulso en cada cambio de lado.

## Experimente
1. Conecte dos FUNCTION lentos, a velocidades distintas, a A y B. Ponga A a la derecha y B a la izquierda.
2. Conecte COMP al gate de un ENVELOPE y CRUZ a un DRUM: dos movimientos lentos que se cruzan se vuelven ritmo.
3. Conecte la misma señal a A y B con A en 1 y B en −1: la mezcla desaparece.
4. Conecte el LVL de un SCOPE a A y ponga LIM en el medio: cuando quien toca pasa de cierto volumen, entra otra voz.
