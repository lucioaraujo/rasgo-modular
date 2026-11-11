## O que é
EPICYCLE dibuja con círculos: hasta ocho, cada uno girando en el borde del anterior. La punta del último traza una figura, y sus coordenadas salen como señales. Es una serie de Fourier dibujada, la misma idea de los epiciclos de Ptolomeo.

## Como pensar nele
La misma máquina sirve en dos escalas. Con pocos círculos y RATE lento, es gesto y ritmo: una figura que vuelve. Con muchos círculos y la altura de una nota en PTCH, gira a la velocidad del sonido y se vuelve timbre. MODO elige la familia de figuras, y DESV desafina las velocidades para que la figura nunca se cierre.

## Controles
- RATE: la velocidad del primer círculo, de 0,01 Hz a 2 kHz.
- CIRC: cuántos círculos, de 1 a 8.
- MODO: cuadrada (velocidades impares), sierra (todas las velocidades), radios de Fibonacci o velocidades de Fibonacci.
- DESV: la desviación de las velocidades; por encima de 0, la figura nunca se cierra.
- FASE: la fase de partida de los círculos.
Entradas: PTCH recibe la altura a un voltio por octava, para el modo de timbre; RST reinicia la figura. Salidas: X e Y son las coordenadas de la punta, R la distancia al centro, ANG el ángulo, GT un pulso cuando Y cruza el cero subiendo.

## Experimente
1. Con RATE en 0,2 Hz y CIRC en 3, conecte X al corte de un FILTER e Y al pan de un canal del MIXER: el sonido dibuja una figura en el espacio.
2. Conecte GT a un ENVELOPE: la figura se vuelve ritmo.
3. Conecte X al MIXER, ponga CIRC en 8 y conecte la altura de un QUANTIZER a PTCH: ahora la figura es un timbre.
4. Cambie MODO y suba DESV despacio: el timbre se abre y no vuelve al mismo lugar.
