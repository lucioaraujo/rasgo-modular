## O que é
KEY escucha a quien toca, estima la tonalidad y entrega la tónica y la escala al QUANTIZER. Así el patch pasa a responder en la tonalidad de quien canta o toca encima, en lugar de quedarse en la escala que eligió la seed.

## Como pensar nele
Cuenta cuánto tiempo sonó cada una de las doce notas (la nota larga pesa más que la de paso) y compara ese retrato con los perfiles tonales de Krumhansl y Kessler. La tonalidad solo cambia cuando otra gana durante ESTAB notas seguidas, para no temblar entre tonalidades vecinas. La salida CERT dice cuándo la tonalidad es ambigua, en lugar de esconderlo. La pantalla muestra las doce notas, la tónica y el nombre de la tonalidad.

## Controles
- MEM: cuánto tiempo recuerda, de 2 a 60 s. Corto sigue las modulaciones; largo se mantiene firme.
- ESTAB: cuántas notas seguidas debe ganar una tonalidad nueva para valer.
Entradas: PTCH recibe la altura, GATE marca cada nota, RST lo olvida todo. Salidas: ROOT es la tónica, SCL la escala, CERT la certeza, MUD un pulso en cada cambio de tonalidad.

## Experimente
1. Conecte el SIGNAL-IN al SCOPE, la salida PIT del SCOPE a PTCH y la salida ONS a GATE. Conecte ROOT y SCL a las entradas de tónica y escala del QUANTIZER.
2. Active el ON del SIGNAL-IN y cante una melodía en re menor: la pantalla muestra Dm y el patch pasa a tocar en esa tonalidad.
3. Conecte MUD a la entrada FRASE del MIXER, con el escenario en modo gesto: cada cambio de tonalidad mueve el escenario.
4. Sin micrófono: un MARKOV a un QUANTIZER, ese QUANTIZER a KEY y KEY a un segundo QUANTIZER. La segunda voz sigue la tonalidad que la primera parece tener.
