## O que é
O SAMPLER grava um trecho do que entra nele, até uns oito segundos, divide esse trecho em fatias e as toca quando recebe pulsos. É o gesto dos samplers de hip-hop e de música eletrônica: pegar uma gravação, picá-la e tocá-la em outra ordem, mais rápida, mais lenta ou ao contrário.

## Como pensar nele
Enquanto a entrada REC estiver ligada, ele grava o que chega em IN. Quando REC desliga, o trecho fica guardado. A partir daí, cada pulso em TRIG toca uma fatia, e a entrada POS escolhe qual. Com um TRIGSEQ disparando e uma SEQUENCE escolhendo as fatias, a gravação se recombina num ritmo novo. Ligando a saída do próprio patch na entrada, o instrumento passa a reaproveitar o que acabou de tocar.

## Controles
- START: onde, dentro da fatia, a leitura começa.
- SPEED: a velocidade de leitura, de um quarto a quatro vezes a original. Negativo toca ao contrário. Normalmente, mais rápido também soa mais agudo.
- SLICE: em quantas fatias iguais a gravação é dividida, de 1 a 16.
- REPIT: desligado, velocidade e altura andam juntas, como numa fita; ligado, a altura vem da entrada PIT e a duração da fatia se mantém.
- WEAR: desgaste a cada disparo: um início impreciso, perda de definição, som granulado.
- LOOP: desligado, cada fatia toca uma vez; ligado, ela se repete.
Entradas: TRIG, o pulso que toca; IN, o som a gravar; REC, o sinal que grava enquanto está ligado; POS, para escolher a fatia; PIT, para afinar. Saída: OUT.

## Experimente
1. Ligue uma voz que toque uma frase, por exemplo uma SEQUENCE tocando um OSC, na entrada IN.
2. Para gravar, ligue a saída DIV de um LOGIC em REC, com a saída CLK do CLOCK no CLK do LOGIC e DIV em 16. Ligue OUT do SAMPLER no MIXER.
3. Ponha SLICE em 8, ligue T1 de um TRIGSEQ em TRIG e a saída CV de um TURING em POS. A frase gravada volta picada e reordenada.
4. Mova SPEED para a esquerda do centro e ouça as fatias ao contrário.
