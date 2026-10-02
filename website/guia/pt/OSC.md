## O que é
O OSC é um oscilador: ele produz uma nota contínua, sempre igual, a partir da qual quase todo som de sintetizador é construído. Ele entrega cinco formas de onda ao mesmo tempo, cada uma num jack de saída: seno, triângulo, dente de serra, pulso e uma voz uma ou duas oitavas abaixo (o sub). Você escolhe qual usar, ou usa várias.

## Como pensar nele
Pense no OSC como a matéria-prima. Sozinho ele soa cru, e é de propósito: o caráter vem do que você liga depois dele, como um filtro que tira brilho ou um envelope que dá forma à nota. Se você quer uma voz com personalidade pronta, o STRING e o MATTER já trazem isso; se quer um timbre que se transforma sozinho, experimente o WAVETABLE. O OSC é a base previsível, e por isso é o melhor lugar para aprender.

## Controles
- FREQ: a altura da nota, de 8 a 8000 Hz. Valores baixos viram vibração lenta; a região musical fica entre uns 50 e 1000 Hz.
- FINE: afinação fina, até um semitom para cima ou para baixo. Use para afinar com outra voz, ou para desafinar de leve e engrossar o som.
- PW: a largura do pulso, que só afeta a saída PLS. No meio, a onda é quadrada e soa oca; perto das pontas, fica fina e anasalada.
- FM: quanto o sinal ligado na entrada FM mexe na altura. Com outro oscilador ali e este knob alto, o timbre fica metálico, de sino.
- DRIFT: uma oscilação lenta e pequena na afinação, como a de um oscilador analógico. Em zero, a nota fica perfeitamente parada.
- SUB2: escolhe se a saída SUB fica uma oitava (desligado) ou duas (ligado) abaixo da nota.
- SYNC: liga a entrada SYNC. Com ela ativa, outro oscilador mais lento força este a recomeçar o ciclo, e girar FREQ passa a mudar o timbre em vez da altura.
- PROX: mistura cada saída com uma versão mais escura de si mesma. É um jeito de amaciar o som sem gastar um filtro.
Entradas: 1V/O recebe a altura vinda de um sequenciador ou quantizador, FM e PWM aceitam modulação, e SYNC recebe o oscilador que comanda o sincronismo.

## Experimente
1. Ligue a saída SAW do OSC numa entrada do MIXER e ouça um zumbido constante.
2. Gire FREQ devagar: a nota sobe e desce.
3. Troque o cabo para a saída PLS e mova PW de um lado ao outro. O timbre muda de oco para fino, sem a altura mudar.
4. Volte para SAW e passe o som por um FILTER antes do MIXER. Agora você está fazendo síntese subtrativa: começar com um som rico e tirar o que sobra.
