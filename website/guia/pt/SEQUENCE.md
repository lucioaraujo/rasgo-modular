## O que é
A SEQUENCE toca uma frase curta que você escreve: até oito notas, cada uma com sua altura, cada uma ligada ou em silêncio. A cada pulso na entrada CLK ela avança um passo, entregando a altura na saída PTCH e o pulso da nota na saída GATE. Serve para uma linha de baixo, um riff, uma figura que se repete.

## Como pensar nele
O TURING faz a frase nascer do acaso; a SEQUENCE parte de uma frase que você decide. A variação vem do jeito de ler: para a frente, de trás para a frente, indo e voltando, em ordem sorteada, ou vagando de um passo para o vizinho. A mesma frase de oito notas rende muita coisa mudando só MODE e LEN. Para que as alturas caiam nas notas de uma escala, passe PTCH por um QUANTIZER antes do oscilador.

## Controles
- LEN: quantos dos oito passos entram na frase.
- MODE: a ordem de leitura: para a frente, para trás, ida e volta, sorteada, ou vagando passo a passo.
- RATE: o relógio interno, usado quando nada chega em CLK.
- GATE: quanto tempo cada nota fica ligada dentro do passo. Curto soa destacado; longo, ligado.
- GLIDE: faz a altura escorregar de uma nota à seguinte.
- RANGE: quantas oitavas os oito valores cobrem, até duas.
- P1 a P8: a altura de cada passo.
- G1 a G8: liga ou silencia cada passo.
Entradas: CLK, o pulso que avança; RST, para voltar ao primeiro passo. Saídas: PTCH, a altura; GATE, o pulso da nota; EOS, um pulso a cada fim de frase.

## Experimente
1. Ligue a saída CLK do CLOCK em CLK da SEQUENCE.
2. Ligue PTCH na entrada CV de um QUANTIZER, e PTCH do QUANTIZER em 1V/O de um OSC. Passe o OSC por um ENVELOPE até o MIXER e ligue GATE da SEQUENCE em GATE do envelope.
3. Mexa em P1 a P8 até a frase agradar e desligue alguns passos com G1 a G8.
4. Troque MODE e ouça a mesma frase lida de outros jeitos.
