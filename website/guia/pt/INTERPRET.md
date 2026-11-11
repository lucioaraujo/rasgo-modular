## O que é
O INTERPRET toca as notas que chegam como um intérprete tocaria. Ele não inventa notas: recebe a altura e o gate de qualquer sequenciador e devolve a nota tocada, com glide pelo intervalo, um pequeno excesso nos saltos, vibrato que cresce na nota longa, articulação pelo contexto, fôlego e brilho.

## Como pensar nele
A partitura diz a nota; o intérprete diz como. O módulo olha para as notas recentes: marca o salto, sustenta o ápice da frase, encurta duas notas rápidas, liga o resto. O fôlego se gasta ao longo da frase e volta numa pausa. Três eixos de caráter (ENERG, SUAV, BRILHO) dão a personalidade, e DERIVA a faz mudar devagar. Com EXPR em 0, a nota sai crua.

## Controles
- EXPR: o quanto de interpretação; em 0, a nota crua.
- GLIDE: o tempo de deslizar entre notas, até 0,3 s.
- VIB: a profundidade do vibrato nas notas longas.
- ENERG: o caráter enérgico, que marca os saltos.
- SUAV: o caráter suave, que liga as notas.
- BRILHO: o caráter brilhante, que abre o timbre no ápice.
- DERIVA: o quanto o caráter muda com o tempo.
Entradas: PTCH recebe a altura, GATE o gate de cada nota, CAR move o caráter por CV. Saídas: PTCH é a altura tocada, AMP a dinâmica, BRT o brilho, FRASE a posição na frase, GT o gate articulado.

## Experimente
1. Ligue a altura e o gate de um QUANTIZER em PTCH e GATE. Ligue a saída PTCH no 1V/O de um OSC e GT no envelope da voz.
2. Ponha EXPR em 0 e depois em 0,8: compare a nota crua com a tocada.
3. Ligue AMP no CV1 do VCA da voz e BRT no corte do filtro: a linha ganha dinâmica e cor.
4. Suba VIB e GLIDE numa linha lenta: ela passa a cantar.
