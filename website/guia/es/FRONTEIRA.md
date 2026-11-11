## O que é
FRONTEIRA escucha una melodía y marca dónde empieza una idea nueva. Oímos una frontera donde algo cambia: un salto tras grados conjuntos, una nota larga tras notas cortas, una pausa. El módulo mide esos tres cambios nota a nota y pulsa cuando su fuerza pasa del umbral.

## Como pensar nele
Es el modelo de Cambouropoulos, a partir de las reglas de agrupamiento de Lerdahl y Jackendoff. Para saber si una nota es frontera necesita oír la siguiente, así que el pulso llega una nota después. Un silencio largo cierra la frase enseguida. El PHRASE genera un motivo; la FRONTEIRA escucha.

## Controles
- LIM: el umbral de fuerza por encima del cual un cambio se vuelve frontera.
- MEM: cuántas notas usa como referencia de lo normal.
- MIN: el mínimo de notas entre dos fronteras.
- SIL: cuánto silencio cierra la frase, de 0,3 a 4 s.
Entradas: PTCH recibe la altura, GATE marca cada nota. Salidas: FRONT pulsa al comienzo de cada célula, FORCA es la fuerza de la última frontera, NOTAS cuántas notas tuvo la célula, FIM pulsa cuando el silencio cierra la frase.

## Experimente
1. Conecte el SIGNAL-IN al SCOPE, la salida PIT del SCOPE a PTCH y la salida ONS a GATE. Active el ON del SIGNAL-IN.
2. Conecte FRONT a la entrada FRASE del MIXER, con el escenario en modo gesto: cada idea nueva de quien toca mueve el escenario.
3. Conecte FIM al RST de un SEQUENCE: cuando quien toca se detiene, el acompañamiento vuelve a empezar con la frase siguiente.
4. Conecte NOTAS a la duración de un ENVELOPE: las frases largas piden notas más largas.
