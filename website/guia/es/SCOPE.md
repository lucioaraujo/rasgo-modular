## O que é
SCOPE muestra la forma de onda de una señal, como un osciloscopio, y mide lo que oye: el volumen, el brillo, la altura y el comienzo de cada nota. Esas mediciones salen por cables, como señales de control. Así el patch puede reaccionar a su propio sonido.

## Como pensar nele
En un osciloscopio común, la pantalla es el final del camino. Aquí, lo que SCOPE mide puede volver al patch: el brillo de la mezcla abre un filtro, la altura de una voz afina otra, el comienzo de cada nota dispara una envolvente. El sonido pasa por él sin cambios, por la salida THRU, así que puede ir en medio de cualquier cadena. Con SIGNAL-IN, también escucha un sonido de fuera y hace que Rasgo Modular lo siga.

## Controles
- TRIG: el nivel que la señal debe cruzar para estabilizar la imagen en la pantalla.
- EDGE: si ese cruce cuenta al subir o al bajar.
- REJ: un margen de tolerancia, para que el ruido no dispare la pantalla varias veces.
- RESP: la rapidez de las mediciones de volumen, brillo y altura. Rápidas, siguen cada detalle; lentas, muestran la tendencia.
- HOLD: congela las mediciones en el valor actual.
- SENS: la sensibilidad del detector de comienzo de nota. Alta, cualquier subida cuenta; baja, solo los ataques fuertes.
Entradas: IN, el sonido; EXT, una señal externa para estabilizar la pantalla. Salidas: THRU, el sonido intacto; TRIG, el pulso del disparo; LVL, el volumen; BRT, el brillo; PIT, la altura; ONS, un pulso al comienzo de cada nota.

## Experimente
1. Ponga SCOPE entre una voz y el MIXER: la voz en IN, THRU al MIXER. Mire la forma de onda.
2. Conecte BRT a la entrada FC de un FILTER de otra voz. Cuando la primera se vuelve brillante, la segunda también se abre.
3. Conecte ONS a la entrada GATE de un DRUM. Cada nota de la primera voz llega ahora con un golpe.
