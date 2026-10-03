## O que é
O MULT pega um sinal de controle e o distribui para quatro saídas, cada uma com seu ajuste de tamanho e de deslocamento. Sem nada na entrada, ele vira um banco de quatro valores fixos que você regula à mão.

## Como pensar nele
No Rasgo Modular, uma saída já pode ir para vários destinos com cabos comuns. O MULT existe para quando cada destino precisa de uma dose diferente: o mesmo envelope abrindo o filtro por inteiro, baixando um pouco o volume de outra voz, mexendo de leve num efeito. Uma entrada, quatro versões ajustadas. Com DUAL ligado, ele se divide em dois distribuidores de duas saídas cada.

## Controles
- DUAL: desligado, IN vai para as quatro saídas; ligado, IN vai para O1 e O2, e IN2 vai para O3 e O4.
- SCL1, SCL2, SCL3, SCL4: o tamanho do sinal em cada saída. Em 1 passa igual; abaixo de 1, diminui; acima, aumenta; negativo, inverte. Em zero, sobra só o deslocamento.
- OFF1, OFF2, OFF3, OFF4: um valor somado a cada saída. Sem nada na entrada, é o próprio valor da saída.
- SLEW: suaviza as quatro saídas juntas.
Entradas: IN e IN2. Saídas: O1 a O4.

## Experimente
1. Faça duas vozes, cada uma com seu FILTER, passando pelo MIXER.
2. Ligue a saída ENV de um ENVELOPE, tocado pelo CLOCK, em IN do MULT. Ligue O1 em FC do primeiro FILTER e O2 em FC do segundo.
3. Deixe SCL1 em 1 e leve SCL2 para um valor negativo. A cada nota, um filtro abre e o outro fecha.
