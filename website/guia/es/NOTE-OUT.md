## O que é
NOTE-OUT anota las notas de una voz. Colocado en el camino de las señales de altura y de disparo, registra cada nota que pasa, con altura, duración, intensidad y acento, en la partitura que la aplicación graba junto con el audio. No tiene controles y no cambia el sonido.

## Como pensar nele
En un patch generativo, las notas aparecen y desaparecen. NOTE-OUT guarda un registro de ellas para quien quiera estudiar, transcribir o retocar la pieza después. Va en medio del camino: la altura entra por PITCH y sale igual por PTHR; el pulso entra por GATE y sale igual por GTHR. Al grabar con REC, el archivo de texto que acompaña al audio incluye esas notas.

## Controles
NOTE-OUT no tiene controles.
Entradas: GATE, el pulso de cada nota; PITCH, la altura; VEL, la intensidad; ACC, el acento. Salidas: GTHR y PTHR, las mismas señales, para seguir hasta la voz.

## Experimente
1. En una melodía hecha con QUANTIZER, OSC y ENVELOPE, conecte PTCH del QUANTIZER a PITCH de NOTE-OUT, y PTHR a 1V/O del OSC.
2. Conecte el pulso que dispara la envolvente a GATE de NOTE-OUT, y GTHR a GATE de la envolvente. El sonido sigue igual.
3. Grabe un tramo con REC. Junto al archivo de audio, el archivo de texto de la partitura lista cada nota tocada.
