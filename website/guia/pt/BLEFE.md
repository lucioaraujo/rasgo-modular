## O que é
O BLEFE é um jogo de confiança feito de sinais. Cada gatilho vira um sinal, que soa sempre igual, seja verdade ou blefe. Uma janela depois, a verdade se confirma e o blefe não. A cada resolução, a confiança anda: sobe com verdades, cai com blefes. Quando a confiança construída despenca, é traição, e a resposta se recusa até o perdão.

## Como pensar nele
Um número aleatório não tem intenção; um blefe tem. O módulo não sorteia notas, ele cria uma relação entre vozes: uma voz que responde enquanto confia, hesita quando desconfia e se cala depois de traída. VERD é a taxa de verdade, MEM a velocidade com que a confiança muda, LIM o ponto da traição. A entrada RST é o perdão. SEED fixa o jogo: a mesma semente dá as mesmas verdades e mentiras.

## Controles
- VERD: a taxa de verdade, de 0 a 1.
- JAN: a janela entre o sinal e a confirmação, de 0,05 a 4 s.
- MEM: a velocidade com que a confiança anda.
- LIM: abaixo deste nível, a confiança construída vira traição.
- SEED: a semente do jogo.
Entradas: IN recebe os gatilhos, RST perdoa e recomeça. Saídas: SIG é o sinal, sempre igual; VERD entrega a verdade escondida na hora, para quem quiser trapacear; CONF pulsa quando a verdade se confirma; CNF é o nível de confiança; RESP é a resposta, que se cala depois da traição; TRAI pulsa no instante da traição.

## Experimente
1. Ligue o CLOCK em IN, SIG no gate de um ENVELOPE e CONF num DRUM: o sinal e a confirmação soam como dois sons.
2. Ligue RESP no CV1 do VCA de outra voz: ela responde enquanto confia.
3. Comece com VERD em 0,9 e depois gire para 0,2. A voz que responde cresce, hesita e se cala.
4. Ligue o EOS de um SEQUENCE em RST: a cada frase, a relação recomeça.
