## O que é
O CUTUP recorta o próprio som no andamento do patch. Ele guarda os últimos quatro segundos e, quando chamado, faz um gesto de montagem sobre os últimos tempos do relógio: gagueira, rajada embaralhada, repetição microscópica ou silêncio.

## Como pensar nele
É a técnica de recorte de William Burroughs e da colagem de fita, posta no barramento. Os gestos vêm por gate, cada um na sua entrada, ou sozinhos com CHANCE. Cada fragmento tem bordas suaves de 3 ms, e o gesto inteiro também, para o corte não estalar.

## Controles
- CHANCE: a frequência dos gestos espontâneos; em 0, só por gate.
- MICRO: quantas repetições tem o gesto microscópico, de 2 a 8.
- MIX: a proporção entre o som original e o recortado.
- SEED: a semente que escolhe gestos e fatias.
Entradas: IN recebe o som, CLK o relógio do patch, STUT dispara a gagueira, BURST a rajada de fatias embaralhadas, MICRO a repetição microscópica, GAP um tempo de silêncio. Saídas: OUT é o som recortado, ATIVO um gate enquanto um gesto soa.

## Experimente
1. Ligue a saída do MIXER em IN, OUT no MASTER e o CLOCK em CLK.
2. Ponha CHANCE em 0,15: de vez em quando, o patch se recorta no tempo certo.
3. Ligue a saída euclidiana do CLOCK em STUT: a gagueira cai nos tempos do ritmo.
4. Ligue o EOS de um SEQUENCE em GAP: cada frase termina com um silêncio cortado.
