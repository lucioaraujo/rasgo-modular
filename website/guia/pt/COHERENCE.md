## O que é
O COHERENCE mede o quanto uma melodia é previsível e rompe quando ela fica ordenada demais. Ele guarda as últimas notas e calcula a entropia de Shannon delas: uma nota repetida dá ordem 1, as doze notas por igual dão 0. Quando a ordem passa do limiar por tempo suficiente, ele dispara um pulso de ruptura.

## Como pensar nele
Costuma-se pensar que a ruptura nasce do caos. Aqui é o contrário: a ordem extrema também rompe. A pressão sobe a cada nota previsível; depois de PAC notas seguidas, ela rompe. A ruptura deixa cicatriz: a janela esquece a metade mais antiga e, por CIC notas, a pressão não volta a subir. O módulo não soa: ele decide quando, e o cabo decide o quê.

## Controles
- JAN: a janela, quantas notas ele lembra, de 4 a 32.
- LIM: o limiar de ordem acima do qual a pressão sobe.
- PAC: a paciência, quantas notas seguidas acima do limiar até romper.
- CIC: a cicatriz, quantas notas depois da ruptura sem nova pressão.
Entradas: PTCH recebe a altura, GATE marca cada nota (sem ele, conta cada mudança de altura), RST esquece tudo. Saídas: ORD é a ordem medida, PRES a pressão acumulada, COL o pulso de ruptura, SURP a surpresa de cada nota.

## Experimente
1. Ligue a altura e o gate de um QUANTIZER que toca um SEQUENCE curto em PTCH e GATE.
2. Ligue COL no RST de um TURING que alimenta outra voz: quando a primeira linha se repete demais, a outra se refaz.
3. Ligue PRES no corte de um FILTER: o timbre abre enquanto a tensão cresce e fecha na ruptura.
4. Ligue SURP no nível de um VCA: a nota rara soa mais forte.
