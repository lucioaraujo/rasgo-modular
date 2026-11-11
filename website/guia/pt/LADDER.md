## O que é
O LADDER é o filtro em escada, o filtro do grave: quatro degraus de 6 dB em cascata, com a saída do último devolvida à entrada como ressonância. É o desenho que R. Moog patenteou em 1969, aqui com um knob que vai da escada de transistores, redonda, à escada de diodos, ácida.

## Como pensar nele
O FILTER é limpo e estável; o LADDER é o filtro que se comprime e encorpa quando é empurrado. Três coisas o distinguem. DIODO acopla os degraus, e no meio do caminho há escadas que não existem em circuito. O pico fica afinado no CUT em qualquer posição de DIODO, e em RESO 1 a escada canta uma senoide afinada. E COMP devolve o grave que, na escada original, some quando a ressonância sobe.

## Controles
- CUT: a frequência de corte, de 20 Hz a 20 kHz.
- RESO: a ressonância. Em 1, a escada auto-oscila; 0,9 significa a mesma proximidade do limiar em qualquer DIODO.
- DRIVE: empurra a entrada contra a saturação dentro do laço; o nível de saída é compensado.
- COMP: devolve o grave. Em 1, o grave fica inteiro com qualquer ressonância; em 0, afunda como no original.
- DIODO: de 0 (transistores, redondo) a 1 (diodos, ácido), contínuo.
Entradas: IN recebe o som, FC move o corte em 1 volt por oitava, Q move a ressonância. Saídas: LP4 é a saída de 24 dB por oitava, LP2 a tomada do segundo degrau, de 12 dB por oitava, mais brilhante.

## Experimente
1. Ligue a saída SAW de um OSC em IN, LP4 num VCA e o VCA no MIXER. Ponha CUT perto de 200 Hz.
2. Suba RESO com COMP em 1: o pico canta e o grave fica. Baixe COMP e ouça o grave afundar.
3. Ponha DIODO em 1, RESO em 0,85 e DRIVE em 0,6, com um envelope curto em FC: o baixo ácido.
4. Tire o cabo de IN, ponha RESO em 1 e ligue a altura de um QUANTIZER em FC: a escada toca a melodia como um oscilador.
