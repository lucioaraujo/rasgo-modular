## O que é
GLIDE hace que una nota se deslice hasta la siguiente en lugar de saltar. Se coloca en el camino de la señal de altura, entre quien elige las notas, como SEQUENCE o QUANTIZER, y el oscilador. Ese deslizamiento, llamado portamento, es lo que da su canto a las líneas de bajo ácidas y a los solos ligados.

## Como pensar nele
Un deslizamiento siempre activo cansa enseguida. Lo interesante es elegir cuándo deslizar, y GLIDE lo decide nota a nota: puede deslizar siempre, solo cuando la entrada SLIDE está alta, o solo cuando las notas están ligadas, es decir, cuando la nota nueva llega antes de soltar la anterior. Las salidas MOV y DONE avisan cuándo hay un deslizamiento en curso y cuándo termina, y pueden disparar otras cosas.

## Controles
- TIME: cuánto tarda en subir una octava, de cero, un salto seco, a dos segundos.
- FALL: hace la bajada más rápida o más lenta que la subida. En el centro, son iguales.
- CURVE: la forma del deslizamiento. En cero la velocidad es constante y la nota llega a tiempo exacto; al máximo va frenando y tarda más en asentarse.
- MODE: cuándo deslizar: siempre; solo con SLIDE alto, como en el sintetizador de bajo TB-303; o solo entre notas ligadas, leyendo la entrada GATE.
Entradas: PITCH, la altura que llega; SLIDE y GATE, que deciden cuándo deslizar. Salidas: OUT, la altura conducida; MOV, alta durante el deslizamiento; DONE, un pulso cuando termina.

## Experimente
1. Arme una línea: la salida CLK del CLOCK a la entrada CLK de una SEQUENCE, y la salida SAW de un OSC al MIXER.
2. Conecte PTCH de la SEQUENCE a PITCH del GLIDE, y OUT del GLIDE a 1V/O del OSC.
3. Suba TIME: las notas empiezan a deslizarse unas en otras.
4. Ponga MODE en la tercera posición y conecte GATE de la SEQUENCE a GATE del GLIDE. Ahora solo se deslizan las notas ligadas.
