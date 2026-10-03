## O que é
STRING simula una cuerda: pulsada, como en una guitarra; percutida, como en un piano; o frotada, como en un violín. Recrea el comportamiento de una cuerda que vibra y va perdiendo energía, y por eso el ataque y el cuerpo del sonido suenan naturales aunque sean sintéticos.

## Como pensar nele
MATTER es el módulo de los objetos que resuenan, como campanas y placas; STRING es el de las cuerdas. Su gesto principal es POS, el punto donde se toca la cuerda: al moverlo, algunos armónicos desaparecen, como cuando un guitarrista toca más cerca del puente. Un pulso en PLK pulsa la cuerda; un sonido continuo en IN la hace cantar como si se tocara con arco.

## Controles
- FREQ: la afinación de la cuerda, de 20 a 4000 Hz.
- DECAY: cuánto tiempo suena la cuerda después de pulsada. Bajo es seco; alto casi no termina.
- DAMP: el brillo. Alto vuelve oscura la cuerda, como una cuerda vieja; bajo conserva el brillo de una nueva.
- POS: el punto donde se toca la cuerda. Cerca del centro, los armónicos pares desaparecen y el sonido se vuelve hueco.
- EXCIT: cuánto ruido entra en el golpe, como el sonido del dedo o de la púa.
- DRIVE: satura la cuerda por dentro, llevándola del sonido acústico a uno distorsionado.
- MIX: la mezcla entre el golpe crudo y la cuerda. Al máximo, solo la cuerda.
Entradas: IN, un sonido que hace vibrar la cuerda; PLK, un pulso que la pulsa; 1V/O, para la afinación; DMP, para cambiar el brillo.

## Experimente
1. Conecte OUT a una entrada del MIXER, y la salida EUC del CLOCK a la entrada PLK. La cuerda se pulsa al ritmo del reloj.
2. Gire DECAY: de notas secas a notas que se encadenan.
3. Mueva POS despacio y escuche cómo cambia el timbre con el punto de toque.
4. Conecte la salida PTCH de una SEQUENCE, movida por el mismo CLOCK, a la entrada 1V/O. Ahora la cuerda toca una línea melódica.
