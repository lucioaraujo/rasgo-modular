## O que é
O SH, de sample and hold, captura o valor de um sinal no instante de um pulso e o segura até o pulso seguinte. Com nada na entrada, ele sorteia um valor novo a cada pulso. É o jeito tradicional de transformar acaso em degraus: cada pulso, uma nota nova, um timbre novo. O SH tem dois canais.

## Como pensar nele
Dois sorteios independentes não têm nada a ver um com o outro, e às vezes é isso que se quer. Mas muitas vezes uma melodia soa melhor quando o timbre acompanha a nota, sem copiá-la. O controle CORR faz justamente essa relação entre os dois canais: de espelhados, um sobe quando o outro desce, a independentes, a gêmeos, que sorteiam o mesmo valor. Os controles de deslize transformam os degraus em escorregões.

## Controles
- RATE: o ritmo interno de sorteio, de 0,02 a 40 vezes por segundo. Só vale para o canal sem pulso ligado em T1 ou T2.
- SLW1, SLW2: o tempo que cada canal leva para escorregar até o valor novo. Em zero, salta na hora.
- SLOPE: deixa a subida mais rápida que a descida, ou o contrário.
- TRK1, TRK2: em vez de capturar só no instante do pulso, o canal acompanha a entrada enquanto o pulso estiver alto e segura quando ele cai.
- SPRD: muda o jeito do sorteio, tornando as variações pequenas mais frequentes que as grandes.
- CORR: a relação entre os sorteios dos dois canais: espelhados, independentes ou iguais. Só vale para os canais sem nada ligado na entrada.
Entradas: IN1 e IN2, os sinais a capturar; T1 e T2, os pulsos. Saídas: O1 e O2.

## Experimente
1. Faça uma voz: SAW de um OSC na entrada IN de um FILTER, LO do FILTER no MIXER.
2. Ligue a saída CLK do CLOCK em T1 e em T2.
3. Ligue O1 na entrada CV de um QUANTIZER, PTCH do QUANTIZER em 1V/O do OSC, e O2 na entrada FC do FILTER. Cada pulso traz uma nota e um timbre.
4. Gire CORR de um extremo ao outro. Num lado, notas agudas vêm escuras; no meio, sem relação; no outro, notas agudas vêm brilhantes.
