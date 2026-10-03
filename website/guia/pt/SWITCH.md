## O que é
O SWITCH é uma chave seletora comandada pelo patch. Ele pode escolher uma entre quatro fontes e mandá-la para uma saída, ou pegar uma fonte só e mandá-la para uma entre quatro saídas. A troca acontece a cada pulso, em ordem ou ao acaso, ou segue um sinal de controle.

## Como pensar nele
Num patch que se transforma, mudar de material é tão importante quanto mudar de nota: a mesma melodia tocada ora por um oscilador, ora por uma corda; o filtro alimentado ora pelo ruído, ora pelo acorde. Sem o SWITCH, isso exige trocar cabos à mão. Com DEMUX desligado, ele escolhe entre A, B, C e D e entrega em OA. Ligado, pega o que chega em A e o envia a uma das quatro saídas, OA, OB, OC ou OD.

## Controles
- STEP: quantas posições a chave percorre, de 2 a 4.
- MODE: como a posição muda: para a frente, indo e voltando, ao acaso, ou só pelo sinal da entrada ADR.
- DEMUX: desligado, quatro entradas para uma saída; ligado, uma entrada para quatro saídas.
- GLID: uma transição gradual entre as posições, em que as duas fontes se misturam por um instante. Em zero, o corte é seco.
- SLEW: suaviza a troca para evitar estalos. Convém deixar sempre um pouco.
Entradas: A, B, C e D, as fontes; CLK, o pulso que avança; RST, para voltar à primeira posição; ADR, para escolher a posição com um sinal. Saídas: OA, OB, OC e OD; STP, a posição atual como sinal de controle.

## Experimente
1. Ligue a saída SAW de um OSC em A, e OUT de um MATTER, tocado pelo CLOCK, em B. Ligue OA numa entrada do MIXER.
2. Ponha STEP em 2 e ligue a saída DIV de um LOGIC em CLK, com a saída CLK do CLOCK na entrada CLK do LOGIC e DIV em 4.
3. A cada quatro pulsos, o som troca de fonte. Suba GLID para que as trocas se fundam.
