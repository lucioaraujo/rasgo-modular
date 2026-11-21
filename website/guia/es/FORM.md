## O que é
FORM da forma a toda la pieza. Cuatro capas de sonido entran, pasan por un arco con introducción, subida, clímax, caída y coda, y salen con crescendo, diminuendo, un silencio antes del clímax y el morendo del final.

## Como pensar nele
Una forma solo se oye cuando todas las capas la siguen. Por eso FORM es un mezclador: lo que pasa por él sigue el arco. La tensión del arco también sale como señal, para que el filtro se abra en el clímax y TEMPO acelere con él. El nivel nunca pasa del nivel de entrada.

## Controles
- GRAM: la forma — arco, olas, crecimiento, calma entre agitaciones, o kishō, en que el giro es un corte seco.
- LEN: la duración aproximada, de 1 a 20 minutos.
- DEPTH: el contraste entre el valle y el clímax, en dB.
- GEST: la probabilidad de los gestos, como el silencio antes del clímax.
- MODE: imponer el arco, seguir la energía que llega por ENV, o contraponer el arco.
- SEG1 a SEG4: cuánto sigue cada capa la forma. La capa 1 es la esencial; la 4, el adorno.
- LOOP: después de la coda empieza una pieza nueva.
- SEED: la misma semilla da la misma forma.
Entradas: IN1 a IN4, las capas; CLK, para que los cambios caigan en el pulso; NEXT, la siguiente sección; RST, recomenzar; ENV, la energía a seguir. Salidas: O1 a O4 y MIX; TENS, la tensión; SECT, ROLE, CLMX, BRTH y END, los momentos de la forma.

## Experimente
1. Conecte cuatro voces a IN1 a IN4 y MIX al MIXER. Con LEN en 1, oiga la pieza subir, llegar al clímax y terminar.
2. Conecte TENS al cutoff de un FILTER: el filtro se abre con la forma.
3. Conecte SECT, TENS y END a TEMPO: el tempo sigue la misma forma.
