## O que é
O STAGES desenha uma curva feita de vários trechos encadeados, de dois a oito. Conforme os ajustes, essa curva vira um envelope com várias etapas, um movimento lento que não se repete exatamente, uma sequência de valores em degraus, ou até um som de forma estranha.

## Como pensar nele
O FUNCTION faz uma rampa só, e o ENVELOPE, as quatro etapas de sempre. O STAGES faz formas compostas, e você não desenha ponto por ponto: alguns controles moldam o desenho inteiro de uma vez. O controle que mais muda o caráter é HOLD. Com ele em zero, os trechos são rampas e o resultado é um movimento contínuo; no máximo, cada trecho salta e se mantém, e o STAGES passa a funcionar como um sequenciador. Com LOOP ligado, a forma se repete sozinha; desligado, ela corre uma vez a cada pulso em GATE.

## Controles
- SEGS: quantos trechos a forma tem, de 2 a 8.
- RATE: a velocidade de uma volta completa, quando LOOP está ligado.
- CNTR: o desenho dos níveis: uma escada que sobe, um arco, ou uma escada que desce.
- CURVE: como cada trecho vai de um nível ao outro: começando rápido, em linha reta, ou começando devagar.
- HOLD: em zero, rampas suaves; no máximo, degraus que saltam e seguram.
- TILT: deixa os primeiros trechos mais longos que os últimos, ou o contrário.
- JITR: faz níveis e durações variarem de leve a cada volta, sempre igual para a mesma semente.
- LOOP: ligado, a forma se repete; desligado, corre uma vez a cada pulso.
Entradas: GATE, o pulso que dispara; RST, para voltar ao começo; RTM, para modular a velocidade. Saídas: OUT, a curva; EOC, um pulso no fim de cada volta; STEP, um pulso a cada trecho.

## Experimente
1. Faça uma voz: SAW de um OSC na entrada IN de um FILTER, LO do FILTER no MIXER. Baixe CUT.
2. Ligue OUT do STAGES na entrada FC do FILTER, com SEGS em 6. O timbre passeia numa forma que vai e volta.
3. Mova CNTR e TILT e ouça o desenho mudar.
4. Leve HOLD ao máximo e SEGS a 8. Ligue OUT na entrada CV de um QUANTIZER e PTCH do QUANTIZER em 1V/O do OSC: o STAGES agora toca uma melodia.
