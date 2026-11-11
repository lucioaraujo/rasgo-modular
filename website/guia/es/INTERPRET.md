## O que é
INTERPRET toca las notas que llegan como lo haría un intérprete. No inventa notas: recibe la altura y el gate de cualquier secuenciador y devuelve la nota tocada, con glide por el intervalo, un pequeño exceso en los saltos, vibrato que crece en la nota larga, articulación según el contexto, aliento y brillo.

## Como pensar nele
La partitura dice la nota; el intérprete dice cómo. El módulo mira las notas recientes: marca el salto, sostiene el ápice de la frase, acorta dos notas rápidas, liga el resto. El aliento se gasta a lo largo de la frase y vuelve en una pausa. Tres ejes de carácter (ENERG, SUAV, BRILHO) dan la personalidad, y DERIVA la hace cambiar despacio. Con EXPR en 0, la nota sale cruda.

## Controles
- EXPR: cuánta interpretación; en 0, la nota cruda.
- GLIDE: el tiempo de deslizar entre notas, hasta 0,3 s.
- VIB: la profundidad del vibrato en las notas largas.
- ENERG: el carácter enérgico, que marca los saltos.
- SUAV: el carácter suave, que liga las notas.
- BRILHO: el carácter brillante, que abre el timbre en el ápice.
- DERIVA: cuánto cambia el carácter con el tiempo.
Entradas: PTCH recibe la altura, GATE el gate de cada nota, CAR mueve el carácter por CV. Salidas: PTCH es la altura tocada, AMP la dinámica, BRT el brillo, FRASE la posición en la frase, GT el gate articulado.

## Experimente
1. Conecte la altura y el gate de un QUANTIZER a PTCH y GATE. Conecte la salida PTCH al 1V/O de un OSC y GT a la envolvente de la voz.
2. Ponga EXPR en 0 y luego en 0,8: compare la nota cruda con la tocada.
3. Conecte AMP al CV1 del VCA de la voz y BRT al corte del filtro: la línea gana dinámica y color.
4. Suba VIB y GLIDE en una línea lenta: empieza a cantar.
