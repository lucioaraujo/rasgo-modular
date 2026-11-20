## O que é
RUNGLER tiene dos osciladores y un registro de 8 bits. En cada vuelta del oscilador A, el registro avanza una casilla y guarda el estado del oscilador B en ese instante. Los tres últimos bits se vuelven una tensión de 8 escalones, que sale en RUNG y vuelve a modular los dos osciladores.

## Como pensar nele
Es el corazón del Benjolin, de Rob Hordijk. El bucle se cierra: con poca modulación, un patrón que se repite; con mucha, el circuito tropieza entre trabarse en un ritmo y volverse ruido. LOOP es la llave de Hordijk: al máximo, el bit que sale del final vuelve al principio y el patrón de 8 pasos se repite para siempre; en el medio, de vez en cuando entra un bit nuevo y el patrón se transforma.

## Controles
- FRQA: la frecuencia del oscilador A, el reloj.
- FRQB: la frecuencia del oscilador B.
- LOOP: la llave del bucle.
- MODA: cuánto modula el rungler a A.
- MODB: cuánto modula el rungler a B.
- LEVEL: el nivel.
Entradas: PTA y PTB suman alturas en voltios por octava, LOOP se suma a la llave del bucle. Salidas: TRIA y TRIB son los triángulos, PWM el comparador, RUNG los 8 escalones.

## Experimente
1. Ponga FRQA en 8 Hz y conecte RUNG a un QUANTIZER y a un OSC: una melodía que piensa sola.
2. Conecte PWM a un FILTER y a un VCA con las dos frecuencias en audio: el sonido ronco del Benjolin.
3. Suba LOOP hasta el final: el patrón se fija. Bájelo un poco: empieza a cambiar.
4. Suba MODA y MODB: el patrón tropieza hasta el caos.
