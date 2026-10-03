## O que é
O BOXCAR mede um sinal em janelas curtas e tira a média de muitas medidas seguidas. A técnica vem de instrumentos de laboratório, usados para encontrar um sinal fraco escondido no ruído. No patch, ele serve de seguidor de volume muito estável, de reconstrutor de formas de onda e de oscilador feito daquilo que ele mediu.

## Como pensar nele
O SH captura um instante; o BOXCAR captura uma fatia de tempo e faz a média. A cada pulso em TRIG, abre uma janela num ponto do ciclo, mede, e soma essa medida às anteriores. O que se repete igual a cada ciclo vai se firmando, e o que é acaso se cancela. MODE escolhe o que fazer com o resultado: entregar só a média, como um sinal de controle que acompanha o som devagar; tocar de volta o ciclo reconstruído; ou reler esse ciclo sozinho, como um oscilador. Ele tem ainda uma saída à parte, GEIG, com pulsos irregulares como os de um contador Geiger.

## Controles
- DLY: em que ponto do ciclo a janela abre.
- APER: a largura da janela. Estreita, mede quase um instante; larga, faz a média de um trecho.
- AVG: quantas medidas entram na média, de 1 a 64. Mais medidas, menos ruído e resposta mais lenta.
- SCAN: faz a janela percorrer o ciclo sozinha, num sentido ou no outro. Em zero, fica parada.
- MODE: seguidor, reconstrução ou oscilador.
- RATE: o relógio interno, quando nada chega em TRIG, e a velocidade de releitura no modo oscilador.
- THRSH: o nível que o sinal precisa cruzar para disparar uma medida, quando nada chega em TRIG.
- GEI: a densidade dos pulsos na saída GEIG. Em zero, ela fica muda.
- BLEND: a mistura entre o sinal original e o processado.
Entradas: IN, o sinal a medir; TRIG, o pulso de cada medida; SWP, para mover a janela; THR, para modular o limiar. Saídas: OUT e GEIG.

## Experimente
1. Ligue GEIG na entrada GATE de um DRUM ligado ao MIXER e suba GEI. Você ouve golpes em momentos imprevisíveis.
2. Agora ligue uma voz que toque notas, como um OSC passando por um ENVELOPE, na entrada IN, com MODE na primeira posição e APER larga.
3. Ligue OUT na entrada CV1 de um VCA que controle outro som. O segundo som passa a seguir o volume da voz, sem tremer.
