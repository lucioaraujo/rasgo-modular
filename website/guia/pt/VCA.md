## O que é
O VCA controla o volume de um som por meio de outro sinal. Ele tem dois canais iguais: o som entra em IN, um sinal de controle entra em CV, e o volume da saída sobe e desce conforme esse controle. É o módulo mais usado de qualquer modular.

## Como pensar nele
Pense nele como uma mão no botão de volume, movida por outro módulo. Com um envelope no controle, cada nota ganha começo, meio e fim. Com uma onda lenta, o volume ondula, no efeito chamado tremolo. Ele também serve para dosar um sinal de controle antes de mandá-lo a outro lugar, e a saída SUM soma os dois canais, como um mixer pequeno. Atenção: o nível começa em zero, então o canal fica mudo até algo chegar ao CV ou você subir o LVL.

## Controles
- LVL1, LVL2: o volume de cada canal. O sinal de controle soma a este valor.
- CV1, CV2: o quanto o sinal de controle age, e em que sentido. Negativo inverte: o volume cai quando o controle sobe.
- RSP1, RSP2: a curva de resposta. Em zero, linear; no máximo, exponencial, que soa mais natural para volume.
- DRIFT: uma oscilação lenta e pequena nos dois volumes.
Entradas: IN1 e IN2, os sons; CV1 e CV2, os controles. Saídas: O1, O2 e SUM, a soma dos dois.

## Experimente
1. Ligue a saída SAW de um OSC em IN1, e O1 numa entrada do MIXER. Silêncio: LVL1 está em zero.
2. Suba LVL1 devagar e o som aparece.
3. Volte LVL1 a zero e ligue a saída ENV de um ENVELOPE em CV1, com o CLOCK na entrada GATE do envelope. O som agora pulsa com as notas.
4. Troque o envelope pela saída UNI de um FUNCTION lento: o volume ondula, num tremolo.
