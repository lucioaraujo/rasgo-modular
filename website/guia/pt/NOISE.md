## O que é
O NOISE produz ruído, o som de todas as frequências misturadas ao acaso, em várias cores ao mesmo tempo, cada uma numa saída: do branco, chiado e brilhante, ao marrom, grave como o mar. Ele também produz valores aleatórios lentos, que servem para fazer outros módulos mudarem sozinhos.

## Como pensar nele
O NOISE tem dois usos bem diferentes. Como som, é a matéria do vento, da chuva, dos pratos e do estalo de uma caixa. Como acaso, é uma fonte de variação para patches generativos: a saída S&H sorteia um valor novo a cada pulso e o segura até o próximo, e a SMTH desliza de um valor a outro. Ligue uma delas na altura de um oscilador, passando por um quantizador, e você tem uma melodia que nunca se repete igual.

## Controles
- RATE: o ritmo dos sorteios das saídas S&H e SMTH, de muito lento a 2000 por segundo. Se a entrada TRIG estiver ligada, ela manda no lugar deste knob.
- SLEW: quanto tempo a saída SMTH leva para chegar a cada valor novo. Em zero, ela salta; no máximo, desliza devagar.
- SPRD: o tipo de sorteio. Em zero, qualquer valor é igualmente provável; no máximo, os valores perto do meio aparecem mais, e as mudanças ficam mais suaves.
- POIS: troca o ritmo regular dos sorteios por tempos irregulares, ao acaso, mantendo a mesma média.
Saídas de som: WHT (branco), PNK (rosa), BRN (marrom), BLU (azul), VLT (violeta) e BIT (ruído digital). Saídas de acaso: S&H e SMTH. Entradas: TRIG, para disparar os sorteios, e IN.

## Experimente
1. Ligue PNK numa entrada do MIXER. Você ouve um chiado suave, como chuva.
2. Troque para BRN, mais grave, e depois para WHT, mais brilhante.
3. Ligue S&H na entrada CV de um QUANTIZER, e a saída PTCH dele na entrada 1V/O de um OSC que esteja soando. A nota passa a saltar ao acaso, sempre dentro da escala.
4. Ligue a saída CLK do CLOCK na entrada TRIG: os saltos passam a acontecer no ritmo do relógio.
