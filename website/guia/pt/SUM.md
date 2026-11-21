## O que é
O SUM junta sinais. Quatro entradas, cada uma com o seu atenuversor, viram uma saída só: a soma, a soma invertida e a média.

## Como pensar nele
No Rasgo Modular cada entrada recebe um cabo só. Para um filtro andar com dois movimentos ao mesmo tempo, os dois passam antes pelo SUM. O MULT copia um sinal para vários lugares; o SUM junta vários sinais num lugar.

## Controles
- A1 a A4: quanto de cada entrada entra na soma, de −2 a +2. No meio, a entrada fica de fora; abaixo do meio, entra invertida.
- OFS: um valor fixo somado ao resultado.
- SAT: ligado, a soma satura de leve e nunca passa de ±1, bom para somar áudio.
Entradas: IN1 a IN4. Saídas: SUM, a soma; INV, a soma invertida; AVG, a média das entradas ligadas.

## Experimente
1. Ligue o BI de um FUNCTION em IN1 e uma saída de um DRIFT em IN2. Ligue SUM no cutoff de um FILTER: o filtro anda com os dois.
2. Gire A2 para baixo do meio: o DRIFT passa a empurrar o filtro para o lado contrário.
3. Ligue dois osciladores em IN1 e IN2, ligue SAT e mande SUM para o MIXER: os dois tocam juntos sem estourar.
