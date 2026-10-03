## O que é
LOGIC combina y transforma ritmos. Divide un pulso para crear ritmos más lentos, compara dos ritmos para generar un tercero y alterna un estado con cada pulso. Son operaciones simples, de las que se usan en los circuitos digitales, aplicadas al tiempo musical.

## Como pensar nele
Un ritmo interesante rara vez es un pulso recto. Suele ser la relación entre dos: uno que toca a la mitad de velocidad, uno que solo toca cuando otro también toca, uno que toca cuando uno u otro toca pero nunca los dos juntos. LOGIC se coloca entre las fuentes de pulsos, como CLOCK, TURING y SEQUENCE, y los módulos que ellas disparan. Sin nada en la entrada CLK, usa un reloj propio.

## Controles
- RATE: el reloj interno, usado cuando no llega nada a CLK.
- DIV: divide el pulso. En 2, la salida DIV pulsa una vez cada dos pulsos; en 3, una cada tres.
- MULT: añade pulsos intermedios dentro de cada período.
- GATE: cuánto tiempo queda encendido el pulso de la salida DIV.
- DELAY: retrasa el pulso de la salida DIV hasta 200 milisegundos.
Entradas: CLK, el pulso a dividir; A y B, los dos ritmos a comparar; RST, para poner a cero. Salidas: DIV, el pulso dividido; AND, cuando A y B están encendidos a la vez; OR, cuando lo está uno u otro; XOR, cuando lo está solo uno de ellos; FLIP, que cambia de estado con cada pulso en A.

## Experimente
1. Conecte la salida CLK del CLOCK a la entrada CLK de LOGIC y DIV a la entrada GATE de un DRUM conectado al MIXER. Cambie DIV para oír cómo el pulso se vuelve más lento.
2. Conecte EUC del CLOCK a A, y la salida PLS de un TURING a B.
3. Lleve la percusión a la salida XOR y luego a AND. Escuche cómo cada combinación genera un ritmo distinto a partir de los mismos dos.
