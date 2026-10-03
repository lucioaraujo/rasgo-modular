## O que é
O LOGIC combina e transforma ritmos. Ele divide um pulso para criar ritmos mais lentos, compara dois ritmos para gerar um terceiro e alterna um estado a cada pulso. São operações simples, das que se usam em circuitos digitais, aplicadas ao tempo musical.

## Como pensar nele
Um ritmo interessante raramente é um pulso reto. Costuma ser a relação entre dois: um que toca na metade da velocidade, um que só toca quando outro também toca, um que toca quando um ou outro toca mas nunca os dois juntos. O LOGIC fica entre as fontes de pulsos, como CLOCK, TURING e SEQUENCE, e os módulos que eles disparam. Sem nada na entrada CLK, ele usa um relógio próprio.

## Controles
- RATE: o relógio interno, usado quando nada chega em CLK.
- DIV: divide o pulso. Em 2, a saída DIV pulsa uma vez a cada dois pulsos; em 3, uma a cada três.
- MULT: acrescenta pulsos intermediários dentro de cada período.
- GATE: quanto tempo o pulso da saída DIV fica ligado.
- DELAY: atrasa o pulso da saída DIV em até 200 milissegundos.
Entradas: CLK, o pulso a dividir; A e B, os dois ritmos a comparar; RST, para zerar. Saídas: DIV, o pulso dividido; AND, quando A e B estão ligados juntos; OR, quando um ou outro está; XOR, quando só um deles está; FLIP, que muda de estado a cada pulso em A.

## Experimente
1. Ligue a saída CLK do CLOCK na entrada CLK do LOGIC e DIV na entrada GATE de um DRUM ligado ao MIXER. Mude DIV para ouvir o pulso ficar mais lento.
2. Ligue EUC do CLOCK em A, e PLS de um TURING em B.
3. Leve a percussão para a saída XOR e depois para AND. Ouça como cada combinação gera um ritmo diferente a partir dos mesmos dois.
