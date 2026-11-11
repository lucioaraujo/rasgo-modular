## O que é
O KEY ouve quem toca, estima o tom e entrega a tônica e a escala ao QUANTIZER. Assim o patch passa a responder no tom de quem canta ou toca por cima, em vez de ficar na escala que a seed escolheu.

## Como pensar nele
Ele conta quanto tempo cada uma das doze notas soou (a nota longa pesa mais que a de passagem) e compara esse retrato com os perfis de tonalidade de Krumhansl e Kessler. O tom só muda quando outro vence por ESTAB notas seguidas, para não tremular entre tons vizinhos. A saída CERT diz quando o tom é ambíguo, em vez de esconder isso. O visor mostra as doze notas, a tônica e o nome do tom.

## Controles
- MEM: quanto tempo ele lembra, de 2 a 60 s. Curto segue as modulações; longo fica firme.
- ESTAB: quantas notas seguidas um tom novo precisa vencer para valer.
Entradas: PTCH recebe a altura, GATE marca cada nota, RST esquece tudo. Saídas: ROOT é a tônica, SCL a escala, CERT a certeza, MUD um pulso a cada troca de tom.

## Experimente
1. Ligue o SIGNAL-IN no SCOPE, a saída PIT do SCOPE em PTCH e a saída ONS em GATE. Ligue ROOT e SCL nas entradas de tônica e escala do QUANTIZER.
2. Ligue o ON do SIGNAL-IN e cante uma melodia em ré menor: o visor mostra Dm e o patch passa a tocar nesse tom.
3. Ligue MUD na entrada FRASE do MIXER, com o palco no modo gesto: cada troca de tom muda o palco.
4. Sem microfone: um MARKOV num QUANTIZER, o QUANTIZER no KEY e o KEY num segundo QUANTIZER. A segunda voz segue o tom que a primeira parece ter.
