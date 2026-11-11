## O que é
POLYGON es la polimetría dibujada como figura. Hay un ciclo común y hasta cuatro capas; cada capa es un polígono de N vértices inscrito en ese ciclo, y cada vértice es un pulso. Un triángulo y un cuadrado en el mismo ciclo dan tres contra cuatro.

## Como pensar nele
Piense en un reloj con varias agujas invisibles, cada una marcando el ciclo en partes iguales distintas. Los pulsos de cada capa son regulares; el interés está donde se encuentran. La salida COINC marca esos encuentros, y ahí es donde la música suele pedir un acento. F gira cada polígono, hasta un lado, para desplazar la capa sin cambiar su división.

## Controles
- N1, N2, N3, N4: cuántos vértices tiene cada capa, de 0 (apagada) a 16.
- F1, F2, F3, F4: la rotación de cada capa, hasta un lado del polígono.
- CICLO: la duración del ciclo, de 0,25 a 32 s, cuando no hay nada conectado en CLK.
- PASSOS: con un reloj en CLK, cuántos pulsos dura el ciclo.
Entradas: CLK conduce el ciclo por pulsos, RST reinicia. Salidas: G1 a G4 son los pulsos de cada capa, COINC pulsa cuando dos o más vértices caen juntos, EOS al final de cada ciclo, FASE es la posición en el ciclo.

## Experimente
1. Con N1 en 3 y N2 en 4, conecte G1 a un DRUM y G2 a otro: tres contra cuatro.
2. Conecte COINC a un tercer sonido: solo los encuentros.
3. Ponga N3 en 5 y gire F3 despacio: la capa de cinco se desliza sobre las otras.
4. Conecte el CLOCK a CLK con PASSOS en 16: la polimetría pasa a andar al tempo del patch.
