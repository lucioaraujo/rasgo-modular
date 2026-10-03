## O que é
O CRUSH suja o som do jeito digital. Ele imita os defeitos dos primeiros samplers e videogames, e também os de uma conexão ruim: menos amostras por segundo, menos degraus de volume, números que estouram e voltam do outro lado, trechos que travam ou somem. O resultado vai de uma leve aspereza a uma voz de robô em pedaços.

## Como pensar nele
O SHAPE distorce como um circuito analógico, arredondando e dobrando a onda. O CRUSH quebra o som em degraus. Cada controle é um tipo diferente de estrago, e você pode usar um só ou somar vários. As falhas são sorteadas, mas a partir da semente do patch, então o mesmo patch estraga o som sempre do mesmo jeito. Com um envelope na entrada MXM, a nota começa limpa e vai se degradando.

## Controles
- RATE: quantas vezes por segundo o som é lido, de 100 a 24000. Valores baixos deixam o som em escada e trazem notas fantasmas, aquele chiado metálico dos aparelhos de 8 bits.
- BITS: quantos degraus de volume restam, de 1 a 16. Em 16 quase não se nota; em 1, tudo vira onda quadrada.
- DRIVE: o ganho antes do estrago, que empurra o som para fora dos limites.
- WRAP: o que acontece quando o som passa do limite. Em zero, ele é cortado; no máximo, reaparece do lado oposto, o que soa muito mais agressivo, sobretudo com DRIVE alto.
- GLTCH: a chance de falha: um trecho que trava, um buraco de silêncio, um repique.
- JITR: torna a leitura irregular, e a afinação oscila um pouco, como uma fita digital cansada.
- TONE: um filtro simples na saída. Para a esquerda escurece; para a direita deixa só o chiado agudo; no meio não muda nada.
- MIX: a mistura entre o som limpo e o estragado. Em zero, o som passa intacto.
Entradas: IN, o som; RTM, para modular RATE; MXM, para modular MIX.

## Experimente
1. Ligue a saída SAW de um OSC na entrada IN, e OUT numa entrada do MIXER.
2. Baixe RATE até uns 4000 e BITS até 8. O som fica granulado.
3. Suba GLTCH aos poucos e ouça as falhas aparecerem.
4. Baixe MIX para zero e ligue a saída ENV de um ENVELOPE em MXM, com o CLOCK na entrada GATE do envelope. Cada nota começa limpa e se desfaz.
