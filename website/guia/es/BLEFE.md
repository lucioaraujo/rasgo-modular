## O que é
BLEFE es un juego de confianza hecho de señales. Cada disparo se vuelve una señal, que suena siempre igual, sea verdad o farol. Una ventana después, la verdad se confirma y el farol no. Con cada resolución, la confianza se mueve: sube con las verdades, baja con los faroles. Cuando la confianza construida se derrumba, es traición, y la respuesta se niega hasta el perdón.

## Como pensar nele
Un número aleatorio no tiene intención; un farol sí. El módulo no sortea notas, crea una relación entre voces: una voz que responde mientras confía, duda cuando desconfía y calla después de ser traicionada. VERD es la tasa de verdad, MEM la velocidad con que cambia la confianza, LIM el punto de la traición. La entrada RST es el perdón. SEED fija el juego: la misma semilla da las mismas verdades y mentiras.

## Controles
- VERD: la tasa de verdad, de 0 a 1.
- JAN: la ventana entre la señal y la confirmación, de 0,05 a 4 s.
- MEM: la velocidad con que se mueve la confianza.
- LIM: por debajo de este nivel, la confianza construida se vuelve traición.
- SEED: la semilla del juego.
Entradas: IN recibe los disparos, RST perdona y vuelve a empezar. Salidas: SIG es la señal, siempre igual; VERD entrega enseguida la verdad escondida, para quien quiera hacer trampa; CONF pulsa cuando la verdad se confirma; CNF es el nivel de confianza; RESP es la respuesta, que calla tras la traición; TRAI pulsa en el instante de la traición.

## Experimente
1. Conecte el CLOCK a IN, SIG al gate de un ENVELOPE y CONF a un DRUM: la señal y la confirmación suenan como dos sonidos.
2. Conecte RESP al CV1 del VCA de otra voz: responde mientras confía.
3. Empiece con VERD en 0,9 y luego gírelo hacia 0,2. La voz que responde crece, duda y calla.
4. Conecte el EOS de un SEQUENCE a RST: en cada frase, la relación vuelve a empezar.
