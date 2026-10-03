## O que é
VOCODER hace que un sonido hable con la voz de otro. Escucha un sonido, normalmente una voz, y mide cuánta energía hay en cada banda de frecuencia; luego aplica ese dibujo a un segundo sonido, rico en armónicos. El resultado es el segundo sonido con el habla del primero: el efecto de voz robótica de tanta música electrónica.

## Como pensar nele
Necesita dos entradas: CAR, la portadora, que es el sonido que va a hablar (una sierra, un acorde), y MOD, que es quien habla (una voz por SIGNAL-IN, un tambor, una grabación). Con pocas bandas el resultado es tosco y robótico; con muchas, el habla se vuelve clara. ¿Nada en CAR? Usa una sierra interna, afinable por la entrada PIT.

## Controles
- BANDS: cuántas bandas de frecuencia, de 4 a 20. Pocas suenan robóticas; muchas hacen inteligible el habla.
- SHIFT: cambia el tamaño de la voz resultante, de grande a pequeña, sin cambiar lo que se dice.
- ATK: la rapidez con que responde cada banda. Corto, las consonantes salen secas; largo, todo se suaviza.
- REL: cuánto tarda cada banda en soltar. Corto es nítido; largo funde las sílabas en una textura.
- SIBIL: cuánto del agudo de la voz pasa directo, para que las S y las T se oigan claras.
- FRZ: congela el dibujo actual. La portadora sigue diciendo la última sílaba indefinidamente.
- MIX: la mezcla entre la portadora original y la portadora que habla.
Entradas: CAR, la portadora; MOD, la voz que habla; PIT, la altura de la sierra interna cuando no hay nada en CAR.

## Experimente
1. Conecte la salida SAW de un OSC a CAR, y la salida L de SIGNAL-IN, con un micrófono, a MOD. Conecte OUT a una entrada del MIXER y hable.
2. Baje BANDS a 6 y escuche la voz volverse robótica; súbalo a 20 y se vuelve clara.
3. Suba REL: las palabras se funden en un sonido continuo.
4. Sin micrófono, conecte la salida OUT de un DRUM a MOD. Cada golpe abre la portadora en su propio timbre.
