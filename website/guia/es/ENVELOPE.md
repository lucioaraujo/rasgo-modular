## O que é
ENVELOPE da forma a una nota en el tiempo: cómo empieza, cómo cae, cuánto se sostiene y cómo se apaga. Recibe un pulso en la entrada GATE y dibuja una curva con cada pulso. Ya trae un control de volumen incorporado, así que basta con hacer pasar un sonido por él para oír notas en lugar de un sonido continuo.

## Como pensar nele
Las fuentes suenan todo el tiempo; ENVELOPE convierte ese flujo en frases. Tiene dos usos que pueden darse a la vez. Por la entrada IN y la salida OUT, articula el sonido que pasa. Por la salida ENV, entrega la curva misma, que usted puede enviar a abrir un filtro, cambiar un timbre o dosificar un efecto. Las cuatro etapas de la curva tienen nombres en inglés que aparecen en todos los sintetizadores: attack, decay, sustain y release, de ahí ADSR.

## Controles
- ATK: el ataque, el tiempo que tarda la nota en llegar al máximo después del pulso. Corto suena percusivo; largo, como un arco que entra despacio.
- DEC: el decaimiento, el tiempo de la caída después del máximo hasta el nivel de SUS.
- SUS: el nivel en que la nota se sostiene mientras el pulso sigue alto. En cero no hay sostenimiento y la nota suena pulsada.
- REL: el tiempo que tarda la nota en apagarse cuando termina el pulso.
- CURVE: la forma de las curvas, de más suave a más incisiva, con el ataque percibido antes.
- TRIG: en una posición, la nota se sostiene mientras dure el pulso; en la otra, cada pulso dispara la curva entera, sin esperar.
- VCA: cuánto actúa el volumen incorporado sobre el sonido que pasa. En cero, el sonido atraviesa sin cambios y solo importa la salida ENV.
- LVL: el volumen de salida.
Entradas: IN, el sonido; GATE, el pulso que dispara la nota; TIME, para acelerar o retardar todas las etapas. Salidas: OUT, el sonido articulado; ENV, la curva.

## Experimente
1. Conecte la salida SAW de un OSC a la entrada IN, OUT a una entrada del MIXER, y la salida EUC del CLOCK a GATE. Oye notas.
2. Suba ATK y escuche cada nota entrar despacio. Vuelva y juegue con DEC para hacer las notas cortas o largas.
3. Ponga un FILTER entre el OSC y el ENVELOPE, con CUT bajo, y conecte ENV a la entrada FC del filtro. Ahora cada nota también abre el timbre.
