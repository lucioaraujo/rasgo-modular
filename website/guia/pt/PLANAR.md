## O que é
O PLANAR mistura quatro sons postos nos cantos de um quadrado. Um ponto se move dentro do quadrado, e o som que sai é a mistura dos quatro, pesada pela distância do ponto a cada canto. Movendo o ponto devagar, o timbre se transforma de um som em outro. A técnica se chama síntese vetorial.

## Como pensar nele
Pense num joystick: X move o ponto da esquerda para a direita, Y de baixo para cima. Você pode mover à mão, com duas ondas lentas, ou com um DRIFT, e o ponto desenha figuras. A posição também sai pelas saídas X' e Y', o que permite que o mesmo movimento conduza outros controles do patch. Com um pulso longo na entrada GST, o PLANAR grava o trajeto que você fizer nos knobs X e Y e depois o repete sem parar.

## Controles
- X, Y: a posição do ponto no quadrado. Os sinais nas entradas de mesmo nome se somam a estes valores.
- CURVE: o jeito de misturar. Num extremo, a mistura é linear, boa para sinais de controle; no outro, mantém o volume constante, e o som não afunda quando o ponto está no meio.
- SMTH: faz o ponto escorregar até a posição nova, em vez de saltar.
- RATE: a velocidade da repetição do gesto gravado e do passeio de DRIFT.
- DRIFT: faz o ponto passear sozinho pelo quadrado.
Entradas: A, B, C e D, os quatro sons; X e Y, para mover o ponto; GST, para gravar e repetir um gesto. Saídas: OUT, a mistura; X' e Y', a posição do ponto.

## Experimente
1. Ligue quatro fontes diferentes em A, B, C e D: por exemplo SAW de um OSC, OUT de um WAVETABLE, OUT do CHORD e PNK de um NOISE. Ligue OUT do PLANAR no MIXER.
2. Mova X e Y à mão e ouça o som passar de um canto ao outro.
3. Ligue BI de dois FUNCTION lentos, em velocidades diferentes, nas entradas X e Y. O ponto desenha uma figura e o timbre não para de mudar.
4. Ligue Y' na entrada FC de um FILTER no caminho do som: o filtro acompanha o movimento.
