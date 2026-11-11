## O que é
O ACCENT dá fraseado a uma linha. Um intérprete não acentua só com volume: a nota acentuada é mais forte, mais brilhante e mais curta, e ele acentua por várias razões. O ACCENT junta essas causas, cada uma com o seu peso, numa ênfase de 0 a 1, e entrega essa ênfase em várias saídas ao mesmo tempo: nível, brilho, duração, varredura.

## Como pensar nele
São seis causas de acento: o tempo forte (METR), o tamanho do salto (SALTO), a nota mais aguda da frase recente (APICE), o acento que vem de fora (EXT), a nota depois de uma nota longa (AGOG) e o acento de fora num tempo fraco (SINC). Três controles moldam o conjunto: HER deixa uma cauda na nota seguinte, SUPR impede que tudo vire acento, ROT faz o tempo forte migrar devagar. PROF é quanto disso chega às saídas.

## Controles
- METR: o peso do tempo forte de cada ciclo.
- SALTO: o peso do tamanho do intervalo desde a nota anterior.
- APICE: o peso da nota mais aguda das últimas oito.
- EXT: o peso do acento que chega pela entrada ACC.
- AGOG: o peso da nota que vem depois de uma nota longa.
- SINC: o peso de um acento de fora num tempo fraco.
- CICLO: de quantas notas é o ciclo métrico, de 2 a 16.
- ROT: a velocidade com que o tempo forte muda de lugar.
- HER: a cauda que um acento deixa na nota seguinte.
- SUPR: depois de um acento forte, o seguinte custa mais.
- PROF: a profundidade com que a ênfase chega às saídas.
Entradas: GATE marca cada nota, PTCH recebe a altura, ACC o acento de fora, RST recomeça o ciclo. Saídas: ENF é a ênfase, VCA o nível, FC o brilho para um filtro, DEC a duração para um envelope, SWP uma varredura que sobe com acentos seguidos, GT um gate só nas notas acentuadas, NIV a ênfase em quatro degraus (0, um terço, dois terços, 1).

## Experimente
1. Toque uma linha de SEQUENCE por um QUANTIZER num OSC. Ligue a altura e o gate do QUANTIZER em PTCH e GATE.
2. Ligue a saída VCA no CV1 do VCA da voz: a linha passa a respirar como uma frase.
3. Passe o OSC por um LADDER e ligue SWP no FC dele: os acentos seguidos sobem a varredura, como num baixo ácido.
4. Ponha CICLO em 5 e ROT em 0,5: o tempo forte muda de lugar e as notas continuam as mesmas.
