## O que é
O DRIFT produz movimentos muito lentos, na escala dos minutos. Ligado a alguns controles de um patch, ele faz a música mudar aos poucos por conta própria: o timbre aos trinta segundos é outro aos quatro minutos, sem saltos e sem voltar ao mesmo ponto.

## Como pensar nele
Um patch sem ninguém mexendo tende a andar em círculos depois de um tempo. O DRIFT é a camada de desenvolvimento lento. Por dentro existe um único valor passeando, e as quatro saídas A, B, C e D são versões desse mesmo passeio, cada uma com seu peso. Por isso elas contam a mesma história de ângulos diferentes: quando uma sobe, as outras tendem a subir também. O passeio também tem inércia: quando ganha uma direção, tende a mantê-la por um tempo.

## Controles
- RATE: o ritmo do passeio, de um passo por segundo a um a cada oito minutos, mais ou menos.
- DEPTH: o alcance, quanto o passeio pode se afastar do ponto de repouso.
- MOMT: a inércia. Alto, o caminho é suave e insiste numa direção; baixo, muda de rumo a todo momento.
- STRD: em zero, as quatro saídas andam quase juntas; no máximo, cada uma vai para um lado.
- ANCHR: a memória. Em zero, o passeio nunca volta; alto, ele tende a retornar a lugares por onde já passou, o que dá à peça algo como temas que reaparecem.
- BIAS: desloca o ponto de repouso para cima ou para baixo.
Entradas: ADV, um pulso que força um passo; RATE, para modular a velocidade. Saídas: A, B, C e D, os quatro aspectos do passeio; FLD, o passeio sem pesos; EVT, um pulso a cada passo.

## Experimente
1. Monte uma voz qualquer que passe por um FILTER e por um SPACE antes do MIXER.
2. Ligue A na entrada FC do FILTER e B na entrada FBK do SPACE. Suba DEPTH.
3. Deixe tocar alguns minutos. O timbre e o espaço vão mudando juntos, sem pressa.
4. Para acompanhar a forma da música, ligue a saída EOS de uma SEQUENCE em ADV: o passeio dá um passo a cada fim de frase.
