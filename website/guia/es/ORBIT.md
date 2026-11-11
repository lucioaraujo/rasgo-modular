## O que é
ORBIT es un pequeño sistema solar: un sol en el centro y dos o tres planetas en gravedad. Las posiciones de los planetas salen como CV, y sus encuentros salen como pulsos.

## Como pensar nele
Son las leyes de Newton y de Kepler tocando el patch. Las órbitas más alejadas tardan más, así que los planetas andan en ritmos que casi se alinean y se desencuentran. MASS hace que los planetas se atraigan entre sí: baja, las órbitas son regulares; alta, se deshacen en caos. Un planeta que escapa vuelve a su órbita, así que el sistema nunca se pierde. LIGHT retrasa el pulso de conjunción según la distancia, como la luz que tarda en llegar.

## Controles
- PLNTS: cuántos planetas, 2 o 3.
- RATIO: cuánto se alejan las órbitas unas de otras.
- ECC: el achatamiento de las elipses.
- MASS: cuánto se atraen los planetas entre sí.
- SPEED: la velocidad del tiempo del sistema.
- LIGHT: el retraso de la conjunción según la distancia, hasta cerca de 1 s.
Entradas: NUDGE empuja el primer planeta y cambia la forma de su órbita, SPD mueve la velocidad por CV. Salidas: X1 e Y1 son la posición del primer planeta, X2 e Y2 la del segundo, DIST la distancia entre ellos, CONJ un pulso en la conjunción, PERI un pulso cuando el primer planeta pasa más cerca del sol.

## Experimente
1. Conecte CONJ a un DRUM y PERI a otro: dos ritmos lentos que casi se repiten.
2. Conecte X1 a un QUANTIZER y el QUANTIZER a un OSC: una melodía en órbita.
3. Suba MASS despacio: las órbitas se deshacen y el ritmo se vuelve imprevisible.
4. Conecte DIST al MIX de un SPACE: cuando los planetas se alejan, el espacio se abre.
