## O que é
O WASP é um filtro que distorce. Inspirado num sintetizador do fim dos anos 1970 conhecido pelo som áspero, ele corta frequências como o FILTER, mas suja o som quando a ressonância sobe. É o filtro para baixos agressivos, solos que cortam e drones que rangem.

## Como pensar nele
O FILTER é limpo e preciso; o WASP é o contraponto sujo. Três knobs dão a medida da sujeira: DRIVE empurra o som contra a distorção antes de filtrar, GRIT decide o quanto o filtro range ao ressoar, e BIAS deixa a distorção desequilibrada, mais zumbida. Com a ressonância alta, ele soa sozinho, e esse som também é áspero.

## Controles
- CUT: a frequência de corte, de 20 a 24000 Hz.
- RESO: a ressonância. Perto do máximo, o filtro soa sozinho.
- MODE: o tipo de filtro, passando de corta-agudos para faixa do meio e corta-graves.
- DRIVE: o ganho de entrada, que empurra o som para a distorção antes de filtrar.
- GRIT: o quanto o filtro distorce ao ressoar. Baixo é quase limpo; alto range.
- BIAS: desequilibra a distorção, acrescentando harmônicos pares e um zumbido.
- DRIFT: uma oscilação lenta no corte e na ressonância.
Entradas: IN, o som a filtrar; FC, para mover o corte; Q, para a ressonância.

## Experimente
1. Ligue a saída SAW de um OSC na entrada IN, e OUT numa entrada do MIXER.
2. Suba RESO até uns dois terços e gire CUT: ouça o filtro morder.
3. Suba GRIT e DRIVE. O som fica áspero e agressivo.
4. Ligue a saída ENV de um ENVELOPE em FC, com o CLOCK na entrada GATE do envelope. Você tem um baixo ácido.
