## O que é
O GLIDE faz uma nota escorregar até a seguinte, em vez de saltar. Ele fica no caminho do sinal de altura, entre quem escolhe as notas, como SEQUENCE ou QUANTIZER, e o oscilador. Esse escorregão, chamado portamento, é o que dá o canto às linhas de baixo ácidas e aos solos ligados.

## Como pensar nele
Um deslize sempre ligado cansa logo. O interessante é escolher quando escorregar, e o GLIDE decide isso nota por nota: pode deslizar sempre, só quando a entrada SLIDE está alta, ou só quando as notas estão ligadas, isto é, quando a nota nova chega antes de soltar a anterior. As saídas MOV e DONE avisam quando um deslize está acontecendo e quando ele termina, e podem disparar outras coisas.

## Controles
- TIME: quanto tempo leva para subir uma oitava, de zero, um salto seco, até dois segundos.
- FALL: deixa a descida mais rápida ou mais lenta que a subida. No meio, as duas são iguais.
- CURVE: o formato do deslize. Em zero, a velocidade é constante e a nota chega no tempo exato; no máximo, ela vai freando e demora mais para encostar.
- MODE: quando deslizar: sempre; só com SLIDE alto, como no sintetizador de baixo TB-303; ou só entre notas ligadas, lendo a entrada GATE.
Entradas: PITCH, a altura que chega; SLIDE e GATE, que decidem quando deslizar. Saídas: OUT, a altura conduzida; MOV, alta durante o deslize; DONE, um pulso quando ele termina.

## Experimente
1. Monte uma linha: a saída CLK do CLOCK na entrada CLK de uma SEQUENCE, e a saída SAW de um OSC no MIXER.
2. Ligue PTCH da SEQUENCE em PITCH do GLIDE, e OUT do GLIDE em 1V/O do OSC.
3. Suba TIME: as notas passam a escorregar umas nas outras.
4. Ponha MODE na terceira posição e ligue GATE da SEQUENCE em GATE do GLIDE. Agora só as notas ligadas escorregam.
