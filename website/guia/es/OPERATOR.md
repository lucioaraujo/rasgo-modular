## O que é
OPERATOR es un sintetizador FM con cuatro osciladores senoidales, llamados operadores. En FM, un operador no suena solo: empuja la frecuencia de otro hacia adelante y hacia atrás muy rápido, y de eso nacen armónicos nuevos. Así se hacen pianos eléctricos, campanas, bajos chasqueantes y timbres metálicos, como en los sintetizadores digitales de los años ochenta.

## Como pensar nele
Tres decisiones hacen el timbre: quién modula a quién (ALGO), la relación de frecuencia entre los operadores (RB, RC, RD) y la fuerza de la modulación (INDEX). Empiece con INDEX bajo y súbalo poco a poco: el sonido va de una senoidal limpia a algo brillante y áspero. El gesto más útil es conectar una envolvente a la entrada IDX, para que cada nota empiece brillante y se vaya oscureciendo, como una tecla de piano eléctrico.

## Controles
- FREQ: la altura de la nota, de 8 a 8000 Hz.
- FINE: afinación fina, hasta un semitono hacia cada lado.
- ALGO: elige uno de ocho arreglos de quién modula a quién. Al principio los operadores están en cadena y el sonido es más áspero; al final suenan uno al lado del otro, más cerca de un órgano.
- RB, RC, RD: la relación de frecuencia de cada operador con el primero. Los números enteros suenan afinados; los valores fraccionarios, como 2,5, suenan a campana; los valores altos, a metal.
- INDEX: la fuerza de la modulación. En cero solo oye senoidales puras; al máximo, un timbre muy brillante.
- FBK: hace que el primer operador se module a sí mismo. Por sí solo ya convierte la senoidal en algo parecido a un diente de sierra.
- DRIFT: desafina cada operador un poquito, despacio. En cero, todo queda estable.
Entradas: 1V/O para la altura e IDX para modular la fuerza de la FM.

## Experimente
1. Conecte OUT a una entrada del MIXER con INDEX en cero: una senoidal simple.
2. Suba INDEX despacio y escuche aparecer los armónicos.
3. Ponga RB en un valor fraccionario y el sonido se vuelve campana.
4. Conecte la salida ENV de un ENVELOPE a la entrada IDX, y la salida CLK del CLOCK a la entrada GATE del ENVELOPE. Cada nota se abre brillante y se apaga, como un piano eléctrico.
