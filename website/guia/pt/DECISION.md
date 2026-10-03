## O que é
O DECISION é a fonte de escolhas do patch. A cada pulso, ele decide se dispara ou não um sinal na saída GATE, e sorteia dois valores nas saídas X e Y. Você controla a chance de disparar, o tamanho e a forma dos sorteios, e quanto ele se lembra do que já fez.

## Como pensar nele
Uma onda lenta é previsível demais, e o ruído puro não tem forma. O DECISION fica no meio: acaso com regras. BIAS decide se algo acontece quase sempre ou só às vezes. SHAPE decide se os valores se espalham por igual ou se concentram perto do centro, com saltos grandes raros. DEJA é a memória: em vez de sortear de novo, ele relê os últimos passos, e a música passa a ter repetições, como um tema que volta.

## Controles
- RATE: o relógio interno, usado quando nada chega em TRIG.
- BIAS: a chance de GATE disparar a cada pulso. Em zero, nunca; no máximo, sempre.
- SPRD: o alcance dos sorteios em X e Y. Baixo, valores próximos do centro; alto, a faixa inteira.
- SHAPE: em zero, todos os valores têm a mesma chance; no máximo, os valores perto do centro aparecem bem mais.
- STEPS: divide os sorteios em degraus. Em 1, são contínuos.
- SLEW: faz cada valor novo escorregar a partir do anterior.
- DEJA: a chance de repetir um valor da memória em vez de sortear outro. Alto, a mesma frase tende a se repetir.
- LOOP: o tamanho dessa memória, de 1 a 16 passos.
Entradas: TRIG, o pulso de cada decisão; BIAS e SPRD, para modular esses controles. Saídas: X e Y, os valores sorteados; GATE, a decisão.

## Experimente
1. Ligue a saída EUC do CLOCK em TRIG.
2. Ligue X na entrada CV de um QUANTIZER, PTCH em 1V/O de um OSC, e o OSC por um ENVELOPE até o MIXER. Ligue GATE do DECISION em GATE do envelope.
3. Baixe BIAS e ouça as notas ficarem esparsas, só às vezes.
4. Suba DEJA aos poucos. A melodia começa a se repetir, e com DEJA no máximo fica presa num laço de LOOP passos.
