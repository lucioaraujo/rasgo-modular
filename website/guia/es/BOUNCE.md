## O que é
BOUNCE suelta pelotas que rebotan, cada vez más bajo y más rápido, hasta detenerse. Son hasta tres pelotas. Cada rebote es un gate, la fuerza del rebote es un acento, y la altura de cada pelota es una CV que sube y baja.

## Como pensar nele
Es un ritmo que acelera solo, como una pelota de ping-pong que se suelta sobre la mesa. ELAST decide cuánto devuelve la pelota en cada rebote, GRAV es su prisa. Con AUTO, las pelotas que se detienen se vuelven a lanzar, y WIND las empuja suavemente, así que el módulo toca para siempre sin repetirse.

## Controles
- BALLS: cuántas pelotas, de 1 a 3.
- GRAV: la gravedad. Más alta, rebotes más rápidos.
- ELAST: cuánto devuelve cada rebote. Cerca de 1, la pelota tarda en detenerse.
- SPRD: cuánto se sueltan las pelotas desde alturas distintas.
- WIND: un viento leve que impide las repeticiones exactas.
- AUTO: vuelve a lanzar las pelotas que se detuvieron.
Entradas: DROP suelta de nuevo las pelotas, GRAV mueve la gravedad por CV. Salidas: GT es el gate de cada rebote, ACC la fuerza del rebote, H1, H2 y H3 la altura de cada pelota.

## Experimente
1. Conecte GT a un DRUM y el DRUM al MIXER: escuche el rebote acelerar hasta detenerse.
2. Conecte ACC al nivel del DRUM: los primeros rebotes suenan más fuertes.
3. Conecte H1 a un QUANTIZER y el QUANTIZER a un OSC: una melodía que cae.
4. Conecte GT al CLK de un ARP: el arpegio acelera junto con la pelota.
