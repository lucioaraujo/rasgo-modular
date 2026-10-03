## O que é
O TURING inventa melodias por acaso e deixa você guardar as que agradam. Ele mantém um laço de alguns valores que gira a cada pulso; a cada volta, cada valor pode se manter ou ser trocado por outro. Com o controle LOCK você decide quanto o laço muda, de acaso total a repetição fixa.

## Como pensar nele
Aqui você compõe escolhendo pelo ouvido, não escrevendo nota por nota. Deixe LOCK no meio, ouça a melodia se transformar e, quando surgir algo bom, gire LOCK até o fim: o laço trava e se repete. Para variar de novo, solte um pouco. O nome homenageia o módulo Turing Machine, da Music Thing Modular, que popularizou essa ideia. A SEQUENCE toca uma frase escrita; o TURING faz frases nascerem e se fixarem.

## Controles
- RATE: o relógio interno, usado quando nada chega em CLK.
- LEN: o tamanho do laço, de 2 a 16 passos.
- LOCK: a chance de o laço se manter. Em zero, tudo é sorteado de novo; no máximo, a frase trava e se repete.
- MUT: quando um valor muda, o quanto muda. Baixo, pequenas variações do que já estava; alto, um sorteio novo.
- RANGE: a extensão dos valores de saída, ou seja, o tamanho dos saltos da melodia.
- STEPS: divide a saída em degraus. Em 1, os valores são contínuos.
- OFST: o centro dos valores, que sobe ou desce a região da melodia.
Entradas: CLK, o pulso que faz o laço girar; LOCK, para modular o travamento. Saídas: CV, a melodia; CV2, outra leitura do mesmo laço, aparentada mas diferente; PLS, um ritmo tirado do mesmo laço.

## Experimente
1. Ligue a saída CLK do CLOCK na entrada CLK do TURING.
2. Ligue CV na entrada CV de um QUANTIZER, e PTCH do QUANTIZER em 1V/O de um OSC passando por um ENVELOPE até o MIXER. Ligue PLS na entrada GATE do envelope.
3. Com LOCK no meio, ouça a melodia mudar aos poucos.
4. Quando gostar do que ouviu, leve LOCK ao máximo. A frase fica.
