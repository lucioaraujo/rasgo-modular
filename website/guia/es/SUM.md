## O que é
SUM junta señales. Cuatro entradas, cada una con su atenuversor, se vuelven una sola salida: la suma, la suma invertida y la media.

## Como pensar nele
En Rasgo Modular cada entrada recibe un solo cable. Para que un filtro se mueva con dos movimientos a la vez, los dos pasan antes por SUM. MULT copia una señal a varios lugares; SUM junta varias señales en un lugar.

## Controles
- A1 a A4: cuánto de cada entrada entra en la suma, de −2 a +2. En el centro la entrada queda fuera; por debajo del centro entra invertida.
- OFS: un valor fijo sumado al resultado.
- SAT: encendido, la suma satura suavemente y nunca pasa de ±1, bueno para sumar audio.
Entradas: IN1 a IN4. Salidas: SUM, la suma; INV, la suma invertida; AVG, la media de las entradas conectadas.

## Experimente
1. Conecte el BI de un FUNCTION a IN1 y una salida de un DRIFT a IN2. Conecte SUM al cutoff de un FILTER: el filtro se mueve con los dos.
2. Gire A2 por debajo del centro: el DRIFT ahora empuja el filtro hacia el otro lado.
3. Conecte dos osciladores a IN1 e IN2, encienda SAT y mande SUM al MIXER: los dos suenan juntos sin saturar.
