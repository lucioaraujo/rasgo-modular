## O que é
O CONTROL ajusta sinais de controle antes que cheguem ao destino. Ele pode diminuir, aumentar ou inverter um sinal, deslocá-lo para cima ou para baixo, suavizar seus degraus e somar dois sinais. São operações discretas, mas aparecem em quase todo patch que vai além do básico.

## Como pensar nele
Às vezes uma modulação tem a forma certa e o tamanho errado, ou o sentido errado: um envelope que deveria fechar o filtro em vez de abri-lo, uma onda que deveria oscilar só acima de zero. O ganho do cabo diminui um sinal, mas não inverte nem desloca. O CONTROL faz tudo isso num painel, em dois canais iguais e uma saída que junta os dois. Com um som de verdade na entrada, ele também acompanha o volume desse som e o transforma em controle, o que se chama seguidor de envelope.

## Controles
- SCALE: o tamanho do sinal, em cada canal. Em 1 passa igual; em 0,5, pela metade; negativo, invertido. Em zero, sobra só o deslocamento de OFF, e o canal vira um controle manual.
- OFF: um valor somado ao sinal, que o desloca para cima ou para baixo.
- RECT: dobra para cima a parte negativa do sinal. No meio, a parte negativa some; no máximo, ela é espelhada.
- SLEW: suaviza mudanças bruscas, em até dois segundos. Degraus viram rampas.
- CRV: o formato dessa suavização, de rampa reta a curva que vai freando.
- SUM: se a saída SUM soma os dois canais ou tira a média deles.
- DRIFT: um passeio lento nos deslocamentos, que tira a rigidez de um valor parado.
Entradas: IN1 e IN2. Saídas: O1, O2 e SUM.

## Experimente
1. Faça uma voz: SAW de um OSC na entrada IN de um FILTER, e LO do FILTER no MIXER. Baixe CUT.
2. Ligue a saída ENV de um ENVELOPE, tocado pelo CLOCK, em IN1 do CONTROL, e O1 na entrada FC do FILTER. Cada nota abre o filtro.
3. Leve SCALE do primeiro canal para −1 e suba CUT. Agora cada nota fecha o filtro.
4. Ligue a saída BI de um FUNCTION em IN2 e escute a saída SUM no lugar de O1: as duas modulações agem juntas.
