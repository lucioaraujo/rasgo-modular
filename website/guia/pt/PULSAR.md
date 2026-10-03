## O que é
O PULSAR produz um trem de pequenos pulsos sonoros, cada um seguido de um silêncio. Quando os pulsos se repetem depressa, você ouve uma nota; quando ficam lentos, ouve um ritmo. A técnica vem do compositor Curtis Roads e tem uma propriedade curiosa: a altura e o timbre são controlados separadamente, então dá para mudar a cor do som sem desafiná-lo.

## Como pensar nele
FREQ decide quantas vezes por segundo o pulso se repete, e portanto a nota. FRMT decide o que acontece dentro de cada pulso, e portanto o timbre: baixo soa oco, alto soa nasal. Abaixe FREQ até umas 30 repetições por segundo e cada pulso vira um evento que se ouve separado; com MASK e JITR, esses eventos ficam irregulares, como uma nuvem.

## Controles
- FREQ: a taxa de repetição dos pulsos, de 20 a 2000 Hz. É a altura da nota.
- FRMT: a frequência dentro de cada pulso, de 0,1 a 8 vezes FREQ. É o timbre: baixo é oco, alto é brilhante.
- SHAPE: a forma de cada pulso, de um seno simples a um pulso mais estreito e rico em harmônicos.
- WIND: o contorno de cada pulso. Para a esquerda, as bordas são duras e o som é brilhante; no meio, é limpo; para a direita, cada pulso ataca rápido e decai, como uma percussão.
- JITR: torna irregular o tempo e o volume de cada pulso. Em zero, o trem é rígido.
- MASK: a chance de cada pulso ser pulado. Abre buracos e cria padrões rítmicos sem mudar a nota.
- SPRD: espalha pulsos alternados entre esquerda e direita, alargando o som em estéreo.
- LEVEL: o volume de saída.
Entradas: PIT para a altura e FQM para modular o timbre. Saídas L e R, para o estéreo.

## Experimente
1. Ligue L numa entrada do MIXER e gire FRMT devagar. O timbre muda, mas a nota fica no mesmo lugar.
2. Leve FREQ para perto do mínimo. A nota se desfaz em pulsos que se ouvem um a um.
3. Com FREQ baixo, suba MASK e JITR. Os pulsos ficam esparsos e irregulares, como uma chuva.
4. Ligue a saída BI de um FUNCTION lento na entrada FQM. O timbre passeia sozinho sem desafinar.
