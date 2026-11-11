## O que é
MARKOV es una cadena de Márkov: aprende de dónde se va a dónde en una melodía (después de esta nota vino aquella tres veces, aquella otra una vez) e improvisa sorteando la nota siguiente según lo que aprendió. Sin nada conectado, ya toca, empezando por un paseo pentatónico.

## Como pensar nele
Es un alumno que imita el estilo de lo que oye, no las notas exactas. TEMP es su osadía: cerca de 0 repite el camino más común, cerca de 2 inventa. MEM es cuánto recuerda. La salida SURP dice cuán inesperada fue cada nota, y eso también puede mover el patch.

## Controles
- RATE: el reloj interno, cuando no hay nada conectado en CLK.
- GATE: la duración de cada nota.
- TEMP: la osadía del sorteo, de 0 a 2.
- MEM: cuánto de lo que aprendió conserva.
- LEARN: activa o desactiva el aprendizaje. Desactivado, congela su vocabulario.
- OCT: transpone la salida en octavas.
Entradas: CLK avanza una nota, LRN.P y LRN.G reciben la melodía que aprender (altura y gate), RST vuelve a empezar. Salidas: PTCH es la nota, GT el gate, SURP el grado de sorpresa.

## Experimente
1. Conecte PTCH a un OSC y GT a un ENVELOPE que controla un VCA. Escuche el paseo inicial.
2. Conecte la altura y el gate de un PHRASE a LRN.P y LRN.G: el MARKOV pasa a responder en el estilo de esa voz.
3. Después de enseñarle, desactive LEARN y suba TEMP: improvisa con el vocabulario que guardó.
4. Conecte SURP al MIX de un SPACE: las notas inesperadas ganan eco.
