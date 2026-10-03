## O que é
OSC es un oscilador: produce una nota continua y estable, la materia prima de casi todos los sonidos de sintetizador. Entrega cinco formas de onda a la vez, cada una en su propia salida: senoidal, triangular, diente de sierra, pulso y una voz una o dos octavas más grave (el sub). Usted elige cuál usar, o usa varias.

## Como pensar nele
Piense en OSC como materia prima. Solo suena desnudo, y es a propósito: el carácter viene de lo que conecte después, como un filtro que quita brillo o una envolvente que da forma a la nota. Si busca una voz que ya tenga personalidad, STRING y MATTER la tienen; si quiere un timbre que se transforme solo, pruebe WAVETABLE. OSC es la base previsible, y por eso es el mejor lugar para aprender.

## Controles
- FREQ: la altura de la nota, de 8 a 8000 Hz. Los valores bajos se vuelven una vibración lenta; la zona musical está más o menos entre 50 y 1000 Hz.
- FINE: afinación fina, hasta un semitono hacia arriba o hacia abajo. Sirve para afinar con otra voz, o para desafinar un poco y engordar el sonido.
- PW: el ancho del pulso, que solo afecta a la salida PLS. En el centro la onda es cuadrada y suena hueca; cerca de los extremos se vuelve fina y nasal.
- FM: cuánto mueve la altura la señal conectada a la entrada FM. Con otro oscilador ahí y esta perilla alta, el timbre se vuelve metálico, como una campana.
- DRIFT: una oscilación lenta y pequeña en la afinación, como la de un oscilador analógico. En cero, la nota queda perfectamente quieta.
- SUB2: hace que la salida SUB quede una octava (apagado) o dos (encendido) por debajo de la nota.
- SYNC: activa la entrada SYNC. Con ella, otro oscilador más lento obliga a este a reiniciar su ciclo, y girar FREQ pasa a cambiar el timbre en lugar de la altura.
- PROX: mezcla cada salida con una versión más oscura de sí misma. Una manera de suavizar el sonido sin gastar un filtro.
Entradas: 1V/O recibe la altura desde un secuenciador o cuantizador, FM y PWM aceptan modulación, y SYNC recibe el oscilador que manda la sincronía.

## Experimente
1. Conecte la salida SAW de OSC a una entrada del MIXER y escuche un zumbido constante.
2. Gire FREQ despacio: la nota sube y baja.
3. Pase el cable a la salida PLS y mueva PW de un lado al otro. El timbre va de hueco a fino sin que la altura cambie.
4. Vuelva a SAW y haga pasar el sonido por un FILTER antes del MIXER. Ahora está haciendo síntesis sustractiva: partir de un sonido rico y quitar lo que sobra.
