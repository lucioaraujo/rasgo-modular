## O que é
O GENDYN é uma fonte sonora em que a onda é um polígono de poucos pontos. A cada volta, cada ponto muda um pouco de altura e de distância até o próximo, ao acaso. A onda nunca se repete, mas também nunca foge: um som que vive.

## Como pensar nele
É a síntese estocástica dinâmica de Iannis Xenakis, a do Gendy3, de 1991. Cada ponto anda um passo ao acaso, como um bêbado, e as bordas o refletem de volta, para ele nunca grudar nelas. Com BAND em 0, a altura fica presa e só o desenho da onda respira: uma nota afinada e viva. Com BAND em 1, o período também passeia, e o som vai até o ruído. DIST escolhe o tipo de passo; a distribuição de Cauchy, a preferida de Xenakis, dá saltos raros e grandes.

## Controles
- PTS: quantos pontos tem o polígono.
- FREQ: a altura central.
- BAND: o quanto a altura pode fugir.
- AMP: o passo das amplitudes.
- DUR: o passo das durações.
- DIST: o tipo de passo.
- LEVEL: o volume.
Entradas: PTCH soma uma altura em volts por oitava, FRZ congela a onda enquanto o gate está alto, AMP soma no passo das amplitudes. Saída: OUT é o som.

## Experimente
1. Ponha BAND em 0,1 e ligue OUT num FILTER e num VCA: um baixo que respira.
2. Suba BAND e AMP até 1: a massa de ruído de Xenakis.
3. Ligue um gate lento em FRZ: a onda congela e descongela.
4. Ligue uma sequência em PTCH com BAND baixo: uma melodia de timbres vivos.
