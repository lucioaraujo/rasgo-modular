## O que é
CLOCK es el reloj del patch. Emite pulsos regulares al tempo que usted elija y, a partir de ellos, crea ritmos y acentos. Casi todos los patches empiezan por él: sus pulsos hacen avanzar la secuencia, disparan las envolventes y tocan la percusión.

## Como pensar nele
En lugar de programar golpe a golpe, usted describe el ritmo con dos números. LEN dice cuántos pasos tiene el ciclo, FILL cuántos de ellos suenan, y CLOCK reparte esos golpes de la forma más uniforme posible. Este método, llamado ritmo euclidiano, reproduce muchos patrones tradicionales del mundo entero: 3 en 8 da el tresillo cubano, 5 en 8 el cinquillo. La salida CLK pulsa en todos los pasos; EUC solo en los pasos elegidos; ACC marca los acentos.

## Controles
- BPM: el tempo, de 20 a 300 pulsos por minuto.
- MULT: cuántos pasos caben en un pulso.
- LEN: el largo del ciclo, de 1 a 32 pasos.
- FILL: cuántos pasos del ciclo suenan en la salida EUC. Pocos dejan el ritmo escaso; cerca de LEN, casi continuo.
- ROT: gira el patrón para que empiece en otro punto. La densidad es la misma, pero la sensación cambia.
- SWING: retrasa un paso de cada dos, dando balanceo.
- DRIFT: deja que el tempo varíe levemente, como un baterista que apura y se contiene.
- GATE: cuánto tiempo queda encendido cada pulso, dentro del paso.
- ACC-A, ACC-B: los acentos caen en los pasos múltiplos de estos números. Con 4 y 3, por ejemplo, los acentos forman un patrón de tres contra cuatro.
- AND: decide si el acento necesita los dos números a la vez o basta con uno.
- FEEL: en cuántas partes se divide cada pulso: dos, tres (tresillos), cinco, siete, nueve, once, o una división sorteada en cada paso.
Entradas: EXT, para seguir pulsos de fuera; RST, para volver al principio; BPM, para modular el tempo. Salidas: CLK, EUC y ACC.

## Experimente
1. Conecte EUC a la entrada GATE de un DRUM, y OUT del DRUM a una entrada del MIXER.
2. Ponga LEN en 8 y mueva FILL de 1 a 8. En 3 y en 5 oirá patrones conocidos.
3. Gire ROT y escuche el mismo patrón empezar en otro lugar.
4. Conecte ACC a la entrada ACC del DRUM y cambie ACC-A y ACC-B para oír cómo se desplazan los acentos.
