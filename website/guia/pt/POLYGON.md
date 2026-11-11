## O que é
O POLYGON é um polímetro desenhado como figura. Há um ciclo comum e até quatro camadas; cada camada é um polígono de N vértices inscrito nesse ciclo, e cada vértice é um pulso. Um triângulo e um quadrado no mesmo ciclo dão três contra quatro.

## Como pensar nele
Pense num relógio com vários ponteiros invisíveis, cada um marcando o ciclo em partes iguais diferentes. Os pulsos de cada camada são regulares; o interesse está onde eles se encontram. A saída COINC marca esses encontros, e é ali que a música costuma pedir um acento. F gira cada polígono, até um lado, para deslocar a camada sem mudar a sua divisão.

## Controles
- N1, N2, N3, N4: quantos vértices tem cada camada, de 0 (desligada) a 16.
- F1, F2, F3, F4: a rotação de cada camada, até um lado do polígono.
- CICLO: a duração do ciclo, de 0,25 a 32 s, quando nada está ligado em CLK.
- PASSOS: com um relógio em CLK, quantos pulsos dura o ciclo.
Entradas: CLK conduz o ciclo por pulsos, RST recomeça. Saídas: G1 a G4 são os pulsos de cada camada, COINC pulsa quando dois ou mais vértices caem juntos, EOS no fim de cada ciclo, FASE é a posição no ciclo.

## Experimente
1. Com N1 em 3 e N2 em 4, ligue G1 num DRUM e G2 noutro: três contra quatro.
2. Ligue COINC num terceiro som: só os encontros.
3. Ponha N3 em 5 e gire F3 devagar: a camada de cinco desliza sobre as outras.
4. Ligue o CLOCK em CLK e PASSOS em 16: o polímetro passa a andar no andamento do patch.
