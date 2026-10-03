## O que é
MULT toma una señal de control y la distribuye a cuatro salidas, cada una con su propio ajuste de tamaño y desplazamiento. Sin nada en la entrada, se vuelve un banco de cuatro valores fijos que usted ajusta a mano.

## Como pensar nele
En Rasgo Modular, una salida ya puede ir a varios destinos con cables comunes. MULT existe para cuando cada destino necesita una dosis distinta: la misma envolvente abriendo un filtro del todo, bajando un poco el volumen de otra voz, rozando un efecto. Una entrada, cuatro versiones ajustadas. Con DUAL encendido, se divide en dos distribuidores de dos salidas cada uno.

## Controles
- DUAL: apagado, IN va a las cuatro salidas; encendido, IN va a O1 y O2, e IN2 va a O3 y O4.
- SCL1, SCL2, SCL3, SCL4: el tamaño de la señal en cada salida. En 1 pasa igual; por debajo de 1, se achica; por encima, se agranda; negativo, se invierte. En cero solo queda el desplazamiento.
- OFF1, OFF2, OFF3, OFF4: un valor que se suma a cada salida. Sin nada en la entrada, es el propio valor de la salida.
- SLEW: suaviza las cuatro salidas juntas.
Entradas: IN e IN2. Salidas: O1 a O4.

## Experimente
1. Arme dos voces, cada una con su FILTER, pasando por el MIXER.
2. Conecte la salida ENV de un ENVELOPE, tocado por el CLOCK, a IN del MULT. Conecte O1 a FC del primer FILTER y O2 a FC del segundo.
3. Deje SCL1 en 1 y lleve SCL2 a un valor negativo. Con cada nota, un filtro se abre y el otro se cierra.
