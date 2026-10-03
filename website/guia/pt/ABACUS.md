## O que é
O ABACUS trata sinais como números. Ele faz contas entre duas entradas, conta pulsos e transforma a contagem em ritmos e escadas, e corta ou espelha a parte negativa de um sinal. Mesmo sem nada nas entradas A e B, o contador interno gera padrões que servem de ritmo e de melodia.

## Como pensar nele
Contar é a forma mais simples de criar padrão. Um contador que vai de 0 a 7 e recomeça, olhado bit a bit, produz ritmos sincopados sem nenhum sequenciador: cada bit liga e desliga numa velocidade diferente. A saída QNT transforma a contagem numa escada de valores, boa para mandar a um QUANTIZER. E a saída RCT, o retificador, é útil sozinha sempre que se precisa da metade positiva de um sinal ou de seu valor absoluto.

## Controles
- OP: a conta entre A e B que sai em MTH: soma, subtração, multiplicação, resto da divisão, ou quatro operações bit a bit.
- MOD: até quanto o contador conta antes de recomeçar, de 2 a 32.
- STEP: em quantos degraus a saída QNT é dividida.
- RNG: o tamanho da janela de valores usada pelas saídas MTH, QNT e RCT.
- RECT: o modo do retificador: só a parte positiva, só a negativa, as duas espelhadas para cima, ou apenas o sinal, mais ou menos.
- CNT: quanto o contador anda a cada pulso. Negativo conta para trás.
- PAT: qual bit do contador vira a saída P1. Bits baixos trocam rápido; bits altos, devagar.
- SLEW: suaviza as saídas MTH e QNT.
- RATE: o relógio interno, usado quando nada chega em CLK.
Entradas: A e B, os números da conta; CLK, o pulso que faz contar; RST, para zerar. Saídas: MTH, a conta; QNT, a escada; RCT, o retificador; P1 e P2, dois ritmos tirados do contador; CRY, um pulso cada vez que a contagem recomeça.

## Experimente
1. Ligue a saída CLK do CLOCK na entrada CLK do ABACUS, com MOD em 8.
2. Ligue P1, P2 e CRY nas entradas GATE de três DRUM ligados ao MIXER. Você ouve um ritmo sincopado que se fecha a cada oito pulsos.
3. Gire PAT e ouça P1 ficar mais rápido ou mais lento.
4. Ligue QNT na entrada CV de um QUANTIZER, e PTCH em 1V/O de um OSC ligado ao MIXER. A contagem vira uma melodia em escada.
