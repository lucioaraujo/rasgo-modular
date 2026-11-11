## O que é
O EPICYCLE desenha com círculos: até oito, cada um girando na borda do anterior. A ponta do último traça uma figura, e as coordenadas dela saem como sinais. É uma série de Fourier desenhada, a mesma ideia dos epiciclos de Ptolomeu.

## Como pensar nele
A mesma máquina serve a duas escalas. Com poucos círculos e RATE lento, ela é gesto e ritmo: uma figura que volta. Com muitos círculos e a altura de uma nota em PTCH, ela gira na velocidade do som e vira timbre. MODO escolhe a família de figuras, e DESV desafina as velocidades para que a figura nunca feche.

## Controles
- RATE: a velocidade do primeiro círculo, de 0,01 Hz a 2 kHz.
- CIRC: quantos círculos, de 1 a 8.
- MODO: quadrada (velocidades ímpares), serra (todas as velocidades), raios de Fibonacci ou velocidades de Fibonacci.
- DESV: o desvio das velocidades; acima de 0, a figura nunca se fecha.
- FASE: a fase de partida dos círculos.
Entradas: PTCH recebe a altura em 1 volt por oitava, para o modo de timbre; RST recomeça a figura. Saídas: X e Y são as coordenadas da ponta, R a distância ao centro, ANG o ângulo, GT um pulso quando Y cruza o zero subindo.

## Experimente
1. Com RATE em 0,2 Hz e CIRC em 3, ligue X no corte de um FILTER e Y no pan de um canal do MIXER: o som desenha uma figura no espaço.
2. Ligue GT num ENVELOPE: a figura vira ritmo.
3. Ligue X no MIXER, ponha CIRC em 8 e ligue a altura de um QUANTIZER em PTCH: agora a figura é um timbre.
4. Troque MODO e suba DESV devagar: o timbre se abre e não volta ao mesmo lugar.
