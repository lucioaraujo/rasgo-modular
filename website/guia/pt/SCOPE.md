## O que é
O SCOPE mostra a forma de onda de um sinal, como um osciloscópio, e mede o que ouve: o volume, o brilho, a altura e o início de cada nota. Essas medidas saem por cabos, como sinais de controle. Assim o patch pode reagir ao próprio som.

## Como pensar nele
Num osciloscópio comum, a tela é o fim do caminho. Aqui, o que o SCOPE mede pode voltar ao patch: o brilho da mistura abre um filtro, a altura de uma voz afina outra, o começo de cada nota dispara um envelope. O som passa por ele sem mudança, pela saída THRU, então ele pode ficar no meio de qualquer cadeia. Com o SIGNAL-IN, ele também escuta um som de fora e faz o Rasgo Modular acompanhá-lo.

## Controles
- TRIG: o nível que o sinal precisa cruzar para estabilizar a imagem na tela.
- EDGE: se esse cruzamento conta na subida ou na descida.
- REJ: uma margem de tolerância, para que ruído não dispare a tela várias vezes.
- RESP: a rapidez das medidas de volume, brilho e altura. Rápido, acompanham cada detalhe; lento, mostram a tendência.
- HOLD: congela as medidas no valor atual.
- SENS: a sensibilidade do detector de início de nota. Alto, qualquer subida conta; baixo, só ataques fortes.
Entradas: IN, o som; EXT, um sinal externo para estabilizar a tela. Saídas: THRU, o som intacto; TRIG, o pulso do disparo; LVL, o volume; BRT, o brilho; PIT, a altura; ONS, um pulso no início de cada nota.

## Experimente
1. Ponha o SCOPE entre uma voz e o MIXER: a voz em IN, THRU no MIXER. Veja a forma de onda.
2. Ligue BRT na entrada FC de um FILTER de outra voz. Quando a primeira fica brilhante, a segunda também se abre.
3. Ligue ONS na entrada GATE de um DRUM. Cada nota da primeira voz agora vem com um golpe.
