## O que é
O PLL é um oscilador que sabe seguir outro. Sozinho, funciona como qualquer oscilador, com altura própria e uma forma de onda que você escolhe. Quando recebe um segundo sinal na entrada REF, ele ajusta a própria velocidade aos poucos até andar junto com esse sinal, como um músico que acerta o passo com outro.

## Como pensar nele
O OSC também pode se prender a outro oscilador, mas de um golpe só (o sync). O PLL faz isso devagar, e esse caminho até travar é audível: um deslizar de altura. Com RATIO, ele trava numa oitava acima, numa abaixo ou em divisões mais fundas da referência. A saída LOCK diz o quanto ele está travado, e serve para fazer outras coisas reagirem a isso.

## Controles
- FREQ: a altura quando não há referência, e o ponto de partida quando há.
- FINE: afinação fina, até um semitom para cada lado.
- SHAPE: a forma da onda, passando de seno para triângulo, dente de serra e quadrada.
- RATIO: em que relação com a referência ele trava. Em 1, na mesma nota; em 2, uma oitava acima; perto do mínimo, muito abaixo.
- LOCK: a força com que persegue a referência. Baixo, ele desliza devagar até chegar; alto, trava logo, mas fica mais nervoso.
- FM: quanto o sinal na entrada FM mexe na altura.
- FBK: quanto o próprio sinal volta para dentro do circuito, acrescentando textura sem desafinar.
- FTYP: escolhe entre seis tipos de retorno, cada um com uma cor diferente.
Entradas: 1V/O para a altura, FM e REF, a referência a seguir. Saídas: OUT, RING (o PLL multiplicado pela referência) e LOCK, um sinal de controle que indica o quanto ele está travado.

## Experimente
1. Ligue OUT numa entrada do MIXER. Sem nada em REF, é um oscilador comum; gire SHAPE para ouvir as formas.
2. Ligue a saída SAW de um OSC na entrada REF. O PLL procura a altura do OSC e se prende a ela.
3. Baixe LOCK. Agora ele demora para chegar, e você ouve a altura deslizando.
4. Mude RATIO para 2. Ele trava uma oitava acima do OSC.
