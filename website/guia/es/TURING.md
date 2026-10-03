## O que é
TURING inventa melodías al azar y deja que usted guarde las que le gusten. Mantiene un bucle de unos pocos valores que gira con cada pulso; en cada vuelta, cada valor puede quedarse o ser cambiado por otro. Con el control LOCK usted decide cuánto cambia el bucle, del azar total a la repetición fija.

## Como pensar nele
Aquí se compone eligiendo con el oído, no escribiendo nota por nota. Deje LOCK en el centro, escuche cómo se transforma la melodía y, cuando aparezca algo bueno, gire LOCK hasta el final: el bucle se traba y se repite. Para variar otra vez, aflójelo un poco. El nombre homenajea al módulo Turing Machine de Music Thing Modular, que popularizó la idea. SEQUENCE toca una frase escrita; TURING hace que las frases nazcan y se fijen.

## Controles
- RATE: el reloj interno, usado cuando no llega nada a CLK.
- LEN: el largo del bucle, de 2 a 16 pasos.
- LOCK: la probabilidad de que el bucle se mantenga. En cero, todo se sortea de nuevo; al máximo, la frase se traba y se repite.
- MUT: cuando un valor cambia, cuánto cambia. Bajo, pequeñas variaciones de lo que había; alto, un sorteo nuevo.
- RANGE: la extensión de los valores de salida, es decir, el tamaño de los saltos de la melodía.
- STEPS: divide la salida en escalones. En 1, los valores son continuos.
- OFST: el centro de los valores, que sube o baja el registro de la melodía.
Entradas: CLK, el pulso que hace girar el bucle; LOCK, para modular el trabado. Salidas: CV, la melodía; CV2, otra lectura del mismo bucle, emparentada pero distinta; PLS, un ritmo sacado del mismo bucle.

## Experimente
1. Conecte la salida CLK del CLOCK a la entrada CLK de TURING.
2. Conecte CV a la entrada CV de un QUANTIZER, y PTCH del QUANTIZER a 1V/O de un OSC que pase por un ENVELOPE hasta el MIXER. Conecte PLS a la entrada GATE de la envolvente.
3. Con LOCK en el centro, escuche cómo la melodía cambia poco a poco.
4. Cuando le guste lo que oye, lleve LOCK al máximo. La frase se queda.
