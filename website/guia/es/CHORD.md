## O que é
CHORD convierte una nota en un acorde. Usted le da una altura y toca de dos a cuatro voces afinadas sobre ella, con formas que van del unísono a acordes con novena. En un modular, un acorde normalmente exige varios osciladores y cuantizadores; aquí basta un módulo.

## Como pensar nele
Piense en él como una mano sobre el teclado que ya conoce la forma del acorde. La perilla CHORD elige la forma, VOX cuántas notas tiene e INV cómo se disponen. Una onda lenta en la entrada CHRD cambia la forma sola; la salida ROOT de HARMONY en la entrada PITCH hace que la fundamental siga los cambios de tonalidad. Con VLEAD, cada voz va a la nota más cercana del acorde siguiente, deslizándose, y el cambio suena suave como un coro.

## Controles
- FREQ: la nota fundamental, de 16 a 4000 Hz.
- CHORD: la forma del acorde, entre diez tablas, del unísono a acordes con novena.
- VOX: cuántas voces suenan, de dos a cuatro.
- INV: sube una octava las notas más graves. Cambia la cara del acorde sin cambiar las notas.
- VLEAD: en el cambio de acorde, hace que cada voz vaya a la nota más cercana, deslizándose. En cero, todas saltan a la vez.
- DTUNE: desafina las voces entre sí. Un poco engorda el sonido; mucho se vuelve una pared.
- WAVE: la forma de onda de las voces, de diente de sierra a pulso y triangular.
- DRIFT: deja que cada voz oscile un poco en la afinación, despacio.
Entradas: PITCH para la fundamental, CHRD para cambiar la forma, y FM.

## Experimente
1. Conecte OUT a una entrada del MIXER y gire CHORD despacio. Escuche pasar las formas.
2. Cambie VOX de 2 a 4 y el acorde se llena.
3. Suba un poco DTUNE: las voces se separan y el sonido se ensancha.
4. Conecte la salida CLK del CLOCK a la entrada CLK de una SEQUENCE, y la salida PTCH de la SEQUENCE a la entrada PITCH del CHORD. El acorde ahora avanza con la secuencia.
