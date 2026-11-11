## O que é
O MAGNET é uma grade de 8 por 8 pequenos ímãs, cada um apontando para cima ou para baixo. Cada ímã quer se alinhar com os vizinhos, e a temperatura os agita. Uma cabeça anda pela grade, uma casa por pulso, e o ímã sob ela dá o gate.

## Como pensar nele
É o modelo de Ising, da física dos materiais. Abaixo da temperatura crítica (perto de 2,27), formam-se domínios grandes e o ritmo fica estável. Acima, o ruído vence. Perto dela, domínios de todos os tamanhos aparecem e somem: é a borda do caos, e é ali que o módulo soa mais vivo. COUPL negativo faz os ímãs se oporem aos vizinhos, como um tabuleiro de xadrez.

## Controles
- TEMP: a temperatura; a transição fica perto de 2,27.
- COUPL: o acoplamento. Positivo, os ímãs se alinham; negativo, se opõem.
- FIELD: um ímã externo que empurra a grade inteira para um lado.
- SWEEP: quantas tentativas de virar ímãs há a cada pulso.
- RATE: o relógio interno, quando nada está ligado em CLK.
- GATE: a duração de cada gate.
Entradas: CLK avança um pulso, QNCH esquenta tudo de uma vez e a grade volta a se organizar, TEMP e FLD movem temperatura e campo por CV. Saídas: GT é o gate da cabeça, MAG a magnetização, ENER a energia da grade, FLIP quanto da grade virou neste pulso.

## Experimente
1. Ligue GT num DRUM. Com TEMP em 1, ouça um ritmo quase parado; em 4, um ritmo de ruído.
2. Ponha TEMP perto de 2,3: domínios aparecem e somem, e o ritmo respira.
3. Ligue um gatilho raro em QNCH: a grade se desfaz e se reorganiza.
4. Ligue MAG no corte de um FILTER: o timbre segue para que lado a grade pende.
