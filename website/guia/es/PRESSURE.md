## O que é
PRESSURE es una caja con gas. Las partículas avanzan en línea recta y rebotan en las paredes. La pared de la derecha es un pistón, y cada choque en ella da un gate en HIT. La fuerza media de los choques es la presión, que sale en PRES.

## Como pensar nele
Es la teoría cinética de los gases, de Maxwell y Boltzmann. Más calor, las partículas corren más y chocan más. Menos volumen, chocan más a menudo, porque la pared está más cerca: la presión sube, como en la ley de Boyle. Con ATTR positivo, el gas se vuelve un gas real: las partículas se atraen y, con fuerza, se condensan en una gota, y la presión baja.

## Controles
- N: cuántas partículas hay en la caja.
- TEMP: la temperatura, la velocidad media.
- VOL: el volumen; la posición del pistón.
- ATTR: atracción (positivo) o repulsión (negativo) entre las partículas.
- GATE: la duración de cada gate.
Entradas: HEAT da un golpe de calor a todas, VOL y TEMP mueven volumen y temperatura por CV. Salidas: HIT es el gate de los choques en el pistón, PRES la presión, TEMP la temperatura medida, X e Y la posición de una partícula.

## Experimente
1. Conecte HIT a un DRUM con N en 3: un ritmo escaso e irregular. Suba N hasta 20: se vuelve un chisporroteo de choques.
2. Conecte un LFO lento a VOL: cuando el pistón comprime, el ritmo se acelera y PRES sube.
3. Conecte X e Y al PAN y al corte de un FILTER: la partícula dibuja el timbre.
4. Suba ATTR despacio: el gas se condensa, los choques escasean y la presión baja.
