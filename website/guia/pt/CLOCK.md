## O que é
O CLOCK é o relógio do patch. Ele emite pulsos regulares no andamento que você escolher e, a partir deles, cria ritmos e acentos. Quase todo patch começa por ele: os pulsos fazem a sequência avançar, disparam os envelopes e tocam a percussão.

## Como pensar nele
Em vez de programar batida por batida, você descreve o ritmo com dois números. LEN diz quantos passos tem o ciclo, FILL diz quantos deles tocam, e o CLOCK espalha esses toques da forma mais uniforme possível. Esse método, chamado ritmo euclidiano, reproduz muitos padrões tradicionais do mundo inteiro: 3 em 8 dá o tresillo cubano, 5 em 8, o cinquillo. A saída CLK pulsa em todos os passos; EUC só nos passos escolhidos; ACC marca os acentos.

## Controles
- BPM: o andamento, de 20 a 300 batidas por minuto.
- MULT: quantos passos cabem numa batida.
- LEN: o tamanho do ciclo, de 1 a 32 passos.
- FILL: quantos passos do ciclo tocam na saída EUC. Poucos deixam o ritmo esparso; perto de LEN, quase contínuo.
- ROT: gira o padrão, começando de outro ponto. A densidade é a mesma, mas a sensação muda.
- SWING: atrasa um passo a cada dois, dando balanço.
- DRIFT: deixa o andamento variar de leve, como um baterista que acelera e recua.
- GATE: quanto tempo cada pulso fica ligado, dentro do passo.
- ACC-A, ACC-B: os acentos caem nos passos múltiplos destes números. Com 4 e 3, por exemplo, os acentos formam um padrão de três contra quatro.
- AND: decide se o acento precisa dos dois números ao mesmo tempo ou basta um deles.
- FEEL: em quantas partes cada batida se divide: em dois, em três (tercinas), em cinco, sete, nove, onze, ou numa divisão sorteada a cada passo.
Entradas: EXT, para seguir pulsos de fora; RST, para voltar ao começo; BPM, para modular o andamento. Saídas: CLK, EUC e ACC.

## Experimente
1. Ligue EUC na entrada GATE de um DRUM, e OUT do DRUM numa entrada do MIXER.
2. Ponha LEN em 8 e mova FILL de 1 a 8. Em 3 e em 5 você ouve padrões conhecidos.
3. Gire ROT e ouça o mesmo padrão começar de outro lugar.
4. Ligue ACC na entrada ACC do DRUM e mude ACC-A e ACC-B para ouvir os acentos se deslocarem.
